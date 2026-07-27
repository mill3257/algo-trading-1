#pragma once

namespace dsp_trader::dsp {

/// Biquad coefficients in Direct Form II (transposed).
/// H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
///
/// Factory methods give you common designs without hand-computing coefficients.
/// All use bilinear transform from analog prototypes; wc is in radians/sample.
///
/// TODO (you implement factories):
///   - lowpass_butterworth(wc)  : 2nd-order Butterworth LP via bilinear transform
///   - highpass_butterworth(wc) : same, HP prototype
///   - ema(alpha)               : EMA as degenerate IIR (b0=alpha, a1=-(1-alpha), rest 0)
///   - bandpass(wc, Q)          : optional, useful for cycle isolation
struct BiquadCoeffs {
    double b0{1}, b1{0}, b2{0}; // numerator
    double a1{0}, a2{0};        // denominator (a0 normalized to 1)

    static BiquadCoeffs lowpass_butterworth(double wc);
    static BiquadCoeffs highpass_butterworth(double wc);
    static BiquadCoeffs ema(double alpha);
};

/// Single biquad section. O(1) per sample, no branches.
///
/// TODO (you implement):
///   - process(x)    : Direct Form II update, return output sample
///   - initialize(x0): pre-fill delay state for steady-state at x0
///                     (prevents cold-start transient from generating false signals)
///   - reset()       : zero the delay line
class BiquadFilter {
public:
    explicit BiquadFilter(const BiquadCoeffs& c);

    double process(double x) noexcept;    // TODO
    void   initialize(double x0) noexcept; // TODO
    void   reset() noexcept;              // TODO

    void                set_coeffs(const BiquadCoeffs& c);
    const BiquadCoeffs& coeffs() const { return c_; }

private:
    BiquadCoeffs c_;
    double w1_{0.0}, w2_{0.0}; // DF2 delay elements
};

} // namespace dsp_trader::dsp
