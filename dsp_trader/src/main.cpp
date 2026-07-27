#include "core/backtest_harness.h"
#include "core/latency_tracker.h"
#include "alpha/alpha_generator.h"
#include "alpha/regime_detector.h"
#include "risk/risk_manager.h"
#include "risk/fill_model.h"
#include "execution/paper_trader.h"
#include "ipc/param_channel.h"

#include <cstdio>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Pipeline wiring
//
// Data flow per bar:
//   OHLCVBar → AlphaGenerator (DSP pipeline inside) → RegimeDetector
//            → RiskManager → PaperTrader → fill recorded in BacktestHarness
//
// Latency is measured end-to-end across the pipeline stages.
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    const char* csv_path = (argc > 1) ? argv[1] : "data/sample/spy_ticks.csv";

    // ── Module construction ───────────────────────────────────────────────
    dsp_trader::risk::RiskManager::Config risk_cfg;
    dsp_trader::risk::RiskManager risk(risk_cfg, 100'000.0);

    dsp_trader::risk::FillModel::Config fill_cfg;
    dsp_trader::risk::FillModel fill_model(fill_cfg);

    dsp_trader::execution::PaperTrader::Config exec_cfg;
    exec_cfg.use_simulated_fills = true;
    dsp_trader::execution::PaperTrader trader(exec_cfg, risk, fill_model);

    dsp_trader::alpha::AlphaGenerator::Config alpha_cfg;
    dsp_trader::alpha::AlphaGenerator alpha(alpha_cfg);

    dsp_trader::alpha::RegimeDetector regime_det;

    dsp_trader::core::LatencyTracker latency;

    // Phase 4: uncomment to enable ZMQ parameter updates from Python
    // dsp_trader::ipc::ParamChannel param_ch;
    // dsp_trader::ipc::ZMQParamSubscriber zmq_sub(param_ch);
    // zmq_sub.start();

    // ── Backtest harness ──────────────────────────────────────────────────
    dsp_trader::core::BacktestHarness::Config bt_cfg;
    bt_cfg.csv_path        = csv_path;
    bt_cfg.bar_duration_ns = 60'000'000'000LL; // 1-minute bars
    bt_cfg.initial_capital = 100'000.0;
    bt_cfg.verbose         = true;

    dsp_trader::core::BacktestHarness harness(bt_cfg);

    // ── Pipeline callback (runs once per completed OHLCV bar) ─────────────
    harness.set_pipeline([&](const dsp_trader::core::OHLCVBar& bar) {
        using S = dsp_trader::core::LatencyTracker::Stage;
        latency.reset();
        latency.mark(S::BarEmitted);

        // TODO: optionally pull fresh params from param_ch here
        // auto params = param_ch.read();
        // alpha.kalman_.set_params(params.kalman);
        // regime_det.set_override(params.regime);

        latency.mark(S::FilterApplied);
        auto sig = alpha.on_bar(bar, regime_det.current_regime());
        latency.mark(S::KalmanUpdated);
        latency.mark(S::AlphaComputed);

        auto decision = risk.evaluate(
            trader.get_position("SPY"),
            sig.position * 10.0,  // 10-share unit size
            bar.close,
            trader.portfolio_value(bar.close)
        );
        latency.mark(S::RiskChecked);

        if (sig.signal_changed && decision.approved) {
            dsp_trader::execution::Order order{};
            std::strncpy(order.symbol, "SPY", 7);
            order.type        = dsp_trader::execution::OType::Market;
            order.side        = (sig.position > 0)
                              ? dsp_trader::execution::Side::Buy
                              : dsp_trader::execution::Side::Sell;
            order.qty         = decision.approved_qty;
            order.limit_price = bar.close;

            trader.submit(order);
            latency.mark(S::OrderSubmitted);
        }

        if (bt_cfg.verbose) latency.report();
    });

    // ── Run ───────────────────────────────────────────────────────────────
    std::printf("=== DSP Trader Backtest — %s ===\n\n", csv_path);
    auto result = harness.run();

    std::printf("\n=== Results ===\n");
    std::printf("  Total Return : %.2f%%\n", result.total_return * 100.0);
    std::printf("  Max Drawdown : %.2f%%\n", result.max_drawdown  * 100.0);
    std::printf("  Total Trades : %d\n",     result.total_trades);

    return 0;
}
