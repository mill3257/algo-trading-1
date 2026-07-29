#pragma once
#include "core/ohlcv_aggregator.h"
#include "dsp/kalman_filter.h"
#include "dsp/iir_filter_bank.h"
#include "dsp/lms_adaptive_filter.h"
#include "dsp/spectral_analyzer.h"

namespace dsp_trader::alpha {

/*
Market regime:
Directional bias (Bullish, bearish, sideways)
paired with volatility character (quiet/volatile)

Detection inputs:
Direction: Kalman velocity or EMA slope (positive = bullish, negative = bearish, near zero = sideways)
Volatility: spectral entropy or realized variance from the Kalman filter
Low entropy or low variance = quiet, high entropy or high variance = volatile
*/

enum class Regime {
    BullishQuiet,
    BullishVolatile,
    BearishQuiet,
    BearishVolatile,
    SidewaysQuiet,
    SidewaysVolatile
};

inline bool is_bullish (Regime r)
{ return r == Regime::BullishQuiet || r == Regime::BullishVolatile; }

inline bool is_bearish (Regime r)
{ return r == Regime::BearishQuiet || r == Regime::BearishVolatile;}

inline bool is_sideways(Regime r)
{ return r == Regime::SidewaysQuiet || r == Regime::SidewaysVolatile; }

inline bool is_volatile(Regime r)
{ return r == Regime::BullishVolatile || r == Regime::BearishVolatile || r == Regime::SidewaysVolatile; }

inline bool is_quiet(Regime r)
{ return r == Regime::BearishQuiet || r == Regime::BullishQuiet || r == Regime::SidewaysQuiet; }

struct AlphaSignal {
    double raw; // combined signal value between -1 and 1 inclusive
    double position; // target position after hysteresis: {-1, 0, +1}
    Regime regime; //market microstructure
    double confidence; // proxy for signal quality (e.g. inverse Kalman variance)
    double ofi; //order flow imbalance in range [-1, 1]
    bool signal_changed; // true only when position flips (drives order submission)
};
/*
Combines DSP pipeline outputs into a scalar position signal.
Hysteresis thresholding (Schmitt trigger dead-band):
raw > threshold_high = long (+1)
raw < threshold_low = short (-1)
Threshold_low is negative (IMPORTANT)


Regime-conditional weighting:
    BullishQuiet = strong trend-filter weight, tight stop, full size
    BullishVolatile = trend-filter weight reduced, Kalman velocity confirms momentum
    BearishQuiet = mirror of BullishQuiet on short side
    BearishVolatile = mirror of BullishVolatile
    SidewaysQuiet = Kalman velocity / mean-reversion weight, full size
    SidewaysVolatile = all weights halved, OFI used as a tie breaker


Order-flow imbalance (OFI) is computed per bar from tick-level Lee-Ready
trade signing and added as a confirmatory signal weighted by ofi_weight

TODO:
- on_bar(): run DSP pipeline, accumulate OFI, combine, apply hysteresis
- combine_signals(): 6-way regime-conditional weighted sum
- apply_hysteresis(): Schmitt trigger logic
- accumulate_ofi() : per-tick Lee-Ready sign, summed over bar, normalized to [-1, 1]
*/
class AlphaGenerator {
public:
    struct Config {
        //hysteresis thresholds, these are initial guesses, will implement ML algorithm to determine optimized thresholds
        double threshold_high = 0.3;
        double threshold_low = -0.3;

        //regime conditional signal weights (sum to 1 within each regime)
        double kalman_weight = 0.5;
        double filter_weight = 0.3;
        double adaptive_weight = 0.1;
        double ofi_weight = 0.1;
    };

    explicit AlphaGenerator(const Config& cfg);

    //process one completed OHLCV bar, regime comes from RegimeDetector
    //Returns the combined alpha signal including OFI
    AlphaSignal on_bar(const core::OHLCVBar& bar, Regime regime); // TODO

    //feed a single raw tick for OFI accumulation (before on_bar fires)
    //Uses Lee-Ready rule:
    //  trade price >= ask --> buyer-initiated (+1)
    //  trade price <= bid --> seller-initiated (-1)
    void on_tick_ofi(double price, double bid, double ask, double size); //TODO

    void reset();

    const dsp::KalmanFilter& kalman() const { return kalman_; }

private:
    Config cfg_;
    double current_position_{0.0};

    //Intra-bar ofi accumulator
    double ofi_numerator_{0.0};
    double ofi_denominator_{0.0};

    // DSP components: construct with sensible defaults, will tune with ML later i.e. set_params()
    dsp::KalmanFilter kalman_;
    dsp::IIRFilterBank trend_filter_;
    dsp::LMSAdaptiveFilter adaptive_;
    dsp::SpectralAnalyzer spectral_;

    double combine_signals(double kalman_sig, double filter_sig, double adaptive_sig, double ofi_sig, Regime regime) const; // TODO

    //Schmitt trigger: compare raw against threshold_high and threshold_low directly
    //threshold_low is stored as negative already, do not try to negate it again
    double apply_hysteresis(double raw);  // TODO

    //normalize accumulated OFI to [-1, 1] and reset accumulators for next bar
    double flush_ofi();
};

}
