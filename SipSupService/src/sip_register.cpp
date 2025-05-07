// sip_register.cpp

#include "sip_register.h"
#include "global_ctl.h"
#include "pjsip_utils.h"

#include <array>
#include <chrono>
#include <ctime>
#include <sys/sysinfo.h>

#include <chrono>
#include <iomanip>
#include <sstream>

// 认证凭证回调函数
static pj_status_t auth_cred_callback(
    pj_pool_t *pool,
    const pj_str_t *realm,
    const pj_str_t *acc_name,
    pjsip_cred_info *cred_info  
)
{
    // 验证用户名是否正确
    pj_str_t usr = pj_str((char*)GlobalCtl::getInstance().getConfig().getSipUsr().c_str());
    if(pj_stricmp(&usr, acc_name) != 0)
    {
        LOG(ERROR) << "Username mismatch: expected " << usr.ptr << ", got " << acc_name->ptr;
        return PJ_FALSE;
    }
    pj_str_t pwd = pj_str((char*)GlobalCtl::getInstance().getConfig().getSipPwd().c_str());
    
    // 设置认证信息（realm、用户名、密码等）
    cred_info->realm = *realm;
    cred_info->username = usr;
    cred_info->data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    cred_info->data = pwd;
    return PJ_SUCCESS;
}

std::shared_ptr<SipRegister> SipRegister::instance_ = nullptr;
std::mutex SipRegister::instance_mutex_;

std::shared_ptr<SipRegister> SipRegister::getInstance(IDomainManager& domain_manager) 
{
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) 
    {
        instance_ = std::shared_ptr<SipRegister>(new SipRegister(domain_manager));
    }
    return instance_;
}

SipRegister::SipRegister(IDomainManager& domain_manager) 
    : reg_timer_(std::make_shared<TaskTimer>())
    , domain_manager_(domain_manager)
{
    reg_timer_->setInterval(10000); // 设置10秒间隔
    reg_timer_->start();
}

SipRegister::~SipRegister() 
{
    LOG(INFO) << "Destroying SipRegister";
    if (reg_timer_) 
    {
        reg_timer_->stop();
        LOG(INFO) << "Registration timer stopped";
    }
}

// 启动注册服务，设置定时器回调函数并启动定时器
void SipRegister::startRegService() 
{
    LOG(INFO) << "Starting registration service";
    if(reg_timer_)
    {
        auto self = shared_from_this();
        reg_timer_->addTask([weak_this = std::weak_ptr<SipRegister>(self)](){
            if(auto shared_this = weak_this.lock())
            {
                try{
                    shared_this->checkRegisterProc();
                    LOG(INFO) << "checkRegisterProc task executed";
                } catch (const std::exception& e) {
                    LOG(ERROR) << "Error in registration task: " << e.what();
                    throw;
                }
            }
        });
        LOG(INFO) << "Registration timer started successfully";
    } else {
        LOG(ERROR) << "Timer not initialized";
    }
}

// 定期的注册检查程序，比较当前时间和上次注册时间
void SipRegister::checkRegisterProc()
{
    LOG(INFO) << "checkRegisterProc called";

    // 获取当前系统运行时间
    time_t reg_time = 0;
    struct sysinfo info;
    if (sysinfo(&info) == 0){
        reg_time = info.uptime;
        LOG(INFO) << "System uptime: " << info.uptime << " seconds";
    }else{
        reg_time = std::time(nullptr);
        LOG(ERROR) << "Failed to get system uptime, using current time: " << reg_time;
    }
    // 使用RAII方式管理锁的生命周期
    try {
        PjSipUtils::ThreadRegistrar thread_registrar;
        std::lock_guard<std::mutex> lock(register_mutex_);
        LOG(INFO) << "checkRegisterProc: lock acquired";


        auto& domains = GlobalCtl::getInstance().getDomainInfoList();
        LOG(INFO) << "DomainInfoList size: " << domains.size();

        // 遍历所有子域信息
        for (auto& domain : domains)
        {
            if (domain.registered)
            {
                LOG(INFO) << "reg_time: " << reg_time << ", last_reg_time: " << domain.last_reg_time;
                // 检查注册是否过期
                if(reg_time - domain.last_reg_time >= domain.expires)
                {
                    // 如果过期则标记为未注册状态
                    domain.registered = false;
                    LOG(INFO) << "Registration has expired.";
                }
            }
        }
    } catch (const std::exception& e) {
        LOG(ERROR) << "Exception in checkRegisterProc: " << e.what();
    }
}

