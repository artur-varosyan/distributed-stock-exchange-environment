cmake_minimum_required(VERSION 3.22)
project(simulation)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.76.0 COMPONENTS system filesystem serialization program_options REQUIRED)

if(Boost_FOUND)

    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS})

endif()

add_executable(simulation src/main.cpp
                          src/networking/tcpserver.cpp
                          src/networking/tcpconnection.cpp
                          src/networking/udpserver.cpp
                          src/networking/networkentity.cpp
                          src/agent/agent.cpp
                          src/agent/traderagent.cpp
                          src/agent/stockexchange.cpp
                          src/order/orderqueue.cpp
                          src/order/orderbook.cpp
                          src/config/configreader.cpp
                          src/pugi/pugixml.cpp)

if(Boost_FOUND)

    target_link_libraries(simulation ${Boost_LIBRARIES})

endif()
