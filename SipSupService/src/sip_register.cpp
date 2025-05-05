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

    PjSipUtils::ThreadRegistrar thread_registrar;
    std::lock_guard<std::mutex> lock(register_mutex_);

    time_t reg_time = 0;
    struct sysinfo info;
    if (sysinfo(&info) == 0){
        reg_time = info.uptime;
        LOG(INFO) << "System uptime: " << info.uptime << " seconds";
    }else{
        reg_time = std::time(nullptr);
        LOG(ERROR) << "Failed to get system uptime, using current time: " << reg_time;
    }

    auto& domains = GlobalCtl::getInstance().getDomainInfoList();
    LOG(INFO) << "DomainInfoList size: " << domains.size();
    for (auto& domain : domains)
    {
        if (domain.registered)
        {
            LOG(INFO) << "reg_time: " << reg_time << ", last_reg_time: " << domain.last_reg_time;
            if(reg_time - domain.last_reg_time >= domain.expires)
            {
                domain.registered = false;
                LOG(INFO) << "Registration has expired.";
            }
        }
    }
}

pj_status_t SipRegister::runRxTask(SipTypes::RxDataPtr rdata) 
{
    // 处理SIP消息数据(RxDataPtr)，调用链中涉及SIP栈操作，需要添加线程注册
    PjSipUtils::ThreadRegistrar thread_registrar;
    LOG(INFO) << "runRxTask called";
    return registerReqMsg(rdata);
}


pj_status_t SipRegister::registerReqMsg(SipTypes::RxDataPtr rdata)
{
    LOG(INFO) << "registerReqMsg called";
    if (!rdata) 
    {
        LOG(ERROR) << "registerReqMsg: rdata is null";
        return PJ_EINVAL;
    }
    // 处理SIP消息数据(RxDataPtr)，调用链中涉及SIP栈操作，需要添加线程注册
    PjSipUtils::ThreadRegistrar thread_registrar;
    pjsip_msg* msg = rdata->msg_info.msg;
    if(GlobalCtl::getInstance().getAuthInfo(parseFromHeader(msg)))
    {
        LOG(INFO) << "Authentication required for domain: " << parseFromHeader(msg);
        return handleAuthRegister(rdata); 
    }else{
        LOG(INFO) << "No authentication required for domain: " << parseFromHeader(msg);
        return handleRegister(rdata);
    }
}

pj_status_t SipRegister::handleAuthRegister(SipTypes::RxDataPtr rdata)
{
    LOG(INFO) << "handleAuthRegister called";
    pjsip_msg* msg = rdata->msg_info.msg;
    pjsip_hdr* hdr_list;
    pj_list_init(&hdr_list);
    pj_status_t status = PJ_SUCCESS;
    int  status_code = static_cast<int>(SipStatusCode::SIP_FORBIDEN);
    if(pjsip_msg_find_hdr(msg, PJSIP_H_AUTHORIZATION, nullptr) == nullptr)
    {
        auto hdr = pjsip_www_authenticate_hdr_create(rdata->tp_info.pool);
        hdr->scheme = pj_str("Digest");
        std::string nonce = GlobalCtl::getRandomNum(32);
        LOG(INFO) << "Generated nonce: " << nonce;
        hdr->challenge.digest.nonce = pj_str((char*)nonce.c_str());      
        hdr->challenge.digest.realm = pj_str((char*)GlobalCtl::getInstance().getConfig().getRealm().c_str());
        std::string nonce = GlobalCtl::getRandomNum(32);
        LOG(INFO) << "Generated nonce: " << nonce;
        hdr->challenge.digest.opaque = pj_str((char*)nonce.c_str());
        hdr->challenge.digest.algorithm = pj_str("MD5");

        pj_list_push_back(&hdr_list, hdr);
        status = pjsip_endpt_respond(GlobalCtl::getInstance().getSipCore().getEndPoint().get(), 
            rdata.get(), status_code, nullptr, &hdr_list, nullptr, nullptr);
        if(status != PJ_SUCCESS) 
        {
            LOG(ERROR) << "Failed to send authentication response: " << status;
            return status;
        }
    }

}


pj_status_t SipRegister::handleRegister(SipTypes::RxDataPtr rdata)
{
    LOG(INFO) << "handleRegister called";
    std::string random = GlobalCtl::getRandomNum(32);
    // 处理SIP消息数据(RxDataPtr)，调用链中涉及SIP栈操作，需要添加线程注册
    PjSipUtils::ThreadRegistrar thread_registrar;
    if (!rdata || !rdata->msg_info.msg) 
    {
        LOG(ERROR) << "handleRegister: rdata or message is null";
        return PJ_EINVAL;
    }

    
    std::string from_id;
    try{
        from_id = parseFromHeader(rdata->msg_info.msg);
    }catch(const std::exception& e){
        LOG(ERROR) << "Failed to parse From header: " << e.what();
        return PJ_EINVAL;
    }

    int status_code = static_cast<int>(SipStatusCode::SIP_OK);
    int expires_value { 0 };

    // checkIsValid改为只判断是否有这个id（注册状态后续要改）
    // 只要有该域即可（不必已注册）
    // 域检查使用omain_manager_替代GlobalCtl::getInstance()
    bool domain_exists = false;
    {
        std::shared_lock<std::shared_mutex> lock(domain_manager_.getMutex());  // 读锁
        auto domain = domain_manager_.findDomain(from_id);
        domain_exists = (domain != nullptr);
    }

    if (!domain_exists)
    {
        status_code = static_cast<int> (SipStatusCode::SIP_NOT_FOUND);
        LOG(ERROR) << "Domain not found: " << from_id;
        return PJ_EINVAL;
    }
    else if(auto expires_raw = static_cast<pjsip_expires_hdr*>(
            pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_EXPIRES, nullptr)))
    {
        expires_value = expires_raw->ivalue;
        LOG(INFO) << "Expires header: " << expires_value;
    }

    // SipCore 相关操作仍然使用 GlobalCtl::getInstance()
    // SipCore 是全局资源，不能在多线程中创建和销毁
    // getSipCore() 本来就是 GlobalCtl 的特有功能，不属于域管理的职责
    // 没必要为了统一接口而在 IDomainManager 中添加与域管理无关的方法
    auto endpt = GlobalCtl::getInstance().getSipCore().getEndPoint();
    if (!endpt) 
    {
        LOG(ERROR) << "Failed to get endpoint";
        return PJ_EINVAL;
    }

    // 不要用智能指针管理 txdata 交给 PJSIP 内部管理
    pjsip_tx_data* txdata { nullptr };
    auto status = pjsip_endpt_create_response(
        endpt.get(),
        rdata.get(),
        status_code, 
        nullptr, 
        &txdata
    );
    if (status != PJ_SUCCESS || !txdata) 
    {
        LOG(ERROR) << "Failed to create response: " << status;
        return status;
    }

    if(!addDateHeader(txdata->msg, rdata->tp_info.pool))
    {
        LOG(ERROR) << "Failed to add Date header";
        return PJ_EINVAL;
    }

    pjsip_response_addr res_addr;
    status = pjsip_get_response_addr(txdata->pool, rdata.get(), &res_addr);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "Failed to get response address: " << status;
        return status;
    }

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
        return status;
    }

    {
        std::unique_lock<std::shared_mutex> lock(domain_manager_.getMutex()); 
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
        }else if(expires_value == 0)
        {
            domain_manager_.updateRegistration(from_id, 0, false, 0);
            LOG(INFO) << "Unregistration successful for domain: " << from_id;
        }
    }
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
        if (!date_hdr) {
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



