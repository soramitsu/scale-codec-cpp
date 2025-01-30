set(SCRIPT_PATH "${CMAKE_SOURCE_DIR}/scripts/generate_aggregate_hpp.sh")
set(TARGET_FILE "${CMAKE_SOURCE_DIR}/include/scale/detail/aggregate.hpp")
set(STAMP_FILE "${CMAKE_BINARY_DIR}/include/scale/detail/aggregate.hpp.stamp")
add_custom_command(
    OUTPUT  ${STAMP_FILE}
    COMMAND ${CMAKE_COMMAND} -E echo "Running: ${SCRIPT_PATH} '${TARGET_FILE}' '${MAX_AGGREGATE_FIELDS}' '${STAMP_FILE}'"
    COMMAND ${SCRIPT_PATH} ${TARGET_FILE} ${MAX_AGGREGATE_FIELDS} ${STAMP_FILE}
    DEPENDS ${SCRIPT_PATH} ${TARGET_FILE}
    COMMENT "Generating include/scale/detail/aggregate.hpp"
    VERBATIM
)
add_custom_target(generate_aggregate_hpp ALL DEPENDS ${STAMP_FILE})

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
