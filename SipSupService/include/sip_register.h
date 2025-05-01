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
    // 接受必要的依赖
    explicit SipRegister(IDomainManager& domain_manager);
    ~SipRegister() override;

public:
    void startRegService() override;

public:   
    // SipRegTaskBase 接口实现
    pj_status_t runRxTask(SipTypes::RxDataPtr rdata) override;
    pj_status_t registerReqMsg(SipTypes::RxDataPtr rdata) override;
    
    
private:   
    // 注册相关方法
    pj_status_t handleRegister(SipTypes::RxDataPtr rdata);
    std::string parseFromHeader(pjsip_msg* msg);
    bool addDateHeader(pjsip_msg* msg, pj_pool_t* pool);
    void updateRegistrationStatus(const std::string& from_id, pj_int32_t expires_value);

private:   
    std::shared_ptr<TaskTimer> reg_timer_;
    std::mutex register_mutex_;
    
    // 依赖注入
    IDomainManager& domain_manager_; 
};