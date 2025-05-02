#pragma once

#include "common.h"
#include <memory>

// 前向声明
namespace SipTypes {
    using EndpointPtr = std::shared_ptr<pjsip_endpoint>;
}

class ISipCore {
public:
    virtual ~ISipCore() = default;
    virtual pj_status_t initSip(int sip_port) = 0;
    virtual SipTypes::EndpointPtr getEndPoint() const = 0;

protected:
    ISipCore() = default;
};