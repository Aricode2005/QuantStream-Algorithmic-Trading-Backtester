#include "ZScoreArbitrage.hpp"
#include <cmath>

ZScoreArbitrage::ZScoreArbitrage(int windowSize, double threshold)
    : lookbackWindow(windowSize), zScoreThreshold(threshold) {}

// Single-asset rolling z-score mean reversion (not pairs arbitrage).
Signal ZScoreArbitrage::generateSignal(const Tick& currentTick) {
    priceHistory.push_back(currentTick.close);
    if (static_cast<int>(priceHistory.size()) < lookbackWindow) return Signal::Hold;

    double mean = 0.0;
    for (size_t i = priceHistory.size() - lookbackWindow; i < priceHistory.size(); ++i) mean += priceHistory[i];
    mean /= lookbackWindow;

    double var = 0.0;
    for (size_t i = priceHistory.size() - lookbackWindow; i < priceHistory.size(); ++i)
        var += (priceHistory[i] - mean) * (priceHistory[i] - mean);
    double sd = std::sqrt(var / lookbackWindow);
    if (sd == 0.0) return Signal::Hold;

    double z = (currentTick.close - mean) / sd;
    if (z < -zScoreThreshold) return Signal::Buy;
    if (z > zScoreThreshold) return Signal::Sell;
    return Signal::Hold;
}
