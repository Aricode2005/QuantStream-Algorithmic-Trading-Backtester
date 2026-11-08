#pragma once
#include <vector>
#include <string>
#include "Strategy.hpp"

class ZScoreArbitrage : public Strategy {
private:
    int lookbackWindow;
    double zScoreThreshold;
    std::vector<double> priceHistory;

public:
    ZScoreArbitrage(int windowSize, double threshold);
    std::string evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) override;
};
