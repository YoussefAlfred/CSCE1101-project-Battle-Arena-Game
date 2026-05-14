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
#include <QVariantAnimation>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QApplication>
#include <QPolygon>
#include <QImage>
#include <QAbstractButton>
#include <QMouseEvent>
#include <QUrl>
#include <ctime>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioDevice>
#include <QMediaDevices>
//for API integration stuff :D
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextEdit>
#include <QLineEdit>
#include <QScrollBar>
 
#ifdef BATTLE_HAS_AUDIO

#endif

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

    QRadialGradient rg(w/2, h/2, w*0.7);
    rg.setColorAt(0.0, QColor(90, 40, 180, 60));
    rg.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(0, 0, w, h, rg);

    p.setPen(QColor(255, 255, 255, 6));
    for (int y = 0; y < h; y += 4)
        p.drawLine(0, y, w, y);

    p.setPen(QColor(100, 60, 220, 10));
    for (int x = 0; x < w; x += 40)
        p.drawLine(x, 0, x, h);
    for (int y = 0; y < h; y += 40)
        p.drawLine(0, y, w, y);

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

// ═══════════════════════════════════════════════════════════════
//  Sprite drawing — true arcade pixel-art style
//
//  type:  0 = Warrior, 1 = Mage, 2 = Archer
//  pose:  0 = Idle, 1 = Attack, 2 = Special
//  size:  pixel dimensions (square)
// ═══════════════════════════════════════════════════════════════

// Helper to fill a pixel block (no antialiasing — hard pixel edges)
static void px(QPainter& p, int x, int y, int w, int h, QColor c) {
    p.fillRect(x, y, w, h, c);
}

static void drawSpriteStage(QPainter& p, int type, int pose, int G) {
    const QColor aura[3] = {
        QColor(255, 196, 48, 75),
        QColor(64, 224, 255, 78),
        QColor(255, 124, 48, 70)
    };
    const QColor flare = aura[type];

    // Every fighter now reads like a tiny arena champion: shadow, aura, and class crest.
    px(p, 4*G, 14*G, 8*G, G, QColor(0, 0, 0, 95));
    px(p, 3*G, 15*G, 10*G, G, QColor(0, 0, 0, 60));

    px(p, 2*G, 3*G, G, 9*G, QColor(flare.red(), flare.green(), flare.blue(), 28));
    px(p, 13*G, 3*G, G, 9*G, QColor(flare.red(), flare.green(), flare.blue(), 28));
    px(p, 4*G, G, 8*G, G, QColor(flare.red(), flare.green(), flare.blue(), 34));
    px(p, 4*G, 13*G, 8*G, G, QColor(flare.red(), flare.green(), flare.blue(), 36));

    if (pose == 2) {
        px(p, 0, 7*G, 16*G, 2*G, QColor(flare.red(), flare.green(), flare.blue(), 34));
        px(p, 7*G, 0, 2*G, 16*G, QColor(255, 255, 255, 26));
    }

    if (type == 0) {
        px(p, 1*G, 1*G, G, G, QColor("#ffe060"));
        px(p, 14*G, 1*G, G, G, QColor("#ffe060"));
        px(p, 1*G, 12*G, G, G, QColor("#b8920a"));
        px(p, 14*G, 12*G, G, G, QColor("#b8920a"));
    } else if (type == 1) {
        px(p, 1*G, 2*G, G, G, QColor("#80ffff"));
        px(p, 14*G, 2*G, G, G, QColor("#80ffff"));
        px(p, 2*G, 12*G, G, G, QColor("#ffe060"));
        px(p, 13*G, 12*G, G, G, QColor("#ffe060"));
    } else {
        px(p, 1*G, 4*G, 2*G, G, QColor("#f0c030"));
        px(p, 13*G, 4*G, 2*G, G, QColor("#f0c030"));
        px(p, 1*G, 11*G, 2*G, G, QColor("#d07028"));
        px(p, 13*G, 11*G, 2*G, G, QColor("#d07028"));
    }
}

static void drawObstacle(QGraphicsScene* scene, int c, int r, int cell) {
    const int x = c * cell;
    const int y = r * cell;
    const int margin = cell / 7;  // ~14px on a 100px cell

    // ── Ambient ground glow ────────────────────────────────────
    QPixmap glowPx(cell, cell);
    glowPx.fill(Qt::transparent);
    {
        QPainter gp(&glowPx);
        gp.setRenderHint(QPainter::Antialiasing);
        QRadialGradient rg(cell / 2, cell * 0.7, cell * 0.45);
        rg.setColorAt(0.0, QColor(125, 246, 255, 50));
        rg.setColorAt(0.55, QColor(80, 140, 220, 20));
        rg.setColorAt(1.0, QColor(0, 0, 0, 0));
        gp.fillRect(0, 0, cell, cell, rg);
    }
    auto* glowItem = scene->addPixmap(glowPx);
    glowItem->setPos(x, y);
    glowItem->setZValue(1.2);

    // ── Shadow beneath the pillar ─────────────────────────────
    const int sw = cell * 0.55;
    const int sh = cell * 0.10;
    auto* shadow = scene->addEllipse(x + (cell - sw) / 2, y + cell - margin - sh / 2,
                                     sw, sh, Qt::NoPen, QBrush(QColor(0, 0, 0, 110)));
    shadow->setZValue(1.3);

    // ── Stone pillar body ─────────────────────────────────────
    QPixmap pillarPx(cell, cell);
    pillarPx.fill(Qt::transparent);
    {
        QPainter pp(&pillarPx);
        pp.setRenderHint(QPainter::Antialiasing);
        pp.setPen(Qt::NoPen);

        const int pw = cell * 0.50;   // pillar width
        const int ph = cell * 0.65;   // pillar height
        const int px0 = (cell - pw) / 2;
        const int py0 = (cell - ph) / 2 - 2;

        // Main stone body gradient
        QLinearGradient stoneGrad(px0, py0, px0 + pw, py0 + ph);
        stoneGrad.setColorAt(0.0,  QColor("#2a2a48"));
        stoneGrad.setColorAt(0.35, QColor("#1a1a30"));
        stoneGrad.setColorAt(0.7,  QColor("#111122"));
        stoneGrad.setColorAt(1.0,  QColor("#0d0d1a"));
        pp.setBrush(stoneGrad);
        pp.drawRoundedRect(px0, py0, pw, ph, 6, 6);

        // Highlight edge on the left
        pp.setBrush(QColor(200, 200, 240, 25));
        pp.drawRoundedRect(px0, py0, pw / 4, ph, 4, 4);

        // ── Cracks ────────────────────────────────────────────
        pp.setPen(QPen(QColor(8, 8, 16, 180), 1.2));
        pp.drawLine(px0 + pw * 0.3, py0 + 4, px0 + pw * 0.5, py0 + ph * 0.35);
        pp.drawLine(px0 + pw * 0.5, py0 + ph * 0.35, px0 + pw * 0.35, py0 + ph * 0.6);
        pp.drawLine(px0 + pw * 0.6, py0 + ph * 0.5, px0 + pw * 0.75, py0 + ph - 4);

        // Lighter crack highlight next to dark cracks
        pp.setPen(QPen(QColor(60, 60, 100, 50), 0.8));
        pp.drawLine(px0 + pw * 0.3 + 2, py0 + 5, px0 + pw * 0.5 + 2, py0 + ph * 0.35 + 1);
        pp.drawLine(px0 + pw * 0.6 + 2, py0 + ph * 0.5 + 1, px0 + pw * 0.75 + 1, py0 + ph - 3);

        // ── Arcane rune glow (center) ─────────────────────────
        const int cx = cell / 2;
        const int cy = cell / 2 - 2;
        const int runeR = cell * 0.12;

        QRadialGradient runeGlow(cx, cy, runeR * 2.5);
        runeGlow.setColorAt(0.0, QColor(125, 246, 255, 100));
        runeGlow.setColorAt(0.4, QColor(125, 246, 255, 40));
        runeGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
        pp.setPen(Qt::NoPen);
        pp.setBrush(runeGlow);
        pp.drawEllipse(QPointF(cx, cy), runeR * 2.5, runeR * 2.5);

        // Cross / star rune shape
        pp.setPen(QPen(QColor(125, 246, 255, 210), 2.0, Qt::SolidLine, Qt::RoundCap));
        pp.drawLine(cx - runeR, cy, cx + runeR, cy);
        pp.drawLine(cx, cy - runeR, cx, cy + runeR);
        // Diagonal arms (smaller)
        const int dR = runeR * 0.65;
        pp.setPen(QPen(QColor(160, 220, 255, 140), 1.4, Qt::SolidLine, Qt::RoundCap));
        pp.drawLine(cx - dR, cy - dR, cx + dR, cy + dR);
        pp.drawLine(cx + dR, cy - dR, cx - dR, cy + dR);

        // Bright core dot
        pp.setPen(Qt::NoPen);
        pp.setBrush(QColor(220, 250, 255, 200));
        pp.drawEllipse(QPointF(cx, cy), 3, 3);

        // ── Top cap ───────────────────────────────────────────
        QLinearGradient capGrad(px0, py0 - 2, px0 + pw, py0 + 6);
        capGrad.setColorAt(0.0, QColor("#3a3a5a"));
        capGrad.setColorAt(1.0, QColor("#1e1e38"));
        pp.setBrush(capGrad);
        pp.drawRoundedRect(px0 - 3, py0 - 3, pw + 6, 8, 3, 3);

        // ── Bottom base ───────────────────────────────────────
        pp.setBrush(QColor("#1a1a2e"));
        pp.drawRoundedRect(px0 - 2, py0 + ph - 4, pw + 4, 8, 3, 3);
    }
    auto* pillarItem = scene->addPixmap(pillarPx);
    pillarItem->setPos(x, y);
    pillarItem->setZValue(1.5);
}

