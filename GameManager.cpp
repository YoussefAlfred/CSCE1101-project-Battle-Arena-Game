#include "GameManager.h"
#include "Character.h"

GameManager::GameManager(QObject* parent)
    : QObject(parent),
      player(nullptr),
      enemy(nullptr),
      state(GameState::MENU),
      score(0),
      hardMode(false),
      timer(new QTimer(this))
{
    connect(timer, &QTimer::timeout, this, &GameManager::onTimerTick);
}

GameManager::~GameManager() {
    // player and enemy are owned externally; don't delete here
}
void GameManager::pauseGame() {
    if (timer) {
        timer->stop();
    }
    state = GameState::PAUSED; // for the game to stop
}
void GameManager::resumeGame() {
    state = GameState::PLAYING;
    int interval = hardMode ? TIMER_INTERVAL_HARD_MS : TIMER_INTERVAL_MS;
    timer->start(interval);
    emit gameStateChanged(state);
}
void GameManager::setHardMode(bool hard) {
    hardMode = hard;
}

void GameManager::startGame(Character* playerCharacter, Character* enemyCharacter) {
    player = playerCharacter;
    enemy  = enemyCharacter;
    score  = 0;
    battleGrid.reset();

    // Place characters on opposite corners of the 8×8 grid
    battleGrid.placeCharacter(player, 0, 0);
    battleGrid.placeCharacter(enemy,  7, 7);

    state = GameState::PLAYING;
    emit gameStateChanged(state);

    int interval = hardMode ? TIMER_INTERVAL_HARD_MS : TIMER_INTERVAL_MS;
    timer->start(interval);
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
    player            = nullptr;
    enemy             = nullptr;
    score             = 0;
    hardMode          = false;
    playerSlowed      = false;
    slownessTicksLeft = 0;
    state             = GameState::CHARACTER_SELECT;
    emit gameStateChanged(state);
}

void GameManager::applySlowness() {
    playerSlowed      = true;
    slownessTicksLeft = 10;
    // halve the enemy's attack speed
    timer->setInterval(hardMode ? 800 : 1600);
}

void GameManager::applyPlayerSlow() {
    // Enemy attacks twice as fast — halve the timer interval
    playerSlowed      = true;
    slownessTicksLeft = 10;
    timer->setInterval(hardMode ? 200 : 400);
}

void GameManager::tickSlowness() {
    if (!playerSlowed) return;
    slownessTicksLeft--;
    if (slownessTicksLeft <= 0) {
        playerSlowed = false;
        timer->setInterval(hardMode ? TIMER_INTERVAL_HARD_MS : TIMER_INTERVAL_MS);
    }
}

bool GameManager::isPlayerSlowed() const { return playerSlowed; }

void GameManager::onTimerTick() {
    tickSlowness();
    emit enemyTurnTriggered();
    checkWinCondition();
}

Character*  GameManager::getPlayer() const { return player; }
Character*  GameManager::getEnemy()  const { return enemy;  }
BattleGrid* GameManager::getGrid()         { return &battleGrid; }
GameState   GameManager::getState()  const { return state;  }
int         GameManager::getScore()  const { return score;  }
