#include "ScheduleDB.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace {

std::size_t utf8VisibleLen(const std::string& s) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < s.size(); ) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        std::size_t step;
        if      ((c & 0x80u) == 0u)    step = 1;
        else if ((c & 0xE0u) == 0xC0u) step = 2;
        else if ((c & 0xF0u) == 0xE0u) step = 3;
        else if ((c & 0xF8u) == 0xF0u) step = 4;
        else                            step = 1;
        if (i + step > s.size()) break;
        i += step;
        ++count;
    }
    return count;
}

std::string padRight(const std::string& s, std::size_t width) {
    std::size_t vis = utf8VisibleLen(s);
    if (vis >= width) return s;
    return s + std::string(width - vis, ' ');
}

constexpr std::size_t kColId      = 4;
constexpr std::size_t kColGroup   = 8;
constexpr std::size_t kColSubject = 30;
constexpr std::size_t kColTeacher = 16;
constexpr std::size_t kColDay     = 12;
constexpr std::size_t kColPair    = 4;

const char* kColSep = " | ";

std::size_t totalTableWidth() {
    return kColId + kColGroup + kColSubject + kColTeacher + kColDay + kColPair
         + 5 * 3;
}

void printTableHeader() {
    std::cout
        << padRight("ID",            kColId)      << kColSep
        << padRight("Группа",        kColGroup)   << kColSep
        << padRight("Предмет",       kColSubject) << kColSep
        << padRight("Преподаватель", kColTeacher) << kColSep
        << padRight("День",          kColDay)     << kColSep
        << padRight("Пара",          kColPair)    << '\n';
    std::cout << std::string(totalTableWidth(), '-') << '\n';
}

void printTableRow(const ScheduleRecord& r) {
    std::cout
        << padRight(std::to_string(r.id()),                 kColId)      << kColSep
        << padRight(r.group(),                              kColGroup)   << kColSep
        << padRight(r.subject(),                            kColSubject) << kColSep
        << padRight(r.teacher(),                            kColTeacher) << kColSep
        << padRight(ScheduleRecord::dayName(r.dayOfWeek()), kColDay)     << kColSep
        << padRight(std::to_string(r.pairNumber()),         kColPair)    << '\n';
}

} // namespace

std::size_t ScheduleDB::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Не удалось открыть файл: " + path);
    }

    records_.clear();
    sortedById_ = false;

    std::string line;
    std::size_t lineNum = 0;
    std::size_t loaded  = 0;
    while (std::getline(in, line)) {
        ++lineNum;
        if (line.empty()) continue;
        if (lineNum == 1 && !std::isdigit(static_cast<unsigned char>(line[0]))) {
            continue;
        }

        ScheduleRecord rec;
        if (!ScheduleRecord::fromCsv(line, rec)) {
            std::cerr << "Предупреждение: пропущена некорректная строка "
                      << lineNum << ": " << line << '\n';
            continue;
        }
        records_.push_back(std::move(rec));
        ++loaded;
    }
    return loaded;
}

void ScheduleDB::saveToFile(const std::string& path) const {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Не удалось открыть файл для записи: " + path);
    }
    out << "id;group;subject;teacher;day;pair\n";
    for (const auto& r : records_) {
        out << r.toCsv() << '\n';
    }
    if (!out) {
        throw std::runtime_error("Ошибка записи в файл: " + path);
    }
}

void ScheduleDB::displayAll() const {
    if (records_.empty()) {
        std::cout << "База данных пуста.\n";
        return;
    }
    printTableHeader();
    for (const auto& r : records_) printTableRow(r);
    std::cout << "Всего записей: " << records_.size() << '\n';
}

bool ScheduleDB::addRecord(const ScheduleRecord& rec, std::string& errOut) {
    std::string verr;
    if (!rec.validate(verr)) {
        errOut = "Невалидная запись: " + verr;
        return false;
    }
    auto it = std::find_if(records_.begin(), records_.end(),
                           [&](const ScheduleRecord& r) { return r.id() == rec.id(); });
    if (it != records_.end()) {
        errOut = "Запись с id=" + std::to_string(rec.id()) + " уже существует.";
        return false;
    }
    records_.push_back(rec);
    sortedById_ = false;
    return true;
}

bool ScheduleDB::removeById(int id) {
    auto it = std::find_if(records_.begin(), records_.end(),
                           [&](const ScheduleRecord& r) { return r.id() == id; });
    if (it == records_.end()) return false;
    records_.erase(it);
    return true;
}

