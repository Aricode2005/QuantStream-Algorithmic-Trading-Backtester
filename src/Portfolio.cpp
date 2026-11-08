#include "Portfolio.hpp"

Portfolio::Portfolio(double initialCash, double fee)
    : availableCash(initialCash), averageEntryPrice(0.0), lastFillPrice(0.0),
      feeRate(fee), assetQuantity(0) {}

bool Portfolio::buyAsset(double currentPrice, int orderQuantity) {
    if (orderQuantity <= 0) return false;
    double cost = currentPrice * orderQuantity * (1.0 + feeRate);
    if (availableCash < cost) return false;

    averageEntryPrice = (averageEntryPrice * assetQuantity + currentPrice * orderQuantity)
                        / (assetQuantity + orderQuantity);
    availableCash -= cost;
    assetQuantity += orderQuantity;
    lastFillPrice = currentPrice;
    return true;
}

bool Portfolio::sellAsset(double currentPrice, int orderQuantity) {
    if (orderQuantity <= 0 || assetQuantity < orderQuantity) return false;
    availableCash += currentPrice * orderQuantity * (1.0 - feeRate);
    assetQuantity -= orderQuantity;
    lastFillPrice = currentPrice;
    if (assetQuantity == 0) averageEntryPrice = 0.0;
    return true;
}

double Portfolio::getTotalPortfolioValue(double currentPrice) const {
    return availableCash + (assetQuantity * currentPrice);
}
double Portfolio::getAvailableCash() const { return availableCash; }
int Portfolio::getAssetQuantity() const { return assetQuantity; }
double Portfolio::getAverageEntryPrice() const { return averageEntryPrice; }
double Portfolio::getLastFillPrice() const { return lastFillPrice; }
