#include "risk/fill_model.h"

namespace dsp_trader::risk {

FillModel::FillModel(const Config& cfg) : cfg_(cfg) {}

double FillModel::simulate_fill_price(double market_price, double /*qty*/, bool is_buy) const {
    double slip = cfg_.slippage_bps * 1e-4; // basis points → fraction
    return is_buy ? market_price * (1.0 + slip)
                  : market_price * (1.0 - slip);
}

double FillModel::commission(double qty, double /*price*/) const {
    return cfg_.commission_flat * qty;
}

} // namespace dsp_trader::risk
