#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include "httplib.h"
#include "json.hpp"

#include "Types.hpp"
#include "Portfolio.hpp"
#include "Strategy.hpp"
#include "SMACrossover.hpp"
#include "EMACrossover.hpp"
#include "BollingerBands.hpp"
#include "RSIMomentum.hpp"
#include "ZScoreArbitrage.hpp"
#include "Analytics.hpp"

using json = nlohmann::json;

int main() {
    httplib::Server backtestServer;

    backtestServer.Get("/", [](const httplib::Request&, httplib::Response& response) {
        std::ifstream htmlFile("index.html");
        std::stringstream fileBuffer;
        fileBuffer << htmlFile.rdbuf();
        response.set_content(fileBuffer.str(), "text/html; charset=utf-8");
    });

    backtestServer.Post("/api/run_backtest", [&](const httplib::Request& request, httplib::Response& response) {
        try {
            auto requestJson = json::parse(request.body);

            double riskMultiplier = requestJson.value("risk_pct", 0.10);
            double startingCash = requestJson["start_cash"];
            std::string selectedStrategy = requestJson.value("strat_type", "SMA");

            Portfolio activePortfolio(startingCash, requestJson.value("fee_pct", 0.0));
            std::unique_ptr<Strategy> tradingStrategy;

            if (selectedStrategy == "BBAND") {
                tradingStrategy = std::make_unique<BollingerBands>(requestJson.value("fast_sma", 20), 2.0);
            } else if (selectedStrategy == "RSI") {
                tradingStrategy = std::make_unique<RSIMomentum>(requestJson.value("fast_sma", 14));
            } else if (selectedStrategy == "ZSCORE") {
                tradingStrategy = std::make_unique<ZScoreArbitrage>(requestJson.value("fast_sma", 20), 2.0);
            } else if (selectedStrategy == "EMA") {
                tradingStrategy = std::make_unique<EMACrossover>(requestJson.value("fast_sma", 10), requestJson.value("slow_sma", 50));
            } else {
                tradingStrategy = std::make_unique<SMACrossover>(requestJson.value("fast_sma", 10), requestJson.value("slow_sma", 50));
            }

            json responseJson;
            responseJson["equity_curve"] = json::array();
            responseJson["trades"] = json::array();

            std::vector<double> historicalPrices;
            std::vector<double> equityValues;
            int totalExecutedTrades = 0;

            for (auto& dataPoint : requestJson["market_data"]) {
                double closePrice = dataPoint.value("close", dataPoint.value("price", 0.0));
                double openPrice = dataPoint.value("open", closePrice);
                double highPrice = dataPoint.value("high", closePrice);
                double lowPrice = dataPoint.value("low", closePrice);

                Tick currentTick = {dataPoint["date"], openPrice, highPrice, lowPrice, closePrice};
                historicalPrices.push_back(currentTick.close);

                std::string tradeAction = tradingStrategy->evaluateTrade(currentTick, activePortfolio, riskMultiplier);

                if (tradeAction != "HOLD") {
                    double executedPrice = activePortfolio.getLastFillPrice();

                    responseJson["trades"].push_back({
                        {"date", currentTick.timestamp},
                        {"action", tradeAction},
                        {"price", executedPrice}
                    });
                    totalExecutedTrades++;
                }

                double currentEquity = activePortfolio.getTotalPortfolioValue(currentTick.close);
                equityValues.push_back(currentEquity);

                responseJson["equity_curve"].push_back({
                    {"date", currentTick.timestamp},
                    {"val", currentEquity},
                    {"price", currentTick.close}
                });
            }

            if (historicalPrices.empty()) {
                throw std::runtime_error("Market data array is empty.");
            }

            double hurstExponent = calculateHurstExponent(historicalPrices);
            std::string hurstMsg = getHurstRegimeMessage(hurstExponent);
            double sharpe = calculateSharpeRatio(equityValues, 252.0);
            double maxDd = calculateMaxDrawdown(equityValues);

            responseJson["final_value"] = activePortfolio.getTotalPortfolioValue(historicalPrices.back());
            responseJson["total_trades"] = totalExecutedTrades;
            responseJson["sharpe_ratio"] = sharpe;
            responseJson["max_drawdown"] = maxDd;
            responseJson["hurst_exponent"] = hurstExponent;
            responseJson["hurst_msg"] = hurstMsg;

            response.set_content(responseJson.dump(), "application/json");

        } catch (const json::exception& e) {
            std::cerr << "JSON Error: " << e.what() << std::endl;
            response.status = 400;
            response.set_content(std::string("JSON Error: ") + e.what(), "text/plain");
        } catch (const std::exception& e) {
            std::cerr << "Server Error: " << e.what() << std::endl;
            response.status = 500;
            response.set_content(std::string("Server Error: ") + e.what(), "text/plain");
        } catch (...) {
            std::cerr << "Unknown Fatal Error" << std::endl;
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
