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
#include <QMap>
#include <QPair>
#include <QLineEdit>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>

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

    // ── AI Chatbot ──────────────────────────────────────────
    void onChatSendClicked();
    void onChatReplyFinished(QNetworkReply* reply);

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
    QWidget*        enemyPanel  = nullptr; // for border flash
    QLabel*         lblStreak   = nullptr;
    int             winStreak   = 0;
    int             bestStreak  = 0;

    QLabel*         lblTurnInfo;
    QLabel*         lblScore;

    // ── Combat log ──────────────────────────────────────────
    QLabel*         lblCombatLog = nullptr;
    QStringList     combatMessages;

    // ── Bottom status bar ───────────────────────────────────
    QLabel*         lblTurnCounter   = nullptr;
    QLabel*         lblDistance      = nullptr;
    QLabel*         lblDiffBadge     = nullptr;
    QLabel*         lblSlowIndicator = nullptr;

    // ── Action buttons ──────────────────────────────────────
    QPushButton*    btnActionAttack  = nullptr;
    QPushButton*    btnActionSpecial = nullptr;
    QPushButton*    btnResume        = nullptr;

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

    // ── Menu / battle animations ────────────────────────────
    QLabel*         menuSprites[3]    = {nullptr, nullptr, nullptr};
    int             menuPoseFrame     = 0;
    QTimer*         menuAnimTimer     = nullptr;
    QTimer*         battleAnimTimer   = nullptr;
    int             battleAnimFrame   = 0;
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

    // Spell rune graphics: key = (row, col)
    QMap<QPair<int,int>, QGraphicsItem*> runeItems;

    // ── logic ───────────────────────────────────────────────
    GameManager* gameManager;
    Character*   selectedCharacter;
    int          selectedType    = -1;  // 0=Warrior 1=Mage 2=Archer
    int          enemyType       = -1;
    int          specialCooldown = 0;
    int          healCooldown    = 0;
    int          slowCooldown    = 0;
    bool         enemySlowed     = false;
    bool         hardMode        = false;
    int          playerFacing    = 0;
    int          enemyFacing     = 1;
    int          playerWalkFrame = 0;
    int          enemyWalkFrame  = 0;
    int          score           = 0;

    // ── AI Chatbot ──────────────────────────────────────────
    QNetworkAccessManager* chatNetworkManager  = nullptr;
    QTextEdit*             chatDisplay         = nullptr;
    QLineEdit*             chatInput           = nullptr;
    QString                loadedApiKey;  // read from api_key.txt at startup
    QPushButton*           chatSendBtn         = nullptr;
    // conversation history for multi-turn: alternating user/assistant
    QList<QPair<QString,QString>> chatHistory; // role -> content

    void appendChatMessage(const QString& role, const QString& text);

    // ── helpers ─────────────────────────────────────────────
    void buildMenuPage();
    void buildCharacterPage();
    void buildDifficultyPage();
    void buildGamePage();
    void buildGameOverPage();
    void startBattle();

    void initAudio();
    void playClickSound();
    void playVictorySound();
    void playGameOverSound();

    QWidget* buildHUDPanel(bool isPlayer);
    void     applyGlobalStyle();

    void drawGrid();
    void drawRunes();
    void removeRune(int row, int col);
    void checkSpellCell(int row, int col, bool isPlayer);
    void updateTokenPositions();
    void updateHUD();
    void showGameOver(bool playerWon);

    void flashAttackPose(bool isPlayer, int pose);
    void addCombatMessage(const QString& msg);
    void updateBottomBar();

    void castHeal();
    void castSlowness();
    void playSpellAnimation(bool isHeal, int row, int col);

    static constexpr int CELL  = 100;
    static constexpr int GCOLS = 8;
    static constexpr int GROWS = 8;
};

#endif // MAINWINDOW_H