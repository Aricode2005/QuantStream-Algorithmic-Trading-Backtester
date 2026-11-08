#include "SMACrossover.hpp"

SMACrossover::SMACrossover(int fast, int slow)
    : fastWindow(fast), slowWindow(slow) {}

std::string SMACrossover::evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) {
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
    if (static_cast<int>(priceHistory.size()) < slowWindow) {
        return "HOLD";
    }

    double fastMovingAverage = 0.0;
    for (size_t i = priceHistory.size() - fastWindow; i < priceHistory.size(); ++i) {
        fastMovingAverage += priceHistory[i];
    }
    fastMovingAverage /= fastWindow;

    double slowMovingAverage = 0.0;
    for (size_t i = priceHistory.size() - slowWindow; i < priceHistory.size(); ++i) {
        slowMovingAverage += priceHistory[i];
    }
    slowMovingAverage /= slowWindow;

    if (fastMovingAverage > slowMovingAverage && userPortfolio.getAvailableCash() >= currentTick.close) {
        int orderQuantity = static_cast<int>((userPortfolio.getAvailableCash() * riskPct) / currentTick.close);
        if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
            return "BUY " + std::to_string(orderQuantity);
        }
    } else if (fastMovingAverage < slowMovingAverage && userPortfolio.getAssetQuantity() > 0) {
        int orderQuantity = userPortfolio.getAssetQuantity();
        if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
            return "SELL " + std::to_string(orderQuantity);
        }
    }
    return "HOLD";
}
