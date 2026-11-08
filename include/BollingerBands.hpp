#pragma once
#include <vector>
#include <string>
#include "Strategy.hpp"

class BollingerBands : public Strategy {
private:
    int windowSize;
    double stdDevMultiplier;
    std::vector<double> priceHistory;

public:
    BollingerBands(int window, double multiplier);
    std::string evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) override;
};
