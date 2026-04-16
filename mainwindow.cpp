#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QMessageBox>
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QFrame>

// ═══════════════════════════════════════════════════════════════
//  Palette
// ═══════════════════════════════════════════════════════════════
namespace Pal {
    const QString BG        = "#0d0d1a";   // deep navy-black
    const QString PANEL     = "#14142b";   // slightly lighter panel
    const QString BORDER    = "#2a2a4a";   // subtle border
    const QString ACCENT    = "#7c5cbf";   // purple accent
    const QString TEXT      = "#e8e4f0";   // off-white text
    const QString MUTED     = "#7a7a99";   // muted labels
    const QString GREEN     = "#3dba6e";   // HP bar / win
    const QString RED       = "#d94f4f";   // enemy / lose
    const QString AMBER     = "#d4a017";   // warrior gold
    const QString TEAL      = "#28a89a";   // mage teal
    const QString ORANGE    = "#c86e2a";   // archer orange
    const QString GRID_DARK = "#1a1a30";
    const QString GRID_LITE = "#1f1f3a";
    const QString CELL_BORD = "#2d2d55";
}

// ═══════════════════════════════════════════════════════════════
//  Constructor / Destructor
// ═══════════════════════════════════════════════════════════════

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      selectedCharacter(nullptr),
      selectedType(-1)
{
    setWindowTitle("⚔  Battle Arena");
    resize(1100, 720);
    setMinimumSize(900, 600);

    gameManager = new GameManager(this);
    connect(gameManager, &GameManager::gameStateChanged,
            this,        &MainWindow::onGameStateChanged);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    buildMenuPage();
    buildCharacterPage();
    buildGamePage();

    applyGlobalStyle();

    stack->setCurrentWidget(menuPage);
}

MainWindow::~MainWindow() {}

// ═══════════════════════════════════════════════════════════════
//  Global stylesheet
// ═══════════════════════════════════════════════════════════════

void MainWindow::applyGlobalStyle() {
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #0d0d1a;
            color: #e8e4f0;
            font-family: "Segoe UI", Arial, sans-serif;
        }
        QLabel {
            background: transparent;
        }
        QProgressBar {
            border: 1px solid #2a2a4a;
            border-radius: 4px;
            background: #0d0d1a;
            text-align: center;
            height: 14px;
        }
        QProgressBar::chunk {
            border-radius: 4px;
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #3dba6e, stop:1 #28d47e);
        }
        QGraphicsView {
            border: 2px solid #2a2a4a;
            border-radius: 6px;
            background: #1a1a30;
        }
    )");
}

// ═══════════════════════════════════════════════════════════════
//  Page 0 — Menu
// ═══════════════════════════════════════════════════════════════

void MainWindow::buildMenuPage() {
    menuPage = new QWidget();
    QVBoxLayout* lay = new QVBoxLayout(menuPage);
    lay->setAlignment(Qt::AlignCenter);
    lay->setSpacing(28);

    // ── title ──────────────────────────────────────────
    QLabel* title = new QLabel("⚔  BATTLE ARENA");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 48px;
        font-weight: 900;
        letter-spacing: 6px;
        color: #7c5cbf;
        text-shadow: 0 0 20px #7c5cbf;
    )");

    QLabel* sub = new QLabel("CSCE 1101  ·  Spring 2026");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 13px; color: #7a7a99; letter-spacing: 2px;");

    // ── decorative separator ──────────────────────────
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("border: 1px solid #2a2a4a;");
    line->setFixedWidth(360);

    // ── play button ───────────────────────────────────
    QPushButton* btnPlay = new QPushButton("  ▶   PLAY");
    btnPlay->setFixedSize(280, 60);
    btnPlay->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #5a3ea0, stop:1 #7c5cbf);
            color: #fff;
            font-size: 18px;
            font-weight: bold;
            letter-spacing: 3px;
            border-radius: 10px;
            border: none;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #6b4db5, stop:1 #9370d4);
        }
        QPushButton:pressed { background: #4a3090; }
    )");
    connect(btnPlay, &QPushButton::clicked, this, [=]() {
        stack->setCurrentWidget(characterPage);
    });

    // ── credits ───────────────────────────────────────
    QLabel* credits = new QLabel("Youssef Gohar · Mohamed · Ahmed · Youssef Al-Ghobary");
    credits->setAlignment(Qt::AlignCenter);
    credits->setStyleSheet("font-size: 11px; color: #444466;");

    lay->addStretch(2);
    lay->addWidget(title,   0, Qt::AlignCenter);
    lay->addWidget(sub,     0, Qt::AlignCenter);
    lay->addWidget(line,    0, Qt::AlignCenter);
    lay->addSpacing(10);
    lay->addWidget(btnPlay, 0, Qt::AlignCenter);
    lay->addStretch(1);
    lay->addWidget(credits, 0, Qt::AlignCenter);
    lay->addStretch(1);

    stack->addWidget(menuPage);
}

