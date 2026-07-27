#pragma once
#include "dsp/biquad_filter.h"
#include <vector>

namespace dsp_trader::dsp {

/// Cascaded biquad filter bank (second-order sections / SOS).
/// Higher-order IIR filters are implemented as a cascade of biquads to
/// avoid the coefficient sensitivity of a single high-order direct form.
///
/// TODO (you implement):
///   - process(x)        : chain input through all stages in order
///   - reset()           : reset all stages
///   - initialize(x0)    : initialize all stages for steady-state at x0
///
/// Factory methods (you implement):
///   - butterworth_lowpass(order, cutoff_hz, fs) : N-th order LP as SOS
///     Hint: for proper pole placement, each biquad section has a different Q.
///     Look up the Butterworth pole angles: θ_k = π(2k+N-1)/(2N), k=1..N/2
///   - ema_cascade(alpha, stages) : multiple EMA stages in series
class IIRFilterBank {
public:
    void add_stage(const BiquadCoeffs& c);

    double process(double x) noexcept;   // TODO
    void   reset();                       // TODO
    void   initialize(double x0);        // TODO

    std::size_t num_stages() const { return stages_.size(); }
    std::size_t order()      const { return stages_.size() * 2; }

    static IIRFilterBank butterworth_lowpass(int order, double cutoff_hz, double fs);
    static IIRFilterBank ema_cascade(double alpha, int stages);

private:
    std::vector<BiquadFilter> stages_;
};

} // namespace dsp_trader::dsp
