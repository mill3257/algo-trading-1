#include "alpha/regime_detector.h"
#include <cmath>

namespace dsp_trader::alpha {

RegimeDetector::RegimeDetector() : cfg_({}) {}
RegimeDetector::RegimeDetector(const Config& cfg) : cfg_(cfg) {}

Regime RegimeDetector::update(const dsp::SpectralAnalyzer::SpectralFeatures& features,
        double kalman_velocity, double kalman_var_velocity) 
{
    if (use_override_) { current_ = override_; return current_; }

    ++bars_seen_;
    if (bars_seen_ < cfg_.min_bars) return current_;

    // classify direction from kalman velocity
    // widen the sideways band by one standard deviation of velocity uncertainty
    // so we don't flip direction on noise before the filter has converged
    Regime direction = direction_from_velocity(
        kalman_velocity, cfg_.velocity_thresh + std::sqrt(kalman_var_velocity), kalman_var_velocity);
    
    //classify volatility from spectran entropy and combine
    current_ = apply_volatility(direction, features.spectral_entropy, cfg_.entropy_quiet);

    return current_;
}

Regime RegimeDetector::direction_from_velocity(double velocity, double thresh, double var_velocity)
{
    if (velocity > thresh) return Regime::BullishQuiet; // placeholder direction half
    if (velocity < thresh) return Regime::BearishQuiet; // volatility axis added below
    return Regime::SidewaysQuiet;
}

Regime RegimeDetector::apply_volatility(Regime direction_regime, double entropy, double entropy_thresh)
{
    bool isvolatile = (entropy >= entropy_thresh);

    //map direction (stored as quiet variant) to quiet or volatile
    if (direction_regime == Regime::BullishQuiet)
        return isvolatile ? Regime::BullishVolatile : Regime::BullishQuiet;
    if (direction_regime == Regime::BearishQuiet)
        return isvolatile ? Regime::BearishVolatile : Regime::BearishQuiet;
    return isvolatile ? Regime::SidewaysVolatile : Regime::SidewaysQuiet;
}

} // namespace dsp_trader::alpha
