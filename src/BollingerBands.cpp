#include "BollingerBands.hpp"
#include <cmath>

BollingerBands::BollingerBands(int window, double multiplier)
    : windowSize(window), stdDevMultiplier(multiplier) {}

Signal BollingerBands::generateSignal(const Tick& currentTick) {
    priceHistory.push_back(currentTick.close);
    if (static_cast<int>(priceHistory.size()) < windowSize) return Signal::Hold;

    double mean = 0.0;
    for (size_t i = priceHistory.size() - windowSize; i < priceHistory.size(); ++i) mean += priceHistory[i];
    mean /= windowSize;

    double variance = 0.0;
    for (size_t i = priceHistory.size() - windowSize; i < priceHistory.size(); ++i)
        variance += (priceHistory[i] - mean) * (priceHistory[i] - mean);
    double sd = std::sqrt(variance / windowSize);

    if (currentTick.close < mean - stdDevMultiplier * sd) return Signal::Buy;
    if (currentTick.close > mean + stdDevMultiplier * sd) return Signal::Sell;
    return Signal::Hold;
}
