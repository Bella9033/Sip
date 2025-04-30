// sip_register.cpp

#include "sip_register.h"
#include "global_ctl.h"
#include "pjsip_utils.h"

#include <array>
#include <chrono>
#include <ctime>

// 初始化静态成员
std::shared_ptr<SipRegister> SipRegister::instance_ = nullptr;
std::mutex SipRegister::singleton_mutex_;

// 实现单例模式的getInstance方法
std::shared_ptr<SipRegister> SipRegister::getInstance()
{
    std::lock_guard<std::mutex> lock(singleton_mutex_);
    if (!instance_) {
        instance_ = std::shared_ptr<SipRegister>(new SipRegister());
        instance_->sip_reg_ = instance_;
        LOG(INFO) << "SipRegister singleton instance created";
    }
    return instance_;
}

// 构造函数中获取IDomainManager引用
SipRegister::SipRegister()
    : reg_timer_(std::make_shared<TaskTimer>()),
      initialized_(false)
{ }

SipRegister::~SipRegister() 
{
    if (reg_timer_) 
    { 
        reg_timer_->stop(); 
    }
    LOG(INFO) << "SipRegister destroyed";
}

static void client_cb(struct pjsip_regc_cbparam *param)
{
    LOG(INFO) << "code: " << param->code;
    if (param->code == 200) // 如果服务器返回 200 OK，表示注册成功
    {
        // 通过回调上下文中的 token 获取对应的 domain 信息
        auto* domain = (DomainInfo*)param->token;
        if(domain) 
        {
            domain->setRegistered(); // 更新注册状态为已注册
            LOG(INFO) << "domain.registered = true";
            LOG(INFO) << "Registered successfully for domain: " << domain->sip_id;
        }
    }
    // 其他错误码处理可以根据需要补充
    return;
}

void SipRegister::startRegService() 
{
    LOG(INFO) << "registerServiceStart called";
    // 启动定时器，定时器会在指定时间间隔内调用 registerProc 函数
    // 用 shared_ptr 管理对象，并用 weak_ptr 安全绑定回调，避免悬垂指针和内存泄漏。
    std::weak_ptr<SipRegister> self_weak = shared_from_this(); 
    if (reg_timer_) 
    {  
        // 设置定时器到时回调（Lambda捕获this，调用成员函数）
        reg_timer_->addTask([self_weak]() { 
            // 尝试获取当前对象的强引用
            // 如果对象仍然存在，则调用 registerProc
            if (auto ptr = self_weak.lock()) 
            { 
                LOG(INFO) << "Register timer triggered.";
                ptr->registerProc(); // 使用获取的强引用调用方法
            } else 
            {
                LOG(ERROR) << "Failed to lock weak pointer, object may be destroyed.";
            }
        });
        // 确保只初始化一次
        std::lock_guard<std::mutex> lock(init_mutex_);
        LOG(INFO) << "Lock acquired for initialization.";
        if (!initialized_) 
        {
            reg_timer_->start(); 
            initialized_ = true;
            LOG(INFO) << "Register timer initialized.";
        }
    }
}


void SipRegister::registerProc() 
{
    LOG(INFO) << "registerProc called";
    std::lock_guard<std::mutex> lock(register_mutex_);
    auto& domains = GlobalCtl::getInstance().getDomainInfoList();
    LOG(INFO) << "UpNodeInfo domains size: " << domains.size();
    if (domains.empty()) 
    {
        LOG(WARNING) << "No domains to register. Check configuration.";
        return;
    }
    for (auto& domain : domains) 
    {
        if (domain.registered == false) 
        {
            LOG(INFO) << "Registering domain: " << domain.sip_id;
            // 发起注册请求，使用 gbRegister 函数
            if (gbRegister(domain) != PJ_SUCCESS) 
            {
                LOG(ERROR) << "gbRegister failed for domain: " << domain.sip_id;
            } 
        }
    }
}

