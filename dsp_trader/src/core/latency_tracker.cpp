#include "core/latency_tracker.h"
#include <cstdio>
#include <algorithm>

namespace dsp_trader::core {

void LatencyTracker::mark(Stage s) noexcept {
    stamps_[static_cast<int>(s)] =
        std::chrono::steady_clock::now().time_since_epoch().count();
}

int64_t LatencyTracker::delta_ns(Stage from, Stage to) const noexcept {
    int64_t a = stamps_[static_cast<int>(from)];
    int64_t b = stamps_[static_cast<int>(to)];
    if (a == 0 || b == 0) return -1;
    return b - a;
}

void LatencyTracker::report() const {
    using S = Stage;
    struct Pair { S from; S to; };
    constexpr Pair pairs[] = {
        { S::BarEmitted,    S::FilterApplied  },
        { S::FilterApplied, S::KalmanUpdated  },
        { S::KalmanUpdated, S::AlphaComputed  },
        { S::AlphaComputed, S::RiskChecked    },
        { S::RiskChecked,   S::OrderSubmitted },
        { S::BarEmitted,    S::OrderSubmitted }, // total
    };
    for (auto& p : pairs) {
        int64_t d = delta_ns(p.from, p.to);
        if (d >= 0)
            std::printf("  %-18s -> %-18s : %6lld ns\n",
                stage_name(p.from).data(),
                stage_name(p.to).data(),
                static_cast<long long>(d));
    }
}

void LatencyTracker::reset() noexcept {
    stamps_.fill(0);
}

// stage_name must be defined here (constexpr declared in header)
constexpr std::string_view stage_name(LatencyTracker::Stage s) {
    using S = LatencyTracker::Stage;
    switch (s) {
        case S::TickIngested:   return "TickIngested";
        case S::BarEmitted:     return "BarEmitted";
        case S::FilterApplied:  return "FilterApplied";
        case S::KalmanUpdated:  return "KalmanUpdated";
        case S::AlphaComputed:  return "AlphaComputed";
        case S::RiskChecked:    return "RiskChecked";
        case S::OrderSubmitted: return "OrderSubmitted";
        default:                return "Unknown";
    }
}

} // namespace dsp_trader::core
