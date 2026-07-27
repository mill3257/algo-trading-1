#pragma once
#include "core/tick.h"
#include <cstdint>

namespace dsp_trader::execution {

using OrderId = uint64_t;

enum class Side  { Buy, Sell };
enum class OType { Market, Limit };

/// Order lifecycle states — enforced by PaperTrader as an explicit FSM.
/// Transitions: New → Submitted → Filled
///                             → Cancelled
///                             → Rejected
/// PartialFill is deferred to Phase 4 (live trading needs it; backtest doesn't).
enum class OrderState { New, Submitted, Filled, Cancelled, Rejected };

struct Order {
    OrderId    id{0};
    char       symbol[8]{};
    Side       side{Side::Buy};
    OType      type{OType::Market};
    OrderState state{OrderState::New};
    double     qty{0.0};
    double     limit_price{0.0};   // only used for Limit orders
    double     filled_qty{0.0};
    double     avg_fill_price{0.0};
    core::Timestamp submitted_ns{0};
    core::Timestamp filled_ns{0};

    bool is_terminal() const {
        return state == OrderState::Filled    ||
               state == OrderState::Cancelled ||
               state == OrderState::Rejected;
    }
};

} // namespace dsp_trader::execution
