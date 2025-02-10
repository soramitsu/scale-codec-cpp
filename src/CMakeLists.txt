set(SCRIPT_PATH "${CMAKE_SOURCE_DIR}/scripts/generate_aggregate_hpp.sh")
set(AGGREGATE_HPP_IN "${CMAKE_SOURCE_DIR}/include/scale/detail/aggregate.hpp.in")
set(AGGREGATE_HPP "${CMAKE_BINARY_DIR}/include/scale/detail/aggregate.hpp")
add_custom_command(
    OUTPUT  ${AGGREGATE_HPP}
    COMMAND ${CMAKE_COMMAND} -E echo "Running: ${SCRIPT_PATH} '${AGGREGATE_HPP_IN}' '${MAX_AGGREGATE_FIELDS}' '${AGGREGATE_HPP}'"
    COMMAND ${SCRIPT_PATH} ${AGGREGATE_HPP_IN} ${MAX_AGGREGATE_FIELDS} ${AGGREGATE_HPP}
    DEPENDS ${SCRIPT_PATH} ${AGGREGATE_HPP_IN}
    COMMENT "Generating include/scale/detail/aggregate.hpp"
    VERBATIM
)

add_library(scale
    encode_append.cpp
    scale_decoder_stream.cpp
    scale_encoder_stream.cpp
    scale_error.cpp
    ${AGGREGATE_HPP}
    )

target_include_directories(scale PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
target_link_libraries(scale
    Boost::boost
    )
