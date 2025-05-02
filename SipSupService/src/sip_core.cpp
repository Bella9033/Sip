#include "sip_core.h"
#include "sip_register.h"
#include <thread>

std::atomic<bool> SipCore::stop_pool_{false};

pjsip_module SipCore::recv_mod = {
    NULL, NULL,                    // prev, next
    { "mod-recv", 8 },            // name
    -1,                           // id
    PJSIP_MOD_PRIORITY_TSX_LAYER, // priority
    NULL,                         // load
    NULL,                         // start
    NULL,                         // stop
    NULL,                         // unload
    &SipCore::onRxRequestRaw,     // on_rx_request
    NULL,                         // on_rx_response
    NULL,                         // on_tx_request
    NULL,                         // on_tx_response
    NULL,                         // on_tsx_state
};

std::shared_ptr<SipCore> SipCore::create() {
    return std::make_shared<SipCore>();
}

SipCore::SipCore() 
    : caching_pool_(PjSipUtils::createCachingPool(SIP_STACK_SIZE))
    , endpt_()
    , pool_() 
{
    if (!caching_pool_) {
        throw std::runtime_error("Failed to create caching pool");
    }
}

SipCore::~SipCore() {
    LOG(INFO) << "Destroying SipCore";
    stop_pool_ = true;
    PjSipUtils::cleanupCore(caching_pool_, endpt_);
}

pj_status_t SipCore::initSip(int sip_port) {
    LOG(INFO) << "Initializing SIP core on port " << sip_port;

    if (!caching_pool_) {
        LOG(ERROR) << "Caching pool not initialized";
        return PJ_EINVAL;
    }

    auto status = PjSipUtils::initCore(caching_pool_, endpt_);
    if (status != PJ_SUCCESS) {
        LOG(ERROR) << "Failed to initialize core: " << status;
        return status;
    }

    pool_ = PjSipUtils::createEndptPool(endpt_, "sipcore-pool", 
                                       SIP_ALLOC_POOL_1M, SIP_ALLOC_POOL_1M);
    if (!pool_) {
        LOG(ERROR) << "Failed to create endpoint pool";
        return PJ_ENOMEM;
    }

    // 注册模块
    status = pjsip_endpt_register_module(endpt_.get(), &recv_mod);
    if (status != PJ_SUCCESS) {
        LOG(ERROR) << "Failed to register module: " << status;
        return status;
    }

    // 启动事件轮询线程
    std::thread([this]() {
        pollingEventLoop(endpt_);
    }).detach();

    LOG(INFO) << "SIP core initialized successfully";
    return PJ_SUCCESS;
}

void SipCore::pollingEventLoop(PjSipUtils::EndpointPtr endpt) {
    const pj_time_val delay = {0, 10};
    while (!stop_pool_) {
        pjsip_endpt_handle_events(endpt.get(), &delay);
    }
}

pj_bool_t SipCore::onRxRequestRaw(pjsip_rx_data* rdata) {
    if (!rdata) return PJ_TRUE;
    return onRxRequest(PjSipUtils::RxDataPtr(rdata));
}

pj_bool_t SipCore::onRxRequest(PjSipUtils::RxDataPtr rdata) {
    if (!rdata || !rdata->msg_info.msg) {
        LOG(ERROR) << "Invalid request data";
        return PJ_TRUE;
    }

    auto params = std::make_shared<ThRxParams>();
    if (!params) {
        LOG(ERROR) << "Failed to create thread parameters";
        return PJ_TRUE;
    }

    params->rxdata = rdata;

    auto worker = [params = std::move(params)]() -> int {
        try {
            if (!params || !params->taskbase) {
                LOG(ERROR) << "Invalid task parameters";
                return -1;
            }

            LOG(INFO) << "Processing request in worker thread";
            try {
                params->taskbase->runRxTask(params->rxdata);
            } catch (const std::exception& e) {
                LOG(ERROR) << "Exception in runRxTask: " << e.what();
                return -1;
            }
            return 0;
        } catch (const std::exception& e) {
            LOG(ERROR) << "Worker thread exception: " << e.what();
            return -1;
        }
    };

    // 获取全局线程池并提交任务
    try {
        GlobalCtl::getInstance().getThreadPool().submit(std::move(worker));
    } catch (const std::exception& e) {
        LOG(ERROR) << "Failed to submit task: " << e.what();
    }

    return PJ_FALSE;
}