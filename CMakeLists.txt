cmake_minimum_required(VERSION 3.8)

project(jericho)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)

if (NOT DEFINED DEPS_DIR)
    set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext")
endif (NOT DEFINED DEPS_DIR)
message("Deps dir is: ${DEPS_DIR}")

if (NOT DEFINED BUILD_DIR)
    set(BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")
endif (NOT DEFINED BUILD_DIR)
message("Deps dir is: ${DEPS_DIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/include/jericho)

option(BUILD_SHARED_LIBS "Building shared library..." OFF)
if (BUILD_SHARED_LIBS)
    set(SHARED_FLAG SHARED)
else()
    set(SHARED_FLAG STATIC)
endif (BUILD_SHARED_LIBS)

add_library(jericho STATIC
    core/src/crypt/base64.cc
    core/src/crypt/sha1.cc
    core/src/crypt/utf8.cc
    core/src/server/client.cc
    core/src/server/event.cc
    core/src/server/frame.cc
    core/src/server/ping_pong.cc
    core/src/server/response.cc
    core/src/server/shutdown.cc
    core/src/server/smtp.cc
    core/src/server/socket.cc
    core/src/server/server.cc
    core/src/server/resource.cc
    core/src/server/thread_pool.cc
    core/src/server/web_socket.cc
    core/src/server/iris.cc
    core/src/server/request.cc
    core/src/router/router.cc
    core/src/router/route_registry.cc
    core/src/message/message_buffer.cc
    core/src/message/message_broker.cc
    core/src/cluster/cluster_index.cc
    core/src/cluster/cluster_edge.cc
    core/src/cluster/cluster_node.cc
    core/src/cluster/cluster.cc
    core/src/ssh/ssh_auth.cc
    core/src/ssh/ssh_command.cc
    core/src/ssh/ssh_connect.cc
    core/src/ssh/ssh_download.cc
    core/src/ssh/ssh_version.cc        
    core/src/util/file_system.cc   
    core/src/util/scan.cc
    core/src/util/string_utils.cc
    core/src/server/fetch.cc
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# add_executable (curl-demo convert.cpp)
# include_directories(${CURL_INCLUDE_DIR})
# target_link_libraries(curl-demo ${CURL_LIBRARIES})

#if (NOT TARGET prizm-cxx)
#    add_subdirectory(${DEPS_DIR}/prizm-cxx)
#    include_directories(${DEPS_DIR}/prizm-cxx/include)
#endif()

add_subdirectory(${DEPS_DIR}/celerity)
include_directories(${DEPS_DIR}/celerity/build/include)

include_directories(${DEPS_DIR}/openssl/include)
# include_directories(${BUILD_DIR}/libzmq/include)
# include_directories(${BUILD_DIR}/cppzmq/include)
# include_directories(${DEPS_DIR}/tensorflow/include)

link_directories(${DEPS_DIR}/openssl/lib64)
link_directories(${DEPS_DIR}/openssl/lib)
# link_directories(${DEPS_DIR}/tensorflow/lib)
# link_directories(${BUILD_DIR}/libzmq/lib)
# link_directories(${BUILD_DIR}/cppzmq/lib)
link_directories(${DEPS_DIR}/celerity/build/lib)
link_directories(${DEPS_DIR}/celerity/db/mongo-c/lib)

# target_link_libraries(jericho celerity)
add_executable(server core/src/main.cc)
# add_executable(tf example/tensorflow.cc)
# add_executable(zero example/zeromq.cc)
add_executable(client example/client.cc)



target_link_libraries(server PRIVATE jericho)
target_link_libraries(server PRIVATE celerity)
target_link_libraries(server PRIVATE crypto)
target_link_libraries(server PRIVATE ssl)
target_link_libraries(server PRIVATE curl)
target_link_libraries(server PRIVATE prizm-cxx)

target_link_libraries(client PRIVATE ssl)
target_link_libraries(client PRIVATE crypto)

# target_link_libraries(zero PRIVATE zmq)
# target_link_libraries(tf PRIVATE tensorflow)

target_link_libraries(server PRIVATE Threads::Threads)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ext/prizm-cxx/include/prizm DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/include)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/core/include/jericho DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/include)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ext/openssl/include/openssl DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/include)

if (UNIX)
	install(DIRECTORY ${DEPS_DIR}/openssl/lib64/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/lib
     	USE_SOURCE_PERMISSIONS FILES_MATCHING PATTERN "*.so*")
    install(DIRECTORY ${DEPS_DIR}/openssl/lib64/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/lib
     	USE_SOURCE_PERMISSIONS FILES_MATCHING PATTERN "*.a*")
else (UNIX)
	## Similarly for Windows.
endif (UNIX)