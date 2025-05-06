// sip_local_config.cpp

#include "sip_local_config.h"


SipLocalConfig::SipLocalConfig() 
    : conf_reader_(SUP_CONF_FILE)
    { }

bool SipLocalConfig::readConf() 
{
    std::string err;

    // 读取并校验本地服务配置
    auto local_ip_opt   = conf_reader_.getString("local_server", "local_ip",   &err);
    auto local_port_opt = conf_reader_.getInt   ("local_server", "local_port", &err);
    if (!local_ip_opt || !local_port_opt) 
    {
        LOG(ERROR) << "Failed to load local_server config: " << err;
        return false;
    }
    
    // 保存本地服务器配置
    local_ip_ = *local_ip_opt;

    // 读取并校验 SIP 服务配置
    auto sip_id_opt = conf_reader_.getString("sip_server", "sip_id", &err);
    auto sip_ip_opt = conf_reader_.getString("sip_server", "sip_ip", &err);
    auto sip_port_opt = conf_reader_.getInt("sip_server", "sip_port", &err);
    auto sip_realm_opt = conf_reader_.getString("sip_server", "sip_realm", &err);
    auto sip_usr_opt = conf_reader_.getString("sip_server", "sip_usr", &err);
    auto sip_pwd_opt = conf_reader_.getString("sip_server", "sip_pwd", &err);
    
    auto subnode_num_opt = conf_reader_.getInt("sip_server", "subnode_num", &err);
    if(!sip_id_opt || !sip_ip_opt || !sip_port_opt || !subnode_num_opt || 
       !sip_realm_opt || !sip_usr_opt || !sip_pwd_opt) 
    {
        LOG(ERROR) << "Failed to load sip_server config: " << err;
        return false;
    }

    // 将 SIP 服务配置存储到成员变量中
    sip_id_ = *sip_id_opt;
    sip_ip_ = *sip_ip_opt; 
    sip_port_ = *sip_port_opt;
    sip_realm_ = *sip_realm_opt;
    sip_usr_ = *sip_usr_opt;
    sip_pwd_ = *sip_pwd_opt;
    subnode_num_ = *subnode_num_opt;
    
    LOG(INFO) << fmt::format(
        "SIP Server Config: ID={}, IP={}, Port={}, Realm={}, SubnodeNum={}",
        sip_id_, sip_ip_, sip_port_, sip_realm_, subnode_num_
    );

    int num = *subnode_num_opt;
    if (num <= 0) 
    {
        LOG(ERROR) << "subnode_num must be greater than 0";
        return false;
    }

    // 创建临时列表，减少锁的持有时间
    std::vector<NodeInfo> tmp_list;
    tmp_list.reserve(num);

    for(int i = 1; i <= num; ++i)
    {
        std::string err;
        auto id_opt = conf_reader_.getString("sip_server", "subnode_id" + std::to_string(i), &err);
        auto ip_opt = conf_reader_.getString("sip_server", "subnode_ip" + std::to_string(i), &err);
        auto port_opt = conf_reader_.getInt("sip_server", "subnode_port" + std::to_string(i), &err);
        auto proto_opt = conf_reader_.getInt("sip_server", "subnode_proto" + std::to_string(i), &err);
        auto auth_opt = conf_reader_.getString("sip_server", "subnode_auth" + std::to_string(i), &err);
        
        if(!id_opt || !ip_opt || !port_opt || !proto_opt || !auth_opt) 
        {
            LOG(ERROR) << fmt::format("subnode[{}] config error: {}", i, err);
            return false;
        }

        // 使用一一赋值的方式创建 NodeInfo
        NodeInfo node;
        node.id = std::move(*id_opt);
        node.ip = std::move(*ip_opt);
        node.port = *port_opt;
        node.proto = *proto_opt;
        // 将字符串转换为布尔值
        node.auth = (*auth_opt == "true" || *auth_opt == "1");


        LOG(INFO) << fmt::format(
            "Created NodeInfo: ID={}, IP={}, Port={}, Proto={}, Auth={}",
            node.id, node.ip, node.port, node.proto, node.auth
        );

        tmp_list.push_back(std::move(node));
    }

    // 所有节点读取成功后，才更新实际列表
    {
        std::lock_guard<std::mutex> lock(node_mutex_);
        LOG(INFO) << "Starting to update node_info_list_...";
        node_info_list_ = std::move(tmp_list);
        LOG(INFO) << "node_info_list_ size: " << node_info_list_.size();\
        LOG(INFO) << fmt::format("sip_id: {}, sip_ip: {}, sip_port: {}",
            *sip_id_opt, *sip_ip_opt, *sip_port_opt);
    }
    
    return true;
}

