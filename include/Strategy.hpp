#pragma once
#include <string>
#include "Types.hpp"
#include "Portfolio.hpp"

enum class Signal { Hold, Buy, Sell };

// Template Method: evaluateTrade() fixes the order of operations
// (stop-loss -> update indicators -> act on signal -> size order), and
// subclasses only implement generateSignal().
class Strategy {
public:
    virtual ~Strategy() = default;

    std::string evaluateTrade(const Tick& tick, Portfolio& portfolio, double riskPct);

    // Regime filters block new entries only. Indicators keep updating on every
    // bar and exits (signal sells, stop-loss) are never blocked.
    void setEntriesAllowed(bool allowed) { entriesAllowed = allowed; }

protected:
    virtual Signal generateSignal(const Tick& tick) = 0;

private:
    bool entriesAllowed = true;
};
