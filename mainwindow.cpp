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
#include <QPolygon>
#include <ctime>

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

    auto* shadow = scene->addEllipse(x + 12, y + 39, 32, 8, Qt::NoPen, QBrush(QColor(0, 0, 0, 120)));
    shadow->setZValue(1.3);

    QPolygon crystal;
    crystal << QPoint(x + 28, y + 7)
            << QPoint(x + 42, y + 25)
            << QPoint(x + 36, y + 43)
            << QPoint(x + 21, y + 45)
            << QPoint(x + 14, y + 26);

    auto* body = scene->addPolygon(crystal, QPen(QColor("#7df6ff"), 1.2), QBrush(QColor("#21446f")));
    body->setZValue(1.5);

    QPolygon shine;
    shine << QPoint(x + 28, y + 8)
          << QPoint(x + 35, y + 25)
          << QPoint(x + 28, y + 42)
          << QPoint(x + 22, y + 25);
    auto* inner = scene->addPolygon(shine, Qt::NoPen, QBrush(QColor(125, 246, 255, 95)));
    inner->setZValue(1.6);

    auto* core = scene->addRect(x + 26, y + 22, 5, 12, Qt::NoPen, QBrush(QColor(255, 240, 120, 170)));
    core->setZValue(1.7);

    auto* spark1 = scene->addRect(x + 15, y + 13, 4, 4, Qt::NoPen, QBrush(QColor(255, 240, 120, 150)));
    spark1->setZValue(1.7);
    auto* spark2 = scene->addRect(x + 40, y + 14, 3, 3, Qt::NoPen, QBrush(QColor(125, 246, 255, 150)));
    spark2->setZValue(1.7);
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

static QPixmap makeArcadeSprite(int type, int size, int pose = 0, int facing = 0, int frame = 0) {
    return makeNeonChampionSprite(type, size, pose, facing, frame);

    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    // Pixel art = NO antialiasing
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
    return pixmap;
}

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


