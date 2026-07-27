#include "dsp/biquad_filter.h"

namespace dsp_trader::dsp {

// ── Factories ─────────────────────────────────────────────────────────────────
// TODO: implement bilinear-transform designs
BiquadCoeffs BiquadCoeffs::lowpass_butterworth(double /*wc*/)  { return {}; }
BiquadCoeffs BiquadCoeffs::highpass_butterworth(double /*wc*/) { return {}; }
BiquadCoeffs BiquadCoeffs::ema(double /*alpha*/)               { return {}; }

// ── BiquadFilter ──────────────────────────────────────────────────────────────
BiquadFilter::BiquadFilter(const BiquadCoeffs& c) : c_(c) {}

double BiquadFilter::process(double /*x*/) noexcept {
    // TODO: Direct Form II transposed update
    // y  = b0*x + w1_
    // w1 = b1*x - a1*y + w2_
    // w2 = b2*x - a2*y
    // return y
    return 0.0;
}

void BiquadFilter::initialize(double /*x0*/) noexcept {
    // TODO: pre-fill w1_, w2_ for steady-state at x0
}

void BiquadFilter::reset() noexcept {
    w1_ = w2_ = 0.0;
}

void BiquadFilter::set_coeffs(const BiquadCoeffs& c) {
    c_ = c;
    reset();
}

} // namespace dsp_trader::dsp
