#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>

#include "GameManager.h"
#include "../characters/Warrior.h"
#include "../characters/Mage.h"
#include "../characters/Archer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget* stackedWidget;

    // windows
    QWidget* characterPage;
    QWidget* gamePage;

    // buttons
    QPushButton* warriorButton;
    QPushButton* mageButton;
    QPushButton* archerButton;
    QPushButton* startButton;

    // labels
    QLabel* playerInfo;
    QLabel* enemyInfo;

    GameManager* gameManager;

    Character* selectedCharacter;

    void setupCharacterPage();
    void setupGamePage();
};

#endif
