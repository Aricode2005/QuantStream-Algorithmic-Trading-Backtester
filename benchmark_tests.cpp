#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <numeric>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>

#include "Types.hpp"
#include "Portfolio.hpp"
#include "Strategy.hpp"
#include "SMACrossover.hpp"
#include "EMACrossover.hpp"
#include "BollingerBands.hpp"
#include "RSIMomentum.hpp"
#include "ZScoreArbitrage.hpp"
#include "Analytics.hpp"

std::vector<Tick> loadCSV(const std::string& path) {
    std::vector<Tick> ticks;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << path << std::endl;
        return ticks;
    }
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string date, openStr, highStr, lowStr, closeStr;
        if (std::getline(ss, date, ',') &&
            std::getline(ss, openStr, ',') &&
            std::getline(ss, highStr, ',') &&
            std::getline(ss, lowStr, ',') &&
            std::getline(ss, closeStr, ',')) {
            try {
                double o = std::stod(openStr);
                double h = std::stod(highStr);
                double l = std::stod(lowStr);
                double c = std::stod(closeStr);
                ticks.push_back({date, o, h, l, c});
            } catch (...) {}
        }
    }
    return ticks;
}

void testHurstSynthetic() {
    std::cout << "=================================================" << std::endl;
    std::cout << "1. HURST EXPONENT SYNTHETIC VALIDATION" << std::endl;
    std::cout << "=================================================" << std::endl;

    std::mt19937 gen(42);
    std::normal_distribution<double> dist(0.0, 1.0);

    // Random walk (uncorrelated increments)
    std::vector<double> rw(5000, 100.0);
    for (size_t i = 1; i < rw.size(); ++i) {
        rw[i] = rw[i - 1] + dist(gen);
    }
    double h_rw = calculateHurstExponent(rw);
    std::cout << "Synthetic Random Walk (5,000 points): H = " << std::fixed << std::setprecision(4) << h_rw 
              << " (Expected ~0.50)" << std::endl;

    // Persistent / Trending momentum (positively correlated increments: AR(1) return momentum)
    std::vector<double> trend(5000, 100.0);
    double prevRet = 0.0;
    for (size_t i = 1; i < trend.size(); ++i) {
        double ret = 0.7 * prevRet + dist(gen);
        trend[i] = trend[i - 1] + ret;
        prevRet = ret;
    }
    double h_trend = calculateHurstExponent(trend);
    std::cout << "Synthetic Trending Momentum (5,000 points): H = " << h_trend 
              << " (Expected > 0.50)" << std::endl;

    // Mean-reverting series (Ornstein-Uhlenbeck)
    std::vector<double> mr(5000, 100.0);
    double theta = 0.4;
    double mu = 100.0;
    for (size_t i = 1; i < mr.size(); ++i) {
        mr[i] = mr[i - 1] + theta * (mu - mr[i - 1]) + dist(gen);
    }
    double h_mr = calculateHurstExponent(mr);
    std::cout << "Synthetic Mean-Reverting Series (5,000 points): H = " << h_mr 
              << " (Expected < 0.50)" << std::endl;
    std::cout << std::endl;
}

void testThroughput(const std::vector<Tick>& data) {
    std::cout << "=================================================" << std::endl;
    std::cout << "2. THROUGHPUT BENCHMARK (TICKS/SEC @ -O2)" << std::endl;
    std::cout << "=================================================" << std::endl;

    if (data.empty()) return;

    int repeatCount = 1000000 / data.size() + 1;
    std::vector<Tick> bigData;
    bigData.reserve(data.size() * repeatCount);
    for (int r = 0; r < repeatCount; ++r) {
        bigData.insert(bigData.end(), data.begin(), data.end());
    }
    size_t totalTicks = bigData.size();

    std::vector<double> ticksPerSecRuns;
    const int numRuns = 5;

    for (int run = 0; run < numRuns; ++run) {
        Portfolio portfolio(100000.0);
        SMACrossover strat(10, 50);

        auto start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < totalTicks; ++i) {
            strat.evaluateTrade(bigData[i], portfolio, 0.10);
        }
        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        double tps = totalTicks / elapsed.count();
        ticksPerSecRuns.push_back(tps);
        std::cout << "Run " << (run + 1) << ": " << std::fixed << std::setprecision(0)
                  << tps << " ticks/sec (" << totalTicks << " ticks in " 
                  << std::setprecision(4) << elapsed.count() << "s)" << std::endl;
    }

    std::sort(ticksPerSecRuns.begin(), ticksPerSecRuns.end());
    double medianTPS = ticksPerSecRuns[numRuns / 2];
    std::cout << "--> Typical (Median) Throughput: " << std::fixed << std::setprecision(0)
              << medianTPS << " ticks/second" << std::endl << std::endl;
}

struct BacktestResult {
    double totalReturn;
    double sharpe;
    double maxDrawdown;
    int trades;
};

