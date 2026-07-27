#include "ipc/param_channel.h"

namespace dsp_trader::ipc {

ParamChannel::ParamChannel() {
    // Zero-initialize both buffers
    bufs_[0] = MLParams{};
    bufs_[1] = MLParams{};
}

void ParamChannel::write(const MLParams& p) noexcept {
    int inactive = 1 - active_.load(std::memory_order_relaxed);
    bufs_[inactive] = p;
    active_.store(inactive, std::memory_order_release);
}

MLParams ParamChannel::read() const noexcept {
    return bufs_[active_.load(std::memory_order_acquire)];
}

uint64_t ParamChannel::last_seq() const {
    return bufs_[active_.load(std::memory_order_acquire)].seq;
}

// ── ZMQParamSubscriber ────────────────────────────────────────────────────────

ZMQParamSubscriber::ZMQParamSubscriber(ParamChannel& ch, const char* ep)
    : channel_(ch), endpoint_(ep) {}

ZMQParamSubscriber::~ZMQParamSubscriber() { stop(); }

void ZMQParamSubscriber::start() {
    // Phase 4: init ZMQ context, bind SUB socket, spin thread that calls
    // channel_.write() on every received JSON message.
    // Requires: #include <zmq.h> and linking -lzmq
}

void ZMQParamSubscriber::stop() {
    running_ = false;
}

} // namespace dsp_trader::ipc
