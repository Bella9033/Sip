// sip_register.cpp
#include "sip_register.h"
#include "global_ctl.h"
#include "pjsip_utils.h"
#include <array>
#include <chrono>
#include <ctime>
#include <sys/sysinfo.h>
#include <exception>
#include <unordered_map>

// 新增 AuthCache 结构体：
// 用于存储每个域的认证信息，包括从401响应中提取的realm。
struct AuthCache 
{
    std::string nonce;
    std::string opaque;
    std::string realm;
    bool has_auth_info { false };
    time_t last_update { 0 };
};

std::shared_ptr<SipRegister> SipRegister::instance_ = nullptr;
std::mutex SipRegister::instance_mutex_;

// 新增：认证信息缓存
static std::unordered_map<std::string, AuthCache> g_auth_cache;
static std::mutex g_auth_cache_mutex;

std::shared_ptr<SipRegister> SipRegister::getInstance(IDomainManager& domain_manager)
{
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_)
        instance_ = std::shared_ptr<SipRegister>(new SipRegister(domain_manager));
    return instance_;
}

SipRegister::SipRegister(IDomainManager& domain_manager)
    : reg_timer_(std::make_shared<TaskTimer>())
    , domain_manager_(domain_manager)
{
    reg_timer_->setInterval(3000);
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

// 新增：从401响应中提取认证信息
bool SipRegister::extractAuthInfo(pjsip_rx_data* rdata, const std::string& domain_id)
{
    if (!rdata) {
        LOG(ERROR) << "Invalid response data for extractAuthInfo";
        return false;
    }

    pjsip_www_authenticate_hdr* auth_hdr = (pjsip_www_authenticate_hdr*)
        pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_WWW_AUTHENTICATE, NULL);

    if (!auth_hdr) {
        LOG(ERROR) << "No WWW-Authenticate header in 401 response for domain: " << domain_id;
        return false;
    }

    std::lock_guard<std::mutex> lock(g_auth_cache_mutex);
    AuthCache& auth_cache = g_auth_cache[domain_id];

    // 提取realm
    if (auth_hdr->challenge.digest.realm.slen > 0) {
        auth_cache.realm = std::string(auth_hdr->challenge.digest.realm.ptr, 
                                      auth_hdr->challenge.digest.realm.slen);
        LOG(INFO) << "Extracted realm: " << auth_cache.realm << " for domain: " << domain_id;
    } else {
        LOG(WARNING) << "No realm in WWW-Authenticate header for domain: " << domain_id;
        return false;
    }

    // 提取nonce
    if (auth_hdr->challenge.digest.nonce.slen > 0) {
        auth_cache.nonce = std::string(auth_hdr->challenge.digest.nonce.ptr, 
                                      auth_hdr->challenge.digest.nonce.slen);
        LOG(INFO) << "Extracted nonce: " << auth_cache.nonce << " for domain: " << domain_id;
    }

    // 提取opaque
    if (auth_hdr->challenge.digest.opaque.slen > 0) {
        auth_cache.opaque = std::string(auth_hdr->challenge.digest.opaque.ptr, 
                                       auth_hdr->challenge.digest.opaque.slen);
        LOG(INFO) << "Extracted opaque: " << auth_cache.opaque << " for domain: " << domain_id;
    }

    auth_cache.has_auth_info = true;
    auth_cache.last_update = time(NULL);
    
    return true;
}

void SipRegister::startRegService()
{
    LOG(INFO) << "Starting registration service";
    if (reg_timer_)
    {
        auto self = shared_from_this();
        reg_timer_->addTask([weak_this = std::weak_ptr<SipRegister>(self)]() {
            if (auto shared_this = weak_this.lock())
            {
                try {
                    shared_this->registerProc();
                    LOG(INFO) << "Registration task executed";
                } catch (const std::exception& e) {
                    LOG(ERROR) << "Error in registration task: " << e.what();
                    throw;
                }
            }
        });
        LOG(INFO) << "Registration timer started successfully";
    }
    else
    {
        LOG(ERROR) << "Timer not initialized";
    }
}