pj_status_t SipRegister::runRxTask(pjsip_rx_data* rdata) 
{
    LOG(INFO) << "runRxTask called";
    // 处理SIP消息数据(RxDataPtr)，调用链中涉及SIP栈操作，需要添加线程注册
    PjSipUtils::ThreadRegistrar thread_registrar;
    return registerReqMsg(rdata);
}

// 处理注册请求消息
pj_status_t SipRegister::registerReqMsg(pjsip_rx_data* rdata)
{
    LOG(INFO) << "registerReqMsg called";

    pjsip_msg* msg = rdata->msg_info.msg;
    // 根据认证状态决定调用哪种处理方式
    // 分为两种情况：已认证和未认证
    if(GlobalCtl::getInstance().getAuthInfo(parseFromHeader(msg)))
    {
        LOG(INFO) << "Authentication required for domain: " << parseFromHeader(msg);
        return handleAuthRegister(rdata); 
    }else{
        LOG(INFO) << "No authentication required for domain: " << parseFromHeader(msg);
        return handleRegister(rdata);
    }
}

// 处理需要认证的SIP注册请求
pj_status_t SipRegister::handleAuthRegister(pjsip_rx_data* rdata)
{
    LOG(INFO) << "handleAuthRegister called";
    
    if (!rdata || !rdata->msg_info.msg) {
        LOG(ERROR) << "Invalid rdata or message";
        return PJ_EINVAL;
    }

    pjsip_msg* msg = rdata->msg_info.msg;
    auto from_id = parseFromHeader(msg);
    
    int status_code = static_cast<int>(SipStatusCode::SIP_UNAUTHORIZED); // 401
    pj_status_t status = PJ_SUCCESS;

    // 检查是否存在认证头
    if(pjsip_msg_find_hdr(msg, PJSIP_H_AUTHORIZATION, nullptr) == nullptr)
    {
        // 创建响应消息
        pjsip_tx_data* tdata = nullptr;
        status = pjsip_endpt_create_response(
            GlobalCtl::getInstance().getSipCore().getEndPoint().get(),
            rdata,
            status_code,
            nullptr,
            &tdata);

        if (!tdata || status != PJ_SUCCESS) 
        {
            LOG(ERROR) << "Failed to create response";
            return status;
        }

        try {
            // 创建 WWW-Authenticate header
            auto hdr = pjsip_www_authenticate_hdr_create(tdata->pool);
            if (!hdr) 
            {
                // 确保在出错时释放tdata资源
                pjsip_tx_data_dec_ref(tdata);
                throw std::runtime_error("Failed to create WWW-Authenticate header");
            }

            // 设置认证参数
            hdr->scheme = pj_str((char*)"Digest");
            
            // nonce
            std::string nonce = GlobalCtl::getRandomNum(32);
            LOG(INFO) << "Generated nonce: " << nonce;
            hdr->challenge.digest.nonce = pj_strdup3(tdata->pool, nonce.c_str());

            // realm
            std::string realm_str = GlobalCtl::getInstance().getConfig().getSipRealm();
            hdr->challenge.digest.realm = pj_strdup3(tdata->pool, realm_str.c_str());
            
            // opaque
            std::string opaque = GlobalCtl::getRandomNum(32);
            LOG(INFO) << "Generated opaque: " << opaque;
            hdr->challenge.digest.opaque = pj_strdup3(tdata->pool, opaque.c_str());

            // 加密方式
            hdr->challenge.digest.algorithm = pj_str((char*)"MD5");
            
            // 添加头部到响应消息
            pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)hdr);

            // 获取响应地址并发送
            pjsip_response_addr res_addr;
            status = pjsip_get_response_addr(tdata->pool, rdata, &res_addr);
            if (status != PJ_SUCCESS) 
            {
                // 确保释放资源
                pjsip_tx_data_dec_ref(tdata);
                throw std::runtime_error("Failed to get response address");
            }

            status = pjsip_endpt_send_response(
                GlobalCtl::getInstance().getSipCore().getEndPoint().get(),
                &res_addr,
                tdata,
                nullptr,
                nullptr);

            // 不需要手动减少引用计数，pjsip_endpt_send_response会处理
            // tdata在此点已被pjsip内部管理，无需再调用pjsip_tx_data_dec_ref

        } catch (const std::exception& e) {
            LOG(ERROR) << "Exception in auth handling: " << e.what();
            status = PJ_EINVAL;
            // 确保在异常处理中也释放资源
            if (tdata) 
            {
                pjsip_tx_data_dec_ref(tdata);
            }
        }
        return status;
    }
    else
    {
        // 验证认证信息
        pjsip_tx_data* tdata { nullptr };
        
        try {
            pjsip_auth_srv auth_srv;
            std::string realm_str = GlobalCtl::getInstance().getConfig().getSipRealm();
            pj_str_t realm = pj_str((char*)realm_str.c_str());
            
            // 确保使用正确的池创建auth_srv
            // 不要使用rdata->tp_info.pool，因为它可能在某些情况下无效
            pj_pool_t* tmp_pool = pjsip_endpt_create_pool(
                GlobalCtl::getInstance().getSipCore().getEndPoint().get(),
                "auth_pool", 4000, 4000);
            
            if (!tmp_pool) 
            {
                throw std::runtime_error("Failed to create temporary pool");
            }

            status = pjsip_auth_srv_init(tmp_pool, &auth_srv, &realm, &auth_cred_callback, 0);
            if(status != PJ_SUCCESS) 
            {
                pjsip_endpt_release_pool(GlobalCtl::getInstance().getSipCore().getEndPoint().get(), tmp_pool);
                throw std::runtime_error("Failed to initialize authentication server");
            }

            int verify_status = status_code;
            status = pjsip_auth_srv_verify(&auth_srv, rdata, &verify_status);
            if (status == PJ_SUCCESS) 
            {
                status_code = static_cast<int>(SipStatusCode::SIP_OK);
            }

            // 创建响应消息
            status = pjsip_endpt_create_response(
                GlobalCtl::getInstance().getSipCore().getEndPoint().get(),
                rdata,
                status_code,
                nullptr,
                &tdata);

            // 释放临时池
            pjsip_endpt_release_pool(GlobalCtl::getInstance().getSipCore().getEndPoint().get(), tmp_pool);

           if (!tdata) 
           {
                throw std::runtime_error("Failed to create response");
            }

            // 添加日期头部
            if (!addDateHeader(tdata->msg, tdata->pool)) 
            {
                throw std::runtime_error("Failed to add Date header");
            }

            // 获取响应地址并发送
            pjsip_response_addr res_addr;
            status = pjsip_get_response_addr(tdata->pool, rdata, &res_addr);
            if (status != PJ_SUCCESS) {
                throw std::runtime_error("Failed to get response address");
            }

            status = pjsip_endpt_send_response(
                GlobalCtl::getInstance().getSipCore().getEndPoint().get(),
                &res_addr,
                tdata,
                nullptr,
                nullptr);

        } catch (const std::exception& e) {
            LOG(ERROR) << "Exception in auth verification: " << e.what();
            status = PJ_EINVAL;
        }

        // 确保资源释放，无论成功与否
        if (tdata) 
        {
            pjsip_tx_data_dec_ref(tdata);
        }

        return status;
    }
}

