#pragma once

namespace dsp_trader::risk {

/// Simulates market impact and slippage for backtesting realism.
///
/// Phase 1: constant slippage model (slippage_bps applied to fill price)
/// Phase 3: sqrt-impact model or Almgren-Chriss for large orders
///
/// TODO (you implement):
///   - simulate_fill_price() : apply slippage to a market order
///                             Buy:  fill_price = market_price * (1 + slippage_bps/10000)
///                             Sell: fill_price = market_price * (1 - slippage_bps/10000)
class FillModel {
public:
    struct Config {
        double slippage_bps = 5.0;   // constant slippage per trade
        double commission_flat = 0.0;   // flat commission per order
    };

    explicit FillModel(const Config& cfg);

    double simulate_fill_price(double market_price, double qty, bool is_buy) const; // TODO
    double commission(double qty, double price) const; // TODO

private:
    Config cfg_;
};

} // namespace dsp_trader::risk
