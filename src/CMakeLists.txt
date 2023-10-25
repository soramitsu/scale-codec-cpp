add_library(scale
    scale_decoder_stream.cpp
    scale_encoder_stream.cpp
    scale_error.cpp
    )

target_include_directories(scale PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/scale>
    )
target_link_libraries(scale
    Boost::boost
    )

add_library(scale_encode_append
    encode_append.cpp
    )
target_link_libraries(scale_encode_append
    scale
    )
target_include_directories(scale_encode_append PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/scale>
    )
