#pragma once
#include <vector>
#include <string>
#include "Strategy.hpp"

class SMACrossover : public Strategy {
private:
    int fastWindow;
    int slowWindow;
    std::vector<double> priceHistory;

public:
    SMACrossover(int fast, int slow);
    std::string evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) override;
};
