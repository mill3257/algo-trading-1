"""
Regime Classifier
Inputs: spectral and microstructure features from a window of OHLCV bars
Outputs: regime label, same 6 as in C++ but named different, ex: bullish_quiet

Phase 3: rule-based baseline using kalman vel and spectral entropy thresholds
Phase 4: HMM or gradient-boosted tree on full feature set

OFI: true obi requires bid/ask size data not available in the current csv method
We implement Lee-Ready trade signing using bid/ask prices
    ofi_bar = sum(signed_volume) / total volume --> [-1, 1]

    this is a well-established alternative used in academic market microstructure literature
    it is computable from existing Tick struct in C++ without adding bid size or ask size columns

TODO:
    - extract_features(ohlcv_df) --> feature dict
      Features to consider:
        spectral_entropy (from FFT of close prices)
        dominant_period (1 / peak frequency)
        hurst_exponent (R/S analysis or DFA)
        realized_volatility (std of log returns over window)
        autocorrelation_lag1

    - RegimeClassifier.fit(X, y) --> train on labeled windows
    - RegimeClassifier.predict(X) --> return regime label(s)
    - RegimeClassifier.save/load --> persist model to disk
"""

from enum import Enum
from typing import Optional
import math
import numpy as np
from sklearn.ensemble import GradientBoostingClassifier
import joblib

class Regime(str, Enum):
    BULLISH_QUIET = "bullish_quiet"
    BULLISH_VOLATILE = "bullish_volatile"
    BEARISH_QUIET = "bearish_quiet"
    BEARISH_VOLATILE = "bearish_volatile"
    SIDEWAYS_QUIET = "sideways_quiet"
    SIDEWAYS_VOLATILE = "sideways_volatile"

    @property
    def is_bullish(self) -> bool: return self in (self.BULLISH_QUIET, self.BULLISH_VOLATILE)
    @property
    def is_bearish(self) -> bool: return self in (self.BEARISH_QUIET, self.BEARISH_VOLATILE)
    @property
    def is_sideways(self) -> bool: return self in (self.SIDEWAYS_QUIET, self.SIDEWAYS_VOLATILE)
    @property
    def is_volatile(self) -> bool: return self in (self.BULLISH_VOLATILE, self.BEARISH_VOLATILE, self.SIDEWAYS_VOLATILE)
    @property(self)
    def is_quiet(self) -> bool: return not self.is_volatile

def lee_ready_ofi(prices, bids, asks, sizes) ->float:
    """
    compute lee-ready ofi over a sequence of ticks
    returns a value in [-1, 1], positive --> net buying pressure
    
    the C++ "on_tick_ofi()" calculates ofi for one singular tick, this calculates it for a list of ticks
    """
    numerator = 0.0
    denominator = 0.0
    prev_price = None

    for price, bid, ask, size in zip(prices, bids, asks, sizes):
        spread = ask- bid
        mid = (bid + ask) / 2

        if spread > 1e-8:
            # quote based signing
            if price >= ask:
                signed_vol = size
            elif price <= bid:
                signed_vol = -size
            else:
                #fractional position within spread + linear interpolation
                signed_vol = size * 2.0 * (price - mid) / spread
        else:
            #tick test fallback
            if prev_price is None or price == prev_price:
                signed_vol = 0.0
            else:
                signed_vol = size if price > prev_price else -size

        numerator += signed_vol
        denominator += size
        prev_price = price

    return numerator / denominator if denominator > 1e-8 else 0.0

