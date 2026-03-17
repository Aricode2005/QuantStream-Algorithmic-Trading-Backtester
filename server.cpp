#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <cmath>
#include "httplib.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

struct Tick { 
    string timestamp; 
    double open;
    double high;
    double low;
    double close; 
};

class Portfolio {
    double availableCash;
    double averageEntryPrice;
    int assetQuantity;

public:
    int winningTrades = 0;
    int losingTrades = 0;
    double sumWinningPnL = 0;
    double sumLosingPnL = 0;

    Portfolio(double initialCash) : availableCash(initialCash), averageEntryPrice(0), assetQuantity(0) {}
    
    bool buyAsset(double currentPrice, int orderQuantity) { 
        if (availableCash >= currentPrice * orderQuantity) { 
            averageEntryPrice = currentPrice; 
            availableCash -= currentPrice * orderQuantity; 
            assetQuantity += orderQuantity; 
            return true; 
        } 
        return false;
    }
    
    bool sellAsset(double currentPrice, int orderQuantity) { 
        if (assetQuantity >= orderQuantity) { 
            double realizedPnL = (currentPrice - averageEntryPrice) * orderQuantity;
            
            if (realizedPnL > 0) { 
                winningTrades++; 
                sumWinningPnL += realizedPnL; 
            } else { 
                losingTrades++; 
                sumLosingPnL += abs(realizedPnL); 
            }
            
            availableCash += currentPrice * orderQuantity; 
            assetQuantity -= orderQuantity; 
            return true; 
        } 
        return false;
    }
    
    double getTotalPortfolioValue(double currentPrice) { 
        return availableCash + (assetQuantity * currentPrice); 
    }
    
    double getAvailableCash() { return availableCash; }
    int getAssetQuantity() { return assetQuantity; }
    double getAverageEntryPrice() { return averageEntryPrice; } // NEW: Needed for Stop-Loss
};

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual string evaluateTrade(Tick currentTick, Portfolio& userPortfolio, double riskPct) = 0;
};

// 1. SMA CROSSOVER
class SMACrossover : public Strategy {
    int fastWindow, slowWindow;
    vector<double> priceHistory;
public:
    SMACrossover(int fast, int slow) : fastWindow(fast), slowWindow(slow) {}
    
    string evaluateTrade(Tick currentTick, Portfolio& userPortfolio, double riskPct) override {
        // INTRADAY STOP LOSS CHECK (5%)
        if (userPortfolio.getAssetQuantity() > 0) {
            double stopLossPrice = userPortfolio.getAverageEntryPrice() * 0.95;
            if (currentTick.low <= stopLossPrice) {
                int orderQty = userPortfolio.getAssetQuantity();
                if (userPortfolio.sellAsset(stopLossPrice, orderQty)) return "SELL " + to_string(orderQty) + " (STOP LOSS)";
            }
        }

        priceHistory.push_back(currentTick.close);
        if (priceHistory.size() < slowWindow) return "HOLD";
        
        double fastMovingAverage = 0, slowMovingAverage = 0;
        
        for (int i = priceHistory.size() - fastWindow; i < priceHistory.size(); i++) {
            fastMovingAverage += priceHistory[i];
        }
        fastMovingAverage /= fastWindow;
        
        for (int i = priceHistory.size() - slowWindow; i < priceHistory.size(); i++) {
            slowMovingAverage += priceHistory[i];
        }
        slowMovingAverage /= slowWindow;

        if (fastMovingAverage > slowMovingAverage && userPortfolio.getAvailableCash() >= currentTick.close) {
            int orderQuantity = (userPortfolio.getAvailableCash() * riskPct) / currentTick.close;
            if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
                return "BUY " + to_string(orderQuantity);
            }
        } else if (fastMovingAverage < slowMovingAverage && userPortfolio.getAssetQuantity() > 0) {
            int orderQuantity = userPortfolio.getAssetQuantity();
            if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
                return "SELL " + to_string(orderQuantity);
            }
        }
        return "HOLD";
    }
};

// 2. BOLLINGER BANDS
class BollingerBands : public Strategy {
    int windowSize;
    double stdDevMultiplier;
    vector<double> priceHistory;
public:
    BollingerBands(int window, double multiplier) : windowSize(window), stdDevMultiplier(multiplier) {}
    
