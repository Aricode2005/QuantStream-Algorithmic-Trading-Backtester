#pragma once
#include "Types.hpp"
#include "Portfolio.hpp"

// Single, shared implementation of the engine's risk controls.
// Every strategy goes through Strategy::evaluateTrade, which calls these.
namespace RiskManager {

constexpr double kStopLossPct = 0.05;

// Stop-loss evaluated against the bar's Low. If the bar gaps open below the
// stop, the fill is at the Open (you cannot fill at a price that never traded).
bool checkStopLoss(const Tick& tick, const Portfolio& portfolio, double& fillPrice);

// Capital-proportional sizing: floor(cash * riskPct / price).
int positionSize(double availableCash, double price, double riskPct);

}
