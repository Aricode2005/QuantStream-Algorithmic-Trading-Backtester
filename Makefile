CXX = g++
CXXFLAGS = -O3 -Wall -std=c++17 -Iinclude -I.
LDFLAGS = -lpthread

SRCS = server.cpp \
       src/Portfolio.cpp \
       src/RiskManager.cpp \
       src/Strategy.cpp \
       src/SMACrossover.cpp \
       src/EMACrossover.cpp \
       src/BollingerBands.cpp \
       src/RSIMomentum.cpp \
       src/ZScoreArbitrage.cpp \
       src/Analytics.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) server.exe

bench:
	g++ -O2 -std=c++17 -Iinclude -I. benchmark_tests.cpp src/*.cpp -o benchmark && ./benchmark

.PHONY: all clean bench
