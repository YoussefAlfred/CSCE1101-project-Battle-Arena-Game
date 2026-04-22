#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QFrame>
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QPropertyAnimation>
#include <QDir>

// ═══════════════════════════════════════════════════════════════
//  Palette
// ═══════════════════════════════════════════════════════════════
namespace Pal {
    const QString BG        = "#0a0a18";
    const QString PANEL     = "#12122a";
    const QString BORDER    = "#2a2a4a";
    const QString ACCENT    = "#7c5cbf";
    const QString TEXT      = "#e8e4f0";
    const QString MUTED     = "#7a7a99";
    const QString GREEN     = "#3dba6e";
    const QString RED       = "#d94f4f";
    const QString AMBER     = "#d4a017";
    const QString TEAL      = "#28a89a";
    const QString ORANGE    = "#c86e2a";
    const QString GRID_DARK = "#181830";
    const QString GRID_LITE = "#1e1e38";
    const QString CELL_BORD = "#2d2d55";
}

// ─── helper: draw a scanline/arcade background onto a pixmap ──────────────
static QPixmap makeArcadeBg(int w, int h) {
    QPixmap px(w, h);
    px.fill(QColor("#0a0a18"));
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);

    // Subtle radial glow in centre
    QRadialGradient rg(w/2, h/2, w*0.7);
    rg.setColorAt(0.0, QColor(90, 40, 180, 60));
    rg.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(0, 0, w, h, rg);

    // Horizontal scanlines
    p.setPen(QColor(255, 255, 255, 6));
    for (int y = 0; y < h; y += 4)
        p.drawLine(0, y, w, y);

    // Vertical grid lines (very faint)
    p.setPen(QColor(100, 60, 220, 10));
    for (int x = 0; x < w; x += 40)
        p.drawLine(x, 0, x, h);
    for (int y = 0; y < h; y += 40)
        p.drawLine(0, y, w, y);

    // Corner accent glows
    auto corner = [&](int cx, int cy) {
        QRadialGradient cg(cx, cy, 200);
        cg.setColorAt(0.0, QColor(120, 60, 255, 40));
        cg.setColorAt(1.0, QColor(0,0,0,0));
        p.fillRect(0, 0, w, h, cg);
    };
    corner(0, 0); corner(w, 0); corner(0, h); corner(w, h);

    p.end();
    return px;
}

