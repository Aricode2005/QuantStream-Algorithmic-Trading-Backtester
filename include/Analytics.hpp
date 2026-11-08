#pragma once
#include <vector>
#include <string>

double calculateHurstExponent(const std::vector<double>& priceData);
std::string getHurstRegimeMessage(double hurstExponent);
double calculateSharpeRatio(const std::vector<double>& equityCurve, double barsPerYear = 252.0);
double calculateMaxDrawdown(const std::vector<double>& equityCurve);
