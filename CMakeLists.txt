cmake_minimum_required(VERSION 3.22)

project(jericho)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)
set(OPENSSL_ROOT_DIR /usr/local/comet/build)

if (NOT DEFINED DEPS_DIR)
    set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext")
endif (NOT DEFINED DEPS_DIR)
message("Deps dir is: ${DEPS_DIR}")

if (NOT DEFINED BUILD_DIR)
    set(BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")
endif (NOT DEFINED BUILD_DIR)
message("Deps dir is: ${DEPS_DIR}")

if (NOT DEFINED COMET_DIR)
    set(COMET_DIR "/usr/local/comet")
endif (NOT DEFINED COMET_DIR)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/include/jericho)

option(ENABLE_STATIC "Building static library..." OFF)
if (ENABLE_STATIC)
    set(SHARED_FLAG STATIC)
else()
    set(SHARED_FLAG SHARED)
endif (ENABLE_STATIC)

add_library(jericho SHARED_FLAG
    core/src/crypt/base64.cc
    core/src/crypt/sha1.cc
    core/src/crypt/utf8.cc
    core/src/crypt/rsa.cc
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
    core/src/iris/iris.cc
    core/src/server/request.cc
    core/src/system/router.cc
    core/src/system/system.cc
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
    core/src/api/api_helper.cc
    core/src/federate/federator.cc
    core/src/federate/fed_node.cc
    core/src/message/bifrost.cc
    core/src/util/encoding.cc
    core/src/util/trace.cc
    core/src/system/routes.cc
    # FUCK ME - need to do a UML and track circular dependency or use scripting / 2 pphase compilation
    core/src/controller/auth.cc
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${COMET_DIR}/include)
link_directories(${COMET_DIR}/lib)
# link_directories(${DEPS_DIR}/celerity/build/lib)
# link_directories(${DEPS_DIR}/celerity/db/mongo-c/lib)

# MODULES
# target_link_libraries(jericho ssl)
# target_link_libraries(jericho crypto)
# target_link_libraries(jericho curl)
target_link_libraries(jericho Threads::Threads)

# target_link_libraries(jericho celerity)
add_executable(server core/src/main.cc)

# <<<<< PRIZM TEST
# add_executable(trace_test core/test/trace.cc)
# add_executable(jericho_migrate_test core/test/migrate.cc)
# add_executable(url_test core/test/url.cc)
# add_executable(request_test core/test/request.cc)
# add_executable(clock_test core/test/clock.cc)
# add_executable(celerity_integration_test core/test/celerity_integration.cc)
# add_executable(session_test core/test/session.cc)
# add_executable(iris_interpret_test core/test/iris/iris_interpret.cc)
# add_executable(iris_form_test core/test/iris/iris_form.cc)
# add_executable(iris_doc_test core/test/iris/iris_doc.cc)
# add_executable(iris_part_test core/test/iris/iris_part.cc)
# add_executable(iris_temp_test core/test/iris/iris_temp.cc)
# add_executable(cookie_test core/test/cookie.cc)
# add_executable(response_test core/test/response.cc)
# add_executable(system_test core/test/system.cc)
# add_executable(router_test core/test/router.cc)
# add_executable(rsa_test core/test/rsa.cc)
# add_executable(crypt_playground_test core/test/crypt_playground.cc)
# add_executable(cgi_test core/test/cgi.cc)
# add_executable(jwt_test core/test/jwt.cc)

# target_link_libraries(trace_test jericho prizm-cxx celerity)
# target_link_libraries(jericho_migrate_test jericho prizm-cxx celerity)
# target_link_libraries(url_test jericho prizm-cxx celerity)
# target_link_libraries(request_test jericho prizm-cxx celerity)
# target_link_libraries(clock_test jericho prizm-cxx celerity)
# target_link_libraries(celerity_integration_test jericho prizm-cxx celerity)
# target_link_libraries(session_test jericho prizm-cxx celerity)
# target_link_libraries(iris_interpret_test jericho prizm-cxx celerity)
# target_link_libraries(iris_doc_test jericho prizm-cxx celerity)
# target_link_libraries(iris_temp_test jericho prizm-cxx celerity)
# target_link_libraries(iris_form_test jericho prizm-cxx celerity)
# target_link_libraries(iris_part_test jericho prizm-cxx celerity)
# target_link_libraries(cookie_test jericho prizm-cxx celerity)
# target_link_libraries(response_test jericho prizm-cxx celerity)
# target_link_libraries(system_test jericho prizm-cxx celerity)
# target_link_libraries(router_test jericho prizm-cxx celerity)
# target_link_libraries(rsa_test jericho prizm-cxx celerity)
# target_link_libraries(crypt_playground_test jericho prizm-cxx celerity)
# target_link_libraries(cgi_test jericho prizm-cxx celerity)
# target_link_libraries(jwt_test jericho prizm-cxx celerity)

set(TESTS
    trace
    jericho_migrate
    url
    request
    clock
    celerity_integration
    session
    iris_interpret
    iris_doc
    iris_temp
    iris_form
    iris_part
    cookie
    response
    system
    router
    rsa
    crypt_playground
    cgi
    jwt
)

foreach(TEST ${TESTS})
    add_executable(${TEST}_test core/test/${TEST}.cc)
    target_link_libraries(${TEST}_test jericho prizm celerity)
    install(TARGETS ${TEST}_test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/test)
endforeach()

add_executable(auth_cgi core/cgi/auth.cc)
target_link_libraries(auth_cgi jericho prizm-cxx celerity)

# target_link_libraries(interpret_test jericho prizm-cxx celerity)
# >>>>>>

# target_link_libraries(jericho celerity)

target_link_libraries(server jericho)
target_link_libraries(server celerity)
target_link_libraries(server prizm-cxx)

# target_link_libraries(jericho celerity)
# target_link_libraries(jericho prizm-cxx)

# target_link_libraries(client PRIVATE ssl)
# target_link_libraries(client PRIVATE crypto)

# target_link_libraries(zero PRIVATE zmq)
# target_link_libraries(tf PRIVATE tensorflow)

# if (UNIX)
# 	install(DIRECTORY ${DEPS_DIR}/openssl/lib64/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/lib
#      	USE_SOURCE_PERMISSIONS FILES_MATCHING PATTERN "*.so*")
#     install(DIRECTORY ${DEPS_DIR}/openssl/lib64/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export/lib
#      	USE_SOURCE_PERMISSIONS FILES_MATCHING PATTERN "*.a*")
# else (UNIX)
# 	## Similarly for Windows.
# endif (UNIX)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/core/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(TARGETS jericho LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# INSTALL(TARGETS prizm-cxx DESTINATION bin)