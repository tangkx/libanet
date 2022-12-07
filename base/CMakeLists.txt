set(base_SRCS
    AsyncLogging.cc
    Condition.cc
    CountDownLatch.cc
    CurrentThread.cc
    Date.cc
    Exception.cc
    FileUtil.cc
    LogFile.cc
    Logging.cc
    LogStream.cc
    ProcessInfo.cc
    Timestamp.cc
    Thread.cc
    ThreadPool.cc
    TimeZone.cc
)

add_library(libanet_base ${base_SRCS})
target_link_libraries(libanet_base pthread rt)
install(TARGETS libanet_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/libanet/base)

if(LIBANET_BUILD_EXAMPLES)
    add_subdirectory(tests)
endif()