// 普通注册处理
pj_status_t SipRegister::handleRegister(pjsip_rx_data* rdata)
{
    LOG(INFO) << "handleRegister called";
    // 生成32位随机数（用于安全目的）
    std::string random = GlobalCtl::getRandomNum(32);
    // 创建线程注册器实例，用于管理线程相关的资源
    PjSipUtils::ThreadRegistrar thread_registrar;
    // 检查输入参数是否有效
    // 如果接收数据或消息为空，记录错误并返回无效参数错误码
    if (!rdata || !rdata->msg_info.msg) 
    {
        LOG(ERROR) << "handleRegister: rdata or message is null";
        return PJ_EINVAL;
    }

    // 尝试从SIP消息中解析From头部
    std::string from_id;
    try{
        from_id = parseFromHeader(rdata->msg_info.msg);
    }catch(const std::exception& e){
        LOG(ERROR) << "Failed to parse From header: " << e.what();
        return PJ_EINVAL;
    }

    // 初始化状态码为200（OK）
    int status_code { static_cast<int>(SipStatusCode::SIP_OK) };
    // 初始化过期时间为0
    int expires_value { 0 };

    // 域检查
    bool domain_exists { false };
    {
        // 使用读锁保护域管理器的并发访问，确保锁在合适范围内被释放
        std::shared_lock<std::shared_mutex> lock(domain_manager_.getMutex());  // 读锁
        // 检查请求的域是否存在
        auto domain = domain_manager_.findDomain(from_id);
        domain_exists = (domain != nullptr);
    } // 读锁在此释放

    // 如果域不存在，返回404错误
    if (!domain_exists)
    {
        status_code = static_cast<int> (SipStatusCode::SIP_NOT_FOUND);
        LOG(ERROR) << "Domain not found: " << from_id;
        return PJ_EINVAL;
    }
    // 如果域存在，获取Expires头部的值
    else if(auto expires_raw = static_cast<pjsip_expires_hdr*>(
            pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_EXPIRES, nullptr)))
    {
        expires_value = expires_raw->ivalue;
        LOG(INFO) << "Expires header: " << expires_value;
    }

    // 创建响应消息
    // 获取SIP终端点
    auto endpt = GlobalCtl::getInstance().getSipCore().getEndPoint();
    if (!endpt) 
    {
        LOG(ERROR) << "Failed to get endpoint";
        return PJ_EINVAL;
    }

    // 不要用智能指针管理 txdata ，交给 PJSIP 内部管理
    // 创建响应消息
    pjsip_tx_data* txdata { nullptr };
    auto status = pjsip_endpt_create_response(
        endpt.get(),
        rdata,
        status_code, 
        nullptr, 
        &txdata
    );
    if (status != PJ_SUCCESS || !txdata) 
    {
        LOG(ERROR) << "Failed to create response: " << status;
        return status;
    }

    // 添加日期头部和发送响应
    if(!addDateHeader(txdata->msg, rdata->tp_info.pool))
    {
        LOG(ERROR) << "Failed to add Date header";
        pjsip_tx_data_dec_ref(txdata);  // 确保释放资源
        return PJ_EINVAL;
    }
    // 获取响应地址
    pjsip_response_addr res_addr;
    status = pjsip_get_response_addr(txdata->pool, rdata, &res_addr);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "Failed to get response address: " << status;
        pjsip_tx_data_dec_ref(txdata);  // 确保释放资源
        return status;
    }
    // 发送响应消息
    status = pjsip_endpt_send_response(
        endpt.get(),
        &res_addr,
        txdata,
        nullptr,
        nullptr
    );
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "Failed to send response: " << status;
        pjsip_tx_data_dec_ref(txdata);  // 确保释放资源
        return status;
    }

    // 更新注册状态
    {
        // 使用写锁保护更新操作
        std::unique_lock<std::shared_mutex> lock(domain_manager_.getMutex()); 
        LOG(INFO) << "handleRegister: lock acquired for updateRegistration";
        // 根据过期时间更新注册状态
        // 如果过期时间大于0，记录注册时间
        if(expires_value > 0)
        {
            time_t reg_time = 0;
            // 获取系统运行时间
            // 这里使用 sysinfo 来获取系统运行时间
            struct sysinfo info;
            if (sysinfo(&info) == 0){
                reg_time = info.uptime;
                LOG(INFO) << "System uptime: " << info.uptime << " seconds";
            }else{
                reg_time = std::time(nullptr);
                LOG(ERROR) << "Failed to get system uptime, using current time: " << reg_time;
            }
            domain_manager_.updateRegistration(from_id, expires_value, true, reg_time);
            LOG(INFO) << "Registration successful for domain: " << from_id;
            LOG(INFO) << "Registration time: " << reg_time;
        }
        // 如果过期时间为0，表示注销请求
        else if(expires_value == 0)
        {
            domain_manager_.updateRegistration(from_id, 0, false, 0);
            LOG(INFO) << "Unregistration successful for domain: " << from_id;
        }
    } // 写锁在此释放
    return status;
}

