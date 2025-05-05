#pragma once

#include "common.h"
#include <string>
#include <string_view>

// 节点信息
struct NodeInfo 
{
    std::string id;
    std::string ip;
    int port { 0 };
    int proto { 0 };
    bool auth { false };
    std::string realm;

    NodeInfo(std::string id_, std::string ip_, 
        int port_, int proto_, bool auth_,
        std::string realm_ )
        : id(std::move(id_))
        , ip(std::move(ip_))
        , port(port_)
        , proto(proto_)
        , auth(auth_) 
        , realm(std::move(realm_))
    { }

    NodeInfo() = default;
};

// 域信息
struct DomainInfo 
{
    std::string sip_id;
    std::string addr_ip;
    int sip_port { 0 };
    int proto { 0 };
    bool auth { false };
    int expires { 0 };
    bool registered { false };
    time_t last_reg_time { 0 };

    explicit DomainInfo(const NodeInfo& node)
        : sip_id(node.id)
        , addr_ip(node.ip)
        , sip_port(node.port)
        , proto(node.proto)
        , auth(node.auth)
        , expires(60)
        , registered(false)
        , last_reg_time(0) 
    { }

    DomainInfo() = default;
};