// ═══════════════════════════════════════════════════════════════
//  Constructor / Destructor
// ═══════════════════════════════════════════════════════════════

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      selectedCharacter(nullptr),
      selectedType(-1)
{
    setWindowTitle("⚔  Battle Arena");
    resize(1100, 750);
    setMinimumSize(1000, 680);

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

    // ── Menu animation: cycle character poses ──────────────
    menuAnimTimer = new QTimer(this);
    connect(menuAnimTimer, &QTimer::timeout, this, [this]() {
        menuPoseFrame = (menuPoseFrame + 1) % 3;
        for (int i = 0; i < 3; i++) {
            if (menuSprites[i])
                menuSprites[i]->setPixmap(makeArcadeSprite(i, 100, menuPoseFrame));
        }
    });
    menuAnimTimer->start(1200);

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
    lay->setSpacing(12);
    lay->setContentsMargins(40, 20, 40, 16);

    // ── top decorative border ────────────────────────────────
    QLabel* topBorder = new QLabel("╔══════════════════════════════════════════════╗");
    topBorder->setAlignment(Qt::AlignCenter);
    topBorder->setStyleSheet("font-size: 11px; color: #3a3a60; font-family: 'Courier New', monospace;");

    // ── coin row ─────────────────────────────────────────────
    QHBoxLayout* coinRow = new QHBoxLayout();
    coinRow->setSpacing(16);
    coinRow->setAlignment(Qt::AlignCenter);
    for (int i = 0; i < 7; i++) {
        QLabel* coin = new QLabel("◆");
        coin->setAlignment(Qt::AlignCenter);
        coin->setStyleSheet(QString("font-size: 14px; color: %1;")
            .arg(i % 2 == 0 ? "#d4a017" : "#7c5cbf"));
        coinRow->addWidget(coin);
    }

    // ── title ────────────────────────────────────────────────
    QLabel* title = new QLabel("⚔  BATTLE ARENA  ⚔");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 54px;
        font-weight: 900;
        letter-spacing: 8px;
        color: #ffffff;
        font-family: "Impact", "Arial Black", sans-serif;
    )");
    QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect();
    glow->setBlurRadius(40);
    glow->setColor(QColor("#7c5cbf"));
    glow->setOffset(0, 0);
    title->setGraphicsEffect(glow);

    // ── subtitle ─────────────────────────────────────────────
    QLabel* sub = new QLabel("CSCE 1101  ·  Spring 2026  ·  Team Project");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 12px; color: #7a7a99; letter-spacing: 3px; font-family: 'Courier New', monospace;");

    // ── HIGH SCORE display ───────────────────────────────────
    QLabel* highScore = new QLabel("HIGH SCORE: 000");
    highScore->setAlignment(Qt::AlignCenter);
    highScore->setStyleSheet(R"(
        font-size: 16px; font-weight: bold;
        color: #d4a017; letter-spacing: 4px;
        font-family: "Courier New", monospace;
    )");

    // ── character preview row — ANIMATED sprites ─────────────
    QHBoxLayout* charRow = new QHBoxLayout();
    charRow->setSpacing(40);
    charRow->setAlignment(Qt::AlignCenter);
    QStringList charNames = {"WARRIOR", "MAGE", "ARCHER"};
    QStringList charColors = {Pal::AMBER, Pal::TEAL, Pal::ORANGE};
    QStringList charWeapons = {"⚔ Sword", "✦ Staff", "➹ Bow"};
    for (int i = 0; i < 3; i++) {
        QVBoxLayout* col = new QVBoxLayout();
        col->setAlignment(Qt::AlignCenter);
        col->setSpacing(4);

        QLabel* portrait = new QLabel();
        portrait->setFixedSize(100, 100);
        portrait->setPixmap(makeArcadeSprite(i, 100, 0));
        portrait->setAlignment(Qt::AlignCenter);
        portrait->setStyleSheet(QString(
            "border: 2px solid %1; border-radius: 6px; background: rgba(0,0,0,100);"
        ).arg(charColors[i]));
        menuSprites[i] = portrait;  // store for animation

        QLabel* name = new QLabel(charNames[i]);
        name->setAlignment(Qt::AlignCenter);
        name->setStyleSheet(QString("font-size: 12px; color: %1; letter-spacing: 2px; font-weight: bold; font-family: 'Courier New', monospace;").arg(charColors[i]));

        QLabel* weapon = new QLabel(charWeapons[i]);
        weapon->setAlignment(Qt::AlignCenter);
        weapon->setStyleSheet(QString("font-size: 9px; color: %1;").arg(Pal::MUTED));

        col->addWidget(portrait);
        col->addWidget(name);
        col->addWidget(weapon);
        charRow->addLayout(col);
    }

    // ── decorative divider ───────────────────────────────────
    QLabel* divider = new QLabel("━━━━━━━━━━━━━━━━  ◆  ━━━━━━━━━━━━━━━━");
    divider->setAlignment(Qt::AlignCenter);
    divider->setStyleSheet("font-size: 11px; color: #2a2a4a;");

    // ── PLAY button ──────────────────────────────────────────
    QPushButton* btnPlay = new QPushButton("  ▶   PLAY");
    btnPlay->setFixedSize(320, 64);
    btnPlay->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #5a3ea0, stop:1 #7c5cbf);
            color: #fff;
            font-size: 22px;
            font-weight: bold;
            letter-spacing: 4px;
            border-radius: 10px;
            border: 2px solid #9370d4;
            font-family: "Impact", "Arial Black", sans-serif;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #6b4db5, stop:1 #9370d4);
            border: 2px solid #b090e0;
        }
        QPushButton:pressed { background: #4a3090; }
    )");
    connect(btnPlay, &QPushButton::clicked, this, [=]() {
        stack->setCurrentWidget(characterPage);
    });

    // ── Controls hint panel ──────────────────────────────────
    QLabel* controls = new QLabel("↑↓←→  MOVE   ·   SPACE  ATTACK   ·   Q  SPECIAL");
    controls->setAlignment(Qt::AlignCenter);
    controls->setStyleSheet(R"(
        font-size: 11px; color: #555580; letter-spacing: 2px;
        border: 1px solid #1e1e3a; border-radius: 6px;
        padding: 8px 20px;
        font-family: "Courier New", monospace;
    )");

    // ── credits ──────────────────────────────────────────────
    QLabel* credits = new QLabel("Youssef Gohar · Mohamed · Ahmed · Youssef Al-Ghobary");
    credits->setAlignment(Qt::AlignCenter);
    credits->setStyleSheet("font-size: 11px; color: #444466; letter-spacing: 1px;");

    // ── bottom border + version ──────────────────────────────
    QLabel* botBorder = new QLabel("╚══════════════════════════════════════════════╝");
    botBorder->setAlignment(Qt::AlignCenter);
    botBorder->setStyleSheet("font-size: 11px; color: #3a3a60; font-family: 'Courier New', monospace;");

    QLabel* version = new QLabel("v1.0  ·  Qt6  ·  C++17");
    version->setAlignment(Qt::AlignCenter);
    version->setStyleSheet("font-size: 9px; color: #2a2a48; letter-spacing: 2px;");

    // ── assemble ─────────────────────────────────────────────
    lay->addWidget(topBorder, 0, Qt::AlignCenter);
    lay->addSpacing(4);
    lay->addLayout(coinRow);
    lay->addSpacing(6);
    lay->addWidget(title,      0, Qt::AlignCenter);
    lay->addWidget(sub,        0, Qt::AlignCenter);
    lay->addWidget(highScore,  0, Qt::AlignCenter);
    lay->addSpacing(10);
    lay->addLayout(charRow);
    lay->addSpacing(4);
    lay->addWidget(divider,    0, Qt::AlignCenter);
    lay->addSpacing(6);
    lay->addWidget(btnPlay,    0, Qt::AlignCenter);
    lay->addSpacing(8);
    lay->addWidget(controls,   0, Qt::AlignCenter);
    lay->addSpacing(10);
    lay->addWidget(credits,    0, Qt::AlignCenter);
    lay->addWidget(botBorder,  0, Qt::AlignCenter);
    lay->addWidget(version,    0, Qt::AlignCenter);

    stack->addWidget(menuPage);
}

// ═══════════════════════════════════════════════════════════════
//  Page 1 — Character Select
// ═══════════════════════════════════════════════════════════════

