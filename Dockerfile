# STAGE 1: Build Environment
FROM gcc:bookworm AS builder
WORKDIR /app

# Copy only the 4 essential files
COPY server.cpp .
COPY index.html .
COPY httplib.h .
COPY json.hpp .

# Compile the C++ code for Linux
RUN g++ -O3 server.cpp -o server -lpthread

# STAGE 2: Minimal Runtime Environment
FROM debian:bookworm-slim
WORKDIR /app

# Install the required C++ standard library
RUN apt-get update && apt-get install -y libstdc++6 && rm -rf /var/lib/apt/lists/*

# Copy the compiled server and the HTML file from the builder stage
COPY --from=builder /app/server .
COPY --from=builder /app/index.html .

# Expose Render's default port
EXPOSE 10000

# Start the server
CMD ["./server"]