    string evaluateTrade(Tick currentTick, Portfolio& userPortfolio, double riskPct) override {
        if (userPortfolio.getAssetQuantity() > 0) {
            double stopLossPrice = userPortfolio.getAverageEntryPrice() * 0.95;
            if (currentTick.low <= stopLossPrice) {
                int orderQty = userPortfolio.getAssetQuantity();
                if (userPortfolio.sellAsset(stopLossPrice, orderQty)) return "SELL " + to_string(orderQty) + " (STOP LOSS)";
            }
        }

        priceHistory.push_back(currentTick.close);
        if (priceHistory.size() < windowSize) return "HOLD";

        double movingAverage = 0;
        for (int i = priceHistory.size() - windowSize; i < priceHistory.size(); i++) {
            movingAverage += priceHistory[i];
        }
        movingAverage /= windowSize;

        double variance = 0;
        for (int i = priceHistory.size() - windowSize; i < priceHistory.size(); i++) {
            variance += (priceHistory[i] - movingAverage) * (priceHistory[i] - movingAverage);
        }
        variance /= windowSize;
        double standardDeviation = sqrt(variance);

        double upperBand = movingAverage + (stdDevMultiplier * standardDeviation);
        double lowerBand = movingAverage - (stdDevMultiplier * standardDeviation);

        if (currentTick.close < lowerBand && userPortfolio.getAvailableCash() >= currentTick.close) {
            int orderQuantity = (userPortfolio.getAvailableCash() * riskPct) / currentTick.close;
            if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
                return "BUY " + to_string(orderQuantity);
            }
        } else if (currentTick.close > upperBand && userPortfolio.getAssetQuantity() > 0) {
            int orderQuantity = userPortfolio.getAssetQuantity();
            if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
                return "SELL " + to_string(orderQuantity);
            }
        }
        return "HOLD";
    }
};

// 3. RSI MOMENTUM
class RSIMomentum : public Strategy {
    int lookbackWindow;
    vector<double> priceHistory;
public:
    RSIMomentum(int window) : lookbackWindow(window) {}
    
    string evaluateTrade(Tick currentTick, Portfolio& userPortfolio, double riskPct) override {
        if (userPortfolio.getAssetQuantity() > 0) {
            double stopLossPrice = userPortfolio.getAverageEntryPrice() * 0.95;
            if (currentTick.low <= stopLossPrice) {
                int orderQty = userPortfolio.getAssetQuantity();
                if (userPortfolio.sellAsset(stopLossPrice, orderQty)) return "SELL " + to_string(orderQty) + " (STOP LOSS)";
            }
        }

        priceHistory.push_back(currentTick.close);
        if (priceHistory.size() <= lookbackWindow) return "HOLD"; 

        double averageGain = 0, averageLoss = 0;
        
        for (int i = priceHistory.size() - lookbackWindow; i < priceHistory.size(); i++) {
            double priceDifference = priceHistory[i] - priceHistory[i-1];
            if (priceDifference > 0) {
                averageGain += priceDifference;
            } else {
                averageLoss -= priceDifference; 
            }
        }
        
        averageGain /= lookbackWindow;
        averageLoss /= lookbackWindow;

        double rsiValue = 100.0;
        if (averageLoss != 0) {
            double relativeStrength = averageGain / averageLoss;
            rsiValue = 100.0 - (100.0 / (1.0 + relativeStrength));
        }

        if (rsiValue < 30.0 && userPortfolio.getAvailableCash() >= currentTick.close) {
            int orderQuantity = (userPortfolio.getAvailableCash() * riskPct) / currentTick.close;
            if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
                return "BUY " + to_string(orderQuantity);
            }
        } else if (rsiValue > 70.0 && userPortfolio.getAssetQuantity() > 0) {
            int orderQuantity = userPortfolio.getAssetQuantity();
            if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
                return "SELL " + to_string(orderQuantity);
            }
        }
        return "HOLD";
    }
};

// 4. Z-SCORE ARBITRAGE
class ZScoreArbitrage : public Strategy {
    int lookbackWindow; 
    double zScoreThreshold; 
    vector<double> priceHistory;
public:
    ZScoreArbitrage(int windowSize, double threshold) : lookbackWindow(windowSize), zScoreThreshold(threshold) {}
    
