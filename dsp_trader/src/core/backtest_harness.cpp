#include "core/backtest_harness.h"
#include "core/csv_loader.h"
#include <cmath>
#include <cstdio>
#include <numeric>
#include <stdexcept>

namespace dsp_trader::core {

BacktestHarness::BacktestHarness(Config cfg)
    : cfg_(std::move(cfg))
    , cash_(cfg_.initial_capital)
    , peak_equity_(cfg_.initial_capital)
{}

BacktestResult BacktestHarness::run() {
    if (!pipeline_cb_)
        throw std::runtime_error("BacktestHarness: no pipeline callback set");

    CSVLoader loader({ cfg_.csv_path });
    OHLCVAggregator agg(cfg_.bar_duration_ns, [this](const OHLCVBar& bar) {
        // Mark-to-market equity before running the pipeline
        double equity = cash_ + position_ * bar.close;
        peak_equity_  = std::max(peak_equity_, equity);
        double dd     = (peak_equity_ - equity) / peak_equity_;
        max_drawdown_ = std::max(max_drawdown_, dd);
        equity_curve_.push_back(equity);

        pipeline_cb_(bar);
    });

    if (cfg_.verbose)
        std::printf("Loading: %s\n", cfg_.csv_path.c_str());

    loader.stream([&](const Tick& t) { agg.on_tick(t); });
    agg.flush();

    if (cfg_.verbose)
        std::printf("Bars processed: %zu | Trades: %d\n",
                    equity_curve_.size(), trade_count_);

    // ── Compute results ───────────────────────────────────────────────────
    BacktestResult r{};
    r.total_return = (cash_ - cfg_.initial_capital) / cfg_.initial_capital;
    r.max_drawdown = max_drawdown_;
    r.total_trades = trade_count_;

    // Sharpe and vol from equity curve (Phase 3: fill in)
    if (equity_curve_.size() > 1) {
        std::vector<double> rets;
        rets.reserve(equity_curve_.size() - 1);
        for (std::size_t i = 1; i < equity_curve_.size(); ++i)
            rets.push_back((equity_curve_[i] - equity_curve_[i-1])
                         / equity_curve_[i-1]);

        double mean = std::accumulate(rets.begin(), rets.end(), 0.0) / rets.size();
        double var  = 0.0;
        for (double x : rets) var += (x - mean) * (x - mean);
        var /= rets.size();
        double sigma = std::sqrt(var);

        // Annualise assuming 252 trading days × bars_per_day
        // Placeholder: assume 1-minute bars, 390 bars/day
        double ann_factor = std::sqrt(252.0 * 390.0);
        r.annualized_vol = sigma * ann_factor;
        r.sharpe_ratio   = (sigma > 0) ? (mean / sigma) * ann_factor : 0.0;
    }

    r.avg_pipeline_ns = 0.0; // populated by LatencyTracker in main

    return r;
}

void BacktestHarness::record_fill(double price, double qty, bool is_buy) {
    if (is_buy) {
        cash_     -= price * qty;
        position_ += qty;
    } else {
        cash_     += price * qty;
        position_ -= qty;
    }
    ++trade_count_;
}

} // namespace dsp_trader::core
