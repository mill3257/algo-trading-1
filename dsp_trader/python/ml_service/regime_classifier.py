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

TODO (you implement):
    - extract_features(ohlcv_df) → feature dict
      Features to consider:
        spectral_entropy    (from FFT of close prices)
        dominant_period     (1 / peak frequency)
        hurst_exponent      (R/S analysis or DFA)
        realized_volatility (std of log returns over window)
        autocorrelation_lag1

    - RegimeClassifier.fit(X, y)    → train on labeled windows
    - RegimeClassifier.predict(X)   → return regime label(s)
    - RegimeClassifier.save/load    → persist model to disk
"""

from enum import Enum
from typing import Optional
import math


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


def extract_features(ohlcv_df) -> dict:
    """
    TODO: compute spectral and statistical features from a DataFrame
    with columns [open, high, low, close, volume].
    Returns a flat dict of scalar feature values.
    """
    raise NotImplementedError


class RegimeClassifier:
    """
    TODO: implement fit(), predict(), save(), load().
    Start with rule-based entropy thresholds; swap for sklearn/hmmlearn model later.
    """

    def __init__(self, entropy_low: float = 0.3, entropy_high: float = 0.7):
        self.entropy_low  = entropy_low
        self.entropy_high = entropy_high
        self._model = None  # sklearn or hmmlearn model placeholder

    def predict(self, features: dict) -> Regime:
        """Rule-based baseline — replace with trained model in Phase 4."""
        # TODO: use self._model.predict() if available
        entropy = features.get("spectral_entropy", 0.5)
        if entropy < self.entropy_low:
            return Regime.TRENDING
        elif entropy > self.entropy_high:
            return Regime.NOISY
        return Regime.MEAN_REVERTING

    def fit(self, X, y):
        """TODO: train classifier. X = feature matrix, y = regime labels."""
        raise NotImplementedError

    def save(self, path: str) -> None:
        """TODO: pickle or joblib dump."""
        raise NotImplementedError

    @classmethod
    def load(cls, path: str) -> "RegimeClassifier":
        """TODO: load from disk."""
        raise NotImplementedError
