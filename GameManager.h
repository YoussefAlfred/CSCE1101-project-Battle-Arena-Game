#pragma once
#include "BattleGrid.h"
#include <QObject>
#include <QTimer>

class Character; // forward declaration

enum class GameState {
    MENU,
    CHARACTER_SELECT,
    PLAYING,
    PAUSED,
    GAME_OVER
};

class GameManager : public QObject {
    Q_OBJECT

public:
    explicit GameManager(QObject* parent = nullptr);
    ~GameManager();

    void resumeGame();
    void pauseGame();
    void startGame(Character* playerCharacter, Character* enemyCharacter);
    void checkWinCondition();
    void restartGame();
    void setHardMode(bool hard);
    void applySlowness();
    void applyPlayerSlow();
    bool isPlayerSlowed() const;

    Character*  getPlayer() const;
    Character*  getEnemy()  const;
    BattleGrid* getGrid();
    GameState   getState()  const;
    int         getScore()  const;

signals:
    void gameStateChanged(GameState newState);
    void enemyTurnTriggered();
    void scoreUpdated(int newScore);

public slots:
    void onTimerTick();

private:
    Character* player;
    Character* enemy;
    GameState  state;
    int        score;
    bool       hardMode;
    bool       playerSlowed     = false;
    int        slownessTicksLeft = 0;
    QTimer*    timer;
    BattleGrid battleGrid;

    void tickSlowness();

    static const int TIMER_INTERVAL_MS      = 800;
    static const int TIMER_INTERVAL_HARD_MS = 400;
};
