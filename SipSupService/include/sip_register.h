#pragma once

#include "common.h"
#include "task_timer.h"
#include "ev_thread.h"
#include "sip_task_base.h"
#include "interfaces/isip_register.h"
#include "interfaces/idomain_manager.h"

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <atomic>

class SipRegister : public SipRegTaskBase, 
                    public ISipRegister,
                    public std::enable_shared_from_this<SipRegister>
{
public:
    // 工厂方法创建实例
    static std::shared_ptr<SipRegister> create(IDomainManager& domain_manager);
    
    // 构造函数不再private
    explicit SipRegister(IDomainManager& domain_manager);
    ~SipRegister() override;

    // 禁用拷贝和移动
    SipRegister(const SipRegister&) = delete;
    SipRegister& operator=(const SipRegister&) = delete;
    SipRegister(SipRegister&&) = delete;
    SipRegister& operator=(SipRegister&&) = delete;

public:
    void startRegService() override;
    pj_status_t runRxTask(PjSipUtils::RxDataPtr rdata) override;
    pj_status_t registerReqMsg(PjSipUtils::RxDataPtr rdata) override;

private:
    pj_status_t handleRegister(PjSipUtils::RxDataPtr rdata);
    std::string parseFromHeader(pjsip_msg* msg);
    bool addDateHeader(pjsip_msg* msg, pj_pool_t* pool);
    
    // 新增：原子操作更新注册状态
    void updateRegistrationStatus(const std::string& from_id, 
                                pj_int32_t expires_value,
                                bool is_registered,
                                time_t reg_time);

private:
    std::shared_ptr<TaskTimer> reg_timer_;
    mutable std::shared_mutex register_mutex_; // 改用读写锁
    std::atomic<bool> is_running_{false};      // 原子标志位
    
    IDomainManager& domain_manager_;           // 依赖注入的域管理器
};