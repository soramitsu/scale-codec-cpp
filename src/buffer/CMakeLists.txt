
add_library(buffer
    hexutil.cpp
    )
target_include_directories(buffer PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    )
target_link_libraries(buffer
    Boost::boost
    )
