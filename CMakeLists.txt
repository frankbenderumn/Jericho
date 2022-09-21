cmake_minimum_required(VERSION 3.8)

project(jericho)

set(CMAKE_BUILD_TYPE Release)

if (NOT DEFINED DEPS_DIR)
    set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext")
endif (NOT DEFINED DEPS_DIR)
message("Deps dir is: ${DEPS_DIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/jericho)

option(BUILD_SHARED_LIBS "Building shared library..." OFF)
if (BUILD_SHARED_LIBS)
    set(SHARED_FLAG SHARED)
else()
    set(SHARED_FLAG STATIC)
endif (BUILD_SHARED_LIBS)

add_library(jericho ${SHARED_FLAG} 
    src/crypt/base64.cc
    src/crypt/sha1.cc
    src/crypt/utf8.cc
    src/server/client.cc
    src/server/event.cc
    src/server/frame.cc
    src/server/ping_pong.cc
    src/server/response.cc
    src/server/shutdown.cc
    src/server/smtp.cc
    src/server/socket.cc
    src/server/server.cc
    src/server/resource.cc
    src/server/thread_pool.cc
    src/server/web_socket.cc
    src/server/router.cc
    src/server/iris.cc
    src/ssh/ssh_auth.cc
    src/ssh/ssh_command.cc
    src/ssh/ssh_connect.cc
    src/ssh/ssh_download.cc
    src/ssh/ssh_version.cc        
    src/util/file_system.cc   
    src/util/scan.cc
    src/client_ui.cc
    src/trash.c
    src/unbel.cc
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


add_subdirectory(${DEPS_DIR}/prizm-cxx)
include_directories(${DEPS_DIR}/prizm-cxx/include)
include_directories(${DEPS_DIR}/openssl/include)
link_directories(${DEPS_DIR}/openssl/lib64)
target_link_libraries(jericho prizm-cxx)
add_executable(server src/main.cc)

target_link_libraries(server PRIVATE jericho)
target_link_libraries(server PRIVATE prizm-cxx)
target_link_libraries(server PRIVATE crypto)
target_link_libraries(server PRIVATE ssl)

target_link_libraries(server PRIVATE Threads::Threads)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ext/prizm-cxx/include/prizm DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/include)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/jericho DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/include)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ext/openssl/include/openssl DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/include)

if (UNIX)
	install(DIRECTORY ${DEPS_DIR}/openssl/lib64/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/lib
     	USE_SOURCE_PERMISSIONS FILES_MATCHING PATTERN "*.so*")
    install(DIRECTORY ${DEPS_DIR}/openssl/lib64/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/lib
     	USE_SOURCE_PERMISSIONS FILES_MATCHING PATTERN "*.a*")
else (UNIX)
	## Similarly for Windows.
endif (UNIX)