#include "Strategy.hpp"
#include "RiskManager.hpp"

std::string Strategy::evaluateTrade(const Tick& tick, Portfolio& portfolio, double riskPct) {
    double stopFill = 0.0;
    bool stopped = RiskManager::checkStopLoss(tick, portfolio, stopFill);

    Signal signal = generateSignal(tick);   // always update indicator state

    if (stopped) {
        int qty = portfolio.getAssetQuantity();
        if (portfolio.sellAsset(stopFill, qty)) {
            return "SELL " + std::to_string(qty) + " (STOP LOSS)";
        }
    }

    if (signal == Signal::Buy && entriesAllowed) {
        int qty = RiskManager::positionSize(portfolio.getAvailableCash(), tick.close, riskPct);
        if (qty > 0 && portfolio.buyAsset(tick.close, qty)) {
            return "BUY " + std::to_string(qty);
        }
    } else if (signal == Signal::Sell && portfolio.getAssetQuantity() > 0) {
        int qty = portfolio.getAssetQuantity();
        if (portfolio.sellAsset(tick.close, qty)) {
            return "SELL " + std::to_string(qty);
        }
    }
    return "HOLD";
}
