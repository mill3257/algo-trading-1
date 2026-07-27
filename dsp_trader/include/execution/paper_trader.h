#pragma once
#include "execution/order.h"
#include "risk/risk_manager.h"
#include "risk/fill_model.h"
#include <functional>
#include <unordered_map>
#include <string>

namespace dsp_trader::execution {

/// Paper trading engine.
///
/// Phase 1-3: simulates fills instantly against close price via FillModel.
/// Phase 4:   forwards orders to Alpaca paper trading REST API.
///            Enable by setting use_simulated_fills = false and providing API keys
///            via environment variables (never hardcode).
///
/// TODO (you implement):
///   - submit()        : run risk check, assign id, simulate or forward fill
///   - cancel()        : cancel a pending order (no-op if already terminal)
///   - simulate_fill() : apply FillModel slippage, update cash/position,
///                       invoke fill_cb_, update risk manager
///   - portfolio_value(): cash + position * current_price
class PaperTrader {
public:
    struct Config {
        bool        use_simulated_fills = true;
        std::string alpaca_key;     // read from env: ALPACA_API_KEY
        std::string alpaca_secret;  // read from env: ALPACA_API_SECRET
        std::string base_url = "https://paper-api.alpaca.markets";
        double      initial_cash = 100'000.0;
    };

    using FillCallback = std::function<void(const Order&)>;

    explicit PaperTrader(Config cfg, risk::RiskManager& risk,
                         const risk::FillModel& fill_model);

    OrderId       submit(Order order);             // TODO
    bool          cancel(OrderId id);              // TODO

    void          on_fill(FillCallback cb)         { fill_cb_ = std::move(cb); }

    const Order*  get_order(OrderId id)      const;
    double        get_position(const char* symbol) const { return position_; }
    double        get_cash()                 const { return cash_; }
    double        portfolio_value(double current_price) const;

private:
    Config         cfg_;
    risk::RiskManager&    risk_;
    const risk::FillModel& fill_model_;
    FillCallback   fill_cb_;

    std::unordered_map<OrderId, Order> orders_;
    OrderId next_id_{1};
    double  cash_;
    double  position_{0.0};

    void simulate_fill(Order& order, double market_price); // TODO
    bool submit_to_alpaca(Order& order);                   // Phase 4 TODO
};

} // namespace dsp_trader::execution
