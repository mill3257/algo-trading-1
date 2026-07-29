#pragma once
#include "alpha/alpha_generator.h"
#include "dsp/spectral_analyzer.h"

namespace dsp_trader::alpha {

/*
classifies current market into one of six regimes: 
{Bullish, Bearish, Sideways} x {Quiet, Volatile}

Detection logic (rule-based baseline, phase 1-3)

Step 1: direction from kalman velocity:
    velocity > velocity_thresh --> bullish
    velocity < -velocity_thresh --> bearish
    otherwise --> sideways

Step 2: volatility from spectral entropy
    entropy < entropy_queit --> quiet
    entropy >= entropy_quiet --> volatile

Combined direction and volatility produce on of the six Regime values

Velocity threshold calibration:
    typical SPY kalman velocity at 1-minute bars = +-0.02 $/bar
    set velocity_thresh slightly above the noise floor of the kalman estimate
    the kalman var_velocity gives a data-driven uncertainty bound

Phase 4: rule-based path serves as a fallback when the ML service (from ZMQ) is stale
    set_override() installs an ML-provided label, clear_override() reverts

TODO:
    -update(features, kalman_state): classify direction from velocity
    -volatility from psectral_entropy, return combined regime
*/
class RegimeDetector {
public:
    struct Config {
        double velocity_thresh = 0.02; //|velocity| below this -->sideways
        double entropy_quiet = 0.45; //entropy below this --> quiet, above --> volatile
        int min_bars = 5; //suppress classification until DSP window fills
    };

    RegimeDetector();
    explicit RegimeDetector(const Config& cfg);

    //classify regime from spectral features and current kalman state
    //kalman_velocity comes from KalmanFilter::state().velocity
    //kalman_var_velocity comes from KalmanFilter::state().var_velocity,
    // use it to widen the sideways band when kalman is still uncertain
    Regime update(const dsp::SpectralAnalyzer::SpectralFeatures& features, double kalman_velocity, double kalman_var_velocity = 0.0); // TODO

    Regime current_regime() const { return current_; }

    // Phase 4: called by ZMQ subscriber thread with ML service label
    void set_override(Regime r) { override_ = r; use_override_ = true; }
    void clear_override()       { use_override_ = false; }

private:
    Config cfg_;
    Regime current_{Regime::SidewaysVolatile};
    Regime override_{Regime::SidewaysVolatile};
    bool use_override_{false};
    int bars_seen_{0};

    //classify directional bias from velocity
    static Regime direction_from_velocity(double velocity, double thresh, double var_velocity);

    //apply volatility axis from spectral entropy
    static Regime apply_volatility(Regime direction_regime, double entropy, double entropy_thresh);
};

} 
