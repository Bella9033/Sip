// sip_register.h
#pragma once

#include "common.h"
#include "task_timer.h"
#include "sip_task_base.h"
#include "interfaces/isip_register.h"
#include "interfaces/idomain_manager.h"

#include <memory>
#include <mutex>

// 前向声明
class SipCore;

class SipRegister : public SipRegTaskBase, 
                    public ISipRegister,
                    public std::enable_shared_from_this<SipRegister>
{
public:
    static std::shared_ptr<SipRegister> createInstance();
    
    SipRegister();
    virtual ~SipRegister();

    // 实现基类的纯虚函数
    pj_status_t runRxTask(SipTypes::RxDataPtr rdata) override;
    pj_status_t registerReqMsg(SipTypes::RxDataPtr rdata) override;
    
    // 从ISipRegister接口实现
    void startRegService() override;
    
private:
    pj_status_t handleReg(SipTypes::RxDataPtr rdata);

    // 使用weak_ptr避免循环引用
    std::weak_ptr<SipRegister> sip_reg_;
    std::shared_ptr<TaskTimer> reg_timer_;
    std::mutex register_mutex_;
    
    // 依赖注入：域名管理器的引用
    IDomainManager& domain_manager_;
};