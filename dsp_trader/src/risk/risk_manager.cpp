#include "risk/risk_manager.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cmath>

namespace dsp_trader::risk {

RiskManager::RiskManager(const Config& cfg, double initial_capital)
    : cfg_(cfg), peak_equity_(initial_capital) {}

RiskManager::Decision RiskManager::evaluate(
        double current_qty, double target_qty,
        double price, double portfolio_value) const {

    Decision d{};
    d.reason[0] = '\0';

    if (tripped_) {
        d.approved     = false;
        d.approved_qty = 0.0;
        std::snprintf(d.reason, sizeof(d.reason), "Circuit breaker tripped");
        return d;
    }

    if (daily_pnl_ < -cfg_.max_daily_loss_usd) {
        d.approved     = false;
        d.approved_qty = 0.0;
        std::snprintf(d.reason, sizeof(d.reason),
            "Daily loss limit $%.0f exceeded", cfg_.max_daily_loss_usd);
        return d;
    }

    // Compute raw desired change
    double delta_qty = target_qty - current_qty;
    double target_value = std::abs(target_qty) * price;

    d.approved     = true;
    d.approved_qty = std::abs(delta_qty);

    // Cap position value
    if (target_value > cfg_.max_position_usd) {
        d.approved_qty = (cfg_.max_position_usd / (price + 1e-12))
                       - std::abs(current_qty);
        d.approved_qty = std::max(0.0, d.approved_qty);
        std::snprintf(d.reason, sizeof(d.reason),
            "Position capped at $%.0f", cfg_.max_position_usd);
    }

    if (d.approved_qty < 1e-9) {
        d.approved     = false;
        d.approved_qty = 0.0;
    }

    return d;
}

void RiskManager::update_equity(double equity) {
    peak_equity_ = std::max(peak_equity_, equity);
    double drawdown = (peak_equity_ - equity) / (peak_equity_ + 1e-12);
    if (drawdown > cfg_.max_drawdown_pct)
        tripped_ = true;
}

void RiskManager::record_fill(double pnl_delta) {
    daily_pnl_ += pnl_delta;
}

void RiskManager::reset_daily() {
    daily_pnl_ = 0.0;
}

} // namespace dsp_trader::risk
