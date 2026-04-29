#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>

#include "GameManager.h"
#include "Warrior.h"
#include "Mage.h"
#include "Archer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onCharacterSelected(int type);
    void onStartClicked();
    void onGameStateChanged(GameState state);
    void onEnemyTurn();

private:
    // ── pages ───────────────────────────────────────────────
    QStackedWidget* stack;
    QWidget*        menuPage;
    QWidget*        characterPage;
    QWidget*        difficultyPage;
    QWidget*        gamePage;

    // ── character select ────────────────────────────────────
    QPushButton*    cardWarrior;
    QPushButton*    cardMage;
    QPushButton*    cardArcher;
    QWidget*        cardWarriorWidget = nullptr;
    QWidget*        cardMageWidget    = nullptr;
    QWidget*        cardArcherWidget  = nullptr;
    QPushButton*    btnStart;
    QLabel*         selectionLabel;

    // ── game HUD ────────────────────────────────────────────
    QLabel*         playerPortraitLabel = nullptr;
    QLabel*         enemyPortraitLabel  = nullptr;

    QLabel*         lblPlayerName;
    QLabel*         lblPlayerClass;
    QProgressBar*   barPlayerHP;
    QLabel*         lblPlayerHPVal;

    QLabel*         lblEnemyName;
    QLabel*         lblEnemyClass;
    QProgressBar*   barEnemyHP;
    QLabel*         lblEnemyHPVal;

    QLabel*         lblTurnInfo;
    QLabel*         lblScore;

    // ── grid ────────────────────────────────────────────────
    QGraphicsScene* scene;
    QGraphicsView*  gridView;

    // Tokens are pixmap items — arcade sprite per character
    QGraphicsPixmapItem* playerToken = nullptr;
    QGraphicsPixmapItem* enemyToken  = nullptr;

    // ── logic ───────────────────────────────────────────────
    GameManager*    gameManager;
    Character*      selectedCharacter;
    int             selectedType;   // 0/1/2
    int             specialCooldown = 0;
    bool            hardMode        = false;

    // ── helpers ─────────────────────────────────────────────
    void buildMenuPage();
    void buildCharacterPage();
    void buildDifficultyPage();
    void buildGamePage();
    void startBattle();

    QWidget* buildHUDPanel(bool isPlayer);

    void applyGlobalStyle();

    void drawGrid();
    void updateTokenPositions();
    void updateHUD();
    void showGameOver(bool playerWon);

    // Flash a token/portrait to attack or special pose then restore idle
    // pose: 1 = attack, 2 = special
    void flashAttackPose(bool isPlayer, int pose);

    // grid drawing constants
    static constexpr int CELL  = 56;
    static constexpr int GCOLS = 8;
    static constexpr int GROWS = 8;
};

#endif // MAINWINDOW_H