// node_info.h

#pragma once

#include "common.h"
#include <string>
#include <string_view>

struct NodeInfo 
{
    std::string id;
    std::string ip;
    int port { 0 };
    int proto { 0 };
    bool auth { false }; 
    int expires { 60 }; 
    std::string usr;
    std::string pwd;
    std::string realm; 


    // 构造函数，从配置参数构造 NodeInfo
    NodeInfo(std::string id_, 
        std::string ip_, 
        int port_,
        int proto_,
        int expires_,
        bool auth_,
        std::string usr_,
        std::string pwd_,
        std::string realm_)
   : id(std::move(id_))
   , ip(std::move(ip_))
   , port(port_)
   , proto(proto_)
   , expires(expires_)
   , auth(auth_)
   , usr(std::move(usr_))
   , pwd(std::move(pwd_))
   , realm(std::move(realm_))
    { }

    // 默认构造函数
    NodeInfo() = default;
};

// 域信息
struct DomainInfo 
{
    std::string sip_id;
    std::string addr_ip;
    int sip_port { 0 };
    int proto { 0 };
    int expires { 60 };
    bool registered { false };
    bool isAuth { false }; // 是否需要认证
    std::string usr;
    std::string pwd;
    std::string realm; 

    // 从 NodeInfo 构造 DomainInfo
    explicit DomainInfo(const NodeInfo& node)
        : sip_id(node.id)
        , addr_ip(node.ip)
        , sip_port(node.port)
        , proto(node.proto)
        , expires(node.expires)
        , registered(false)
        , isAuth(node.auth)
        , usr(node.usr)
        , pwd(node.pwd)
        , realm(node.realm)
    { 
        // if (isAuth) 
        // {
        //     isAuth = true;
        //     usr = node.usr;
        //     pwd = node.pwd;
        //     realm = node.realm;
        // }
    }

    // 默认构造函数
    DomainInfo() = default;
};
