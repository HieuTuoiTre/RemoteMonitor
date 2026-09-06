cmake_minimum_required(VERSION 3.21)
project(RemoteMonitor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Sql Test)

add_library(monitor_common
    common/protocol/Message.cpp
    common/protocol/Message.h
    common/transport/FramedSocket.cpp
    common/transport/FramedSocket.h
)
target_link_libraries(monitor_common PUBLIC Qt6::Core Qt6::Network)
target_include_directories(monitor_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(monitor_manager
    manager/main.cpp
    manager/database/Database.cpp
    manager/database/Database.h
    manager/network/ManagerServer.cpp
    manager/network/ManagerServer.h
    manager/ui/ManagerWindow.cpp
    manager/ui/ManagerWindow.h
    manager/ui/ControlView.cpp
    manager/ui/ControlView.h
)
target_link_libraries(monitor_manager PRIVATE monitor_common Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Sql)

add_executable(monitor_agent
    agent/main.cpp
    agent/network/AgentClient.cpp
    agent/network/AgentClient.h
    agent/system/SystemInfo.cpp
    agent/system/SystemInfo.h
    agent/capture/ScreenCapture.cpp
    agent/capture/ScreenCapture.h
    agent/input/InputController.cpp
    agent/input/InputController.h
    agent/ui/AgentWindow.cpp
    agent/ui/AgentWindow.h
)
target_link_libraries(monitor_agent PRIVATE monitor_common Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)

if(WIN32)
    target_compile_definitions(monitor_agent PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

enable_testing()
add_executable(monitor_common_tests tests/MessageTests.cpp)
target_link_libraries(monitor_common_tests PRIVATE monitor_common Qt6::Test)
add_test(NAME monitor_common_tests COMMAND monitor_common_tests)
