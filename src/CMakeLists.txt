message(STATUS "<src>")

find_package(gflags REQUIRED)
set(glog_DIR /home/liujiaxi/third_party_lib/glog/lib/cmake/glog)
find_package(glog REQUIRED)

add_library(smart_routine
smart_coro.cpp
smart_epoller.cpp
smart_routine.cpp
smart_runtime.cpp
smart_thread.cpp
epoll_item.cpp
internal/libcontext.cpp
internal/smart_routine_impl.cpp
common/buffer.cpp
common/dynamic_buffer.cpp
smart_timer.cpp
endpoint.cpp
acceptor.cpp
address.cpp
tcp_socket.cpp
)

target_include_directories(smart_routine PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(smart_routine PUBLIC glog::glog ${GFLAGS_LIBRARIES})