BacktestResult runSimulation(const std::vector<Tick>& data, Strategy* strat, bool useHurstFilter, int hurstWindow = 50) {
    Portfolio portfolio(100000.0);
    std::vector<double> equityCurve;
    equityCurve.reserve(data.size());
    std::vector<double> priceHistory;
    priceHistory.reserve(data.size());
    int trades = 0;

    for (size_t i = 0; i < data.size(); ++i) {
        const auto& tick = data[i];
        priceHistory.push_back(tick.close);

        bool allowTrade = true;
        if (useHurstFilter && priceHistory.size() >= static_cast<size_t>(hurstWindow)) {
            std::vector<double> windowPrices(priceHistory.end() - hurstWindow, priceHistory.end() - 1);
            double h = calculateHurstExponent(windowPrices);
            if (h <= 0.50) {
                allowTrade = false;
            }
        }

        std::string action = "HOLD";
        if (allowTrade) {
            action = strat->evaluateTrade(tick, portfolio, 0.10);
        } else {
            if (portfolio.getAssetQuantity() > 0) {
                double stopLossPrice = portfolio.getAverageEntryPrice() * 0.95;
                if (tick.low <= stopLossPrice) {
                    int qty = portfolio.getAssetQuantity();
                    portfolio.sellAsset(stopLossPrice, qty);
                    action = "SELL (STOP LOSS)";
                }
            }
        }

        if (action != "HOLD") trades++;
        equityCurve.push_back(portfolio.getTotalPortfolioValue(tick.close));
    }

    double finalVal = equityCurve.back();
    double totalRet = ((finalVal - 100000.0) / 100000.0) * 100.0;
    double sharpe = calculateSharpeRatio(equityCurve, 252.0);
    double mdd = calculateMaxDrawdown(equityCurve);

    return {totalRet, sharpe, mdd, trades};
}

void testHurstImpact(const std::vector<Tick>& data, const std::string& datasetName) {
    std::cout << "=================================================" << std::endl;
    std::cout << "3. A/B EXPERIMENT: HURST REGIME SWITCHING" << std::endl;
    std::cout << "Dataset: " << datasetName << " (" << data.size() << " trading days)" << std::endl;
    std::cout << "=================================================" << std::endl;

    {
        SMACrossover stratBase(10, 50);
        BacktestResult base = runSimulation(data, &stratBase, false);

        SMACrossover stratHurst(10, 50);
        BacktestResult hurst = runSimulation(data, &stratHurst, true, 60);

        std::cout << "--- SMA Crossover Strategy (10/50) ---" << std::endl;
        std::cout << std::left << std::setw(22) << "Configuration"
                  << std::setw(16) << "Return (%)"
                  << std::setw(14) << "Sharpe"
                  << std::setw(16) << "Max DD (%)"
                  << std::setw(10) << "Trades" << std::endl;
        std::cout << std::left << std::setw(22) << "Baseline (Always On)"
                  << std::setw(16) << std::fixed << std::setprecision(2) << base.totalReturn
                  << std::setw(14) << std::setprecision(2) << base.sharpe
                  << std::setw(16) << std::setprecision(2) << base.maxDrawdown
                  << std::setw(10) << base.trades << std::endl;
        std::cout << std::left << std::setw(22) << "Hurst Regime Filter"
                  << std::setw(16) << std::fixed << std::setprecision(2) << hurst.totalReturn
                  << std::setw(14) << std::setprecision(2) << hurst.sharpe
                  << std::setw(16) << std::setprecision(2) << hurst.maxDrawdown
                  << std::setw(10) << hurst.trades << std::endl;
    }

    std::cout << std::endl;

    {
        EMACrossover stratBase(10, 50);
        BacktestResult base = runSimulation(data, &stratBase, false);

        EMACrossover stratHurst(10, 50);
        BacktestResult hurst = runSimulation(data, &stratHurst, true, 60);

        std::cout << "--- EMA Crossover Strategy (10/50) ---" << std::endl;
        std::cout << std::left << std::setw(22) << "Configuration"
                  << std::setw(16) << "Return (%)"
                  << std::setw(14) << "Sharpe"
                  << std::setw(16) << "Max DD (%)"
                  << std::setw(10) << "Trades" << std::endl;
        std::cout << std::left << std::setw(22) << "Baseline (Always On)"
                  << std::setw(16) << std::fixed << std::setprecision(2) << base.totalReturn
                  << std::setw(14) << std::setprecision(2) << base.sharpe
                  << std::setw(16) << std::setprecision(2) << base.maxDrawdown
                  << std::setw(10) << base.trades << std::endl;
        std::cout << std::left << std::setw(22) << "Hurst Regime Filter"
                  << std::setw(16) << std::fixed << std::setprecision(2) << hurst.totalReturn
                  << std::setw(14) << std::setprecision(2) << hurst.sharpe
                  << std::setw(16) << std::setprecision(2) << hurst.maxDrawdown
                  << std::setw(10) << hurst.trades << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    testHurstSynthetic();

    auto niftyData = loadCSV("data/nifty50_daily_2015_2025.csv");
    if (!niftyData.empty()) {
        testThroughput(niftyData);
        testHurstImpact(niftyData, "NIFTY 50 Daily (2015-2025)");
    }

    auto btcData = loadCSV("data/btcusdt_daily_2017_2025.csv");
    if (!btcData.empty()) {
        testHurstImpact(btcData, "Binance BTC/USDT Daily (2017-2025)");
    }

    return 0;
}
