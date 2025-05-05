// sip_register.h
#pragma once

#include "common.h"
#include "task_timer.h"
#include "ev_thread.h"
#include "sip_task_base.h"
#include  "task_timer.h"

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
    explicit SipRegister(IDomainManager& domain_manager);
    ~SipRegister() override;

    // 单例工厂
    static std::shared_ptr<SipRegister> getInstance(IDomainManager& domain_manager);

public:   
    void startRegService() override;
    pj_status_t runRxTask(SipTypes::RxDataPtr rdata) override;
    pj_status_t registerReqMsg(SipTypes::RxDataPtr rdata) override;
    
    std::string parseFromHeader(pjsip_msg* msg) override;
private:   
    // 处理注册请求
    pj_status_t handleRegister(SipTypes::RxDataPtr rdata);
    // 添加处理需要认证的注册请求的函数声明
    pj_status_t handleAuthRegister(SipTypes::RxDataPtr rdata); 
      
    void checkRegisterProc();


    static std::string formatSIPDate(const std::tm& tm_utc);
    static std::tm getCurrentUTC();
    bool addDateHeader(pjsip_msg* msg, pj_pool_t* pool);

    void updateRegistrationStatus(const std::string& from_id, pj_int32_t expires_value);

private:   
    std::shared_ptr<TaskTimer> reg_timer_;

    std::mutex register_mutex_;

    IDomainManager& domain_manager_; 

    static std::shared_ptr<SipRegister> instance_;
    static std::mutex instance_mutex_;
};