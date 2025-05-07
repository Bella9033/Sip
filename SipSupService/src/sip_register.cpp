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
    // 验证参数
    if (!pool || !realm || !acc_name || !cred_info) {
        LOG(ERROR) << "Invalid parameters in auth_cred_callback";
        return PJ_EINVAL;
    }

    try {
        // 记录请求中的用户名
        std::string acc_name_str(acc_name->ptr, acc_name->slen);
        LOG(INFO) << "Auth request for username: " << acc_name_str;
        
        // 获取配置中的用户名和密码
        std::string usr_str = GlobalCtl::getInstance().getConfig().getSipUsr();
        std::string pwd_str = GlobalCtl::getInstance().getConfig().getSipPwd();
        if (usr_str.empty()) 
        {
            LOG(ERROR) << "Empty SIP username configuration";
            return PJ_EINVAL;
        }

        // 增加用户名验证逻辑
        std::string config_usr = GlobalCtl::getInstance().getConfig().getSipUsr();
        LOG(INFO) << fmt::format("Configure_username: {}, Username: {}, Password: {}", 
            config_usr, acc_name_str, pwd_str);

        // 验证用户名匹配
        pj_str_t usr;
        if (pj_stricmp(acc_name, pj_cstr(&usr, config_usr.c_str())) != 0)
        {
            LOG(ERROR) << "Username mismatch";
            return PJ_EINVAL;
        }
        
        // 设置认证信息
        cred_info->realm = *realm;
        cred_info->username = *acc_name; 
        cred_info->data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
        cred_info->data = pj_str((char*)pwd_str.c_str());
        
        LOG(INFO) << "Credentials set for requested username: " << acc_name_str;
        return PJ_SUCCESS;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Exception in auth_cred_callback: " << e.what();
        return PJ_EINVAL;
    }
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
                // 增加超时处理的容错机制
                if (reg_time < domain.last_reg_time) 
                {
                    LOG(WARNING) << "System time anomaly detected";
                    reg_time = std::time(nullptr);
                }
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

// 修改为接收智能指针
// 修改为使用智能指针
pj_status_t SipRegister::runRxTask(SipTypes::RxDataPtr rdata) 
{
    LOG(INFO) << "runRxTask called with rdata=" << (void*)rdata.get();
    PjSipUtils::ThreadRegistrar thread_registrar;
    return registerReqMsg(rdata);
}

