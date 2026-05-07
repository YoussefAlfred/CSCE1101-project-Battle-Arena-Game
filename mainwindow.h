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
#include <QTimer>

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
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onCharacterSelected(int type);
    void onStartClicked();
    void onGameStateChanged(GameState state);
    void onEnemyTurn();
    void onPauseClicked();
    void onLoadClicked();

private:
    // ── pages ───────────────────────────────────────────────
    QStackedWidget* stack;
    QWidget*        menuPage;
    QWidget*        characterPage;
    QWidget*        difficultyPage;
    QWidget*        gamePage;
    QWidget*        gameOverPage;

    // ── character select ────────────────────────────────────
    QWidget*        cardWidget[3] = {nullptr, nullptr, nullptr};
    QPushButton*    btnStart;
    QLabel*         selectionLabel;

    // ── Character select live preview ───────────────────────
    QLabel*         selPreviewSprite   = nullptr;
    QLabel*         selPreviewName     = nullptr;
    QLabel*         selPreviewSubtitle = nullptr;
    QLabel*         selPreviewSpecial  = nullptr;
    QLabel*         selPreviewDesc     = nullptr;
    QLabel*         selPreviewPoseTag  = nullptr;
    QTimer*         selPreviewTimer    = nullptr;
    int             selPreviewType     = 0;
    int             selPreviewPose     = 0;
    int             selPreviewFrame    = 0;
    int             selPoseTicksLeft   = 0;

    // ── game HUD ────────────────────────────────────────────
    QLabel*         playerPortraitLabel = nullptr;
    QLabel*         enemyPortraitLabel  = nullptr;

    QLabel*         lblPlayerName;
    QLabel*         lblPlayerClass;
    QProgressBar*   barPlayerHP;
    QLabel*         lblPlayerHPVal;
    QLabel*         lblPlayerDanger = nullptr;

    QLabel*         lblEnemyName;
    QLabel*         lblEnemyClass;
    QProgressBar*   barEnemyHP;
    QLabel*         lblEnemyHPVal;
    QLabel*         lblEnemyDanger = nullptr;

    QLabel*         lblTurnInfo;
    QLabel*         lblScore;

    // ── Combat log ──────────────────────────────────────────
    QLabel*         lblCombatLog = nullptr;
    QStringList     combatMessages;

    // ── Bottom status bar ───────────────────────────────────
    QLabel*         lblTurnCounter = nullptr;
    QLabel*         lblDistance    = nullptr;
    QLabel*         lblDiffBadge  = nullptr;

    // ── Action buttons ──────────────────────────────────────
    QPushButton*    btnActionAttack  = nullptr;
    QPushButton*    btnActionSpecial = nullptr;
    QPushButton*    btnResume = nullptr;
    // ── Game over overlay ───────────────────────────────────
    QLabel*         lblGOTitle   = nullptr;
    QLabel*         lblGOMessage = nullptr;
    QLabel*         lblGOScore   = nullptr;
    QLabel*         lblGOSprite  = nullptr;


    // ── Turn counting ───────────────────────────────────────
    int             turnCount = 0;

    // ── Difficulty page preview ─────────────────────────────
    QLabel*         diffCharPreview = nullptr;
    QLabel*         diffCharName    = nullptr;

    // ── Menu animation ──────────────────────────────────────
    QLabel*         menuSprites[3] = {nullptr, nullptr, nullptr};
    int             menuPoseFrame  = 0;
    QTimer*         menuAnimTimer  = nullptr;
    QTimer*         battleAnimTimer = nullptr;
    int             battleAnimFrame = 0;
    QTimer*         gameOverAnimTimer = nullptr;
    int             gameOverAnimFrame = 0;
    int             gameOverAnimType  = 0;
    int             gameOverAnimPose  = 2;
    int             combatAnimLocks   = 0;

    // ── grid ────────────────────────────────────────────────
    QGraphicsScene*      scene;
    QGraphicsView*       gridView;
    QGraphicsPixmapItem* playerToken = nullptr;
    QGraphicsPixmapItem* enemyToken  = nullptr;

    // ── logic ───────────────────────────────────────────────
    GameManager* gameManager;
    Character*   selectedCharacter;
    int          selectedType  = -1;   // 0=Warrior 1=Mage 2=Archer
    int          enemyType     = -1;
    int          specialCooldown = 0;
    bool         hardMode        = false;
    int          playerFacing    = 0;    // 0=down/front, 1=up/back, 2=left, 3=right
    int          enemyFacing     = 1;
    int          playerWalkFrame = 0;
    int          enemyWalkFrame  = 0;

    // ── helpers ─────────────────────────────────────────────
    void buildMenuPage();
    void buildCharacterPage();
    void buildDifficultyPage();
    void buildGamePage();
    void buildGameOverPage();
    void startBattle();

    // ── audio (no-op when Qt6 Multimedia is not available) ──
    void initAudio();
    void playClickSound();
    void playVictorySound();
    void playGameOverSound();

    QWidget* buildHUDPanel(bool isPlayer);

    void applyGlobalStyle();

    void drawGrid();
    void updateTokenPositions();
    void updateHUD();
    void showGameOver(bool playerWon);

    // Flash a token/portrait to attack or special pose then restore idle
    // pose: 1 = attack, 2 = special
    void flashAttackPose(bool isPlayer, int pose);

    void addCombatMessage(const QString& msg);
    void updateBottomBar();

    // grid drawing constants
    static constexpr int CELL  = 100;
    static constexpr int GCOLS = 8;
    static constexpr int GROWS = 8;

    int score = 0;

};

#endif // MAINWINDOW_H
