#pragma once
#include "core/tick.h"
#include <string>
#include <vector>
#include <functional>

namespace dsp_trader::core {

/// Loads historical tick data from CSV for backtesting.
///
/// Expected format (header required):
///   timestamp_ns, price, bid, ask, size, symbol
///
/// Alpaca / Polygon historical exports can be pre-processed to this format
/// with the scripts/normalize_csv.py helper.
class CSVLoader {
public:
    struct Config {
        std::string filepath;
        char        delimiter  = ',';
        bool        has_header = true;
    };

    explicit CSVLoader(Config cfg);

    /// Stream rows chronologically through a callback (low memory).
    void stream(std::function<void(const Tick&)> on_tick) const;

    /// Load entire file into memory (unit tests / small datasets).
    std::vector<Tick> load_all() const;

    std::size_t row_count() const { return row_count_; }

private:
    Config              cfg_;
    mutable std::size_t row_count_{0};

    Tick parse_row(const std::string& line) const;
};

} // namespace dsp_trader::core
