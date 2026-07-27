add_library(core_lib
    core/tick.cpp
    core/ohlcv_aggregator.cpp
    core/ring_buffer.cpp
    core/csv_loader.cpp
    core/backtest_harness.cpp
    core/latency_tracker.cpp
)
target_include_directories(core_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(dsp_lib
    dsp/biquad_filter.cpp
    dsp/iir_filter_bank.cpp
    dsp/kalman_filter.cpp
    dsp/lms_adaptive_filter.cpp
    dsp/spectral_analyzer.cpp
)
target_include_directories(dsp_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(alpha_lib
    alpha/alpha_generator.cpp
    alpha/regime_detector.cpp
)
target_include_directories(alpha_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(alpha_lib PRIVATE dsp_lib)

add_library(risk_lib
    risk/risk_manager.cpp
    risk/fill_model.cpp
)
target_include_directories(risk_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(execution_lib
    execution/order.cpp
    execution/paper_trader.cpp
)
target_include_directories(execution_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(execution_lib PRIVATE risk_lib)
