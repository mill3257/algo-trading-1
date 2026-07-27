"""
ML Parameter Service
Runs on the cold path, publishes updated Kalman Q/R and regime labels
to the C++ core over ZMQ every N minutes.

Usage:
    python -m ml_service.main --interval 300 --endpoint tcp://*:5555

Dependencies:
    pip install pyzmq numpy scikit-learn hmmlearn
"""

import argparse
import json
import time
import logging

# import zmq           # uncomment in Phase 4
# import numpy as np
# from .regime_classifier import RegimeClassifier
# from .kalman_em import KalmanEMEstimator

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(message)s")
log = logging.getLogger(__name__)


def build_param_message(kalman_params: dict, regime: str, lms_mu: float, seq: int) -> dict:
    """Serialize MLParams to JSON for ZMQ transmission."""
    return {
        "kalman": kalman_params,
        "regime": regime,
        "lms_mu": lms_mu,
        "seq": seq,
    }


def run(interval_sec: int, endpoint: str) -> None:
    log.info(f"ML service starting — endpoint={endpoint}, interval={interval_sec}s")

    # TODO Phase 4: initialize ZMQ context and PUB socket
    # ctx  = zmq.Context()
    # sock = ctx.socket(zmq.PUB)
    # sock.bind(endpoint)

    # TODO: initialize classifier and estimator
    # classifier = RegimeClassifier.load("models/regime_clf.pkl")
    # estimator  = KalmanEMEstimator()

    seq = 0
    while True:
        # TODO: pull recent OHLCV from shared state or a queue
        # ohlcv = fetch_recent_bars(n=200)

        # TODO: extract spectral features
        # features = extract_spectral_features(ohlcv)

        # TODO: classify regime
        # regime = classifier.predict(features)  # "trending" | "mean_reverting" | "noisy"

        # TODO: run EM to update Kalman Q/R for current regime
        # q_price, q_vel, r = estimator.fit(ohlcv["close"].values)

        regime = "noisy"  # placeholder
        params = build_param_message(
            kalman_params={"Q_price": 1e-3, "Q_velocity": 1e-4, "R": 1e-2, "dt": 1.0},
            regime=regime,
            lms_mu=0.01,
            seq=seq,
        )

        # TODO Phase 4: publish over ZMQ
        # sock.send_string(json.dumps(params))
        log.info(f"[seq={seq}] Would publish: {params}")

        seq += 1
        time.sleep(interval_sec)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--interval", type=int, default=300, help="Update interval (seconds)")
    parser.add_argument("--endpoint", default="tcp://*:5555", help="ZMQ PUB endpoint")
    args = parser.parse_args()
    run(args.interval, args.endpoint)
