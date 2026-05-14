cmake_minimum_required(VERSION 3.16)
project(BattleArenaGame)

set(CMAKE_CXX_STANDARD 17)

# Required Qt6 modules
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Optional: Qt6 Multimedia for click/victory/game-over sounds.
# Install with: sudo apt install qt6-multimedia-dev libqt6multimedia6
find_package(Qt6 QUIET COMPONENTS Multimedia)

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

target_link_libraries(game Qt6::Widgets Qt6::Network)

if(Qt6Multimedia_FOUND)
    message(STATUS "Qt6 Multimedia found — audio enabled")
    target_link_libraries(game Qt6::Multimedia)
    target_compile_definitions(game PRIVATE BATTLE_HAS_AUDIO=1)
else()
    message(STATUS "Qt6 Multimedia NOT found — audio disabled (install qt6-multimedia-dev)")
endif()