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
protected:
    Signal generateSignal(const Tick& currentTick) override;
};
