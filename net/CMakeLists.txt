aux_source_directory(. DIR_LIB_SRCS)

add_library(libserver_net ${DIR_LIB_SRCS}) 

target_link_libraries(libserver_net pthread rt libserver_base profiler)