// 处理注册请求消息，修改为接收智能指针
pj_status_t SipRegister::registerReqMsg(SipTypes::RxDataPtr rdata)
{
    LOG(INFO) << "registerReqMsg called with rdata=" << (void*)rdata.get();
    
    if (!rdata || !rdata->msg_info.msg) {
        LOG(ERROR) << "Invalid rdata or message";
        return PJ_EINVAL;
    }

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

// 处理需要认证的SIP注册请求，修改为接收智能指针
pj_status_t SipRegister::handleAuthRegister(SipTypes::RxDataPtr rdata)
{
    LOG(INFO) << "handleAuthRegister called with rdata=" << (void*)rdata.get();
    
    if (!rdata || !rdata->msg_info.msg) {
        LOG(ERROR) << "Invalid rdata or message";
        return PJ_EINVAL;
    }

    // 使用互斥锁保护认证过程
    std::lock_guard<std::mutex> lock(auth_mutex_);

    // 获取SIP终端点
    auto endpt = GlobalCtl::getInstance().getSipCore().getEndPoint();
    if (!endpt) {
        LOG(ERROR) << "Failed to get SIP endpoint";
        return PJ_EINVAL;
    }

    pjsip_msg* msg = rdata->msg_info.msg;
    auto from_id = parseFromHeader(msg);
    LOG(INFO) << "Processing auth request for user: " << from_id;
    
    int status_code = static_cast<int>(SipStatusCode::SIP_UNAUTHORIZED); // 401
    pj_status_t status = PJ_SUCCESS;

    // 检查是否存在认证头
    auto auth_hdr = pjsip_msg_find_hdr(msg, PJSIP_H_AUTHORIZATION, nullptr);
    if(auth_hdr == nullptr)
    {
        LOG(INFO) << "No Authorization header found, sending challenge";
    
        // 创建响应消息
        pjsip_tx_data* tdata = nullptr;
        status = pjsip_endpt_create_response(
            endpt.get(),
            rdata.get(),
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
            if (!hdr) {
                pjsip_tx_data_dec_ref(tdata);
                throw std::runtime_error("Failed to create WWW-Authenticate header");
            }

            // 设置认证参数
            hdr->scheme = pj_str((char*)"Digest");
            
            // nonce
            std::string nonce = GlobalCtl::getRandomNum(32);
            LOG(INFO) << "Generated nonce: " << nonce;
            hdr->challenge.digest.nonce = pj_strdup3(tdata->pool, nonce.c_str());

            // realm - 使用from_id作为realm，确保匹配
            hdr->challenge.digest.realm = pj_strdup3(tdata->pool, from_id.c_str());

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
            status = pjsip_get_response_addr(tdata->pool, rdata.get(), &res_addr);
            if (status != PJ_SUCCESS) {
                pjsip_tx_data_dec_ref(tdata);
                throw std::runtime_error("Failed to get response address");
            }

            status = pjsip_endpt_send_response(
                endpt.get(),
                &res_addr,
                tdata,
                nullptr,
                nullptr);

        } catch (const std::exception& e) {
            LOG(ERROR) << "Exception in auth handling: " << e.what();
            status = PJ_EINVAL;
            if (tdata) {
                pjsip_tx_data_dec_ref(tdata);
            }
        }
        return status;
    }
    else
    {
        // 验证认证信息
        pjsip_tx_data* tdata = nullptr;
        
        // 检查认证头信息
        pjsip_authorization_hdr* auth_hdr;

        if (auth_hdr) 
        {
            std::string auth_username(auth_hdr->credential.digest.username.ptr, 
                                    auth_hdr->credential.digest.username.slen);
            std::string auth_realm(auth_hdr->credential.digest.realm.ptr, 
                                auth_hdr->credential.digest.realm.slen);
            LOG(INFO) << "Authorization header found, username: " << auth_username 
                    << ", realm: " << auth_realm;
        }
        
        try {
            LOG(INFO) << "Creating temporary pool for authentication";
            pj_pool_t* tmp_pool = pjsip_endpt_create_pool(
                endpt.get(),
                "auth_pool", 4000, 4000);
            
            if (!tmp_pool) {
                throw std::runtime_error("Failed to create temporary pool");
            }
            LOG(INFO) << "Created temporary pool: " << (void*)tmp_pool;

            // RAII方式管理临时池的生命周期
            std::unique_ptr<void, std::function<void(void*)>> pool_guard(
                tmp_pool,
                [&endpt](void* p) { 
                    if (p) {
                        pjsip_endpt_release_pool(endpt.get(), static_cast<pj_pool_t*>(p));
                        LOG(INFO) << "Authentication pool released";
                    }
                }
            );

            // 认证验证代码
            pjsip_auth_srv auth_srv;
            auto realm = pj_str((char*)GlobalCtl::getInstance().getConfig().getSipRealm().c_str());
            status = pjsip_auth_srv_init(tmp_pool, &auth_srv, &realm, &auth_cred_callback, 0);
            if(status != PJ_SUCCESS) {
                throw std::runtime_error("Failed to initialize auth server");
            }
            pjsip_auth_srv_verify(&auth_srv, rdata.get(), &status_code);
            // // 自定义认证处理，跳过PJSIP内置认证机制
            // // 这里直接假设认证成功，在实际应用中应该进行真实的密码验证
            // status_code = static_cast<int>(SipStatusCode::SIP_OK);
            // LOG(INFO) << "Authentication successful, setting status code to 200";

            // 创建响应消息
            status = pjsip_endpt_create_response(
                endpt.get(),
                rdata.get(),
                status_code,
                nullptr,
                &tdata);

            if (!tdata) {
                throw std::runtime_error("Failed to create response");
            }
            LOG(INFO) << "Created response with status code: " << status_code;

            // 添加日期头部
            if (!addDateHeader(tdata->msg, tdata->pool)) {
                throw std::runtime_error("Failed to add Date header");
            }
            LOG(INFO) << "Date header added to response";

            // 获取响应地址并发送
            pjsip_response_addr res_addr;
            status = pjsip_get_response_addr(tdata->pool, rdata.get(), &res_addr);
            if (status != PJ_SUCCESS) {
                throw std::runtime_error("Failed to get response address");
            }

            LOG(INFO) << "Sending response...";
            status = pjsip_endpt_send_response(
                endpt.get(),
                &res_addr,
                tdata,
                nullptr,
                nullptr);
            LOG(INFO) << "Response sent with status: " << status;
            
            // 如果认证成功，更新注册状态
            if (status == PJ_SUCCESS && status_code == static_cast<int>(SipStatusCode::SIP_OK)) 
            {
                // 获取Expires头部值
                int expires_value = 3600;  // 默认1小时
                if (auto expires_hdr = (pjsip_expires_hdr*)pjsip_msg_find_hdr(msg, PJSIP_H_EXPIRES, nullptr)) 
                {
                    expires_value = expires_hdr->ivalue;
                }
                GlobalCtl::getInstance().setExpires(from_id, expires_value);
                
                // 使用写锁保护更新操作
                std::unique_lock<std::shared_mutex> domain_lock(domain_manager_.getMutex()); 
                LOG(INFO) << "Updating registration for domain: " << from_id;
                
                time_t reg_time = 0;
                struct sysinfo info;
                if (sysinfo(&info) == 0) {
                    reg_time = info.uptime;
                } else {
                    reg_time = std::time(nullptr);
                }
                
                domain_manager_.updateRegistration(from_id, expires_value, true, reg_time);
                LOG(INFO) << "Registration updated: expires=" << expires_value << ", time=" << reg_time;
            }

        } catch (const std::exception& e) {
            LOG(ERROR) << "Exception in auth verification: " << e.what();
            status = PJ_EINVAL;
        }

        // 确保资源释放，无论成功与否
        if (tdata) 
        {
            pjsip_tx_data_dec_ref(tdata);
            LOG(INFO) << "Transaction data reference decremented";
        }

        return status;
    }
}

// 普通注册处理，修改为接收智能指针
pj_status_t SipRegister::handleRegister(SipTypes::RxDataPtr rdata)
{
    LOG(INFO) << "handleRegister called with rdata=" << (void*)rdata.get();
    // 生成32位随机数（用于安全目的）
    std::string random = GlobalCtl::getRandomNum(32);
    // 创建线程注册器实例，用于管理线程相关的资源
    PjSipUtils::ThreadRegistrar thread_registrar;
    // 检查输入参数是否有效
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
        rdata.get(),  // 使用智能指针的get()方法获取原始指针
        status_code, 
        nullptr, 
        &txdata
    );
    if (status != PJ_SUCCESS || !txdata) 
    {
        LOG(ERROR) << "Failed to create response: " << status;
        return status;
    }

    // 创建池用于日期头部，而不是使用rdata->tp_info.pool
    pj_pool_t* date_pool = pjsip_endpt_create_pool(
        endpt.get(),
        "date_pool", 
        1000, 
        1000
    );
    if (!date_pool) {
        LOG(ERROR) << "Failed to create date pool";
        pjsip_tx_data_dec_ref(txdata);
        return PJ_ENOMEM;
    }

    // RAII方式管理日期池
    std::unique_ptr<void, std::function<void(void*)>> date_pool_guard(
        date_pool,
        [&endpt](void* p) { 
            if (p) {
                pjsip_endpt_release_pool(endpt.get(), static_cast<pj_pool_t*>(p));
                LOG(INFO) << "Date pool released";
            }
        }
    );

    // 添加日期头部和发送响应
    if(!addDateHeader(txdata->msg, txdata->pool))  // 使用txdata->pool而不是rdata->tp_info.pool
    {
        LOG(ERROR) << "Failed to add Date header";
        pjsip_tx_data_dec_ref(txdata);
        return PJ_EINVAL;
    }
    
    // 获取响应地址
    pjsip_response_addr res_addr;
    status = pjsip_get_response_addr(txdata->pool, rdata.get(), &res_addr);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "Failed to get response address: " << status;
        pjsip_tx_data_dec_ref(txdata);
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
        pjsip_tx_data_dec_ref(txdata);
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



