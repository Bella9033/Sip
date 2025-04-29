#include "sip_register.h"
#include "global_ctl.h"

#include <array>
#include <chrono>
#include <ctime>

std::shared_ptr<SipRegister> SipRegister::createInstance()
{
    auto instance = std::make_shared<SipRegister>();
    // 使用weak_ptr存储
    instance->sip_reg_ = instance;
    LOG(INFO) << "SipRegister instance created";
    return instance;
}

// 构造函数中获取IDomainManager引用
SipRegister::SipRegister()
    : reg_timer_(std::make_shared<TaskTimer>()),
      domain_manager_(GlobalCtl::getInstance()) // 注入依赖
{}

SipRegister::~SipRegister() 
{
    if (reg_timer_) 
    { 
        reg_timer_->stop(); 
    }
    LOG(INFO) << "SipRegister destroyed";
}

void SipRegister::startRegService() 
{
    if (reg_timer_) 
    {
        LOG(INFO) << "Register timer started.";
    }
}

pj_status_t SipRegister::runRxTask(SipTypes::RxDataPtr rdata) 
{
    LOG(INFO) << "runRxTask called";
    return registerReqMsg(rdata);
}

pj_status_t SipRegister::registerReqMsg(SipTypes::RxDataPtr rdata)
{
    if (!rdata) 
    {
        LOG(ERROR) << "registerReqMsg: rdata is null";
        return PJ_EINVAL;
    }
    
    return handleReg(rdata);
}

pj_status_t SipRegister::handleReg(SipTypes::RxDataPtr rdata)
{
    LOG(INFO) << "Handling register logic";
    if (!rdata || !rdata->msg_info.msg) 
    {
        LOG(ERROR) << "handleReg: rdata is null";
        return PJ_EINVAL;
    }

    std::array<char,1024> buf {};
    int size { 0 };
    std::string from_id;
    int status_code { 200 };

    // 使用 get() 获取原始指针
    auto from_raw = static_cast<pjsip_from_hdr*>(pjsip_msg_find_hdr(
        rdata->msg_info.msg, 
        PJSIP_H_FROM, 
        nullptr));

    if(from_raw == nullptr) 
    {
        LOG(ERROR) << "from_raw is nullptr";
        return -1;
    }
    if(from_raw->vptr && from_raw->vptr->print_on)
    {
        size = from_raw->vptr->print_on(from_raw, buf.data(), static_cast<int>(buf.size()));
    }
    if(size > 0)
    {
        std::string temp(buf.data(), size);
        if(temp.size() > 11)
        {
            from_id = temp.substr(11,20);
            LOG(INFO) << "from_id: " << from_id;
        }
    }

    // 使用接口而不是直接调用GlobalCtl
    if(!domain_manager_.checkIsValid(from_id))
    {
        status_code = static_cast<int>(StatusCode::SIP_NOT_FOUND);
    }
    else
    {
        auto expires_raw = static_cast<pjsip_expires_hdr*>(
            pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_EXPIRES, nullptr));
        pj_int32_t expires_value = expires_raw->ivalue;
        domain_manager_.setExpires(from_id, expires_value);
    }

    pjsip_tx_data* tdata;
    auto status = pjsip_endpt_create_response(
        GlobalCtl::getInstance().getSipCore().getEndPoint().get(),
        rdata.get(),
        status_code, 
        nullptr, 
        &tdata);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_endpt_create_response failed: " << status;
        return status;
    }

    // 使用UTC时间而不是本地时间
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm* utc_tm = std::gmtime(&time_t_now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", utc_tm);
    std::string time_str(buffer);
    LOG(INFO) << "Current UTC time: " << time_str;

    pj_str_t value_time = pj_str(const_cast<char*>(time_str.c_str()));
    pj_str_t key_time = pj_str(const_cast<char*>("Data"));
    auto date_header = pjsip_date_hdr_create(rdata->tp_info.pool, &key_time, &value_time);
    pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)date_header);
    
    return PJ_SUCCESS;
}