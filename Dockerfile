FROM debian:bookworm-slim
WORKDIR /app

RUN apt-get update && apt-get install -y g++ libstdc++6

COPY . .

RUN g++ -O3 -std=c++17 -Iinclude -I. server.cpp src/*.cpp -o server -lpthread

EXPOSE 10000
CMD ["./server"]
