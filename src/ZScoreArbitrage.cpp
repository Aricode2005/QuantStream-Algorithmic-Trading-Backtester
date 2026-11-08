#include "ZScoreArbitrage.hpp"
#include <cmath>

ZScoreArbitrage::ZScoreArbitrage(int windowSize, double threshold)
    : lookbackWindow(windowSize), zScoreThreshold(threshold) {}

std::string ZScoreArbitrage::evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) {
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
    if (static_cast<int>(priceHistory.size()) < lookbackWindow) {
        return "HOLD";
    }

    double rollingMean = 0.0;
    double rollingVariance = 0.0;

    for (size_t i = priceHistory.size() - lookbackWindow; i < priceHistory.size(); ++i) {
        rollingMean += priceHistory[i];
    }
    rollingMean /= lookbackWindow;

    for (size_t i = priceHistory.size() - lookbackWindow; i < priceHistory.size(); ++i) {
        rollingVariance += std::pow(priceHistory[i] - rollingMean, 2);
    }
    double standardDeviation = std::sqrt(rollingVariance / lookbackWindow);

    if (standardDeviation == 0.0) {
        return "HOLD";
    }

    double currentZScore = (currentTick.close - rollingMean) / standardDeviation;

    if (currentZScore < -zScoreThreshold && userPortfolio.getAvailableCash() >= currentTick.close) {
        int orderQuantity = static_cast<int>((userPortfolio.getAvailableCash() * riskPct) / currentTick.close);
        if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
            return "BUY " + std::to_string(orderQuantity);
        }
    } else if (currentZScore > zScoreThreshold && userPortfolio.getAssetQuantity() > 0) {
        int orderQuantity = userPortfolio.getAssetQuantity();
        if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
            return "SELL " + std::to_string(orderQuantity);
        }
    }
    return "HOLD";
}
