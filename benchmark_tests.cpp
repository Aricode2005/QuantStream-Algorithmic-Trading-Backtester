// Reproducible experiments for QuantStream.
//   g++ -O2 -std=c++17 -Iinclude -I. benchmark_tests.cpp src/*.cpp -o benchmark && ./benchmark
// Prints: (1) throughput, (2) buy-and-hold, (3) every strategy with/without a
// Hurst entry filter, gross and net of fees, with true average exposure.
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <functional>

#include "Types.hpp"
#include "Portfolio.hpp"
#include "Strategy.hpp"
#include "SMACrossover.hpp"
#include "EMACrossover.hpp"
#include "BollingerBands.hpp"
#include "RSIMomentum.hpp"
#include "ZScoreArbitrage.hpp"
#include "Analytics.hpp"

static const double kStartCash = 100000.0;
static const double kRiskPct = 0.10;
static const double kFee = 0.001;        // 0.10% per trade
static const int kHurstWindow = 60;

std::vector<Tick> loadCSV(const std::string& path) {
    std::vector<Tick> ticks;
    std::ifstream file(path);
    if (!file.is_open()) { std::cerr << "Failed to open " << path << "\n"; return ticks; }
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string d, o, h, l, c;
        if (std::getline(ss, d, ',') && std::getline(ss, o, ',') && std::getline(ss, h, ',') &&
            std::getline(ss, l, ',') && std::getline(ss, c, ',')) {
            try { ticks.push_back({d, std::stod(o), std::stod(h), std::stod(l), std::stod(c)}); } catch (...) {}
        }
    }
    return ticks;
}

enum class Gate { None, Trend, MeanRev };   // Trend: enter only if H>0.5; MeanRev: only if H<0.5

struct Result { double ret, sharpe, maxDd, avgExposure, gateOnPct; int trades; };

// The Hurst value at bar i uses closes [i-60, i-1] only (no lookahead).
Result runBacktest(const std::vector<Tick>& data, Strategy& strat, Gate gate, double fee, double barsPerYear) {
    Portfolio portfolio(kStartCash, fee);
    std::vector<double> equity, closes;
    equity.reserve(data.size());
    closes.reserve(data.size());
    int trades = 0, gateOn = 0, gateBars = 0;
    double expo = 0.0;

    for (const Tick& t : data) {
        bool allow = true;
        if (gate != Gate::None && closes.size() >= static_cast<size_t>(kHurstWindow)) {
            std::vector<double> w(closes.end() - kHurstWindow, closes.end());
            double h = calculateHurstExponent(w);
            allow = (gate == Gate::Trend) ? (h > 0.5) : (h < 0.5);
            gateBars++; if (allow) gateOn++;
        }
        strat.setEntriesAllowed(allow);
        if (strat.evaluateTrade(t, portfolio, kRiskPct) != "HOLD") trades++;
        closes.push_back(t.close);
        double v = portfolio.getTotalPortfolioValue(t.close);
        equity.push_back(v);
        expo += portfolio.getAssetQuantity() * t.close / v;
    }
    return {(equity.back() / kStartCash - 1.0) * 100.0, calculateSharpeRatio(equity, barsPerYear),
            calculateMaxDrawdown(equity), 100.0 * expo / data.size(),
            gateBars ? 100.0 * gateOn / gateBars : 100.0, trades};
}

void benchmarkThroughput(const std::vector<Tick>& data) {
    int reps = 1000000 / data.size() + 1;
    std::vector<Tick> big;
    big.reserve(data.size() * reps);
    for (int r = 0; r < reps; ++r) big.insert(big.end(), data.begin(), data.end());

    std::cout << "## Throughput (evaluateTrade only, " << big.size() << " bars replayed from NIFTY data,\n"
              << "   sizeof(Tick)=" << sizeof(Tick) << "B => " << big.size() * sizeof(Tick) / (1024 * 1024)
              << " MB working set, single thread, median of 5)\n\n";

    double totalNsPerBar = 0.0;
    auto bench = [&](const std::string& name, std::function<std::unique_ptr<Strategy>()> make) {
        std::vector<double> runs;
        for (int r = 0; r < 5; ++r) {
            Portfolio p(kStartCash);
            auto s = make();
            auto t0 = std::chrono::steady_clock::now();
            for (const Tick& t : big) s->evaluateTrade(t, p, kRiskPct);
            std::chrono::duration<double> el = std::chrono::steady_clock::now() - t0;
            runs.push_back(big.size() / el.count());
        }
        std::sort(runs.begin(), runs.end());
        std::cout << std::left << std::setw(18) << name << std::right << std::fixed << std::setprecision(1)
                  << std::setw(8) << runs[2] / 1e6 << " M bars/s  ~" << std::setw(5) << 1e9 / runs[2]
                  << " ns/bar  [" << runs[0] / 1e6 << " - " << runs[4] / 1e6 << " M]\n";
        totalNsPerBar += 1e9 / runs[2];
    };
    bench("SMACrossover",     [] { return std::make_unique<SMACrossover>(10, 50); });
    bench("EMACrossover",     [] { return std::make_unique<EMACrossover>(10, 50); });
    bench("BollingerBands",   [] { return std::make_unique<BollingerBands>(20, 2.0); });
    bench("RSIMomentum",      [] { return std::make_unique<RSIMomentum>(14); });
    bench("ZScoreArbitrage",  [] { return std::make_unique<ZScoreArbitrage>(20, 2.0); });
    std::cout << "All five on every bar: ~" << std::setprecision(0) << totalNsPerBar << " ns/bar => ~"
              << std::setprecision(1) << 1e3 / totalNsPerBar << " M bars/s\n\n";
}