// 获取当前UTC时间
std::tm SipRegister::getCurrentUTC()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t_now = std::chrono::system_clock::to_time_t(now);
    struct tm tm_utc;
    
    #ifdef _WIN32
        if (gmtime_s(&tm_utc, &t_now) != 0) {
            throw std::runtime_error("Failed to get UTC time");
        }
    #else
        if (!gmtime_r(&t_now, &tm_utc)) {
            throw std::runtime_error("Failed to get UTC time");
        }
    #endif
    
    return tm_utc;
}

// 格式化为SIP协议要求的格式
std::string SipRegister::formatSIPDate(const std::tm& tm_utc)
{
    char buf[128];
    // RFC 3261规范的格式：Fri, 02 May 2025 02:42:16 GMT
    if (!std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_utc)) 
    {
        throw std::runtime_error("Failed to format UTC time");
    }
    LOG(INFO) << "Formatted SIP Date: " << buf;
    return std::string(buf);
}

// 添加SIP Date header
bool SipRegister::addDateHeader(pjsip_msg* msg, pj_pool_t* pool)
{
    try {
        // 获取当前UTC时间
        std::tm tm_utc = getCurrentUTC();
        
        // 格式化为SIP日期格式
        std::string date_str = formatSIPDate(tm_utc);
        
        // 创建并添加header
        pj_str_t value_time = pj_str(const_cast<char*>(date_str.c_str()));
        pj_str_t key_time = pj_str(const_cast<char*>("Date"));
        
        auto date_hdr = pjsip_date_hdr_create(pool, &key_time, &value_time);
        if (!date_hdr) 
        {
            LOG(ERROR) << "Failed to create Date header";
            return false;
        }
        
        pjsip_msg_add_hdr(msg, reinterpret_cast<pjsip_hdr*>(date_hdr));
        LOG(INFO) << "Date header added: " << date_str;
        return true;
        
    } catch (const std::exception& e) {
        LOG(ERROR) << "Failed to add Date header: " << e.what();
        return false;
    }
}