// ─── helper: render a character portrait as a pixmap ─────────────────────
// Uses provided image path if available, otherwise draws SVG-style pixel art
static QPixmap makeCharacterPixmap(int type, int size,
                                   const QString& imagePath = QString()) {
    if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
        QPixmap pm(imagePath);
        return pm.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QPixmap px(size, size);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);

    // Background glow
    QColor glowColor;
    if (type == 0)      glowColor = QColor("#d4a017");
    else if (type == 1) glowColor = QColor("#28a89a");
    else                glowColor = QColor("#c86e2a");

    QRadialGradient bg(size/2, size/2, size/2);
    bg.setColorAt(0.0, QColor(glowColor.red(), glowColor.green(), glowColor.blue(), 40));
    bg.setColorAt(1.0, Qt::transparent);
    p.fillRect(0, 0, size, size, bg);

    // Draw pixel-art style character silhouette
    p.setPen(Qt::NoPen);
    int s = size / 6;  // unit

    auto fillR = [&](int x, int y, int w, int h, QColor c) {
        p.setBrush(c);
        p.drawRoundedRect(x, y, w, h, 2, 2);
    };

    if (type == 0) {  // Warrior — gold armor
        QColor body("#c0940a"), light("#f0c030"), dark("#8a6800"), skin("#f4c07a");
        // helmet
        fillR(size/2-s, s,   s*2, s,   light);
        fillR(size/2-s+2, s/2, s*2-4, s/2, dark);
        // face
        fillR(size/2-s+2, s*2, s*2-4, s, skin);
        // shoulder
        fillR(size/2-s*2, s*2, s, s, body);
        fillR(size/2+s,   s*2, s, s, body);
        // torso
        fillR(size/2-s, s*3, s*2, s*2, body);
        // belt
        fillR(size/2-s, s*5, s*2, s/2, dark);
        // arms
        fillR(size/2-s*2, s*3, s, s*2, body);
        fillR(size/2+s,   s*3, s, s*2, body);
        // sword (right)
        fillR(size/2+s*2, s,   s/2, s*4, light);
        fillR(size/2+s*2-s/2, s*2, s*2, s/2, dark);
        // legs
        fillR(size/2-s, s*5+s/2, s-2, s*2, dark);
        fillR(size/2+2, s*5+s/2, s-2, s*2, dark);
        // boots
        fillR(size/2-s, s*7+s/2, s, s/2, dark);
        fillR(size/2+2, s*7+s/2, s+s/2, s/2, dark);
    }
    else if (type == 1) { // Mage — teal robes
        QColor robe("#1a7a72"), light("#30c0b0"), dark("#0a4a46"), skin("#f4c07a"),
               star("#ffe060");
        // hat
        fillR(size/2-s/2, 0, s, s*2, dark);
        fillR(size/2-s,   s, s*2, s,   robe);
        // face
        fillR(size/2-s+4, s*2, s*2-8, s, skin);
        // robe
        fillR(size/2-s, s*3, s*2, s*3, robe);
        fillR(size/2-s-s/2, s*4, s/2+4, s*2, robe);
        fillR(size/2+s,     s*4, s/2+4, s*2, robe);
        // staff
        fillR(size/2-s*2-s/2, s, s/3, s*7, dark);
        // orb on staff
        p.setBrush(QColor(star)); p.setPen(QPen(light,1));
        p.drawEllipse(size/2-s*3+2, s/2, s, s);
        p.setPen(Qt::NoPen);
        // stars/sparks
        p.setBrush(star);
        for (int i = 0; i < 4; i++) {
            int sx = size/2-s*3 + (i%2)*s*5;
            int sy = s + (i/2)*s*5;
            p.drawEllipse(sx, sy, 4, 4);
        }
    }
    else {              // Archer — orange leather
        QColor leather("#a05018"), light("#d07028"), dark("#603010"), skin("#f4c07a"),
               bow("#8b4513"), string_c("#e8d8b0");
        // hood
        fillR(size/2-s, 0,   s*2, s,   dark);
        fillR(size/2-s, s,   s*2, s/2, dark);
        // face
        fillR(size/2-s+4, s+s/2, s*2-8, s, skin);
        // torso
        fillR(size/2-s, s*3, s*2, s*3, leather);
        // arms
        fillR(size/2-s*2, s*3, s, s*2, leather);
        fillR(size/2+s,   s*3, s, s*2, leather);
        // legs
        fillR(size/2-s, s*6, s-2, s*2, dark);
        fillR(size/2+2, s*6, s-2, s*2, dark);
        // bow (left side)
        p.setBrush(QColor(bow)); p.setPen(Qt::NoPen);
        p.drawArc(size/2-s*3-s/2, s*2, s*2, s*5, 30*16, 120*16);
        p.setPen(QPen(QColor(string_c), 1));
        p.drawLine(size/2-s*3+s/4, s*2+s/2, size/2-s*3+s/4, s*7-s/2);
        p.setPen(Qt::NoPen);
        // arrow
        p.setBrush(QColor(string_c));
        fillR(size/2-s*2-s/2, s*4, s*3, 3, string_c);
        // arrowhead
        QPolygon arrowHead;
        arrowHead << QPoint(size/2+s/2, s*4-3)
                  << QPoint(size/2+s/2, s*4+6)
                  << QPoint(size/2+s,   s*4+1);
        p.drawPolygon(arrowHead);
    }

    p.end();
    return px;
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
    setMinimumSize(900, 620);

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
            background-color: #0a0a18;
            color: #e8e4f0;
            font-family: "Segoe UI", Arial, sans-serif;
        }
        QLabel {
            background: transparent;
        }
        QProgressBar {
            border: 1px solid #2a2a4a;
            border-radius: 4px;
            background: #0a0a18;
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
            background: #181830;
        }
    )");
}

