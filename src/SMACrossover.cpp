#include "SMACrossover.hpp"

SMACrossover::SMACrossover(int fast, int slow)
    : fastWindow(fast), slowWindow(slow) {}

Signal SMACrossover::generateSignal(const Tick& currentTick) {
    priceHistory.push_back(currentTick.close);
    if (static_cast<int>(priceHistory.size()) < slowWindow) return Signal::Hold;

    double fastMA = 0.0;
    for (size_t i = priceHistory.size() - fastWindow; i < priceHistory.size(); ++i) fastMA += priceHistory[i];
    fastMA /= fastWindow;

    double slowMA = 0.0;
    for (size_t i = priceHistory.size() - slowWindow; i < priceHistory.size(); ++i) slowMA += priceHistory[i];
    slowMA /= slowWindow;

    if (fastMA > slowMA) return Signal::Buy;
    if (fastMA < slowMA) return Signal::Sell;
    return Signal::Hold;
}
