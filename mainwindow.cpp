#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), selectedCharacter(nullptr)
{
  gameManager = new GameManager(this);

    // main container (switch between screens)
  stackedWidget = new QStackedWidget(this);
  setCentralWidget(stackedWidget);

    // building both screens
  setupCharacterPage();
  setupGamePage();

    // select your character
  stackedWidget->setCurrentWidget(characterPage);
  setStyleSheet(R"( // using raw string literals
    QWidget {
        background-color: #1e1e2f;
        color: white;
        font-size: 14px;
    }

    QPushButton {
        background-color: #3a3a5a;
        padding: 10px;
        border-radius: 8px;
    }

    QPushButton:hover {
        background-color: #50507a;
    }

    QLabel {
        font-size: 16px;
    }
)");
}

  MainWindow::~MainWindow() {}


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

  //trying to make the "start game" button special
  startButton->setStyleSheet(
      "QPushButton {"
      "background-color: green;"
      "color: white;"
      "font-size: 16px;"
      "padding: 12px;"
      "border-radius: 6px;"
      "}"
      "QPushButton:hover {"
      "background-color: darkgreen;"
      "}"
      );

  warriorButton->setMinimumHeight(40);
  mageButton->setMinimumHeight(40);
  archerButton->setMinimumHeight(40);

  startButton->setMinimumHeight(60);

    // adding to layout
  layout->addWidget(title);
  layout->addWidget(warriorButton);
  layout->addWidget(mageButton);
  layout->addWidget(archerButton);
  layout->addWidget(startButton);

  characterPage->setLayout(layout);
  stackedWidget->addWidget(characterPage);

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

        // create the enemy
        Character* enemy = new Warrior("Enemy");

        gameManager->startGame(selectedCharacter, enemy);

        // update labels
        playerInfo->setText("Player: " + QString::fromStdString(selectedCharacter->getName()));

        enemyInfo->setText("Enemy: " + QString::fromStdString(enemy->getName()));

        // go to game screen
        stackedWidget->setCurrentWidget(gamePage);
    });
}

void MainWindow::drawGrid() {
    int cellsize = 40;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            scene->addRect(col * cellsize, row * cellsize, cellsize, cellsize);
        }
    }
}


void MainWindow::setupGamePage() {
  gamePage = new QWidget();

  QHBoxLayout* mainLayout = new QHBoxLayout();

    // left player info
  QVBoxLayout* leftLayout = new QVBoxLayout();

  QLabel* playerTitle = new QLabel("Player Info");
  playerInfo = new QLabel("No player yet");

  leftLayout->addWidget(playerTitle);
  leftLayout->addWidget(playerInfo);

    // Battlefield
  QVBoxLayout* centerLayout = new QVBoxLayout();

QLabel* battlefieldTitle = new QLabel("Battlefield");
battlefieldTitle->setAlignment(Qt::AlignCenter);

// graphics view + scene
QGraphicsView* view = new QGraphicsView();
scene = new QGraphicsScene();

view->setScene(scene);

// turning the rect to a grid
drawGrid();

centerLayout->addWidget(battlefieldTitle);
centerLayout->addWidget(view);

    // the enemy info
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