std::string SipRegister::parseFromHeader(pjsip_msg* msg)
{
    if (!msg) {
        LOG(ERROR) << "parseFromHeader: msg is null";
        throw std::runtime_error("Message is null");
    }

    // 获取 From 头部
    auto from_hdr = static_cast<pjsip_from_hdr*>(
        pjsip_msg_find_hdr(msg, PJSIP_H_FROM, nullptr));
    
    if (!from_hdr) {
        LOG(ERROR) << "parseFromHeader: From header not found";
        throw std::runtime_error("From header not found");
    }

    // 检查 URI 是否有效
    if (!from_hdr->uri) {
        LOG(ERROR) << "parseFromHeader: From URI is null";
        throw std::runtime_error("From URI is null");
    }

    // 创建缓冲区用于打印 URI
    std::array<char, 1024> buf{};
    
    // 获取打印方法
    auto print_uri = pjsip_uri_print(PJSIP_URI_IN_FROMTO_HDR, 
                                   from_hdr->uri, 
                                   buf.data(), 
                                   buf.size());
    
    if (print_uri <= 0) {
        LOG(ERROR) << "parseFromHeader: Failed to print From URI";
        throw std::runtime_error("Failed to print From URI");
    }

    // 将打印结果转换为字符串
    std::string uri_str(buf.data(), print_uri);
    
    // 解析 SIP URI 格式: sip:user@domain
    size_t sip_prefix = uri_str.find("sip:");
    if (sip_prefix == std::string::npos) {
        LOG(ERROR) << "parseFromHeader: Invalid SIP URI format";
        throw std::runtime_error("Invalid SIP URI format: " + uri_str);
    }

    // 跳过 "sip:" 前缀
    size_t start = sip_prefix + 4;
    
    // 查找 @ 符号
    size_t at_pos = uri_str.find('@', start);
    if (at_pos == std::string::npos) {
        LOG(ERROR) << "parseFromHeader: Invalid SIP URI format (no @)";
         // 如果没有 @ 符号，抛出异常
        throw std::runtime_error("Invalid SIP URI format (no @): " + uri_str);
    }

    // 提取用户 ID (从 sip: 后面到 @ 之前)
    std::string user_id = uri_str.substr(start, at_pos - start);
    
    if (user_id.empty()) {
        LOG(ERROR) << "parseFromHeader: Empty user ID in URI";
         // 如果用户 ID 为空，抛出异常
        throw std::runtime_error("Empty user ID in URI: " + uri_str);
    }

    LOG(INFO) << "Parsed From header user ID: " << user_id;
    return user_id;
}



