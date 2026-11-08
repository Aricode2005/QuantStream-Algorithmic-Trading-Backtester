# 📈 QuantStream: C++ Algorithmic Backtesting Engine

**Live Deployment:** https://quantstream-algorithmic-trading-2sby.onrender.com/

QuantStream is an institutional-grade, event-driven backtesting engine designed for quantitative trading strategies. Built with a modular **C++ backend** and a modern, responsive **Web UI**, it processes historical OHLC (Open, High, Low, Close) market data to simulate complex trading algorithms. The engine provides statistical insights including market regime classification (Hurst Exponent), annualized Sharpe Ratio, Max Drawdown, and realistic intraday risk controls.

## ⚡ Core Architecture & Verified Specifications

### 1. Implemented Algorithmic Strategies (5 Total)
* **SMA Crossover:** Dual moving average momentum trend-following (`SMACrossover`).
* **EMA Crossover:** Exponentially weighted moving average with dynamic alpha weighting (`EMACrossover`).
* **Bollinger Bands:** Mean-reversion volatility channel strategy (`BollingerBands`).
* **RSI Momentum:** Wilder-style Relative Strength Index momentum oscillator (`RSIMomentum`).
* **Z-Score Arbitrage:** Rolling standard score anomaly detection (`ZScoreArbitrage`).

### 2. Risk Management Module
* **Position Sizing:** Capital-proportional position sizing allocating a user-defined risk fraction (default 10%) of available liquid cash per trade (`orderQty = (availableCash * riskPct) / price`).
* **Intraday Stop-Loss:** Enforces a dynamic 5% stop-loss evaluated against bar `Low` price to protect against intraday flash crashes and prevent close-only survivor bias.

### 3. Historical Datasets Tested
* **NIFTY 50 Daily (2015–2024):** 2,459 trading days (Jan 2, 2015 – Dec 31, 2024) downloaded from Yahoo Finance (`^NSEI`).
* **Binance BTC/USDT Daily (2017–2025):** 3,000 continuous calendar days (Aug 17, 2017 – Nov 2, 2025, 0 missing days/gaps) downloaded from Binance Public API.

### 4. Throughput Benchmarks (Ticks/sec @ `-O2`)
Benchmarked on **1,000,813 ticks** (cyclic in-memory buffer of market data in L3 cache) on an **Intel(R) Core(TM) i5-1235U** (12 logical threads, up to 4.4 GHz Turbo) compiled with `g++ -O2 -std=c++17`.

| Strategy | Median Throughput | Latency / Bar | Run Variance [Min – Max] |
| :--- | :--- | :--- | :--- |
| **EMA Crossover** | **116.3M ticks/sec** | ~8.6 ns | [109.7M – 119.1M] |
| **SMA Crossover** | **31.0M ticks/sec** | ~32.2 ns | [27.5M – 32.0M] |
| **Z-Score Arbitrage** | **30.6M ticks/sec** | ~32.7 ns | [26.1M – 33.8M] |
| **Bollinger Bands** | **27.5M ticks/sec** | ~36.3 ns | [27.2M – 32.4M] |
| **RSI Momentum** | **11.5M ticks/sec** | ~86.8 ns | [6.8M – 13.0M] |
| **5-Strategy Average**| **43.4M ticks/sec** | **~23.0 ns** | — |

### 5. Hurst Exponent Analysis & A/B Experiment

#### A. Synthetic Verification
The 2-lag variance ratio estimator $H = 0.5 \log_2\left(\frac{\operatorname{Var}(\Delta_2)}{\operatorname{Var}(\Delta_1)}\right)$ was verified on 5,000-point synthetic benchmarks:
* **Synthetic Random Walk:** $H = \mathbf{0.4988}$ (Theoretical benchmark: $\approx 0.50$).
* **Synthetic Autoregressive Momentum ($r_t = 0.7 r_{t-1} + \epsilon_t$):** $H = \mathbf{0.8775}$ (Theoretical benchmark: $> 0.50$).
* **Synthetic Mean-Reverting (Ornstein-Uhlenbeck):** $H = \mathbf{0.3544}$ (Theoretical benchmark: $< 0.50$).

#### B. Empirical Regime Distribution
Analyzing a 60-day rolling window across real market data reveals:
* **NIFTY 50 Daily:** Mean $H = 0.515$ (Median: $0.524$, $\sigma = 0.102$), with $61.2\%$ of bars $H > 0.50$.
* **Binance BTC/USDT Daily:** Mean $H = 0.453$ (Median: $0.462$, $\sigma = 0.106$), with only **$35.9\%$ of bars $H > 0.50$** (the filter blocks entries $64.1\%$ of the time).

#### C. A/B Strategy Performance vs. Buy-and-Hold
Signals evaluate on past price history up to bar $t-1$ without lookahead.

##### NIFTY 50 Daily (2015–2024) — 2,459 Bars
* **Buy-and-Hold Benchmark:** **+181.64%** return | Sharpe **0.72** | Max Drawdown **38.44%**
* **SMA Crossover (10% sizing):** **+10.17%** return | Sharpe **0.74** | Max Drawdown **3.15%** (30 trades)
* **EMA Crossover (10% sizing):** **+11.46%** return | Sharpe **0.78** | Max Drawdown **1.91%** (37 trades)
* *With Hurst Regime Filter ($H > 0.50$):* SMA Return **+0.36%** (Sharpe 0.05) | EMA Return **+0.69%** (Sharpe 0.09)
* *Note on NIFTY:* Low drawdown (1.9–3.2%) and modest return (+10–11%) stem from conservative 10% position sizing and significant cash drag, which avoided the 38.4% COVID crash but trailed total buy-and-hold index compounding.

##### Binance BTC/USDT Daily (2017–2025) — 3,000 Bars
* **Buy-and-Hold Benchmark:** **+2,479.66%** return | Sharpe **0.92** | Max Drawdown **83.19%**
* **SMA Crossover (10% sizing):** **+410.63%** return | Sharpe **0.82** | Max Drawdown **30.16%** (370 trades)
* **EMA Crossover (10% sizing):** **+653.70%** return | Sharpe **0.92** | Max Drawdown **28.78%** (379 trades)
* *With Hurst Regime Filter ($H > 0.50$):* SMA Return **+13.36%** (Sharpe 0.17) | EMA Return **+9.24%** (Sharpe 0.13)
* *Note on BTC:* Trend-following reduced maximum drawdown from **83.2% down to ~28.8%**. The dramatic return reduction under the Hurst filter is primarily because this rolling variance ratio sits below 0.50 on BTC for 64.1% of all bars, blocking trades during explosive momentum breakouts; this underperformance is likely due to rolling estimator noise and lag, and applies specifically to this 2-lag estimator on these strategies rather than Hurst filtering in general.

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
│   ├── nifty50_daily_2015_2024.csv
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
make
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