    string evaluateTrade(Tick currentTick, Portfolio& userPortfolio, double riskPct) override {
        if (userPortfolio.getAssetQuantity() > 0) {
            double stopLossPrice = userPortfolio.getAverageEntryPrice() * 0.95;
            if (currentTick.low <= stopLossPrice) {
                int orderQty = userPortfolio.getAssetQuantity();
                if (userPortfolio.sellAsset(stopLossPrice, orderQty)) return "SELL " + to_string(orderQty) + " (STOP LOSS)";
            }
        }

        priceHistory.push_back(currentTick.close);
        if (priceHistory.size() < lookbackWindow) return "HOLD";
        
        double rollingMean = 0;
        double rollingVariance = 0;
        
        for (int i = priceHistory.size() - lookbackWindow; i < priceHistory.size(); i++) {
            rollingMean += priceHistory[i];
        }
        rollingMean /= lookbackWindow;
        
        for (int i = priceHistory.size() - lookbackWindow; i < priceHistory.size(); i++) {
            rollingVariance += pow(priceHistory[i] - rollingMean, 2);
        }
        double standardDeviation = sqrt(rollingVariance / lookbackWindow);
        
        if (standardDeviation == 0) return "HOLD";
        
        double currentZScore = (currentTick.close - rollingMean) / standardDeviation;

        if (currentZScore < -zScoreThreshold && userPortfolio.getAvailableCash() >= currentTick.close) {
            int orderQuantity = (userPortfolio.getAvailableCash() * riskPct) / currentTick.close;
            if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
                return "BUY " + to_string(orderQuantity);
            }
        } else if (currentZScore > zScoreThreshold && userPortfolio.getAssetQuantity() > 0) {
            int orderQuantity = userPortfolio.getAssetQuantity();
            if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
                return "SELL " + to_string(orderQuantity);
            }
        }
        return "HOLD";
    }
};

// 5. KALMAN FILTER
class KalmanFilterStrategy : public Strategy {
    double processNoise;
    double measurementNoise;
    double estimatedPrice;
    double errorCovariance;
    bool isInitialized;
public:
    KalmanFilterStrategy(double qNoise = 1e-4, double rNoise = 1e-2) 
        : processNoise(qNoise), measurementNoise(rNoise), estimatedPrice(0), errorCovariance(1), isInitialized(false) {}
        
    string evaluateTrade(Tick currentTick, Portfolio& userPortfolio, double riskPct) override {
        if (userPortfolio.getAssetQuantity() > 0) {
            double stopLossPrice = userPortfolio.getAverageEntryPrice() * 0.95;
            if (currentTick.low <= stopLossPrice) {
                int orderQty = userPortfolio.getAssetQuantity();
                if (userPortfolio.sellAsset(stopLossPrice, orderQty)) return "SELL " + to_string(orderQty) + " (STOP LOSS)";
            }
        }

        if (!isInitialized) { 
            estimatedPrice = currentTick.close; 
            isInitialized = true; 
            return "HOLD"; 
        }
        
        double predictedCovariance = errorCovariance + processNoise;
        double kalmanGain = predictedCovariance / (predictedCovariance + measurementNoise);
        estimatedPrice = estimatedPrice + kalmanGain * (currentTick.close - estimatedPrice);
        errorCovariance = (1 - kalmanGain) * predictedCovariance;

        if (currentTick.close > estimatedPrice * 1.002 && userPortfolio.getAvailableCash() >= currentTick.close) {
            int orderQuantity = (userPortfolio.getAvailableCash() * riskPct) / currentTick.close;
            if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
                return "BUY " + to_string(orderQuantity);
            }
        } else if (currentTick.close < estimatedPrice * 0.998 && userPortfolio.getAssetQuantity() > 0) {
            int orderQuantity = userPortfolio.getAssetQuantity();
            if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
                return "SELL " + to_string(orderQuantity);
            }
        }
        return "HOLD";
    }
};

// 6. EMA CROSSOVER
class EMACrossover : public Strategy {
    int fastWindow, slowWindow;
    double fastEMA, slowEMA;
    int ticks;
public:
    EMACrossover(int fast, int slow) : fastWindow(fast), slowWindow(slow), fastEMA(0), slowEMA(0), ticks(0) {}