def _hurst_exponent(prices:list, max_lag: int = 20) ->float:
    """
    Estimate hurst exponent via r/s analysis
    If H > .5 --> trending, H = .5 --> random walk, H < .5 --> mean-reverting
    """
    if len(prices) < max_lag * 2:
        return -.5 #insufficient data

    rs_values = []
    lags = range(4, max_lag + 1)
    for lag in lags:
        chunks = [prices[i:i+lag] for i in range(0, len(prices) - lag, lag)]
        if not chunks:
            continue
        rs_chunk = []
        for chunk in chunks:
            mean_c = sum(chunk) / len(chunk)
            devs = [c - mean_c for c in chunk]
            cumdevs = [sum(devs[:i+1]) for i in range(len(devs))]
            r = max(cumdevs) - min(cumdevs)
            s = math.sqrt(sum(d**2 for d in devs) / len(devs)) if len(devs) > 1 else 1e-8
            rs_chunk.append(r / s if s > 1e-8 else 0.0)
        if rs_chunk:
            rs_values.append((math.log(lag), math.log(sum(rs_chunk)/len(rs_chunk) + 1e-8)))

    if len(rs_values) < 2:
        return 0.5

    #linear regression, H = slope of log(r/s) vs log(lag)
    n = len(rs_values)
    sx = sum(x for x, _ in rs_values)
    sy = sum (y for _, y in rs_values)
    sxy = sum(x*y for x, y in rs_values)
    sxx = sum(x*x for x, _ in rs_values)
    denom = n * sxx - sx*sx
    return (n * sxy - sx*sy) / denom if abs(denom) > 1e-8 else 0.5

def extract_features(ohlcv_df, tick_df = None) -> dict:
    """
    extract spectral and microstructure features from ohlcv history
    ohlcv_df: dataframe with columns [open, high, low, close, volume]
    tick_df: optional dataframe with columns [price, bid, ask, size] for ofi calculation. If none, ofi is set to 0
    returns a flat dict of scalar feature values ready for a classifier
    spectral_entropy: normalized shannon entropy of FFT PSD, proxy for market disorder. trending-->low, noisy-->high
    nominant_period: 1/f_peak in bar units, strongest price cycle
    realized_volatility: std of log returns over window, annualized
    hurst_exponent: R/S estimate, >.5 treading, <.5 mean reverting
    autocorr_larg1 : lag-1 autocorrelation of log returns
    ema_slope: slope of 10-bar EMA normalized by price level
    kalman_velocity: best supplied externally from the C++ kalman filter. When unavailable, approximated as ema slope
    ofi: lee-ready ofi in [-1,1], positive --> net buying pressure
    obi: true order-book imbalance, set to none if level-2 data is unavailable
    """
    closes = list(ohlcv_df["close"])
    n = len(closes)
    if n < 4:
        raise ValueError("Need at least 4 bars to extract features")

    #log returns
    log_rets = [math.log(closes[i]/closes[i-1]) for i in range(1, n) if closes[i-1] > 0]

    #realized volatility (annualized, assuming 1-min bars, 390 bars per day)
    mean_r = sum(log_rets) / len(log_rets) if log_rets else 0.0
    var_r = sum((r - mean_r)**2 for r in log_rets) / max(len(log_rets)-1, 1)
    realized_vol = math.sqrt(var_r * 252 * 390)

    #lag 1 autocorrelation
    autocorr_lag1 = 0.0
    if len(log_rets) > 2:
        r1 = log_rets[:-1]
        r2 = log_rets[1:]
        m1, m2 = sum(r1)/len(r1), sum(r2)/len(r2)
        num = sum((a-m1)*(b-m2) for a,b in zip(r1,r2))
        den = math.sqrt(sum((a-m1)**2 for a in r1)* sum((b-m2)**2) for b in r2)
        autocorr_lag1 = num/den if den > 1e-8 else 0.0

    #ema slope (10-bar) normalized to price level
    ema_period = min(10, n)
    alpha_ema = 2.0 / (ema_period + 1)
    ema = closes[0]
    for c in closes[1:]:
        ema = alpha_ema * c + (1-alpha_ema) * ema
    #slope approximation
    start_idx = max(0, n - ema_period)
    ema_slope = (closes[-1] - closes[start_idx]) / (ema_period * closes[-1] + 1e-8)

    #hurst exponent
    hurst = _hurst_exponent(closes)

    #spectral entropy - FFT
    fft_vals = np.abs(np.fft.rfft(closes - np.mean(closes)))**2
    fft_vals /= (fft_vals.sum() + 1e-12) #normalize
    spectral_entropy = float(-np.sum(fft_vals * np.log2(fft_vals + 1e-12)) / math.log2(len(fft_vals)))
    dominant_period = float(len(closes)) / (np.argmax(fft_vals[1:]) + 1)

    #ofi from tick data
    ofi = 0.0
    if tick_df is not None and len(tick_df) > 0:
        ofi = lee_ready_ofi(
            tick_df["price"].tolist(),
            tick_df["bid"].tolist(),
            tick_df["ask"].tolist(),
            tick_df["size"].tolist()
        )

    #true obi, only available with level 2 data
    obi: Optional[float] = None
    if tick_df is not None and "bid_size" in tick_df.columns and "ask_size" i tick_df.columns:
        total_bid = tick_df["bid_size"].sum()
        total_ask = tick_df["ask_size"].sum()
        denom = total_ask + total_ask
        obi = (total_bid - total_ask) / denom if denom > 1e-8 else 0.0

    return {
        "spectral_entropy" : spectral_entropy,
        "dominant_period": dominant_period,
        "realized_volatility": realized_vol,
        "hurst_exponent": hurst,
        "autocorr_lag1": autocorr_lag1,
        "ema_slope": ema_slope,
        "kalman_velocity": ema_slope, #substitute, replace with C++ value via zmq
        "ofi": ofi,
        "obi": obi
    }

