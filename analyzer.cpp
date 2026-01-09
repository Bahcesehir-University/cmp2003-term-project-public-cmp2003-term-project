#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <vector>

using namespace std;

static void cleanString(string &s)
{
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
    {
        s = s.substr(1, s.size() - 2);
    }
}

// ===================== COMMON PARSING LOGIC =====================

void TripAnalyzer::ingestFile(const string &csvPath)
{
    ifstream file(csvPath);
    if (!file.is_open())
        return;

    string line;
    if (!getline(file, line))
        return;
    if (!line.empty() && line.back() == '\r')
        line.pop_back();

    int expectedCols = 6;
    int zoneIdx = 1;
    int timeIdx = 3;
    bool firstLineIsHeader = false;

    if (!line.empty() && !isdigit(line[0]))
    {
        stringstream ss(line);
        string col;
        int idx = 0;
        int foundZone = -1;
        int foundTime = -1;

        while (getline(ss, col, ','))
        {
            cleanString(col);
            if (col == "PickupZoneID" || col == "PULocationID")
                foundZone = idx;
            else if (col == "PickupTime" || col == "PickupDateTime" || col == "tpep_pickup_datetime")
                foundTime = idx;
            idx++;
        }

        if (foundZone != -1 || foundTime != -1)
        {
            firstLineIsHeader = true;
            expectedCols = idx;
            if (foundZone != -1)
                zoneIdx = foundZone;
            if (foundTime != -1)
                timeIdx = foundTime;
        }
    }

    auto processLine = [&](const string &row)
    {
        if (row.empty())
            return;

        vector<string> columns;
        columns.reserve(8);

        bool inQuotes = false;
        size_t tokenStart = 0;

        for (size_t i = 0; i < row.size(); ++i)
        {
            if (row[i] == '"')
            {
                inQuotes = !inQuotes;
            }
            else if (row[i] == ',' && !inQuotes)
            {
                columns.push_back(row.substr(tokenStart, i - tokenStart));
                tokenStart = i + 1;
            }
        }
        columns.push_back(row.substr(tokenStart));

        if (columns.size() != (size_t)expectedCols)
        {
            return;
        }

        string zoneStr = columns[zoneIdx];
        string timeStr = columns[timeIdx];

        if (zoneStr.empty() || timeStr.empty())
            return;

        cleanString(zoneStr);
        cleanString(timeStr);

        if (timeStr.size() < 13)
            return;

        size_t spacePos = timeStr.find(' ');
        if (spacePos == string::npos || spacePos + 2 >= timeStr.size())
            return;

        char h1 = timeStr[spacePos + 1];
        char h2 = timeStr[spacePos + 2];

        if (isdigit(h1) && isdigit(h2))
        {
            int hour = (h1 - '0') * 10 + (h2 - '0');
            if (hour >= 0 && hour <= 23)
            {
                ZoneStats &s = m_data[zoneStr];
                s.totalTrips++;
                s.hourlyTrips[hour]++;
            }
        }
    };

    if (!firstLineIsHeader)
        processLine(line);

    while (getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        processLine(line);
    }
}

// ===================== Added for HackerRank input reading =====================

void TripAnalyzer::ingestStdin()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    string line;
    if (!getline(cin, line))
        return;
    if (!line.empty() && line.back() == '\r')
        line.pop_back();

    int expectedCols = 6;
    int zoneIdx = 1;
    int timeIdx = 3;
    bool firstLineIsHeader = false;

    if (!line.empty() && !isdigit(line[0]))
    {
        stringstream ss(line);
        string col;
        int idx = 0;
        int foundZone = -1;
        int foundTime = -1;

        while (getline(ss, col, ','))
        {
            cleanString(col);
            if (col == "PickupZoneID" || col == "PULocationID")
                foundZone = idx;
            else if (col == "PickupTime" || col == "PickupDateTime" || col == "tpep_pickup_datetime")
                foundTime = idx;
            idx++;
        }

        if (foundZone != -1 || foundTime != -1)
        {
            firstLineIsHeader = true;
            expectedCols = idx;
            if (foundZone != -1)
                zoneIdx = foundZone;
            if (foundTime != -1)
                timeIdx = foundTime;
        }
    }

    auto processLine = [&](const string &row)
    {
        if (row.empty())
            return;

        vector<string> columns;
        columns.reserve(8);

        bool inQuotes = false;
        size_t tokenStart = 0;

        for (size_t i = 0; i < row.size(); ++i)
        {
            if (row[i] == '"')
            {
                inQuotes = !inQuotes;
            }
            else if (row[i] == ',' && !inQuotes)
            {
                columns.push_back(row.substr(tokenStart, i - tokenStart));
                tokenStart = i + 1;
            }
        }
        columns.push_back(row.substr(tokenStart));

        if (columns.size() != (size_t)expectedCols)
        {
            return;
        }

        string zoneStr = columns[zoneIdx];
        string timeStr = columns[timeIdx];

        if (zoneStr.empty() || timeStr.empty())
            return;

        cleanString(zoneStr);
        cleanString(timeStr);

        if (timeStr.size() < 13)
            return;

        size_t spacePos = timeStr.find(' ');
        if (spacePos == string::npos || spacePos + 2 >= timeStr.size())
            return;

        char h1 = timeStr[spacePos + 1];
        char h2 = timeStr[spacePos + 2];

        if (isdigit(h1) && isdigit(h2))
        {
            int hour = (h1 - '0') * 10 + (h2 - '0');
            if (hour >= 0 && hour <= 23)
            {
                ZoneStats &s = m_data[zoneStr];
                s.totalTrips++;
                s.hourlyTrips[hour]++;
            }
        }
    };

    if (!firstLineIsHeader)
        processLine(line);

    while (getline(cin, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        processLine(line);
    }
}

// ===================== OUTPUT LOGIC =====================

vector<ZoneCount> TripAnalyzer::topZones(int k) const
{
    vector<ZoneCount> result;
    result.reserve(m_data.size());
    for (const auto &pair : m_data)
        result.push_back({pair.first, pair.second.totalTrips});

    sort(result.begin(), result.end(), [](const ZoneCount &a, const ZoneCount &b)
         {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone; });

    if ((int)result.size() > k)
        result.resize(k);
    return result;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const
{
    vector<SlotCount> result;
    for (const auto &pair : m_data)
    {
        for (int h = 0; h < 24; ++h)
        {
            long long c = pair.second.hourlyTrips[h];
            if (c > 0)
                result.push_back({pair.first, h, c});
        }
    }

    sort(result.begin(), result.end(), [](const SlotCount &a, const SlotCount &b)
         {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone; 
        return a.hour < b.hour; });

    if ((int)result.size() > k)
        result.resize(k);
    return result;
}
