#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <array>

// ===================== DATA STRUCTURES =====================

struct ZoneCount
{
    std::string zone;
    long long count;
};

struct SlotCount
{
    std::string zone;
    int hour;
    long long count;
};

// ===================== CLASS DEFINITION =====================

class TripAnalyzer
{
private:
    struct ZoneStats
    {
        long long totalTrips = 0;
        std::array<long long, 24> hourlyTrips = {0};
    };

    std::unordered_map<std::string, ZoneStats> m_data;

public:
    void ingestFile(const std::string &csvPath);
    void ingestStdin(); // added for HackerRank Compatiblity
    std::vector<ZoneCount> topZones(int k = 10) const;
    std::vector<SlotCount> topBusySlots(int k = 10) const;
};

#endif