// 回调函数增加对401响应的处理
static void client_cb(struct pjsip_regc_cbparam *param)
{
    LOG(INFO) << "Registration response code: " << param->code;
    
    auto* domain = (DomainInfo*)param->token;
    if (!domain) {
        LOG(ERROR) << "Invalid domain token in registration callback";
        return;
    }
    
    std::string domain_id = domain->sip_id;
    
    if (param->code == 200) {
        domain->registered = true;
        LOG(INFO) << "domain.registered = true";
        LOG(INFO) << "Registered successfully for domain: " << domain_id;
    } 
    else if (param->code == 401) {
        LOG(INFO) << "Received 401 Unauthorized for domain: " << domain_id;
        LOG(INFO) << "Will extract authentication information for next attempt";
        
        // 修改这里的调用方式
        auto sipRegister = SipRegister::getInstance(GlobalCtl::getInstance());
        sipRegister->extractAuthInfo(param->rdata, domain_id);
    }else {
        LOG(WARNING) << "Registration failed with code " << param->code 
                   << " for domain: " << domain_id;
    }
}

void SipRegister::registerProc()
{
    LOG(INFO) << "registerProc called";
    PjSipUtils::ThreadRegistrar thread_registrar;
    std::lock_guard<std::mutex> lock(register_mutex_);
    auto& domains = GlobalCtl::getInstance().getDomainInfoList();
    LOG(INFO) << "DomainInfoList size: " << domains.size();
    if (domains.empty())
    {
        LOG(WARNING) << "No domains to register. Check configuration.";
        return;
    }
    for (auto& domain : domains)
    {
        if (!domain.registered)
        {
            LOG(INFO) << "Registering domain: " << domain.sip_id;
            if (gbRegister(domain) != PJ_SUCCESS)
            {
                LOG(ERROR) << "gbRegister failed for domain: " << domain.sip_id;
            }
        }
    }
}

