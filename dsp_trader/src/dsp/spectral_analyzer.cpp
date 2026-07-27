#include "dsp/spectral_analyzer.h"
#include <cmath>

namespace dsp_trader::dsp {

SpectralAnalyzer::SpectralAnalyzer(const Config& cfg) : cfg_(cfg) {
    if (cfg_.apply_hanning) {
        hann_win_.resize(cfg_.window_size);
        for (std::size_t i = 0; i < cfg_.window_size; ++i)
            hann_win_[i] = 0.5 * (1.0 - std::cos(
                2.0 * 3.14159265358979 * i / (cfg_.window_size - 1)));
    }
}

void SpectralAnalyzer::push(double price) {
    buffer_.push_back(price);
    if (buffer_.size() > cfg_.window_size)
        buffer_.erase(buffer_.begin()); // TODO: switch to proper circular buffer
}

std::optional<SpectralAnalyzer::SpectralFeatures> SpectralAnalyzer::compute() const {
    if (!window_full()) return std::nullopt;
    std::vector<double> psd;
    compute_dft(psd);   // TODO: swap for FFTW3 in Phase 2
    double entropy = shannon_entropy(psd);
    return SpectralFeatures{ 0.0, 0.0, entropy, 0.0, psd };
}

void SpectralAnalyzer::compute_dft(std::vector<double>& /*psd_out*/) const {
    // TODO: naive O(N^2) DFT for now
    // for k in 0..N/2:
    //   re = sum(w[n] * buf[n] * cos(2π*k*n/N))
    //   im = sum(w[n] * buf[n] * sin(2π*k*n/N))
    //   psd[k] = (re^2 + im^2) / N^2
}

double SpectralAnalyzer::shannon_entropy(const std::vector<double>& /*psd*/) const {
    // TODO: H = -sum(p_k * log2(p_k)) normalised to [0,1]
    return 0.5; // placeholder
}

} // namespace dsp_trader::dsp
