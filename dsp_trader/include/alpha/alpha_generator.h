#pragma once
#include "core/ohlcv_aggregator.h"
#include "dsp/kalman_filter.h"
#include "dsp/iir_filter_bank.h"
#include "dsp/lms_adaptive_filter.h"
#include "dsp/spectral_analyzer.h"

namespace dsp_trader::alpha {

enum class Regime { Trending, MeanReverting, Noisy };

struct AlphaSignal {
    double raw;            // combined signal value between -1 and 1 inclusive
    double position;       // target position after hysteresis: {-1, 0, +1}
    Regime regime;
    double confidence;     // proxy for signal quality (e.g. inverse Kalman variance)
    bool   signal_changed; // true only when position flips (drives order submission)
};

// Combines DSP pipeline outputs into a scalar position signal.
// Hysteresis thresholding (Schmitt trigger):
//   Enter long  when raw > threshold_high
//   Enter short when raw < threshold_low
//   Hold current position within the dead-band
//   -prevents order chatter near the threshold
//   -dead-band width should be calibrated to break-even transaction cost
// Regime-conditional weighting:
//   Trending = weight trend filter more heavily
//   MeanReverting= weight Kalman velocity more heavily
//   Noisy =reduce all weights, lower conviction
// TODO:
//   - on_bar()          : run DSP pipeline, combine signals, apply hysteresis
//   - combine_signals() : regime-conditional weighted sum of sub-signals
//   - apply_hysteresis(): Schmitt trigger logic
class AlphaGenerator {
public:
    struct Config {
        double threshold_high = 0.3;
        double threshold_low = -0.3;
        double kalman_weight = 0.5;
        double filter_weight = 0.3;
        double adaptive_weight = 0.2;
    };

    explicit AlphaGenerator(const Config& cfg);

    AlphaSignal on_bar(const core::OHLCVBar& bar, Regime regime); // TODO
    void        reset();

private:
    Config cfg_;
    double current_position_{0.0};

    // DSP components — construct with sensible defaults, tunable later
    dsp::KalmanFilter kalman_;
    dsp::IIRFilterBank trend_filter_;
    dsp::LMSAdaptiveFilter adaptive_;
    dsp::SpectralAnalyzer spectral_;

    double combine_signals(double kalman_sig, double filter_sig,
                           double adaptive_sig, Regime regime) const; // TODO
    double apply_hysteresis(double raw);                               // TODO
};

} // namespace dsp_trader::alpha