void ScheduleDB::sortById() {
    std::sort(records_.begin(), records_.end(),
              [](const ScheduleRecord& a, const ScheduleRecord& b) {
                  return a.id() < b.id();
              });
    sortedById_ = true;
}

const ScheduleRecord* ScheduleDB::findById(int id) {
    if (records_.empty()) return nullptr;
    if (!sortedById_) sortById();

    ScheduleRecord key;
    key.setId(id);
    auto it = std::lower_bound(records_.begin(), records_.end(), key,
                               [](const ScheduleRecord& a, const ScheduleRecord& b) {
                                   return a.id() < b.id();
                               });
    if (it == records_.end() || it->id() != id) return nullptr;
    return &(*it);
}

std::vector<ScheduleRecord> ScheduleDB::selectByDayRange(unsigned char dayFrom,
                                                         unsigned char dayTo) const {
    if (dayFrom > dayTo) std::swap(dayFrom, dayTo);
    std::vector<ScheduleRecord> result;
    std::copy_if(records_.begin(), records_.end(), std::back_inserter(result),
                 [&](const ScheduleRecord& r) {
                     return r.dayOfWeek() >= dayFrom && r.dayOfWeek() <= dayTo;
                 });
    return result;
}

int ScheduleDB::nextId() const {
    int maxId = 0;
    for (const auto& r : records_) maxId = std::max(maxId, r.id());
    return maxId + 1;
}

std::size_t ScheduleDB::reportConflicts() const {
    using Slot = std::tuple<std::string, unsigned char, unsigned char>;

    std::map<Slot, std::vector<std::string>> byGroup;
    std::map<Slot, std::vector<std::string>> byTeacher;

    for (const auto& r : records_) {
        byGroup  [{r.group(),   r.dayOfWeek(), r.pairNumber()}].push_back(r.subject());
        byTeacher[{r.teacher(), r.dayOfWeek(), r.pairNumber()}].push_back(r.group());
    }

    std::size_t conflicts = 0;

    std::cout << "\n=== Накладки по ГРУППАМ (две и более пары одновременно) ===\n";
    bool anyGroup = false;
    for (const auto& [slot, subjects] : byGroup) {
        if (subjects.size() <= 1) continue;
        anyGroup = true;
        ++conflicts;
        const auto& [grp, day, pair] = slot;
        std::cout << "Группа " << grp << ", " << ScheduleRecord::dayName(day)
                  << ", пара " << static_cast<int>(pair) << ": ";
        for (std::size_t i = 0; i < subjects.size(); ++i) {
            std::cout << subjects[i] << (i + 1 < subjects.size() ? ", " : "");
        }
        std::cout << '\n';
    }
    if (!anyGroup) std::cout << "Накладок по группам не найдено.\n";

    std::cout << "\n=== Накладки по ПРЕПОДАВАТЕЛЯМ ===\n";
    bool anyTeacher = false;
    for (const auto& [slot, groups] : byTeacher) {
        if (groups.size() <= 1) continue;
        anyTeacher = true;
        ++conflicts;
        const auto& [teacher, day, pair] = slot;
        std::cout << "Преподаватель " << teacher << ", " << ScheduleRecord::dayName(day)
                  << ", пара " << static_cast<int>(pair) << ": ";
        for (std::size_t i = 0; i < groups.size(); ++i) {
            std::cout << groups[i] << (i + 1 < groups.size() ? ", " : "");
        }
        std::cout << '\n';
    }
    if (!anyTeacher) std::cout << "Накладок по преподавателям не найдено.\n";

    return conflicts;
}

void ScheduleDB::printGroupSchedule(const std::string& group) const {
    std::map<unsigned char, std::vector<const ScheduleRecord*>> byDay;
    for (const auto& r : records_) {
        if (r.group() == group) byDay[r.dayOfWeek()].push_back(&r);
    }

    if (byDay.empty()) {
        std::cout << "Для группы '" << group << "' записей не найдено.\n";
        return;
    }

    std::cout << "\nРасписание группы " << group << ":\n";
    for (auto& [day, items] : byDay) {
        std::sort(items.begin(), items.end(),
                  [](const ScheduleRecord* a, const ScheduleRecord* b) {
                      return a->pairNumber() < b->pairNumber();
                  });
        std::cout << "\n  " << ScheduleRecord::dayName(day) << ":\n";
        for (const auto* r : items) {
            std::cout << "    Пара " << static_cast<int>(r->pairNumber())
                      << " — " << r->subject()
                      << " (" << r->teacher() << ")\n";
        }
    }
}
