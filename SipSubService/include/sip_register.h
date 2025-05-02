// sip_register.h
#pragma once

#include "common.h"
#include "task_timer.h"
#include "ev_thread.h"
#include "interfaces/isip_register.h"
#include "interfaces/idomain_manager.h"
#include "sip_msg.h"

#include <memory>
#include <mutex>
#include <atomic>

class SipRegister : public ISipRegister,
                    public std::enable_shared_from_this<SipRegister>
{
public:
    // 单例工厂
    static std::shared_ptr<SipRegister> getInstance(IDomainManager& domain_manager);

    // 禁止拷贝和赋值
    SipRegister(const SipRegister&) = delete;
    SipRegister& operator=(const SipRegister&) = delete;
    ~SipRegister() override;

    void startRegService() override;

private:
    explicit SipRegister(IDomainManager& domain_manager);

    void registerProc();
    pj_status_t gbRegister(DomainInfo& domains);

    std::shared_ptr<TaskTimer> reg_timer_;
    std::mutex register_mutex_;
    IDomainManager& domain_manager_;

    // 单例相关
    static std::shared_ptr<SipRegister> instance_;
    static std::mutex instance_mutex_;
};