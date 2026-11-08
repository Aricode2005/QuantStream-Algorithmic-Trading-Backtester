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
protected:
    Signal generateSignal(const Tick& currentTick) override;
};