class RegimeClassifier:
    """
    Classifies market regime into one of six classes
    Direction from ema slope and kalman velocity
    Volatility from spectral entropy
    """

    def __init__(self, velocity_thresh: float = 0.0002, entropy_thresh: float = 0.45):
        self.entropy_low  = velocity_thresh
        self.entropy_high = entropy_thresh
        self._model = None  # sklearn or hmmlearn model placeholder

    def predict(self, features: dict) -> Regime:
        #return regime from features dict produced by extract_features()
        entropy = features.get("spectral_entropy", 0.5)
        if self._model is not None:
            return self._predict_ml(features)
        return self._predict_rules(features)

    def _predict_rules(self, features: dict) ->Regime:
        entropy = features.get("spectral_entropy", 0.5)
        velocity = features.get("kalman_velocity", 0.0)
        ofi = features.get("ofi", 0.0)

        #direction axis
        if velocity > self.velocity_thresh:
            direction = "bullish"
        elif velocity < -self.velocity_thresh:
            direction = "bearish"
        else:
            #ofi as a tiebreaker within the sideways band
            if ofi > 0.3: direction = "bullish"
            elif ofi < -0.3: direction = "bearish"
            else: direction = "sideways"

        #volatility axis
        suffix = "volatile" if entropy >= self.entropy_thresh else "quiet"
        return Regime(f"{direction}_{suffix}")

    def _predict_ml(self, features: dict) -> Regime:
        feature_vector = [
            features.get("spectral_entropy", 0.5),
            features.get("dominant_period", 32.0), 
            features.get("realized_volatility", 0.15),
            features.get("hurst_exponent", 0.5),
            features.get("autocorr_lag1", 0.0),
            features.get("ema_slope", 0.0),
            features.get("kalman_velocity", 0.0),
            features.get("ofi", 0.0)
        ]
        label = self._model.predict([feature_vector])[0]
        return Regime(label)

    def fit(self, X, y):
        """
        Train a gradient-boosted classifier on labelled windows
        X: list of feature dicts (from extract_features) or 2D array
        y: list of regime strings matching the regime enum values
        """
        if isinstance(X[0], dict):
            feature_keys = ["spectral_entropy", "dominant_period", "realized_volatility",
                "hurst_exponent", "autocorr_lag1", "ema_slope",
                "kalman_velocity", "ofi"]

            X = [[row.get(k, 0.0) for k in feature_keys] for row in X]

        labels = [r.value if isinstance(r, Regime) else r for r in y]
        self._model = GradientBoostingClassifier(n_estimators = 200, max_depth=4, learning_rate=0.05)
        self._model.fit(X, labels)

    def save(self, path: str) -> None:
        #persist the trained model to disk using joblib
        joblib.dump(self._model, path)

    @classmethod
    def load(cls, path: str) -> "RegimeClassifier":
        #load a previously saved classifier
        obj = cls()
        obj._model = joblib.load(path)
        return obj
