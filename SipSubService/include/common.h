#pragma once

#include "log_level.h"

#include <fmt/format.h>
#include <string>
#include "tinyxml2.h"  
#include <json/json.h> 

#include <stdlib.h>
#include <stdio.h> 
#include <iostream>  

#include <event2/event.h>                        
#include <event2/listener.h>                     
#include <event2/bufferevent.h>                  
#include <event2/buffer.h>                       
#include <event2/thread.h>    


#include <exception>

// PJSIP相关头文件
#include <pjlib-util.h> 
#include <pjmedia.h> 
#include <pjsip.h> 
#include <pjsip_ua.h>
#include <pjsip/sip_auth.h>
#include <pjsip/sip_module.h>
#include <pjsip/sip_endpoint.h>
#include <pjlib.h>
#include <pjlib-util.h>

#define SUB_CONF_FILE "/mnt/hgfs/share/conf/sip_sub_service.conf"
#define SUP_CONF_FILE "/mnt/hgfs/share/conf/sip_sup_service.conf"

#define GCONF(x) GlobalCtl::getInstance().getConfig().x()



// SIP协议栈缓存池初始化大小
constexpr size_t SIP_STACK_SIZE = 1024 * 256;
// SIP分配池大小
constexpr size_t SIP_ALLOC_POOL_1M = 1024 * 1024 * 1;


enum class SipStatusCode
{
    SIP_OK = 200,
    SIP_FORBIDEN = 403, // 禁止访问
    SIP_NOT_FOUND = 404, // 找不到
};

