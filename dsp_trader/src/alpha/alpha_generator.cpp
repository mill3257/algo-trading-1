#include "alpha/alpha_generator.h"
#include <cmath>
#include <algorithm>

namespace dsp_trader::alpha {

AlphaGenerator::AlphaGenerator(const Config& cfg)
    : cfg_(cfg)
    , kalman_(dsp::KalmanFilter::Params{})
    , trend_filter_(dsp::IIRFilterBank::ema_cascade(0.1, 2))
    , adaptive_(dsp::LMSAdaptiveFilter::Config{8, 0.01, 0.0})
    , spectral_(dsp::SpectralAnalyzer::Config{64, 1.0, true})
{}

AlphaSignal AlphaGenerator::on_bar(const core::OHLCVBar& bar, Regime regime) {
    /*TODO: run each DSP component on bar.close, collect sub-signals, combine
    For now returns a neutral signal so the pipeline can run end-to-end
    */
    // auto kstate = kalman_.step(bar.close);
    // double filt = trend_filter_.process(bar.close);
    // spectral_.push(bar.close);
    // double adapt = adaptive_.process(bar.close, bar.close); //AR(1) prediction error

    //normalize subsignals to [-1, 1] relative to recent range/uncertainty
    // double ksig = kstate.velocity / (std::sqrt(kstate.var_velocity) + 1e-8);
    // double fsig = (filt - bar.close) / (std::abs(bar.close) * 0.01 + 1e-8);
    // double asig = -adapt; // negative error = overshot high = bearish lean
    // double ofi = flush_ofi();

    // double raw = combine_signals(ksig, fsig, asig, ofi, regime);
    //double pos = apply_hysteresis(raw);
    // bool changed = (pos != current_position_);
    // current_position_ = pos;
    // double confidence = 1.0 / (kstate.var_price + 1e-8);
    // return AlphaSignal{raw, pos, regime, confidence, ofi, changed};


    //placeholder: neutral signal so the full pipeline runs wo crashing
    double ofi = flush_ofi();
    return AlphaSignal{ 0.0, current_position_, regime, 0.0, false };
}

void AlphaGenerator::on_tick_ofi(double price, double bid, double ask, double size)
{
    //Lee-ready trade signing: compare trade price to midpoint
    //price >= ask --> buyer-initiated --> +size
    // price <= bid --> seller-initiated --> -size
    //between bid and ask --> ambiguous --> weight by position within spread

    double mid = (bid + ask) / 2.0;
    double spread = ask - bid;
    if (spread < 1e-8) return; //degenerate quote, skip

    double signed_vol;
    if (price >= ask)
        signed_vol = size;
    else if (price <= bid)
        signed_vol = -size;
    else //interpolate: maps [bid, ask] linearly to [-size, size]
        signed_vol = size * (2.0 * (price - mid) / spread);

    ofi_numerator_ += signed_vol;
    ofi_denominator_ += size;

}
double AlphaGenerator::flush_ofi()
{
    double ofi = 0.0;
    if (ofi_denominator_ > 1e-8) //clamp to [-1, 1], raw ratio is already in that range by construction
        ofi = std::clamp(ofi_numerator_ / ofi_denominator_, -1.0, 1.0);
    
    ofi_numerator_ = 0.0;
    ofi_denominator_ = 0.0;
    return ofi;
}

double AlphaGenerator::combine_signals(double kalman_sig, double filter_sig, double adaptive_sig, 
        double ofi_sig, Regime regime) const {
    //regime-determined weighted sum
    // hard-coded weights are placeholders
    //TODO in python ML service: tune weights via grid-search or Bayesian optimization on the backtest sharpe ratio
    
    double kw, fw, aw, ow; // kalman, filter, adaptive, and ofi weights

    switch(regime)
    {
        case Regime::BullishQuiet: //strong directional signal, lean on IIR trend filter
            kw = 0.30; fw = 0.45; aw = 0.15; ow = 0.10; break;
        case Regime::BullishVolatile: //direction confirmed but noisy, Kalman velocity is useful
            kw = 0.40; fw = 0.30; aw = 0.10; ow = 0.20; break;
        case Regime::BearishQuiet: //same logic as BullishQuiet
            kw = 0.30; fw = 0.45; aw = 0.15; ow = 0.10; break;
        case Regime::BearishVolatile: //same logic as BullishVolatile
            kw = 0.40; fw = 0.30; aw = 0.10; ow = 0.20; break;
        case Regime::SidewaysQuiet: //mean reversion strong, Kalman is the most useful
            kw = 0.50; fw = 0.20; aw = 0.20; ow = 0.10; break;
        case Regime::SidewaysVolatile: //noisy, reduce DSP weights, ofi helps distinguish real signal vs noise
            kw = 0.25; fw = 0.15; aw = 0.15; ow = 0.45; break;
    }

    double raw = kw * kalman_sig + fw * filter_sig + aw * adaptive_sig + ow * ofi_sig;
    return std::clamp(raw, -1.0, 1.0);
}

double AlphaGenerator::apply_hysteresis(double raw) {
    //Schmitt trigger logic
    if (raw > cfg_.threshold_high) current_position_ = 1.0;
    if (raw < cfg_.threshold_low) current_position_ = -1.0;
    //deadband: if raw is between two thresholds, hold current position
    return current_position_;
}

void AlphaGenerator::reset() {
    current_position_ = 0.0;
    ofi_numerator_ = 0.0;
    ofi_denominator_ = 0.0;
    trend_filter_.reset();
    adaptive_.reset();
}

}
