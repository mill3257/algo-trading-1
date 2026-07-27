#include "alpha/regime_detector.h"

namespace dsp_trader::alpha {

RegimeDetector::RegimeDetector() : cfg_({}) {}
RegimeDetector::RegimeDetector(const Config& cfg) : cfg_(cfg) {}

Regime RegimeDetector::update(const dsp::SpectralAnalyzer::SpectralFeatures& f) {
    if (use_override_) { current_ = override_; return current_; }
    ++bars_seen_;
    if (bars_seen_ < cfg_.min_bars) return current_;
    // TODO: use actual spectral features once SpectralAnalyzer is implemented
    if      (f.spectral_entropy < cfg_.entropy_low)  current_ = Regime::Trending;
    else if (f.spectral_entropy > cfg_.entropy_high) current_ = Regime::Noisy;
    else                                             current_ = Regime::MeanReverting;
    return current_;
}

} // namespace dsp_trader::alpha