// ═══════════════════════════════════════════════════════════════
//  Page 1 — Character Select
// ═══════════════════════════════════════════════════════════════

QString MainWindow::cardStyle(const QString& accent, bool selected) const {
    QString border = selected ? accent : "#2a2a4a";
    QString glow   = selected ? "border: 2px solid " + accent + "; " : "border: 1px solid #2a2a4a; ";
    return QString(R"(
        QPushButton {
            background-color: #14142b;
            %1
            border-radius: 12px;
            color: #e8e4f0;
            padding: 14px;
            text-align: left;
        }
        QPushButton:hover {
            background-color: #1e1e3a;
            border: 2px solid %2;
        }
        QPushButton:pressed { background-color: #101028; }
    )").arg(glow, accent);
}

void MainWindow::buildCharacterPage() {
    characterPage = new QWidget();
    QVBoxLayout* root = new QVBoxLayout(characterPage);
    root->setContentsMargins(40, 30, 40, 30);
    root->setSpacing(20);

    // ── header ────────────────────────────────────────
    QLabel* header = new QLabel("Choose Your Fighter");
    header->setAlignment(Qt::AlignCenter);
    header->setStyleSheet("font-size: 28px; font-weight: bold; color: #7c5cbf; letter-spacing: 2px;");

    selectionLabel = new QLabel("No character selected");
    selectionLabel->setAlignment(Qt::AlignCenter);
    selectionLabel->setStyleSheet("font-size: 13px; color: #7a7a99;");

    // ── cards row ─────────────────────────────────────
    QHBoxLayout* cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(20);

    auto makeCard = [&](const QString& icon, const QString& name,
                        const QString& hp, const QString& atk,
                        const QString& special, const QString& desc,
                        const QString& accent) -> QPushButton* {
        QPushButton* btn = new QPushButton();
        btn->setMinimumSize(240, 260);
        btn->setStyleSheet(cardStyle(accent, false));
        // Build label text with HTML
        btn->setText(
            icon + "  <b style='font-size:17px;'>" + name + "</b><br><br>"
            "<span style='font-size:11px; color:#7a7a99;'>"
            "HP: <b style='color:#3dba6e;'>" + hp + "</b>   "
            "ATK: <b style='color:#d94f4f;'>" + atk + "</b></span><br><br>"
            "<span style='font-size:11px; color:#b0a0d0;'>✦ " + special + "</span><br><br>"
            "<span style='font-size:10px; color:#606080;'>" + desc + "</span>"
        );
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        return btn;
    };

    cardWarrior = makeCard("🗡", "Warrior", "200", "20",
        "Power Strike (1.5× ATK)",
        "A heavy-hitting frontliner built to absorb punishment.",
        Pal::AMBER);

    cardMage = makeCard("🔮", "Mage", "100", "20",
        "Arcane Storm (3× ATK)",
        "Fragile but devastating — one burst can turn the tide.",
        Pal::TEAL);

    cardArcher = makeCard("🏹", "Archer", "150", "15",
        "Double Shot (2× ATK)",
        "Balanced ranger who strikes reliably from any range.",
        Pal::ORANGE);

    connect(cardWarrior, &QPushButton::clicked, this, [=]() { onCharacterSelected(0); });
    connect(cardMage,    &QPushButton::clicked, this, [=]() { onCharacterSelected(1); });
    connect(cardArcher,  &QPushButton::clicked, this, [=]() { onCharacterSelected(2); });

    cardsRow->addWidget(cardWarrior);
    cardsRow->addWidget(cardMage);
    cardsRow->addWidget(cardArcher);

    // ── bottom buttons ────────────────────────────────
    QHBoxLayout* btnRow = new QHBoxLayout();

    QPushButton* btnBack = new QPushButton("← Back");
    btnBack->setFixedSize(120, 42);
    btnBack->setStyleSheet(R"(
        QPushButton {
            background: #1e1e3a; color: #7a7a99;
            border: 1px solid #2a2a4a; border-radius: 8px;
            font-size: 13px;
        }
        QPushButton:hover { background: #25253d; color: #e8e4f0; }
    )");
    connect(btnBack, &QPushButton::clicked, this, [=]() {
        stack->setCurrentWidget(menuPage);
    });

    btnStart = new QPushButton("Start Battle  ▶");
    btnStart->setFixedSize(180, 42);
    btnStart->setEnabled(false);
    btnStart->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #c0392b, stop:1 #e74c3c);
            color: #fff; font-size: 14px; font-weight: bold;
            border: none; border-radius: 8px;
        }
        QPushButton:hover { background: #e74c3c; }
        QPushButton:disabled { background: #2a2a40; color: #44445a; }
    )");
    connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);

    btnRow->addWidget(btnBack, 0, Qt::AlignLeft);
    btnRow->addStretch();
    btnRow->addWidget(btnStart, 0, Qt::AlignRight);

    root->addWidget(header);
    root->addWidget(selectionLabel);
    root->addLayout(cardsRow);
    root->addLayout(btnRow);

    stack->addWidget(characterPage);
}

void MainWindow::onCharacterSelected(int type) {
    selectedType = type;

    // Reset all card borders
    cardWarrior->setStyleSheet(cardStyle(Pal::AMBER,  type == 0));
    cardMage   ->setStyleSheet(cardStyle(Pal::TEAL,   type == 1));
    cardArcher ->setStyleSheet(cardStyle(Pal::ORANGE, type == 2));

    static const QStringList names = {"Warrior", "Mage", "Archer"};
    selectionLabel->setText("Selected: <b style='color:#7c5cbf;'>" + names[type] + "</b>");
    selectionLabel->setTextFormat(Qt::RichText);

    btnStart->setEnabled(true);
}

void MainWindow::onStartClicked() {
    delete selectedCharacter;
    selectedCharacter = nullptr;

    if      (selectedType == 0) selectedCharacter = new Warrior("Player");
    else if (selectedType == 1) selectedCharacter = new Mage("Player");
    else if (selectedType == 2) selectedCharacter = new Archer("Player");
    else return;

    Character* enemy = new Warrior("Enemy");   // Milestone 2 will randomise

    gameManager->startGame(selectedCharacter, enemy);

    // ── update HUD ─────────────────────────────────────
    static const QStringList classNames = {"Warrior", "Mage", "Archer"};
    lblPlayerClass->setText(classNames[selectedType]);
    lblEnemyClass->setText("Warrior");

    updateHUD();
    updateTokenPositions();
    stack->setCurrentWidget(gamePage);
}

// ═══════════════════════════════════════════════════════════════
//  Page 2 — Game
// ═══════════════════════════════════════════════════════════════

QWidget* MainWindow::buildHUDPanel(bool isPlayer) {
    QWidget* panel = new QWidget();
    panel->setMinimumWidth(190);
    panel->setMaximumWidth(240);
    panel->setStyleSheet("background: #14142b; border-radius: 10px;");

    QVBoxLayout* lay = new QVBoxLayout(panel);
    lay->setContentsMargins(14, 16, 14, 16);
    lay->setSpacing(8);

    // Role badge
    QLabel* role = new QLabel(isPlayer ? "YOU" : "ENEMY");
    role->setAlignment(Qt::AlignCenter);
    role->setStyleSheet(QString(
        "background: %1; color: #fff; border-radius: 4px; "
        "font-size: 11px; font-weight: bold; padding: 3px; letter-spacing: 2px;"
    ).arg(isPlayer ? "#5a3ea0" : "#8b2020"));

    // Name
    QLabel*& nameL = isPlayer ? lblPlayerName : lblEnemyName;
    nameL = new QLabel("—");
    nameL->setAlignment(Qt::AlignCenter);
    nameL->setStyleSheet("font-size: 18px; font-weight: bold; color: #e8e4f0;");

    // Class
    QLabel*& classL = isPlayer ? lblPlayerClass : lblEnemyClass;
    classL = new QLabel("—");
    classL->setAlignment(Qt::AlignCenter);
    classL->setStyleSheet("font-size: 12px; color: #7c5cbf;");

    // Separator
    QFrame* sep = new QFrame(); sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("border-color: #2a2a4a;");

    // HP label
    QLabel* hpTitle = new QLabel("HP");
    hpTitle->setStyleSheet("font-size: 10px; color: #7a7a99; letter-spacing: 1px;");

    QProgressBar*& bar = isPlayer ? barPlayerHP : barEnemyHP;
    bar = new QProgressBar();
    bar->setRange(0, 200);
    bar->setValue(200);
    bar->setTextVisible(false);
    bar->setFixedHeight(12);

    QLabel*& hpVal = isPlayer ? lblPlayerHPVal : lblEnemyHPVal;
    hpVal = new QLabel("200 / 200");
    hpVal->setAlignment(Qt::AlignCenter);
    hpVal->setStyleSheet("font-size: 11px; color: #7a7a99;");

    lay->addWidget(role);
    lay->addSpacing(6);
    lay->addWidget(nameL);
    lay->addWidget(classL);
    lay->addWidget(sep);
    lay->addWidget(hpTitle);
    lay->addWidget(bar);
    lay->addWidget(hpVal);
    lay->addStretch();

    // Controls hint (player side only)
    if (isPlayer) {
        QLabel* hint = new QLabel(
            "<b>Controls</b><br>"
            "↑ ↓ ← →  Move<br>"
            "Space      Attack<br>"
            "Q           Special"
        );
        hint->setTextFormat(Qt::RichText);
        hint->setStyleSheet(
            "font-size: 10px; color: #444466; "
            "border: 1px solid #1e1e3a; border-radius: 6px; padding: 8px;"
        );
        lay->addWidget(hint);
    }

    return panel;
}

void MainWindow::buildGamePage() {
    gamePage = new QWidget();
    QVBoxLayout* root = new QVBoxLayout(gamePage);
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(10);

    // ── top bar ───────────────────────────────────────
    QHBoxLayout* topBar = new QHBoxLayout();
    lblTurnInfo = new QLabel("Your turn — move or attack");
    lblTurnInfo->setStyleSheet("font-size: 13px; color: #7c5cbf; font-weight: bold;");
    lblScore = new QLabel("Score: 0");
    lblScore->setStyleSheet("font-size: 13px; color: #d4a017;");
    topBar->addWidget(lblTurnInfo);
    topBar->addStretch();
    topBar->addWidget(lblScore);

    // ── main row: [left HUD] [grid] [right HUD] ───────
    QHBoxLayout* midRow = new QHBoxLayout();
    midRow->setSpacing(16);

    QWidget* leftPanel  = buildHUDPanel(true);
    QWidget* rightPanel = buildHUDPanel(false);

    lblPlayerName->setText("Player");
    lblEnemyName->setText("Enemy");

    // Grid
    scene    = new QGraphicsScene(this);
    gridView = new QGraphicsView(scene);
    gridView->setFixedSize(GCOLS * CELL + 4, GROWS * CELL + 4);
    gridView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gridView->setRenderHint(QPainter::Antialiasing);
    gridView->setStyleSheet("background: #1a1a30; border: 2px solid #2a2a4a; border-radius: 8px;");

    drawGrid();

    midRow->addWidget(leftPanel,  0, Qt::AlignTop);
    midRow->addWidget(gridView,   0, Qt::AlignCenter);
    midRow->addWidget(rightPanel, 0, Qt::AlignTop);

    root->addLayout(topBar);
    root->addLayout(midRow);

    stack->addWidget(gamePage);
    gamePage->setFocusPolicy(Qt::StrongFocus);
}

void MainWindow::drawGrid() {
    scene->clear();
    playerToken = nullptr;
    enemyToken  = nullptr;

    for (int r = 0; r < GROWS; r++) {
        for (int c = 0; c < GCOLS; c++) {
            bool dark = (r + c) % 2 == 0;
            QColor fill(dark ? Pal::GRID_DARK : Pal::GRID_LITE);
            QColor border(Pal::CELL_BORD);

            QGraphicsRectItem* cell = scene->addRect(
                c * CELL, r * CELL, CELL, CELL,
                QPen(border, 0.8),
                QBrush(fill)
            );
            cell->setZValue(0);
        }
    }

    // ── Player token: purple circle ───────────────────
    playerToken = scene->addEllipse(
        4, 4, CELL - 8, CELL - 8,
        QPen(QColor("#9b6dff"), 2),
        QBrush(QColor("#5a3ea0"))
    );
    playerToken->setZValue(2);

    QGraphicsTextItem* pTxt = scene->addText("P");
    pTxt->setDefaultTextColor(Qt::white);
    pTxt->setFont(QFont("Arial", 14, QFont::Bold));
    pTxt->setPos(CELL/2 - 8, CELL/2 - 12);
    pTxt->setZValue(3);

    // ── Enemy token: red diamond shape via rect rotated ──
    enemyToken = scene->addEllipse(
        7 * CELL + 4, 7 * CELL + 4, CELL - 8, CELL - 8,
        QPen(QColor("#ff5555"), 2),
        QBrush(QColor("#8b2020"))
    );
    enemyToken->setZValue(2);

    QGraphicsTextItem* eTxt = scene->addText("E");
    eTxt->setDefaultTextColor(Qt::white);
    eTxt->setFont(QFont("Arial", 14, QFont::Bold));
    eTxt->setPos(7 * CELL + CELL/2 - 8, 7 * CELL + CELL/2 - 12);
    eTxt->setZValue(3);
}

void MainWindow::updateTokenPositions() {
    if (!gameManager || !gameManager->getPlayer() || !gameManager->getEnemy()) return;

    Character* p = gameManager->getPlayer();
    Character* e = gameManager->getEnemy();

    if (playerToken)
        playerToken->setPos(p->getGridY() * CELL, p->getGridX() * CELL);
    if (enemyToken)
        enemyToken->setPos(e->getGridY() * CELL, e->getGridX() * CELL);
}

void MainWindow::updateHUD() {
    if (!gameManager || !gameManager->getPlayer() || !gameManager->getEnemy()) return;

    Character* p = gameManager->getPlayer();
    Character* e = gameManager->getEnemy();

    int pHP = p->getCurrentHealth();
    int eHP = e->getCurrentHealth();

    // determine max based on class
    int pMax = 200; // Warrior default
    if (selectedType == 1) pMax = 100;
    else if (selectedType == 2) pMax = 150;

    barPlayerHP->setMaximum(pMax);
    barPlayerHP->setValue(pHP);
    lblPlayerHPVal->setText(QString("%1 / %2").arg(pHP).arg(pMax));

    barEnemyHP->setMaximum(200);
    barEnemyHP->setValue(eHP);
    lblEnemyHPVal->setText(QString("%1 / 200").arg(eHP));

    lblScore->setText("Score: " + QString::number(gameManager->getScore()));
}

void MainWindow::onGameStateChanged(GameState state) {
    if (state == GameState::GAME_OVER) {
        bool playerWon = gameManager->getPlayer() && gameManager->getPlayer()->isAlive();
        showGameOver(playerWon);
    }
}

void MainWindow::showGameOver(bool playerWon) {
    QMessageBox* box = new QMessageBox(this);
    box->setWindowTitle("Battle Over");
    box->setStyleSheet(R"(
        QMessageBox {
            background-color: #0d0d1a;
            color: #e8e4f0;
        }
        QLabel { color: #e8e4f0; font-size: 14px; }
        QPushButton {
            background: #5a3ea0; color: white;
            padding: 8px 20px; border-radius: 6px;
            font-size: 13px; border: none;
        }
        QPushButton:hover { background: #7c5cbf; }
    )");

    if (playerWon) {
        box->setWindowTitle("Victory!");
        box->setText(
            "<b style='font-size:20px; color:#3dba6e;'>⚔  VICTORY!</b><br><br>"
            "You defeated the enemy.<br>"
            "Score: <b>" + QString::number(gameManager->getScore()) + "</b>"
        );
    } else {
        box->setText(
            "<b style='font-size:20px; color:#d94f4f;'>💀  DEFEATED</b><br><br>"
            "You were slain in battle.<br>"
            "Better luck next time."
        );
    }

    QPushButton* restart = box->addButton("Play Again", QMessageBox::AcceptRole);
    QPushButton* menu    = box->addButton("Main Menu",  QMessageBox::RejectRole);
    Q_UNUSED(restart); Q_UNUSED(menu);

    box->exec();

    if (box->clickedButton() == restart) {
        gameManager->restartGame();
        stack->setCurrentWidget(characterPage);
        // reset card selection
        cardWarrior->setStyleSheet(cardStyle(Pal::AMBER,  false));
        cardMage   ->setStyleSheet(cardStyle(Pal::TEAL,   false));
        cardArcher ->setStyleSheet(cardStyle(Pal::ORANGE, false));
        selectionLabel->setText("No character selected");
        btnStart->setEnabled(false);
        selectedType = -1;
        delete selectedCharacter;
        selectedCharacter = nullptr;
    } else {
        gameManager->restartGame();
        stack->setCurrentWidget(menuPage);
    }
}