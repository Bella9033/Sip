#include "sip_register.h"
#include "global_ctl.h"
#include "pjsip_utils.h"

#include <array>
#include <chrono>
#include <ctime>
#include <sys/sysinfo.h>

std::shared_ptr<SipRegister> SipRegister::create(IDomainManager& domain_manager) {
    return std::make_shared<SipRegister>(domain_manager);
}

SipRegister::SipRegister(IDomainManager& domain_manager)
    : reg_timer_(std::make_shared<TaskTimer>(3000)) // 3秒间隔
    , domain_manager_(domain_manager)
{
    if (!reg_timer_) {
        throw std::runtime_error("Failed to create registration timer");
    }
}

SipRegister::~SipRegister() {
    is_running_ = false;
    if (reg_timer_) {
        reg_timer_->stop();
    }
}

void SipRegister::startRegService() {
    LOG(INFO) << "Starting registration service";
    
    // 设置运行标志
    bool expected = false;
    if (!is_running_.compare_exchange_strong(expected, true)) {
        LOG(WARNING) << "Registration service already running";
        return;
    }

    if (!reg_timer_) {
        LOG(ERROR) << "Timer not initialized";
        return;
    }

    // 使用weak_ptr防止循环引用
    auto weak_this = std::weak_ptr<SipRegister>(shared_from_this());
    reg_timer_->addTask([weak_this]() {
        if (auto shared_this = weak_this.lock()) {
            try {
                LOG(INFO) << "Executing registration task";
                // 这里可以添加具体的注册任务逻辑
            } catch (const std::exception& e) {
                LOG(ERROR) << "Error in registration task: " << e.what();
            }
        }
    });

    if (!reg_timer_->start()) {
        LOG(ERROR) << "Failed to start registration timer";
        is_running_ = false;
        return;
    }
    
    LOG(INFO) << "Registration timer started successfully";
}

pj_status_t SipRegister::runRxTask(PjSipUtils::RxDataPtr rdata) {
    PjSipUtils::ThreadRegistrar thread_registrar;
    LOG(INFO) << "runRxTask called";
    return registerReqMsg(rdata);
}

pj_status_t SipRegister::registerReqMsg(PjSipUtils::RxDataPtr rdata) {
    PjSipUtils::ThreadRegistrar thread_registrar;
    if (!rdata) {
        LOG(ERROR) << "registerReqMsg: rdata is null";
        return PJ_EINVAL;
    }
    return handleRegister(rdata);
}

pj_status_t SipRegister::handleRegister(PjSipUtils::RxDataPtr rdata) {
    LOG(INFO) << "handleRegister called";
    PjSipUtils::ThreadRegistrar thread_registrar;

    if (!rdata || !rdata->msg_info.msg) {
        LOG(ERROR) << "handleRegister: rdata or message is null";
        return PJ_EINVAL;
    }

    std::string from_id;
    try {
        from_id = parseFromHeader(rdata->msg_info.msg);
    } catch (const std::exception& e) {
        LOG(ERROR) << "Failed to parse From header: " << e.what();
        return PJ_EINVAL;
    }

    LOG(INFO) << "Parsed From header user ID: " << from_id;
    int status_code = static_cast<int>(SipStatusCode::SIP_OK);
    int expires_value = 0;

    // 使用读锁检查域是否存在
    {
        std::shared_lock<std::shared_mutex> lock(register_mutex_);
        auto domain = domain_manager_.findDomain(from_id);
        if (!domain) {
            status_code = static_cast<int>(SipStatusCode::SIP_NOT_FOUND);
            LOG(ERROR) << "Domain not found: " << from_id;
            return PJ_EINVAL;
        }
    }

    // 处理Expires头部
    if (auto expires_hdr = static_cast<pjsip_expires_hdr*>(
            pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_EXPIRES, nullptr))) {
        expires_value = expires_hdr->ivalue;
        LOG(INFO) << "Expires header: " << expires_value;
    }

    // 获取SIP终点
    auto endpt = GlobalCtl::getInstance().getSipCore().getEndPoint();
    if (!endpt) {
        LOG(ERROR) << "Failed to get endpoint";
        return PJ_EINVAL;
    }

    // 创建响应
    pjsip_tx_data* txdata = nullptr;
    auto status = pjsip_endpt_create_response(
        endpt.get(),
        rdata.get(),
        status_code,
        nullptr,
        &txdata
    );

    if (status != PJ_SUCCESS || !txdata) {
        LOG(ERROR) << "Failed to create response: " << status;
        return status;
    }

    auto txdata_guard = PjSipUtils::makeTxData(txdata);

    // 添加日期头部
    if (!addDateHeader(txdata->msg, rdata->tp_info.pool)) {
        LOG(ERROR) << "Failed to add Date header";
        return PJ_EINVAL;
    }

    // 发送响应
    pjsip_response_addr res_addr;
    status = pjsip_get_response_addr(txdata->pool, rdata.get(), &res_addr);
    if (status != PJ_SUCCESS) {
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

    if (status != PJ_SUCCESS) {
        LOG(ERROR) << "Failed to send response: " << status;
        return status;
    }

    // 更新注册状态
    time_t reg_time = 0;
    if (expires_value > 0) {
        struct sysinfo info;
        if (sysinfo(&info) == 0) {
            reg_time = info.uptime;
        } else {
            reg_time = std::time(nullptr);
        }
        updateRegistrationStatus(from_id, expires_value, true, reg_time);
        LOG(INFO) << "Registration successful for domain: " << from_id;
        LOG(INFO) << "Registration time: " << reg_time;
    } else if (expires_value == 0) {
        updateRegistrationStatus(from_id, 0, false, 0);
        LOG(INFO) << "Unregistration successful for domain: " << from_id;
    }

    return status;
}

void SipRegister::updateRegistrationStatus(
    const std::string& from_id,
    pj_int32_t expires_value,
    bool is_registered,
    time_t reg_time
) {
    std::unique_lock<std::shared_mutex> lock(register_mutex_);
    domain_manager_.updateRegistration(from_id, expires_value, is_registered, reg_time);
}