// sip_common.h
// 包含PJSIP 相关的定义、常量、枚举和基本类型定义

#pragma once

#include <string>

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


// SIP协议栈缓存池初始化大小
constexpr size_t SIP_STACK_SIZE = 1024 * 256;
// SIP分配池大小
constexpr size_t SIP_ALLOC_POOL_1M = 1024 * 1024 * 1;


enum class StatusCode
{
    SIP_SUCCESS = 200,
    SIP_FORBIDEN = 403, // 禁止访问
    SIP_NOT_FOUND = 404, // 找不到
};
