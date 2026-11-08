#pragma once

class Portfolio {
private:
    double availableCash;
    double averageEntryPrice;   // volume-weighted average cost of the open position
    double lastFillPrice;
    double feeRate;             // proportional fee per trade, e.g. 0.001 = 0.10%
    int assetQuantity;

public:
    explicit Portfolio(double initialCash, double feeRate = 0.0);

    bool buyAsset(double currentPrice, int orderQuantity);
    bool sellAsset(double currentPrice, int orderQuantity);

    double getTotalPortfolioValue(double currentPrice) const;
    double getAvailableCash() const;
    int getAssetQuantity() const;
    double getAverageEntryPrice() const;
    double getLastFillPrice() const;
};
