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
    // 单例获取方法
    static std::shared_ptr<SipRegister> getInstance();
    
    // 析构函数
    ~SipRegister() override;
   
public:
    // 从ISipRegister接口实现
    void startRegService() override;

public:
    void registerProc(); 
    pj_status_t gbRegister(DomainInfo& domains); 
private:
    // 启动注册服务
    SipRegister();
    // 注册处理回调函数
    void handleRegistration();
    
    // 禁用复制
    SipRegister(const SipRegister&) = delete;
    SipRegister& operator=(const SipRegister&) = delete;
    

    std::shared_ptr<TaskTimer> reg_timer_;
    std::mutex register_mutex_;
    
     
};