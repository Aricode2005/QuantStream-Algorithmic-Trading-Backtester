#include "EMACrossover.hpp"

EMACrossover::EMACrossover(int fast, int slow)
    : fastWindow(fast), slowWindow(slow), fastEMA(0.0), slowEMA(0.0), ticks(0) {}

std::string EMACrossover::evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) {
    if (userPortfolio.getAssetQuantity() > 0) {
        double stopLossPrice = userPortfolio.getAverageEntryPrice() * 0.95;
        if (currentTick.low <= stopLossPrice) {
            int orderQty = userPortfolio.getAssetQuantity();
            if (userPortfolio.sellAsset(stopLossPrice, orderQty)) {
                return "SELL " + std::to_string(orderQty) + " (STOP LOSS)";
            }
        }
    }

    ticks++;
    double fastAlpha = 2.0 / (fastWindow + 1);
    double slowAlpha = 2.0 / (slowWindow + 1);

    if (ticks == 1) {
        fastEMA = currentTick.close;
        slowEMA = currentTick.close;
        return "HOLD";
    }

    fastEMA = (currentTick.close - fastEMA) * fastAlpha + fastEMA;
    slowEMA = (currentTick.close - slowEMA) * slowAlpha + slowEMA;

    if (ticks < slowWindow) {
        return "HOLD";
    }

    if (fastEMA > slowEMA && userPortfolio.getAvailableCash() >= currentTick.close) {
        int orderQuantity = static_cast<int>((userPortfolio.getAvailableCash() * riskPct) / currentTick.close);
        if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
            return "BUY " + std::to_string(orderQuantity);
        }
    } else if (fastEMA < slowEMA && userPortfolio.getAssetQuantity() > 0) {
        int orderQuantity = userPortfolio.getAssetQuantity();
        if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
            return "SELL " + std::to_string(orderQuantity);
        }
    }
    return "HOLD";
}
