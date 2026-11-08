#pragma once
#include <string>
#include "Strategy.hpp"

class EMACrossover : public Strategy {
private:
    int fastWindow;
    int slowWindow;
    double fastEMA;
    double slowEMA;
    int ticks;

public:
    EMACrossover(int fast, int slow);
    std::string evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) override;
};
