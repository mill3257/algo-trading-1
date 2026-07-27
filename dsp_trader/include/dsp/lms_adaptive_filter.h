#pragma once
#include <vector>
#include <deque>

namespace dsp_trader::dsp {

/// LMS (Least Mean Squares) adaptive FIR filter.
///
/// Weight update rule:
///   y[n]      = w^T * x[n]          (filter output)
///   e[n]      = d[n] - y[n]         (error / residual)
///   w[n+1]    = (1 - leak)*w[n] + mu * e[n] * x[n]   (leaky LMS)
///
/// Trading applications:
///   1. Noise floor estimation: reference = mid-price, desired = trade price
///      → residual e[n] captures microstructure noise
///   2. Online AR prediction: lagged prices as input, next price as desired
///      → weights converge to AR coefficients
///   3. Spread modeling: adaptively track bid-ask dynamics
///
/// Stability: 0 < mu < 2 / (N * E[x^2])
/// Leaky LMS (leak > 0) prevents weight blow-up in non-stationary environments.
///
/// TODO (you implement):
///   - process(x_n, desired) : update delay line, compute output, apply LMS update
///                             return prediction error e[n]
///   - filter(x_n)           : inference only, frozen weights (no update)
///   - reset()               : zero weights and delay line
class LMSAdaptiveFilter {
public:
    struct Config {
        int    taps  = 8;    // filter order N
        double mu    = 0.01; // step size / learning rate
        double leak  = 0.0;  // leakage coefficient (0 = standard LMS)
    };

    explicit LMSAdaptiveFilter(const Config& cfg);

    double process(double x_n, double desired); // TODO: returns error e[n]
    double filter(double x_n) const;             // TODO: inference, no weight update

    void reset();
    void set_mu(double mu)  { cfg_.mu = mu; }

    const std::vector<double>& weights()     const { return w_; }
    double                     last_error()  const { return last_e_; }
    double                     last_output() const { return last_y_; }

private:
    Config              cfg_;
    std::vector<double> w_;        // filter weights
    std::deque<double>  x_buf_;   // delay line
    double              last_e_{0.0};
    double              last_y_{0.0};
};

} // namespace dsp_trader::dsp
