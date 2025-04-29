// RTP相关头文件
#include "rtpsession.h"
#include "rtpsourcedata.h"
#include "rtptcptransmitter.h"
#include "rtptcpaddress.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtplibraryversion.h"
#include "rtcpsrpacket.h"


#include <memory>

#include "interfaces/iconfig_provider.h"
#include "interfaces/idomain_manager.h"


#include "global_ctl.h"        // 依赖于 SipLocalConfig 的定义
#include "sip_local_config.h"  // 必须在使用 SipLocalConfig 之前包含
#include "sip_register.h"  // SipRegister 依赖于 SipLocalConfig
#include "common.h"


int main(int argc, char* argv[])
{

    srand(time(0));
    SetLogLevel glog(SetLogLevel::LogLevel::INFO);

    LOG(INFO) << "========= Test SipCore =========";

    // 创建一个SipLocalConfig实例
    auto config = std::make_unique<SipLocalConfig>();
    
    // 初始化全局控制器
    if (!GlobalCtl::getInstance().init(std::move(config))) 
    {
        LOG(ERROR) << "Failed to initialize GlobalCtl";
        return -1;
    }

    LOG(INFO) << "local_ip is: " << GlobalCtl::getInstance().getConfig().getLocalIp();

    auto regc = SipRegister::createInstance();
    regc->startRegService();


    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
    return 0;
}