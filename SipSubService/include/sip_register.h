// sip_register.h
#pragma once

#include "common.h"
#include "task_timer.h"
#include "ev_thread.h"

#include "interfaces/isip_register.h"
#include "interfaces/idomain_manager.h"

#include "sip_msg.h" // SIP消息处理

#include <memory>
#include <mutex>

// 前向声明
class SipCore;

class SipRegister : public ISipRegister,
                    public std::enable_shared_from_this<SipRegister>
{
public:
    // 接受必要的依赖
    explicit SipRegister(IDomainManager& domain_manager);
    ~SipRegister() override;

public:
    void startRegService() override;

private:
    // 注册处理函数
    void registerProc(); 
    pj_status_t gbRegister(DomainInfo& domains); 

private:

    std::shared_ptr<TaskTimer> reg_timer_;
    std::mutex register_mutex_;
    
    
    // 依赖注入
    IDomainManager& domain_manager_;      
};