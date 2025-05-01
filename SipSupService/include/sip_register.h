// sip_register.h
#pragma once

#include "common.h"
#include "task_timer.h"
#include "ev_thread.h"
#include "sip_task_base.h"

#include "interfaces/isip_register.h"
#include "interfaces/idomain_manager.h"

#include <memory>
#include <mutex>
#include <atomic>

// 前向声明
class SipCore;

class SipRegister : public SipRegTaskBase, 
                    public ISipRegister,
                    public std::enable_shared_from_this<SipRegister>
{
public:
    // 单例获取方法
    static std::shared_ptr<SipRegister> getInstance();
    
    // 析构函数
    ~SipRegister() override;

public:      
    // ISipRegister 接口实现
    void startRegService() override;

public:   
    // SipRegTaskBase 接口实现
    pj_status_t runRxTask(SipTypes::RxDataPtr rdata) override;
    pj_status_t registerReqMsg(SipTypes::RxDataPtr rdata) override;
    
private:
    // 私有构造函数
    SipRegister();
    
    // 禁用复制
    SipRegister(const SipRegister&) = delete;
    SipRegister& operator=(const SipRegister&) = delete;
    
    // 处理注册请求
    pj_status_t handleRegister(SipTypes::RxDataPtr rdata);
    std::string parseFromHeader(pjsip_msg* msg);
    bool addDateHeader(pjsip_msg* msg, pj_pool_t* pool);
    void updateRegistrationStatus(const std::string& from_id, pj_int32_t expires_value);

    std::shared_ptr<TaskTimer> reg_timer_;
    std::mutex register_mutex_;
    
    // 依赖注入
    IDomainManager& domain_manager_;
};