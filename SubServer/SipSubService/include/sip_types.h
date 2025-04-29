// sip_types.h

#pragma once

#include <memory>
#include "common.h" 

// SIP数据类型的智能指针封装
namespace SipTypes {
    // 定义基本PJSIP类型的智能指针
    using RxDataPtr = std::shared_ptr<pjsip_rx_data>;
    using EndpointPtr = std::shared_ptr<pjsip_endpoint>;
    using CachingPoolPtr = std::shared_ptr<pj_caching_pool>;
    using PoolPtr = std::shared_ptr<pj_pool_t>;
    
    // RxData 相关工具函数
    inline RxDataPtr cloneRxData(pjsip_rx_data* raw_data) 
    {
        if (!raw_data) return nullptr;
        
        pjsip_rx_data* cloned_data { nullptr };
        pj_status_t status = pjsip_rx_data_clone(raw_data, 0, &cloned_data);
        if (status != PJ_SUCCESS || !cloned_data) 
        {
            return nullptr;
        }
        
        // 使用自定义删除器
        return RxDataPtr(cloned_data, [](pjsip_rx_data* p) {
            if (p) pjsip_rx_data_free_cloned(p);
        });
    }
    
    // 包装现有的 pjsip_rx_data 指针（不克隆）
    inline RxDataPtr wrapRxData(pjsip_rx_data* raw_data, bool should_free = false) 
    {
        if (!raw_data) return nullptr;
        
        if (should_free) 
        {
            return RxDataPtr(raw_data, [](pjsip_rx_data* p) {
                if (p) pjsip_rx_data_free_cloned(p);
            });
        } else {
            return RxDataPtr(raw_data, [](pjsip_rx_data*) {
                // 空删除器，不释放资源
            });
        }
    }
    
    // 用于回调的便捷方法
    inline RxDataPtr wrapRxDataForCallback(pjsip_rx_data* raw_data)
    {
        return wrapRxData(raw_data, false);
    }
    
    // 辅助函数：检查智能指针是否有效
    template<typename T>
    inline bool isValid(const std::shared_ptr<T>& ptr) {
        return ptr != nullptr;
    }
}