#include "Menu.h"
#include "ScheduleDB.h"

#include <exception>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    // иначе кириллица в консоли превратится в кашу
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // путь к бд: из аргумента, иначе дефолтный
    std::string dataPath = (argc > 1) ? argv[1] : "data/schedule.csv";

    std::cout << "База данных: Расписание (вариант 12)\n";
    std::cout << "Файл по умолчанию: " << dataPath << '\n';

    try {
        ScheduleDB db;
        Menu menu(db, dataPath);
        menu.run();
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
