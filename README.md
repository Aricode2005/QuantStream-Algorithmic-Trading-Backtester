# 📈 QuantStream: C++ Algorithmic Backtesting Engine

**Live Deployment:** https://quantstream-algorithmic-trading-2sby.onrender.com/

QuantStream is an institutional-grade, event-driven backtesting engine designed for quantitative trading strategies. Built with a modular **C++ backend** and a modern, responsive **Web UI**, it processes historical OHLC (Open, High, Low, Close) market data to simulate complex trading algorithms. The engine provides statistical insights including market regime classification (Hurst Exponent), annualized Sharpe Ratio, Max Drawdown, and intraday risk management.

## ⚡ Core Architecture & Verified Specifications

### 1. Implemented Algorithmic Strategies (5 Total)
* **SMA Crossover:** Dual moving average momentum trend-following (`SMACrossover`).
* **EMA Crossover:** Exponentially weighted moving average with faster reaction (`EMACrossover`).
* **Bollinger Bands:** Mean-reversion volatility channel strategy (`BollingerBands`).
* **RSI Momentum:** Wilder-style Relative Strength Index momentum oscillator (`RSIMomentum`).
* **Z-Score Arbitrage:** Rolling standard score anomaly detection (`ZScoreArbitrage`).

### 2. Risk Management Architecture
* **Position Sizing:** Capital-proportional position sizing allocating a user-defined risk fraction of available liquid cash per trade (`orderQty = (availableCash * riskPct) / price`).
* **Intraday Stop-Loss:** Enforces a 5% stop-loss evaluated against bar `Low` price to protect against intraday drawdowns and prevent close-only survivor bias.
* **Exposure Limits:** Strict cash-constrained, unleveraged capital ceiling (100% max equity, long-only, zero debt/margin allowance).

### 3. Benchmarked Performance & Datasets
* **Engine Throughput:** **~28.6 Million ticks/second** (compiled with `-O2` on C++17, median of 5 benchmark runs across 1,000,000+ ticks).
* **Historical Datasets Tested:**
  * **NIFTY 50 Daily (2015–2025):** 2,459 trading days downloaded from Yahoo Finance (`^NSEI`).
  * **Binance BTC/USDT Daily (2017–2025):** 3,000 trading days downloaded from Binance Public API.

### 4. Market Regime Analysis (Hurst Exponent)
* **Regime Classification:** Validated against synthetic benchmarks:
  * Synthetic Random Walk: $H \approx 0.499$ (uncorrelated increments)
  * Synthetic Momentum Trend: $H \approx 0.878$ (positively correlated returns)
  * Synthetic Mean-Reverting: $H \approx 0.354$ (Ornstein-Uhlenbeck process)
* **Empirical Finding:** A/B testing demonstrates that rolling Hurst exponent **classifies market regimes** effectively, but gating strategies dynamically on rolling Hurst does not improve Sharpe or total returns due to estimation lag during inflection points.

## 📁 Project Structure

```text
├── Dockerfile              # Docker build & deployment config for Render
├── Makefile                # Build automation
├── httplib.h               # Multithreaded HTTP Server library
├── json.hpp                # JSON serialization library
├── index.html              # Frontend dashboard
├── server.cpp              # HTTP Server entry point & routing
├── benchmark_tests.cpp     # Standalone engine benchmarking suite
├── data/                   # Historical market datasets
│   ├── nifty50_daily_2015_2025.csv
│   └── btcusdt_daily_2017_2025.csv
├── include/                # Modular Header files
│   ├── Analytics.hpp       # Hurst exponent, Sharpe Ratio, Max Drawdown
│   ├── BollingerBands.hpp  # Bollinger Bands strategy
│   ├── EMACrossover.hpp    # EMA Crossover strategy
│   ├── Portfolio.hpp       # Portfolio & execution state
│   ├── RSIMomentum.hpp     # RSI Momentum strategy
│   ├── SMACrossover.hpp    # SMA Crossover strategy
│   ├── Strategy.hpp        # Base strategy interface
│   ├── Types.hpp           # Market data structures (Tick)
│   └── ZScoreArbitrage.hpp # Z-Score Arbitrage strategy
└── src/                    # Modular Source files
    ├── Analytics.cpp
    ├── BollingerBands.cpp
    ├── EMACrossover.cpp
    ├── Portfolio.cpp
    ├── RSIMomentum.cpp
    ├── SMACrossover.cpp
    └── ZScoreArbitrage.cpp
```

## 🛠️ Tech Stack

* **Backend:** C++17, `cpp-httplib` (Multithreaded HTTP Server), `nlohmann/json` (Data Serialization)
* **Frontend:** HTML5, CSS3, Vanilla JavaScript, Chart.js (Dual-axis plotting)
* **Infrastructure:** Docker, Render

## 🚀 Local Installation & Setup

### Prerequisites
* A C++17 compatible compiler (`g++`)
* Docker (Optional, for containerized deployment)

### Linux / macOS Compilation
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

### Running the Engine Benchmarks
```bash
g++ -O2 -std=c++17 -Iinclude -I. benchmark_tests.cpp src/*.cpp -o benchmark.exe
./benchmark.exe
```
