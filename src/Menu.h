#pragma once

#include "ScheduleDB.h"

// консольная оболочка над ScheduleDB
class Menu {
public:
    explicit Menu(ScheduleDB& db, std::string defaultPath);
    void run();

private:
    void showMenu() const;
    void doLoad();
    void doSave();
    void doView();
    void doAdd();
    void doRemove();
    void doSort();
    void doSearch();
    void doFilterByDayRange();
    void doConflicts();
    void doGroupSchedule();

    bool readInt(const char* prompt, int& out, int minV, int maxV);
    bool readLine(const char* prompt, std::string& out);

    ScheduleDB& db_;
    std::string defaultPath_;
};
