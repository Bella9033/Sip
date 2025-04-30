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

    // 创建一个SipLocalConfig实例
    auto config = std::make_unique<SipLocalConfig>();
    
    // 初始化全局控制器
    if (!GlobalCtl::getInstance().init(std::move(config))) 
    {
        LOG(ERROR) << "Failed to initialize GlobalCtl";
        return -1;
    }

    LOG(INFO) << "local_ip is: " << GCONF(getLocalIp);

    auto regc = SipRegister::createInstance();
    regc->startRegService();

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
    return 0;

// 假设 createThread 已在 ThreadUtil 命名空间中定义
// using namespace ThreadUtil;


    // // 1. 准备要在线程中执行的函数（这里是 lambda）
    // auto func = [](int a, const std::string& msg) {
    //     std::cout << msg << ": " << (a * 2) << std::endl;
    //     return a * 2;
    // };

    // // 2. 将实参打包到 tuple 中
    // auto args = std::make_tuple(21, std::string("Result"));

    // // 3. 调用 createThread，获取 std::future<int>
    // auto fut = ThreadUtil::createThread(
    //     std::move(func),      // Func&& —— 完美转发
    //     std::move(args)       // tuple<Args...>
    //     // 其余参数使用默认值：thread_id_out=nullptr, priority=NORMAL, timeout=0
    // );

    // // 4. 等待线程执行并获取返回值
    // int result = fut.get();  // 阻塞直到线程完成
    // std::cout << "Returned: " << result << std::endl;
    // return 0;


}