// ═══════════════════════════════════════════════════════════════
//  Arcade Background Widget helper
// ═══════════════════════════════════════════════════════════════

// A QWidget subclass that paints the arcade background
class ArcadeBgWidget : public QWidget {
    QPixmap bgCache;
public:
    explicit ArcadeBgWidget(QWidget* parent = nullptr) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent*) override {
        if (bgCache.size() != size())
            bgCache = makeArcadeBg(width(), height());
        QPainter p(this);
        p.drawPixmap(0, 0, bgCache);
    }
    void resizeEvent(QResizeEvent* e) override {
        bgCache = QPixmap(); // invalidate cache
        QWidget::resizeEvent(e);
    }
};

// ═══════════════════════════════════════════════════════════════
//  Page 0 — Menu
// ═══════════════════════════════════════════════════════════════

void MainWindow::buildMenuPage() {
    ArcadeBgWidget* bg = new ArcadeBgWidget();
    menuPage = bg;

    QVBoxLayout* lay = new QVBoxLayout(menuPage);
    lay->setAlignment(Qt::AlignCenter);
    lay->setSpacing(24);

    // ── pixel-art coin animation row ──────────────────
    QHBoxLayout* coinRow = new QHBoxLayout();
    coinRow->setSpacing(16);
    for (int i = 0; i < 5; i++) {
        QLabel* coin = new QLabel("◆");
        coin->setAlignment(Qt::AlignCenter);
        coin->setStyleSheet(QString("font-size: 14px; color: %1;")
            .arg(i % 2 == 0 ? "#d4a017" : "#7c5cbf"));
        coinRow->addWidget(coin);
    }

    // ── INSERT COIN label ─────────────────────────────
    QLabel* insertCoin = new QLabel("INSERT COIN");
    insertCoin->setAlignment(Qt::AlignCenter);
    insertCoin->setStyleSheet(R"(
        font-size: 12px;
        font-weight: bold;
        color: #d4a017;
        letter-spacing: 6px;
        font-family: "Courier New", monospace;
    )");

    // ── title ─────────────────────────────────────────
    QLabel* title = new QLabel("⚔  BATTLE ARENA");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 52px;
        font-weight: 900;
        letter-spacing: 8px;
        color: #ffffff;
        font-family: "Impact", "Arial Black", sans-serif;
    )");
    // Drop shadow effect on the title label
    QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect();
    glow->setBlurRadius(32);
    glow->setColor(QColor("#7c5cbf"));
    glow->setOffset(0, 0);
    title->setGraphicsEffect(glow);

    QLabel* sub = new QLabel("CSCE 1101  ·  Spring 2026");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 12px; color: #7a7a99; letter-spacing: 3px; font-family: 'Courier New', monospace;");

    // ── character preview row ─────────────────────────
    QHBoxLayout* charRow = new QHBoxLayout();
    charRow->setSpacing(30);
    charRow->setAlignment(Qt::AlignCenter);
    QStringList charNames = {"WARRIOR", "MAGE", "ARCHER"};
    QStringList charColors = {Pal::AMBER, Pal::TEAL, Pal::ORANGE};
    for (int i = 0; i < 3; i++) {
        QVBoxLayout* col = new QVBoxLayout();
        col->setAlignment(Qt::AlignCenter);
        col->setSpacing(6);

        QLabel* portrait = new QLabel();
        portrait->setFixedSize(80, 80);
        portrait->setPixmap(makeCharacterPixmap(i, 80));
        portrait->setAlignment(Qt::AlignCenter);
        portrait->setStyleSheet(QString(
            "border: 2px solid %1; border-radius: 8px; background: rgba(0,0,0,60);"
        ).arg(charColors[i]));

        QLabel* name = new QLabel(charNames[i]);
        name->setAlignment(Qt::AlignCenter);
        name->setStyleSheet(QString("font-size: 10px; color: %1; letter-spacing: 2px; font-weight: bold; font-family: 'Courier New', monospace;").arg(charColors[i]));

        col->addWidget(portrait);
        col->addWidget(name);
        charRow->addLayout(col);
    }

    // ── separator ─────────────────────────────────────
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("border: 1px solid #2a2a4a;");
    line->setFixedWidth(400);

    // ── play button ───────────────────────────────────
    QPushButton* btnPlay = new QPushButton("  ▶   PLAY");
    btnPlay->setFixedSize(300, 64);
    btnPlay->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #5a3ea0, stop:1 #7c5cbf);
            color: #fff;
            font-size: 20px;
            font-weight: bold;
            letter-spacing: 4px;
            border-radius: 10px;
            border: none;
            font-family: "Impact", "Arial Black", sans-serif;
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

    QLabel* credits = new QLabel("Youssef Gohar · Mohamed · Ahmed · Youssef Al-Ghobary");
    credits->setAlignment(Qt::AlignCenter);
    credits->setStyleSheet("font-size: 10px; color: #333355; letter-spacing: 1px;");

    lay->addStretch(2);
    lay->addLayout(coinRow);
    lay->addWidget(insertCoin, 0, Qt::AlignCenter);
    lay->addSpacing(8);
    lay->addWidget(title,      0, Qt::AlignCenter);
    lay->addWidget(sub,        0, Qt::AlignCenter);
    lay->addSpacing(12);
    lay->addLayout(charRow);
    lay->addWidget(line,       0, Qt::AlignCenter);
    lay->addSpacing(8);
    lay->addWidget(btnPlay,    0, Qt::AlignCenter);
    lay->addStretch(1);
    lay->addWidget(credits,    0, Qt::AlignCenter);
    lay->addStretch(1);

    stack->addWidget(menuPage);
}

