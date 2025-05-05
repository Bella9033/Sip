// sip_register.cpp
#include "sip_register.h"
#include "global_ctl.h"
#include "pjsip_utils.h"
#include <array>
#include <chrono>
#include <ctime>
#include <sys/sysinfo.h>
#include <exception>

std::shared_ptr<SipRegister> SipRegister::instance_ = nullptr;
std::mutex SipRegister::instance_mutex_;

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

static void client_cb(struct pjsip_regc_cbparam *param)
{
    LOG(INFO) << "code: " << param->code;
    if (param->code == 200)
    {
        auto* domain = (DomainInfo*)param->token;
        if(domain)
        {
            domain->registered = true;
            LOG(INFO) << "domain.registered = true";
            LOG(INFO) << "Registered successfully for domain: " << domain->sip_id;
        }
    }
    return;
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

    if (config.getSipId().empty() || config.getSipIp().empty())
    {
        LOG(ERROR) << "Local config error: sip_id or sip_ip is empty!";
        LOG(ERROR) << "sip_id=" << config.getSipId() << ", sip_ip=" << config.getSipIp();
        return PJ_EINVAL;
    }
    if (domains.sip_id.empty() || domains.addr_ip.empty() || domains.sip_port <= 0)
    {
        LOG(ERROR) << "Domain config error: id, ip or port is invalid!";
        LOG(ERROR) << "sip_id=" << domains.sip_id << ", addr_ip=" << domains.addr_ip << ", sip_port=" << domains.sip_port;
        return PJ_EINVAL;
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
    pj_status_t status = pjsip_regc_create(
        GlobalCtl::getInstance().getSipCore().getEndPoint().get(),
        nullptr,
        &client_cb,
        &regc);
    if (status != PJ_SUCCESS || !regc)
    {
        LOG(ERROR) << "pjsip_regc_create failed, code: " << status;
        return status;
    }

    status = pjsip_regc_init(regc, &req_line, &from, &to, 1, &contact, domains.expires);
    if (status != PJ_SUCCESS)
    {
        pjsip_regc_destroy(regc);
        LOG(ERROR) << "pjsip_regc_init failed, code: " << status;
        return status;
    }

    if(domains.isAuth)
    {
        pjsip_cred_info cred;
        pj_bzero(&cred, sizeof(pjsip_cred_info));
        cred.scheme = pj_str("digest");
        cred.realm = pj_str((char*)domains.realm.c_str());
        cred.username = pj_str((char*)domains.usr.c_str());
        cred.data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
        cred.data = pj_str((char*)domains.pwd.c_str());

        pjsip_regc_set_credentials(regc,1,&cred);
        if(status != PJ_SUCCESS)
        {
            pjsip_regc_destroy(regc);
            LOG(ERROR) << "pjsip_regc_set_credentials failed, code: " << status;
            return status;
        }

    }

    pjsip_tx_data* tdata { nullptr };
    status = pjsip_regc_register(regc, PJ_TRUE, &tdata);
    if (status != PJ_SUCCESS)
    {
        pjsip_regc_destroy(regc);
        LOG(ERROR) << "pjsip_regc_register failed, code: " << status;
        return status;
    }


    status = pjsip_regc_send(regc, tdata);
    if (status != PJ_SUCCESS)
    {
        pjsip_regc_destroy(regc);
        LOG(ERROR) << "pjsip_regc_send failed, code: " << status;
        return status;
    }
    LOG(INFO) << "REGISTER sent successfully for domain: " << domains.sip_id;
    return PJ_SUCCESS;
}