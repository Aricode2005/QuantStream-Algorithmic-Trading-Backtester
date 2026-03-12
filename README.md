# 📈 QuantStream: C++ Algorithmic Backtesting Engine

QuantStream is a high-performance, lightweight backtesting engine designed for quantitative trading strategies. Built with a robust **C++ backend** and a modern, responsive **Web UI**, it processes historical market data to simulate complex trading algorithms and provides deep statistical insights like market memory (Hurst Exponent) and optimal position sizing (Kelly Criterion).

## ⚡ Core Features

* **High-Speed C++ Engine:** Leverages raw C++ for rapid data crunching and minimal latency during simulations.
* **Multiple Trading Algorithms:**
    * **SMA Crossover:** Trend-following momentum strategy.
    * **Bollinger Bands:** Mean-reversion statistical strategy.
    * **RSI Momentum:** Relative Strength Index oscillator.
    * **Z-Score Arbitrage:** Standard deviation-based anomaly detection.
    * **Kalman Filter:** Advanced noise reduction and price estimation.
* **Quantitative Analytics:**
    * **Hurst Exponent ($H$):** Determines the market regime (trending vs. mean-reverting).
    * **Kelly Criterion ($f^*$):** Calculates the mathematically optimal fraction of capital to risk per trade.
* **Dynamic Web Dashboard:** Built with HTML5, JavaScript, and Chart.js to visualize the dual-axis **Portfolio Value (₹) vs. Asset Price** curve.
* **Cloud-Ready:** Fully Dockerized using a multi-stage build for seamless deployment to Linux environments like Render or AWS.

## 🛠️ Tech Stack

* **Backend:** C++ (Standard Library), `cpp-httplib` (Multithreaded HTTP Server), `nlohmann/json` (Data Serialization)
* **Frontend:** HTML/CSS/JS, Chart.js (Dual-axis plotting)
* **Deployment:** Docker (Multi-stage Debian Bookworm build)

## 🚀 Local Installation & Setup

### Prerequisites
* A C++ compiler (`g++`)
* Docker (Optional, for containerized running)

### Windows Compilation (MinGW)
Ensure you have MinGW-w64 installed. Open your terminal in the project directory and run:
```bash
g++ -O3 server.cpp -o server -D_WIN32_WINNT=0x0A00 -lws2_32 -lpthread
./server
