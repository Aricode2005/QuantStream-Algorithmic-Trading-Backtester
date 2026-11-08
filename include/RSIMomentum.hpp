#pragma once
#include <vector>
#include <string>
#include "Strategy.hpp"

class RSIMomentum : public Strategy {
private:
    int lookbackWindow;
    std::vector<double> priceHistory;

public:
    RSIMomentum(int window);
    std::string evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) override;
};
