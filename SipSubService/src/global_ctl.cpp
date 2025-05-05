// global_ctl.cpp

#include "global_ctl.h"
#include "sip_core.h"



bool GlobalCtl::init(std::unique_ptr<IConfigProvider> config) 
{
    LOG(INFO) << "GlobalCtl instance init..."; 
        // 测试小内存分配
        try {
            auto test_allocation = std::make_unique<char[]>(1024);
            LOG(INFO) << "Test allocation of 1KB successful";
        } catch (const std::bad_alloc& e) {
            LOG(ERROR) << "Even small memory allocation failed: " << e.what();
            return false;
        }
    
    g_config_ = std::move(config);
    
    if (!g_config_) 
    {
        LOG(ERROR) << "GlobalCtl instance init failed: no config provider!";
        return false;
    }
    
    if(!g_config_->readConf()) 
    {
        LOG(ERROR) << "GlobalCtl instance readConfig failed!";
        return false;
    }

    
    buildDomainInfoList();
    if (domain_info_list_.empty()) 
    {
        LOG(ERROR) << "GlobalCtl instance buildDomainInfoList failed!";
        return false;
    }

    // 在初始化ThreadPool前添加调试输出
    LOG(INFO) << "About to initialize ThreadPool";
    if (!g_thread_pool_) 
    {
        try {
            g_thread_pool_ = std::make_unique<ThreadPool>(2); // 尝试使用更少的线程
            LOG(INFO) << "ThreadPool initialized successfully";
        } catch (const std::bad_alloc& e) {
            LOG(ERROR) << "ThreadPool initialization failed: " << e.what();
            return false;
        }
    }
    LOG(INFO) << "Initializing ThreadPool with 4 threads";

    
    // 在初始化SipCore前添加调试输出
    LOG(INFO) << "About to initialize SipCore";   
    if (!g_sip_core_) 
    {
        try {
            g_sip_core_ = std::make_shared<SipCore>();
            LOG(INFO) << "SipCore object created successfully";
        } catch (const std::bad_alloc& e) {
            LOG(ERROR) << "SipCore creation failed: " << e.what();
            return false;
        }
    }
    try {
        g_sip_core_->initSip(g_config_->getSipPort());
        LOG(INFO) << "SipCore initialized successfully";
    } catch (const std::bad_alloc& e) {
        LOG(ERROR) << "SipCore initialization failed: " << e.what();
        return false;
    }
    
    LOG(INFO) << "Initializing SipCore at port: " << g_config_->getSipPort();
    LOG(INFO) << "GlobalCtl instance init success!";
    return true;
}

void GlobalCtl::buildDomainInfoList()
{
    LOG(INFO) << "Building DomainInfo list...";
    std::unique_lock<std::shared_mutex> lock(domain_mutex_);
    const auto& nodes = g_config_->getNodeInfoList();
    domain_info_list_.clear();
    if (nodes.size() > 1000) // 假设 100,000 是最大允许值
    {
        LOG(ERROR) << "Too many nodes in configuration: " << nodes.size();
        return;
    }
    try {
        domain_info_list_.reserve(nodes.size());
    } catch (const std::bad_alloc& e) {
        LOG(ERROR) << "Memory allocation failed in buildDomainInfoList: " << e.what();
        return;
    }
    for(const auto& node : nodes)
    {
        domain_info_list_.emplace_back(node);
    }
    LOG(INFO) << "Built " << domain_info_list_.size() << " domain entries";
}

