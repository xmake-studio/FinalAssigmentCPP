#include "ScheduleRecord.h"

#include <array>
#include <sstream>
#include <stdexcept>

ScheduleRecord::ScheduleRecord(int id,
                               std::string group,
                               std::string subject,
                               std::string teacher,
                               unsigned char dayOfWeek,
                               unsigned char pairNumber)
    : id_(id),
      group_(std::move(group)),
      subject_(std::move(subject)),
      teacher_(std::move(teacher)),
      dayOfWeek_(dayOfWeek),
      pairNumber_(pairNumber) {}

std::string ScheduleRecord::toCsv() const {
    std::ostringstream os;
    os << id_       << ';'
       << group_    << ';'
       << subject_  << ';'
       << teacher_  << ';'
       << static_cast<int>(dayOfWeek_)  << ';'
       << static_cast<int>(pairNumber_);
    return os.str();
}

bool ScheduleRecord::fromCsv(const std::string& line, ScheduleRecord& out) {
    if (line.empty()) return false;

    // разделитель ';' выбран чтобы не конфликтовать с запятыми в русских строках
    std::array<std::string, 6> fields;
    std::size_t fieldIdx = 0;
    std::string current;
    for (char c : line) {
        if (c == ';') {
            if (fieldIdx >= fields.size()) return false;
            fields[fieldIdx++] = current;
            current.clear();
        } else if (c != '\r') {
            // \r игнорируем чтобы корректно читать файлы с crlf
            current.push_back(c);
        }
    }
    if (fieldIdx >= fields.size()) return false;
    fields[fieldIdx++] = current;
    if (fieldIdx != fields.size()) return false;

    try {
        int id     = std::stoi(fields[0]);
        int day    = std::stoi(fields[4]);
        int pair   = std::stoi(fields[5]);
        if (day  < kMinDay  || day  > kMaxDay)  return false;
        if (pair < kMinPair || pair > kMaxPair) return false;

        out.setId(id);
        out.setGroup(fields[1]);
        out.setSubject(fields[2]);
        out.setTeacher(fields[3]);
        out.setDayOfWeek(static_cast<unsigned char>(day));
        out.setPairNumber(static_cast<unsigned char>(pair));
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string ScheduleRecord::dayName(unsigned char day) {
    static const char* names[] = {
        "Понедельник", "Вторник", "Среда", "Четверг",
        "Пятница", "Суббота", "Воскресенье"
    };
    if (day < kMinDay || day > kMaxDay) return "?";
    return names[day - 1];
}

bool ScheduleRecord::validate(std::string& errOut) const {
    if (group_.empty())   { errOut = "пустое поле 'группа'";        return false; }
    if (subject_.empty()) { errOut = "пустое поле 'предмет'";       return false; }
    if (teacher_.empty()) { errOut = "пустое поле 'преподаватель'"; return false; }
    if (dayOfWeek_  < kMinDay  || dayOfWeek_  > kMaxDay)  {
        errOut = "день недели должен быть в диапазоне 1..7";
        return false;
    }
    if (pairNumber_ < kMinPair || pairNumber_ > kMaxPair) {
        errOut = "номер пары должен быть в диапазоне 1..4";
        return false;
    }
    return true;
}
