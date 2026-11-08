#include "Analytics.hpp"
#include <cmath>
#include <numeric>

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
        return "Market is Trending (H > 0.55). Historical persistence indicates trend-following suitability.";
    } else if (hurstExponent < 0.45) {
        return "Market is Mean-Reverting (H < 0.45). Historical anti-persistence indicates mean-reversion suitability.";
    } else {
        return "Market is Random Walk (H ~ 0.50). Historical behavior shows no persistent statistical direction.";
    }
}

double calculateSharpeRatio(const std::vector<double>& equityCurve, double barsPerYear) {
    if (equityCurve.size() < 2) return 0.0;
    std::vector<double> returns;
    returns.reserve(equityCurve.size() - 1);
    for (size_t i = 1; i < equityCurve.size(); ++i) {
        if (equityCurve[i - 1] > 0.0) {
            returns.push_back((equityCurve[i] - equityCurve[i - 1]) / equityCurve[i - 1]);
        }
    }
    if (returns.size() < 2) return 0.0;

    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double sumSq = 0.0;
    for (double r : returns) {
        sumSq += (r - mean) * (r - mean);
    }
    double sd = std::sqrt(sumSq / (returns.size() - 1));
    if (sd == 0.0) return 0.0;

    return (mean / sd) * std::sqrt(barsPerYear);
}

double calculateMaxDrawdown(const std::vector<double>& equityCurve) {
    if (equityCurve.empty()) return 0.0;
    double peak = equityCurve[0];
    double maxDd = 0.0;
    for (double val : equityCurve) {
        if (val > peak) peak = val;
        if (peak > 0.0) {
            double dd = (peak - val) / peak;
            if (dd > maxDd) maxDd = dd;
        }
    }
    return maxDd * 100.0;
}
