aux_source_directory(. DIR_LIB_SRCS)

add_library(WEBSERVER ${DIR_LIB_SRCS})

target_link_libraries(WEBSERVER pthread rt)
