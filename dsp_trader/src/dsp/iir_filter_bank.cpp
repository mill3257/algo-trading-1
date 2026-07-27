#include "dsp/iir_filter_bank.h"

namespace dsp_trader::dsp {

void IIRFilterBank::add_stage(const BiquadCoeffs& c) {
    stages_.emplace_back(c);
}

double IIRFilterBank::process(double x) noexcept {
    // TODO: chain x through each stage in order
    for (auto& s : stages_) x = s.process(x);
    return x;
}

void IIRFilterBank::reset() {
    for (auto& s : stages_) s.reset();
}

void IIRFilterBank::initialize(double x0) {
    for (auto& s : stages_) s.initialize(x0);
}

IIRFilterBank IIRFilterBank::butterworth_lowpass(int order, double cutoff_hz, double fs) {
    // TODO: compute per-section Q values from Butterworth pole angles,
    // call BiquadCoeffs::lowpass_butterworth() per section
    IIRFilterBank bank;
    double wc = 2.0 * 3.14159265358979 * cutoff_hz / fs;
    for (int i = 0; i < order / 2; ++i)
        bank.add_stage(BiquadCoeffs::lowpass_butterworth(wc));
    return bank;
}

IIRFilterBank IIRFilterBank::ema_cascade(double alpha, int n_stages) {
    IIRFilterBank bank;
    for (int i = 0; i < n_stages; ++i)
        bank.add_stage(BiquadCoeffs::ema(alpha));
    return bank;
}

} // namespace dsp_trader::dsp
