cmake_minimum_required(VERSION 3.16)
project(BattleArenaGame)

set(CMAKE_CXX_STANDARD 17)

# Find Qt (Qt6 preferred)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    main.cpp
    mainwindow.cpp
    Archer.cpp
    BattleGrid.cpp
    Cell.cpp
    Character.cpp
    GameManager.cpp
    Mage.cpp
    Warrior.cpp
)

set(HEADERS
    mainwindow.h
    Archer.h
    BattleGrid.h
    Cell.h
    Character.h
    GameManager.h
    Mage.h
    Warrior.h
)

add_executable(game ${SOURCES} ${HEADERS})

target_link_libraries(game Qt6::Widgets)
