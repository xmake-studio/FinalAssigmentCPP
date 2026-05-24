#pragma once

#include "ScheduleRecord.h"

#include <string>
#include <vector>

class ScheduleDB {
public:
    std::size_t loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;

    void displayAll() const;

    bool addRecord(const ScheduleRecord& rec, std::string& errOut);
    bool removeById(int id);

    void sortById();
    const ScheduleRecord* findById(int id);

    std::vector<ScheduleRecord> selectByDayRange(unsigned char dayFrom,
                                                 unsigned char dayTo) const;

    std::size_t reportConflicts() const;
    void printGroupSchedule(const std::string& group) const;

    const std::vector<ScheduleRecord>& records() const { return records_; }
    bool empty() const { return records_.empty(); }
    std::size_t size() const { return records_.size(); }

    int nextId() const;

private:
    std::vector<ScheduleRecord> records_;
    bool sortedById_ {false};
};