// ═══════════════════════════════════════════════════════════════
//  Page 1 — Character Select
// ═══════════════════════════════════════════════════════════════

void MainWindow::buildCharacterPage() {
    ArcadeBgWidget* bg = new ArcadeBgWidget();
    characterPage = bg;

    QVBoxLayout* root = new QVBoxLayout(characterPage);
    root->setContentsMargins(40, 28, 40, 28);
    root->setSpacing(20);

    // ── header ────────────────────────────────────────
    QLabel* header = new QLabel("CHOOSE YOUR FIGHTER");
    header->setAlignment(Qt::AlignCenter);
    header->setStyleSheet(R"(
        font-size: 30px;
        font-weight: bold;
        color: #ffffff;
        letter-spacing: 4px;
        font-family: "Impact", "Arial Black", sans-serif;
    )");
    QGraphicsDropShadowEffect* hGlow = new QGraphicsDropShadowEffect();
    hGlow->setBlurRadius(20); hGlow->setColor(QColor("#7c5cbf")); hGlow->setOffset(0,0);
    header->setGraphicsEffect(hGlow);

    selectionLabel = new QLabel("No character selected");
    selectionLabel->setAlignment(Qt::AlignCenter);
    selectionLabel->setTextFormat(Qt::RichText);      // ← FIX: enable HTML
    selectionLabel->setStyleSheet("font-size: 13px; color: #7a7a99;");

    // ── cards row ─────────────────────────────────────
    QHBoxLayout* cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(24);

    struct CardInfo {
        int type;
        QString name, hp, atk, special, desc, accent, imagePath;
    };
    QList<CardInfo> cards = {
        {0, "Warrior", "200", "20", "Power Strike (1.5× ATK)",
         "A heavy-hitting frontliner built to absorb punishment.",
         Pal::AMBER, ":/images/warrior.png"},
        {1, "Mage",    "100", "20", "Arcane Storm (3× ATK)",
         "Fragile but devastating — one burst can turn the tide.",
         Pal::TEAL,  ":/images/mage.png"},
        {2, "Archer",  "150", "15", "Double Shot (2× ATK)",
         "Balanced ranger who strikes reliably from any range.",
         Pal::ORANGE,":/images/archer.png"},
    };

    QPushButton** cardPtrs[3] = {&cardWarrior, &cardMage, &cardArcher};

    for (int i = 0; i < 3; i++) {
        const CardInfo& ci = cards[i];
        QWidget* card = new QWidget();
        card->setMinimumSize(240, 300);
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(QString(R"(
            QWidget#card%1 {
                background: #12122a;
                border: 2px solid #2a2a4a;
                border-radius: 14px;
            }
            QWidget#card%1:hover {
                background: #1a1a38;
                border: 2px solid %2;
            }
        )").arg(i).arg(ci.accent));
        card->setObjectName(QString("card%1").arg(i));

        QVBoxLayout* cLay = new QVBoxLayout(card);
        cLay->setContentsMargins(16, 20, 16, 20);
        cLay->setSpacing(10);
        cLay->setAlignment(Qt::AlignCenter);

        // Portrait
        QLabel* portrait = new QLabel();
        portrait->setFixedSize(100, 100);
        portrait->setAlignment(Qt::AlignCenter);
        QPixmap pm = makeCharacterPixmap(ci.type, 100, ci.imagePath);
        portrait->setPixmap(pm);
        portrait->setStyleSheet(QString(
            "border: 2px solid %1; border-radius: 10px; "
            "background: rgba(0,0,0,80);"
        ).arg(ci.accent));
        QGraphicsDropShadowEffect* pGlow = new QGraphicsDropShadowEffect();
        pGlow->setBlurRadius(16); pGlow->setColor(QColor(ci.accent)); pGlow->setOffset(0,0);
        portrait->setGraphicsEffect(pGlow);

        // Name
        QLabel* nameL = new QLabel(ci.name.toUpper());
        nameL->setAlignment(Qt::AlignCenter);
        nameL->setStyleSheet(QString("font-size: 18px; font-weight: bold; color: %1; letter-spacing: 2px; font-family: 'Impact', sans-serif;").arg(ci.accent));

        // Stats row — use RichText via QLabel, not QPushButton
        QLabel* statsL = new QLabel();
        statsL->setTextFormat(Qt::RichText);           // ← FIX
        statsL->setAlignment(Qt::AlignCenter);
        statsL->setText(
            "<span style='font-size:11px; color:#7a7a99;'>"
            "HP: <b style='color:#3dba6e;'>" + ci.hp + "</b>"
            "&nbsp;&nbsp;&nbsp;"
            "ATK: <b style='color:#d94f4f;'>" + ci.atk + "</b>"
            "</span>"
        );

        // Special ability
        QLabel* specL = new QLabel();
        specL->setTextFormat(Qt::RichText);            // ← FIX
        specL->setAlignment(Qt::AlignCenter);
        specL->setWordWrap(true);
        specL->setText(
            "<span style='font-size:11px; color:#b0a0d0;'>✦ " + ci.special + "</span>"
        );

        // Separator
        QFrame* sep = new QFrame();
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("border-color: #2a2a4a;");

        // Description
        QLabel* descL = new QLabel(ci.desc);
        descL->setAlignment(Qt::AlignCenter);
        descL->setWordWrap(true);
        descL->setStyleSheet("font-size: 10px; color: #606080;");

        // Invisible click button on top
        QPushButton* btn = new QPushButton();
        btn->setFlat(true);
        btn->setStyleSheet("background: transparent; border: none;");

        cLay->addWidget(portrait, 0, Qt::AlignCenter);
        cLay->addWidget(nameL,    0, Qt::AlignCenter);
        cLay->addWidget(statsL,   0, Qt::AlignCenter);
        cLay->addWidget(specL,    0, Qt::AlignCenter);
        cLay->addWidget(sep);
        cLay->addWidget(descL,    0, Qt::AlignCenter);

        *cardPtrs[i] = btn; // still store as QPushButton for slot compat
        // Make entire card clickable via an overlay
        // Use an event-filter approach with a transparent QPushButton overlay
        QPushButton* overlay = new QPushButton(card);
        overlay->setGeometry(0, 0, 300, 400);
        overlay->setStyleSheet("background: transparent; border: none;");
        overlay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        int t = i;
        connect(overlay, &QPushButton::clicked, this, [=]() { onCharacterSelected(t); });
        *cardPtrs[i] = overlay;

        cardsRow->addWidget(card);
        // Store card widget for border update
        if (i == 0) cardWarriorWidget = card;
        else if (i == 1) cardMageWidget = card;
        else cardArcherWidget = card;
    }

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
    btnStart->setFixedSize(200, 44);
    btnStart->setEnabled(false);
    btnStart->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #c0392b, stop:1 #e74c3c);
            color: #fff; font-size: 15px; font-weight: bold;
            letter-spacing: 1px;
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
    auto setCardBorder = [](QWidget* w, const QString& accent, bool selected) {
        w->setStyleSheet(QString(R"(
            QWidget {
                background: %1;
                border: 2px solid %2;
                border-radius: 14px;
            }
        )").arg(selected ? "#1e1e3a" : "#12122a")
           .arg(selected ? accent    : "#2a2a4a"));
    };
    setCardBorder(cardWarriorWidget, Pal::AMBER,  type == 0);
    setCardBorder(cardMageWidget,    Pal::TEAL,   type == 1);
    setCardBorder(cardArcherWidget,  Pal::ORANGE, type == 2);

    static const QStringList names = {"Warrior", "Mage", "Archer"};
    static const QStringList colors = {Pal::AMBER, Pal::TEAL, Pal::ORANGE};
    // FIX: selectionLabel already has setTextFormat(Qt::RichText) set
    selectionLabel->setText(
        "<span style='color:#7a7a99;'>Selected: </span>"
        "<b style='color:" + colors[type] + ";'>" + names[type] + "</b>"
    );

    btnStart->setEnabled(true);
}

