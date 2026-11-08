#include "RSIMomentum.hpp"

RSIMomentum::RSIMomentum(int window)
    : lookbackWindow(window) {}

// Note: uses a simple (Cutler) average of gains/losses over the window,
// not Wilder's exponential smoothing.
Signal RSIMomentum::generateSignal(const Tick& currentTick) {
    priceHistory.push_back(currentTick.close);
    if (static_cast<int>(priceHistory.size()) <= lookbackWindow) return Signal::Hold;

    double gain = 0.0, loss = 0.0;
    for (size_t i = priceHistory.size() - lookbackWindow; i < priceHistory.size(); ++i) {
        double diff = priceHistory[i] - priceHistory[i - 1];
        if (diff > 0) gain += diff; else loss -= diff;
    }
    gain /= lookbackWindow;
    loss /= lookbackWindow;

    double rsi = 100.0;
    if (loss != 0.0) rsi = 100.0 - 100.0 / (1.0 + gain / loss);

    if (rsi < 30.0) return Signal::Buy;
    if (rsi > 70.0) return Signal::Sell;
    return Signal::Hold;
}
