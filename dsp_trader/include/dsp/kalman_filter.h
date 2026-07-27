#pragma once
#include <array>

namespace dsp_trader::dsp {

/// 2-state Kalman filter: tracks price level and momentum (first derivative).
///
/// State:   x = [price, velocity]^T
/// Process: x_k = F*x_{k-1} + w,  w ~ N(0, Q)
///          F = [[1, dt], [0, 1]]   (constant-velocity model)
/// Measure: z_k = H*x_k + v,       v ~ N(0, R)
///          H = [1, 0]              (observe price directly)
///
/// Q and R are the key tuning knobs:
///   Q_price, Q_velocity : how much the price/velocity can change per step
///   R                   : measurement noise (calibrate from bid-ask spread or
///                         tick variance)
///
/// In Phase 4, Q and R are updated by the Python ML service (EM algorithm)
/// and swapped atomically via the ParamChannel. set_params() is the hook.
///
/// TODO (you implement):
///   - predict()      : propagate state and covariance (F*x, F*P*F^T + Q)
///   - update(z)      : Kalman gain, state correction, covariance update
///   - step(z)        : predict + update, return new state (convenience)
///   - initialize(p0) : set state to known price, reset P to low uncertainty
///   - set_params(p)  : hot-swap Q/R (called from ZMQ subscriber thread)
class KalmanFilter {
public:
    struct Params {
        double Q_price    = 1e-3; // process noise: price
        double Q_velocity = 1e-4; // process noise: velocity
        double R          = 1e-2; // measurement noise
        double dt         = 1.0;  // time step (seconds or bar units)
    };

    struct State {
        double price;
        double velocity;
        double var_price;    // P[0][0]: price estimation variance
        double var_velocity; // P[1][1]: velocity estimation variance
    };

    explicit KalmanFilter(const Params& p);

    void  predict();
    void  update(double z);
    State step(double z);     // predict + update

    void initialize(double price0);
    void set_params(const Params& p); // thread-safe enough for double-buffer pattern

    const State&  state()       const { return state_; }
    const Params& params()      const { return p_; }
    double        kalman_gain() const { return last_K_; }

private:
    Params p_;
    State  state_{};
    // 2x2 covariance P stored flat: [p00, p01, p10, p11]
    std::array<double, 4> P_{1e4, 0, 0, 1e4};
    double last_K_{0.0};
};

} // namespace dsp_trader::dsp
