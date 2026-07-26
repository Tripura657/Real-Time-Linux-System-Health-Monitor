# Linux Real-Time System Health Monitor

A lightweight C++ system daemon designed to monitor real-time CPU and Memory utilization on Linux environments by parsing virtual filesystem interfaces.

## Features
- **Kernel Interface Parsing:** Reads metrics directly from `/proc/stat` and `/proc/meminfo`.
- **Asynchronous Telemetry:** Logs resource threshold alerts using non-blocking daemon threads (`std::thread::detach`).
- **Thread Safety:** Enforces safe log writes across concurrent operations using `std::mutex` and `std::lock_guard`.

## Compilation & Usage
```bash
# Compile with C++17 support and pthread library
g++ -std=c++17 sys_monitor.cpp -lpthread -o sys_monitor

# Run the daemon
./sys_monitor
