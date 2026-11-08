#include "Analytics.hpp"
#include <cmath>

double calculateHurstExponent(const std::vector<double>& priceData) {
    int totalDataPoints = static_cast<int>(priceData.size());
    if (totalDataPoints < 10) return 0.5;

    double varianceLag1 = 0.0, varianceLag2 = 0.0;
    double meanLag1 = 0.0, meanLag2 = 0.0;

    for (int i = 1; i < totalDataPoints; i++) meanLag1 += (priceData[i] - priceData[i - 1]);
    meanLag1 /= (totalDataPoints - 1);
    for (int i = 1; i < totalDataPoints; i++) varianceLag1 += std::pow((priceData[i] - priceData[i - 1]) - meanLag1, 2);
    varianceLag1 /= (totalDataPoints - 1);

    for (int i = 2; i < totalDataPoints; i++) meanLag2 += (priceData[i] - priceData[i - 2]);
    meanLag2 /= (totalDataPoints - 2);
    for (int i = 2; i < totalDataPoints; i++) varianceLag2 += std::pow((priceData[i] - priceData[i - 2]) - meanLag2, 2);
    varianceLag2 /= (totalDataPoints - 2);

    if (varianceLag1 == 0.0) return 0.5;
    return 0.5 * std::log2(varianceLag2 / varianceLag1);
}

std::string getHurstRegimeMessage(double hurstExponent) {
    if (hurstExponent > 0.55) {
        return "Market is Trending. Trend-following strategies (SMA/EMA) are optimal.";
    } else if (hurstExponent < 0.45) {
        return "Market is Ranging. Mean-reversion strategies (RSI/Bollinger/Z-Score) are optimal.";
    } else {
        return "Market is Random. No statistical edge based on market regime. Trading is highly risky.";
    }
}
