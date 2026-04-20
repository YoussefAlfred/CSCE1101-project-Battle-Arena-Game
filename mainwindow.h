#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QProgressBar>
#include <QFrame>
#include <QKeyEvent>

#include "GameManager.h"
#include "Warrior.h"
#include "Mage.h"
#include "Archer.h"

// ─────────────────────────────────────────────────────────────
//  Pages
//    0 – MenuPage        (title + Play button)
//    1 – CharacterPage   (pick Warrior / Mage / Archer)
//    2 – GamePage        (8×8 grid + HUD)
// ─────────────────────────────────────────────────────────────

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent* event) override;   // Member 2: movement

private slots:
    void onCharacterSelected(int type);   // 0=Warrior 1=Mage 2=Archer
    void onStartClicked();
    void onGameStateChanged(GameState state);

private:
    // ── pages ──────────────────────────────────────────────
    QStackedWidget* stack;
    QWidget*        menuPage;
    QWidget*        characterPage;
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

    // Tokens are pixmap items (pixel-art portrait) — replaces the old ellipses
    QGraphicsPixmapItem* playerToken = nullptr;
    QGraphicsPixmapItem* enemyToken  = nullptr;

    // ── logic ───────────────────────────────────────────────
    GameManager*    gameManager;
    Character*      selectedCharacter;
    int             selectedType;   // 0/1/2

    // ── helpers ─────────────────────────────────────────────
    void buildMenuPage();
    void buildCharacterPage();
    void buildGamePage();

    QWidget* buildHUDPanel(bool isPlayer);

    void applyGlobalStyle();

    void drawGrid();
    void updateTokenPositions();
    void updateHUD();
    void showGameOver(bool playerWon);

    // grid drawing constants
    static constexpr int CELL  = 56;
    static constexpr int GCOLS = 8;
    static constexpr int GROWS = 8;
};

#endif // MAINWINDOW_H