// 其余函数保持不变...
// 发起SIP REGISTER请求，注册指定下级域
// 注册请求的来源信息正确反映了本地节点信息
// 目标信息指向要注册的下级域
pj_status_t SipRegister::gbRegister(DomainInfo& domains)
{
    LOG(INFO) << "gbRegister called for domains: " << domains.sip_id;
    
    // 获取本地配置信息
    auto& config = GlobalCtl::getInstance().getConfig();

    // 检查本地配置是否有效
    if (config.getSipId().empty() || config.getSipIp().empty()) {
        LOG(ERROR) << "Local config error: sip_id or sip_ip is empty!";
        LOG(ERROR) << "sip_id=" << config.getSipId() << ", sip_ip=" << config.getSipIp();
        return PJ_EINVAL;
    }

    LOG(INFO) << "Using local SIP config - ID: " << config.getSipId() 
              << ", IP: " << config.getSipIp() 
              << ", Port: " << config.getSipPort();
    
    // 检查域配置是否有效
    if (domains.sip_id.empty() || domains.addr_ip.empty() || domains.sip_port <= 0) {
        LOG(ERROR) << "Domain config error: id, ip or port is invalid!";
        LOG(ERROR) << "sip_id=" << domains.sip_id 
                   << ", addr_ip=" << domains.addr_ip 
                   << ", sip_port=" << domains.sip_port;
        return PJ_EINVAL;
    }

    // 构建SIP消息头，正确构建 From 和 To
    std::string from_hdr = fmt::format("<sip:{}@{}:{}>", 
        config.getSipId(), config.getSipIp(), config.getSipPort());
    
    // 修复：To 头应该使用目标域信息，而不是本地配置
    std::string to_hdr = fmt::format("<sip:{}@{}:{}>", 
        domains.sip_id, domains.addr_ip, domains.sip_port);
    
    std::string contact_hdr = fmt::format("sip:{}@{}:{}", 
        config.getSipId(), config.getSipIp(), config.getSipPort());
    
    std::string req_uri = fmt::format("sip:{}@{}:{};transport={}", 
        domains.sip_id, domains.addr_ip, domains.sip_port,
        domains.proto == 1 ? "tcp" : "udp");

    LOG(INFO) << "From header: " << from_hdr;
    LOG(INFO) << "To header: " << to_hdr;
    LOG(INFO) << "Contact header: " << contact_hdr;
    LOG(INFO) << "Request URI: " << req_uri;

    // 使用安全的缓冲区拷贝，避免指针生命周期问题
    char from_buf[256], to_buf[256], contact_buf[256], req_buf[256];
    std::strncpy(from_buf, from_hdr.c_str(), sizeof(from_buf)-1);
    std::strncpy(to_buf, to_hdr.c_str(), sizeof(to_buf)-1);
    std::strncpy(contact_buf, contact_hdr.c_str(), sizeof(contact_buf)-1);
    std::strncpy(req_buf, req_uri.c_str(), sizeof(req_buf)-1);
    
    // 确保以 null 结尾
    from_buf[sizeof(from_buf)-1] = '\0';
    to_buf[sizeof(to_buf)-1] = '\0';
    contact_buf[sizeof(contact_buf)-1] = '\0';
    req_buf[sizeof(req_buf)-1] = '\0';

    // 转为 PJSIP 格式
    pj_str_t from = pj_str(from_buf);
    pj_str_t to = pj_str(to_buf);
    pj_str_t contact = pj_str(contact_buf);
    pj_str_t req_line = pj_str(req_buf);

    // 检查域的 expires 设置，确保有有效值
    if (domains.expires <= 0) {
        LOG(WARNING) << "Domain expires is invalid or zero, using default 3600";
        domains.expires = 3600;
    }


    pjsip_regc* regc;
    pj_status_t status = pjsip_regc_create(
        GlobalCtl::getInstance().getSipCore().getEndPoint().get(),
        nullptr,
        &client_cb,
        &regc);

    if (status != PJ_SUCCESS || !regc) // 检查创建是否成功
    {
        LOG(ERROR) << "pjsip_regc_create failed, code: " << status;
        return status;
    }

    // 初始化注册客户端实例
    status = pjsip_regc_init(regc, &req_line, &from, &to, 1, &contact, domains.expires);
    if (status != PJ_SUCCESS) // 如果初始化失败，销毁注册客户端实例
    {
        pjsip_regc_destroy(regc);
        LOG(ERROR) << "pjsip_regc_init failed, code: " << status;
        return status;
    }

    // 创建 REGISTER 请求
    pjsip_tx_data* tdata { nullptr };
    status = pjsip_regc_register(regc, PJ_TRUE, &tdata);
    if (status != PJ_SUCCESS) // 如果请求创建失败，销毁注册客户端实例
    {
        pjsip_regc_destroy(regc);
        LOG(ERROR) << "pjsip_regc_register failed, code: " << status;
        return status;
    }

    // 发送 REGISTER 请求
    status = pjsip_regc_send(regc, tdata);
    if (status != PJ_SUCCESS) // 如果发送失败，销毁注册客户端实例
    {
        pjsip_regc_destroy(regc);
        LOG(ERROR) << "pjsip_regc_send failed, code: " << status;
        return status;
    }

    LOG(INFO) << "REGISTER sent successfully for domain: " << domains.sip_id;
    return PJ_SUCCESS; // 注册成功
}