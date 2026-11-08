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
protected:
    Signal generateSignal(const Tick& currentTick) override;
};
