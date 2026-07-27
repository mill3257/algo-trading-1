set(TEST_LIBS core_lib dsp_lib alpha_lib risk_lib execution_lib)

function(add_dsp_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE ${TEST_LIBS})
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_dsp_test(test_ring_buffer)
add_dsp_test(test_biquad)
add_dsp_test(test_kalman)
add_dsp_test(test_lms)
add_dsp_test(test_pipeline)
