#include "Portfolio.hpp"

Portfolio::Portfolio(double initialCash)
    : availableCash(initialCash), averageEntryPrice(0.0), assetQuantity(0) {}

bool Portfolio::buyAsset(double currentPrice, int orderQuantity) {
    if (availableCash >= currentPrice * orderQuantity) {
        averageEntryPrice = currentPrice;
        availableCash -= currentPrice * orderQuantity;
        assetQuantity += orderQuantity;
        return true;
    }
    return false;
}

bool Portfolio::sellAsset(double currentPrice, int orderQuantity) {
    if (assetQuantity >= orderQuantity) {
        availableCash += currentPrice * orderQuantity;
        assetQuantity -= orderQuantity;
        return true;
    }
    return false;
}

double Portfolio::getTotalPortfolioValue(double currentPrice) const {
    return availableCash + (assetQuantity * currentPrice);
}

double Portfolio::getAvailableCash() const {
    return availableCash;
}

int Portfolio::getAssetQuantity() const {
    return assetQuantity;
}

double Portfolio::getAverageEntryPrice() const {
    return averageEntryPrice;
}
