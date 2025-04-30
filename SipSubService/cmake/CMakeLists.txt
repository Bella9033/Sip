# CMakeLists.txt for SipSubService

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")



# 指定CMake的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 声明项目名称
project(SipSubService)

# 设置 C++ 标准为 C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必要的库
find_package(glog REQUIRED)
find_package(gflags REQUIRED)
find_package(fmt REQUIRED)

# 添加编译选项，忽略特定警告
add_compile_options(-Wall -Wno-unused-function -Wno-unused-variable)

# 添加必要的宏定义 (只在这里定义，不要在源文件中重复定义)
add_definitions(
    -DGLOG_NO_ABBREVIATED_SEVERITIES
    -DGOOGLE_GLOG_DLL_DECL
    -DGLOG_USE_GLOG_EXPORT
)

# 设置编译后的可执行名称并指定变量名
SET(EXE_NAME SipSubService)

# 判断当前使用的操作系统
MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

# 在编译时使用-g选项添加调试信息
ADD_DEFINITIONS(-g)

# 指定需要编译的源文件路径
AUX_SOURCE_DIRECTORY(../src SRC)

# 包含目录
INCLUDE_DIRECTORIES(
    ../include
    ../../../Third/include
    ../../../Third/include/pjsip
    ../../../Third/include/jthread
    ../../../Third/include/jrtplib3
    ../../../Third/include/libevent
    ../../../Third/include/tinyxml2
    
)

# 指定需要链接的库文件路径
LINK_DIRECTORIES(
    ../../../Third/lib
    /usr/local/lib
)

# 生成可执行文件
ADD_EXECUTABLE(${EXE_NAME} ${SRC})

# 添加链接库（注意顺序很重要,一般原则是：被依赖的库应该放在后面。）
target_link_libraries(${EXE_NAME} 
    libglog.a
    libgflags.a
    -lunwind
    libjrtp.a
    libjthread.a
    libevent.a
    libevent_pthreads.a
    libtinyxml2.a
    -lpjsua2-x86_64-unknown-linux-gnu 
    -lpjsua-x86_64-unknown-linux-gnu 
    -lpjsip-ua-x86_64-unknown-linux-gnu 
    -lpjsip-simple-x86_64-unknown-linux-gnu
    -lpjsip-x86_64-unknown-linux-gnu
    -lpjmedia-x86_64-unknown-linux-gnu
    -lpjnath-x86_64-unknown-linux-gnu
    -lpjlib-util-x86_64-unknown-linux-gnu
    -lpj-x86_64-unknown-linux-gnu
    -lssl            
    -lcrypto           
    -luuid             
    -lpthread 
    fmt::fmt
)