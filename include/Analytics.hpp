#pragma once
#include <vector>
#include <string>

double calculateHurstExponent(const std::vector<double>& priceData);
std::string getHurstRegimeMessage(double hurstExponent);
