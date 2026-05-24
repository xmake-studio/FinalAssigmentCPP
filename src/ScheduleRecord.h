#pragma once

#include <string>

class ScheduleRecord {
public:
    static constexpr unsigned char kMinDay = 1;
    static constexpr unsigned char kMaxDay = 7;
    static constexpr unsigned char kMinPair = 1;
    static constexpr unsigned char kMaxPair = 4;

    ScheduleRecord() = default;
    ScheduleRecord(int id,
                   std::string group,
                   std::string subject,
                   std::string teacher,
                   unsigned char dayOfWeek,
                   unsigned char pairNumber);

    int           id()         const { return id_; }
    const std::string& group()   const { return group_; }
    const std::string& subject() const { return subject_; }
    const std::string& teacher() const { return teacher_; }
    unsigned char dayOfWeek()    const { return dayOfWeek_; }
    unsigned char pairNumber()   const { return pairNumber_; }

    void setId(int v)                    { id_ = v; }
    void setGroup(std::string v)         { group_ = std::move(v); }
    void setSubject(std::string v)       { subject_ = std::move(v); }
    void setTeacher(std::string v)       { teacher_ = std::move(v); }
    void setDayOfWeek(unsigned char v)   { dayOfWeek_ = v; }
    void setPairNumber(unsigned char v)  { pairNumber_ = v; }

    std::string toCsv() const;
    static bool fromCsv(const std::string& line, ScheduleRecord& out);
    static std::string dayName(unsigned char day);
    bool validate(std::string& errOut) const;

private:
    int           id_         {0};
    std::string   group_;
    std::string   subject_;
    std::string   teacher_;
    unsigned char dayOfWeek_  {1};
    unsigned char pairNumber_ {1};
};
