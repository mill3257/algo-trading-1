"""
Kalman Q/R Estimator via EM Algorithm
Estimates process noise (Q) and measurement noise (R) from observed price data
without requiring labeled data — purely unsupervised.

Reference: Shumway & Stoffer, "An Approach to Time Series Smoothing and
           Forecasting Using the EM Algorithm" (1982)

TODO (you implement):
    KalmanEMEstimator.fit(prices, n_iter=20):
        E-step: run Kalman smoother (forward + backward pass) to get
                expected state estimates and covariances
        M-step: update Q and R as closed-form MLE from smoother output

        Repeat until convergence (change in log-likelihood < tol).

    Key insight: Q and R appear in closed-form M-step updates:
        R_new = (1/T) * sum((z_t - H * x_hat_t)^2 + H * P_t * H^T)
        Q_new = (1/T) * sum(cov of state innovation)  [see reference]
"""


class KalmanEMEstimator:
    """
    TODO: implement fit() using the EM algorithm for state-space models.

    Parameters learned:
        Q_price    — process noise variance for price state
        Q_velocity — process noise variance for velocity state
        R          — measurement noise variance

    Returns a dict compatible with dsp::KalmanFilter::Params.
    """

    def __init__(self, dt: float = 1.0, n_iter: int = 20, tol: float = 1e-4):
        self.dt     = dt
        self.n_iter = n_iter
        self.tol    = tol

        # Current parameter estimates
        self.Q_price    = 1e-3
        self.Q_velocity = 1e-4
        self.R          = 1e-2

    def fit(self, prices) -> dict:
        """
        TODO: run EM on a 1-D array of prices.
        Returns dict: {"Q_price": ..., "Q_velocity": ..., "R": ..., "dt": self.dt}
        """
        raise NotImplementedError

    def _e_step(self, prices):
        """
        TODO: Kalman smoother (RTS smoother).
        Forward pass:  predict/update to get filtered estimates x_hat, P
        Backward pass: RTS smoother to get smoothed estimates x_smooth, P_smooth
        Returns smoothed means, covariances, and cross-covariances.
        """
        raise NotImplementedError

    def _m_step(self, prices, smoothed_means, smoothed_covs, cross_covs):
        """
        TODO: closed-form MLE update for Q and R given smoother output.
        Update self.Q_price, self.Q_velocity, self.R in place.
        """
        raise NotImplementedError

    def to_params(self) -> dict:
        return {
            "Q_price":    self.Q_price,
            "Q_velocity": self.Q_velocity,
            "R":          self.R,
            "dt":         self.dt,
        }
