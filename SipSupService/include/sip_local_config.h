// sip_local_config.h

#pragma once

#include "common.h"
#include "conf_reader.h"
#include "node_info.h"
#include "interfaces/iconfig_provider.h"

#include <string>
#include <vector>
#include <mutex>

// SipLocalConfig实现IConfigProvider接口
class SipLocalConfig : public IConfigProvider
{
public:
    SipLocalConfig();
    ~SipLocalConfig() = default;

    // 实现IConfigProvider接口
    bool readConf() override;
    const std::string& getLocalIp() const override { return local_ip_; }
    const std::string& getSipId() const override { return sip_id_; }
    const std::string& getSipIp() const override { return sip_ip_; }
    int getSipPort() const override { return sip_port_; }
    const std::string& getSipRealm() const override { return sip_realm_; }
    const std::string& getSipUsr() const override { return sip_usr_; }
    const std::string& getSipPwd() const override { return sip_pwd_; }
    const std::vector<NodeInfo>& getNodeInfoList() const override { return node_info_list_; }
    
    // 非const版本用于内部修改
    std::vector<NodeInfo>& getNodeInfoList() { return node_info_list_; }
    
private:
    ConfReader conf_reader_;

    std::string local_ip_;
    std::string sip_id_;
    std::string sip_ip_;
    int sip_port_{ 0 }; 
    std::string sip_realm_;
    std::string sip_usr_;
    std::string sip_pwd_;
    int subnode_num_{ 0 };

    std::mutex node_mutex_;

    std::vector<NodeInfo> node_info_list_;
};