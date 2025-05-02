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

#include "common.h"
#include "global_ctl.h"
#include "sip_local_config.h"
#include "sip_register.h"
#include <chrono>
#include <thread>

#include "common.h"
#include "global_ctl.h"
#include "sip_local_config.h"
#include "sip_register.h"
#include <chrono>
#include <thread>

int main(int argc, char* argv[]) {
    srand(time(0));
    SetLogLevel glog(SetLogLevel::LogLevel::INFO);

    // 初始化全局控制器
    auto config = std::make_unique<SipLocalConfig>();
    if (!GlobalCtl::getInstance().init(std::move(config))) {
        LOG(ERROR) << "Failed to initialize GlobalCtl";
        return -1;
    }

    LOG(INFO) << "local_ip is: " << GCONF(getLocalIp);

    // 使用工厂方法创建SipRegister实例
    auto reg = SipRegister::create(GlobalCtl::getInstance());
    if (!reg) {
        LOG(ERROR) << "Failed to create SipRegister instance";
        return -1;
    }

    reg->startRegService();

    // 主循环
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
    
    return 0;
}