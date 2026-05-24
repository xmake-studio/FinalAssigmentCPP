#include "Menu.h"

#include <iostream>
#include <limits>
#include <sstream>
#include <string>

Menu::Menu(ScheduleDB& db, std::string defaultPath)
    : db_(db), defaultPath_(std::move(defaultPath)) {}

void Menu::showMenu() const {
    std::cout <<
        "\n========== Меню =========="
        "\n 1. Загрузить БД из файла"
        "\n 2. Сохранить БД в файл"
        "\n 3. Показать все записи"
        "\n 4. Добавить запись"
        "\n 5. Удалить запись по ID"
        "\n 6. Сортировать по ID"
        "\n 7. Поиск по ID"
        "\n 8. Выборка по диапазону дней недели"
        "\n 9. Проверить накладки в расписании"
        "\n10. Показать расписание группы"
        "\n 0. Выход"
        "\nВыберите пункт: ";
}

bool Menu::readInt(const char* prompt, int& out, int minV, int maxV) {
    while (true) {
        std::cout << prompt;
        std::string s;
        if (!std::getline(std::cin, s)) return false;
        // eof() гарантирует что после числа нет мусора типа "5abc"
        std::istringstream iss(s);
        int v;
        if ((iss >> v) && iss.eof() && v >= minV && v <= maxV) {
            out = v;
            return true;
        }
        std::cout << "  Ожидалось целое число в диапазоне ["
                  << minV << ".." << maxV << "]. Повторите ввод.\n";
    }
}

bool Menu::readLine(const char* prompt, std::string& out) {
    std::cout << prompt;
    if (!std::getline(std::cin, out)) return false;
    return true;
}

void Menu::run() {
    // пробуем подхватить файл по умолчанию, отсутствие не критично
    try {
        std::size_t n = db_.loadFromFile(defaultPath_);
        std::cout << "Загружено записей из '" << defaultPath_ << "': " << n << '\n';
    } catch (const std::exception& e) {
        std::cout << "Стартовая загрузка пропущена: " << e.what() << '\n';
    }

    while (true) {
        showMenu();
        int choice = -1;
        if (!readInt("", choice, 0, 10)) {
            std::cout << "\nЗавершение работы (EOF).\n";
            return;
        }
        switch (choice) {
            case 0: std::cout << "До свидания!\n"; return;
            case 1: doLoad();              break;
            case 2: doSave();              break;
            case 3: doView();              break;
            case 4: doAdd();               break;
            case 5: doRemove();            break;
            case 6: doSort();              break;
            case 7: doSearch();            break;
            case 8: doFilterByDayRange();  break;
            case 9: doConflicts();         break;
            case 10: doGroupSchedule();    break;
            default: std::cout << "Неизвестный пункт меню.\n";
        }
    }
}

void Menu::doLoad() {
    std::string path;
    readLine("Путь к файлу [Enter — по умолчанию]: ", path);
    if (path.empty()) path = defaultPath_;
    try {
        std::size_t n = db_.loadFromFile(path);
        std::cout << "Загружено записей: " << n << '\n';
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << '\n';
    }
}

void Menu::doSave() {
    std::string path;
    readLine("Путь к файлу [Enter — по умолчанию]: ", path);
    if (path.empty()) path = defaultPath_;
    try {
        db_.saveToFile(path);
        std::cout << "БД сохранена в '" << path << "' (" << db_.size() << " записей).\n";
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << '\n';
    }
}

void Menu::doView() {
    db_.displayAll();
}

void Menu::doAdd() {
    ScheduleRecord rec;

    int suggestedId = db_.nextId();
    int id = suggestedId;
    std::cout << "Предлагаемый ID: " << suggestedId << '\n';
    if (!readInt("ID записи: ", id, 1, std::numeric_limits<int>::max())) return;
    rec.setId(id);

    std::string s;
    readLine("Группа: ",         s); rec.setGroup(s);
    readLine("Предмет: ",        s); rec.setSubject(s);
    readLine("Преподаватель: ",  s); rec.setTeacher(s);

    int day = 1, pair = 1;
    if (!readInt("День недели (1-Пн ... 7-Вс): ", day, 1, 7)) return;
    if (!readInt("Номер пары (1..4): ", pair, 1, 4)) return;
    rec.setDayOfWeek(static_cast<unsigned char>(day));
    rec.setPairNumber(static_cast<unsigned char>(pair));

    std::string err;
    if (db_.addRecord(rec, err)) {
        std::cout << "Запись добавлена.\n";
    } else {
        std::cout << "Не удалось добавить запись: " << err << '\n';
    }
}

void Menu::doRemove() {
    int id = 0;
    if (!readInt("ID записи для удаления: ", id, 1, std::numeric_limits<int>::max())) return;
    if (db_.removeById(id)) {
        std::cout << "Запись с id=" << id << " удалена.\n";
    } else {
        std::cout << "Запись с id=" << id << " не найдена.\n";
    }
}

void Menu::doSort() {
    db_.sortById();
    std::cout << "БД отсортирована по ID.\n";
}

void Menu::doSearch() {
    int id = 0;
    if (!readInt("Искомый ID: ", id, 1, std::numeric_limits<int>::max())) return;
    if (const ScheduleRecord* r = db_.findById(id)) {
        std::cout << "Найдено: " << r->toCsv() << '\n';
        std::cout << "  Группа        : " << r->group()   << '\n'
                  << "  Предмет       : " << r->subject() << '\n'
                  << "  Преподаватель : " << r->teacher() << '\n'
                  << "  День          : " << ScheduleRecord::dayName(r->dayOfWeek()) << '\n'
                  << "  Пара          : " << static_cast<int>(r->pairNumber()) << '\n';
    } else {
        std::cout << "Запись с id=" << id << " не найдена.\n";
    }
}

void Menu::doFilterByDayRange() {
    int from = 1, to = 7;
    if (!readInt("День ОТ (1..7): ", from, 1, 7)) return;
    if (!readInt("День ДО (1..7): ", to,   1, 7)) return;

    auto sub = db_.selectByDayRange(static_cast<unsigned char>(from),
                                    static_cast<unsigned char>(to));
    if (sub.empty()) {
        std::cout << "В заданном диапазоне дней записей нет.\n";
        return;
    }

    std::cout << "Найдено записей: " << sub.size() << '\n';
    for (const auto& r : sub) {
        std::cout << "  id=" << r.id() << " | " << r.group()
                  << " | " << r.subject()
                  << " | " << r.teacher()
                  << " | " << ScheduleRecord::dayName(r.dayOfWeek())
                  << " | пара " << static_cast<int>(r.pairNumber()) << '\n';
    }
}

void Menu::doConflicts() {
    std::size_t n = db_.reportConflicts();
    std::cout << "\nИтого накладок: " << n << '\n';
}

void Menu::doGroupSchedule() {
    std::string group;
    readLine("Название группы: ", group);
    if (group.empty()) {
        std::cout << "Группа не указана.\n";
        return;
    }
    db_.printGroupSchedule(group);
}
