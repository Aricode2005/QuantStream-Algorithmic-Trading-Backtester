#pragma once
#include <string>
#include "Types.hpp"
#include "Portfolio.hpp"

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual std::string evaluateTrade(const Tick& currentTick, Portfolio& userPortfolio, double riskPct) = 0;
};
