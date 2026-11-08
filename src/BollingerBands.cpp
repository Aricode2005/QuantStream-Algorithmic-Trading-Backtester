#include "BollingerBands.hpp"
#include <cmath>

BollingerBands::BollingerBands(int window, double multiplier)
    : windowSize(window), stdDevMultiplier(multiplier) {}

std::string BollingerBands::evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) {
    if (userPortfolio.getAssetQuantity() > 0) {
        double stopLossPrice = userPortfolio.getAverageEntryPrice() * 0.95;
        if (currentTick.low <= stopLossPrice) {
            int orderQty = userPortfolio.getAssetQuantity();
            if (userPortfolio.sellAsset(stopLossPrice, orderQty)) {
                return "SELL " + std::to_string(orderQty) + " (STOP LOSS)";
            }
        }
    }

    priceHistory.push_back(currentTick.close);
    if (static_cast<int>(priceHistory.size()) < windowSize) {
        return "HOLD";
    }

    double movingAverage = 0.0;
    for (size_t i = priceHistory.size() - windowSize; i < priceHistory.size(); ++i) {
        movingAverage += priceHistory[i];
    }
    movingAverage /= windowSize;

    double variance = 0.0;
    for (size_t i = priceHistory.size() - windowSize; i < priceHistory.size(); ++i) {
        variance += (priceHistory[i] - movingAverage) * (priceHistory[i] - movingAverage);
    }
    variance /= windowSize;
    double standardDeviation = std::sqrt(variance);

    double upperBand = movingAverage + (stdDevMultiplier * standardDeviation);
    double lowerBand = movingAverage - (stdDevMultiplier * standardDeviation);

    if (currentTick.close < lowerBand && userPortfolio.getAvailableCash() >= currentTick.close) {
        int orderQuantity = static_cast<int>((userPortfolio.getAvailableCash() * riskPct) / currentTick.close);
        if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
            return "BUY " + std::to_string(orderQuantity);
        }
    } else if (currentTick.close > upperBand && userPortfolio.getAssetQuantity() > 0) {
        int orderQuantity = userPortfolio.getAssetQuantity();
        if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
            return "SELL " + std::to_string(orderQuantity);
        }
    }
    return "HOLD";
}
