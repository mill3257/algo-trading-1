#include "dsp/kalman_filter.h"
#include <cassert>
#include <cstdio>
#include <cmath>

using namespace dsp_trader::dsp;

// TODO: fill in once KalmanFilter::predict() and update() are implemented

void test_steady_state_constant_signal() {
    // Feed a constant price for many steps; estimated price should converge to it
    // TODO: assert std::abs(state.price - 100.0) < 0.1 after 100 steps
    std::puts("[SKIP] steady state — implement predict/update first");
}

void test_velocity_tracks_ramp() {
    // Feed a linearly increasing price; velocity should converge to the slope
    // TODO: assert std::abs(state.velocity - slope) < tolerance after warmup
    std::puts("[SKIP] velocity tracks ramp — implement predict/update first");
}

void test_uncertainty_decreases() {
    // var_price should decrease monotonically after initialization until steady state
    // TODO: assert var_price[t+1] <= var_price[t] for first N steps
    std::puts("[SKIP] uncertainty decreases — implement predict/update first");
}

void test_initialize_no_transient() {
    // After initialize(100.0), first step on observation 100.0 should not spike
    // TODO: assert state.price stays near 100.0
    std::puts("[SKIP] initialize no transient — implement initialize() first");
}

void test_high_Q_tracks_fast() {
    // High Q_price → filter tracks sudden price changes faster
    // Low Q_price  → filter is smoother but lags
    // TODO: compare convergence speed with two different Q settings
    std::puts("[SKIP] Q tuning effect — implement full filter first");
}

int main() {
    test_steady_state_constant_signal();
    test_velocity_tracks_ramp();
    test_uncertainty_decreases();
    test_initialize_no_transient();
    test_high_Q_tracks_fast();
    return 0;
}
