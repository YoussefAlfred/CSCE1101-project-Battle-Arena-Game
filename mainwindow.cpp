#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), selectedCharacter(nullptr)
{
    // create game manager
  gameManager = new GameManager(this);

    // main container (switch between screens)
  stackedWidget = new QStackedWidget(this);
  setCentralWidget(stackedWidget);

    // build both screens
  setupCharacterPage();
  setupGamePage();

    // start with character selection
  stackedWidget->setCurrentWidget(characterPage);
}

MainWindow::~MainWindow() {}

// Character Seletcion

void MainWindow::setupCharacterPage() {
    characterPage = new QWidget();

    QVBoxLayout* layout = new QVBoxLayout();

    // title
    QLabel* title = new QLabel("Choose Your Character");
    title->setAlignment(Qt::AlignCenter);

    // buttons
    warriorButton = new QPushButton("Warrior");
    mageButton = new QPushButton("Mage");
    archerButton = new QPushButton("Archer");
    startButton = new QPushButton("Start Game");

    // adding to layout
    layout->addWidget(title);
    layout->addWidget(warriorButton);
    layout->addWidget(mageButton);
    layout->addWidget(archerButton);
    layout->addWidget(startButton);

    characterPage->setLayout(layout);
    stackedWidget->addWidget(characterPage);

    // choose character
    connect(warriorButton, &QPushButton::clicked, this, [=]() {
        selectedCharacter = new Warrior("Player");
    });

    connect(mageButton, &QPushButton::clicked, this, [=]() {
        selectedCharacter = new Mage("Player");
    });

    connect(archerButton, &QPushButton::clicked, this, [=]() {
        selectedCharacter = new Archer("Player");
    });

    // start the game (through connecting the buttons to signals)
    connect(startButton, &QPushButton::clicked, this, [=]() {

        if (!selectedCharacter)
            return; // no character selected

        // create simple enemy
        Character* enemy = new Warrior("Enemy");

        // start game logic
        gameManager->startGame(selectedCharacter, enemy);

        // update labels
	playerInfo->setText("Player: " + QString::fromStdString(selectedCharacter->getName()));

	enemyInfo->setText("Enemy: " + QString::fromStdString(enemy->getName()));

        // go to game screen
	stackedWidget->setCurrentWidget(gamePage);
    });
}

// Game Screen

void MainWindow::setupGamePage() {
    gamePage = new QWidget();

    QHBoxLayout* mainLayout = new QHBoxLayout();

    // player info
    QVBoxLayout* leftLayout = new QVBoxLayout();

    QLabel* playerTitle = new QLabel("Player Info");
    playerInfo = new QLabel("No player yet");

    leftLayout->addWidget(playerTitle);
    leftLayout->addWidget(playerInfo);

    // Battlefield
    QVBoxLayout* centerLayout = new QVBoxLayout();

    QLabel* battlefieldTitle = new QLabel("Battlefield");
    battlefieldTitle->setAlignment(Qt::AlignCenter);

    QLabel* battlefield = new QLabel("[ Game Area ]");
    battlefield->setAlignment(Qt::AlignCenter);

    centerLayout->addWidget(battlefieldTitle);
    centerLayout->addWidget(battlefield);

    // the enemy info =====
    QVBoxLayout* rightLayout = new QVBoxLayout();

    QLabel* enemyTitle = new QLabel("Enemy Info");
    enemyInfo = new QLabel("No enemy yet");

    rightLayout->addWidget(enemyTitle);
    rightLayout->addWidget(enemyInfo);

    // add all sections to main layout
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(centerLayout);
    mainLayout->addLayout(rightLayout);

    gamePage->setLayout(mainLayout);
    stackedWidget->addWidget(gamePage);
}
