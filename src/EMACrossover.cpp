#include "EMACrossover.hpp"

EMACrossover::EMACrossover(int fast, int slow)
    : fastWindow(fast), slowWindow(slow), fastEMA(0.0), slowEMA(0.0), ticks(0) {}

Signal EMACrossover::generateSignal(const Tick& currentTick) {
    ticks++;
    double fastAlpha = 2.0 / (fastWindow + 1);
    double slowAlpha = 2.0 / (slowWindow + 1);

    if (ticks == 1) {
        fastEMA = currentTick.close;
        slowEMA = currentTick.close;
        return Signal::Hold;
    }

    fastEMA = (currentTick.close - fastEMA) * fastAlpha + fastEMA;
    slowEMA = (currentTick.close - slowEMA) * slowAlpha + slowEMA;

    if (ticks < slowWindow) return Signal::Hold;
    if (fastEMA > slowEMA) return Signal::Buy;
    if (fastEMA < slowEMA) return Signal::Sell;
    return Signal::Hold;
}
