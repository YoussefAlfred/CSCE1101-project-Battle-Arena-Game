#include "GameManager.h"
#include "Character.h"

GameManager::GameManager(QObject* parent)
    : QObject(parent),
      player(nullptr),
      enemy(nullptr),
      state(GameState::MENU),
      score(0),
      timer(new QTimer(this))
{
    connect(timer, &QTimer::timeout, this, &GameManager::onTimerTick);
}

GameManager::~GameManager() {
    // player and enemy are owned externally; don't delete here
}

void GameManager::startGame(Character* playerCharacter, Character* enemyCharacter) {
    player = playerCharacter;
    enemy  = enemyCharacter;
    score  = 0;

    // Place characters on opposite corners of the 8x8 grid
    battleGrid.placeCharacter(player, 0, 0);
    battleGrid.placeCharacter(enemy,  7, 7);

    state = GameState::PLAYING;
    emit gameStateChanged(state);

    timer->start(TIMER_INTERVAL_MS);
}

void GameManager::checkWinCondition() {
    if (!player || !enemy) return;

    if (!enemy->isAlive()) {
        score += 100;
        timer->stop();
        state = GameState::GAME_OVER;
        emit scoreUpdated(score);
        emit gameStateChanged(state);
    } else if (!player->isAlive()) {
        timer->stop();
        state = GameState::GAME_OVER;
        emit gameStateChanged(state);
    }
}

void GameManager::restartGame() {
    timer->stop();
    player = nullptr;
    enemy  = nullptr;
    score  = 0;
    state  = GameState::CHARACTER_SELECT;
    emit gameStateChanged(state);
}

void GameManager::onTimerTick() {
    // Milestone 2: enemy AI movement + attack will be triggered here
    emit enemyTurnTriggered();
    checkWinCondition();
}

Character* GameManager::getPlayer() const { return player; }
Character* GameManager::getEnemy()  const { return enemy;  }
BattleGrid* GameManager::getGrid()        { return &battleGrid; }
GameState  GameManager::getState()  const { return state;  }
int         GameManager::getScore() const { return score;  }
