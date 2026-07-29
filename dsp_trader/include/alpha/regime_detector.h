#pragma once
#include "alpha/alpha_generator.h"
#include "dsp/spectral_analyzer.h"

namespace dsp_trader::alpha {

// Classifies the current market regime from spectral features
// Phase 1-3: rule-based threshold on spectral_entropy
//   entropy < low_thresh = Trending
//   entropy > high_thresh = Noisy
//   otherwise = MeanReverting
//
// Phase 4+: this classification is replaced by the Python ML service
// (HMM or gradient-boosted tree on spectral features), which pushes a
// Regime label over ZMQ. set_override() installs that label; the
// rule-based path serves as fallback if the ZMQ update is stale.
//
// TODO (you implement):
//   - update() : apply thresholds to SpectralFeatures, return regime
class RegimeDetector {
public:
    struct Config {
        double entropy_low  = 0.3;  // below = trending
        double entropy_high = 0.7;  // above = noisy
        int    min_bars     = 5;    // suppress regime changes until window fills
    };

    RegimeDetector();
    explicit RegimeDetector(const Config& cfg);

    Regime update(const dsp::SpectralAnalyzer::SpectralFeatures& features); // TODO
    Regime current_regime() const { return current_; }

    // Phase 4: called by ZMQ subscriber thread with ML service label
    void set_override(Regime r) { override_ = r; use_override_ = true; }
    void clear_override()       { use_override_ = false; }

private:
    Config cfg_;
    Regime current_{Regime::Noisy};
    Regime override_{Regime::Noisy};
    bool   use_override_{false};
    int    bars_seen_{0};
};

} 
