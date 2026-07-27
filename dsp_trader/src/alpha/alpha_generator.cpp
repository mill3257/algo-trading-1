#include "alpha/alpha_generator.h"

namespace dsp_trader::alpha {

AlphaGenerator::AlphaGenerator(const Config& cfg)
    : cfg_(cfg)
    , kalman_(dsp::KalmanFilter::Params{})
    , trend_filter_(dsp::IIRFilterBank::ema_cascade(0.1, 2))
    , adaptive_(dsp::LMSAdaptiveFilter::Config{ 8, 0.01, 0.0 })
    , spectral_(dsp::SpectralAnalyzer::Config{ 64, 1.0, true })
{}

AlphaSignal AlphaGenerator::on_bar(const core::OHLCVBar& bar, Regime regime) {
    // TODO: run each DSP component, call combine_signals(), apply_hysteresis()
    // For now returns a neutral signal so the pipeline can run end-to-end
    return AlphaSignal{ 0.0, current_position_, regime, 0.0, false };
}

double AlphaGenerator::combine_signals(double, double, double, Regime) const {
    // TODO: regime-conditional weighted sum
    return 0.0;
}

double AlphaGenerator::apply_hysteresis(double raw) {
    if (raw >  cfg_.threshold_high) current_position_ =  1.0;
    if (raw < -cfg_.threshold_low)  current_position_ = -1.0;
    return current_position_;
}

void AlphaGenerator::reset() {
    current_position_ = 0.0;
    trend_filter_.reset();
    adaptive_.reset();
}

} // namespace dsp_trader::alpha
