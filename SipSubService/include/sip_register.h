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
    // 使用真正的单例模式，而不是每次调用createInstance创建新实例
    static std::shared_ptr<SipRegister> getInstance();
    
public:
    virtual ~SipRegister();
    
    // 从ISipRegister接口实现
    void startRegService() override;

    void registerProc(); 
    pj_status_t gbRegister(DomainInfo& domains); 
private:
    // 私有化构造函数，防止外部创建实例
    SipRegister();

    static std::shared_ptr<SipRegister> instance_;
    static std::mutex singleton_mutex_;
    
    // 使用weak_ptr避免循环引用
    std::weak_ptr<SipRegister> sip_reg_;
    std::shared_ptr<TaskTimer> reg_timer_;
    std::mutex register_mutex_;
    
    // 添加初始化状态标志和互斥锁
    std::mutex init_mutex_;
    bool initialized_{false}; // 确保正确初始化
};