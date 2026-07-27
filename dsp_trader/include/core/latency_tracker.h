#pragma once
#include <chrono>
#include <array>
#include <cstdint>
#include <string_view>

namespace dsp_trader::core {

/// Nanosecond-resolution pipeline stage timer.
/// Call mark() at each stage boundary; call report() to print deltas.
/// In production (NDEBUG), mark() compiles to a single rdtsc-class read.
class LatencyTracker {
public:
    enum class Stage : int {
        TickIngested = 0,
        BarEmitted,
        FilterApplied,
        KalmanUpdated,
        AlphaComputed,
        RiskChecked,
        OrderSubmitted,
        kCount
    };

    void    mark(Stage s) noexcept;
    int64_t delta_ns(Stage from, Stage to) const noexcept;
    void    report() const;
    void    reset()  noexcept;

private:
    static constexpr int N = static_cast<int>(Stage::kCount);
    std::array<int64_t, N> stamps_{};
};

constexpr std::string_view stage_name(LatencyTracker::Stage s);

} // namespace dsp_trader::core
