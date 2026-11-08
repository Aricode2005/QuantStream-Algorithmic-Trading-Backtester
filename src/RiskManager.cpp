#include "RiskManager.hpp"
#include <algorithm>
#include <cmath>

namespace RiskManager {

bool checkStopLoss(const Tick& tick, const Portfolio& portfolio, double& fillPrice) {
    if (portfolio.getAssetQuantity() <= 0) return false;
    double stopPrice = portfolio.getAverageEntryPrice() * (1.0 - kStopLossPct);
    if (tick.low > stopPrice) return false;
    fillPrice = std::min(tick.open, stopPrice);
    return true;
}

int positionSize(double availableCash, double price, double riskPct) {
    if (price <= 0.0 || riskPct <= 0.0) return 0;
    return static_cast<int>(std::floor((availableCash * riskPct) / price));
}

}
