#pragma once

class Portfolio {
private:
    double availableCash;
    double averageEntryPrice;
    int assetQuantity;

public:
    Portfolio(double initialCash);

    bool buyAsset(double currentPrice, int orderQuantity);
    bool sellAsset(double currentPrice, int orderQuantity);

    double getTotalPortfolioValue(double currentPrice) const;
    double getAvailableCash() const;
    int getAssetQuantity() const;
    double getAverageEntryPrice() const;
};
