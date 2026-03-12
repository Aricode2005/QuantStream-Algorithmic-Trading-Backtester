# Use the official Debian C++ environment for both building and running
FROM debian:bookworm-slim
WORKDIR /app

# Install the exact compiler and libraries for this specific Debian version
RUN apt-get update && apt-get install -y g++ libstdc++6

# Copy your files
COPY . .

# Compile the engine
RUN g++ -O3 server.cpp -o server -lpthread

# Expose port and run
EXPOSE 10000
CMD ["./server"]
