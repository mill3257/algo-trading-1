#include "core/csv_loader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>

namespace dsp_trader::core {

CSVLoader::CSVLoader(Config cfg) : cfg_(std::move(cfg)) {}

void CSVLoader::stream(std::function<void(const Tick&)> on_tick) const {
    std::ifstream f(cfg_.filepath);
    if (!f.is_open())
        throw std::runtime_error("CSVLoader: cannot open '" + cfg_.filepath + "'");

    std::string line;
    if (cfg_.has_header) std::getline(f, line); // skip header row

    row_count_ = 0;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        on_tick(parse_row(line));
        ++row_count_;
    }
}

std::vector<Tick> CSVLoader::load_all() const {
    std::vector<Tick> out;
    out.reserve(4096);
    stream([&](const Tick& t) { out.push_back(t); });
    return out;
}

Tick CSVLoader::parse_row(const std::string& line) const {
    // Columns: timestamp_ns, price, bid, ask, size, symbol
    Tick t{};
    std::istringstream ss(line);
    std::string tok;
    int col = 0;
    while (std::getline(ss, tok, cfg_.delimiter)) {
        // Trim whitespace
        auto b = tok.find_first_not_of(" \t\r");
        auto e = tok.find_last_not_of(" \t\r");
        if (b != std::string::npos) tok = tok.substr(b, e - b + 1);

        switch (col) {
            case 0: t.ts_ns = std::stoll(tok);                        break;
            case 1: t.price = std::stod(tok);                         break;
            case 2: t.bid   = std::stod(tok);                         break;
            case 3: t.ask   = std::stod(tok);                         break;
            case 4: t.size  = std::stod(tok);                         break;
            case 5: std::strncpy(t.symbol, tok.c_str(), 7);           break;
            default: break;
        }
        ++col;
    }
    return t;
}

} // namespace dsp_trader::core
