#include "execution/paper_trader.h"
#include <chrono>
#include <cstring>
#include <stdexcept>

namespace dsp_trader::execution {

PaperTrader::PaperTrader(Config cfg, risk::RiskManager& risk,
                         const risk::FillModel& fill_model)
    : cfg_(std::move(cfg))
    , risk_(risk)
    , fill_model_(fill_model)
    , cash_(cfg_.initial_cash)
{}

OrderId PaperTrader::submit(Order order) {
    order.id           = next_id_++;
    order.state        = OrderState::Submitted;
    order.submitted_ns = std::chrono::steady_clock::now()
                             .time_since_epoch().count();

    if (cfg_.use_simulated_fills) {
        simulate_fill(order, order.limit_price);
    } else {
        submit_to_alpaca(order); // Phase 4
    }

    orders_[order.id] = order;
    return order.id;
}

bool PaperTrader::cancel(OrderId id) {
    auto it = orders_.find(id);
    if (it == orders_.end())    return false;
    if (it->second.is_terminal()) return false;
    it->second.state = OrderState::Cancelled;
    return true;
}

void PaperTrader::simulate_fill(Order& order, double market_price) {
    bool is_buy      = (order.side == Side::Buy);
    double fill_price = fill_model_.simulate_fill_price(market_price, order.qty, is_buy);
    double comm       = fill_model_.commission(order.qty, fill_price);

    order.filled_qty      = order.qty;
    order.avg_fill_price  = fill_price;
    order.state           = OrderState::Filled;
    order.filled_ns       = std::chrono::steady_clock::now()
                                .time_since_epoch().count();

    double trade_value = fill_price * order.qty;
    double pnl_delta   = 0.0;

    if (is_buy) {
        cash_      -= trade_value + comm;
        position_  += order.qty;
        pnl_delta   = -(trade_value + comm); // cost
    } else {
        cash_      += trade_value - comm;
        position_  -= order.qty;
        pnl_delta   = trade_value - comm;    // proceeds
    }

    risk_.record_fill(pnl_delta);
    risk_.update_equity(portfolio_value(fill_price));

    if (fill_cb_) fill_cb_(order);
}

bool PaperTrader::submit_to_alpaca(Order& /*order*/) {
    // Phase 4: HTTP POST to Alpaca REST API
    // Requires libcurl or cpp-httplib
    return false;
}

const Order* PaperTrader::get_order(OrderId id) const {
    auto it = orders_.find(id);
    return it != orders_.end() ? &it->second : nullptr;
}

double PaperTrader::portfolio_value(double current_price) const {
    return cash_ + position_ * current_price;
}

} // namespace dsp_trader::execution