void runDataset(const std::string& name, const std::vector<Tick>& data, double barsPerYear) {
    std::vector<double> closes;
    for (const Tick& t : data) closes.push_back(t.close);
    std::cout << "## " << name << " (" << data.size() << " bars, " << data.front().timestamp << " to "
              << data.back().timestamp << ", Sharpe annualised with sqrt(" << barsPerYear << "))\n\n";
    std::cout << "Buy & hold: ret " << std::fixed << std::setprecision(1) << (closes.back() / closes.front() - 1) * 100
              << "%  Sharpe " << std::setprecision(2) << calculateSharpeRatio(closes, barsPerYear)
              << "  MaxDD " << std::setprecision(1) << calculateMaxDrawdown(closes) << "%\n\n";

    std::cout << std::left << std::setw(16) << "Strategy" << std::setw(22) << "Config"
              << std::right << std::setw(9) << "Ret%" << std::setw(8) << "Sharpe" << std::setw(8) << "MaxDD%"
              << std::setw(8) << "Trades" << std::setw(9) << "AvgExp%" << std::setw(9) << "GateOn%" << "\n";

    struct S { std::string n; std::function<std::unique_ptr<Strategy>()> make; Gate g; };
    std::vector<S> strategies = {
        {"SMACrossover",    [] { return std::make_unique<SMACrossover>(10, 50); },    Gate::Trend},
        {"EMACrossover",    [] { return std::make_unique<EMACrossover>(10, 50); },    Gate::Trend},
        {"BollingerBands",  [] { return std::make_unique<BollingerBands>(20, 2.0); }, Gate::MeanRev},
        {"RSIMomentum",     [] { return std::make_unique<RSIMomentum>(14); },         Gate::MeanRev},
        {"ZScoreArbitrage", [] { return std::make_unique<ZScoreArbitrage>(20, 2.0); }, Gate::MeanRev},
    };
    auto row = [&](const std::string& sn, const std::string& cfg, const Result& r) {
        std::cout << std::left << std::setw(16) << sn << std::setw(22) << cfg << std::right << std::fixed
                  << std::setprecision(1) << std::setw(9) << r.ret << std::setprecision(2) << std::setw(8) << r.sharpe
                  << std::setprecision(1) << std::setw(8) << r.maxDd << std::setw(8) << r.trades
                  << std::setw(9) << r.avgExposure << std::setw(9) << r.gateOnPct << "\n";
    };
    for (auto& s : strategies) {
        auto a = s.make(); row(s.n, "baseline, no fees",   runBacktest(data, *a, Gate::None, 0.0, barsPerYear));
        auto b = s.make(); row(s.n, "baseline, 0.10% fee", runBacktest(data, *b, Gate::None, kFee, barsPerYear));
        auto c = s.make(); row(s.n, s.g == Gate::Trend ? "Hurst H>0.5, 0.10% fee" : "Hurst H<0.5, 0.10% fee",
                               runBacktest(data, *c, s.g, kFee, barsPerYear));
    }
    std::cout << "\n";
}

int main() {
    auto nifty = loadCSV("data/nifty50_daily_2015_2024.csv");
    auto btc = loadCSV("data/btcusdt_daily_2017_2025.csv");
    if (nifty.empty() || btc.empty()) { std::cerr << "Run from the repo root (data/ not found)\n"; return 1; }
    benchmarkThroughput(nifty);
    runDataset("NIFTY 50 daily", nifty, 252.0);
    runDataset("BTC/USDT daily", btc, 365.0);   // crypto trades 365 days/year
    return 0;
}
