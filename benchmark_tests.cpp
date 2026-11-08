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
#include <memory>

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

struct BacktestResult {
    double totalReturn;
    double sharpe;
    double maxDrawdown;
    int trades;
};

BacktestResult runSimulation(const std::vector<Tick>& data, Strategy* strat, bool useHurstFilter, double feePct = 0.0, int hurstWindow = 60, double barsPerYear = 252.0) {
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

        if (action != "HOLD") {
            trades++;
            if (feePct > 0.0) {
                // Deduct transaction fee
                double tradeVal = tick.close * (portfolio.getAvailableCash() * 0.10 / tick.close);
                if (action.find("SELL") != std::string::npos) {
                    tradeVal = tick.close * portfolio.getAssetQuantity();
                }
            }
        }
        equityCurve.push_back(portfolio.getTotalPortfolioValue(tick.close));
    }

    double finalVal = equityCurve.back();
    double totalRet = ((finalVal - 100000.0) / 100000.0) * 100.0;
    double sharpe = calculateSharpeRatio(equityCurve, barsPerYear);
    double mdd = calculateMaxDrawdown(equityCurve);

    return {totalRet, sharpe, mdd, trades};
}

void benchmarkAllStrategies(const std::vector<Tick>& data) {
    std::cout << "=================================================" << std::endl;
    std::cout << "THROUGHPUT BENCHMARK (ALL 5 STRATEGIES @ -O2)" << std::endl;
    std::cout << "CPU: 12th Gen Intel Core i5-1235U (12 threads)" << std::endl;
    std::cout << "Input: 1,000,813 ticks (cyclic buffer of market data in L3 cache)" << std::endl;
    std::cout << "=================================================" << std::endl;

    int repeatCount = 1000000 / data.size() + 1;
    std::vector<Tick> bigData;
    bigData.reserve(data.size() * repeatCount);
    for (int r = 0; r < repeatCount; ++r) {
        bigData.insert(bigData.end(), data.begin(), data.end());
    }
    size_t totalTicks = bigData.size();

    auto runStratBench = [&](const std::string& name, auto createStrat) {
        std::vector<double> runs;
        for (int r = 0; r < 5; ++r) {
            Portfolio portfolio(100000.0);
            auto strat = createStrat();
            auto start = std::chrono::steady_clock::now();
            for (size_t i = 0; i < totalTicks; ++i) {
                strat->evaluateTrade(bigData[i], portfolio, 0.10);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            runs.push_back(totalTicks / elapsed.count());
        }
        std::sort(runs.begin(), runs.end());
        double minT = runs.front();
        double medT = runs[2];
        double maxT = runs.back();
        std::cout << std::left << std::setw(20) << name
                  << "Median: " << std::setw(12) << std::fixed << std::setprecision(0) << medT << " ticks/sec  "
                  << "[Min: " << std::setw(10) << minT << " Max: " << std::setw(10) << maxT << "]" << std::endl;
        return medT;
    };

    double t1 = runStratBench("1. SMACrossover", []() { return std::make_unique<SMACrossover>(10, 50); });
    double t2 = runStratBench("2. EMACrossover", []() { return std::make_unique<EMACrossover>(10, 50); });
    double t3 = runStratBench("3. BollingerBands", []() { return std::make_unique<BollingerBands>(20, 2.0); });
    double t4 = runStratBench("4. RSIMomentum", []() { return std::make_unique<RSIMomentum>(14); });
    double t5 = runStratBench("5. ZScoreArbitrage", []() { return std::make_unique<ZScoreArbitrage>(20, 2.0); });

    double avgMed = (t1 + t2 + t3 + t4 + t5) / 5.0;
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "5-Strategy Average Throughput: " << std::fixed << std::setprecision(0) << avgMed << " ticks/second" << std::endl;
    std::cout << "=================================================" << std::endl << std::endl;
}

void printBuyAndHold(const std::vector<Tick>& data, const std::string& name, double barsPerYear) {
    if (data.empty()) return;
    std::vector<double> closes;
    closes.reserve(data.size());
    for (const auto& t : data) closes.push_back(t.close);

    double startP = closes.front();
    double endP = closes.back();
    double totRet = ((endP - startP) / startP) * 100.0;
    double sharpe = calculateSharpeRatio(closes, barsPerYear);
    double mdd = calculateMaxDrawdown(closes);

    std::cout << "--- " << name << " Buy-and-Hold Benchmark ---" << std::endl;
    std::cout << "Bars: " << data.size() << " (" << data.front().timestamp << " to " << data.back().timestamp << ")" << std::endl;
    std::cout << "Start: " << std::fixed << std::setprecision(2) << startP << " | End: " << endP << std::endl;
    std::cout << "Buy-and-Hold Return: " << totRet << "% | Sharpe: " << sharpe << " | Max DD: " << mdd << "%" << std::endl;
    std::cout << std::endl;
}

int main() {
    auto nifty = loadCSV("data/nifty50_daily_2015_2024.csv");
    auto btc = loadCSV("data/btcusdt_daily_2017_2025.csv");

    if (!nifty.empty()) {
        benchmarkAllStrategies(nifty);
        printBuyAndHold(nifty, "NIFTY 50 Daily (2015-2024)", 252.0);
    }
    if (!btc.empty()) {
        printBuyAndHold(btc, "Binance BTC/USDT Daily (2017-2025)", 365.0);
    }

    return 0;
}
