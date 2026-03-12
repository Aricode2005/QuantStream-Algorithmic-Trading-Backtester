# STAGE 1: Build (Force version to Bookworm)
FROM gcc:12.2-bookworm AS builder
WORKDIR /usr/src/app

COPY . .

# Compile for Linux
RUN g++ -O3 server.cpp -o server -lpthread

# STAGE 2: Run (Use the MATCHING OS version)
FROM debian:bookworm-slim
WORKDIR /app

# Install runtime libraries needed for C++
RUN apt-get update && apt-get install -y libstdc++6 && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/src/app/server .
COPY --from=builder /usr/src/app/index.html .
COPY --from=builder /usr/src/app/adani_ports.csv .

EXPOSE 10000
CMD ["./server"]