pj_status_t SipRegister::gbRegister(DomainInfo& domains)
{
    LOG(INFO) << "gbRegister called for domains: " << domains.sip_id;
    auto& config = GlobalCtl::getInstance().getConfig();
    pj_status_t status = PJ_SUCCESS;
    
    // 使用do-while(0)模式统一处理错误
    do {
        if (config.getSipId().empty() || config.getSipIp().empty())
        {
            LOG(ERROR) << "Local config error: sip_id or sip_ip is empty!";
            LOG(ERROR) << "sip_id=" << config.getSipId() << ", sip_ip=" << config.getSipIp();
            status = PJ_EINVAL;
            break;
        }
        
        if (domains.sip_id.empty() || domains.addr_ip.empty() || domains.sip_port <= 0)
        {
            LOG(ERROR) << "Domain config error: id, ip or port is invalid!";
            LOG(ERROR) << "sip_id=" << domains.sip_id << ", addr_ip=" << domains.addr_ip << ", sip_port=" << domains.sip_port;
            status = PJ_EINVAL;
            break;
        }

        std::string from_hdr = fmt::format("<sip:{}@{}:{}>", config.getSipId(), config.getSipIp(), config.getSipPort());
        std::string to_hdr = fmt::format("<sip:{}@{}:{}>", domains.sip_id, domains.addr_ip, domains.sip_port);
        std::string contact_hdr = fmt::format("sip:{}@{}:{}", config.getSipId(), config.getSipIp(), config.getSipPort());
        std::string req_uri = fmt::format("sip:{}@{}:{};transport={}", domains.sip_id, domains.addr_ip, domains.sip_port, domains.proto == 1 ? "tcp" : "udp");

        char from_buf[256], to_buf[256], contact_buf[256], req_buf[256];
        std::strncpy(from_buf, from_hdr.c_str(), sizeof(from_buf)-1);
        std::strncpy(to_buf, to_hdr.c_str(), sizeof(to_buf)-1);
        std::strncpy(contact_buf, contact_hdr.c_str(), sizeof(contact_buf)-1);
        std::strncpy(req_buf, req_uri.c_str(), sizeof(req_buf)-1);

        from_buf[sizeof(from_buf)-1] = '\0';
        to_buf[sizeof(to_buf)-1] = '\0';
        contact_buf[sizeof(contact_buf)-1] = '\0';
        req_buf[sizeof(req_buf)-1] = '\0';

        pj_str_t from = pj_str(from_buf);
        pj_str_t to = pj_str(to_buf);
        pj_str_t contact = pj_str(contact_buf);
        pj_str_t req_line = pj_str(req_buf);

        if (domains.expires <= 0)
        {
            LOG(WARNING) << "Domain expires is invalid or zero, using default 3600";
            domains.expires = 3600;
        }

        pjsip_regc* regc;
        status = pjsip_regc_create(
            GlobalCtl::getInstance().getSipCore().getEndPoint().get(),
            &domains, // 传递domains指针作为token
            &client_cb,
            &regc);
        if (status != PJ_SUCCESS || !regc)
        {
            LOG(ERROR) << "pjsip_regc_create failed, code: " << status 
                     << ", error: " << PjSipUtils::getPjStatusString(status);
            break;
        }

        status = pjsip_regc_init(regc, &req_line, &from, &to, 1, &contact, domains.expires);
        if (status != PJ_SUCCESS)
        {
            LOG(ERROR) << "pjsip_regc_init failed, code: " << status
                     << ", error: " << PjSipUtils::getPjStatusString(status);
            pjsip_regc_destroy(regc);
            break;
        }

        if(domains.isAuth)
        {
            pjsip_cred_info cred;
            pj_bzero(&cred, sizeof(pjsip_cred_info));
            
            char digest_scheme[] = "digest"; 
            cred.scheme = pj_str(digest_scheme);
            
            // 优化：优先使用缓存的realm，然后使用配置的realm，最后使用默认值
            std::string realm_to_use;
            
            {
                std::lock_guard<std::mutex> lock(g_auth_cache_mutex);
                auto it = g_auth_cache.find(domains.sip_id);
                if (it != g_auth_cache.end() && it->second.has_auth_info) {
                    realm_to_use = it->second.realm;
                    LOG(INFO) << "Using cached realm: " << realm_to_use << " for domain: " << domains.sip_id;
                }
            }
            
            if (realm_to_use.empty()) {
                // 若没有缓存的realm，尝试使用配置的realm
                realm_to_use = domains.realm;
                if (realm_to_use.empty()) {
                    // 最后的备选方案，使用"11000000002000000001"作为默认值
                    realm_to_use = "11000000002000000001";
                    LOG(WARNING) << "Using default realm: " << realm_to_use << " for domain: " << domains.sip_id;
                } else {
                    LOG(INFO) << "Using configured realm: " << realm_to_use << " for domain: " << domains.sip_id;
                }
            }
            
            std::vector<char> realm_buf(realm_to_use.begin(), realm_to_use.end());
            realm_buf.push_back('\0'); // 确保字符串以null结尾
            
            cred.realm = pj_str(realm_buf.data());
            
            // 用户名和密码
            std::string username = domains.usr;
            std::string password = domains.pwd;
            
            if (username.empty() || password.empty()) {
                LOG(ERROR) << "Auth credentials missing for domain: " << domains.sip_id;
                LOG(ERROR) << "Username: " << (username.empty() ? "MISSING" : username);
                LOG(ERROR) << "Password: " << (password.empty() ? "MISSING" : "****");
                status = PJ_EINVAL;
                pjsip_regc_destroy(regc);
                break;
            }
            
            std::vector<char> username_buf(username.begin(), username.end());
            username_buf.push_back('\0');
            
            std::vector<char> password_buf(password.begin(), password.end());
            password_buf.push_back('\0');
            
            cred.username = pj_str(username_buf.data());
            cred.data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
            cred.data = pj_str(password_buf.data());
            
            LOG(INFO) << "Setting auth credentials for domain: " << domains.sip_id;
            LOG(INFO) << "Realm: " << realm_to_use << ", Username: " << username;

            status = pjsip_regc_set_credentials(regc, 1, &cred);
            if(status != PJ_SUCCESS)
            {
                LOG(ERROR) << "pjsip_regc_set_credentials failed, code: " << status
                         << ", error: " << PjSipUtils::getPjStatusString(status);
                pjsip_regc_destroy(regc);
                break;
            }
        }
        else {
            LOG(WARNING) << "Authentication disabled for domain: " << domains.sip_id;
        }

        pjsip_tx_data* tdata { nullptr };
        status = pjsip_regc_register(regc, PJ_TRUE, &tdata);
        if (status != PJ_SUCCESS)
        {
            LOG(ERROR) << "pjsip_regc_register failed, code: " << status
                     << ", error: " << PjSipUtils::getPjStatusString(status);
            pjsip_regc_destroy(regc);
            break;
        }

        status = pjsip_regc_send(regc, tdata);
        if (status != PJ_SUCCESS)
        {
            LOG(ERROR) << "pjsip_regc_send failed, code: " << status
                     << ", error: " << PjSipUtils::getPjStatusString(status);
            pjsip_regc_destroy(regc);
            break;
        }
        
        LOG(INFO) << "REGISTER sent successfully for domain: " << domains.sip_id;
    } while (0);
    
    return status;
}

// 新增：将PJSIP错误码转换为字符串描述的实用方法
std::string PjSipUtils::getPjStatusString(pj_status_t status) {
    static char buf[PJ_ERR_MSG_SIZE];
    pj_strerror(status, buf, sizeof(buf));
    return std::string(buf);
}