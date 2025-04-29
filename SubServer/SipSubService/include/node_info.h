// node_info.h

#pragma once

#include <string>

struct NodeInfo 
{
    std::string id;
    std::string ip;
    int port { 0 };
    int proto { 0 };
    int auth { 0 };
    int expires { 3600 }; // 默认过期时间为3600秒

    // 构造函数，从配置参数构造 NodeInfo
    NodeInfo(std::string id_, 
        std::string ip_, 
        int port_,
        int proto_,
        int auth_,
        int expires_)
    : id(std::move(id_))
    , ip(std::move(ip_))
    , port(port_)
    , proto(proto_)
    , auth(auth_)
    , expires(expires_)
    { }

    // 默认构造函数
    NodeInfo() = default;
};

struct DomainInfo 
{
    std::string sip_id;
    std::string addr_ip;
    int sip_port;
    int proto { 0 };
    int expires { 3600 };
    bool registered { false };

    // 从 NodeInfo 构造 DomainInfo
    explicit DomainInfo(const NodeInfo& node)
        : sip_id(node.id)
        , addr_ip(node.ip)
        , sip_port(node.port)
        , proto(node.proto)
        , expires(node.expires)
        , registered(false)  // 初始化为未注册状态
    { }

    // 默认构造函数
    DomainInfo() = default;

    void setRegistered() { registered = true; }
};