static void moveTokenSmoothly(QGraphicsPixmapItem* token, const QPointF& target, bool animated) {
    if (!token) return;
    if (!animated || (token->pos() - target).manhattanLength() < 0.5) {
        token->setPos(target);
        return;
    }

    QVariantAnimation* anim = new QVariantAnimation(token->scene());
    anim->setDuration(170);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(token->pos());
    anim->setEndValue(target);
    QObject::connect(anim, &QVariantAnimation::valueChanged, [token](const QVariant& value) {
        token->setPos(value.toPointF());
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

static void sparkle(QPainter& p, qreal x, qreal y, qreal r, const QColor& color) {
    p.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(x - r, y), QPointF(x + r, y));
    p.drawLine(QPointF(x, y - r), QPointF(x, y + r));
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(color.red(), color.green(), color.blue(), 120));
    p.drawEllipse(QPointF(x, y), r * 0.35, r * 0.35);
}

static int facingFromDelta(int dRow, int dCol, int fallback) {
    if (std::abs(dRow) >= std::abs(dCol) && dRow != 0)
        return dRow < 0 ? 1 : 0;
    if (dCol != 0)
        return dCol < 0 ? 2 : 3;
    return fallback;
}

static void drawBackChampion(QPainter& p, int type, int pose, int frame, const QColor& aura) {
    const qreal bob = frame % 2 == 0 ? -1.4 : 1.4;

    p.setBrush(QColor(aura.red(), aura.green(), aura.blue(), pose == 2 ? 60 : 32));
    p.drawRoundedRect(QRectF(22, 28 + bob, 52, 50), 18, 18);

    if (type == 0) {
        QLinearGradient cape(28, 30, 66, 82);
        cape.setColorAt(0.0, QColor("#b91f45"));
        cape.setColorAt(0.55, QColor("#5c1230"));
        cape.setColorAt(1.0, QColor("#210716"));
        p.setBrush(cape);
        p.drawPolygon(QPolygonF() << QPointF(28, 28 + bob) << QPointF(68, 28 + bob)
                                  << QPointF(76, 82) << QPointF(20, 82));

        QLinearGradient backPlate(31, 33, 65, 65);
        backPlate.setColorAt(0.0, QColor("#fff0a6"));
        backPlate.setColorAt(0.48, QColor("#c98c1f"));
        backPlate.setColorAt(1.0, QColor("#513009"));
        p.setBrush(backPlate);
        p.drawRoundedRect(QRectF(32, 35 + bob, 32, 31), 8, 8);
        p.setBrush(QColor("#ffd85a"));
        p.drawPolygon(QPolygonF() << QPointF(44, 42 + bob) << QPointF(52, 42 + bob)
                                  << QPointF(57, 56 + bob) << QPointF(48, 62 + bob)
                                  << QPointF(39, 56 + bob));

        p.setBrush(QColor("#d79b22"));
        p.drawEllipse(QRectF(24, 35 + bob, 16, 15));
        p.drawEllipse(QRectF(56, 35 + bob, 16, 15));
        p.setBrush(QColor("#24345d"));
        p.drawRoundedRect(QRectF(36, 21 + bob, 24, 20), 8, 8);
        p.setBrush(QColor("#f6cf57"));
        p.drawPolygon(QPolygonF() << QPointF(31, 27 + bob) << QPointF(48, 11 + bob)
                                  << QPointF(65, 27 + bob));
        p.setPen(QPen(QColor("#fff8c7"), pose == 2 ? 8 : 5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(69, 69 + bob), QPointF(74, 15 + bob));
        p.setPen(Qt::NoPen);

    } else if (type == 1) {
        QLinearGradient robe(28, 28, 68, 84);
        robe.setColorAt(0.0, QColor("#73f4ff"));
        robe.setColorAt(0.46, QColor("#2c52c8"));
        robe.setColorAt(1.0, QColor("#0b1041"));
        p.setBrush(robe);
        p.drawPolygon(QPolygonF() << QPointF(36, 31 + bob) << QPointF(60, 31 + bob)
                                  << QPointF(73, 84) << QPointF(23, 84));
        p.setBrush(QColor("#081138"));
        p.drawRoundedRect(QRectF(41, 38 + bob, 14, 39), 7, 7);
        p.setBrush(QColor("#f4d35e"));
        p.drawEllipse(QRectF(43, 51 + bob, 10, 10));

        QLinearGradient hat(33, 7, 63, 36);
        hat.setColorAt(0.0, QColor("#bdfcff"));
        hat.setColorAt(0.55, QColor("#3b2ed6"));
        hat.setColorAt(1.0, QColor("#120b47"));
        p.setBrush(hat);
        p.drawPolygon(QPolygonF() << QPointF(31, 27 + bob) << QPointF(48, 5 + bob)
                                  << QPointF(65, 27 + bob) << QPointF(58, 36 + bob)
                                  << QPointF(38, 36 + bob));
        p.setPen(QPen(QColor("#6b3b1f"), 4, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(24, 29 + bob), QPointF(24, 82 + bob));
        p.setPen(Qt::NoPen);
        sparkle(p, 24, 21 + bob, pose == 2 ? 8 : 5, QColor("#ffffff"));

    } else {
        QLinearGradient cloak(30, 28, 66, 83);
        cloak.setColorAt(0.0, QColor("#5aaa52"));
        cloak.setColorAt(0.48, QColor("#23512d"));
        cloak.setColorAt(1.0, QColor("#112517"));
        p.setBrush(cloak);
        p.drawPolygon(QPolygonF() << QPointF(30, 29 + bob) << QPointF(66, 29 + bob)
                                  << QPointF(75, 81) << QPointF(22, 83));
        p.setBrush(QColor("#233318"));
        p.drawPolygon(QPolygonF() << QPointF(27, 29 + bob) << QPointF(48, 10 + bob)
                                  << QPointF(69, 29 + bob) << QPointF(60, 47 + bob)
                                  << QPointF(36, 47 + bob));
        p.setBrush(QColor("#7a3e18"));
        p.drawRoundedRect(QRectF(62, 31 + bob, 11, 35), 4, 4);
        p.setBrush(QColor("#ffe16a"));
        p.drawRect(QRectF(64, 21 + bob, 3, 14));
        p.drawRect(QRectF(69, 23 + bob, 3, 12));
        p.setPen(QPen(QColor("#ffcf62"), pose == 2 ? 6 : 4, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(QRectF(17, 29 + bob, 24, 45), 80 * 16, 205 * 16);
        p.setPen(Qt::NoPen);
    }

    p.setBrush(QColor("#172140"));
    p.drawRoundedRect(QRectF(35, 68 + bob, 11, 16), 4, 4);
    p.drawRoundedRect(QRectF(51, 68 - bob, 11, 16), 4, 4);
    p.setBrush(QColor("#0d0d1d"));
    p.drawRoundedRect(QRectF(31, 82 + bob, 17, 6), 3, 3);
    p.drawRoundedRect(QRectF(49, 82 - bob, 17, 6), 3, 3);
}

static void drawSideChampion(QPainter& p, int type, int pose, int frame, const QColor& aura) {
    const qreal bob = frame % 2 == 0 ? -1.2 : 1.2;
    const qreal lunge = pose == 1 ? 5.0 : 0.0;

    p.setBrush(QColor(aura.red(), aura.green(), aura.blue(), pose == 2 ? 54 : 26));
    p.drawRoundedRect(QRectF(24 + lunge, 27 + bob, 45, 50), 18, 18);

    if (type == 0) {
        p.setBrush(QColor("#7b1732"));
        p.drawPolygon(QPolygonF() << QPointF(34 + lunge, 32 + bob) << QPointF(19 + lunge, 77)
                                  << QPointF(47 + lunge, 82) << QPointF(51 + lunge, 36 + bob));
        QLinearGradient armor(32 + lunge, 34, 65 + lunge, 68);
        armor.setColorAt(0.0, QColor("#fff0a6"));
        armor.setColorAt(0.5, QColor("#d69b20"));
        armor.setColorAt(1.0, QColor("#5f3a0e"));
        p.setBrush(armor);
        p.drawRoundedRect(QRectF(34 + lunge, 36 + bob, 29, 31), 8, 8);
        p.setBrush(QColor("#f3bd7d"));
        p.drawEllipse(QRectF(42 + lunge, 22 + bob, 20, 18));
        p.setBrush(QColor("#f6cf57"));
        p.drawPolygon(QPolygonF() << QPointF(36 + lunge, 26 + bob) << QPointF(52 + lunge, 10 + bob)
                                  << QPointF(66 + lunge, 27 + bob) << QPointF(61 + lunge, 35 + bob)
                                  << QPointF(41 + lunge, 35 + bob));
        p.setBrush(QColor("#61f7ff"));
        p.drawRoundedRect(QRectF(56 + lunge, 30 + bob, 7, 4), 2, 2);
        p.setBrush(QColor("#d69b20"));
        p.drawEllipse(QRectF(58 + lunge, 39 + bob, 14, 13));
        p.setPen(QPen(QColor("#fff8c7"), pose == 2 ? 8 : 5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(62 + lunge, 58 + bob), QPointF(87, pose == 1 ? 20 : 13));
        p.setPen(QPen(QColor("#69f6ff"), 2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(64 + lunge, 56 + bob), QPointF(87, pose == 1 ? 20 : 13));
        p.setPen(Qt::NoPen);

    } else if (type == 1) {
        QLinearGradient robe(32 + lunge, 27, 66 + lunge, 82);
        robe.setColorAt(0.0, QColor("#82fbff"));
        robe.setColorAt(0.46, QColor("#326bd6"));
        robe.setColorAt(1.0, QColor("#151b65"));
        p.setBrush(robe);
        p.drawPolygon(QPolygonF() << QPointF(39 + lunge, 33 + bob) << QPointF(60 + lunge, 35 + bob)
                                  << QPointF(69 + lunge, 82) << QPointF(29 + lunge, 82));
        p.setBrush(QColor("#f3bd7d"));
        p.drawEllipse(QRectF(43 + lunge, 23 + bob, 19, 17));
        p.setBrush(QColor("#ecfbff"));
        p.drawRect(QRectF(56 + lunge, 29 + bob, 5, 3));
        QLinearGradient hat(34 + lunge, 8, 65 + lunge, 35);
        hat.setColorAt(0.0, QColor("#bdfcff"));
        hat.setColorAt(0.55, QColor("#3b2ed6"));
        hat.setColorAt(1.0, QColor("#120b47"));
        p.setBrush(hat);
        p.drawPolygon(QPolygonF() << QPointF(33 + lunge, 27 + bob) << QPointF(50 + lunge, 5 + bob)
                                  << QPointF(67 + lunge, 27 + bob) << QPointF(58 + lunge, 36 + bob));
        p.setPen(QPen(QColor("#6b3b1f"), 4, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(27, 28 + bob), QPointF(27, 82 + bob));
        p.setPen(Qt::NoPen);
        sparkle(p, 27, 21 + bob, pose == 2 ? 8 : 5, QColor("#ffffff"));
        if (pose != 0) {
            p.setPen(QPen(QColor("#7df7ff"), pose == 2 ? 6 : 4, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(60 + lunge, 47 + bob), QPointF(88, 42));
            p.setPen(Qt::NoPen);
        }

    } else {
        QLinearGradient leather(32 + lunge, 41, 65 + lunge, 76);
        leather.setColorAt(0.0, QColor("#f59a45"));
        leather.setColorAt(0.45, QColor("#4f9f57"));
        leather.setColorAt(1.0, QColor("#1b4028"));
        p.setBrush(leather);
        p.drawRoundedRect(QRectF(35 + lunge, 42 + bob, 29, 28), 8, 8);
        p.setBrush(QColor("#233318"));
        p.drawPolygon(QPolygonF() << QPointF(31 + lunge, 29 + bob) << QPointF(50 + lunge, 11 + bob)
                                  << QPointF(68 + lunge, 29 + bob) << QPointF(60 + lunge, 45 + bob)
                                  << QPointF(39 + lunge, 45 + bob));
        p.setBrush(QColor("#f0b678"));
        p.drawEllipse(QRectF(44 + lunge, 27 + bob, 18, 16));
        p.setBrush(QColor("#ffe46a"));
        p.drawRect(QRectF(56 + lunge, 33 + bob, 5, 3));
        p.setPen(QPen(QColor("#ffcf62"), pose == 2 ? 6 : 4, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(QRectF(20, 25 + bob, 25, 48), 80 * 16, 205 * 16);
        p.setPen(QPen(QColor("#f8f1c7"), 1.8, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(30, 29 + bob), QPointF(pose == 0 ? 35 : 62 + lunge, 50 + bob));
        p.drawLine(QPointF(30, 72 + bob), QPointF(pose == 0 ? 35 : 62 + lunge, 50 + bob));
        if (pose != 0) {
            p.setPen(QPen(QColor("#ffe16a"), pose == 2 ? 4 : 3, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(32, 50 + bob), QPointF(88, pose == 2 ? 42 : 50));
        }
        p.setPen(Qt::NoPen);
    }

    p.setBrush(QColor("#172140"));
    p.drawRoundedRect(QRectF(36 + lunge, 68 + bob, 10, 16), 4, 4);
    p.drawRoundedRect(QRectF(54 + lunge, 68 - bob, 10, 16), 4, 4);
    p.setBrush(QColor("#0d0d1d"));
    p.drawRoundedRect(QRectF(32 + lunge, 82 + bob, 16, 6), 3, 3);
    p.drawRoundedRect(QRectF(51 + lunge, 82 - bob, 16, 6), 3, 3);
}

static QPixmap makeNeonChampionSprite(int type, int size, int pose, int facing = 0, int frame = 0) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.scale(size / 96.0, size / 96.0);
    p.setPen(Qt::NoPen);

    const QColor auraColors[3] = {
        QColor("#ffd85a"), QColor("#62f7ff"), QColor("#ff9b45")
    };
    const QColor aura = auraColors[type];

    QRadialGradient auraGlow(48, 48, pose == 2 ? 50 : 38);
    auraGlow.setColorAt(0.0, QColor(aura.red(), aura.green(), aura.blue(), pose == 2 ? 150 : 95));
    auraGlow.setColorAt(0.58, QColor(aura.red(), aura.green(), aura.blue(), 38));
    auraGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.setBrush(auraGlow);
    p.drawEllipse(QRectF(3, 3, 90, 90));

    if (pose == 1) {
        QLinearGradient slash(12, 20, 88, 70);
        slash.setColorAt(0.0, QColor(255, 255, 255, 0));
        slash.setColorAt(0.45, QColor(aura.red(), aura.green(), aura.blue(), 120));
        slash.setColorAt(1.0, QColor(255, 255, 255, 0));
        p.setPen(QPen(QBrush(slash), 5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(13, 24), QPointF(86, 65));
        p.setPen(Qt::NoPen);
    }

    if (pose == 2) {
        p.setPen(QPen(QColor(aura.red(), aura.green(), aura.blue(), 160), 3));
        p.drawEllipse(QRectF(14, 12, 68, 68));
        p.drawEllipse(QRectF(21, 19, 54, 54));
        p.setPen(Qt::NoPen);
        sparkle(p, 18, 26, 5, QColor("#ffffff"));
        sparkle(p, 78, 28, 5, aura);
        sparkle(p, 22, 72, 4, aura);
        sparkle(p, 75, 70, 4, QColor("#ffffff"));
    }

    p.setBrush(QColor(0, 0, 0, 130));
    p.drawEllipse(QRectF(25, 80, 46, 8));

    if (facing == 1) {
        drawBackChampion(p, type, pose, frame, aura);
        p.end();
        return pixmap;
    }

    if (facing == 2 || facing == 3) {
        p.save();
        if (facing == 2) {
            p.translate(96, 0);
            p.scale(-1, 1);
        }
        drawSideChampion(p, type, pose, frame, aura);
        p.restore();
        p.end();
        return pixmap;
    }

    if (type == 0) {
        // Solar knight: sharp armor, royal cape, enormous luminous blade.
        const qreal lean = pose == 1 ? 4 : 0;
        p.setBrush(QColor("#7b1732"));
        p.drawPolygon(QPolygonF() << QPointF(30 + lean, 30) << QPointF(22, 75)
                                  << QPointF(47, 84) << QPointF(53, 33));

        QLinearGradient armor(29, 31, 66, 72);
        armor.setColorAt(0.0, QColor("#fff0a6"));
        armor.setColorAt(0.45, QColor("#d69b20"));
        armor.setColorAt(1.0, QColor("#5f3a0e"));
        p.setBrush(armor);
        p.drawRoundedRect(QRectF(30 + lean, 34, 36, 33), 8, 8);
        p.drawEllipse(QRectF(24 + lean, 36, 15, 15));
        p.drawEllipse(QRectF(57 + lean, 36, 15, 15));

        p.setBrush(QColor("#172140"));
        p.drawRoundedRect(QRectF(34 + lean, 64, 11, 18), 4, 4);
        p.drawRoundedRect(QRectF(52 + lean, 64, 11, 18), 4, 4);
        p.setBrush(QColor("#2b1830"));
        p.drawRoundedRect(QRectF(30 + lean, 79, 17, 7), 3, 3);
        p.drawRoundedRect(QRectF(50 + lean, 79, 17, 7), 3, 3);

        p.setBrush(QColor("#f3bd7d"));
        p.drawRoundedRect(QRectF(36 + lean, 22, 24, 18), 7, 7);
        p.setBrush(QColor("#f6cf57"));
        p.drawPolygon(QPolygonF() << QPointF(31 + lean, 23) << QPointF(48 + lean, 10)
                                  << QPointF(65 + lean, 23) << QPointF(60 + lean, 31)
                                  << QPointF(36 + lean, 31));
        p.setBrush(QColor("#fff3ad"));
        p.drawPolygon(QPolygonF() << QPointF(39 + lean, 20) << QPointF(48 + lean, 8)
                                  << QPointF(56 + lean, 20));
        p.setBrush(QColor("#24345d"));
        p.drawRoundedRect(QRectF(40 + lean, 29, 16, 5), 2, 2);
        p.setBrush(QColor("#61f7ff"));
        p.drawRect(QRectF(42 + lean, 30, 4, 2));
        p.drawRect(QRectF(51 + lean, 30, 4, 2));

        p.setPen(QPen(QColor("#fff8c7"), pose == 2 ? 8 : 5, Qt::SolidLine, Qt::RoundCap));
        if (pose == 1)
            p.drawLine(QPointF(54, 58), QPointF(86, 14));
        else
            p.drawLine(QPointF(70, 66), QPointF(72, 13));
        p.setPen(QPen(QColor("#69f6ff"), 2, Qt::SolidLine, Qt::RoundCap));
        if (pose == 1)
            p.drawLine(QPointF(57, 55), QPointF(86, 14));
        else
            p.drawLine(QPointF(72, 63), QPointF(73, 15));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#ffe16a"));
        p.drawRoundedRect(QRectF(58, 56, 18, 6), 2, 2);

    } else if (type == 1) {
        // Astral mage: floating robe, comet staff, orbiting runes.
        QLinearGradient robe(31, 26, 65, 80);
        robe.setColorAt(0.0, QColor("#82fbff"));
        robe.setColorAt(0.45, QColor("#326bd6"));
        robe.setColorAt(1.0, QColor("#151b65"));
        p.setBrush(robe);
        p.drawPolygon(QPolygonF() << QPointF(38, 34) << QPointF(58, 34)
                                  << QPointF(70, 82) << QPointF(26, 82));
        p.setBrush(QColor("#0a123c"));
        p.drawPolygon(QPolygonF() << QPointF(45, 37) << QPointF(51, 37)
                                  << QPointF(55, 80) << QPointF(41, 80));

        p.setBrush(QColor("#f3bd7d"));
        p.drawEllipse(QRectF(38, 23, 20, 18));
        QLinearGradient hat(34, 8, 62, 34);
        hat.setColorAt(0.0, QColor("#bdfcff"));
        hat.setColorAt(0.55, QColor("#3b2ed6"));
        hat.setColorAt(1.0, QColor("#120b47"));
        p.setBrush(hat);
        p.drawPolygon(QPolygonF() << QPointF(31, 26) << QPointF(48, 5)
                                  << QPointF(65, 26) << QPointF(58, 35)
                                  << QPointF(38, 35));
        p.setBrush(QColor("#ffe66d"));
        p.drawRoundedRect(QRectF(35, 30, 26, 5), 2, 2);
        p.setBrush(QColor("#ecfbff"));
        p.drawRect(QRectF(41, 29, 5, 3));
        p.drawRect(QRectF(51, 29, 5, 3));

        p.setPen(QPen(QColor("#6b3b1f"), 4, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(pose == 1 ? 22 : 25, 28), QPointF(pose == 1 ? 21 : 25, 80));
        p.setPen(Qt::NoPen);
        QRadialGradient orb(24, 22, pose == 2 ? 14 : 10);
        orb.setColorAt(0.0, QColor("#ffffff"));
        orb.setColorAt(0.38, QColor("#7df7ff"));
        orb.setColorAt(1.0, QColor(125, 247, 255, 0));
        p.setBrush(orb);
        p.drawEllipse(QRectF(13, 11, 22, 22));

        p.setPen(QPen(QColor("#a8fbff"), 2, Qt::DashLine, Qt::RoundCap));
        p.drawArc(QRectF(18, 20, 60, 40), 20 * 16, 250 * 16);
        p.drawArc(QRectF(14, 28, 68, 30), 200 * 16, 230 * 16);
        p.setPen(Qt::NoPen);
        sparkle(p, 68, 25, 4, QColor("#ffe66d"));
        sparkle(p, 72, 61, 5, QColor("#7df7ff"));
        if (pose == 1 || pose == 2) {
            p.setPen(QPen(QColor("#7df7ff"), pose == 2 ? 6 : 4, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(56, 47), QPointF(85, 40));
            p.setPen(Qt::NoPen);
            sparkle(p, 86, 39, 7, QColor("#ffffff"));
        }

    } else {
        // Ember ranger: feathered hood, leaf armor, bright energy bow.
        const qreal aim = pose == 1 ? -5 : 0;
        p.setBrush(QColor("#233318"));
        p.drawPolygon(QPolygonF() << QPointF(27, 28) << QPointF(48, 11)
                                  << QPointF(69, 28) << QPointF(61, 45)
                                  << QPointF(35, 45));
        p.setBrush(QColor("#f0b678"));
        p.drawRoundedRect(QRectF(38, 27, 20, 17), 7, 7);
        p.setBrush(QColor("#ffe46a"));
        p.drawRect(QRectF(41, 33, 5, 3));
        p.drawRect(QRectF(51, 33, 5, 3));

        QLinearGradient leather(31, 42, 65, 78);
        leather.setColorAt(0.0, QColor("#f59a45"));
        leather.setColorAt(0.45, QColor("#4f9f57"));
        leather.setColorAt(1.0, QColor("#1b4028"));
        p.setBrush(leather);
        p.drawRoundedRect(QRectF(32, 42, 32, 29), 8, 8);
        p.setBrush(QColor("#502910"));
        p.drawPolygon(QPolygonF() << QPointF(39, 43) << QPointF(50, 43)
                                  << QPointF(58, 71) << QPointF(48, 72));
        p.setBrush(QColor("#172a1d"));
        p.drawRoundedRect(QRectF(34, 69, 11, 15), 4, 4);
        p.drawRoundedRect(QRectF(52, 69, 11, 15), 4, 4);
        p.setBrush(QColor("#5b2f16"));
        p.drawRoundedRect(QRectF(29, 80, 18, 6), 3, 3);
        p.drawRoundedRect(QRectF(50, 80, 18, 6), 3, 3);

        p.setBrush(QColor("#7a3e18"));
        p.drawRoundedRect(QRectF(62, 30, 11, 34), 4, 4);
        p.setBrush(QColor("#ffe16a"));
        p.drawRect(QRectF(64, 22, 3, 14));
        p.drawRect(QRectF(69, 24, 3, 12));

        p.setPen(QPen(QColor("#ffcf62"), pose == 2 ? 6 : 4, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(QRectF(15, 24 + aim, 27, 48), 80 * 16, 205 * 16);
        p.setPen(QPen(QColor("#f8f1c7"), 1.8, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(26, 27 + aim), QPointF(pose == 1 ? 60 : 31, 50));
        p.drawLine(QPointF(26, 71 + aim), QPointF(pose == 1 ? 60 : 31, 50));
        if (pose == 1 || pose == 2) {
            p.setPen(QPen(QColor("#ffe16a"), pose == 2 ? 4 : 3, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(30, 50), QPointF(86, pose == 2 ? 42 : 50));
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#fff7b0"));
            p.drawPolygon(QPolygonF() << QPointF(86, pose == 2 ? 42 : 50)
                                      << QPointF(76, pose == 2 ? 36 : 44)
                                      << QPointF(77, pose == 2 ? 48 : 56));
            sparkle(p, 79, pose == 2 ? 37 : 46, 4, QColor("#ffef8a"));
        }
        p.setPen(Qt::NoPen);
    }

    p.end();
    return pixmap;
}

static QString resolveAssetPath(const QString& relativePath) {
    if (QFile::exists(relativePath)) return relativePath;

    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir(appDir).filePath(relativePath),
        QDir(appDir).filePath(QStringLiteral("../") + relativePath),
        QDir(appDir).filePath(QStringLiteral("../../") + relativePath)
    };
    for (const QString& p : candidates) {
        const QString clean = QDir::cleanPath(p);
        if (QFile::exists(clean)) return clean;
    }
    return QString();
}

static QString spriteFolderForType(int type) {
    if (type == 0) return QStringLiteral("Knight_1");
    if (type == 1) return QStringLiteral("Wanderer Magican");
    return QStringLiteral("Gorgon_2");
}

static QString spriteSheetPathForPose(int type, int pose) {
    int effectivePose = pose;
    // Gorgon feels better if normal attack uses the snappier strip.
    if (type == 2) {
        if (pose == 1) effectivePose = 2;
        else if (pose == 2) effectivePose = 1;
    }

    auto buildPath = [&](const QStringList& names) -> QString {
        const QString folder = spriteFolderForType(type);
        for (const QString& n : names) {
            const QString p = QStringLiteral("assets/sprites/%1/%2").arg(folder, n);
            const QString resolved = resolveAssetPath(p);
            if (!resolved.isEmpty()) return resolved;
        }
        return QString();
    };

    if (effectivePose == 3) {
        return buildPath({QStringLiteral("Dead.png")});
    }
    if (effectivePose == 1) {
        return buildPath({
            QStringLiteral("Attack_1.png"),
            QStringLiteral("Attack 1.png"),
            QStringLiteral("Attack_2.png"),
            QStringLiteral("Attack 2.png"),
            QStringLiteral("Attack_3.png"),
            QStringLiteral("Attack 3.png")
        });
    }
    if (effectivePose == 2) {
        return buildPath({
            QStringLiteral("Special.png"),
            QStringLiteral("Fireball.png"),
            QStringLiteral("Flame_jet.png"),
            QStringLiteral("Light_ball.png"),
            QStringLiteral("Light_charge.png"),
            QStringLiteral("Magic_sphere.png"),
            QStringLiteral("Magic_arrow.png"),
            QStringLiteral("Charge.png"),
            QStringLiteral("Charge_1.png"),
            QStringLiteral("Charge_2.png"),
            QStringLiteral("Protect.png"),
            QStringLiteral("Defend.png"),
            QStringLiteral("Run+Attack.png")
        });
    }
    if (effectivePose == 4) {
        const QString walk = buildPath({QStringLiteral("Walk.png"), QStringLiteral("Run.png")});
        if (!walk.isEmpty()) return walk;
    }
    return buildPath({QStringLiteral("Idle.png"), QStringLiteral("Idle_2.png"), QStringLiteral("Walk.png")});
}

static int spriteFrameCountForPose(int type, int pose) {
    const QString path = spriteSheetPathForPose(type, pose);
    QPixmap sheet(path);
    if (sheet.isNull() || sheet.height() <= 0 || sheet.width() < sheet.height()) return 1;
    return std::max(1, sheet.width() / sheet.height());
}

static QPixmap makeArcadeSprite(int type, int size, int pose = 0, int facing = 0, int frame = 0) {
    const bool backFacing = (facing == 1);
    const QString sheetPath = spriteSheetPathForPose(type, pose);
    if (!sheetPath.isEmpty()) {
        QPixmap sheet(sheetPath);
        if (!sheet.isNull() && sheet.height() > 0 && sheet.width() >= sheet.height()) {
            const int frameSide = sheet.height();
            const int frameCount = std::max(1, sheet.width() / frameSide);
            const int safeIndex = ((frame % frameCount) + frameCount) % frameCount;
            QPixmap framePx = sheet.copy(safeIndex * frameSide, 0, frameSide, frameSide);

            // Trim transparent margins so the character fills more of the target size.
            const QImage frameImg = framePx.toImage().convertToFormat(QImage::Format_ARGB32);
            int minX = frameImg.width(), minY = frameImg.height();
            int maxX = -1, maxY = -1;
            for (int y = 0; y < frameImg.height(); ++y) {
                const QRgb* row = reinterpret_cast<const QRgb*>(frameImg.constScanLine(y));
                for (int x = 0; x < frameImg.width(); ++x) {
                    if (qAlpha(row[x]) > 8) {
                        minX = std::min(minX, x);
                        minY = std::min(minY, y);
                        maxX = std::max(maxX, x);
                        maxY = std::max(maxY, y);
                    }
                }
            }
            if (maxX >= minX && maxY >= minY) {
                const QRect alphaBounds(minX, minY, maxX - minX + 1, maxY - minY + 1);
                framePx = framePx.copy(alphaBounds);
            }

            // Side profile handling: right = original, left = mirrored.
            if (facing == 2) framePx = framePx.transformed(QTransform().scale(-1, 1));

            // Back profile handling: same strip with subtle dark tint.
            if (backFacing) {
                QPainter tintPainter(&framePx);
                tintPainter.fillRect(framePx.rect(), QColor(0, 0, 0, 34));
            }

            return framePx.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    return makeNeonChampionSprite(type, size, pose, facing, frame);
}

// (Removed: ~440 lines of unreachable legacy 16x16 pixel-art sprite code.
//  The active fallback when a sprite sheet fails to load is
//  makeNeonChampionSprite() above, which returns just before this.)

#if 0  // dead code, kept for reference only — never executed
static void makeArcadeSpriteLegacy(int type, int size, int pose) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);

    // Pixel grid unit — scale everything to 'size'
    // We design in a 16×16 grid and scale up
    const int G = size / 16;
    if (G < 1) { p.end(); return pixmap; }
    drawSpriteStage(p, type, pose, G);

    // --- WARRIOR ---
    if (type == 0) {
        QColor skin("#f5c07a");
        QColor hair("#3a2010");
        QColor armor("#b8920a");
        QColor armorLight("#f0d060");
        QColor armorDark("#806400");
        QColor belt("#2a1800");
        QColor boot("#2a1a0a");
        QColor sword("#d0d8e8");
        QColor swordGold("#f0c030");
        QColor blood("#d94f4f");
        QColor spark("#fff080");

        if (pose == 0) {
            // ── IDLE: standing at attention ──────────────────
            // helmet
            px(p, 6*G, 0,    4*G, G,   armorDark);
            px(p, 5*G, G,    6*G, G,   armor);
            px(p, 5*G, 2*G,  G,   G,   armorDark);   // visor left
            px(p, 10*G,2*G,  G,   G,   armorDark);   // visor right
            px(p, 6*G, 2*G,  4*G, G,   skin);        // face
            px(p, 7*G, 2*G,  G,   G,   hair);        // eye shadow
            px(p, 8*G, 2*G,  G,   G,   hair);
            // shoulders
            px(p, 3*G, 3*G,  3*G, 2*G, armor);
            px(p, 10*G,3*G,  3*G, 2*G, armor);
            px(p, 3*G, 3*G,  G,   G,   armorLight);  // shoulder highlight
            px(p, 12*G,3*G,  G,   G,   armorLight);
            // torso
            px(p, 5*G, 3*G,  6*G, 5*G, armor);
            px(p, 5*G, 3*G,  G,   5*G, armorLight);  // chest highlight
            px(p, 7*G, 4*G,  2*G, 2*G, armorDark);   // chest emblem
            // belt
            px(p, 5*G, 8*G,  6*G, G,   belt);
            px(p, 7*G, 8*G,  2*G, G,   swordGold);   // buckle
            // arms
            px(p, 3*G, 5*G,  2*G, 4*G, armorDark);
            px(p, 11*G,5*G,  2*G, 4*G, armorDark);
            // hands/gauntlets
            px(p, 3*G, 9*G,  2*G, G,   armorDark);
            px(p, 11*G,9*G,  2*G, G,   armorDark);
            // legs
            px(p, 5*G, 9*G,  2*G, 5*G, armorDark);
            px(p, 9*G, 9*G,  2*G, 5*G, armorDark);
            px(p, 5*G, 9*G,  G,   5*G, armor);       // leg highlight
            px(p, 9*G, 9*G,  G,   5*G, armor);
            // boots
            px(p, 4*G, 14*G, 3*G, 2*G, boot);
            px(p, 9*G, 14*G, 3*G, 2*G, boot);
            // sword (right side, pointing up)
            px(p, 13*G,0,    G,   10*G,sword);
            px(p, 13*G,0,    G,   G,   armorLight);  // tip
            px(p, 12*G,4*G,  3*G, G,   swordGold);   // crossguard

        } else if (pose == 1) {
            // ── ATTACK: sword raised, lunging right ──────────
            // helmet (tilted forward)
            px(p, 5*G, G,    5*G, G,   armorDark);
            px(p, 4*G, 2*G,  6*G, G,   armor);
            px(p, 4*G, 3*G,  G,   G,   armorDark);
            px(p, 5*G, 3*G,  4*G, G,   skin);
            // angry eyes
            px(p, 5*G, 3*G,  G,   G,   hair);
            px(p, 7*G, 3*G,  G,   G,   hair);
            // torso leaning forward
            px(p, 4*G, 4*G,  8*G, 4*G, armor);
            px(p, 4*G, 4*G,  G,   4*G, armorLight);
            px(p, 6*G, 5*G,  2*G, 2*G, armorDark);
            // belt
            px(p, 4*G, 8*G,  7*G, G,   belt);
            // legs (wide stance)
            px(p, 3*G, 9*G,  2*G, 6*G, armorDark);
            px(p, 9*G, 9*G,  3*G, 6*G, armorDark);
            px(p, 3*G, 9*G,  G,   6*G, armor);
            px(p, 9*G, 9*G,  G,   6*G, armor);
            px(p, 2*G, 14*G, 3*G, 2*G, boot);
            px(p, 9*G, 14*G, 4*G, 2*G, boot);
            // sword arm extended forward+up (right side)
            px(p, 9*G, 4*G,  4*G, G,   armorDark);   // arm
            // SWORD slashing diagonally — bright
            px(p, 11*G,0,    G,   8*G, sword);
            px(p, 12*G,0,    G,   2*G, armorLight);  // tip glow
            px(p, 10*G,3*G,  4*G, G,   swordGold);   // crossguard
            // motion slash lines
            px(p, 13*G,2*G,  2*G, G,   QColor(255,255,200,180));
            px(p, 14*G,4*G,  2*G, G,   QColor(255,255,200,120));
            px(p, 14*G,1*G,  2*G, G,   QColor(255,255,200,100));
            // left arm (shield up)
            px(p, 2*G, 4*G,  3*G, 4*G, armorDark);
            px(p, G,   4*G,  2*G, 5*G, armor);       // shield

        } else {
            // ── SPECIAL: power strike — glowing golden blade ─
            // helmet
            px(p, 5*G, 0,    6*G, G,   armorDark);
            px(p, 4*G, G,    8*G, G,   armor);
            px(p, 4*G, 2*G,  G,   G,   armorDark);
            px(p, 5*G, 2*G,  4*G, G,   skin);
            // determined eyes (narrowed)
            px(p, 5*G, 2*G,  2*G, G,   hair);
            px(p, 7*G, 2*G,  G,   G,   hair);
            // torso
            px(p, 4*G, 3*G,  8*G, 5*G, armor);
            px(p, 4*G, 3*G,  G,   5*G, armorLight);
            px(p, 6*G, 4*G,  2*G, 3*G, armorDark);
            // belt
            px(p, 4*G, 8*G,  8*G, G,   belt);
            px(p, 7*G, 8*G,  2*G, G,   swordGold);
            // legs wide planted
            px(p, 3*G, 9*G,  2*G, 6*G, armorDark);
            px(p, 10*G,9*G,  3*G, 6*G, armorDark);
            px(p, 2*G, 14*G, 4*G, 2*G, boot);
            px(p, 10*G,14*G, 4*G, 2*G, boot);
            // both arms raised — sword held aloft two-handed
            px(p, 2*G, 3*G,  3*G, 5*G, armorDark);  // left arm up
            px(p, 11*G,3*G,  3*G, 5*G, armorDark);  // right arm up
            // HUGE glowing sword above head
            px(p, 7*G, 0,    2*G, 4*G, swordGold);
            px(p, 6*G, 0,    G,   4*G, QColor(255,220,50,200));
            px(p, 9*G, 0,    G,   4*G, QColor(255,220,50,200));
            px(p, 7*G, 0,    2*G, G,   QColor(255,255,180,255)); // tip super bright
            // golden aura sparks
            for (int i = 0; i < 8; i++) {
                int sx = (1 + (i*3)%14) * G;
                int sy = (i % 4) * G;
                px(p, sx, sy, G, G, QColor(255, 220, 60, 200 - i*20));
            }
            // crossguard
            px(p, 4*G, 4*G,  8*G, G,   swordGold);
        }
    }

    // --- MAGE ---
    else if (type == 1) {
        QColor skin("#f4c07a");
        QColor robe("#1a6a7a");
        QColor robeDark("#0d4050");
        QColor robeLight("#30b0c8");
        QColor hat("#0d4050");
        QColor hatBand("#d4a017");
        QColor staff("#6b3a1f");
        QColor orb("#80ffff");
        QColor orbGlow("#40e0ff");
        QColor rune("#ffe060");
        QColor eye("#2020a0");
        QColor beard("#e8e0d0");

        if (pose == 0) {
            // ── IDLE: standing, staff in hand ────────────────
            // pointy hat
            px(p, 7*G, 0,    2*G, G,   hat);
            px(p, 6*G, G,    4*G, G,   hat);
            px(p, 5*G, 2*G,  6*G, G,   hat);
            px(p, 5*G, 2*G,  6*G, G,   hat);
            px(p, 5*G, 3*G,  G,   G,   hatBand);     // hat band details
            px(p, 10*G,3*G,  G,   G,   hatBand);
            px(p, 4*G, 3*G,  8*G, G,   hat);
            px(p, 4*G, 4*G,  8*G, G,   hatBand);     // band
            // face
            px(p, 5*G, 5*G,  6*G, 3*G, skin);
            px(p, 6*G, 5*G,  G,   G,   eye);         // left eye
            px(p, 9*G, 5*G,  G,   G,   eye);         // right eye
            px(p, 6*G, 6*G,  G,   G,   QColor("#2a180a")); // brow
            px(p, 9*G, 6*G,  G,   G,   QColor("#2a180a"));
            // beard
            px(p, 5*G, 7*G,  6*G, 2*G, beard);
            px(p, 6*G, 8*G,  4*G, G,   beard);
            // robe body
            px(p, 4*G, 8*G,  8*G, 6*G, robe);
            px(p, 4*G, 8*G,  G,   6*G, robeLight);   // robe edge highlight
            px(p, 11*G,8*G,  G,   6*G, robeDark);
            // robe trim
            px(p, 4*G, 13*G, 8*G, G,   robeLight);
            // sleeves wide
            px(p, 2*G, 8*G,  3*G, 4*G, robe);
            px(p, 11*G,8*G,  3*G, 4*G, robe);
            px(p, 2*G, 8*G,  G,   4*G, robeLight);
            // hands
            px(p, 2*G, 12*G, 2*G, G,   skin);
            px(p, 12*G,12*G, 2*G, G,   skin);
            // feet peeking below robe
            px(p, 5*G, 14*G, 2*G, 2*G, robeDark);
            px(p, 9*G, 14*G, 2*G, 2*G, robeDark);
            // staff (left side)
            px(p, G,   2*G,  G,   12*G,staff);
            // orb atop staff
            px(p, 0,   G,    3*G, 2*G, orb);
            px(p, 0,   G,    G,   G,   QColor(200,255,255,200)); // orb shine

        } else if (pose == 1) {
            // ── ATTACK: staff extended, shooting bolt ─────────
            // hat (tilted slightly)
            px(p, 7*G, 0,    2*G, G,   hat);
            px(p, 6*G, G,    4*G, G,   hat);
            px(p, 5*G, 2*G,  6*G, G,   hat);
            px(p, 4*G, 3*G,  8*G, G,   hat);
            px(p, 4*G, 4*G,  8*G, G,   hatBand);
            // face
            px(p, 5*G, 5*G,  6*G, 3*G, skin);
            px(p, 6*G, 5*G,  G,   G,   eye);
            px(p, 9*G, 5*G,  G,   G,   eye);
            // concentrating brows
            px(p, 5*G, 5*G,  2*G, G,   QColor("#2a180a"));
            px(p, 9*G, 5*G,  2*G, G,   QColor("#2a180a"));
            px(p, 6*G, 7*G,  4*G, G,   beard);
            // robe
            px(p, 4*G, 8*G,  8*G, 6*G, robe);
            px(p, 4*G, 8*G,  G,   6*G, robeLight);
            px(p, 11*G,8*G,  G,   6*G, robeDark);
            px(p, 4*G, 13*G, 8*G, G,   robeLight);
            // right arm extended forward casting
            px(p, 11*G,8*G,  4*G, 2*G, robe);
            px(p, 14*G,8*G,  G,   2*G, skin);        // extended hand
            // feet
            px(p, 5*G, 14*G, 2*G, 2*G, robeDark);
            px(p, 9*G, 14*G, 2*G, 2*G, robeDark);
            // MAGIC BOLT shooting right
            px(p, 15*G,7*G,  G,   G,   orbGlow);     // bolt core
            px(p, 14*G,8*G,  2*G, G,   QColor(100,240,255,220));
            // bolt trail sparks
            px(p, 13*G,6*G,  G,   G,   QColor(200,255,255,180));
            px(p, 14*G,5*G,  G,   G,   QColor(100,240,255,140));
            px(p, 15*G,6*G,  G,   G,   QColor(60, 200,255,100));
            // left hand with staff
            px(p, 2*G, 8*G,  2*G, 5*G, robe);
            px(p, G,   3*G,  G,   12*G,staff);
            px(p, 0,   2*G,  3*G, 2*G, orb);
            px(p, 0,   2*G,  G,   G,   QColor(200,255,255,200));
            // casting glow on the mage
            px(p, 11*G,8*G,  G,   G,   QColor(100,240,255,100));

        } else {
            // ── SPECIAL: Arcane Storm — runes swirling ────────
            // hat with glow
            px(p, 7*G, 0,    2*G, G,   hat);
            px(p, 6*G, G,    4*G, G,   hat);
            px(p, 5*G, 2*G,  6*G, G,   hat);
            px(p, 4*G, 3*G,  8*G, G,   QColor(30,100,180));   // glowing hat
            px(p, 4*G, 4*G,  8*G, G,   QColor(60,180,255));   // glowing band
            // face eyes glowing
            px(p, 5*G, 5*G,  6*G, 3*G, skin);
            px(p, 6*G, 5*G,  2*G, G,   QColor(100,220,255));  // glowing eyes
            px(p, 9*G, 5*G,  2*G, G,   QColor(100,220,255));
            px(p, 6*G, 7*G,  4*G, G,   beard);
            // robe with arcane energy
            px(p, 3*G, 8*G,  10*G,6*G, QColor(20,80,120));    // dark robe base
            px(p, 3*G, 8*G,  G,   6*G, QColor(60,160,255));   // bright edge
            px(p, 12*G,8*G,  G,   6*G, QColor(60,160,255));
            px(p, 3*G, 13*G, 10*G,G,   QColor(60,160,255));
            // both arms raised wide
            px(p, G,   7*G,  3*G, 4*G, QColor(20,80,120));
            px(p, 12*G,7*G,  3*G, 4*G, QColor(20,80,120));
            // rune symbols scattered
            px(p, 0,   3*G,  2*G, G,   rune);
            px(p, 14*G,3*G,  2*G, G,   rune);
            px(p, 0,   10*G, G,   2*G, rune);
            px(p, 15*G,10*G, G,   2*G, rune);
            px(p, 3*G, 0,    G,   2*G, rune);
            px(p, 13*G,0,    G,   2*G, QColor(100,220,255));
            // central arcane burst
            px(p, 7*G, 3*G,  2*G, 2*G, QColor(255,255,200,230));
            px(p, 6*G, 4*G,  4*G, G,   QColor(200,240,255,200));
            px(p, 7*G, 5*G,  2*G, G,   QColor(255,255,255,180));
            // feet
            px(p, 5*G, 14*G, 2*G, 2*G, robeDark);
            px(p, 9*G, 14*G, 2*G, 2*G, robeDark);
        }
    }

    // --- ARCHER ---
    else {
        QColor skin("#f4c07a");
        QColor leather("#a05018");
        QColor leatherLight("#d07028");
        QColor leatherDark("#603010");
        QColor hood("#3a2010");
        QColor cloak("#5a3018");
        QColor bowWood("#8b5010");
        QColor bowString("#e8d8b0");
        QColor arrow("#d8c890");
        QColor arrowHead("#a0b0c0");
        QColor eye("#2a1800");
        QColor quiver("#4a2808");

        if (pose == 0) {
            // ── IDLE: relaxed, bow at side ────────────────────
            // hood
            px(p, 6*G, 0,    4*G, G,   hood);
            px(p, 5*G, G,    6*G, G,   hood);
            px(p, 5*G, 2*G,  G,   G,   hood);         // shadow under hood
            px(p, 10*G,2*G,  G,   G,   hood);
            // face
            px(p, 6*G, 2*G,  4*G, 3*G, skin);
            px(p, 7*G, 3*G,  G,   G,   eye);           // eyes
            px(p, 9*G, 3*G,  G,   G,   eye);
            px(p, 7*G, 4*G,  2*G, G,   QColor("#8b4513")); // nose/mouth hint
            // cloak/hood drape
            px(p, 4*G, 3*G,  G,   4*G, cloak);
            px(p, 11*G,3*G,  G,   4*G, cloak);
            // torso
            px(p, 5*G, 5*G,  6*G, 5*G, leather);
            px(p, 5*G, 5*G,  G,   5*G, leatherLight);  // torso highlight
            px(p, 10*G,5*G,  G,   5*G, leatherDark);
            // armor straps
            px(p, 7*G, 5*G,  2*G, G,   leatherDark);   // chest strap
            px(p, 7*G, 7*G,  2*G, G,   leatherDark);
            // arms
            px(p, 3*G, 5*G,  2*G, 4*G, leather);
            px(p, 11*G,5*G,  2*G, 4*G, leather);
            // quiver (back right)
            px(p, 12*G,2*G,  2*G, 8*G, quiver);
            px(p, 12*G,2*G,  2*G, G,   leatherDark);   // quiver top
            // arrow shafts in quiver
            px(p, 13*G,0,    G,   4*G, arrow);
            px(p, 12*G,G,    G,   3*G, arrow);
            // legs
            px(p, 5*G, 10*G, 2*G, 5*G, leatherDark);
            px(p, 9*G, 10*G, 2*G, 5*G, leatherDark);
            px(p, 5*G, 10*G, G,   5*G, leather);
            // boots
            px(p, 4*G, 14*G, 3*G, 2*G, leatherDark);
            px(p, 9*G, 14*G, 3*G, 2*G, leatherDark);
            // bow (left side, relaxed)
            p.setPen(QPen(QColor(bowWood), G*1.5));
            p.drawArc(0, 2*G, 4*G, 12*G, 30*16, 120*16);
            p.setPen(QPen(QColor(bowString), (int)(G*0.5 + 1)));
            p.drawLine(G, 4*G, G, 12*G);
            p.setPen(Qt::NoPen);

        } else if (pose == 1) {
            // ── ATTACK: drawing bowstring, aiming ────────────
            // hood
            px(p, 6*G, 0,    4*G, G,   hood);
            px(p, 5*G, G,    6*G, G,   hood);
            px(p, 5*G, 2*G,  G,   G,   hood);
            px(p, 10*G,2*G,  G,   G,   hood);
            // face — focused, leaning forward
            px(p, 6*G, 2*G,  4*G, 3*G, skin);
            px(p, 7*G, 3*G,  G,   G,   eye);
            px(p, 9*G, 3*G,  G,   G,   eye);
            // focus squint brow lines
            px(p, 6*G, 2*G,  2*G, G,   hood);
            px(p, 8*G, 2*G,  2*G, G,   hood);
            // torso leaning into draw
            px(p, 4*G, 5*G,  7*G, 5*G, leather);
            px(p, 4*G, 5*G,  G,   5*G, leatherLight);
            px(p, 10*G,5*G,  G,   5*G, leatherDark);
            // draw arm pulled back (right arm)
            px(p, 10*G,5*G,  4*G, 2*G, leather);
            px(p, 13*G,5*G,  2*G, G,   skin);         // hand pulling string
            // bow arm extended left
            px(p, G,   5*G,  4*G, 2*G, leather);
            // quiver
            px(p, 12*G,2*G,  2*G, 7*G, quiver);
            // legs (wide stance)
            px(p, 4*G, 10*G, 2*G, 5*G, leatherDark);
            px(p, 9*G, 10*G, 3*G, 5*G, leatherDark);
            px(p, 4*G, 14*G, 3*G, 2*G, leatherDark);
            px(p, 9*G, 14*G, 4*G, 2*G, leatherDark);
            // ARROW nocked and drawn
            px(p, 3*G, 6*G,  10*G,G,   arrow);        // shaft
            px(p, 13*G,5*G,  3*G, 3*G, arrowHead);    // head (pointing right)
            px(p, 2*G, 5*G,  G,   3*G, QColor("#8b6030")); // fletching
            // BOW fully drawn (curved arc, taut)
            p.setPen(QPen(QColor(bowWood), G*2));
            p.drawArc(-G, 2*G, 6*G, 10*G, 40*16, 100*16);
            p.setPen(QPen(QColor(bowString), (int)(G*0.7 + 1)));
            // taut string pulled back to cheek
            p.drawLine(G, 3*G, 13*G, 6*G+G/2);
            p.drawLine(G, 11*G, 13*G, 6*G+G/2);
            p.setPen(Qt::NoPen);
            // tension highlight on bow arm
            px(p, 2*G, 5*G,  G,   G,   leatherLight);

        } else {
            // ── SPECIAL: Double Shot — two arrows, golden ────
            // hood
            px(p, 6*G, 0,    4*G, G,   hood);
            px(p, 5*G, G,    6*G, G,   hood);
            // face with golden gleam
            px(p, 6*G, 2*G,  4*G, 3*G, skin);
            px(p, 7*G, 3*G,  G,   G,   QColor("#d4a017")); // golden gleam in eyes
            px(p, 9*G, 3*G,  G,   G,   QColor("#d4a017"));
            // torso
            px(p, 4*G, 5*G,  7*G, 5*G, leather);
            px(p, 4*G, 5*G,  G,   5*G, leatherLight);
            // draw arm (right) pulled back
            px(p, 10*G,5*G,  4*G, 2*G, leather);
            px(p, 13*G,5*G,  2*G, G,   skin);
            // bow arm (left) extended
            px(p, G,   5*G,  4*G, 2*G, leather);
            // quiver
            px(p, 12*G,2*G,  2*G, 7*G, quiver);
            // legs
            px(p, 4*G, 10*G, 2*G, 5*G, leatherDark);
            px(p, 9*G, 10*G, 3*G, 5*G, leatherDark);
            px(p, 4*G, 14*G, 3*G, 2*G, leatherDark);
            px(p, 9*G, 14*G, 4*G, 2*G, leatherDark);
            // TWO glowing golden arrows!
            px(p, 3*G, 5*G,  10*G,G,   QColor("#f0c030"));  // arrow 1 (top)
            px(p, 3*G, 7*G,  10*G,G,   QColor("#f0c030"));  // arrow 2 (bottom)
            // golden arrowheads
            px(p, 13*G,4*G,  3*G, 3*G, QColor("#ffe060"));
            px(p, 13*G,6*G,  3*G, 3*G, QColor("#ffe060"));
            // golden fletching
            px(p, 2*G, 4*G,  G,   3*G, QColor("#d4a017"));
            px(p, 2*G, 6*G,  G,   3*G, QColor("#d4a017"));
            // BOW glowing gold
            p.setPen(QPen(QColor("#d4a017"), G*2));
            p.drawArc(-G, 2*G, 6*G, 10*G, 40*16, 100*16);
            p.setPen(QPen(QColor("#ffe080"), (int)(G*0.7 + 1)));
            p.drawLine(G, 3*G, 13*G, 6*G);
            p.drawLine(G, 11*G, 13*G, 8*G);
            p.setPen(Qt::NoPen);
            // golden aura sparks
            px(p, 14*G,2*G,  G,   G,   QColor(255,240,100,220));
            px(p, 15*G,5*G,  G,   G,   QColor(255,220,50,180));
            px(p, 14*G,8*G,  G,   G,   QColor(255,200,0,140));
            px(p, 15*G,10*G, G,   G,   QColor(255,240,100,120));
        }
    }

    p.end();
}
#endif  // dead-code reference block

// Wrapper that checks for image path first, then falls back to arcade sprite
static QPixmap makeCharacterPixmap(int type, int size,
                                   const QString& imagePath = QString(),
                                   int pose = 0) {
    if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
        QPixmap pm(imagePath);
        return pm.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return makeArcadeSprite(type, size, pose);
}

// ─── Battle sprite: same source pixmap as player, but for the enemy we add a
//     red glow halo + tint so a Warrior vs Warrior fight still reads clearly.
static QPixmap makeBattleSprite(int type, int size, int pose,
                                int facing, int frame, bool isEnemy) {
    QPixmap base = makeArcadeSprite(type, size, pose, facing, frame);
    if (!isEnemy) return base;

    QPixmap result(base.size());
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Build a flat-red silhouette of the sprite by re-coloring opaque pixels.
    QImage silh = base.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    {
        QPainter sp(&silh);
        sp.setCompositionMode(QPainter::CompositionMode_SourceIn);
        sp.fillRect(silh.rect(), QColor(255, 60, 60));
    }
    const QPixmap silhPx = QPixmap::fromImage(silh);

    // Stamp the silhouette around the original to produce an aggressive red outline.
    p.setOpacity(0.55);
    const int o = std::max(2, size / 64);
    for (int dx = -o; dx <= o; dx += o) {
        for (int dy = -o; dy <= o; dy += o) {
            if (dx == 0 && dy == 0) continue;
            p.drawPixmap(dx, dy, silhPx);
        }
    }
    p.setOpacity(1.0);

    // Base sprite on top of the red outline. No interior wash — the silhouette
    // stamps around the edges already make the enemy unmistakable.
    p.drawPixmap(0, 0, base);
    p.end();
    return result;
}

// ═══════════════════════════════════════════════════════════════
//  Audio — guarded by BATTLE_HAS_AUDIO so the project still compiles
//  on systems without qt6-multimedia-dev installed.
// ═══════════════════════════════════════════════════════════════

namespace BattleAudio {
#ifdef BATTLE_HAS_AUDIO
    static bool sAudioEnabled         = false;
    static constexpr float kMusicMenuVolume   = 0.34f;
    static constexpr float kMusicBattleVolume = 0.18f;
    static QMediaPlayer* sMusicPlayer    = nullptr;
    static QAudioOutput* sMusicOut       = nullptr;
    static QMediaPlayer* sClickPlayer    = nullptr;
    static QAudioOutput* sClickOut       = nullptr;
    static QMediaPlayer* sVictoryPlayer  = nullptr;
    static QAudioOutput* sVictoryOut     = nullptr;
    static QMediaPlayer* sGameOverPlayer = nullptr;
    static QAudioOutput* sGameOverOut    = nullptr;

    static QMediaPlayer* makePlayer(QObject* parent, const QString& fileRel,
                                    QAudioOutput** outSlot, float vol) {
        QMediaPlayer* p = new QMediaPlayer(parent);
        QAudioOutput* o = new QAudioOutput(parent);
        p->setAudioOutput(o);
        o->setVolume(vol);
        const QString resolved = resolveAssetPath(fileRel);
        if (!resolved.isEmpty())
            p->setSource(QUrl::fromLocalFile(resolved));
        *outSlot = o;
        return p;
    }

    static void restartPlayer(QMediaPlayer* player) {
        if (!player) return;
        const QUrl source = player->source();
        player->stop();
        player->setSource(QUrl());
        player->setSource(source);
        player->play();
    }

    static void setMusicBattleMode(bool inBattle) {
        if (!sAudioEnabled || !sMusicOut) return;
        sMusicOut->setVolume(inBattle ? kMusicBattleVolume : kMusicMenuVolume);
    }

    static void stopMusic() {
        if (!sAudioEnabled || !sMusicPlayer) return;
        sMusicPlayer->stop();
    }

    static void startMusic(bool inBattle) {
        if (!sAudioEnabled || !sMusicPlayer) return;
        setMusicBattleMode(inBattle);
        sMusicPlayer->play();
    }

    static bool shouldEnableAudio() {
#ifdef Q_OS_LINUX
        const QByteArray audioOptIn = qgetenv("BATTLE_ENABLE_AUDIO");
        if (audioOptIn == "1" || audioOptIn.compare("true", Qt::CaseInsensitive) == 0)
            return true;

        qDebug() << "Audio disabled on Linux by default to avoid multimedia backend freezes."
                 << "Set BATTLE_ENABLE_AUDIO=1 to re-enable it.";
        return false;
#else
        const QByteArray audioDisable = qgetenv("BATTLE_DISABLE_AUDIO");
        if (audioDisable == "1" || audioDisable.compare("true", Qt::CaseInsensitive) == 0) {
            qDebug() << "Audio disabled by BATTLE_DISABLE_AUDIO.";
            return false;
        }
        return true;
#endif
    }
#endif
}

void MainWindow::initAudio() {
#ifdef BATTLE_HAS_AUDIO
    if (!BattleAudio::shouldEnableAudio()) {
        BattleAudio::sAudioEnabled = false;
        return;
    }

    // Skip audio entirely if no output device is available.
    // This prevents GStreamer from blocking for 10+ seconds per player
    // on systems without a sound card (WSL, headless, etc.).
    if (QMediaDevices::audioOutputs().isEmpty()) {
        qDebug() << "No audio output device found — audio disabled.";
        BattleAudio::sAudioEnabled = false;
        return;
    }

    BattleAudio::sMusicPlayer = BattleAudio::makePlayer(
        this, "mondamusic-retro-arcade-game-music-512837.mp3",
        &BattleAudio::sMusicOut, BattleAudio::kMusicMenuVolume);

    BattleAudio::sClickPlayer = BattleAudio::makePlayer(
        this, "soundreality-sound-of-mouse-click-4-478760.mp3",
        &BattleAudio::sClickOut, 0.55f);

    BattleAudio::sVictoryPlayer = BattleAudio::makePlayer(
        this, "phatphrogstudio-phatphrogstudiocom-victory-fanfare-2-474663.mp3",
        &BattleAudio::sVictoryOut, 0.85f);

    BattleAudio::sGameOverPlayer = BattleAudio::makePlayer(
        this, "tuomas_data-game-over-31-179699.mp3",
        &BattleAudio::sGameOverOut, 0.85f);

    BattleAudio::sAudioEnabled = true;

    if (BattleAudio::sMusicPlayer) {
        QObject::connect(
            BattleAudio::sMusicPlayer, &QMediaPlayer::mediaStatusChanged,
            BattleAudio::sMusicPlayer,
            [](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia && BattleAudio::sMusicPlayer) {
                    BattleAudio::sMusicPlayer->setPosition(0);
                    BattleAudio::sMusicPlayer->play();
                }
            });
        BattleAudio::startMusic(false);
    }
#endif
}

void MainWindow::playClickSound() {
#ifdef BATTLE_HAS_AUDIO
    if (!BattleAudio::sAudioEnabled) return;
    if (!BattleAudio::sClickPlayer) return;
    // Restart from the beginning so rapid clicks always trigger.
    BattleAudio::sClickPlayer->stop();
    BattleAudio::sClickPlayer->setPosition(0);
    BattleAudio::sClickPlayer->play();
#endif
}

void MainWindow::playVictorySound() {
#ifdef BATTLE_HAS_AUDIO
    if (!BattleAudio::sAudioEnabled) return;
    if (!BattleAudio::sVictoryPlayer) return;
    BattleAudio::stopMusic();
    if (BattleAudio::sGameOverPlayer) BattleAudio::sGameOverPlayer->stop();
    BattleAudio::restartPlayer(BattleAudio::sVictoryPlayer);
#endif
}

void MainWindow::playGameOverSound() {
#ifdef BATTLE_HAS_AUDIO
    if (!BattleAudio::sAudioEnabled) return;
    if (!BattleAudio::sGameOverPlayer) return;
    BattleAudio::stopMusic();
    if (BattleAudio::sVictoryPlayer) BattleAudio::sVictoryPlayer->stop();
    BattleAudio::restartPlayer(BattleAudio::sGameOverPlayer);
#endif
}


// ═══════════════════════════════════════════════════════════════
//  Constructor / Destructor
// ═══════════════════════════════════════════════════════════════

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      selectedCharacter(nullptr),
      selectedType(-1)
{
    setWindowTitle("Battle Arena");
    resize(1700, 1050);
    setMinimumSize(1500, 950);

    // Defer audio init so the window appears instantly.
    // If no audio device exists, initAudio() will skip gracefully.
    QTimer::singleShot(0, this, &MainWindow::initAudio);
    // App-wide event filter so every QPushButton release plays the click sound.
    if (qApp) qApp->installEventFilter(this);

    gameManager = new GameManager(this);
    connect(gameManager, &GameManager::gameStateChanged,
            this,        &MainWindow::onGameStateChanged);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    buildMenuPage();
    buildCharacterPage();
    buildDifficultyPage();
    buildGamePage();
    buildGameOverPage();

    applyGlobalStyle();

    // ── Load API key from file ──────────────────────────────
    {
        const QStringList keyPaths = {
            QStringLiteral("api_key.txt"),
            QDir(QCoreApplication::applicationDirPath()).filePath("api_key.txt"),
            QDir(QCoreApplication::applicationDirPath()).filePath("../api_key.txt"),
            QDir(QCoreApplication::applicationDirPath()).filePath("../../api_key.txt")
        };
        for (const QString& kp : keyPaths) {
            QFile f(kp);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                loadedApiKey = QString::fromUtf8(f.readAll()).trimmed();
                f.close();
                if (!loadedApiKey.isEmpty()) break;
            }
        }
    }

    // ── Menu animation: cycle character poses ──────────────
    menuAnimTimer = new QTimer(this);
    connect(menuAnimTimer, &QTimer::timeout, this, [this]() {
        menuPoseFrame++;
        for (int i = 0; i < 3; i++) {
            if (menuSprites[i])
                menuSprites[i]->setPixmap(makeArcadeSprite(i, 178, 0, 0, menuPoseFrame));
        }
    });
    menuAnimTimer->start(300);

    // Keep battle sprites alive with lightweight idle/walk frame animation.
    battleAnimTimer = new QTimer(this);
    connect(battleAnimTimer, &QTimer::timeout, this, [this]() {
        if (!gameManager || gameManager->getState() != GameState::PLAYING) return;
        if (combatAnimLocks > 0) return; // don't overwrite attack/special playback
        battleAnimFrame++;

        if (playerToken)
            playerToken->setPixmap(makeBattleSprite(selectedType, CELL - 6, 0, playerFacing, battleAnimFrame, false));
        if (enemyToken)
            enemyToken->setPixmap(makeBattleSprite(enemyType, CELL - 6, 0, enemyFacing, battleAnimFrame + 2, true));

        if (playerPortraitLabel)
            playerPortraitLabel->setPixmap(makeBattleSprite(selectedType, 120, 0, playerFacing, battleAnimFrame, false));
        if (enemyPortraitLabel)
            enemyPortraitLabel->setPixmap(makeBattleSprite(enemyType, 120, 0, enemyFacing, battleAnimFrame + 2, true));
    });
    battleAnimTimer->start(320);

    gameOverAnimTimer = new QTimer(this);
    connect(gameOverAnimTimer, &QTimer::timeout, this, [this]() {
        if (!lblGOSprite) return;
        gameOverAnimFrame++;
        lblGOSprite->setPixmap(makeArcadeSprite(gameOverAnimType, 240, gameOverAnimPose, 0, gameOverAnimFrame));
    });
    gameOverAnimTimer->start(260);
    gameOverAnimTimer->stop();

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

struct FloatingParticle {
    float x, y, speed;
    int sz;
    QColor color;
};

class ArcadeBgWidget : public QWidget {
    QPixmap bgCache;
    QVector<FloatingParticle> particles;
    QTimer* animTimer;
public:
    explicit ArcadeBgWidget(QWidget* parent = nullptr) : QWidget(parent) {
        for (int i = 0; i < 35; i++) {
            FloatingParticle fp;
            fp.x = rand() % 1400;
            fp.y = rand() % 900;
            fp.speed = 0.3f + (rand() % 12) / 10.0f;
            fp.sz = 1 + rand() % 3;
            int ct = rand() % 4;
            if (ct == 0) fp.color = QColor(120, 80, 200, 90 + rand() % 80);
            else if (ct == 1) fp.color = QColor(200, 160, 40, 70 + rand() % 60);
            else if (ct == 2) fp.color = QColor(60, 180, 200, 70 + rand() % 60);
            else fp.color = QColor(220, 80, 80, 50 + rand() % 50);
            particles.append(fp);
        }
        animTimer = new QTimer(this);
        connect(animTimer, &QTimer::timeout, this, [this]() {
            for (auto& fp : particles) {
                fp.y -= fp.speed;
                fp.x += (rand() % 5 - 2) * 0.15f;
                if (fp.y < -10) {
                    fp.y = height() + 5;
                    fp.x = rand() % (width() > 0 ? width() : 1);
                }
            }
            update();
        });
        animTimer->start(50);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        if (bgCache.size() != size())
            bgCache = makeArcadeBg(width(), height());
        QPainter p(this);
        p.drawPixmap(0, 0, bgCache);
        p.setRenderHint(QPainter::Antialiasing, false);
        for (const auto& fp : particles)
            p.fillRect((int)fp.x, (int)fp.y, fp.sz, fp.sz, fp.color);
    }
    void resizeEvent(QResizeEvent* e) override {
        bgCache = QPixmap();
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
    lay->setSpacing(20);
    lay->setContentsMargins(80, 50, 80, 40);

    lay->addStretch(1);

    // ── coin row (decorative, no broken emoji) ───────────────
    QHBoxLayout* coinRow = new QHBoxLayout();
    coinRow->setSpacing(28);
    coinRow->setAlignment(Qt::AlignCenter);
    for (int i = 0; i < 9; i++) {
        QLabel* coin = new QLabel("◆");
        coin->setAlignment(Qt::AlignCenter);
        coin->setStyleSheet(QString("font-size: 20px; color: %1;")
            .arg(i % 2 == 0 ? "#d4a017" : "#7c5cbf"));
        coinRow->addWidget(coin);
    }

    // ── title ────────────────────────────────────────────────
    QLabel* title = new QLabel("BATTLE  ARENA");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 110px;
        font-weight: 900;
        letter-spacing: 18px;
        color: #ffffff;
        font-family: "Impact", "Arial Black", sans-serif;
    )");
    QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect();
    glow->setBlurRadius(72);
    glow->setColor(QColor("#7c5cbf"));
    glow->setOffset(0, 0);
    title->setGraphicsEffect(glow);

    // ── subtitle ─────────────────────────────────────────────
    QLabel* sub = new QLabel("CSCE  1101   ·   SPRING  2026   ·   TEAM  PROJECT");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet(
        "font-size: 16px; color: #7a7a99; letter-spacing: 8px; "
        "font-family: 'Courier New', monospace;"
    );


    // ── character preview row — ANIMATED sprites ─────────────
    QHBoxLayout* charRow = new QHBoxLayout();
    charRow->setSpacing(80);
    charRow->setAlignment(Qt::AlignCenter);
    QStringList charNames = {"WARRIOR", "MAGE", "GORGON"};
    QStringList charColors = {Pal::AMBER, Pal::TEAL, Pal::ORANGE};
    QStringList charWeapons = {"Sword & Shield", "Arcane Staff", "Stoneborn Claws"};
    for (int i = 0; i < 3; i++) {
        QVBoxLayout* col = new QVBoxLayout();
        col->setAlignment(Qt::AlignCenter);
        col->setSpacing(8);

        QLabel* portrait = new QLabel();
        portrait->setFixedSize(190, 190);
        portrait->setPixmap(makeArcadeSprite(i, 178, 0));
        portrait->setAlignment(Qt::AlignCenter);
        portrait->setStyleSheet(QString(
            "border: 2px solid %1; border-radius: 12px; "
            "background: qradialgradient(cx:0.5, cy:0.95, radius:0.7, "
            "  stop:0 rgba(60,40,120,140), stop:1 rgba(0,0,0,200));"
        ).arg(charColors[i]));
        menuSprites[i] = portrait;  // store for animation

        QLabel* name = new QLabel(charNames[i]);
        name->setAlignment(Qt::AlignCenter);
        name->setStyleSheet(QString(
            "font-size: 18px; color: %1; letter-spacing: 4px; font-weight: 900; "
            "font-family: 'Impact', 'Arial Black', sans-serif;"
        ).arg(charColors[i]));

        QLabel* weapon = new QLabel(charWeapons[i]);
        weapon->setAlignment(Qt::AlignCenter);
        weapon->setStyleSheet(QString(
            "font-size: 11px; color: %1; letter-spacing: 2px; "
            "font-family: 'Courier New', monospace;"
        ).arg(Pal::MUTED));

        col->addWidget(portrait);
        col->addWidget(name);
        col->addWidget(weapon);
        charRow->addLayout(col);
    }

    // ── decorative divider ───────────────────────────────────
    QLabel* divider = new QLabel("━━━━━━━━━━━━━━━━━━━━━━━━━━  ◆  ━━━━━━━━━━━━━━━━━━━━━━━━━━");
    divider->setAlignment(Qt::AlignCenter);
    divider->setStyleSheet("font-size: 14px; color: #2a2a4a; letter-spacing: 1px;");

    // ── PLAY button ──────────────────────────────────────────
    QPushButton* btnPlay = new QPushButton("▶  PLAY");
    btnPlay->setFixedSize(440, 88);
    btnPlay->setCursor(Qt::PointingHandCursor);
    btnPlay->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #5a3ea0, stop:1 #7c5cbf);
            color: #fff;
            font-size: 32px;
            font-weight: 900;
            letter-spacing: 10px;
            border-radius: 14px;
            border: 3px solid #9370d4;
            font-family: "Impact", "Arial Black", sans-serif;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #6b4db5, stop:1 #9370d4);
            border: 3px solid #b090e0;
        }
        QPushButton:pressed { background: #4a3090; }
    )");
    QGraphicsDropShadowEffect* playGlow = new QGraphicsDropShadowEffect();
    playGlow->setBlurRadius(40); playGlow->setColor(QColor("#7c5cbf")); playGlow->setOffset(0,0);
    btnPlay->setGraphicsEffect(playGlow);
    connect(btnPlay, &QPushButton::clicked, this, [=]() {
        stack->setCurrentWidget(characterPage);
    });

    // ── Controls hint panel ──────────────────────────────────
    QLabel* controls = new QLabel("↑ ↓ ← →  MOVE   ·   SPACE  ATTACK   ·   Q  SPECIAL");
    controls->setAlignment(Qt::AlignCenter);
    controls->setStyleSheet(R"(
        font-size: 14px; color: #6a6a90; letter-spacing: 4px;
        border: 1px solid #2a2a4a; border-radius: 8px;
        padding: 12px 28px;
        background: rgba(10,10,24,180);
        font-family: "Courier New", monospace;
    )");

    // ── credits ──────────────────────────────────────────────
    QLabel* credits = new QLabel("YOUSSEF GOHAR  ·  MOHAMED  ·  AHMED  ·  YOUSSEF AL-GHOBARY");
    credits->setAlignment(Qt::AlignCenter);
    credits->setStyleSheet(
        "font-size: 12px; color: #555580; letter-spacing: 4px; "
        "font-family: 'Courier New', monospace;"
    );

    QLabel* version = new QLabel("v2.0  ·  Qt6  ·  C++17");
    version->setAlignment(Qt::AlignCenter);
    version->setStyleSheet(
        "font-size: 11px; color: #2a2a48; letter-spacing: 4px; "
        "font-family: 'Courier New', monospace;"
    );

    // ── assemble ─────────────────────────────────────────────
    lay->addLayout(coinRow);
    lay->addSpacing(8);
    lay->addWidget(title,      0, Qt::AlignCenter);
    lay->addWidget(sub,        0, Qt::AlignCenter);
    lay->addSpacing(20);
    lay->addLayout(charRow);
    lay->addSpacing(10);
    lay->addWidget(divider,    0, Qt::AlignCenter);
    lay->addSpacing(8);
    lay->addWidget(btnPlay,    0, Qt::AlignCenter);
    lay->addSpacing(12);
    // load button
    QPushButton* loadBtn = new QPushButton("📂  LOAD GAME");
    loadBtn->setFixedSize(440, 60);
    loadBtn->setCursor(Qt::PointingHandCursor);
    loadBtn->setStyleSheet(R"(
    QPushButton {
        background: #1a3a5c; color: #7ab8e8; font-size: 20px; font-weight: 900;
        letter-spacing: 6px; border: 2px solid #2980b9; border-radius: 14px;
        font-family: "Impact", "Arial Black", sans-serif;
    }
    QPushButton:hover { background: #1e4a70; color: #ffffff; border: 2px solid #5aabf0; }
)");
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::onLoadClicked);
    lay->addWidget(loadBtn, 0, Qt::AlignCenter);
    lay->addSpacing(12);

    lay->addWidget(controls,   0, Qt::AlignCenter);
    lay->addStretch(1);
    lay->addWidget(credits,    0, Qt::AlignCenter);
    lay->addWidget(version,    0, Qt::AlignCenter);

    stack->addWidget(menuPage);
}

// ═══════════════════════════════════════════════════════════════
//  Page 1 — Character Select
// ═══════════════════════════════════════════════════════════════

// ─── Roster info shared by left list + right preview ────────────────────
struct RosterEntry {
    int     type;
    QString name;
    QString subtitle;
    QString special;
    QString desc;
    QString accent;
    QString emblem;     // single-glyph class crest
    int     hpVal;
    int     atkVal;
};

static const QList<RosterEntry>& rosterEntries() {
    static const QList<RosterEntry> kRoster = {
        {0, "WARRIOR", "Solar Knight",
         "Power Strike (1.5× ATK)",
         "A heavy-hitting frontliner built to absorb punishment and trade blows up close.",
         Pal::AMBER, "W", 200, 20},
        {1, "MAGE", "Wandering Magus",
         "Arcane Storm (3× ATK)",
         "Fragile but devastating — one well-timed burst can turn the entire fight.",
         Pal::TEAL,  "M", 100, 20},
        {2, "GORGON", "Stoneborn Hunter",
         "Stone Burst (2× ATK)",
         "Monstrous striker with relentless close-quarters pressure and brutal openers.",
         Pal::ORANGE, "G", 150, 15},
    };
    return kRoster;
}

void MainWindow::buildCharacterPage() {
    ArcadeBgWidget* bg = new ArcadeBgWidget();
    characterPage = bg;

    QVBoxLayout* root = new QVBoxLayout(characterPage);
    root->setContentsMargins(48, 28, 48, 24);
    root->setSpacing(18);

    // ── Title / sub ───────────────────────────────────────────
    QLabel* header = new QLabel("CHOOSE  YOUR  FIGHTER");
    header->setAlignment(Qt::AlignCenter);
    header->setStyleSheet(R"(
        font-size: 44px;
        font-weight: 900;
        color: #ffffff;
        letter-spacing: 10px;
        font-family: "Impact", "Arial Black", sans-serif;
    )");
    QGraphicsDropShadowEffect* hGlow = new QGraphicsDropShadowEffect();
    hGlow->setBlurRadius(38); hGlow->setColor(QColor("#7c5cbf")); hGlow->setOffset(0,0);
    header->setGraphicsEffect(hGlow);

    QLabel* subtitle = new QLabel("◆  STEP INTO THE ARENA — CLICK A CLASS TO PREVIEW THEM LIVE  ◆");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(
        "font-size: 12px; color: #7a7a99; letter-spacing: 6px; "
        "font-family: 'Courier New', monospace;"
    );

    // ── Two-pane row ──────────────────────────────────────────
    QHBoxLayout* twoPane = new QHBoxLayout();
    twoPane->setSpacing(28);

    // ════════════════════════════════════════════════════════
    //   LEFT — Roster panel (no character image here)
    // ════════════════════════════════════════════════════════
    QFrame* leftFrame = new QFrame();
    leftFrame->setObjectName("rosterFrame");
    leftFrame->setStyleSheet(R"(
        QFrame#rosterFrame {
            background: rgba(15, 15, 38, 215);
            border: 2px solid #2a2a4a;
            border-radius: 18px;
        }
    )");
    leftFrame->setMinimumWidth(560);
    leftFrame->setMaximumWidth(680);

    QVBoxLayout* leftLay = new QVBoxLayout(leftFrame);
    leftLay->setContentsMargins(28, 24, 28, 24);
    leftLay->setSpacing(14);

    QLabel* leftHeader = new QLabel("◢  ROSTER  ◣");
    leftHeader->setAlignment(Qt::AlignCenter);
    leftHeader->setStyleSheet(
        "font-size: 14px; color: #9b8edd; letter-spacing: 6px; "
        "font-weight: bold; font-family: 'Courier New', monospace;"
    );
    leftLay->addWidget(leftHeader);

    QLabel* leftDivider = new QLabel("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    leftDivider->setAlignment(Qt::AlignCenter);
    leftDivider->setStyleSheet("font-size: 11px; color: #2a2a4a;");
    leftLay->addWidget(leftDivider);

    const QList<RosterEntry>& roster = rosterEntries();
    for (int i = 0; i < roster.size(); ++i) {
        const RosterEntry& re = roster[i];
        QWidget* card = new QWidget();
        card->setObjectName(QString("rosterCard%1").arg(i));
        card->setMinimumHeight(100);
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(QString(R"(
            QWidget#rosterCard%1 {
                background: #12122a;
                border: 2px solid #2a2a4a;
                border-radius: 12px;
            }
            QWidget#rosterCard%1:hover {
                background: #1a1a38;
                border: 2px solid %2;
            }
        )").arg(i).arg(re.accent));

        QHBoxLayout* row = new QHBoxLayout(card);
        row->setContentsMargins(14, 12, 14, 12);
        row->setSpacing(14);

        // Class emblem (left): a colored square with the class glyph.
        QLabel* emblem = new QLabel();
emblem->setFixedSize(60, 60);
emblem->setPixmap(makeArcadeSprite(re.type, 56, 0, 0, 0));
emblem->setAlignment(Qt::AlignCenter);
emblem->setStyleSheet(QString(R"(
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 rgba(20,20,48,255),
        stop:1 rgba(40,30,80,255));
    border: 2px solid %1;
    border-radius: 8px;
)").arg(re.accent));

        // Right column: name + stat bars + special tagline
        QVBoxLayout* col = new QVBoxLayout();
        col->setSpacing(4);
        col->setContentsMargins(0, 0, 0, 0);

        QLabel* nameL = new QLabel(re.name);
        nameL->setStyleSheet(QString(
            "font-size: 17px; font-weight: 900; color: %1; "
            "letter-spacing: 4px; font-family: 'Impact', 'Arial Black', sans-serif;"
        ).arg(re.accent));

        QLabel* tagL = new QLabel(re.subtitle);
        tagL->setStyleSheet(
            "font-size: 9px; color: #7a7a99; letter-spacing: 3px; "
            "font-family: 'Courier New', monospace;"
        );

        // HP bar
        QHBoxLayout* hpRow = new QHBoxLayout();
        hpRow->setSpacing(8);
        QLabel* hpLabel = new QLabel("HP");
        hpLabel->setFixedWidth(30);
        hpLabel->setStyleSheet(
            "font-size: 11px; color: #3dba6e; font-weight: bold; "
            "font-family: 'Courier New', monospace;"
        );
        QProgressBar* hpBar = new QProgressBar();
        hpBar->setRange(0, 200);
        hpBar->setValue(re.hpVal);
        hpBar->setTextVisible(false);
        hpBar->setFixedHeight(7);
        hpBar->setStyleSheet(R"(
            QProgressBar { border: 1px solid #2a2a4a; border-radius: 4px; background: #0a0a18; }
            QProgressBar::chunk {
                border-radius: 4px;
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #3dba6e, stop:1 #28d47e);
            }
        )");
        QLabel* hpNum = new QLabel(QString::number(re.hpVal));
        hpNum->setFixedWidth(40);
        hpNum->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        hpNum->setStyleSheet(
            "font-size: 11px; color: #3dba6e; font-family: 'Courier New', monospace;"
        );
        hpRow->addWidget(hpLabel);
        hpRow->addWidget(hpBar, 1);
        hpRow->addWidget(hpNum);

        // ATK bar
        QHBoxLayout* atkRow = new QHBoxLayout();
        atkRow->setSpacing(8);
        QLabel* atkLabel = new QLabel("ATK");
        atkLabel->setFixedWidth(30);
        atkLabel->setStyleSheet(
            "font-size: 11px; color: #d94f4f; font-weight: bold; "
            "font-family: 'Courier New', monospace;"
        );
        QProgressBar* atkBar = new QProgressBar();
        atkBar->setRange(0, 30);
        atkBar->setValue(re.atkVal);
        atkBar->setTextVisible(false);
        atkBar->setFixedHeight(7);
        atkBar->setStyleSheet(R"(
            QProgressBar { border: 1px solid #2a2a4a; border-radius: 4px; background: #0a0a18; }
            QProgressBar::chunk {
                border-radius: 4px;
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #d94f4f, stop:1 #ff6b6b);
            }
        )");
        QLabel* atkNum = new QLabel(QString::number(re.atkVal));
        atkNum->setFixedWidth(40);
        atkNum->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        atkNum->setStyleSheet(
            "font-size: 11px; color: #d94f4f; font-family: 'Courier New', monospace;"
        );
        atkRow->addWidget(atkLabel);
        atkRow->addWidget(atkBar, 1);
        atkRow->addWidget(atkNum);

        QLabel* specL = new QLabel(QString("★  %1").arg(re.special));
        specL->setStyleSheet(QString(
            "font-size: 9px; color: %1; letter-spacing: 1px; font-style: italic;"
        ).arg(re.accent));

        col->addWidget(nameL);
        col->addWidget(tagL);
        col->addLayout(hpRow);
        col->addLayout(atkRow);
        col->addWidget(specL);

        row->addWidget(emblem);
        row->addLayout(col, 1);

        // Make the whole card clickable via an event filter — no overlay button.
        card->setAttribute(Qt::WA_Hover, true);
        card->setProperty("rosterType", re.type);
        card->installEventFilter(this);
        // Children must not swallow the click, otherwise the eventFilter
        // installed on the card itself never fires.
        for (QWidget* child : card->findChildren<QWidget*>())
            child->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        cardWidget[i] = card;
        leftLay->addWidget(card);
    }

    leftLay->addStretch(1);
///////////////////////////////////////////////////////
    {
        QFrame* chatFrame = new QFrame();
        chatFrame->setObjectName("chatFrame");
        chatFrame->setStyleSheet(R"(
            QFrame#chatFrame {
                background: rgba(10, 10, 28, 230);
                border: 2px solid #3a2a6a;
                border-radius: 14px;
            }
        )");
        chatFrame->setMaximumHeight(360);
 
        QVBoxLayout* chatLay = new QVBoxLayout(chatFrame);
        chatLay->setContentsMargins(14, 12, 14, 12);
        chatLay->setSpacing(8);
 
        // ── Header ───────────────────────────────────────────
        QHBoxLayout* chatHeaderRow = new QHBoxLayout();
        chatHeaderRow->setSpacing(8);
 
        // Paint a robot-face icon (no emoji font dependency)
        QPixmap iconPx(28, 28);
        iconPx.fill(Qt::transparent);
        {
            QPainter ip(&iconPx);
            ip.setRenderHint(QPainter::Antialiasing);
            ip.setPen(Qt::NoPen);
            // Head
            ip.setBrush(QColor("#7c5cbf"));
            ip.drawRoundedRect(4, 6, 20, 16, 4, 4);
            // Eyes
            ip.setBrush(QColor("#7df6ff"));
            ip.drawEllipse(8, 10, 5, 5);
            ip.drawEllipse(16, 10, 5, 5);
            // Mouth
            ip.setBrush(QColor("#c8a8ff"));
            ip.drawRect(10, 17, 8, 2);
            // Antenna
            ip.setPen(QPen(QColor("#c8a8ff"), 2));
            ip.drawLine(14, 6, 14, 1);
            ip.setBrush(QColor("#7df6ff"));
            ip.setPen(Qt::NoPen);
            ip.drawEllipse(12, 0, 5, 5);
        }
        QLabel* chatIcon = new QLabel();
        chatIcon->setPixmap(iconPx);
        chatIcon->setFixedSize(28, 28);
        chatIcon->setStyleSheet("background: transparent;");
 
        QLabel* chatTitle = new QLabel("AI ADVISOR");
        chatTitle->setStyleSheet(
            "font-size: 13px; color: #c8a8ff; letter-spacing: 5px; "
            "font-weight: 900; font-family: 'Courier New', monospace; "
            "background: transparent;"
        );
 
        QLabel* chatSubtitle = new QLabel("ask me which fighter to pick");
        chatSubtitle->setStyleSheet(
            "font-size: 10px; color: #555580; letter-spacing: 2px; "
            "font-family: 'Courier New', monospace; background: transparent;"
        );
 
        chatHeaderRow->addWidget(chatIcon);
        chatHeaderRow->addWidget(chatTitle);
        chatHeaderRow->addWidget(chatSubtitle, 1, Qt::AlignRight | Qt::AlignVCenter);
        chatLay->addLayout(chatHeaderRow);
 
        // Thin divider
        QFrame* chatSep = new QFrame();
        chatSep->setFrameShape(QFrame::HLine);
        chatSep->setStyleSheet("border: none; background: #2a1a50; max-height: 1px;");
        chatLay->addWidget(chatSep);
 
        // ── Chat display (scrollable) ─────────────────────────
        chatDisplay = new QTextEdit();
        chatDisplay->setReadOnly(true);
        chatDisplay->setMaximumHeight(180);
        chatDisplay->setMinimumHeight(180);
        chatDisplay->setStyleSheet(R"(
            QTextEdit {
                background: rgba(8, 8, 20, 200);
                color: #c8c0e0;
                border: 1px solid #2a1a50;
                border-radius: 8px;
                font-size: 11px;
                font-family: 'Segoe UI', Arial, sans-serif;
                padding: 6px 8px;
            }
            QScrollBar:vertical {
                background: #0a0a18;
                width: 6px;
                border-radius: 3px;
            }
            QScrollBar::handle:vertical {
                background: #3a2a6a;
                border-radius: 3px;
            }
        )");
        chatDisplay->setPlaceholderText("Chat history appears here...");
        chatLay->addWidget(chatDisplay);
 
        // (API key is loaded from api_key.txt — no UI field needed)
 
        // ── Input + Send row ──────────────────────────────────
        QHBoxLayout* inputRow = new QHBoxLayout();
        inputRow->setSpacing(8);
 
        chatInput = new QLineEdit();
        chatInput->setPlaceholderText("e.g. Which fighter is best for beginners?");
        chatInput->setStyleSheet(R"(
            QLineEdit {
                background: rgba(14, 12, 30, 210);
                color: #e0d8f8;
                border: 1px solid #3a2a6a;
                border-radius: 8px;
                font-size: 12px;
                font-family: 'Segoe UI', Arial, sans-serif;
                padding: 6px 10px;
            }
            QLineEdit:focus { border: 1px solid #7c5cbf; }
            QLineEdit:disabled { color: #444466; background: rgba(10,10,20,180); }
        )");
 
        chatSendBtn = new QPushButton("SEND");
        chatSendBtn->setFixedSize(70, 34);
        chatSendBtn->setCursor(Qt::PointingHandCursor);
        chatSendBtn->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #4a2a90, stop:1 #7c5cbf);
                color: #fff;
                font-size: 11px;
                font-weight: 900;
                letter-spacing: 2px;
                border: 1px solid #9b7ce0;
                border-radius: 8px;
                font-family: 'Courier New', monospace;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #5a38a8, stop:1 #9370d4);
                border: 1px solid #b090ff;
            }
            QPushButton:pressed { background: #3a2080; }
            QPushButton:disabled {
                background: #1e1e38; color: #444466; border: 1px solid #2a2a4a;
            }
        )");
 
        inputRow->addWidget(chatInput, 1);
        inputRow->addWidget(chatSendBtn);
        chatLay->addLayout(inputRow);
 
        leftLay->addWidget(chatFrame);
 
        // ── Network manager (single instance) ─────────────────
        chatNetworkManager = new QNetworkAccessManager(this);
        connect(chatNetworkManager, &QNetworkAccessManager::finished,
                this, &MainWindow::onChatReplyFinished);
 
        // ── Wire send button + Enter key ───────────────────────
        connect(chatSendBtn, &QPushButton::clicked,
                this, &MainWindow::onChatSendClicked);
        connect(chatInput, &QLineEdit::returnPressed,
                this, &MainWindow::onChatSendClicked);
 
        // ── Welcome message ────────────────────────────────────
        appendChatMessage("assistant",
            "Hello! I'm your Battle Arena AI advisor. "
            "Ask me anything — which fighter to pick, strategy tips, or how abilities work!");
    }
    /////////////////////////////////////////////////////////////////////////////////
    selectionLabel = new QLabel("No fighter selected");
    selectionLabel->setAlignment(Qt::AlignCenter);
    selectionLabel->setTextFormat(Qt::RichText);
    selectionLabel->setStyleSheet(
        "font-size: 13px; color: #7a7a99; letter-spacing: 2px; "
        "background: rgba(10,10,24,200); border: 1px solid #1e1e3a; "
        "border-radius: 6px; padding: 8px;"
    );
    leftLay->addWidget(selectionLabel);

    // BACK + START row
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setSpacing(14);

    QPushButton* btnBack = new QPushButton("◀  BACK");
    btnBack->setFixedHeight(48);
    btnBack->setMinimumWidth(140);
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setStyleSheet(R"(
        QPushButton {
            background: #1e1e3a; color: #b0a0d0;
            border: 2px solid #2a2a4a; border-radius: 10px;
            font-size: 14px; font-weight: bold; letter-spacing: 2px;
        }
        QPushButton:hover { background: #25253d; color: #ffffff; border: 2px solid #7c5cbf; }
    )");
    connect(btnBack, &QPushButton::clicked, this, [=]() {
        stack->setCurrentWidget(menuPage);
    });

    btnStart = new QPushButton("START  BATTLE  ▶");
    btnStart->setFixedHeight(48);
    btnStart->setMinimumWidth(220);
    btnStart->setEnabled(false);
    btnStart->setCursor(Qt::PointingHandCursor);
    btnStart->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #c0392b, stop:1 #e74c3c);
            color: #fff; font-size: 16px; font-weight: 900;
            letter-spacing: 3px; border: 2px solid #ff6b6b; border-radius: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #d04030, stop:1 #ff5050);
            border: 2px solid #ff9090;
        }
        QPushButton:disabled { background: #2a2a40; color: #44445a; border: 2px solid #2a2a40; }
    )");
    connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);

    btnRow->addWidget(btnBack);
    btnRow->addWidget(btnStart, 1);
    leftLay->addLayout(btnRow);

    // ════════════════════════════════════════════════════════
    //   RIGHT — Live animated preview stage
    // ════════════════════════════════════════════════════════
    QFrame* rightFrame = new QFrame();
    rightFrame->setObjectName("previewFrame");
    rightFrame->setStyleSheet(R"(
        QFrame#previewFrame {
            background: qradialgradient(cx:0.5, cy:0.55, radius:0.85,
                stop:0 rgba(40, 30, 90, 220),
                stop:0.6 rgba(15, 15, 38, 235),
                stop:1 rgba(8, 8, 20, 245));
            border: 2px solid #3a3a60;
            border-radius: 18px;
        }
    )");
    rightFrame->setMinimumWidth(620);

    QVBoxLayout* rightLay = new QVBoxLayout(rightFrame);
    rightLay->setContentsMargins(28, 24, 28, 24);
    rightLay->setSpacing(12);

    QLabel* rightHeader = new QLabel("◢  LIVE PREVIEW  ◣");
    rightHeader->setAlignment(Qt::AlignCenter);
    rightHeader->setStyleSheet(
        "font-size: 14px; color: #f0c060; letter-spacing: 6px; "
        "font-weight: bold; font-family: 'Courier New', monospace;"
    );
    rightLay->addWidget(rightHeader);

    // Sprite stage
    QFrame* stage = new QFrame();
    stage->setObjectName("previewStage");
    stage->setMinimumSize(420, 420);
    stage->setStyleSheet(R"(
        QFrame#previewStage {
            background: qradialgradient(cx:0.5, cy:0.92, radius:0.7,
                stop:0 rgba(124, 92, 191, 110),
                stop:0.55 rgba(20, 20, 50, 180),
                stop:1 rgba(0, 0, 0, 220));
            border: 1px solid #5a4a90;
            border-radius: 14px;
        }
    )");

    QVBoxLayout* stageLay = new QVBoxLayout(stage);
    stageLay->setContentsMargins(24, 24, 24, 18);
    stageLay->setSpacing(8);
    stageLay->setAlignment(Qt::AlignCenter);

    selPreviewSprite = new QLabel();
    selPreviewSprite->setFixedSize(360, 360);
    selPreviewSprite->setAlignment(Qt::AlignCenter);
    selPreviewSprite->setStyleSheet("background: transparent;");
    selPreviewSprite->setPixmap(makeArcadeSprite(0, 360, 0, 0, 0));

    selPreviewPoseTag = new QLabel("◆  IDLE  ◆");
    selPreviewPoseTag->setAlignment(Qt::AlignCenter);
    selPreviewPoseTag->setStyleSheet(
        "font-size: 11px; color: #ffd85a; letter-spacing: 6px; "
        "font-weight: bold; font-family: 'Courier New', monospace; "
        "background: rgba(0,0,0,160); border: 1px solid #5a4a90; "
        "border-radius: 4px; padding: 4px 14px;"
    );

    stageLay->addStretch();
    stageLay->addWidget(selPreviewSprite, 0, Qt::AlignCenter);
    stageLay->addStretch();
    stageLay->addWidget(selPreviewPoseTag, 0, Qt::AlignCenter);

    rightLay->addWidget(stage, 1);

    // Class info underneath
    selPreviewName = new QLabel(roster[0].name);
    selPreviewName->setAlignment(Qt::AlignCenter);
    selPreviewName->setStyleSheet(QString(
        "font-size: 30px; font-weight: 900; color: %1; "
        "letter-spacing: 8px; font-family: 'Impact', 'Arial Black', sans-serif;"
    ).arg(roster[0].accent));

    selPreviewSubtitle = new QLabel(roster[0].subtitle);
    selPreviewSubtitle->setAlignment(Qt::AlignCenter);
    selPreviewSubtitle->setStyleSheet(
        "font-size: 12px; color: #b0a0d0; letter-spacing: 4px; "
        "font-family: 'Courier New', monospace;"
    );

    selPreviewSpecial = new QLabel(QString("★  %1").arg(roster[0].special));
    selPreviewSpecial->setAlignment(Qt::AlignCenter);
    selPreviewSpecial->setStyleSheet(QString(
        "font-size: 13px; color: %1; letter-spacing: 1px; font-style: italic; "
        "background: rgba(0,0,0,140); border: 1px solid %1; border-radius: 6px; "
        "padding: 6px 18px;"
    ).arg(roster[0].accent));

    selPreviewDesc = new QLabel(roster[0].desc);
    selPreviewDesc->setAlignment(Qt::AlignCenter);
    selPreviewDesc->setWordWrap(true);
    selPreviewDesc->setStyleSheet("font-size: 11px; color: #7a7a99; line-height: 1.4;");

    rightLay->addWidget(selPreviewName);
    rightLay->addWidget(selPreviewSubtitle);
    rightLay->addWidget(selPreviewSpecial, 0, Qt::AlignCenter);
    rightLay->addWidget(selPreviewDesc);

    twoPane->addWidget(leftFrame, 0);
    twoPane->addWidget(rightFrame, 1);

    // Hint at the bottom
    QLabel* tip = new QLabel(
        "TIP  ·  Get adjacent to the enemy to attack  ·  Q triggers your special (3-turn cooldown)  ·  "
        "You will face a RANDOM opponent"
    );
    tip->setAlignment(Qt::AlignCenter);
    tip->setWordWrap(true);
    tip->setStyleSheet(
        "font-size: 11px; color: #555580; letter-spacing: 2px; "
        "font-family: 'Courier New', monospace; "
        "border: 1px solid #1e1e3a; border-radius: 6px; padding: 8px 14px;"
    );

    root->addWidget(header);
    root->addWidget(subtitle);
    root->addLayout(twoPane, 1);
    root->addWidget(tip);

    // ── Live preview animator: cycles Idle → Attack → Idle → Special ──
    selPreviewTimer = new QTimer(this);
    connect(selPreviewTimer, &QTimer::timeout, this, [this]() {
        if (stack->currentWidget() != characterPage) return;

        const QList<RosterEntry>& list = rosterEntries();
        if (selPreviewType < 0 || selPreviewType >= list.size()) selPreviewType = 0;

        selPreviewFrame++;
        if (--selPoseTicksLeft <= 0) {
            // Sequence: 0=idle(long) → 1=attack → 0=idle(short) → 2=special → repeat
            static const int seq[4] = {0, 1, 0, 2};
            static int step = 0;
            step = (step + 1) % 4;
            selPreviewPose = seq[step];
            selPreviewFrame = 0;

            const int frameCount = std::max(1, spriteFrameCountForPose(selPreviewType, selPreviewPose));
            if (selPreviewPose == 0)        selPoseTicksLeft = (step == 0) ? 24 : 12;
            else                            selPoseTicksLeft = frameCount + 2;

            const QStringList tags = {"◆  IDLE  ◆", "◆  ATTACK  ◆", "◆  SPECIAL  ◆"};
            const QStringList tagColors = {"#ffd85a", "#ff8060", "#62f7ff"};
            if (selPreviewPoseTag) {
                selPreviewPoseTag->setText(tags[selPreviewPose]);
                selPreviewPoseTag->setStyleSheet(QString(
                    "font-size: 11px; color: %1; letter-spacing: 6px; "
                    "font-weight: bold; font-family: 'Courier New', monospace; "
                    "background: rgba(0,0,0,160); border: 1px solid %1; "
                    "border-radius: 4px; padding: 4px 14px;"
                ).arg(tagColors[selPreviewPose]));
            }
        }

        if (selPreviewSprite)
            selPreviewSprite->setPixmap(makeArcadeSprite(selPreviewType, 360,
                                                         selPreviewPose, 0, selPreviewFrame));
    });
    selPreviewTimer->start(110);

    stack->addWidget(characterPage);
}



void MainWindow::onCharacterSelected(int type) {
    if (type < 0 || type > 2) return;
    selectedType = type;

    const QList<RosterEntry>& list = rosterEntries();

    // Highlight the active card; reset the others.
    for (int i = 0; i < 3; ++i) {
        if (!cardWidget[i]) continue;
        const RosterEntry& e = list[i];
        const bool active = (i == type);
        cardWidget[i]->setStyleSheet(QString(R"(
            QWidget#rosterCard%1 {
                background: %2;
                border: 2px solid %3;
                border-radius: 12px;
            }
            QWidget#rosterCard%1:hover {
                background: #1a1a38;
                border: 2px solid %4;
            }
        )").arg(i)
           .arg(active ? "#1f1f3e" : "#12122a")
           .arg(active ? e.accent  : QString("#2a2a4a"))
           .arg(e.accent));
    }

    const RosterEntry& re = list[type];

    if (selectionLabel) {
        selectionLabel->setText(
            "<span style='color:#7a7a99; letter-spacing:3px;'>SELECTED  ▸  </span>"
            "<b style='color:" + re.accent + "; letter-spacing:4px;'>" + re.name + "</b>"
        );
    }

    if (selPreviewName) {
        selPreviewName->setText(re.name);
        selPreviewName->setStyleSheet(QString(
            "font-size: 30px; font-weight: 900; color: %1; "
            "letter-spacing: 8px; font-family: 'Impact', 'Arial Black', sans-serif;"
        ).arg(re.accent));
    }
    if (selPreviewSubtitle) selPreviewSubtitle->setText(re.subtitle);
    if (selPreviewSpecial) {
        selPreviewSpecial->setText(QString("★  %1").arg(re.special));
        selPreviewSpecial->setStyleSheet(QString(
            "font-size: 13px; color: %1; letter-spacing: 1px; font-style: italic; "
            "background: rgba(0,0,0,140); border: 1px solid %1; border-radius: 6px; "
            "padding: 6px 18px;"
        ).arg(re.accent));
    }
    if (selPreviewDesc) selPreviewDesc->setText(re.desc);

    // Restart the live preview animation on the new class with a fresh idle.
    selPreviewType = type;
    selPreviewPose = 0;
    selPreviewFrame = 0;
    selPoseTicksLeft = 24;
    if (selPreviewSprite)
        selPreviewSprite->setPixmap(makeArcadeSprite(type, 360, 0, 0, 0));
    if (selPreviewPoseTag) {
        selPreviewPoseTag->setText("◆  IDLE  ◆");
        selPreviewPoseTag->setStyleSheet(
            "font-size: 11px; color: #ffd85a; letter-spacing: 6px; "
            "font-weight: bold; font-family: 'Courier New', monospace; "
            "background: rgba(0,0,0,160); border: 1px solid #ffd85a; "
            "border-radius: 4px; padding: 4px 14px;"
        );
    }

    if (btnStart) btnStart->setEnabled(true);
}

void MainWindow::onStartClicked()
{
    delete selectedCharacter;
    selectedCharacter = nullptr;

    if      (selectedType == 0) selectedCharacter = new Warrior("Player");
    else if (selectedType == 1) selectedCharacter = new Mage("Player");
    else if (selectedType == 2) selectedCharacter = new Archer("Player");
    else return;

    specialCooldown = 0;

    // Update difficulty page's character preview
    static const QStringList classNames = {"WARRIOR", "MAGE", "GORGON"};
    if (diffCharPreview)
        diffCharPreview->setPixmap(makeArcadeSprite(selectedType, 140, 0));
    if (diffCharName)
        diffCharName->setText(classNames[selectedType]);

    // Go to difficulty selection instead of starting immediately
    stack->setCurrentWidget(difficultyPage);
}

// ═══════════════════════════════════════════════════════════════
//  Page 1.5 — Difficulty Selection
// ═══════════════════════════════════════════════════════════════

void MainWindow::buildDifficultyPage() {
    ArcadeBgWidget* bg = new ArcadeBgWidget();
    difficultyPage = bg;

    QVBoxLayout* lay = new QVBoxLayout(difficultyPage);
    lay->setAlignment(Qt::AlignHCenter);
    lay->setSpacing(22);
    lay->setContentsMargins(80, 60, 80, 60);

    QLabel* title = new QLabel("SELECT  DIFFICULTY");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 56px; font-weight: 900;
        color: #ffffff; letter-spacing: 12px;
        font-family: "Impact", "Arial Black", sans-serif;
    )");
    QGraphicsDropShadowEffect* dGlow = new QGraphicsDropShadowEffect();
    dGlow->setBlurRadius(44); dGlow->setColor(QColor("#7c5cbf")); dGlow->setOffset(0,0);
    title->setGraphicsEffect(dGlow);

    QLabel* sub = new QLabel("◆  CHOOSE  YOUR  CHALLENGE  ◆");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet(
        "font-size: 14px; color: #7a7a99; letter-spacing: 8px; "
        "font-family: 'Courier New', monospace;"
    );

    // ── Character preview (updated by onStartClicked) ─────────
    QFrame* previewFrame = new QFrame();
    previewFrame->setObjectName("diffPreviewFrame");
    previewFrame->setStyleSheet(R"(
        QFrame#diffPreviewFrame {
            background: qradialgradient(cx:0.5, cy:0.55, radius:0.85,
                stop:0 rgba(40, 30, 90, 220),
                stop:0.7 rgba(15, 15, 38, 235),
                stop:1 rgba(8, 8, 20, 245));
            border: 2px solid #3a3a60; border-radius: 16px;
        }
    )");
    QVBoxLayout* previewCol = new QVBoxLayout(previewFrame);
    previewCol->setAlignment(Qt::AlignCenter);
    previewCol->setSpacing(8);
    previewCol->setContentsMargins(24, 18, 24, 18);

    QLabel* readyLabel = new QLabel("◢  YOUR  FIGHTER  ◣");
    readyLabel->setAlignment(Qt::AlignCenter);
    readyLabel->setStyleSheet(
        "font-size: 12px; color: #9b8edd; letter-spacing: 6px; "
        "font-family: 'Courier New', monospace; font-weight: bold;"
    );

    diffCharPreview = new QLabel();
    diffCharPreview->setFixedSize(140, 140);
    diffCharPreview->setPixmap(makeArcadeSprite(0, 140, 0));
    diffCharPreview->setAlignment(Qt::AlignCenter);
    diffCharPreview->setStyleSheet(
        "border: 2px solid #7c5cbf; border-radius: 10px; "
        "background: qradialgradient(cx:0.5, cy:0.95, radius:0.7, "
        "  stop:0 rgba(124,92,191,140), stop:1 rgba(0,0,0,180));"
    );
    diffCharName = new QLabel("WARRIOR");
    diffCharName->setAlignment(Qt::AlignCenter);
    diffCharName->setStyleSheet(
        "font-size: 22px; color: #ffffff; font-weight: 900; letter-spacing: 6px; "
        "font-family: 'Impact', 'Arial Black', sans-serif;"
    );

    previewCol->addWidget(readyLabel);
    previewCol->addWidget(diffCharPreview, 0, Qt::AlignCenter);
    previewCol->addWidget(diffCharName);

    // ── Difficulty cards ──────────────────────────────────────
    auto buildDiffCard = [&](const QString& label, const QString& badgeText, const QString& badgeColor,
                             const QString& gradStart, const QString& gradEnd,
                             const QString& borderCol, const QString& borderHover,
                             const QStringList& descLines,
                             QPushButton** btnOut) -> QWidget* {
        QFrame* card = new QFrame();
        card->setObjectName("diffCard");
        card->setMinimumWidth(360);
        card->setStyleSheet(R"(
            QFrame#diffCard {
                background: rgba(15, 15, 38, 220);
                border: 2px solid #2a2a4a;
                border-radius: 16px;
            }
        )");

        QVBoxLayout* col = new QVBoxLayout(card);
        col->setAlignment(Qt::AlignCenter);
        col->setSpacing(14);
        col->setContentsMargins(24, 24, 24, 24);

        QLabel* badge = new QLabel(badgeText);
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet(QString(
            "font-size: 11px; color: %1; letter-spacing: 4px; font-weight: bold; "
            "background: rgba(0,0,0,150); border: 1px solid %1; "
            "border-radius: 4px; padding: 4px 12px; font-family: 'Courier New', monospace;"
        ).arg(badgeColor));

        QPushButton* btn = new QPushButton(label);
        btn->setMinimumSize(300, 90);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(R"(
            QPushButton {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 %1, stop:1 %2);
                color: #fff; font-size: 30px; font-weight: 900;
                border: 2px solid %3; border-radius: 12px; letter-spacing: 8px;
                font-family: "Impact", "Arial Black", sans-serif;
            }
            QPushButton:hover { border: 2px solid %4; }
        )").arg(gradStart, gradEnd, borderCol, borderHover));

        QVBoxLayout* descBox = new QVBoxLayout();
        descBox->setSpacing(4);
        for (const QString& line : descLines) {
            QLabel* L = new QLabel(QString("•  %1").arg(line));
            L->setStyleSheet(QString(
                "font-size: 13px; color: %1; letter-spacing: 1px;"
            ).arg(badgeColor));
            descBox->addWidget(L);
        }

        col->addWidget(badge, 0, Qt::AlignCenter);
        col->addWidget(btn);
        col->addLayout(descBox);

        *btnOut = btn;
        return card;
    };

    QPushButton* btnEasy = nullptr;
    QPushButton* btnHard = nullptr;

    QWidget* easyCard = buildDiffCard(
        "EASY", "RECOMMENDED", "#3dba6e",
        "#1a7a3a", "#3dba6e", "#28d47e", "#50e890",
        {"Normal turn speed", "Basic enemy AI", "Perfect for learning"},
        &btnEasy
    );
    QWidget* hardCard = buildDiffCard(
        "HARD", "ONLY FOR THE BRAVE", "#d94f4f",
        "#8b0000", "#d94f4f", "#ff6b6b", "#ff9090",
        {"2× faster turn timer", "Enemy uses specials below 30% HP", "For experienced warriors"},
        &btnHard
    );

    QHBoxLayout* diffRow = new QHBoxLayout();
    diffRow->setSpacing(40);
    diffRow->setAlignment(Qt::AlignCenter);

    QLabel* vsDivider = new QLabel("VS");
    vsDivider->setAlignment(Qt::AlignCenter);
    vsDivider->setStyleSheet(
        "font-size: 64px; font-weight: 900; color: #5a5a80; "
        "font-family: 'Impact', 'Arial Black', sans-serif; letter-spacing: 2px;"
    );
    QGraphicsDropShadowEffect* vsGlow = new QGraphicsDropShadowEffect();
    vsGlow->setBlurRadius(28); vsGlow->setColor(QColor("#7c5cbf")); vsGlow->setOffset(0,0);
    vsDivider->setGraphicsEffect(vsGlow);

    diffRow->addWidget(easyCard, 1);
    diffRow->addWidget(vsDivider);
    diffRow->addWidget(hardCard, 1);

    // ── Comparison table ─────────────────────────────────────
    QLabel* comparison = new QLabel();
    comparison->setTextFormat(Qt::RichText);
    comparison->setAlignment(Qt::AlignCenter);
    comparison->setText(
        "<table style='font-size: 14px; color: #b0a0d0; border-spacing: 6px;'>"
        "<tr>"
        "<th style='color: #7a7a99; padding: 6px 28px; letter-spacing: 3px;'>FEATURE</th>"
        "<th style='color: #3dba6e; padding: 6px 28px; letter-spacing: 3px;'>EASY</th>"
        "<th style='color: #d94f4f; padding: 6px 28px; letter-spacing: 3px;'>HARD</th>"
        "</tr>"
        "<tr><td style='padding: 4px 28px;'>Enemy Speed</td>"
        "<td style='padding: 4px 28px;'>Normal</td>"
        "<td style='padding: 4px 28px;'>2× Faster</td></tr>"
        "<tr><td style='padding: 4px 28px;'>Enemy AI</td>"
        "<td style='padding: 4px 28px;'>Basic</td>"
        "<td style='padding: 4px 28px;'>Advanced</td></tr>"
        "<tr><td style='padding: 4px 28px;'>Special Attacks</td>"
        "<td style='padding: 4px 28px;'>None</td>"
        "<td style='padding: 4px 28px;'>At low HP</td></tr>"
        "</table>"
    );
    comparison->setStyleSheet(
        "background: rgba(10,10,24,200); border: 1px solid #2a2a4a; "
        "border-radius: 10px; padding: 12px 24px;"
    );

    QPushButton* btnBack = new QPushButton("◀  BACK");
    btnBack->setFixedHeight(48);
    btnBack->setMinimumWidth(160);
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setStyleSheet(R"(
        QPushButton {
            background: #1e1e3a; color: #b0a0d0;
            border: 2px solid #2a2a4a; border-radius: 10px;
            font-size: 14px; font-weight: bold; letter-spacing: 3px;
        }
        QPushButton:hover { background: #25253d; color: #ffffff; border: 2px solid #7c5cbf; }
    )");
QWidget*        enemyPanel  = nullptr; // for border flash
    QLabel*         lblStreak   = nullptr;
    int             winStreak   = 0;
    int             bestStreak  = 0;    connect(btnBack, &QPushButton::clicked, this, [=]() {
        stack->setCurrentWidget(characterPage);
    });

    connect(btnEasy, &QPushButton::clicked, this, [=]() {
        hardMode = false;
        gameManager->setHardMode(false);
        startBattle();
    });
    connect(btnHard, &QPushButton::clicked, this, [=]() {
        hardMode = true;
        gameManager->setHardMode(true);
        startBattle();
    });

    lay->addWidget(title);
    lay->addWidget(sub);
    lay->addSpacing(8);
    lay->addWidget(previewFrame, 0, Qt::AlignCenter);
    lay->addSpacing(10);
    lay->addLayout(diffRow);
    lay->addSpacing(10);
    lay->addWidget(comparison, 0, Qt::AlignCenter);
    lay->addStretch(1);
    lay->addWidget(btnBack, 0, Qt::AlignLeft);

    stack->addWidget(difficultyPage);
}



// Helper: actually launch the game after difficulty is chosen
void MainWindow::startBattle()
{
    if (gameOverAnimTimer) gameOverAnimTimer->stop();

    Character* enemy = nullptr;
    enemyType = rand() % 3;
    if      (enemyType == 0) enemy = new Warrior("Enemy");
    else if (enemyType == 1) enemy = new Mage("Enemy");
    else                     enemy = new Archer("Enemy");

    gameManager->startGame(selectedCharacter, enemy);

    // Reset combat state
    turnCount = 0;
    playerFacing = 0;
    enemyFacing = 1;
    playerWalkFrame = 0;
    enemyWalkFrame = 0;
    combatMessages.clear();
    if (lblCombatLog) lblCombatLog->setText("<span style='color:#555580;'>Battle begins!</span>");

    static const QStringList classNames = {"Warrior", "Mage", "Gorgon"};
    lblPlayerClass->setText(classNames[selectedType]);
    lblEnemyClass->setText(classNames[enemyType]);

    QPixmap pm = makeBattleSprite(selectedType, 120, 0, playerFacing, 0, false);
    if (playerPortraitLabel) playerPortraitLabel->setPixmap(pm);
    QPixmap enemyPm = makeBattleSprite(enemyType, 120, 0, enemyFacing, 0, true);
    if (enemyPortraitLabel) enemyPortraitLabel->setPixmap(enemyPm);

    drawGrid();
    runeItems.clear();
    drawRunes();

    QPixmap playerPm = makeBattleSprite(selectedType, CELL - 6, 0, playerFacing, playerWalkFrame, false);
    playerToken = scene->addPixmap(playerPm);
    playerToken->setZValue(2);

    QPixmap enemyPixmap = makeBattleSprite(enemyType, CELL - 6, 0, enemyFacing, enemyWalkFrame, true);
    enemyToken = scene->addPixmap(enemyPixmap);
    enemyToken->setZValue(2);

    updateTokenPositions();
    updateHUD();
    updateBottomBar();
#ifdef BATTLE_HAS_AUDIO
    BattleAudio::startMusic(true);
#endif
    stack->setCurrentWidget(gamePage);
    gamePage->setFocus();
}

// ═══════════════════════════════════════════════════════════════
//  Page 2 — Game
// ═══════════════════════════════════════════════════════════════

QWidget* MainWindow::buildHUDPanel(bool isPlayer) {
    QWidget* panel = new QWidget();
    panel->setMinimumWidth(240);
    panel->setMaximumWidth(300);
    panel->setStyleSheet("background: rgba(18,18,42,220); border-radius: 12px; border: 1px solid #2a2a4a;");

    QVBoxLayout* lay = new QVBoxLayout(panel);
    lay->setContentsMargins(18, 20, 18, 20);
    lay->setSpacing(10);

    QLabel* role = new QLabel(isPlayer ? "YOU" : "ENEMY");
    role->setAlignment(Qt::AlignCenter);
    role->setStyleSheet(QString(
        "background: %1; color: #fff; border-radius: 6px; "
        "font-size: 15px; font-weight: 900; padding: 6px; letter-spacing: 6px; "
        "font-family: 'Impact', 'Arial Black', sans-serif;"
    ).arg(isPlayer ? "#5a3ea0" : "#8b2020"));

    QLabel*& portraitL = isPlayer ? playerPortraitLabel : enemyPortraitLabel;
    portraitL = new QLabel();
    portraitL->setFixedSize(120, 120);
    portraitL->setAlignment(Qt::AlignCenter);
    portraitL->setStyleSheet(QString(
        "border: 2px solid %1; border-radius: 10px; "
        "background: qradialgradient(cx:0.5, cy:0.95, radius:0.7, "
        "  stop:0 rgba(60,40,120,140), stop:1 rgba(0,0,0,200));"
    ).arg(isPlayer ? "#7c5cbf" : "#d94f4f"));
    QPixmap pm = makeBattleSprite(0, 120, 0, isPlayer ? 0 : 1, 0, !isPlayer);
    portraitL->setPixmap(pm);

    QLabel*& nameL = isPlayer ? lblPlayerName : lblEnemyName;
    nameL = new QLabel("—");
    nameL->setAlignment(Qt::AlignCenter);
    nameL->setStyleSheet(
        "font-size: 22px; font-weight: 900; color: #ffffff; letter-spacing: 2px; "
        "font-family: 'Impact', 'Arial Black', sans-serif;"
    );

    QLabel*& classL = isPlayer ? lblPlayerClass : lblEnemyClass;
    classL = new QLabel("—");
    classL->setAlignment(Qt::AlignCenter);
    classL->setStyleSheet(QString(
        "font-size: 14px; color: %1; letter-spacing: 4px; font-weight: bold; "
        "font-family: 'Courier New', monospace;"
    ).arg(isPlayer ? "#9b8edd" : "#ff8b8b"));

    QFrame* sep = new QFrame(); sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("border: none; background: #2a2a4a; max-height: 1px;");

    QLabel* hpTitle = new QLabel("HP");
    hpTitle->setStyleSheet(
        "font-size: 12px; color: #7a7a99; letter-spacing: 4px; font-weight: bold; "
        "font-family: 'Courier New', monospace;"
    );

    QProgressBar*& bar = isPlayer ? barPlayerHP : barEnemyHP;
    bar = new QProgressBar();
    bar->setRange(0, 200);
    bar->setValue(200);
    bar->setTextVisible(false);
    bar->setFixedHeight(20);
    if (!isPlayer) {
        bar->setStyleSheet(R"(
            QProgressBar { border: 1px solid #2a2a4a; border-radius: 6px; background: #0a0a18; }
            QProgressBar::chunk {
                border-radius: 5px;
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #d94f4f, stop:1 #ff6b6b);
            }
        )");
    } else {
        bar->setStyleSheet(R"(
            QProgressBar { border: 1px solid #2a2a4a; border-radius: 6px; background: #0a0a18; }
            QProgressBar::chunk {
                border-radius: 5px;
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #3dba6e, stop:1 #28d47e);
            }
        )");
    }

    QLabel*& hpVal = isPlayer ? lblPlayerHPVal : lblEnemyHPVal;
    hpVal = new QLabel("200 / 200");
    hpVal->setAlignment(Qt::AlignCenter);
    hpVal->setStyleSheet(
        "font-size: 14px; color: #b0a0d0; font-weight: bold; letter-spacing: 2px; "
        "font-family: 'Courier New', monospace;"
    );

    lay->addWidget(role);
    lay->addSpacing(4);
    lay->addWidget(portraitL, 0, Qt::AlignCenter);
    lay->addWidget(nameL);
    lay->addWidget(classL);
    lay->addWidget(sep);
    lay->addWidget(hpTitle);
    lay->addWidget(bar);
    lay->addWidget(hpVal);

    // DANGER label (hidden by default)
    QLabel*& dangerL = isPlayer ? lblPlayerDanger : lblEnemyDanger;
    dangerL = new QLabel("DANGER");
    dangerL->setAlignment(Qt::AlignCenter);
    dangerL->setStyleSheet("font-size: 11px; font-weight: bold; color: #d94f4f; letter-spacing: 2px;");
    dangerL->setVisible(false);
    lay->addWidget(dangerL);

    lay->addStretch();

    // ── Combat log (inside enemy panel) ────────────────────
    if (!isPlayer) {
        QLabel* logTitle = new QLabel("COMBAT LOG");
        logTitle->setAlignment(Qt::AlignCenter);
        logTitle->setStyleSheet(
            "font-size: 11px; color: #7a7a99; letter-spacing: 4px; font-weight: bold; "
            "font-family: 'Courier New', monospace; background: transparent;"
        );
        lay->addWidget(logTitle);

        lblCombatLog = new QLabel("<span style='color:#7a7a99;'>◆  Battle begins!</span>");
        lblCombatLog->setTextFormat(Qt::RichText);
        lblCombatLog->setWordWrap(true);
        lblCombatLog->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        lblCombatLog->setMinimumHeight(70);
        lblCombatLog->setMaximumHeight(120);
        lblCombatLog->setStyleSheet(
            "font-size: 11px; color: #b0a0d0; line-height: 1.5; "
            "background: rgba(8,8,20,180); border: 1px solid #2a2a4a; "
            "border-radius: 6px; padding: 8px 10px;"
        );
        lay->addWidget(lblCombatLog);
    }

    if (isPlayer) {
        QLabel* hint = new QLabel();
        hint->setTextFormat(Qt::RichText);
        hint->setText(
            "<b style='letter-spacing:2px;'>CONTROLS</b><br><br>"
            "&#8593; &#8595; &#8592; &#8594;&nbsp;&nbsp; Move<br>"
            "Space&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Attack<br>"
            "Q&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Special"
        );
        hint->setStyleSheet(
            "font-size: 12px; color: #6a6a90; line-height: 1.6; "
            "border: 1px solid #1e1e3a; border-radius: 6px; padding: 10px 12px;"
        );
        lay->addWidget(hint);
    }

// save enemy panel so updateHUD() can flash its border
    if (!isPlayer) enemyPanel = panel;

    return panel;
}

void MainWindow::buildGamePage() {
    ArcadeBgWidget* bg = new ArcadeBgWidget();
    gamePage = bg;
    connect(gameManager, &GameManager::enemyTurnTriggered,
        this, &MainWindow::onEnemyTurn);
    QVBoxLayout* root = new QVBoxLayout(gamePage);
    root->setContentsMargins(16, 8, 16, 8);
    root->setSpacing(6);

    QLabel* arenaTitle = new QLabel("◆  BATTLE  ARENA  ◆");
    arenaTitle->setAlignment(Qt::AlignCenter);
    arenaTitle->setFixedHeight(50);
    arenaTitle->setStyleSheet(R"(
        font-size: 50px;
        font-weight: 900;
        color: #fff2a6;
        letter-spacing: 14px;
        font-family: "Impact", "Arial Black", "Courier New", sans-serif;
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 rgba(124, 92, 191, 90),
            stop:0.45 rgba(10, 10, 24, 235),
            stop:0.55 rgba(10, 10, 24, 235),
            stop:1 rgba(40, 168, 154, 90));
        border-top: 2px solid rgba(125, 246, 255, 180);
        border-bottom: 2px solid rgba(255, 216, 90, 195);
        border-radius: 8px;
        padding-bottom: 2px;
    )");
    QGraphicsDropShadowEffect* arenaGlow = new QGraphicsDropShadowEffect();
    arenaGlow->setBlurRadius(46);
    arenaGlow->setColor(QColor("#ffd85a"));
    arenaGlow->setOffset(0, 0);
    arenaTitle->setGraphicsEffect(arenaGlow);

    // ── Top bar ──────────────────────────────────────────────
    QHBoxLayout* topBar = new QHBoxLayout();
    lblStreak = new QLabel("STREAK  ·  0");
    lblStreak->setStyleSheet(
        "font-size: 18px; color: #a8f0a8; letter-spacing: 4px; font-weight: bold; "
        "font-family: 'Courier New', monospace; "
        "background: rgba(10,10,24,200); border: 1px solid #3dba6e; "
        "border-radius: 6px; padding: 6px 16px;"
    );
    topBar->addStretch();
    topBar->addWidget(lblStreak);

    QPushButton* pauseBtn = new QPushButton("SAVE  ·  PAUSE", this);
    pauseBtn->setFixedSize(220, 44);
    pauseBtn->setStyleSheet(
        "font-size: 13px; color: #ffd85a; letter-spacing: 4px; font-weight: bold; "
        "font-family: 'Courier New', monospace; "
        "background: rgba(10,10,24,200); border: 1px solid #d4a017; "
        "border-radius: 6px; padding: 6px 16px;"
        );
    topBar->addWidget(pauseBtn);
    connect(pauseBtn, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    btnResume = new QPushButton("RESUME", this);
    btnResume->setFixedSize(220, 44);
    btnResume->setVisible(false);
    btnResume->setStyleSheet(
        "font-size: 13px; color: #a8f0a8; letter-spacing: 4px; font-weight: bold; "
        "font-family: 'Courier New', monospace; "
        "background: rgba(10,10,24,200); border: 1px solid #3dba6e; "
        "border-radius: 6px; padding: 6px 16px;"
        );
    topBar->addWidget(btnResume);
    connect(btnResume, &QPushButton::clicked, this, [this]() {
        gameManager->resumeGame();
        btnResume->setVisible(false);
        lblTurnInfo->setText("Your turn — move or attack");
    });
    // ── Main row: HUD | Grid | HUD ───────────────────────────
    QHBoxLayout* midRow = new QHBoxLayout();
    midRow->setSpacing(20);

    QWidget* leftPanel  = buildHUDPanel(true);
    QWidget* rightPanel = buildHUDPanel(false);

    lblPlayerName->setText("Player");
    lblEnemyName->setText("Enemy");

    scene    = new QGraphicsScene(this);
    gridView = new QGraphicsView(scene);
    gridView->setFixedSize(GCOLS * CELL + 4, GROWS * CELL + 4);
    gridView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gridView->setRenderHint(QPainter::Antialiasing);
    gridView->setStyleSheet("background: #181830; border: 3px solid #3a3a60; border-radius: 12px;");

    drawGrid();

    midRow->addWidget(leftPanel,  0, Qt::AlignTop);
    midRow->addWidget(gridView,   0, Qt::AlignCenter);
    midRow->addWidget(rightPanel, 0, Qt::AlignTop);

    // ── Action/status strip ──────────────────────────────────
    QHBoxLayout* actionRow = new QHBoxLayout();
    actionRow->setSpacing(14);
    actionRow->setAlignment(Qt::AlignCenter);

    lblTurnInfo = new QLabel("◆  Your turn — move or attack");
    lblTurnInfo->setFixedHeight(40);
    lblTurnInfo->setMinimumWidth(360);
    lblTurnInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    lblTurnInfo->setStyleSheet(
        "font-size: 14px; color: #bda8ee; font-weight: bold; letter-spacing: 2px; "
        "background: rgba(90,62,160,70); border: 1px solid #7c5cbf; "
        "border-radius: 8px; padding: 0 14px;"
    );

    btnActionAttack = new QPushButton("⚔  ATTACK   [Space]");
    btnActionAttack->setFixedSize(220, 40);
    btnActionAttack->setCursor(Qt::PointingHandCursor);
    btnActionAttack->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #8b2020,stop:1 #d94f4f);
            color: #fff; font-size: 15px; font-weight: 900;
            border: 2px solid #ff6b6b; border-radius: 10px; letter-spacing: 3px;
            font-family: "Impact", "Arial Black", sans-serif;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #a02525,stop:1 #ff5555); border: 2px solid #ff9090; }
        QPushButton:pressed { background: #6b1818; }
    )");
    connect(btnActionAttack, &QPushButton::clicked, this, [this]() {
        QKeyEvent evt(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
        keyPressEvent(&evt);
    });

    btnActionSpecial = new QPushButton("★  SPECIAL   [Q]");
    btnActionSpecial->setFixedSize(220, 40);
    btnActionSpecial->setCursor(Qt::PointingHandCursor);
    btnActionSpecial->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #7a5a10,stop:1 #d4a017);
            color: #fff; font-size: 15px; font-weight: 900;
            border: 2px solid #ffe060; border-radius: 10px; letter-spacing: 3px;
            font-family: "Impact", "Arial Black", sans-serif;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #8b6a18,stop:1 #f5c020); border: 2px solid #fff080; }
        QPushButton:pressed { background: #5a4008; }
    )");
    connect(btnActionSpecial, &QPushButton::clicked, this, [this]() {
        QKeyEvent evt(QEvent::KeyPress, Qt::Key_Q, Qt::NoModifier);
        keyPressEvent(&evt);
    });

    actionRow->addWidget(lblTurnInfo, 1);
    actionRow->addWidget(btnActionAttack);
    actionRow->addWidget(btnActionSpecial);

    // (Combat log is now inside the enemy HUD panel — see buildHUDPanel)

    // ── Bottom status bar ────────────────────────────────────
    QHBoxLayout* botBar = new QHBoxLayout();
    botBar->setSpacing(20);

    const QString chipStyle =
        "font-size: 13px; color: #b0a0d0; letter-spacing: 3px; "
        "font-family: 'Courier New', monospace; font-weight: bold; "
        "background: rgba(10,10,24,200); border: 1px solid #2a2a4a; "
        "border-radius: 6px; padding: 6px 14px;";

    lblTurnCounter = new QLabel("TURN  ·  0");
    lblTurnCounter->setStyleSheet(chipStyle);

    lblDistance = new QLabel("DISTANCE  ·  —");
    lblDistance->setStyleSheet(chipStyle);

    lblDiffBadge = new QLabel("EASY");
    lblDiffBadge->setAlignment(Qt::AlignCenter);
    lblDiffBadge->setStyleSheet(
        "font-size: 13px; font-weight: 900; color: #3dba6e; "
        "background: rgba(61,186,110,40); border: 2px solid #3dba6e; "
        "border-radius: 6px; padding: 6px 14px; letter-spacing: 4px; "
        "font-family: 'Impact', 'Arial Black', sans-serif;"
    );

    QLabel* cooldownHint = new QLabel("COOLDOWN  ·  READY");
    cooldownHint->setObjectName("cooldownHint");
    cooldownHint->setStyleSheet(chipStyle);

    botBar->setSpacing(14);
    botBar->setContentsMargins(0, 4, 0, 0);
    botBar->addWidget(lblTurnCounter);
    botBar->addWidget(lblDistance);
    botBar->addStretch();
    botBar->addWidget(cooldownHint);
    botBar->addWidget(lblDiffBadge);

    // ── Assemble ─────────────────────────────────────────────
    root->setSpacing(10);
    root->setContentsMargins(20, 10, 20, 14);
    root->addWidget(arenaTitle);
    root->addLayout(topBar);
    root->addLayout(midRow);
    root->addLayout(actionRow);
    root->addLayout(botBar);

    stack->addWidget(gamePage);
    gamePage->setFocusPolicy(Qt::StrongFocus);
}



// ─── Helper: flash a character token with an attack pose then restore idle ──
void MainWindow::flashAttackPose(bool isPlayer, int pose) {
    Character* ch = isPlayer ? gameManager->getPlayer() : gameManager->getEnemy();
    Character* target = isPlayer ? gameManager->getEnemy() : gameManager->getPlayer();
    if (!ch) return;

    QGraphicsPixmapItem*& token = isPlayer ? playerToken : enemyToken;
    QLabel*& portrait = isPlayer ? playerPortraitLabel : enemyPortraitLabel;
    int type = isPlayer ? selectedType : enemyType;  // use actual enemy class
    int& facing = isPlayer ? playerFacing : enemyFacing;
    int& walkFrame = isPlayer ? playerWalkFrame : enemyWalkFrame;

    if (!token) return;
    if (target) {
        facing = facingFromDelta(target->getGridX() - ch->getGridX(),
                                 target->getGridY() - ch->getGridY(),
                                 facing);
    }

    const int frameCount = std::max(1, spriteFrameCountForPose(type, pose));
    const int frameStepMs = 120;
    const int totalMs = frameCount * frameStepMs;
    combatAnimLocks++;

    const bool isEnemy = !isPlayer;
    for (int i = 0; i < frameCount; ++i) {
        QTimer::singleShot(i * frameStepMs, this, [=]() {
            if (token) token->setPixmap(makeBattleSprite(type, CELL - 6, pose, facing, i, isEnemy));
            if (portrait) portrait->setPixmap(makeBattleSprite(type, 120, pose, facing, i, isEnemy));
        });
    }

    // Restore idle after the full attack/special strip has played.
    QTimer::singleShot(totalMs + 60, this, [=, this]() {
        if (token) token->setPixmap(makeBattleSprite(type, CELL - 6, 0, facing, walkFrame, isEnemy));
        if (portrait) portrait->setPixmap(makeBattleSprite(type, 120, 0, facing, battleAnimFrame, isEnemy));
        combatAnimLocks = std::max(0, combatAnimLocks - 1);
    });
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

    int dr = playerRow - enemyRow;
    int dc = playerCol - enemyCol;

    if (!grid->isAdjacent(enemyRow, enemyCol, playerRow, playerCol)) {
        int bestRow = enemyRow;
        int bestCol = enemyCol;
        int bestDist = std::abs(dr) + std::abs(dc);

        const int moves[4][2] = {
            {dr > 0 ? 1 : -1, 0},
            {0, dc > 0 ? 1 : -1},
            {0, dc > 0 ? -1 : 1},
            {dr > 0 ? -1 : 1, 0}
        };

        for (const auto& mv : moves) {
            int candRow = enemyRow + mv[0];
            int candCol = enemyCol + mv[1];
            Cell* cand = grid->getCell(candRow, candCol);
            if (!grid->isValidMove(candRow, candCol) || (cand && cand->isOccupied()))
                continue;

            int candDist = std::abs(playerRow - candRow) + std::abs(playerCol - candCol);
            if (candDist < bestDist || (bestRow == enemyRow && bestCol == enemyCol)) {
                bestDist = candDist;
                bestRow = candRow;
                bestCol = candCol;
            }
        }

        if (grid->moveCharacter(enemy, bestRow, bestCol)) {
            enemyFacing = facingFromDelta(bestRow - enemyRow, bestCol - enemyCol, enemyFacing);
            enemyWalkFrame++;
            if (enemyToken)
                enemyToken->setPixmap(makeBattleSprite(enemyType, CELL - 6, 4, enemyFacing, enemyWalkFrame, true));
            checkSpellCell(bestRow, bestCol, false);
        }
    }

    if (grid->isAdjacent(enemy->getGridX(), enemy->getGridY(), playerRow, playerCol)) {
        if (hardMode && enemy->getCurrentHealth() < 0.3 * enemy->getMaxHealth()) {
            int dmg = enemy->specialAbility();
            player->takeDamage(dmg);
            flashAttackPose(false, 2);
            addCombatMessage("<span style='color:#ff6b6b;'>Enemy used Special for " + QString::number(dmg) + " damage!</span>");
        } else {
            int dmg = enemy->attack();
            player->takeDamage(dmg);
            flashAttackPose(false, 1);
            addCombatMessage("<span style='color:#d94f4f;'>Enemy attacked for " + QString::number(dmg) + " damage</span>");
        }
    }

    updateTokenPositions();
    updateHUD();
    updateBottomBar();
    gameManager->checkWinCondition();
}

void MainWindow::drawGrid()
{
    scene->clear();
    playerToken = nullptr;
    enemyToken  = nullptr;
    BattleGrid* logicGrid = gameManager ? gameManager->getGrid() : nullptr;

    // Seed a simple deterministic pattern for terrain decoration
    srand(42);
    for (int r = 0; r < GROWS; r++) {
        for (int c = 0; c < GCOLS; c++) {
            bool dark = (r + c) % 2 == 0;
            QColor fill(dark ? Pal::GRID_DARK : Pal::GRID_LITE);
            QColor border(Pal::CELL_BORD);
            bool blocked = logicGrid && logicGrid->isBlocked(r, c);
            if (blocked) {
                fill = QColor("#101526");
                border = QColor("#7df6ff");
            }

            QGraphicsRectItem* cell = scene->addRect(
                c * CELL, r * CELL, CELL, CELL,
                QPen(border, blocked ? 1.4 : 0.8),
                QBrush(fill)
            );
            cell->setZValue(0);

            if (blocked) {
                auto* glow = scene->addRect(
                    c * CELL + 3, r * CELL + 3, CELL - 6, CELL - 6,
                    QPen(QColor(125, 246, 255, 55), 1),
                    QBrush(QColor(20, 68, 111, 55))
                );
                glow->setZValue(0.6);
                drawObstacle(scene, c, r, CELL);
                continue;
            }

            // Terrain decorations — small pixel dots on some cells
            int rnd = rand() % 6;
            if (rnd == 0) {
                // Small grass tuft
                auto* dot = scene->addRect(c*CELL+20, r*CELL+38, 3, 3, Qt::NoPen, QBrush(QColor(40,80,40,80)));
                dot->setZValue(1);
                auto* dot2 = scene->addRect(c*CELL+30, r*CELL+40, 2, 2, Qt::NoPen, QBrush(QColor(50,90,50,60)));
                dot2->setZValue(1);
            } else if (rnd == 1) {
                // Small stone
                auto* stone = scene->addRect(c*CELL+24, r*CELL+36, 4, 3, Qt::NoPen, QBrush(QColor(60,60,80,70)));
                stone->setZValue(1);
            } else if (rnd == 2) {
                // Rune mark
                auto* rune = scene->addRect(c*CELL+26, r*CELL+26, 4, 4, Qt::NoPen, QBrush(QColor(80,60,140,40)));
                rune->setZValue(1);
            }
        }
    }
    srand(time(nullptr));  // restore random seed
}


void MainWindow::updateTokenPositions()
{
    if (!gameManager || !gameManager->getPlayer() || !gameManager->getEnemy()) return;

    Character* p = gameManager->getPlayer();
    Character* e = gameManager->getEnemy();

    if (playerToken) {
        QPointF target(p->getGridY() * CELL + 4, p->getGridX() * CELL + 4);
        moveTokenSmoothly(playerToken, target, turnCount > 0);
        playerToken->setZValue(2.0 + p->getGridX() * 0.05);
    }

    if (enemyToken) {
        QPointF target(e->getGridY() * CELL + 4, e->getGridX() * CELL + 4);
        moveTokenSmoothly(enemyToken, target, turnCount > 0);
        enemyToken->setZValue(2.0 + e->getGridX() * 0.05);
    }
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

    barEnemyHP->setMaximum(e->getMaxHealth());
    barEnemyHP->setValue(eHP);
    lblEnemyHPVal->setText(QString("%1 / %2").arg(eHP).arg(e->getMaxHealth()));

    // HP bar color transitions — green at full → amber → red at low.
    auto hpBarStyle = [](double pct) -> QString {
        QString color1, color2;
        if (pct > 0.6) { color1 = "#3dba6e"; color2 = "#28d47e"; }
        else if (pct > 0.3) { color1 = "#d4a017"; color2 = "#e8b830"; }
        else { color1 = "#d94f4f"; color2 = "#ff6b6b"; }
        return QString(
            "QProgressBar { border: 1px solid #2a2a4a; border-radius: 6px; background: #0a0a18; }"
            "QProgressBar::chunk { border-radius: 5px; "
            "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 %1, stop:1 %2); }"
        ).arg(color1, color2);
    };

    // Enemy bar uses sharper red when critically low
    auto enemyBarStyle = [](double pct) -> QString {
        QString color1, color2;
        if (pct > 0.3)       { color1 = "#d4a017"; color2 = "#e8b830"; } // amber
        else if (pct > 0.15) { color1 = "#d94f4f"; color2 = "#ff6b6b"; } // red
        else                 { color1 = "#cc0000"; color2 = "#ff2020"; } // critical red
        return QString(
            "QProgressBar { border: 1px solid #2a2a4a; border-radius: 6px; background: #0a0a18; }"
            "QProgressBar::chunk { border-radius: 5px; "
            "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 %1, stop:1 %2); }"
        ).arg(color1, color2);
    };

    barPlayerHP->setStyleSheet(hpBarStyle((double)pHP / pMax));
    barEnemyHP->setStyleSheet(enemyBarStyle((double)eHP / e->getMaxHealth()));

    // Flash enemy panel border red when critically low, reset when recovered
    if (enemyPanel) {
        if (eHP < 0.15 * e->getMaxHealth())
            enemyPanel->setStyleSheet("background: rgba(18,18,42,220); border-radius: 12px; border: 2px solid #ff2020;");
        else
            enemyPanel->setStyleSheet("background: rgba(18,18,42,220); border-radius: 12px; border: 1px solid #2a2a4a;");
    }

    // DANGER labels
    if (lblPlayerDanger) lblPlayerDanger->setVisible(pHP < 0.25 * pMax);
    if (lblEnemyDanger)  lblEnemyDanger->setVisible(eHP < 0.25 * e->getMaxHealth());
}

void MainWindow::onGameStateChanged(GameState state) {
    if (state == GameState::GAME_OVER) {
        bool playerWon = gameManager->getPlayer() && gameManager->getPlayer()->isAlive();
        showGameOver(playerWon);
    }
}

void MainWindow::showGameOver(bool playerWon) {
    // Update the game over overlay page
    if (lblGOTitle) {
        if (playerWon) {
            lblGOTitle->setText("VICTORY!");
            lblGOTitle->setStyleSheet(
                "font-size: 110px; font-weight: 900; color: #3dba6e; "
                "letter-spacing: 16px; font-family: 'Impact', 'Arial Black', sans-serif;"
            );
            QGraphicsDropShadowEffect* g = qobject_cast<QGraphicsDropShadowEffect*>(lblGOTitle->graphicsEffect());
            if (g) g->setColor(QColor("#3dba6e"));
        } else {
            lblGOTitle->setText("DEFEATED");
            lblGOTitle->setStyleSheet(
                "font-size: 110px; font-weight: 900; color: #d94f4f; "
                "letter-spacing: 16px; font-family: 'Impact', 'Arial Black', sans-serif;"
            );
            QGraphicsDropShadowEffect* g = qobject_cast<QGraphicsDropShadowEffect*>(lblGOTitle->graphicsEffect());
            if (g) g->setColor(QColor("#d94f4f"));
        }
    }
    if (lblGOMessage) {
        lblGOMessage->setText(playerWon
            ? "You vanquished your foe in glorious combat."
            : "You were slain in battle. The arena claims another...");
    }
    // updating win streak
    if (playerWon) {
        winStreak++;
        bestStreak = std::max(bestStreak, winStreak);
    } else {
        winStreak = 0;
    }
    if (lblStreak)
        lblStreak->setText("STREAK  ·  " + QString::number(winStreak));

    if (lblGOSprite) {
        // On defeat, show the player's dead animation (not the enemy).
        gameOverAnimType = selectedType;
        gameOverAnimPose = playerWon ? 2 : 3;
        gameOverAnimFrame = 0;
        lblGOSprite->setPixmap(makeArcadeSprite(gameOverAnimType, 240, gameOverAnimPose, 0, gameOverAnimFrame));
        if (gameOverAnimTimer) gameOverAnimTimer->start(260);
    }

    stack->setCurrentWidget(gameOverPage);

    // Update turn summary
    QLabel* ts = gameOverPage->findChild<QLabel*>("goTurnSummary");
    if (ts) ts->setText("Turns played:  " + QString::number(turnCount));

    // Play the matching outcome sound exactly once. The helpers stop the
    // other player so victory/defeat audio never overlap.
    if (playerWon) playVictorySound();
    else           playGameOverSound();
}

void MainWindow::addCombatMessage(const QString& msg) {
    combatMessages.prepend(msg);
    while (combatMessages.size() > 4)
        combatMessages.removeLast();
    if (lblCombatLog)
        lblCombatLog->setText(combatMessages.join("<br>"));
}

void MainWindow::updateBottomBar() {
    if (lblTurnCounter)
        lblTurnCounter->setText("TURN  ·  " + QString::number(turnCount));

    if (lblDistance && gameManager && gameManager->getPlayer() && gameManager->getEnemy()) {
        Character* p = gameManager->getPlayer();
        Character* e = gameManager->getEnemy();
        int dist = std::abs(p->getGridX() - e->getGridX()) + std::abs(p->getGridY() - e->getGridY());
        lblDistance->setText("DISTANCE  ·  " + QString::number(dist));
    }

    if (lblDiffBadge) {
        if (hardMode) {
            lblDiffBadge->setText("HARD");
            lblDiffBadge->setStyleSheet(
                "font-size: 13px; font-weight: 900; color: #d94f4f; "
                "background: rgba(217,79,79,40); border: 2px solid #d94f4f; "
                "border-radius: 6px; padding: 6px 14px; letter-spacing: 4px; "
                "font-family: 'Impact', 'Arial Black', sans-serif;"
            );
        } else {
            lblDiffBadge->setText("EASY");
            lblDiffBadge->setStyleSheet(
                "font-size: 13px; font-weight: 900; color: #3dba6e; "
                "background: rgba(61,186,110,40); border: 2px solid #3dba6e; "
                "border-radius: 6px; padding: 6px 14px; letter-spacing: 4px; "
                "font-family: 'Impact', 'Arial Black', sans-serif;"
            );
        }
    }

    // Update cooldown hint
    QLabel* cdHint = gamePage->findChild<QLabel*>("cooldownHint");
    if (cdHint) {
        if (specialCooldown > 0)
            cdHint->setText("COOLDOWN  ·  " + QString::number(specialCooldown) + "  TURNS");
        else
            cdHint->setText("COOLDOWN  ·  READY");
    }
}

void MainWindow::buildGameOverPage() {
    ArcadeBgWidget* bg = new ArcadeBgWidget();
    gameOverPage = bg;

    QVBoxLayout* lay = new QVBoxLayout(gameOverPage);
    lay->setAlignment(Qt::AlignCenter);
    lay->setSpacing(22);
    lay->setContentsMargins(80, 60, 80, 60);

    lblGOTitle = new QLabel("VICTORY!");
    lblGOTitle->setAlignment(Qt::AlignCenter);
    lblGOTitle->setStyleSheet(
        "font-size: 110px; font-weight: 900; color: #3dba6e; "
        "letter-spacing: 16px; font-family: 'Impact', 'Arial Black', sans-serif;"
    );
    QGraphicsDropShadowEffect* goGlow = new QGraphicsDropShadowEffect();
    goGlow->setBlurRadius(72); goGlow->setColor(QColor("#3dba6e")); goGlow->setOffset(0,0);
    lblGOTitle->setGraphicsEffect(goGlow);

    // Sprite stage with stylized backdrop
    QFrame* spriteStage = new QFrame();
    spriteStage->setObjectName("goStage");
    spriteStage->setFixedSize(280, 280);
    spriteStage->setStyleSheet(R"(
        QFrame#goStage {
            background: qradialgradient(cx:0.5, cy:0.92, radius:0.7,
                stop:0 rgba(124, 92, 191, 130),
                stop:0.55 rgba(20, 20, 50, 200),
                stop:1 rgba(0, 0, 0, 240));
            border: 2px solid #5a4a90;
            border-radius: 16px;
        }
    )");
    QVBoxLayout* stageLay = new QVBoxLayout(spriteStage);
    stageLay->setAlignment(Qt::AlignCenter);
    stageLay->setContentsMargins(12, 12, 12, 12);

    lblGOSprite = new QLabel();
    lblGOSprite->setFixedSize(240, 240);
    lblGOSprite->setAlignment(Qt::AlignCenter);
    lblGOSprite->setPixmap(makeArcadeSprite(0, 240, 2));
    lblGOSprite->setStyleSheet("background: transparent;");
    stageLay->addWidget(lblGOSprite);

    lblGOMessage = new QLabel("You vanquished your foe!");
    lblGOMessage->setAlignment(Qt::AlignCenter);
    lblGOMessage->setStyleSheet(
        "font-size: 18px; color: #b0a0d0; letter-spacing: 3px; font-style: italic;"
    );

    QLabel* divider = new QLabel("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    divider->setAlignment(Qt::AlignCenter);
    divider->setStyleSheet("font-size: 12px; color: #2a2a4a; letter-spacing: 1px;");

    QLabel* turnSummary = new QLabel("Turns played: —");
    turnSummary->setObjectName("goTurnSummary");
    turnSummary->setAlignment(Qt::AlignCenter);
    turnSummary->setStyleSheet(
        "font-size: 13px; color: #6a6a90; letter-spacing: 4px; "
        "font-family: 'Courier New', monospace;"
    );

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setSpacing(28);
    btnRow->setAlignment(Qt::AlignCenter);

    QPushButton* btnRestart = new QPushButton("▶  PLAY AGAIN");
    btnRestart->setFixedSize(300, 72);
    btnRestart->setCursor(Qt::PointingHandCursor);
    btnRestart->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #5a3ea0,stop:1 #7c5cbf);
            color: #fff; font-size: 22px; font-weight: 900; letter-spacing: 6px;
            border: 3px solid #9370d4; border-radius: 14px;
            font-family: "Impact", "Arial Black", sans-serif;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #6b4db5,stop:1 #9370d4); border: 3px solid #b090e0; }
        QPushButton:pressed { background: #4a3090; }
    )");
    QGraphicsDropShadowEffect* restartGlow = new QGraphicsDropShadowEffect();
    restartGlow->setBlurRadius(32); restartGlow->setColor(QColor("#7c5cbf")); restartGlow->setOffset(0,0);
    btnRestart->setGraphicsEffect(restartGlow);
    connect(btnRestart, &QPushButton::clicked, this, [this]() {
        if (gameOverAnimTimer) gameOverAnimTimer->stop();
        gameManager->restartGame();
#ifdef BATTLE_HAS_AUDIO
        BattleAudio::startMusic(false);
#endif
        stack->setCurrentWidget(characterPage);

        const QList<RosterEntry>& list = rosterEntries();
        for (int i = 0; i < 3; ++i) {
            if (!cardWidget[i]) continue;
            const RosterEntry& e = list[i];
            cardWidget[i]->setStyleSheet(QString(R"(
                QWidget#rosterCard%1 {
                    background: #12122a;
                    border: 2px solid #2a2a4a;
                    border-radius: 12px;
                }
                QWidget#rosterCard%1:hover {
                    background: #1a1a38;
                    border: 2px solid %2;
                }
            )").arg(i).arg(e.accent));
        }
        if (selectionLabel) selectionLabel->setText("No fighter selected");
        if (btnStart) btnStart->setEnabled(false);
        selectedType = -1;
        delete selectedCharacter;
        selectedCharacter = nullptr;
    });

    QPushButton* btnMenu = new QPushButton("◀  MAIN MENU");
    btnMenu->setFixedSize(300, 72);
    btnMenu->setCursor(Qt::PointingHandCursor);
    btnMenu->setStyleSheet(R"(
        QPushButton {
            background: #1e1e3a; color: #b0a0d0; font-size: 22px; font-weight: 900;
            letter-spacing: 6px; border: 3px solid #2a2a4a; border-radius: 14px;
            font-family: "Impact", "Arial Black", sans-serif;
        }
        QPushButton:hover { background: #25253d; color: #ffffff; border: 3px solid #7c5cbf; }
    )");
    connect(btnMenu, &QPushButton::clicked, this, [this]() {
        if (gameOverAnimTimer) gameOverAnimTimer->stop();
        gameManager->restartGame();
#ifdef BATTLE_HAS_AUDIO
        BattleAudio::startMusic(false);
#endif
        stack->setCurrentWidget(menuPage);
    });

    btnRow->addWidget(btnRestart);
    btnRow->addWidget(btnMenu);

    lay->addStretch(1);
    lay->addWidget(lblGOTitle, 0, Qt::AlignCenter);
    lay->addWidget(spriteStage, 0, Qt::AlignCenter);
    lay->addWidget(lblGOMessage, 0, Qt::AlignCenter);
    lay->addWidget(divider, 0, Qt::AlignCenter);
    lay->addWidget(turnSummary, 0, Qt::AlignCenter);
    lay->addSpacing(20);
    lay->addLayout(btnRow);
    lay->addStretch(1);

    stack->addWidget(gameOverPage);
}

// ═══════════════════════════════════════════════════════════════
//  eventFilter — roster card clicks on the character select page
// ═══════════════════════════════════════════════════════════════

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);

        // Roster card click (handled before generic button click sound).
        for (int i = 0; i < 3; ++i) {
            if (watched == cardWidget[i] && cardWidget[i]) {
                playClickSound();
                onCharacterSelected(i);
                return true;
            }
        }

        // Click sound on any enabled QAbstractButton release within bounds.
        if (me->button() == Qt::LeftButton) {
            QAbstractButton* btn = qobject_cast<QAbstractButton*>(watched);
            if (btn && btn->isEnabled() && btn->rect().contains(me->pos())) {
                playClickSound();
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

// ═══════════════════════════════════════════════════════════════
//  keyPressEvent
// ═══════════════════════════════════════════════════════════════

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // Prevent hold-to-repeat from issuing many moves/attacks in one turn.
    if (event && event->isAutoRepeat()) return;

    if (!gameManager || gameManager->getState() != GameState::PLAYING) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    Character* player = gameManager->getPlayer();
    if (!player) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    int row = player->getGridX();
    int col = player->getGridY();

    int newRow = row;
    int newCol = col;

    switch (event->key()) {
        // ── Movement ────────────────────────────────────
        case Qt::Key_Up:    case Qt::Key_W:  newRow--; break;
        case Qt::Key_Down:  case Qt::Key_S:  newRow++; break;
        case Qt::Key_Left:  case Qt::Key_A:  newCol--; break;
        case Qt::Key_Right: case Qt::Key_D:  newCol++; break;

        // ── Attack (Space) ───────────────────────────────
        case Qt::Key_Space: {
            Character* enemy = gameManager->getEnemy();
            if (!enemy) break;
            BattleGrid* grid = gameManager->getGrid();
            if (grid->isAdjacent(row, col, enemy->getGridX(), enemy->getGridY())) {
		int dmg = player->attack();
                enemy->takeDamage(dmg);
                if (specialCooldown > 0) specialCooldown--;
                turnCount++;
                flashAttackPose(true, 1);
                lblTurnInfo->setText("Attack! Hit for " +
                    QString::number(dmg) + " damage.");
                addCombatMessage("<span style='color:#7c5cbf;'>You attacked for " + QString::number(dmg) + " damage</span>");
                updateHUD();
                updateBottomBar();
                gameManager->checkWinCondition();
            } else {
                lblTurnInfo->setText("Too far to attack — move closer!");
            }
            return;
        }

        // ── Special (Q) ──────────────────────────────────
        case Qt::Key_Q: {
            if (specialCooldown > 0) {
                lblTurnInfo->setText("Special not ready (" +
                    QString::number(specialCooldown) + " turns)");
                return;
            }
            Character* enemy = gameManager->getEnemy();
            if (!enemy) break;
            BattleGrid* grid = gameManager->getGrid();
            if (grid->isAdjacent(row, col, enemy->getGridX(), enemy->getGridY())) {
                int dmg = player->specialAbility();
                enemy->takeDamage(dmg);
                specialCooldown = 3;
                turnCount++;
                flashAttackPose(true, 2);
                lblTurnInfo->setText("Special! Hit for " +
                    QString::number(dmg) + " damage!");
                addCombatMessage("<span style='color:#d4a017;'>You used Special for " + QString::number(dmg) + " damage!</span>");
                updateHUD();
                updateBottomBar();
                gameManager->checkWinCondition();
            } else {
                lblTurnInfo->setText("Too far for special — move closer!");
            }
            return;
        }

        default:
            QMainWindow::keyPressEvent(event);
            return;
    }

    bool moved = gameManager->getGrid()->moveCharacter(player, newRow, newCol);
    if (moved) {
        playerFacing = facingFromDelta(newRow - row, newCol - col, playerFacing);
        playerWalkFrame++;
        if (playerToken)
            playerToken->setPixmap(makeBattleSprite(selectedType, CELL - 6, 4, playerFacing, playerWalkFrame, false));
        if (specialCooldown > 0) specialCooldown--;
        turnCount++;
        checkSpellCell(newRow, newCol, true);
        updateTokenPositions();
        updateHUD();
        updateBottomBar();
        lblTurnInfo->setText("Your turn — move or attack");
    }
}
void MainWindow::onPauseClicked() {
    if (gameManager) {
        gameManager->pauseGame();
    }

    QFile file(QCoreApplication::applicationDirPath() + "/savegame.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);


        out << selectedType << "\n";
        Character* player = gameManager->getPlayer();
        Character* enemy  = gameManager->getEnemy();
        out << (player ? player->getCurrentHealth() : 0) << "\n";
        out << (player ? player->getGridX() : 0) << "\n";
        out << (player ? player->getGridY() : 0) << "\n";
        out << enemyType << "\n";
        out << (enemy ? enemy->getCurrentHealth() : 0) << "\n";
        out << (enemy ? enemy->getGridX() : 0) << "\n";
        out << (enemy ? enemy->getGridY() : 0) << "\n";
        out << score << "\n";
        out << turnCount << "\n";

        file.close();

        // Check if lblTurnInfo exists before trying to use it
        if (lblTurnInfo) {
            lblTurnInfo->setText("GAME SAVED!");
        }
    }
    if (btnResume) btnResume->setVisible(true);
}

void MainWindow::drawRunes()
{
    BattleGrid* grid = gameManager ? gameManager->getGrid() : nullptr;
    if (!grid || !scene) return;

    for (int r = 0; r < BattleGrid::GRID_SIZE; r++) {
        for (int c = 0; c < BattleGrid::GRID_SIZE; c++) {
            SpellType t = grid->getSpell(r, c);
            if (t == SpellType::NONE) continue;

            bool isHeal = (t == SpellType::HEAL);
            QColor base  = isHeal ? QColor(61, 186, 110) : QColor(40, 168, 255);
            QColor glow  = isHeal ? QColor(61, 186, 110, 60) : QColor(40, 168, 255, 60);

            qreal cx = c * CELL + CELL / 2.0;
            qreal cy = r * CELL + CELL / 2.0;
            qreal rs = CELL * 0.32;

            // Outer glow disc
            auto* disc = scene->addEllipse(cx - rs, cy - rs, rs*2, rs*2,
                Qt::NoPen, QBrush(glow));
            disc->setZValue(1.5);

            // Ring
            auto* ring = scene->addEllipse(cx - rs + 4, cy - rs + 4,
                (rs-4)*2, (rs-4)*2,
                QPen(base, 2.5), Qt::NoBrush);
            ring->setZValue(1.6);

            // Inner symbol — + for heal, * for slow
            QString sym = isHeal ? "✚" : "❄";
            auto* txt = scene->addText(sym);
            txt->setDefaultTextColor(base);
            QFont f; f.setPixelSize(20); f.setBold(true);
            txt->setFont(f);
            txt->setPos(cx - txt->boundingRect().width()/2,
                        cy - txt->boundingRect().height()/2);
            txt->setZValue(1.7);

            // Group items so we can remove them together via the disc pointer
            // We store the disc as the key item; we'll also store ring and txt
            // by tagging them with the same cell key
            auto key = QPair<int,int>(r, c);

            // We store a simple container: use a QGraphicsItemGroup
            QGraphicsItemGroup* grp = scene->createItemGroup({disc, ring, txt});
            grp->setZValue(1.5);
            runeItems[key] = grp;
        }
    }
}

void MainWindow::checkSpellCell(int row, int col, bool isPlayer)
{
    BattleGrid* grid = gameManager ? gameManager->getGrid() : nullptr;
    if (!grid) return;

    SpellType t = grid->consumeSpell(row, col);
    if (t == SpellType::NONE) return;

    // Remove rune graphic
    auto key = QPair<int,int>(row, col);
    if (runeItems.contains(key)) {
        QGraphicsItem* item = runeItems.take(key);
        scene->removeItem(item);
        delete item;
    }

    bool isHeal = (t == SpellType::HEAL);
    playSpellAnimation(isHeal, row, col);

    if (isHeal) {
        // Heal whoever stepped on it
        Character* target = isPlayer ? gameManager->getPlayer() : gameManager->getEnemy();
        if (target) {
            int amt = target->getMaxHealth() / 4;
            target->heal(amt);
            if (isPlayer) {
                addCombatMessage("<span style='color:#3dba6e;'>✚ Heal rune! +" + QString::number(amt) + " HP</span>");
                lblTurnInfo->setText("Heal rune! +" + QString::number(amt) + " HP");
            } else {
                addCombatMessage("<span style='color:#ff9966;'>Enemy stepped on a Heal rune! +" + QString::number(amt) + " HP</span>");
            }
        }
    } else {
        // Slow whoever stepped on it (Option C — symmetric)
        if (isPlayer) {
            // Player stepped on slow rune — enemy attacks faster for 10 ticks
            gameManager->applyPlayerSlow();
            addCombatMessage("<span style='color:#ff6b6b;'>❄ Slow rune! You move slower for 10 turns</span>");
            lblTurnInfo->setText("Slow rune! You are slowed for 10 turns.");
            if (lblSlowIndicator) lblSlowIndicator->setVisible(true);
        } else {
            // Enemy stepped on slow rune — enemy gets slowed
            gameManager->applySlowness();
            addCombatMessage("<span style='color:#28a8ff;'>❄ Enemy hit a Slow rune! Enemy slowed for 10 turns</span>");
            lblTurnInfo->setText("Enemy hit a Slow rune! They're slowed.");
            if (lblSlowIndicator) lblSlowIndicator->setVisible(true);
        }
    }

    updateHUD();
    updateBottomBar();
}

void MainWindow::playSpellAnimation(bool isHeal, int row, int col) {
    if (!scene) return;

    QColor color = isHeal ? QColor(61, 186, 110, 180) : QColor(40, 168, 255, 180);
    qreal cx = col * CELL + CELL / 2.0;
    qreal cy = row * CELL + CELL / 2.0;

    for (int i = 0; i < 3; ++i) {
        QGraphicsEllipseItem* ring = scene->addEllipse(
            cx - 10, cy - 10, 20, 20,
            QPen(color, 3),
            Qt::NoBrush
        );
        ring->setZValue(10);

        int delay = i * 130;
        QTimer::singleShot(delay, this, [=]() {
            if (!ring->scene()) return;
            QVariantAnimation* anim = new QVariantAnimation(ring->scene());
            anim->setDuration(500);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            QObject::connect(anim, &QVariantAnimation::valueChanged, [=](const QVariant& v) {
                qreal t = v.toReal();
                qreal r = 10 + t * (CELL * 0.6);
                ring->setRect(cx - r, cy - r, r * 2, r * 2);
                QColor c = color;
                c.setAlpha(static_cast<int>(180 * (1.0 - t)));
                ring->setPen(QPen(c, 3.0 - t * 2.0));
            });
            QObject::connect(anim, &QVariantAnimation::finished, [=]() {
                if (ring->scene()) ring->scene()->removeItem(ring);
                delete ring;
            });
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }

    QGraphicsEllipseItem* flash = scene->addEllipse(
        cx - 10, cy - 10, 20, 20,
        Qt::NoPen,
        QBrush(isHeal ? QColor(61, 186, 110, 210) : QColor(40, 168, 255, 210))
    );
    flash->setZValue(11);
    QTimer::singleShot(280, this, [=]() {
        if (flash->scene()) flash->scene()->removeItem(flash);
        delete flash;
    });
}

void MainWindow::onLoadClicked() {
    QFile file(QCoreApplication::applicationDirPath() + "/savegame.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    selectedType = in.readLine().toInt();
    int pHP = in.readLine().toInt(), pR = in.readLine().toInt(), pC = in.readLine().toInt();
    enemyType = in.readLine().toInt();
    int eHP = in.readLine().toInt(), eR = in.readLine().toInt(), eC = in.readLine().toInt();
    score = in.readLine().toInt();
    turnCount = in.readLine().toInt();
    file.close();

    delete selectedCharacter;
    selectedCharacter = nullptr;
    if      (selectedType == 0) selectedCharacter = new Warrior("Player");
    else if (selectedType == 1) selectedCharacter = new Mage("Player");
    else if (selectedType == 2) selectedCharacter = new Archer("Player");
    else return;

    Character* enemy = nullptr;
    if      (enemyType == 0) enemy = new Warrior("Enemy");
    else if (enemyType == 1) enemy = new Mage("Enemy");
    else if (enemyType == 2) enemy = new Archer("Enemy");
    else return;

    specialCooldown = 0;
    gameManager->startGame(selectedCharacter, enemy);
    BattleGrid* grid = gameManager->getGrid();
    if (!grid) return;

    if ((pR != 0 || pC != 0) &&
        !grid->moveCharacter(gameManager->getPlayer(), pR, pC)) {
        return;
    }
    if ((eR != 7 || eC != 7) &&
        !grid->moveCharacter(gameManager->getEnemy(), eR, eC)) {
        return;
    }

    gameManager->getPlayer()->takeDamage(gameManager->getPlayer()->getMaxHealth() - pHP);
    gameManager->getEnemy()->takeDamage(gameManager->getEnemy()->getMaxHealth() - eHP);
    drawGrid();
    runeItems.clear();
    drawRunes();

    if (playerToken) {
        scene->removeItem(playerToken);
        delete playerToken;
        playerToken = nullptr;
    }
    if (enemyToken) {
        scene->removeItem(enemyToken);
        delete enemyToken;
        enemyToken = nullptr;
    }

    playerFacing = 0;
    enemyFacing = 1;
    playerWalkFrame = 0;
    enemyWalkFrame = 0;

    QPixmap playerPixmap = makeBattleSprite(selectedType, CELL - 6, 0, playerFacing, playerWalkFrame, false);
    playerToken = scene->addPixmap(playerPixmap);
    playerToken->setZValue(2);

    QPixmap enemyPixmap = makeBattleSprite(enemyType, CELL - 6, 0, enemyFacing, enemyWalkFrame, true);
    enemyToken = scene->addPixmap(enemyPixmap);
    enemyToken->setZValue(2);

    gridView->viewport()->update();
    gridView->update();
    scene->update();

    updateTokenPositions();
    updateHUD();
    updateBottomBar();
    if (btnResume) btnResume->setVisible(false);
    if (lblTurnInfo) lblTurnInfo->setText("Your turn — move or attack");
    stack->setCurrentWidget(gamePage);
    gamePage->setFocus();
}
void MainWindow::appendChatMessage(const QString& role, const QString& text) {
    if (!chatDisplay) return;
 
    // Color scheme: user = purple, assistant = teal/white
    QString color  = (role == "user") ? "#c8a8ff" : "#7df7d0";
    QString prefix = (role == "user") ? "YOU" : "AI";
    QString bgColor = (role == "user") ? "rgba(80,40,160,60)" : "rgba(20,80,80,60)";
 
    chatDisplay->append(
        QString("<div style='margin:3px 0; padding:4px 6px; "
                "background:%3; border-radius:5px;'>"
                "<span style='color:%1; font-weight:bold; "
                "font-size:10px; letter-spacing:2px;'>%2 ▸ </span>"
                "<span style='color:#d8d0f0;'>%4</span></div>")
        .arg(color, prefix, bgColor, text.toHtmlEscaped())
    );
 
    // Auto-scroll to bottom
    QScrollBar* sb = chatDisplay->verticalScrollBar();
    if (sb) sb->setValue(sb->maximum());
}
 
// ── Send button / Enter key handler ──────────────────────────
void MainWindow::onChatSendClicked() {
    if (!chatInput || !chatSendBtn) return;
 
    QString userText = chatInput->text().trimmed();
    if (userText.isEmpty()) return;
 
    QString apiKey = loadedApiKey;
    if (apiKey.isEmpty()) {
        appendChatMessage("assistant",
            "⚠ API key not found. Please add your key to api_key.txt in the project directory.");
        return;
    }
 
    // Display user message immediately
    appendChatMessage("user", userText);
    chatHistory.append({"user", userText});
    chatInput->clear();
    chatInput->setEnabled(false);
    chatSendBtn->setEnabled(false);
    chatSendBtn->setText("...");
 
    // Build messages array from history
    QString systemPrompt =
    "You are an AI advisor embedded inside Battle Arena, a 2D grid-based "
    "combat game built with C++ and Qt6. Three playable fighters exist:\n"
    "  WARRIOR: 200 HP, 20 ATK, special = Power Strike (1.5x ATK). Tough frontliner.\n"
    "  MAGE:    100 HP, 20 ATK, special = Arcane Storm (3x ATK). Fragile but devastating.\n"
    "  GORGON:  150 HP, 15 ATK, special = Stone Burst (2x ATK). Aggressive mid-range.\n"
    "The 8x8 grid has obstacles and spell runes (heal/slow). "
    "Easy = normal speed, Hard = 2x faster enemy turns. "
    "Specials have a 3-turn cooldown. Be concise, max 3 sentences.";

QJsonArray messages;

QJsonObject sysMsg;
sysMsg["role"]    = "system";
sysMsg["content"] = systemPrompt;
messages.append(sysMsg);

for (const auto& turn : chatHistory) {
    QJsonObject msg;
    msg["role"]    = turn.first;
    msg["content"] = turn.second;
    messages.append(msg);
}
 
    
 
    // Assemble request body
    QJsonObject body;
    body["model"]      = "anthropic/claude-haiku-4-5";
    body["max_tokens"] = 300;
    body["messages"]   = messages;
 
    QJsonDocument doc(body);
 
    // Build HTTP request
    QNetworkRequest req(QUrl("https://openrouter.ai/api/v1/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
req.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
req.setRawHeader("HTTP-Referer",  "https://battlearena.game");
req.setRawHeader("X-Title",       "Battle Arena");
 
    chatNetworkManager->post(req, doc.toJson(QJsonDocument::Compact));
}
 
// ── Handle API response ───────────────────────────────────────
void MainWindow::onChatReplyFinished(QNetworkReply* reply) {
    if (!chatInput || !chatSendBtn) { reply->deleteLater(); return; }
 
    // Re-enable input regardless of outcome
    chatInput->setEnabled(true);
    chatSendBtn->setEnabled(true);
    chatSendBtn->setText("SEND");
    chatInput->setFocus();
 
    if (reply->error() != QNetworkReply::NoError) {
        // Show a friendly error
        QString errMsg = reply->errorString();
        // Try to parse error body from Anthropic
        QByteArray raw = reply->readAll();
        if (!raw.isEmpty()) {
            QJsonDocument errDoc = QJsonDocument::fromJson(raw);
            if (!errDoc.isNull() && errDoc.object().contains("error")) {
                QJsonObject errObj = errDoc.object()["error"].toObject();
                errMsg = errObj["message"].toString();
            }
        }
        appendChatMessage("assistant",
            "⚠ Error: " + (errMsg.isEmpty() ? "Network request failed." : errMsg));
        reply->deleteLater();
        return;
    }
 
    QByteArray raw = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(raw);
 
    if (doc.isNull() || !doc.isObject()) {
        appendChatMessage("assistant", "⚠ Could not parse the server response.");
        reply->deleteLater();
        return;
    }
 
    QJsonObject root = doc.object();
 
    // Anthropic v1/messages returns: {"content": [{"type":"text","text":"..."}], ...}
    QJsonArray choices = doc.object()["choices"].toArray();
if (choices.isEmpty()) {
    appendChatMessage("assistant", "Received an empty response.");
    reply->deleteLater();
    return;
}
QString assistantText =
    choices[0].toObject()["message"].toObject()["content"].toString().trimmed();
 
    if (assistantText.isEmpty()) {
        appendChatMessage("assistant", "⚠ Received an empty response.");
    } else {
        // Add to history and display
        chatHistory.append({"assistant", assistantText});
        appendChatMessage("assistant", assistantText);
    }
 
    reply->deleteLater();
}
