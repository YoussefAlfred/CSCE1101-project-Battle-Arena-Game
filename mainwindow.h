#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsScene>

#include "GameManager.h"
#include "Warrior.h"
#include "Mage.h"
#include "Archer.h"
#include "Character.h"

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

    // the graphics scene
    QGraphicsScene* scene;

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
    void drawGrid();
};

#endif
