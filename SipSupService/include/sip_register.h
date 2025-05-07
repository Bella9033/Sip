#pragma once

#include "common.h"
#include "task_timer.h"
#include "ev_thread.h"
#include "task_timer.h"

#include "interfaces/isip_task_base.h" 
#include "interfaces/isip_register.h"
#include "interfaces/idomain_manager.h"

#include <memory>
#include <mutex>
#include <atomic>

class SipCore;

// 修改继承关系
class SipRegister : public ISipTaskBase, 
                   public ISipRegister,
                   public std::enable_shared_from_this<SipRegister>
{
public:
    explicit SipRegister(IDomainManager& domain_manager);
    ~SipRegister() override;

    // 单例工厂
    static std::shared_ptr<SipRegister> getInstance(IDomainManager& domain_manager);

    // ISipRegister 接口实现
    void startRegService() override;
    
    // ISipTaskBase 接口实现
    // 实现使用智能指针的接口
    pj_status_t runRxTask(SipTypes::RxDataPtr rdata) override;
    pj_status_t registerReqMsg(SipTypes::RxDataPtr rdata) override;
    
protected:
    // ISipTaskBase 接口实现
    std::string parseFromHeader(pjsip_msg* msg) override;

private:   
    // 私有成员函数
    pj_status_t handleRegister(SipTypes::RxDataPtr rdata);
    pj_status_t handleAuthRegister(SipTypes::RxDataPtr rdata);
    void checkRegisterProc();

    static std::string formatSIPDate(const std::tm& tm_utc);
    static std::tm getCurrentUTC();
    bool addDateHeader(pjsip_msg* msg, pj_pool_t* pool);

    void updateRegistrationStatus(const std::string& from_id, pj_int32_t expires_value);

private:   
    std::shared_ptr<TaskTimer> reg_timer_;

    std::mutex register_mutex_;
    // 用于保护认证过程的互斥锁
    std::mutex auth_mutex_; 
    
    IDomainManager& domain_manager_; 

    static std::shared_ptr<SipRegister> instance_;
    static std::mutex instance_mutex_;
};