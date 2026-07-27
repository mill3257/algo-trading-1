#pragma once
#include <vector>
#include <optional>
#include <cstddef>

namespace dsp_trader::dsp {

/// Sliding-window spectral analyzer for cycle detection and regime identification.
///
/// Phase 1: naive DFT stub (O(N^2)) — good enough for correctness testing
/// Phase 2: swap in FFTW3 (O(N log N)) when HAVE_FFTW3 is defined
///
/// Key outputs:
///   spectral_entropy  : Shannon entropy of the normalized PSD
///                       Low  → power concentrated at a few freqs → Trending
///                       High → power spread uniformly            → Noisy/MeanRev
///   dominant_period   : 1 / f_peak, in bar units
///   dc_power_fraction : fraction of power in the low-frequency band
///
/// TODO (you implement):
///   - push(price)     : append to circular window, evict oldest sample
///   - compute()       : run DFT/FFT on current window, return SpectralFeatures
///                       Apply Hanning window before transform to reduce leakage
///   - shannon_entropy : H = -sum(p * log2(p)) normalized to [0,1]
///
/// Phase 2 TODO: replace compute_dft() with fftw_plan + fftw_execute
class SpectralAnalyzer {
public:
    struct Config {
        std::size_t window_size   = 256;  // must be power of 2 for FFTW3
        double      sample_rate   = 1.0;  // bars per unit time
        bool        apply_hanning = true;
    };

    struct SpectralFeatures {
        double              dominant_period;    // bars
        double              dominant_power;     // normalized [0,1]
        double              spectral_entropy;   // [0,1]
        double              dc_power_fraction;  // fraction in low-freq band
        std::vector<double> psd;                // full power spectral density
    };

    explicit SpectralAnalyzer(const Config& cfg);

    void push(double price);

    // Returns nullopt if window not yet full
    std::optional<SpectralFeatures> compute() const;

    bool        window_full() const { return buffer_.size() >= cfg_.window_size; }
    std::size_t window_size() const { return cfg_.window_size; }

private:
    Config              cfg_;
    std::vector<double> buffer_;    // circular price window (TODO: replace with proper ring)
    std::vector<double> hann_win_;  // precomputed Hanning weights

    void   compute_dft(std::vector<double>& psd_out) const; // TODO: implement, swap for FFTW later
    double shannon_entropy(const std::vector<double>& psd) const; // TODO
};

} // namespace dsp_trader::dsp
