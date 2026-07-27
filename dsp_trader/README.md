# DSP Trader

Automated trading algorithm built around DSP signal processing techniques,
targeting both quant and DSP/embedded SWE positions.

## Architecture

```
[CSV / WebSocket Feed]
        │
        ▼
[Tick Normalizer]          core::OHLCVAggregator
        │
        ▼
[DSP Signal Pipeline]      dsp::BiquadFilter / IIRFilterBank
   IIR filter bank              → trend extraction
   Kalman filter            dsp::KalmanFilter
       → price + velocity        → optimal state estimation
   LMS adaptive filter      dsp::LMSAdaptiveFilter
       → noise floor              → online AR / spread modeling
   Spectral analyzer        dsp::SpectralAnalyzer
       → cycle detection          → regime identification
        │
        ▼
[Alpha Generator]          alpha::AlphaGenerator
   Signal combiner               → regime-conditional weighting
   Hysteresis threshold          → Schmitt trigger / dead-band
        │
        ▼
[Risk Layer]               risk::RiskManager + FillModel
   Position limits
   Drawdown circuit breaker
        │
        ▼
[Execution Engine]         execution::PaperTrader
   Order FSM
   Paper trading (Alpaca)
```

### C++ / Python Split

```
[C++ Core — hot path]             [Python ML Service — cold path]
 Tick ingestion                    Regime classifier (HMM / GBT)
 DSP pipeline            ←params─  Kalman Q/R estimator (EM algorithm)
 Alpha + Risk                       Order flow analyzer
 Execution
```

IPC: ZeroMQ PUB/SUB — Python publishes `MLParams` every N minutes.
C++ swaps parameters atomically via `ipc::ParamChannel` (double buffer).

## Build

```bash
# Prerequisites
sudo apt install cmake pkg-config libfftw3-dev libzmq3-dev  # Ubuntu
brew install cmake fftw zeromq                               # macOS

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Generate sample data
python scripts/normalize_csv.py --output data/sample/spy_ticks.csv --source sample

# Run backtest
./dsp_trader ../data/sample/spy_ticks.csv

# Run tests
ctest --output-on-failure
```

## Implementation Roadmap

### Phase 1 — Foundation (Week 1-2)
- [ ] `core::RingBuffer`   — SPSC lock-free ring buffer
- [ ] `core::OHLCVAggregator` — tick → bar aggregation
- [ ] `core::CSVLoader`    — historical data feed
- [ ] `dsp::BiquadFilter`  — Direct Form II biquad (coefficients + process)
- [ ] `dsp::IIRFilterBank` — cascaded SOS filter bank
- [ ] `core::BacktestHarness` — drive full pipeline from CSV

### Phase 2 — DSP Core (Week 2-4)
- [ ] `dsp::KalmanFilter`  — predict / update / RTS smoother
- [ ] `dsp::LMSAdaptiveFilter` — LMS weight update + leaky variant
- [ ] `dsp::SpectralAnalyzer`  — Hanning window + DFT; swap for FFTW3
- [ ] `alpha::RegimeDetector`  — entropy thresholds → regime label

### Phase 3 — Alpha + Risk (Week 4-6)
- [ ] `alpha::AlphaGenerator`  — hysteresis + regime-conditional weighting
- [ ] `risk::RiskManager`      — position limits, drawdown circuit breaker
- [ ] `risk::FillModel`        — slippage simulation
- [ ] `execution::PaperTrader` — order FSM + simulated fills
- [ ] Sharpe ratio, max drawdown, turnover in `BacktestResult`

### Phase 4 — Live Integration (Week 6+)
- [ ] `ipc::ParamChannel`       — double-buffered parameter store
- [ ] `ipc::ZMQParamSubscriber` — ZMQ SUB socket + subscriber thread
- [ ] Python ML service          — regime classifier + Kalman EM estimator
- [ ] Alpaca paper trading API   — real HTTP orders
- [ ] Latency profiling          — per-stage `std::chrono` timestamps

## Key Design Decisions

| Decision | Rationale |
|---|---|
| SPSC ring buffer (no mutex) | Ingestion thread never blocks DSP thread |
| Biquad SOS (Direct Form II) | Numerically stable; coefficient sensitivity localized per section |
| Kalman Q/R via EM | Principled noise model estimation vs hand-tuning |
| Hysteresis threshold | Prevents order chatter near decision boundary (Schmitt trigger) |
| Spectral entropy → regime | Unsupervised; no labeled data needed for baseline |
| C++ hot path / Python cold path | Mirrors Jane Street / Two Sigma production architecture |
| Double-buffered param swap | Lock-free read on hot path; writer never stalls |

## Signal Model

```
S(t) = f(t) + η(t)
```
- `f(t)` — structured component: trend, cycles, momentum
- `η(t)` — noise: microstructure noise, random walk innovations

The DSP pipeline extracts `f(t)` from observed `S(t)` in real time
under low SNR and non-stationarity.
