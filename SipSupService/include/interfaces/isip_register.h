#pragma once

#include "common.h"
#include "idomain_manager.h"
#include "pjsip_utils.h"

class ISipRegister {
public:
    virtual ~ISipRegister() = default;
    static std::shared_ptr<ISipRegister> create(IDomainManager& domain_manager);
    virtual void startRegService() = 0;
    virtual pj_status_t runRxTask(SipTypes::RxDataPtr rdata) = 0;
    virtual pj_status_t registerReqMsg(SipTypes::RxDataPtr rdata) = 0;

protected:
    ISipRegister() = default;
};