#pragma once

namespace dsp_trader::risk {

/// Gates all position changes before they reach the execution engine.
/// Hard limits: position size, daily loss, drawdown circuit breaker.
///
/// TODO (you implement):
///   - evaluate()      : check proposed position change against all limits,
///                       return approved size (may be reduced) or rejection
///   - update_equity() : call after mark-to-market; trips circuit breaker
///                       if drawdown exceeds max_drawdown_pct
///   - record_fill()   : update daily PnL tracker after each fill
class RiskManager {
public:
    struct Config {
        double max_position_usd   = 10'000.0;
        double max_drawdown_pct   = 0.05;    // 5% → circuit breaker
        double max_daily_loss_usd = 500.0;
        double slippage_bps       = 5.0;     // used by fill model
    };

    struct Decision {
        bool   approved;
        double approved_qty;
        char   reason[64];
    };

    explicit RiskManager(const Config& cfg, double initial_capital);

    Decision evaluate(double current_qty, double target_qty,
                      double price, double portfolio_value) const;

    void update_equity(double equity);
    void record_fill(double pnl_delta);
    void reset_daily();

    bool circuit_breaker_tripped() const { return tripped_; }

private:
    Config cfg_;
    double peak_equity_;
    double daily_pnl_{0.0};
    bool   tripped_{false};
};

} // namespace dsp_trader::risk
