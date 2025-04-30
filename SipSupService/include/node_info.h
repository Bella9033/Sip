// node_info.h

#pragma once

#include <string>
#include <chrono>
#include <fmt/format.h>

struct NodeInfo 
{
    std::string id;
    std::string ip;
    int port { 0 };
    int proto { 0 };
    int auth { 0 };

    // 构造函数，从配置参数构造 NodeInfo
    NodeInfo(std::string id_, 
        std::string ip_, 
        int port_,
        int proto_,
        int auth_)
    : id(std::move(id_))
    , ip(std::move(ip_))
    , port(port_)
    , proto(proto_)
    , auth(auth_)
    { }

    // 默认构造函数
    NodeInfo() = default;
};

struct DomainInfo 
{
    std::string sip_id;
    std::string addr_ip;
    int sip_port { 0 };
    int proto { 0 };
    int auth { 0 };
    int expires { 0};
    bool registered { false };

    //std::string last_update_time;  // 添加最后更新时间

    // 从 NodeInfo 构造 DomainInfo
    explicit DomainInfo(const NodeInfo& node)
        : sip_id(node.id)
        , addr_ip(node.ip)
        , sip_port(node.port)
        , proto(node.proto)
        , auth(node.auth)
        , expires(0)
        , registered(false)
    { }

    // 默认构造函数
    DomainInfo() = default;


};
