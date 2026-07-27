#pragma once
#include "core/ohlcv_aggregator.h"
#include <functional>
#include <string>
#include <vector>

namespace dsp_trader::core {

struct BacktestResult {
    double total_return;
    double sharpe_ratio;        // TODO Phase 3: implement from equity curve
    double max_drawdown;
    double annualized_vol;      // TODO Phase 3
    int    total_trades;
    double avg_pipeline_ns;     // mean tick-to-order latency
};

/// Drives the full pipeline from a CSV file.
/// Wiring: CSVLoader → OHLCVAggregator → [pipeline_cb] → fill recording
///
/// The pipeline_cb is where you plug in DSP → Alpha → Risk → Execution.
/// record_fill() is called by the paper trader on every simulated fill.
class BacktestHarness {
public:
    struct Config {
        std::string csv_path;
        int64_t     bar_duration_ns = 60'000'000'000LL; // 1-minute bars
        double      initial_capital = 100'000.0;
        bool        verbose         = false;
    };

    using PipelineCallback = std::function<void(const OHLCVBar&)>;

    explicit BacktestHarness(Config cfg);

    void set_pipeline(PipelineCallback cb) { pipeline_cb_ = std::move(cb); }

    BacktestResult run();

    void record_fill(double price, double qty, bool is_buy);

private:
    Config           cfg_;
    PipelineCallback pipeline_cb_;

    double cash_;
    double position_{0.0};
    double peak_equity_;
    double max_drawdown_{0.0};
    int    trade_count_{0};
    std::vector<double> equity_curve_; // for Sharpe / vol calculation
};

} // namespace dsp_trader::core