void MainWindow::onStartClicked()
{
    delete selectedCharacter;
    selectedCharacter = nullptr;
 
    if      (selectedType == 0) selectedCharacter = new Warrior("Player");
    else if (selectedType == 1) selectedCharacter = new Mage("Player");
    else if (selectedType == 2) selectedCharacter = new Archer("Player");
    else return;
 
    Character* enemy = new Warrior("Enemy");
 
    // startGame() places both characters on the grid (player at 0,0 enemy at 7,7)
    gameManager->startGame(selectedCharacter, enemy);
 
    // Update HUD labels
    static const QStringList classNames = {"Warrior", "Mage", "Archer"};
    lblPlayerClass->setText(classNames[selectedType]);
    lblEnemyClass->setText("Warrior");
 
    // Update HUD portraits
    QPixmap pm = makeCharacterPixmap(selectedType, 64);
    if (playerPortraitLabel) playerPortraitLabel->setPixmap(pm);
    QPixmap enemyPm = makeCharacterPixmap(0, CELL - 8);
    if (enemyPortraitLabel) enemyPortraitLabel->setPixmap(enemyPm);
 
    // Redraw grid cells cleanly (no tokens yet)
    drawGrid();
 
    // NOW create tokens — characters are on the grid so positions are valid
    QPixmap playerPm = makeCharacterPixmap(selectedType, CELL - 8);
    playerToken = scene->addPixmap(playerPm);
    playerToken->setZValue(2);
 
    QPixmap enemyPixmap = makeCharacterPixmap(0, CELL - 8);   // Warrior
    enemyToken = scene->addPixmap(enemyPixmap);
    enemyToken->setZValue(2);
 
    // Sync token positions to where startGame() placed the characters
    updateTokenPositions();
 
    updateHUD();
    stack->setCurrentWidget(gamePage);
    gamePage->setFocus();
}

