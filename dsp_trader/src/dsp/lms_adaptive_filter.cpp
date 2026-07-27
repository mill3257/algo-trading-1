#include "dsp/lms_adaptive_filter.h"
#include <stdexcept>

namespace dsp_trader::dsp {

LMSAdaptiveFilter::LMSAdaptiveFilter(const Config& cfg) : cfg_(cfg) {
    if (cfg_.taps <= 0) throw std::invalid_argument("LMS: taps must be > 0");
    w_.assign(cfg_.taps, 0.0);
    x_buf_.assign(cfg_.taps, 0.0);
}

double LMSAdaptiveFilter::process(double /*x_n*/, double /*desired*/) {
    // TODO:
    // 1. Push x_n into x_buf_ (shift delay line)
    // 2. y = dot(w_, x_buf_)
    // 3. e = desired - y
    // 4. w_ = (1-leak)*w_ + mu*e*x_buf_   (leaky LMS)
    // 5. Store last_e_, last_y_, return e
    return 0.0;
}

double LMSAdaptiveFilter::filter(double /*x_n*/) const {
    // TODO: y = dot(w_, x_buf_), no weight update
    return 0.0;
}

void LMSAdaptiveFilter::reset() {
    std::fill(w_.begin(),     w_.end(),     0.0);
    std::fill(x_buf_.begin(), x_buf_.end(), 0.0);
    last_e_ = last_y_ = 0.0;
}

} // namespace dsp_trader::dsp
