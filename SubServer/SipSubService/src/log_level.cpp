 // log_level.cpp
 
 #include "common.h"
 #include <filesystem>
 #include <stdexcept>  // 异常处理
 
 
 #define LOG_DIR "/mnt/hgfs/share/log"  
 #define LOG_FILE_NAME "SipSupService.log"
 
 bool SetLogLevel::is_initialized_ = false;  
 
 SetLogLevel::SetLogLevel(LogLevel level)
     : loglevel_(level) 
 {
     // 所有实例共享同一个静态变量
     if (is_initialized_) 
     {  
         LOG(WARNING) << "Glog is already initialized!";
         return;
     }
     try {
         std::filesystem::path log_path(LOG_DIR);
         if (!std::filesystem::exists(log_path)) 
         {
             std::filesystem::create_directories(log_path);
         }
         google::InitGoogleLogging(LOG_FILE_NAME);
         is_initialized_ = true;
         configureLogging();
     } catch (const std::exception& e) {
         LOG(ERROR) << "Failed to initialize logging: " << e.what();
         throw;  
     }
 }
 
 void SetLogLevel::configureLogging() 
 {
     FLAGS_stderrthreshold = static_cast<int>(loglevel_);  // 显式转换
     FLAGS_colorlogtostderr = true;
     FLAGS_logbufsecs = 0;
     FLAGS_max_log_size = 4;
     FLAGS_log_dir = LOG_DIR;
     google::SetLogDestination(google::GLOG_INFO, "");
     google::SetLogDestination(google::GLOG_WARNING, "");
     google::SetLogDestination(google::GLOG_ERROR, "");
     signal(SIGPIPE, SIG_IGN);  // 需明确是否全局需要
 }
 
 SetLogLevel::~SetLogLevel() 
 {
     if (is_initialized_) 
     {
         google::ShutdownGoogleLogging();
         is_initialized_ = false;
     }
 }
 