# 📈 QuantStream: C++ Algorithmic Backtesting Engine

**Live Deployment:** https://quantstream-algorithmic-trading-2sby.onrender.com/

QuantStream is an event-driven backtesting engine designed for quantitative trading strategies. Built with a modular **C++ backend** and a modern, responsive **Web UI**, it processes historical OHLC (Open, High, Low, Close) market data to simulate complex trading algorithms. The engine provides statistical insights including market regime classification (Hurst Exponent), annualized Sharpe Ratio, Max Drawdown, and a shared risk module (position sizing and intraday stop-loss).

## ⚡ Architecture & Verified Results

Everything below is produced by one command from the repo root:

```
g++ -O2 -std=c++17 -Iinclude -I. benchmark_tests.cpp src/*.cpp -o benchmark && ./benchmark
```

### 1. Design

- **`Strategy` (Template Method):** `evaluateTrade()` fixes the order of operations — stop-loss check, indicator update, signal handling, order sizing. Each strategy implements only `generateSignal()` and returns `Buy`, `Sell` or `Hold`.
- **`RiskManager` (shared by every strategy):** capital-proportional position sizing, `floor(cash × riskPct / price)`, and a 5% stop-loss evaluated on the bar's `Low`, measured from the volume-weighted average entry price. If a bar gaps open below the stop, the fill is at the `Open`.
- **`Portfolio`:** long-only, unleveraged, whole-unit quantities, optional proportional fee per trade.
- **Regime filters** block new entries only. Indicators keep updating and exits are never blocked.

### 2. Strategies

| Class | Logic | Family |
|:--|:--|:--|
| `SMACrossover` | 10/50 simple moving-average crossover | trend |
| `EMACrossover` | 10/50 exponential moving-average crossover | trend |
| `BollingerBands` | buy below, sell above 20-bar mean ± 2σ | mean reversion |
| `RSIMomentum` | 14-bar RSI (simple-average variant, not Wilder smoothing), buy < 30, sell > 70 | mean reversion |
| `ZScoreArbitrage` | 20-bar rolling z-score, entry at ±2 (single-asset, not pairs arbitrage) | mean reversion |

> **Known limitation:** `BollingerBands(20, 2.0)` and `ZScoreArbitrage(20, 2.0)` generate identical signals (price vs. mean ± 2σ over 20 bars), so their results below match exactly. They are one distinct strategy in practice.

### 3. Datasets

- **NIFTY 50 daily, 2015–2024:** 2,459 bars (2015-01-02 to 2024-12-31), Yahoo Finance `^NSEI`.
- **BTC/USDT daily, 2017–2025:** 3,000 consecutive calendar days (2017-08-17 to 2025-11-02), no gaps, Binance public API.

### 4. Throughput

Single thread, `g++ -O2`, median of 5 runs. The benchmark replays NIFTY bars cyclically to reach 1M bars and times `evaluateTrade` only (no file I/O). The working set is far larger than any CPU cache. Absolute numbers depend on the CPU; the run below is on an Intel Xeon @ 2.1 GHz cloud VM. Re-run `./benchmark` on your own machine.

```
SMACrossover          16.6 M bars/s  ~ 60.2 ns/bar  [15.8 - 18.7 M]
EMACrossover          43.7 M bars/s  ~ 22.9 ns/bar  [32.3 - 45.0 M]
BollingerBands        24.6 M bars/s  ~ 40.7 ns/bar  [23.9 - 24.9 M]
RSIMomentum           11.2 M bars/s  ~ 89.2 ns/bar  [10.9 - 11.9 M]
ZScoreArbitrage       19.9 M bars/s  ~ 50.3 ns/bar  [17.8 - 22.5 M]
All five on every bar: ~263 ns/bar => ~3.8 M bars/s
```

### 5. Strategy results and the Hurst filter

Starting cash 100,000, 10% of available cash per entry, 5% stop-loss. "AvgExp%" is the average share of equity held in the asset. Sharpe uses √252 for NIFTY and √365 for BTC, applied to both strategies and buy-and-hold. The "Hurst" rows allow new entries only when the rolling 60-bar Hurst estimate (2-lag variance ratio, computed from bars strictly before the current one) favours the strategy's family: H > 0.5 for trend, H < 0.5 for mean reversion.

