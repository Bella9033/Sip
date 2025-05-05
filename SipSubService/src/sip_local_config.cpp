// sip_local_config.cpp

#include "sip_local_config.h"


SipLocalConfig::SipLocalConfig() 
    : conf_reader_(SUB_CONF_FILE)
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
    
    auto supnode_num_opt = conf_reader_.getInt("sip_server", "supnode_num", &err);
    if(!sip_id_opt || !sip_ip_opt || !sip_port_opt || !supnode_num_opt) 
    {
        LOG(ERROR) << "Failed to load sip_server config: " << err;
        return false;
    }
    
    // 将 SIP 服务配置存储到成员变量中
    sip_id_ = *sip_id_opt;
    sip_ip_ = *sip_ip_opt; 
    sip_port_ = *sip_port_opt;
    sip_realm_ = *sip_realm_opt;
    supnode_num_ = *supnode_num_opt;

    int num = *supnode_num_opt;
    if (num <= 0) 
    {
        LOG(ERROR) << "supnode_num must be greater than 0";
        return false;
    }

    // 创建临时列表，减少锁的持有时间
    std::vector<NodeInfo> tmp_list;
    tmp_list.reserve(num);

    for(int i = 1; i <= num; ++i)
    {
        std::string err;
        auto id_opt = conf_reader_.getString("sip_server", "supnode_id" + std::to_string(i), &err);
        auto ip_opt = conf_reader_.getString("sip_server", "supnode_ip" + std::to_string(i), &err);
        auto port_opt = conf_reader_.getInt("sip_server", "supnode_port" + std::to_string(i), &err);
        auto proto_opt = conf_reader_.getInt("sip_server", "supnode_proto" + std::to_string(i), &err);
        auto expires_opt = conf_reader_.getInt("sip_server", "supnode_expires" + std::to_string(i), &err);
        auto auth_opt = conf_reader_.getString("sip_server", "supnode_auth" + std::to_string(i), &err);
        auto usr_opt = conf_reader_.getString("sip_server", "supnode_usr" + std::to_string(i), &err);
        auto pwd_opt = conf_reader_.getString("sip_server", "supnode_pwd" + std::to_string(i), &err);
        auto realm_opt = conf_reader_.getString("sip_server", "supnode_realm" + std::to_string(i), &err);

        if(!id_opt || !ip_opt || !port_opt || !proto_opt 
            || !auth_opt || !usr_opt || !pwd_opt) 
        {
            LOG(ERROR) << fmt::format("supnode[{}] config error: {}", i, err);
            return false;
        }


        // 使用一一赋值的方式创建 NodeInfo
        NodeInfo node;
        node.id = std::move(*id_opt);
        node.ip = std::move(*ip_opt);
        node.port = *port_opt;
        node.proto = *proto_opt;
        node.expires = *expires_opt; // 默认过期时间为60秒
        node.auth = (*auth_opt == "true") ? true : false;
        node.usr = *usr_opt;
        node.pwd = *pwd_opt;
        node.realm = *realm_opt;


        LOG(INFO) << fmt::format(
            "Created NodeInfo: ID={}, IP={}, Port={}, Proto={}, "
            "Expires={}, Auth={}, Username={}, Password={}",
            node.id, node.ip, node.port, node.proto, 
            node.expires, node.auth, node.usr, node.pwd
        );

        tmp_list.push_back(std::move(node));
    }

    // 所有节点读取成功后，才更新实际列表
    {
        std::lock_guard<std::mutex> lock(node_mutex_);
        LOG(INFO) << "Starting to update node_info_list_...";
        node_info_list_ = std::move(tmp_list);
        LOG(INFO) << "node_info_list_ size: " << node_info_list_.size();
        LOG(INFO) << fmt::format("sip_id: {}, sip_ip: {}, sip_port: {}",
            *sip_id_opt, *sip_ip_opt, *sip_port_opt);
    }
    
    return true;
}

