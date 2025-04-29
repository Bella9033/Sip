// sip_register.h
#pragma once


#include "common.h"
#include "task_timer.h"

#include "sip_msg.h" // 独有

#include "interfaces/isip_register.h"
#include "interfaces/idomain_manager.h"

#include <memory>
#include <mutex>

// 前向声明
class SipCore;

class SipRegister : public ISipRegister,
                    public std::enable_shared_from_this<SipRegister>
{
public:
    static std::shared_ptr<SipRegister> createInstance();
    
    SipRegister();
    virtual ~SipRegister();
    
    // 从ISipRegister接口实现
    void startRegService() override;

private:
    void registerProc();
    pj_status_t gbRegister(DomainInfo& domains);

    // 使用weak_ptr避免循环引用
    std::weak_ptr<SipRegister> sip_reg_;
    std::shared_ptr<TaskTimer> reg_timer_;
    std::mutex register_mutex_;
    
};