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
protected:
    Signal generateSignal(const Tick& currentTick) override;
};