// ═══════════════════════════════════════════════════════════════
//  Page 2 — Game
// ═══════════════════════════════════════════════════════════════

QWidget* MainWindow::buildHUDPanel(bool isPlayer) {
    QWidget* panel = new QWidget();
    panel->setMinimumWidth(190);
    panel->setMaximumWidth(240);
    panel->setStyleSheet("background: rgba(18,18,42,220); border-radius: 10px; border: 1px solid #2a2a4a;");

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

    // Portrait
    QLabel*& portraitL = isPlayer ? playerPortraitLabel : enemyPortraitLabel;
    portraitL = new QLabel();
    portraitL->setFixedSize(64, 64);
    portraitL->setAlignment(Qt::AlignCenter);
    portraitL->setStyleSheet(QString(
        "border: 2px solid %1; border-radius: 8px; background: rgba(0,0,0,80);"
    ).arg(isPlayer ? "#7c5cbf" : "#d94f4f"));
    QPixmap pm = makeCharacterPixmap(isPlayer ? 0 : 0, 64); // placeholder
    portraitL->setPixmap(pm);

    // Name
    QLabel*& nameL = isPlayer ? lblPlayerName : lblEnemyName;
    nameL = new QLabel("—");
    nameL->setAlignment(Qt::AlignCenter);
    nameL->setStyleSheet("font-size: 16px; font-weight: bold; color: #e8e4f0;");

    // Class
    QLabel*& classL = isPlayer ? lblPlayerClass : lblEnemyClass;
    classL = new QLabel("—");
    classL->setAlignment(Qt::AlignCenter);
    classL->setStyleSheet(QString("font-size: 12px; color: %1;")
        .arg(isPlayer ? "#7c5cbf" : "#d94f4f"));

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
    if (!isPlayer) {
        bar->setStyleSheet(R"(
            QProgressBar { border:1px solid #2a2a4a; border-radius:4px; background:#0a0a18; }
            QProgressBar::chunk { border-radius:4px; background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #d94f4f,stop:1 #ff6b6b); }
        )");
    }

    QLabel*& hpVal = isPlayer ? lblPlayerHPVal : lblEnemyHPVal;
    hpVal = new QLabel("200 / 200");
    hpVal->setAlignment(Qt::AlignCenter);
    hpVal->setStyleSheet("font-size: 11px; color: #7a7a99;");

    lay->addWidget(role);
    lay->addSpacing(4);
    lay->addWidget(portraitL, 0, Qt::AlignCenter);
    lay->addWidget(nameL);
    lay->addWidget(classL);
    lay->addWidget(sep);
    lay->addWidget(hpTitle);
    lay->addWidget(bar);
    lay->addWidget(hpVal);
    lay->addStretch();

    // Controls hint (player side only)
    if (isPlayer) {
        // FIX: use QLabel with setTextFormat(Qt::RichText) instead of raw HTML in QPushButton
        QLabel* hint = new QLabel();
        hint->setTextFormat(Qt::RichText);             // ← FIX
        hint->setText(
            "<b>Controls</b><br>"
            "&#8593; &#8595; &#8592; &#8594;&nbsp; Move<br>"
            "Space&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Attack<br>"
            "Q&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Special"
        );
        hint->setStyleSheet(
            "font-size: 10px; color: #444466; "
            "border: 1px solid #1e1e3a; border-radius: 6px; padding: 8px;"
        );
        lay->addWidget(hint);
    }

    return panel;
}

void MainWindow::buildGamePage() {
    ArcadeBgWidget* bg = new ArcadeBgWidget();
    gamePage = bg;
    connect(gameManager, &GameManager::enemyTurnTriggered,
        this, &MainWindow::onEnemyTurn);
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
    gridView->setStyleSheet("background: #181830; border: 2px solid #2a2a4a; border-radius: 8px;");

    drawGrid();

    midRow->addWidget(leftPanel,  0, Qt::AlignTop);
    midRow->addWidget(gridView,   0, Qt::AlignCenter);
    midRow->addWidget(rightPanel, 0, Qt::AlignTop);

    root->addLayout(topBar);
    root->addLayout(midRow);

    stack->addWidget(gamePage);
    gamePage->setFocusPolicy(Qt::StrongFocus);
}
void MainWindow::onEnemyTurn() {
    Character* enemy  = gameManager->getEnemy();
    Character* player = gameManager->getPlayer();
    BattleGrid* grid  = gameManager->getGrid();

    if (!enemy || !player) return;
    if (!enemy->isAlive() || !player->isAlive()) return;

    int enemyRow  = enemy->getGridX();
    int enemyCol  = enemy->getGridY();
    int playerRow = player->getGridX();
    int playerCol = player->getGridY();

    // Step 1: try to move one step toward player
    int dr = playerRow - enemyRow;
    int dc = playerCol - enemyCol;

    int newRow = enemyRow;
    int newCol = enemyCol;

    if (std::abs(dr) >= std::abs(dc)) {
        // Move in row direction
        newRow += (dr > 0 ? 1 : -1);
    } else {
        // Move in col direction
        newCol += (dc > 0 ? 1 : -1);
    }

    grid->moveCharacter(enemy, newRow, newCol);

    // Step 2: attack if adjacent after moving
    if (grid->isAdjacent(enemy->getGridX(), enemy->getGridY(), playerRow, playerCol)) {
        player->takeDamage(enemy->attack());
    }

    // Step 3: update UI and check win condition
    updateHUD();
    gameManager->checkWinCondition();
}
void MainWindow::drawGrid()
{
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
    // Tokens are NOT created here — they are created in onStartClicked()
    // after startGame() has placed characters on the grid.
}


void MainWindow::updateTokenPositions()
{
    if (!gameManager || !gameManager->getPlayer() || !gameManager->getEnemy()) return;
 
    Character* p = gameManager->getPlayer();
    Character* e = gameManager->getEnemy();
 
    // getGridX() == row,  getGridY() == col
    // +4 offset keeps the portrait centered with a small margin inside the cell
    if (playerToken)
        playerToken->setPos(p->getGridY() * CELL + 4, p->getGridX() * CELL + 4);
 
    if (enemyToken)
        enemyToken->setPos(e->getGridY() * CELL + 4, e->getGridX() * CELL + 4);
}


void MainWindow::updateHUD() {
    if (!gameManager || !gameManager->getPlayer() || !gameManager->getEnemy()) return;

    Character* p = gameManager->getPlayer();
    Character* e = gameManager->getEnemy();

    int pHP = p->getCurrentHealth();
    int eHP = e->getCurrentHealth();

    int pMax = 200;
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
            background-color: #0a0a18;
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

    // FIX: use Qt::RichText explicitly on the QMessageBox content
    box->setTextFormat(Qt::RichText);

    if (playerWon) {
        box->setWindowTitle("Victory!");
        box->setText(
            "<b style='font-size:20px; color:#3dba6e;'>&#9876;  VICTORY!</b><br><br>"
            "You defeated the enemy.<br>"
            "Score: <b>" + QString::number(gameManager->getScore()) + "</b>"
        );
    } else {
        box->setText(
            "<b style='font-size:20px; color:#d94f4f;'>&#128128;  DEFEATED</b><br><br>"
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
        auto resetCard = [](QWidget* w, const QString& accent) {
            Q_UNUSED(accent);
            w->setStyleSheet(R"(
                QWidget { background: #12122a; border: 2px solid #2a2a4a; border-radius: 14px; }
            )");
        };
        resetCard(cardWarriorWidget, Pal::AMBER);
        resetCard(cardMageWidget,    Pal::TEAL);
        resetCard(cardArcherWidget,  Pal::ORANGE);
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

// ═══════════════════════════════════════════════════════════════
//  keyPressEvent — Member 2 (movement only)
//  Arrow keys / WASD move the player one cell per press.
//  Only runs while the game is in PLAYING state.
// ═══════════════════════════════════════════════════════════════

 
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // Only accept input during an active game
    if (!gameManager || gameManager->getState() != GameState::PLAYING) {
        QMainWindow::keyPressEvent(event);
        return;
    }
 
    Character* player = gameManager->getPlayer();
    if (!player) {
        QMainWindow::keyPressEvent(event);
        return;
    }
 
    int row = player->getGridX();   // X == row
    int col = player->getGridY();   // Y == col
 
    int newRow = row;
    int newCol = col;
 
    switch (event->key()) {
        case Qt::Key_Up:    case Qt::Key_W:  newRow--; break;
        case Qt::Key_Down:  case Qt::Key_S:  newRow++; break;
        case Qt::Key_Left:  case Qt::Key_A:  newCol--; break;
        case Qt::Key_Right: case Qt::Key_D:  newCol++; break;
        default:
            QMainWindow::keyPressEvent(event);
            return;
    }
 
    // moveCharacter validates bounds + occupancy; returns false if invalid — just ignore
    bool moved = gameManager->getGrid()->moveCharacter(player, newRow, newCol);
    if (moved) {
        updateTokenPositions();
        updateHUD();
    }
}