// sip_register.cpp

#include "sip_register.h"
#include "global_ctl.h"
#include "pjsip_utils.h"

#include <array>
#include <chrono>
#include <ctime>

std::shared_ptr<SipRegister> SipRegister::createInstance()
{
    auto instance = std::make_shared<SipRegister>();
    // 使用weak_ptr存储，避免循环引用
    instance->sip_reg_ = instance;
    LOG(INFO) << "SipRegister instance created";
    return instance;
}

// 构造函数中获取IDomainManager引用
SipRegister::SipRegister()
    : reg_timer_(std::make_shared<TaskTimer>()),
      domain_manager_(GlobalCtl::getInstance()) // 注入依赖
{
    // 修复：确保初始化所有成员变量
    initialized_ = false;
}

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
        // 修复：确保只初始化一次
        std::lock_guard<std::mutex> lock(init_mutex_);
        LOG(INFO) << "Lock acquired for initialization.";
        if (!initialized_) 
        {
            reg_timer_->start(); // 使用统一的start接口名称
            initialized_ = true;
            LOG(INFO) << "Register timer initialized.";
        }
    }
}

pj_status_t SipRegister::runRxTask(SipTypes::RxDataPtr rdata) 
{
    LOG(INFO) << "runRxTask called";
    
    // 修复：确保实例已初始化（即使从其他线程调用）
    std::lock_guard<std::mutex> lock(init_mutex_);
    LOG(INFO) << "Lock acquired for runRxTask.";
    if (!initialized_) 
    {
        initialized_ = true;
    }
    
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

    // 添加锁保护共享资源访问
    std::lock_guard<std::mutex> lock(register_mutex_);

    // 修复：确保线程已注册到PJSIP
    PjSipUtils::ThreadRegistrar thread_registrar;
    
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
    
    // 修复：添加更严格的指针检查
    if(from_raw->uri && from_raw->vptr && from_raw->vptr->print_on) 
    {
        size = from_raw->vptr->print_on(from_raw, buf.data(), static_cast<int>(buf.size()));
    }
    else
    {
        LOG(ERROR) << "Invalid FROM header structure";
        return -1;
    }
    
    if(size > 0)
    {
        std::string temp(buf.data(), size);
        // 修复：更安全的字符串处理
        if(temp.size() > 11)
        {
            // 避免可能的越界
            size_t len = std::min<size_t>(temp.size() - 11, 20); 
            from_id = temp.substr(11, len);
            LOG(INFO) << "from_id: " << from_id;
        }
        else
        {
            LOG(WARNING) << "FROM header too short: " << temp;
            return -1;
        }
    }
    else
    {
        LOG(ERROR) << "Failed to print FROM header";
        return -1;
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
            
        // 修正：增加空指针检查
        pj_int32_t expires_value = 3600; // 默认值
        if (expires_raw) {
            expires_value = expires_raw->ivalue;
        }
        
        domain_manager_.setExpires(from_id, expires_value);
    }

    // 修复：获取endpoint的安全处理
    auto endpt = GlobalCtl::getInstance().getSipCore().getEndPoint();
    if (!endpt) {
        LOG(ERROR) << "Failed to get endpoint";
        return PJ_EINVAL;
    }
    
    // 创建响应
    pjsip_tx_data* txdata { nullptr };
    auto status = pjsip_endpt_create_response(
        endpt.get(),
        rdata.get(),
        status_code, 
        nullptr, 
        &txdata);
        
    if(status != PJ_SUCCESS || !txdata)
    {
        LOG(ERROR) << "pjsip_endpt_create_response failed, code: " << status;
        return status;
    }

    // 使用智能指针管理txdata
    auto txdata_guard = SipTypes::makeTxData(txdata);

    // 使用UTC时间而不是本地时间
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // 修复：使用thread-safe的方式处理时间
    std::array<char, 100> buffer {};
    struct tm tm_buf;
    gmtime_r(&time_t_now, &tm_buf); // 使用线程安全版本
    std::strftime(buffer.data(), buffer.size(), "%Y-%m-%d %H:%M:%S", &tm_buf);
    std::string time_str(buffer.data());
    LOG(INFO) << "Current UTC time: " << time_str;

    // 使用栈上缓冲区避免内存泄漏
    pj_str_t value_time = pj_str(const_cast<char*>(time_str.c_str()));
    pj_str_t key_time = pj_str(const_cast<char*>("Date"));
    auto date_header = pjsip_date_hdr_create(rdata->tp_info.pool, &key_time, &value_time);
    
    if (date_header) 
    {
        pjsip_msg_add_hdr(txdata->msg, (pjsip_hdr*)date_header);
    }

    pjsip_response_addr res_addr;
    status = pjsip_get_response_addr(txdata->pool, rdata.get(), &res_addr);
    if(status != PJ_SUCCESS)
    {
        LOG(ERROR) << "pjsip_get_response_addr failed, code: " << status;
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
        LOG(ERROR) << "pjsip_endpt_send_response failed, code: " << status;
        return status;
    }

    // txdata_guard会自动释放资源
    return PJ_SUCCESS;
}