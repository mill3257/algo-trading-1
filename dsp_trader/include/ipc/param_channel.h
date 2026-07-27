#pragma once
#include "dsp/kalman_filter.h"
#include "alpha/alpha_generator.h"
#include <atomic>
#include <array>
#include <string>

namespace dsp_trader::ipc {

/// Parameter bundle pushed by the Python ML service every N minutes.
struct MLParams {
    dsp::KalmanFilter::Params kalman;  // Q/R updated by EM algorithm
    alpha::Regime             regime;  // current regime classification
    double                    lms_mu;  // LMS step size for current regime
    uint64_t                  seq{0};  // monotonic sequence number (for staleness detection)
};

/// Lock-free double-buffered parameter store.
///
/// Writer (ZMQ subscriber thread): calls write() on each received MLParams.
/// Reader (hot-path DSP thread):   calls read() every bar — zero contention.
///
/// Why double-buffer instead of std::atomic?
///   MLParams is larger than what hardware CAS supports.
///   Double-buffer + atomic index gives seqlock-like semantics without
///   requiring 128-bit atomics.
///
/// TODO (you implement):
///   - write() : copy p into the inactive buffer, then atomically flip active_
///   - read()  : load active_ index, return that buffer's contents
class ParamChannel {
public:
    ParamChannel();

    void     write(const MLParams& p) noexcept; // TODO — writer thread
    MLParams read()  const noexcept;            // TODO — reader thread (hot path)

    uint64_t last_seq() const; // peek at the sequence number of the active buffer

private:
    std::array<MLParams, 2> bufs_{};
    std::atomic<int>        active_{0};
};

/// ZMQ subscriber — listens for MLParams published by the Python service.
/// Stub in Phase 1-3. Wire up in Phase 4 when HAVE_ZMQ is defined.
///
/// Protocol: Python serializes MLParams as JSON or MessagePack and publishes
/// on a ZMQ PUB socket. This class SUBscribes, deserializes, and calls
/// channel_.write() on each received message.
class ZMQParamSubscriber {
public:
    explicit ZMQParamSubscriber(ParamChannel& channel,
                                const char* endpoint = "tcp://localhost:5555");
    ~ZMQParamSubscriber();

    void start(); // spawn subscriber thread — TODO Phase 4
    void stop();

private:
    ParamChannel& channel_;
    std::string   endpoint_;
    bool          running_{false};
    // void* zmq_ctx_{nullptr};  // uncomment when linking libzmq
    // void* zmq_sock_{nullptr};
};

} // namespace dsp_trader::ipc
