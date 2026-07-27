#include "dsp/kalman_filter.h"

namespace dsp_trader::dsp {

KalmanFilter::KalmanFilter(const Params& p) : p_(p) {
    P_ = {1e4, 0.0, 0.0, 1e4};
}

void KalmanFilter::initialize(double price0) {
    state_.price       = price0;
    state_.velocity    = 0.0;
    state_.var_price   = 1.0;
    state_.var_velocity = 1.0;
    P_ = {1.0, 0.0, 0.0, 1.0};
}

void KalmanFilter::predict() {
    // TODO: x = F*x, P = F*P*F^T + Q
    // F = [[1, dt],[0, 1]]
}

void KalmanFilter::update(double /*z*/) {
    // TODO: K = P*H^T / (H*P*H^T + R), x += K*(z - H*x), P = (I-KH)*P
    // H = [1, 0]
}

KalmanFilter::State KalmanFilter::step(double z) {
    predict();
    update(z);
    return state_;
}

void KalmanFilter::set_params(const Params& p) {
    p_ = p; // state preserved, only noise model changes
}

} // namespace dsp_trader::dsp