    string evaluateTrade(Tick currentTick, Portfolio& userPortfolio, double riskPct) override {
        if (userPortfolio.getAssetQuantity() > 0) {
            double stopLossPrice = userPortfolio.getAverageEntryPrice() * 0.95;
            if (currentTick.low <= stopLossPrice) {
                int orderQty = userPortfolio.getAssetQuantity();
                if (userPortfolio.sellAsset(stopLossPrice, orderQty)) return "SELL " + to_string(orderQty) + " (STOP LOSS)";
            }
        }

        ticks++;
        double fastAlpha = 2.0 / (fastWindow + 1);
        double slowAlpha = 2.0 / (slowWindow + 1);

        if (ticks == 1) {
            fastEMA = currentTick.close;
            slowEMA = currentTick.close;
            return "HOLD";
        }

        fastEMA = (currentTick.close - fastEMA) * fastAlpha + fastEMA;
        slowEMA = (currentTick.close - slowEMA) * slowAlpha + slowEMA;

        if (ticks < slowWindow) return "HOLD";

        if (fastEMA > slowEMA && userPortfolio.getAvailableCash() >= currentTick.close) {
            int orderQuantity = (userPortfolio.getAvailableCash() * riskPct) / currentTick.close;
            if (orderQuantity > 0 && userPortfolio.buyAsset(currentTick.close, orderQuantity)) {
                return "BUY " + to_string(orderQuantity);
            }
        } else if (fastEMA < slowEMA && userPortfolio.getAssetQuantity() > 0) {
            int orderQuantity = userPortfolio.getAssetQuantity();
            if (orderQuantity > 0 && userPortfolio.sellAsset(currentTick.close, orderQuantity)) {
                return "SELL " + to_string(orderQuantity);
            }
        }
        return "HOLD";
    }
};

// STATISTICAL METRICS
double calculateHurstExponent(const vector<double>& priceData) {
    int totalDataPoints = priceData.size();
    if (totalDataPoints < 10) return 0.5;
    
    double varianceLag1 = 0, varianceLag2 = 0;
    double meanLag1 = 0, meanLag2 = 0;
    
    for(int i = 1; i < totalDataPoints; i++) meanLag1 += (priceData[i] - priceData[i-1]);
    meanLag1 /= (totalDataPoints - 1);
    for(int i = 1; i < totalDataPoints; i++) varianceLag1 += pow((priceData[i] - priceData[i-1]) - meanLag1, 2);
    varianceLag1 /= (totalDataPoints - 1);

    for(int i = 2; i < totalDataPoints; i++) meanLag2 += (priceData[i] - priceData[i-2]);
    meanLag2 /= (totalDataPoints - 2);
    for(int i = 2; i < totalDataPoints; i++) varianceLag2 += pow((priceData[i] - priceData[i-2]) - meanLag2, 2);
    varianceLag2 /= (totalDataPoints - 2);

    if(varianceLag1 == 0) return 0.5;
    return 0.5 * log2(varianceLag2 / varianceLag1);
}

