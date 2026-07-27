"""
Regime Classifier
Inputs:  spectral features extracted from a window of OHLCV bars
Outputs: regime label — one of {trending, mean_reverting, noisy}

Phase 3: rule-based baseline on spectral entropy
Phase 4: HMM or gradient-boosted tree trained on labeled windows

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


class Regime(str, Enum):
    TRENDING       = "trending"
    MEAN_REVERTING = "mean_reverting"
    NOISY          = "noisy"


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
