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
protected:
    Signal generateSignal(const Tick& currentTick) override;
};