void MainWindow::buildCharacterPage() {
    ArcadeBgWidget* bg = new ArcadeBgWidget();
    characterPage = bg;

    QVBoxLayout* root = new QVBoxLayout(characterPage);
    root->setContentsMargins(30, 20, 30, 16);
    root->setSpacing(12);

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
    selectionLabel->setTextFormat(Qt::RichText);
    selectionLabel->setStyleSheet("font-size: 13px; color: #7a7a99;");

    QHBoxLayout* cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(20);

    struct CardInfo {
        int type;
        QString name, hp, atk, special, desc, accent;
        int hpVal, atkVal;
    };
    QList<CardInfo> cards = {
        {0, "Warrior", "200", "20", "Power Strike (1.5× ATK)",
         "A heavy-hitting frontliner built to absorb punishment.",
         Pal::AMBER, 200, 20},
        {1, "Mage",    "100", "20", "Arcane Storm (3× ATK)",
         "Fragile but devastating — one burst can turn the tide.",
         Pal::TEAL, 100, 20},
        {2, "Archer",  "150", "15", "Double Shot (2× ATK)",
         "Balanced ranger who strikes reliably from any range.",
         Pal::ORANGE, 150, 15},
    };

    QPushButton** cardPtrs[3] = {&cardWarrior, &cardMage, &cardArcher};

    for (int i = 0; i < 3; i++) {
        const CardInfo& ci = cards[i];
        QWidget* card = new QWidget();
        card->setMinimumSize(250, 380);
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
        cLay->setContentsMargins(14, 16, 14, 16);
        cLay->setSpacing(8);
        cLay->setAlignment(Qt::AlignCenter);

        // Sprite preview — show all 3 poses in a row
        QHBoxLayout* poseRow = new QHBoxLayout();
        poseRow->setSpacing(4);
        poseRow->setAlignment(Qt::AlignCenter);
        QStringList poseTips = {"Idle", "Attack", "Special"};
        QStringList poseColors = {"#7a7a99", "#d94f4f", "#d4a017"};
        for (int pose = 0; pose < 3; pose++) {
            QVBoxLayout* pCol = new QVBoxLayout();
            pCol->setAlignment(Qt::AlignCenter);
            pCol->setSpacing(2);

            QLabel* spr = new QLabel();
            spr->setFixedSize(60, 60);
            spr->setPixmap(makeArcadeSprite(ci.type, 60, pose));
            spr->setAlignment(Qt::AlignCenter);
            spr->setStyleSheet(QString(
                "border: 1px solid %1; border-radius: 3px; background: rgba(0,0,0,100);"
            ).arg(poseColors[pose]));

            QLabel* poseLabel = new QLabel(poseTips[pose]);
            poseLabel->setAlignment(Qt::AlignCenter);
            poseLabel->setStyleSheet(QString(
                "font-size: 8px; color: %1; letter-spacing: 1px; font-family: 'Courier New', monospace;"
            ).arg(poseColors[pose]));

            pCol->addWidget(spr);
            pCol->addWidget(poseLabel);
            poseRow->addLayout(pCol);
        }

        // Name
        QLabel* nameL = new QLabel(ci.name.toUpper());
        nameL->setAlignment(Qt::AlignCenter);
        nameL->setStyleSheet(QString("font-size: 18px; font-weight: bold; color: %1; letter-spacing: 2px; font-family: 'Impact', sans-serif;").arg(ci.accent));

        // HP stat bar
        QHBoxLayout* hpRow = new QHBoxLayout();
        QLabel* hpLabel = new QLabel("HP");
        hpLabel->setStyleSheet("font-size: 10px; color: #3dba6e; font-weight: bold; font-family: 'Courier New', monospace;");
        hpLabel->setFixedWidth(28);
        QProgressBar* hpBar = new QProgressBar();
        hpBar->setRange(0, 200);
        hpBar->setValue(ci.hpVal);
        hpBar->setTextVisible(false);
        hpBar->setFixedHeight(10);
        hpBar->setStyleSheet(R"(
            QProgressBar { border: 1px solid #2a2a4a; border-radius: 3px; background: #0a0a18; }
            QProgressBar::chunk { border-radius: 3px; background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #3dba6e, stop:1 #28d47e); }
        )");
        QLabel* hpNum = new QLabel(ci.hp);
        hpNum->setStyleSheet("font-size: 10px; color: #3dba6e; font-family: 'Courier New', monospace;");
        hpNum->setFixedWidth(28);
        hpNum->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        hpRow->addWidget(hpLabel);
        hpRow->addWidget(hpBar);
        hpRow->addWidget(hpNum);

        // ATK stat bar
        QHBoxLayout* atkRow = new QHBoxLayout();
        QLabel* atkLabel = new QLabel("ATK");
        atkLabel->setStyleSheet("font-size: 10px; color: #d94f4f; font-weight: bold; font-family: 'Courier New', monospace;");
        atkLabel->setFixedWidth(28);
        QProgressBar* atkBar = new QProgressBar();
        atkBar->setRange(0, 30);
        atkBar->setValue(ci.atkVal);
        atkBar->setTextVisible(false);
        atkBar->setFixedHeight(10);
        atkBar->setStyleSheet(R"(
            QProgressBar { border: 1px solid #2a2a4a; border-radius: 3px; background: #0a0a18; }
            QProgressBar::chunk { border-radius: 3px; background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #d94f4f, stop:1 #ff6b6b); }
        )");
        QLabel* atkNum = new QLabel(ci.atk);
        atkNum->setStyleSheet("font-size: 10px; color: #d94f4f; font-family: 'Courier New', monospace;");
        atkNum->setFixedWidth(28);
        atkNum->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        atkRow->addWidget(atkLabel);
        atkRow->addWidget(atkBar);
        atkRow->addWidget(atkNum);

        // Special ability
        QLabel* specL = new QLabel();
        specL->setTextFormat(Qt::RichText);
        specL->setAlignment(Qt::AlignCenter);
        specL->setWordWrap(true);
        specL->setText(
            "<span style='font-size:10px; color:#b0a0d0;'>✦ " + ci.special + "</span>"
        );

        QFrame* sep = new QFrame();
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("border-color: #2a2a4a;");

        QLabel* descL = new QLabel(ci.desc);
        descL->setAlignment(Qt::AlignCenter);
        descL->setWordWrap(true);
        descL->setStyleSheet("font-size: 10px; color: #606080;");

        cLay->addLayout(poseRow);
        cLay->addWidget(nameL,    0, Qt::AlignCenter);
        cLay->addLayout(hpRow);
        cLay->addLayout(atkRow);
        cLay->addWidget(specL,    0, Qt::AlignCenter);
        cLay->addWidget(sep);
        cLay->addWidget(descL,    0, Qt::AlignCenter);

        QPushButton* overlay = new QPushButton(card);
        overlay->setGeometry(0, 0, 300, 400);
        overlay->setStyleSheet("background: transparent; border: none;");
        overlay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        int t = i;
        connect(overlay, &QPushButton::clicked, this, [=]() { onCharacterSelected(t); });
        *cardPtrs[i] = overlay;

        cardsRow->addWidget(card);
        if (i == 0) cardWarriorWidget = card;
        else if (i == 1) cardMageWidget = card;
        else cardArcherWidget = card;
    }

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

    // ── VS Preview panel ─────────────────────────────────────
    QLabel* vsPanel = new QLabel();
    vsPanel->setAlignment(Qt::AlignCenter);
    vsPanel->setTextFormat(Qt::RichText);
    vsPanel->setText(
        "<span style='font-size: 12px; color: #555580;'>"
        "⚔ You will face a <b style='color: #d94f4f;'>random opponent</b> — choose wisely! ⚔"
        "</span>"
    );
    vsPanel->setStyleSheet("border: 1px solid #1e1e3a; border-radius: 6px; padding: 8px;");

    // ── Gameplay tip ─────────────────────────────────────────
    QLabel* tip = new QLabel("💡 TIP: Get adjacent to the enemy to attack. Use Q for your special ability (3-turn cooldown).");
    tip->setAlignment(Qt::AlignCenter);
    tip->setWordWrap(true);
    tip->setStyleSheet("font-size: 10px; color: #444466; font-style: italic;");

    root->addWidget(header);
    root->addWidget(selectionLabel);
    root->addLayout(cardsRow);
    root->addLayout(btnRow);
    root->addWidget(vsPanel, 0, Qt::AlignCenter);
    root->addWidget(tip, 0, Qt::AlignCenter);

    stack->addWidget(characterPage);
}



void MainWindow::onCharacterSelected(int type) {
    selectedType = type;

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

    specialCooldown = 0;

    // Update difficulty page's character preview
    static const QStringList classNames = {"WARRIOR", "MAGE", "ARCHER"};
    if (diffCharPreview)
        diffCharPreview->setPixmap(makeArcadeSprite(selectedType, 80, 0));
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
    lay->setAlignment(Qt::AlignCenter);
    lay->setSpacing(14);
    lay->setContentsMargins(40, 24, 40, 20);

    QLabel* title = new QLabel("SELECT DIFFICULTY");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 32px; font-weight: 900;
        color: #ffffff; letter-spacing: 6px;
        font-family: "Impact", "Arial Black", sans-serif;
    )");
    QGraphicsDropShadowEffect* dGlow = new QGraphicsDropShadowEffect();
    dGlow->setBlurRadius(24); dGlow->setColor(QColor("#7c5cbf")); dGlow->setOffset(0,0);
    title->setGraphicsEffect(dGlow);

    QLabel* sub = new QLabel("Choose your challenge");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 13px; color: #7a7a99; letter-spacing: 2px;");

    // ── Character preview (updated by onStartClicked) ─────────
    QVBoxLayout* previewCol = new QVBoxLayout();
    previewCol->setAlignment(Qt::AlignCenter);
    previewCol->setSpacing(4);
    diffCharPreview = new QLabel();
    diffCharPreview->setFixedSize(80, 80);
    diffCharPreview->setPixmap(makeArcadeSprite(0, 80, 0));
    diffCharPreview->setAlignment(Qt::AlignCenter);
    diffCharPreview->setStyleSheet("border: 2px solid #7c5cbf; border-radius: 6px; background: rgba(0,0,0,100);");
    diffCharName = new QLabel("WARRIOR");
    diffCharName->setAlignment(Qt::AlignCenter);
    diffCharName->setStyleSheet("font-size: 12px; color: #7c5cbf; font-weight: bold; letter-spacing: 2px; font-family: 'Courier New', monospace;");
    QLabel* readyLabel = new QLabel("YOUR FIGHTER");
    readyLabel->setAlignment(Qt::AlignCenter);
    readyLabel->setStyleSheet("font-size: 9px; color: #555580; letter-spacing: 3px;");
    previewCol->addWidget(readyLabel);
    previewCol->addWidget(diffCharPreview, 0, Qt::AlignCenter);
    previewCol->addWidget(diffCharName);

    // ── Difficulty buttons in a horizontal layout ─────────────
    QHBoxLayout* diffRow = new QHBoxLayout();
    diffRow->setSpacing(30);
    diffRow->setAlignment(Qt::AlignCenter);

    // Easy card
    QVBoxLayout* easyCol = new QVBoxLayout();
    easyCol->setAlignment(Qt::AlignCenter);
    easyCol->setSpacing(8);

    QLabel* easyBadge = new QLabel("★ RECOMMENDED");
    easyBadge->setAlignment(Qt::AlignCenter);
    easyBadge->setStyleSheet("font-size: 9px; color: #3dba6e; letter-spacing: 2px; font-weight: bold;");

    QPushButton* btnEasy = new QPushButton("🛡  EASY");
    btnEasy->setFixedSize(240, 64);
    btnEasy->setCursor(Qt::PointingHandCursor);
    btnEasy->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #1a7a3a, stop:1 #3dba6e);
            color: #fff; font-size: 20px; font-weight: bold;
            border: 2px solid #28d47e; border-radius: 10px; letter-spacing: 2px;
        }
        QPushButton:hover { background: #3dba6e; border: 2px solid #50e890; }
    )");

    QLabel* easyDesc = new QLabel("Normal speed\nBasic attacks only\nPerfect for learning");
    easyDesc->setAlignment(Qt::AlignCenter);
    easyDesc->setStyleSheet("font-size: 10px; color: #3dba6e; line-height: 1.4;");

    easyCol->addWidget(easyBadge);
    easyCol->addWidget(btnEasy);
    easyCol->addWidget(easyDesc);

    // VS divider
    QLabel* vsDivider = new QLabel("VS");
    vsDivider->setAlignment(Qt::AlignCenter);
    vsDivider->setStyleSheet("font-size: 22px; font-weight: 900; color: #444466; font-family: 'Impact', sans-serif;");

    // Hard card
    QVBoxLayout* hardCol = new QVBoxLayout();
    hardCol->setAlignment(Qt::AlignCenter);
    hardCol->setSpacing(8);

    QLabel* hardBadge = new QLabel("⚠ ONLY FOR THE BRAVE ⚠");
    hardBadge->setAlignment(Qt::AlignCenter);
    hardBadge->setStyleSheet("font-size: 9px; color: #d94f4f; letter-spacing: 2px; font-weight: bold;");

    QPushButton* btnHard = new QPushButton("💀  HARD");
    btnHard->setFixedSize(240, 64);
    btnHard->setCursor(Qt::PointingHandCursor);
    btnHard->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #8b0000, stop:1 #d94f4f);
            color: #fff; font-size: 20px; font-weight: bold;
            border: 2px solid #ff6b6b; border-radius: 10px; letter-spacing: 2px;
        }
        QPushButton:hover { background: #d94f4f; border: 2px solid #ff9090; }
    )");

    QLabel* hardDesc = new QLabel("2× faster timer\nEnemy uses specials at low HP\nFor experienced warriors");
    hardDesc->setAlignment(Qt::AlignCenter);
    hardDesc->setStyleSheet("font-size: 10px; color: #d94f4f; line-height: 1.4;");

    hardCol->addWidget(hardBadge);
    hardCol->addWidget(btnHard);
    hardCol->addWidget(hardDesc);

    diffRow->addLayout(easyCol);
    diffRow->addWidget(vsDivider);
    diffRow->addLayout(hardCol);

    // ── Comparison table ─────────────────────────────────────
    QLabel* comparison = new QLabel();
    comparison->setTextFormat(Qt::RichText);
    comparison->setAlignment(Qt::AlignCenter);
    comparison->setText(
        "<table style='font-size: 11px; color: #7a7a99;'>"
        "<tr><td style='color: #555580; padding: 3px 12px;'>Feature</td>"
        "<td style='color: #3dba6e; padding: 3px 12px;'>Easy</td>"
        "<td style='color: #d94f4f; padding: 3px 12px;'>Hard</td></tr>"
        "<tr><td style='padding: 2px 12px;'>Enemy Speed</td>"
        "<td style='padding: 2px 12px;'>Normal</td>"
        "<td style='padding: 2px 12px;'>2× Faster</td></tr>"
        "<tr><td style='padding: 2px 12px;'>Enemy AI</td>"
        "<td style='padding: 2px 12px;'>Basic</td>"
        "<td style='padding: 2px 12px;'>Advanced</td></tr>"
        "<tr><td style='padding: 2px 12px;'>Special Attacks</td>"
        "<td style='padding: 2px 12px;'>None</td>"
        "<td style='padding: 2px 12px;'>At low HP</td></tr>"
        "</table>"
    );
    comparison->setStyleSheet("border: 1px solid #1e1e3a; border-radius: 6px; padding: 8px;");

    QPushButton* btnBack = new QPushButton("← Back");
    btnBack->setFixedSize(120, 38);
    btnBack->setStyleSheet(R"(
        QPushButton {
            background: #1e1e3a; color: #7a7a99;
            border: 1px solid #2a2a4a; border-radius: 8px; font-size: 13px;
        }
        QPushButton:hover { background: #25253d; color: #e8e4f0; }
    )");
    connect(btnBack, &QPushButton::clicked, this, [=]() {
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
    lay->addSpacing(6);
    lay->addLayout(previewCol);
    lay->addSpacing(8);
    lay->addLayout(diffRow);
    lay->addSpacing(6);
    lay->addWidget(comparison, 0, Qt::AlignCenter);
    lay->addSpacing(8);
    lay->addWidget(btnBack, 0, Qt::AlignLeft);

    stack->addWidget(difficultyPage);
}



// Helper: actually launch the game after difficulty is chosen
void MainWindow::startBattle()
{
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

    static const QStringList classNames = {"Warrior", "Mage", "Archer"};
    lblPlayerClass->setText(classNames[selectedType]);
    lblEnemyClass->setText(classNames[enemyType]);

    QPixmap pm = makeArcadeSprite(selectedType, 64, 0);
    if (playerPortraitLabel) playerPortraitLabel->setPixmap(pm);
    QPixmap enemyPm = makeArcadeSprite(enemyType, 64, 0);
    if (enemyPortraitLabel) enemyPortraitLabel->setPixmap(enemyPm);

    drawGrid();

    QPixmap playerPm = makeArcadeSprite(selectedType, CELL - 8, 0, playerFacing, playerWalkFrame);
    playerToken = scene->addPixmap(playerPm);
    playerToken->setZValue(2);

    QPixmap enemyPixmap = makeArcadeSprite(enemyType, CELL - 8, 0, enemyFacing, enemyWalkFrame);
    enemyToken = scene->addPixmap(enemyPixmap);
    enemyToken->setZValue(2);

    updateTokenPositions();
    updateHUD();
    updateBottomBar();
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

    QLabel* role = new QLabel(isPlayer ? "YOU" : "ENEMY");
    role->setAlignment(Qt::AlignCenter);
    role->setStyleSheet(QString(
        "background: %1; color: #fff; border-radius: 4px; "
        "font-size: 11px; font-weight: bold; padding: 3px; letter-spacing: 2px;"
    ).arg(isPlayer ? "#5a3ea0" : "#8b2020"));

    QLabel*& portraitL = isPlayer ? playerPortraitLabel : enemyPortraitLabel;
    portraitL = new QLabel();
    portraitL->setFixedSize(64, 64);
    portraitL->setAlignment(Qt::AlignCenter);
    portraitL->setStyleSheet(QString(
        "border: 2px solid %1; border-radius: 4px; background: rgba(0,0,0,80);"
    ).arg(isPlayer ? "#7c5cbf" : "#d94f4f"));
    QPixmap pm = makeArcadeSprite(0, 64, 0);
    portraitL->setPixmap(pm);

    QLabel*& nameL = isPlayer ? lblPlayerName : lblEnemyName;
    nameL = new QLabel("—");
    nameL->setAlignment(Qt::AlignCenter);
    nameL->setStyleSheet("font-size: 16px; font-weight: bold; color: #e8e4f0;");

    QLabel*& classL = isPlayer ? lblPlayerClass : lblEnemyClass;
    classL = new QLabel("—");
    classL->setAlignment(Qt::AlignCenter);
    classL->setStyleSheet(QString("font-size: 12px; color: %1;")
        .arg(isPlayer ? "#7c5cbf" : "#d94f4f"));

    QFrame* sep = new QFrame(); sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("border-color: #2a2a4a;");

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

    // DANGER label (hidden by default)
    QLabel*& dangerL = isPlayer ? lblPlayerDanger : lblEnemyDanger;
    dangerL = new QLabel("⚠ DANGER");
    dangerL->setAlignment(Qt::AlignCenter);
    dangerL->setStyleSheet("font-size: 11px; font-weight: bold; color: #d94f4f; letter-spacing: 2px;");
    dangerL->setVisible(false);
    lay->addWidget(dangerL);

    lay->addStretch();

    if (isPlayer) {
        QLabel* hint = new QLabel();
        hint->setTextFormat(Qt::RichText);
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
    root->setContentsMargins(16, 8, 16, 8);
    root->setSpacing(6);

    QLabel* arenaTitle = new QLabel("BATTLE ARENA");
    arenaTitle->setAlignment(Qt::AlignCenter);
    arenaTitle->setFixedHeight(56);
    arenaTitle->setStyleSheet(R"(
        font-size: 34px;
        font-weight: 900;
        color: #fff2a6;
        letter-spacing: 10px;
        font-family: "Impact", "Arial Black", "Courier New", sans-serif;
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 rgba(124, 92, 191, 70),
            stop:0.45 rgba(10, 10, 24, 225),
            stop:0.55 rgba(10, 10, 24, 225),
            stop:1 rgba(40, 168, 154, 70));
        border-top: 2px solid rgba(125, 246, 255, 160);
        border-bottom: 2px solid rgba(255, 216, 90, 175);
        border-radius: 6px;
        padding-bottom: 2px;
    )");
    QGraphicsDropShadowEffect* arenaGlow = new QGraphicsDropShadowEffect();
    arenaGlow->setBlurRadius(34);
    arenaGlow->setColor(QColor("#ffd85a"));
    arenaGlow->setOffset(0, 0);
    arenaTitle->setGraphicsEffect(arenaGlow);

    // ── Top bar ──────────────────────────────────────────────
    QHBoxLayout* topBar = new QHBoxLayout();
    lblTurnInfo = new QLabel("Your turn — move or attack");
    lblTurnInfo->setStyleSheet("font-size: 13px; color: #7c5cbf; font-weight: bold;");
    lblScore = new QLabel("Score: 0");
    lblScore->setStyleSheet("font-size: 13px; color: #d4a017;");
    topBar->addWidget(lblTurnInfo);
    topBar->addStretch();
    topBar->addWidget(lblScore);

    // ── Main row: HUD | Grid | HUD ───────────────────────────
    QHBoxLayout* midRow = new QHBoxLayout();
    midRow->setSpacing(12);

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
    gridView->setStyleSheet("background: #181830; border: 2px solid #2a2a4a; border-radius: 8px;");

    drawGrid();

    midRow->addWidget(leftPanel,  0, Qt::AlignTop);
    midRow->addWidget(gridView,   0, Qt::AlignCenter);
    midRow->addWidget(rightPanel, 0, Qt::AlignTop);

    // ── Action buttons row ───────────────────────────────────
    QHBoxLayout* actionRow = new QHBoxLayout();
    actionRow->setSpacing(10);
    actionRow->setAlignment(Qt::AlignCenter);

    btnActionAttack = new QPushButton("⚔  ATTACK  [Space]");
    btnActionAttack->setFixedSize(180, 36);
    btnActionAttack->setCursor(Qt::PointingHandCursor);
    btnActionAttack->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #8b2020,stop:1 #d94f4f);
            color: #fff; font-size: 11px; font-weight: bold;
            border: 1px solid #ff6b6b; border-radius: 6px; letter-spacing: 1px;
        }
        QPushButton:hover { background: #d94f4f; }
    )");
    connect(btnActionAttack, &QPushButton::clicked, this, [this]() {
        QKeyEvent evt(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
        keyPressEvent(&evt);
    });

    btnActionSpecial = new QPushButton("✦  SPECIAL  [Q]");
    btnActionSpecial->setFixedSize(180, 36);
    btnActionSpecial->setCursor(Qt::PointingHandCursor);
    btnActionSpecial->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #7a5a10,stop:1 #d4a017);
            color: #fff; font-size: 11px; font-weight: bold;
            border: 1px solid #ffe060; border-radius: 6px; letter-spacing: 1px;
        }
        QPushButton:hover { background: #d4a017; }
    )");
    connect(btnActionSpecial, &QPushButton::clicked, this, [this]() {
        QKeyEvent evt(QEvent::KeyPress, Qt::Key_Q, Qt::NoModifier);
        keyPressEvent(&evt);
    });

    actionRow->addWidget(btnActionAttack);
    actionRow->addWidget(btnActionSpecial);

    // ── Combat log ───────────────────────────────────────────
    lblCombatLog = new QLabel("<span style='color:#555580;'>Battle begins!</span>");
    lblCombatLog->setTextFormat(Qt::RichText);
    lblCombatLog->setWordWrap(true);
    lblCombatLog->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    lblCombatLog->setMinimumHeight(50);
    lblCombatLog->setMaximumHeight(60);
    lblCombatLog->setStyleSheet(
        "font-size: 10px; color: #7a7a99; "
        "background: rgba(10,10,24,180); border: 1px solid #1e1e3a; "
        "border-radius: 6px; padding: 6px 10px;"
    );

    // ── Bottom status bar ────────────────────────────────────
    QHBoxLayout* botBar = new QHBoxLayout();
    botBar->setSpacing(20);

    lblTurnCounter = new QLabel("Turn: 0");
    lblTurnCounter->setStyleSheet("font-size: 11px; color: #7a7a99; font-family: 'Courier New', monospace;");

    lblDistance = new QLabel("Distance: —");
    lblDistance->setStyleSheet("font-size: 11px; color: #7a7a99; font-family: 'Courier New', monospace;");

    lblDiffBadge = new QLabel("EASY");
    lblDiffBadge->setAlignment(Qt::AlignCenter);
    lblDiffBadge->setStyleSheet(
        "font-size: 10px; font-weight: bold; color: #3dba6e; "
        "background: rgba(61,186,110,30); border: 1px solid #3dba6e; "
        "border-radius: 4px; padding: 2px 8px; letter-spacing: 2px;"
    );

    QLabel* cooldownHint = new QLabel("Cooldown: Ready");
    cooldownHint->setObjectName("cooldownHint");
    cooldownHint->setStyleSheet("font-size: 10px; color: #b0a0d0; font-family: 'Courier New', monospace;");

    botBar->addWidget(lblTurnCounter);
    botBar->addWidget(lblDistance);
    botBar->addStretch();
    botBar->addWidget(cooldownHint);
    botBar->addWidget(lblDiffBadge);

    // ── Assemble ─────────────────────────────────────────────
    root->addWidget(arenaTitle);
    root->addLayout(topBar);
    root->addLayout(midRow);
    root->addLayout(actionRow);
    root->addWidget(lblCombatLog);
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

    // Show attack/special sprite
    QPixmap attackPm = makeArcadeSprite(type, CELL - 8, pose, facing, walkFrame);
    token->setPixmap(attackPm);
    if (portrait) portrait->setPixmap(makeArcadeSprite(type, 64, pose));

    // Restore idle sprite after short delay
    QTimer::singleShot(400, this, [=]() {
        if (token) token->setPixmap(makeArcadeSprite(type, CELL - 8, 0, facing, walkFrame));
        if (portrait) portrait->setPixmap(makeArcadeSprite(type, 64, 0));
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
                enemyToken->setPixmap(makeArcadeSprite(enemyType, CELL - 8, 0, enemyFacing, enemyWalkFrame));
        }
    }

    if (grid->isAdjacent(enemy->getGridX(), enemy->getGridY(), playerRow, playerCol)) {
        if (hardMode && enemy->getCurrentHealth() < 0.3 * enemy->getMaxHealth()) {
            int dmg = enemy->specialAbility();
            player->takeDamage(dmg);
            flashAttackPose(false, 2);
            addCombatMessage("<span style='color:#ff6b6b;'>💀 Enemy used Special for " + QString::number(dmg) + " damage!</span>");
        } else {
            int dmg = enemy->attack();
            player->takeDamage(dmg);
            flashAttackPose(false, 1);
            addCombatMessage("<span style='color:#d94f4f;'>⚔ Enemy attacked for " + QString::number(dmg) + " damage</span>");
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

    // HP bar color transitions
    auto hpBarStyle = [](double pct) -> QString {
        QString color1, color2;
        if (pct > 0.6) { color1 = "#3dba6e"; color2 = "#28d47e"; }
        else if (pct > 0.3) { color1 = "#d4a017"; color2 = "#e8b830"; }
        else { color1 = "#d94f4f"; color2 = "#ff6b6b"; }
        return QString("QProgressBar { border:1px solid #2a2a4a; border-radius:4px; background:#0a0a18; height:14px; }"
                       "QProgressBar::chunk { border-radius:4px; background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %1,stop:1 %2); }")
                .arg(color1, color2);
    };
    barPlayerHP->setStyleSheet(hpBarStyle((double)pHP / pMax));
    barEnemyHP->setStyleSheet(hpBarStyle((double)eHP / e->getMaxHealth()));

    // DANGER labels
    if (lblPlayerDanger) lblPlayerDanger->setVisible(pHP < 0.25 * pMax);
    if (lblEnemyDanger)  lblEnemyDanger->setVisible(eHP < 0.25 * e->getMaxHealth());

    lblScore->setText("Score: " + QString::number(gameManager->getScore()));
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
            lblGOTitle->setText("⚔  VICTORY!  ⚔");
            lblGOTitle->setStyleSheet("font-size: 48px; font-weight: 900; color: #3dba6e; letter-spacing: 6px; font-family: 'Impact', sans-serif;");
        } else {
            lblGOTitle->setText("💀  DEFEATED  💀");
            lblGOTitle->setStyleSheet("font-size: 48px; font-weight: 900; color: #d94f4f; letter-spacing: 6px; font-family: 'Impact', sans-serif;");
        }
    }
    if (lblGOMessage) {
        lblGOMessage->setText(playerWon
            ? "You vanquished your foe in glorious combat!"
            : "You were slain in battle. The arena claims another...");
    }
    if (lblGOScore) {
        lblGOScore->setText(QString("SCORE: %1").arg(gameManager->getScore()));
    }
    if (lblGOSprite) {
        int sprType = playerWon ? selectedType : enemyType;
        lblGOSprite->setPixmap(makeArcadeSprite(sprType, 120, playerWon ? 2 : 1));
    }

    stack->setCurrentWidget(gameOverPage);

    // Update turn summary
    QLabel* ts = gameOverPage->findChild<QLabel*>("goTurnSummary");
    if (ts) ts->setText("Turns played: " + QString::number(turnCount));
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
        lblTurnCounter->setText("Turn: " + QString::number(turnCount));

    if (lblDistance && gameManager && gameManager->getPlayer() && gameManager->getEnemy()) {
        Character* p = gameManager->getPlayer();
        Character* e = gameManager->getEnemy();
        int dist = std::abs(p->getGridX() - e->getGridX()) + std::abs(p->getGridY() - e->getGridY());
        lblDistance->setText("Distance: " + QString::number(dist));
    }

    if (lblDiffBadge) {
        if (hardMode) {
            lblDiffBadge->setText("HARD");
            lblDiffBadge->setStyleSheet(
                "font-size: 10px; font-weight: bold; color: #d94f4f; "
                "background: rgba(217,79,79,30); border: 1px solid #d94f4f; "
                "border-radius: 4px; padding: 2px 8px; letter-spacing: 2px;"
            );
        } else {
            lblDiffBadge->setText("EASY");
            lblDiffBadge->setStyleSheet(
                "font-size: 10px; font-weight: bold; color: #3dba6e; "
                "background: rgba(61,186,110,30); border: 1px solid #3dba6e; "
                "border-radius: 4px; padding: 2px 8px; letter-spacing: 2px;"
            );
        }
    }

    // Update cooldown hint
    QLabel* cdHint = gamePage->findChild<QLabel*>("cooldownHint");
    if (cdHint) {
        if (specialCooldown > 0)
            cdHint->setText("Cooldown: " + QString::number(specialCooldown) + " turns");
        else
            cdHint->setText("Cooldown: Ready");
    }
}

void MainWindow::buildGameOverPage() {
    ArcadeBgWidget* bg = new ArcadeBgWidget();
    gameOverPage = bg;

    QVBoxLayout* lay = new QVBoxLayout(gameOverPage);
    lay->setAlignment(Qt::AlignCenter);
    lay->setSpacing(16);
    lay->setContentsMargins(40, 40, 40, 40);

    lblGOTitle = new QLabel("VICTORY!");
    lblGOTitle->setAlignment(Qt::AlignCenter);
    lblGOTitle->setStyleSheet("font-size: 48px; font-weight: 900; color: #3dba6e; letter-spacing: 6px; font-family: 'Impact', sans-serif;");
    QGraphicsDropShadowEffect* goGlow = new QGraphicsDropShadowEffect();
    goGlow->setBlurRadius(40); goGlow->setColor(QColor("#3dba6e")); goGlow->setOffset(0,0);
    lblGOTitle->setGraphicsEffect(goGlow);

    lblGOSprite = new QLabel();
    lblGOSprite->setFixedSize(120, 120);
    lblGOSprite->setAlignment(Qt::AlignCenter);
    lblGOSprite->setPixmap(makeArcadeSprite(0, 120, 2));
    lblGOSprite->setStyleSheet("border: 2px solid #3a3a60; border-radius: 8px; background: rgba(0,0,0,100);");

    lblGOMessage = new QLabel("You vanquished your foe!");
    lblGOMessage->setAlignment(Qt::AlignCenter);
    lblGOMessage->setStyleSheet("font-size: 14px; color: #7a7a99;");

    lblGOScore = new QLabel("SCORE: 0");
    lblGOScore->setAlignment(Qt::AlignCenter);
    lblGOScore->setStyleSheet("font-size: 28px; font-weight: bold; color: #d4a017; letter-spacing: 4px; font-family: 'Impact', sans-serif;");

    QLabel* divider = new QLabel("━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    divider->setAlignment(Qt::AlignCenter);
    divider->setStyleSheet("font-size: 11px; color: #2a2a4a;");

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setSpacing(20);
    btnRow->setAlignment(Qt::AlignCenter);

    QPushButton* btnRestart = new QPushButton("▶  PLAY AGAIN");
    btnRestart->setFixedSize(200, 52);
    btnRestart->setCursor(Qt::PointingHandCursor);
    btnRestart->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #5a3ea0,stop:1 #7c5cbf);
            color: #fff; font-size: 16px; font-weight: bold; letter-spacing: 2px;
            border: 2px solid #9370d4; border-radius: 10px;
        }
        QPushButton:hover { background: #7c5cbf; border: 2px solid #b090e0; }
    )");
    connect(btnRestart, &QPushButton::clicked, this, [this]() {
        gameManager->restartGame();
        stack->setCurrentWidget(characterPage);
        auto resetCard = [](QWidget* w) {
            w->setStyleSheet("QWidget { background: #12122a; border: 2px solid #2a2a4a; border-radius: 14px; }");
        };
        resetCard(cardWarriorWidget);
        resetCard(cardMageWidget);
        resetCard(cardArcherWidget);
        selectionLabel->setText("No character selected");
        btnStart->setEnabled(false);
        selectedType = -1;
        delete selectedCharacter;
        selectedCharacter = nullptr;
    });

    QPushButton* btnMenu = new QPushButton("⌂  MAIN MENU");
    btnMenu->setFixedSize(200, 52);
    btnMenu->setCursor(Qt::PointingHandCursor);
    btnMenu->setStyleSheet(R"(
        QPushButton {
            background: #1e1e3a; color: #7a7a99; font-size: 16px; font-weight: bold;
            letter-spacing: 2px; border: 1px solid #2a2a4a; border-radius: 10px;
        }
        QPushButton:hover { background: #25253d; color: #e8e4f0; }
    )");
    connect(btnMenu, &QPushButton::clicked, this, [this]() {
        gameManager->restartGame();
        stack->setCurrentWidget(menuPage);
    });

    btnRow->addWidget(btnRestart);
    btnRow->addWidget(btnMenu);

    QLabel* turnSummary = new QLabel("Turns played: —");
    turnSummary->setObjectName("goTurnSummary");
    turnSummary->setAlignment(Qt::AlignCenter);
    turnSummary->setStyleSheet("font-size: 11px; color: #555580;");

    lay->addStretch();
    lay->addWidget(lblGOTitle, 0, Qt::AlignCenter);
    lay->addWidget(lblGOSprite, 0, Qt::AlignCenter);
    lay->addWidget(lblGOMessage, 0, Qt::AlignCenter);
    lay->addWidget(lblGOScore, 0, Qt::AlignCenter);
    lay->addWidget(divider, 0, Qt::AlignCenter);
    lay->addWidget(turnSummary, 0, Qt::AlignCenter);
    lay->addSpacing(10);
    lay->addLayout(btnRow);
    lay->addStretch();

    stack->addWidget(gameOverPage);
}

// ═══════════════════════════════════════════════════════════════
//  keyPressEvent
// ═══════════════════════════════════════════════════════════════

void MainWindow::keyPressEvent(QKeyEvent* event)
{
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
                lblTurnInfo->setText("⚔ Attack! Hit for " +
                    QString::number(dmg) + " damage.");
                addCombatMessage("<span style='color:#7c5cbf;'>⚔ You attacked for " + QString::number(dmg) + " damage</span>");
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
                lblTurnInfo->setText("✦ Special not ready (" +
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
                lblTurnInfo->setText("✦ Special! Hit for " +
                    QString::number(dmg) + " damage!");
                addCombatMessage("<span style='color:#d4a017;'>✦ You used Special for " + QString::number(dmg) + " damage!</span>");
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
            playerToken->setPixmap(makeArcadeSprite(selectedType, CELL - 8, 0, playerFacing, playerWalkFrame));
        if (specialCooldown > 0) specialCooldown--;
        turnCount++;
        updateTokenPositions();
        updateHUD();
        updateBottomBar();
        lblTurnInfo->setText("Your turn — move or attack");
    }
}
