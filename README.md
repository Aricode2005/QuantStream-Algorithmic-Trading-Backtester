# 📈 QuantStream: C++ Algorithmic Backtesting Engine

**Live Deployment:** https://quantstream-algorithmic-trading-2sby.onrender.com/

QuantStream is an institutional-grade, event-driven backtesting engine designed for quantitative trading strategies. Built with a modular **C++ backend** and a modern, responsive **Web UI**, it processes historical OHLC (Open, High, Low, Close) market data to simulate complex trading algorithms. The engine provides deep statistical insights, including market regime memory (Hurst Exponent) and realistic intraday risk management.

## ⚡ Core Architecture & Features

* **Event-Driven C++ Engine:** Processes market data chronologically tick-by-tick, preventing look-ahead bias and accurately simulating real-world execution latency.
* **OHLC Intraday Risk Management:** Goes beyond standard "Close-only" backtesters by utilizing the intraday `Low` price to enforce a dynamic 5% Stop-Loss, ensuring survival against flash crashes and preventing mathematical survivor bias.
* **Multiple Algorithmic Strategies:**
    * **SMA & EMA Crossover:** Trend-following momentum strategies.
    * **Bollinger Bands:** Mean-reversion statistical channel strategy.
    * **RSI Momentum:** Relative Strength Index oscillator (J. Welles Wilder).
    * **Z-Score Arbitrage:** Standard deviation-based anomaly detection.
* **Quantitative Analytics (AI Insights):**
    * **Hurst Exponent (H):** Diagnoses the market regime (Trending, Mean-Reverting, or Random Walk) to validate strategy deployment.
* **Dynamic Web Dashboard:**
    * Advanced regex CSV parsing with automatic Currency Symbol detection.
    * Interactive dual-axis **Portfolio Value vs. Asset Price** curve via Chart.js.
    * Real-time execution log detailing specific BUY, SELL, and STOP LOSS triggers.

## 📁 Project Structure

```text
├── Dockerfile              # Docker build & deployment config for Render
├── Makefile                # Build automation
├── httplib.h               # Multithreaded HTTP Server library
├── json.hpp                # JSON serialization library
├── index.html              # Frontend dashboard
├── server.cpp              # HTTP Server entry point & routing
├── include/                # Header files
│   ├── Analytics.hpp       # Statistical calculations (Hurst Exponent)
│   ├── BollingerBands.hpp  # Bollinger Bands strategy interface
│   ├── EMACrossover.hpp    # EMA Crossover strategy interface
│   ├── Portfolio.hpp       # Portfolio management interface
│   ├── RSIMomentum.hpp     # RSI Momentum strategy interface
│   ├── SMACrossover.hpp    # SMA Crossover strategy interface
│   ├── Strategy.hpp        # Base strategy interface
│   ├── Types.hpp           # Data structures (Tick)
│   └── ZScoreArbitrage.hpp # Z-Score Arbitrage strategy interface
└── src/                    # Source files
    ├── Analytics.cpp
    ├── BollingerBands.cpp
    ├── EMACrossover.cpp
    ├── Portfolio.cpp
    ├── RSIMomentum.cpp
    ├── SMACrossover.cpp
    └── ZScoreArbitrage.cpp
```

## 🛠️ Tech Stack

* **Backend:** C++ (Standard Library), `cpp-httplib` (Multithreaded HTTP Server), `nlohmann/json` (Data Serialization)
* **Frontend:** HTML5, CSS3, Vanilla JavaScript, Chart.js (Dual-axis plotting)
* **Infrastructure:** Docker, Render

## 🚀 Local Installation & Setup

### Prerequisites
* A C++17 compatible compiler (`g++`)
* Docker (Optional, for containerized deployment)

### Linux / macOS Compilation
Open your terminal in the project directory and run:
```bash
# Using Makefile
make
./server

# Or direct compilation:
g++ -O3 -std=c++17 -Iinclude -I. server.cpp src/*.cpp -o server -lpthread
./server
```

### Windows Compilation (MinGW)
```bash
g++ -O3 -std=c++17 -Iinclude -I. server.cpp src/*.cpp -o server.exe -lws2_32
./server.exe
```
