#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>

std::mutex log_mutex;

// Function to read total memory and available memory from /proc/meminfo
double getMemoryUsagePercentage() {
    std::ifstream memFile("/proc/meminfo");
    std::string key;
    size_t value;
    std::string unit;

    size_t totalMem = 0, freeMem = 0, availMem = 0;

    while (memFile >> key >> value >> unit) {
        if (key == "MemTotal:") totalMem = value;
        else if (key == "MemAvailable:") availMem = value;
    }

    if (totalMem == 0) return 0.0;
    return ((double)(totalMem - availMem) / totalMem) * 100.0;
}

// Function to get raw CPU active and total time from /proc/stat
bool getCPUData(size_t &idleTime, size_t &totalTime) {
    std::ifstream statFile("/proc/stat");
    std::string line;
    if (std::getline(statFile, line)) {
        std::istringstream ss(line);
        std::string cpuHeader;
        ss >> cpuHeader; // reads "cpu"
        
        size_t user, nice, system, idle, iowait, irq, softirq, steal;
        if (ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal) {
            idleTime = idle + iowait;
            totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
            return true;
        }
    }
    return false;
}

// Asynchronous logger function running on a separate thread
void logAlertAsync(const std::string& message) {
    std::thread([message]() {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::ofstream logFile("system_alerts.log", std::ios::app);
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::string timeStr = std::ctime(&now);
            timeStr.pop_back(); // Remove trailing newline
            logFile << "[" << timeStr << "] ALERT: " << message << "\n";
        }
    }).detach(); // Run independently in background
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "   Linux Real-Time System Health Monitor   \n";
    std::cout << "===========================================\n";
    std::cout << "Monitoring system resources... (Press Ctrl+C to stop)\n\n";

    size_t prevIdle = 0, prevTotal = 0;
    getCPUData(prevIdle, prevTotal);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 1. Calculate CPU Usage %
        size_t idleTime = 0, totalTime = 0;
        getCPUData(idleTime, totalTime);
        
        size_t totalDiff = totalTime - prevTotal;
        size_t idleDiff = idleTime - prevIdle;
        
        double cpuUsage = 0.0;
        if (totalDiff > 0) {
            cpuUsage = ((double)(totalDiff - idleDiff) / totalDiff) * 100.0;
        }

        prevIdle = idleTime;
        prevTotal = totalTime;

        // 2. Calculate RAM Usage %
        double memUsage = getMemoryUsagePercentage();

        // 3. Display Status
        std::cout << "\r[Status] CPU Usage: " << cpuUsage << "% | RAM Usage: " << memUsage << "%" << std::flush;

        // 4. Real-world Alert Logic (Threshold: RAM > 70%)
        if (memUsage > 70.0) {
            std::string alertMsg = "High Memory Usage Detected: " + std::to_string(memUsage) + "%";
            logAlertAsync(alertMsg);
        }
    }

    return 0;
}