int main() {
    httplib::Server backtestServer;

    backtestServer.Get("/", [](const httplib::Request&, httplib::Response& response) {
        ifstream htmlFile("index.html"); 
        stringstream fileBuffer; 
        fileBuffer << htmlFile.rdbuf();
        response.set_content(fileBuffer.str(), "text/html; charset=utf-8");
    });

   backtestServer.Post("/api/run_backtest", [&](const httplib::Request& request, httplib::Response& response) {
        try {
            auto requestJson = json::parse(request.body);
            
            double riskMultiplier = requestJson.value("risk_pct", 0.10);
            double startingCash = requestJson["start_cash"];
            string selectedStrategy = requestJson.value("strat_type", "SMA"); 
            
            Portfolio activePortfolio(startingCash);
            unique_ptr<Strategy> tradingStrategy;

            if (selectedStrategy == "BBAND") {
                tradingStrategy = make_unique<BollingerBands>(requestJson.value("fast_sma", 20), 2.0);
            } else if (selectedStrategy == "RSI") {
                tradingStrategy = make_unique<RSIMomentum>(requestJson.value("fast_sma", 14));
            } else if (selectedStrategy == "ZSCORE") {
                tradingStrategy = make_unique<ZScoreArbitrage>(requestJson.value("fast_sma", 20), 2.0);
            } else if (selectedStrategy == "KALMAN") {
                tradingStrategy = make_unique<KalmanFilterStrategy>();
            } else if (selectedStrategy == "EMA") {
                tradingStrategy = make_unique<EMACrossover>(requestJson.value("fast_sma", 10), requestJson.value("slow_sma", 50));
            } else {
                tradingStrategy = make_unique<SMACrossover>(requestJson.value("fast_sma", 10), requestJson.value("slow_sma", 50));
            }
            
            json responseJson;
            responseJson["equity_curve"] = json::array();
            responseJson["trades"] = json::array();
            
            vector<double> historicalPrices;
            int totalExecutedTrades = 0;

            for (auto& dataPoint : requestJson["market_data"]) {
                double closePrice = dataPoint.value("close", dataPoint.value("price", 0.0));
                double openPrice = dataPoint.value("open", closePrice);
                double highPrice = dataPoint.value("high", closePrice);
                double lowPrice = dataPoint.value("low", closePrice);

                Tick currentTick = {dataPoint["date"], openPrice, highPrice, lowPrice, closePrice};
                historicalPrices.push_back(currentTick.close);
                
                string tradeAction = tradingStrategy->evaluateTrade(currentTick, activePortfolio, riskMultiplier);
                
                if (tradeAction != "HOLD") {
                    double executedPrice = (tradeAction.find("STOP LOSS") != string::npos) ? (activePortfolio.getAverageEntryPrice() * 0.95) : currentTick.close;

                    responseJson["trades"].push_back({
                        {"date", currentTick.timestamp}, 
                        {"action", tradeAction}, 
                        {"price", executedPrice}
                    });
                    totalExecutedTrades++;
                }
                
                responseJson["equity_curve"].push_back({
                    {"date", currentTick.timestamp}, 
                    {"val", activePortfolio.getTotalPortfolioValue(currentTick.close)}, 
                    {"price", currentTick.close}
                });
            }

            if (historicalPrices.empty()) {
                throw std::runtime_error("Market data array is empty.");
            }

            double hurstExponent = calculateHurstExponent(historicalPrices);
            double optimalKellyFraction = 0.0;
            int totalClosedTrades = activePortfolio.winningTrades + activePortfolio.losingTrades;
            
            if (totalClosedTrades > 0) {
                double winRateProbability = (double)activePortfolio.winningTrades / totalClosedTrades;
                double averageWin = activePortfolio.winningTrades > 0 ? activePortfolio.sumWinningPnL / activePortfolio.winningTrades : 0;
                double averageLoss = activePortfolio.losingTrades > 0 ? activePortfolio.sumLosingPnL / activePortfolio.losingTrades : 1; 
                double winLossRatio = (averageLoss > 0) ? (averageWin / averageLoss) : 1;
                
                optimalKellyFraction = winRateProbability - ((1.0 - winRateProbability) / winLossRatio);
            }

            string hurstMsg = "";
            if (hurstExponent > 0.55) {
                hurstMsg = "Market is Trending. Trend-following strategies (SMA/EMA) are optimal.";
            } else if (hurstExponent < 0.45) {
                hurstMsg = "Market is Ranging. Mean-reversion strategies (RSI/Bollinger/Z-Score) are optimal.";
            } else {
                hurstMsg = "Market is Random. No statistical edge based on market regime. Trading is highly risky.";
            }

            string kellyMsg = "";
            if (optimalKellyFraction > 0) {
                kellyMsg = "Positive Edge. Suggested max risk per trade (Half-Kelly): " + to_string(round((optimalKellyFraction / 2.0) * 10000.0) / 100.0) + "%";
            } else {
                kellyMsg = "Negative Edge. The math strongly advises against deploying this strategy.";
            }

            responseJson["final_value"] = activePortfolio.getTotalPortfolioValue(historicalPrices.back());
            responseJson["total_trades"] = totalExecutedTrades;
            responseJson["hurst_exponent"] = hurstExponent;
            responseJson["kelly_fraction"] = optimalKellyFraction;
            responseJson["hurst_msg"] = hurstMsg;
            responseJson["kelly_msg"] = kellyMsg;
            
            response.set_content(responseJson.dump(), "application/json");

        } catch (const json::exception& e) {
            cerr << "JSON Error: " << e.what() << std::endl;
            response.status = 400; 
            response.set_content(std::string("JSON Error: ") + e.what(), "text/plain");
        } catch (const exception& e) {
            cerr << "Server Error: " << e.what() << std::endl;
            response.status = 500; 
            response.set_content(string("Server Error: ") + e.what(), "text/plain");
        } catch (...) {
            cerr << "Unknown Fatal Error" << std::endl;
            response.status = 500;
            response.set_content("Unknown Error occurred during backtest", "text/plain");
        }
    });

    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::stoi(port_env) : 10000; 

    std::cout << "Server starting on port " << port << std::endl;
    
    backtestServer.listen("0.0.0.0", port);
    return 0;
}
