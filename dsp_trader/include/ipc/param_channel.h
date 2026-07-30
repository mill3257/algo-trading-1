#pragma once
#include "dsp/kalman_filter.h"
#include "alpha/alpha_generator.h"
#include <atomic>
#include <array>
#include <string>

namespace dsp_trader::ipc {

/*
Parameter bundle pushed by the Python ML service every N minutes.
regime is the alpha::Regime enum (6 possible values)
python service serialises it as a string and
zmq subscriber thread maps it back to the C++ enum before calling write()
ex: "bullish_quiet" --> alpha::Regime::BullishQuiet
Unrecognized strings should leave the current regime unchanged (stale fallback)
*/

struct MLParams {
    dsp::KalmanFilter::Params kalman; //Q/R updated by EM algorithm
    alpha::Regime regime; //current regime (6 cases)
    double lms_mu; //LMS step size for current regime
    uint64_t seq{0}; //monotonic sequence number (for staleness detection)
};

/*
Lock-free double-buffered parameter store
Writer (ZMQ subscriber thread): calls write() on each received MLParams
Reader (hot-path DSP thread): calls read() every bar, zero contention

TODO:
    write(): copy p into the inactive buffer, then atomically flip active_
    read(): load active_ index, return that buffer's contents
*/

class ParamChannel {
public:
    ParamChannel();

    void write(const MLParams& p) noexcept; //TODO writer thread

    MLParams read()  const noexcept; //TODO reader thread (hot path)

    uint64_t last_seq() const; //peek at the sequence number of the active buffer

    bool is_stale(uint64_t max_age_bars) const; //TODO compare seq vs expected

private:
    std::array<MLParams, 2> bufs_{};
    std::atomic<int> active_{0};
};

/*
ZMQ subscriber: listens for MLParams published by the Python ML service
Stub in Phase 1-3. Wire up in Phase 4 when HAVE_ZMQ is defined

Protocol: Python publishes JSON on a ZMQ PUB socket
example message:
    {"kalman": {"Q_price":1e-3,"Q_velocity":1e-4,"R":1e-2,"dt":1.0}
    "regime": "bullish_quiet", "lms_mu": 0.01, "seq": 42}
subscriber deserializes the JSON and maps "regime" string to
alpha::Regime using canonical string table in MLParams documentation
*/

class ZMQParamSubscriber {
public:
    explicit ZMQParamSubscriber(ParamChannel& channel,
                                const char* endpoint = "tcp://localhost:5555");
    ~ZMQParamSubscriber();

    void start(); //spawn subscriber thread, TODO Phase 4
    void stop();

private:
    ParamChannel& channel_;
    std::string endpoint_;
    bool running_{false};
    // void* zmq_ctx_{nullptr};  // uncomment when linking libzmq
    // void* zmq_sock_{nullptr};
};

}