```
## NIFTY 50 daily (2459 bars, 2015-01-02 to 2024-12-31, Sharpe annualised with sqrt(252.0))

Buy & hold: ret 181.6%  Sharpe 0.72  MaxDD 38.4%

Strategy        Config                     Ret%  Sharpe  MaxDD%  Trades  AvgExp%  GateOn%
SMACrossover    baseline, no fees          10.2    0.74     3.1      30      5.5    100.0
SMACrossover    baseline, 0.10% fee        10.1    0.76     2.8      29      5.5    100.0
SMACrossover    Hurst H>0.5, 0.10% fee      1.9    0.23     2.8      18      2.5     61.2
EMACrossover    baseline, no fees          11.4    0.78     1.9      37      6.3    100.0
EMACrossover    baseline, 0.10% fee        11.0    0.75     2.0      37      6.3    100.0
EMACrossover    Hurst H>0.5, 0.10% fee      3.0    0.29     2.0      25      3.2     61.2
BollingerBands  baseline, no fees           3.5    0.29     2.4      44      2.9    100.0
BollingerBands  baseline, 0.10% fee         3.0    0.25     2.5      44      2.9    100.0
BollingerBands  Hurst H<0.5, 0.10% fee     -0.0   -0.00     1.7      22      1.2     38.8
RSIMomentum     baseline, no fees          -0.2   -0.01     4.3      47      2.1    100.0
RSIMomentum     baseline, 0.10% fee        -0.4   -0.02     4.4      46      2.0    100.0
RSIMomentum     Hurst H<0.5, 0.10% fee     -0.5   -0.04     3.7      26      1.1     38.8
ZScoreArbitrage baseline, no fees           3.5    0.29     2.4      44      2.9    100.0
ZScoreArbitrage baseline, 0.10% fee         3.0    0.25     2.5      44      2.9    100.0
ZScoreArbitrage Hurst H<0.5, 0.10% fee     -0.0   -0.00     1.7      22      1.2     38.8

## BTC/USDT daily (3000 bars, 2017-08-17 to 2025-11-02, Sharpe annualised with sqrt(365.0))

Buy & hold: ret 2479.7%  Sharpe 0.92  MaxDD 83.2%

Strategy        Config                     Ret%  Sharpe  MaxDD%  Trades  AvgExp%  GateOn%
SMACrossover    baseline, no fees        1497.8    1.20    43.3     510     29.5    100.0
SMACrossover    baseline, 0.10% fee      1319.8    1.17    43.9     487     28.5    100.0
SMACrossover    Hurst H>0.5, 0.10% fee    194.7    0.77    38.4      98      7.6     35.7
EMACrossover    baseline, no fees        1373.7    1.18    41.7     419     30.5    100.0
EMACrossover    baseline, 0.10% fee      1173.5    1.13    42.1     413     29.1    100.0
EMACrossover    Hurst H>0.5, 0.10% fee    188.4    0.76    37.1      81      7.4     35.7
BollingerBands  baseline, no fees          -5.2   -0.35     7.2      59      0.5    100.0
BollingerBands  baseline, 0.10% fee        -5.7   -0.38     7.6      59      0.5    100.0
BollingerBands  Hurst H<0.5, 0.10% fee     -4.3   -0.43     4.7      38      0.3     64.3
RSIMomentum     baseline, no fees          -0.0    0.02    10.9     137      2.0    100.0
RSIMomentum     baseline, 0.10% fee        -1.0   -0.01    11.5     136      2.0    100.0
RSIMomentum     Hurst H<0.5, 0.10% fee     -3.2   -0.09     7.7      71      1.2     64.3
ZScoreArbitrage baseline, no fees          -5.2   -0.35     7.2      59      0.5    100.0
ZScoreArbitrage baseline, 0.10% fee        -5.7   -0.38     7.6      59      0.5    100.0
ZScoreArbitrage Hurst H<0.5, 0.10% fee     -4.3   -0.43     4.7      38      0.3     64.3
```

**What the results say**

- **Trend strategies add risk-adjusted return only on BTC.** On NIFTY, Sharpe is 0.74–0.78 vs 0.72 for buy-and-hold; the drawdown is small only because average exposure is ~6% (mostly cash). On BTC, the trend strategies reach Sharpe ~1.2 vs 0.92 for buy-and-hold, with a 42–43% max drawdown vs 83%. These are in-sample results on one historical path with untuned parameters, during a strong BTC bull run.
- **The Hurst entry filter did not help on either dataset or in either strategy family.** Trend strategies lost Sharpe on both datasets, and mean-reversion strategies were flat or worse. The estimator is a 2-lag variance ratio on a 60-bar window and is noisy; the conclusion applies to this estimator, not to Hurst-based filtering in general.
- **Mean-reversion strategies show no edge here:** Sharpe ≤ 0.29 on NIFTY and between −0.38 and 0.02 on BTC.
- **Fees:** 0.10% per trade costs under 0.5 return points on NIFTY. On BTC the trend strategies lose about 180–200 return points (roughly 12–15% of the no-fee return).
- **Whole-unit sizing:** quantities are integers, so an entry is impossible once one unit costs more than 10% of available cash. On NIFTY (index ≈ 8,000–24,000) and late-sample BTC (≈ 100,000) this makes entries coarse or impossible at 100,000 starting cash.

#### Hurst estimator validation (synthetic series, 5,000 points)

| Series | H | Expected |
|:--|:--|:--|
| Random walk | 0.4988 | ≈ 0.50 |
| AR(1) momentum, r = 0.7 | 0.8775 | > 0.50 |
| Ornstein–Uhlenbeck | 0.3544 | < 0.50 |

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
├── results/                # Saved output of ./benchmark
├── include/                # Modular Header files
│   ├── Analytics.hpp       # Hurst exponent, Sharpe Ratio, Max Drawdown
│   ├── BollingerBands.hpp  # Bollinger Bands strategy
│   ├── EMACrossover.hpp    # EMA Crossover strategy
│   ├── Portfolio.hpp       # Portfolio & execution state
│   ├── RiskManager.hpp     # Shared position sizing & stop-loss
│   ├── RSIMomentum.hpp     # RSI Momentum strategy
│   ├── SMACrossover.hpp    # SMA Crossover strategy
│   ├── Strategy.hpp        # Template-method base class
│   ├── Types.hpp           # Market data structures (Tick)
│   └── ZScoreArbitrage.hpp # Z-Score Arbitrage strategy
└── src/                    # Modular Source files
    ├── Analytics.cpp
    ├── BollingerBands.cpp
    ├── EMACrossover.cpp
    ├── Portfolio.cpp
    ├── RiskManager.cpp
    ├── Strategy.cpp
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
