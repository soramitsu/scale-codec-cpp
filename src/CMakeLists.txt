add_library(scale
    encode_append.cpp
    scale_decoder_stream.cpp
    scale_encoder_stream.cpp
    scale_error.cpp
    )

target_include_directories(scale PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
target_link_libraries(scale
    Boost::boost
    )
