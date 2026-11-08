#include "RSIMomentum.hpp"

RSIMomentum::RSIMomentum(int window)
    : lookbackWindow(window) {}

std::string RSIMomentum::evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) {
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
    if (static_cast<int>(priceHistory.size()) <= lookbackWindow) {
        return "HOLD";
    }

    double averageGain = 0.0;
    double averageLoss = 0.0;

    for (size_t i = priceHistory.size() - lookbackWindow; i < priceHistory.size(); ++i) {
        double priceDifference = priceHistory[i] - priceHistory[i - 1];
        if (priceDifference > 0) {
            averageGain += priceDifference;
        } else {
            averageLoss -= priceDifference;
        }
    }

    averageGain /= lookbackWindow;
    averageLoss /= lookbackWindow;

    double rsiValue = 100.0;
    if (averageLoss != 0.0) {
        double relativeStrength = averageGain / averageLoss;
        rsiValue = 100.0 - (100.0 / (1.0 + relativeStrength));
    }

    if (rsiValue < 30.0 && userPortfolio.getAvailableCash() >= currentTick.close) {
        int orderQuantity = static_cast<int>((userPortfolio.getAvailableCash() * riskPct) / currentTick.close);
        if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
            return "BUY " + std::to_string(orderQuantity);
        }
    } else if (rsiValue > 70.0 && userPortfolio.getAssetQuantity() > 0) {
        int orderQuantity = userPortfolio.getAssetQuantity();
        if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
            return "SELL " + std::to_string(orderQuantity);
        }
    }
    return "HOLD";
}
