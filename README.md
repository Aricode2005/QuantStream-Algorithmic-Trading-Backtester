# 📈 QuantStream: C++ Algorithmic Backtesting Engine

**Live Deployment:** https://quantstream-algorithmic-trading-2sby.onrender.com/

QuantStream is an institutional-grade, event-driven backtesting engine designed for quantitative trading strategies. Built with a robust **C++ backend** and a modern, responsive **Web UI**, it processes historical OHLC (Open, High, Low, Close) market data to simulate complex trading algorithms. The engine provides deep statistical insights, including market memory (Hurst Exponent), optimal position sizing (Kelly Criterion), and realistic intraday risk management.

## ⚡ Core Architecture & Features

* **Event-Driven C++ Engine:** Processes market data chronologically tick-by-tick, preventing look-ahead bias and accurately simulating real-world execution latency.
* **OHLC Intraday Risk Management:** Goes beyond standard "Close-only" backtesters by utilizing the intraday `Low` price to enforce a dynamic 5% Stop-Loss, ensuring survival against flash crashes and preventing mathematical survivor bias.
* **Multiple Algorithmic Strategies:**
    * **SMA & EMA Crossover:** Trend-following momentum strategies.
    * **Bollinger Bands:** Mean-reversion statistical channel strategy.
    * **RSI Momentum:** Relative Strength Index oscillator (J. Welles Wilder).
    * **Z-Score Arbitrage:** Standard deviation-based anomaly detection.
    * **1D Kalman Filter:** Advanced recursive state estimation and noise reduction.
* **Quantitative Analytics (AI Insights):**
    * **Hurst Exponent (H):** Diagnoses the market regime (Trending, Mean-Reverting, or Random Walk) to validate strategy deployment.
    * **Kelly Criterion (f*):** Calculates the mathematically optimal fraction of capital to risk per trade based on historical Win/Loss ratios, deploying a "Half-Kelly" safety buffer.
* **Dynamic Web Dashboard:** * Advanced regex CSV parsing with automatic Currency Symbol detection.
    * Interactive dual-axis **Portfolio Value vs. Asset Price** curve via Chart.js.
    * Real-time execution log detailing specific BUY, SELL, and STOP LOSS triggers.

## 🛠️ Tech Stack

* **Backend:** C++ (Standard Library), `cpp-httplib` (Multithreaded HTTP Server), `nlohmann/json` (Data Serialization)
* **Frontend:** HTML5, CSS3, Vanilla JavaScript, Chart.js (Dual-axis plotting)
* **Infrastructure:** Docker, Render, Railway.app

## 🚀 Local Installation & Setup

### Prerequisites
* A C++11 (or higher) compatible compiler (`g++`)
* Docker (Optional, for containerized deployment)

### Linux / macOS Compilation
Open your terminal in the project directory and run:
```bash
g++ -O3 server.cpp -o server -lpthread
./server
