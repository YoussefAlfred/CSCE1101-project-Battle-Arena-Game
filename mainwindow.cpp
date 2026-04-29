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
#include <QPolygon>

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

static QPixmap makeArcadeSprite(int type, int size, int pose = 0) {
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
    resize(1100, 720);
    setMinimumSize(900, 620);

    gameManager = new GameManager(this);
    connect(gameManager, &GameManager::gameStateChanged,
            this,        &MainWindow::onGameStateChanged);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    buildMenuPage();
    buildCharacterPage();
    buildDifficultyPage();
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
    lay->setSpacing(24);

    QHBoxLayout* coinRow = new QHBoxLayout();
    coinRow->setSpacing(16);
    for (int i = 0; i < 5; i++) {
        QLabel* coin = new QLabel("◆");
        coin->setAlignment(Qt::AlignCenter);
        coin->setStyleSheet(QString("font-size: 14px; color: %1;")
            .arg(i % 2 == 0 ? "#d4a017" : "#7c5cbf"));
        coinRow->addWidget(coin);
    }

    QLabel* insertCoin = new QLabel("INSERT COIN");
    insertCoin->setAlignment(Qt::AlignCenter);
    insertCoin->setStyleSheet(R"(
        font-size: 12px;
        font-weight: bold;
        color: #d4a017;
        letter-spacing: 6px;
        font-family: "Courier New", monospace;
    )");

    QLabel* title = new QLabel("⚔  BATTLE ARENA");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 52px;
        font-weight: 900;
        letter-spacing: 8px;
        color: #ffffff;
        font-family: "Impact", "Arial Black", sans-serif;
    )");
    QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect();
    glow->setBlurRadius(32);
    glow->setColor(QColor("#7c5cbf"));
    glow->setOffset(0, 0);
    title->setGraphicsEffect(glow);

    QLabel* sub = new QLabel("CSCE 1101  ·  Spring 2026");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 12px; color: #7a7a99; letter-spacing: 3px; font-family: 'Courier New', monospace;");

    // ── character preview row — show idle sprites ─────────
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
        portrait->setPixmap(makeArcadeSprite(i, 80, 0));  // idle pose
        portrait->setAlignment(Qt::AlignCenter);
        portrait->setStyleSheet(QString(
            "border: 2px solid %1; border-radius: 4px; background: rgba(0,0,0,80);"
        ).arg(charColors[i]));

        QLabel* name = new QLabel(charNames[i]);
        name->setAlignment(Qt::AlignCenter);
        name->setStyleSheet(QString("font-size: 10px; color: %1; letter-spacing: 2px; font-weight: bold; font-family: 'Courier New', monospace;").arg(charColors[i]));

        col->addWidget(portrait);
        col->addWidget(name);
        charRow->addLayout(col);
    }

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("border: 1px solid #2a2a4a;");
    line->setFixedWidth(400);

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
    cardsRow->setSpacing(24);

    struct CardInfo {
        int type;
        QString name, hp, atk, special, desc, accent;
    };
    QList<CardInfo> cards = {
        {0, "Warrior", "200", "20", "Power Strike (1.5× ATK)",
         "A heavy-hitting frontliner built to absorb punishment.",
         Pal::AMBER},
        {1, "Mage",    "100", "20", "Arcane Storm (3× ATK)",
         "Fragile but devastating — one burst can turn the tide.",
         Pal::TEAL},
        {2, "Archer",  "150", "15", "Double Shot (2× ATK)",
         "Balanced ranger who strikes reliably from any range.",
         Pal::ORANGE},
    };

    QPushButton** cardPtrs[3] = {&cardWarrior, &cardMage, &cardArcher};

    for (int i = 0; i < 3; i++) {
        const CardInfo& ci = cards[i];
        QWidget* card = new QWidget();
        card->setMinimumSize(240, 340);
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
            spr->setFixedSize(56, 56);
            spr->setPixmap(makeArcadeSprite(ci.type, 56, pose));
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

        // Stats row
        QLabel* statsL = new QLabel();
        statsL->setTextFormat(Qt::RichText);
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
        specL->setTextFormat(Qt::RichText);
        specL->setAlignment(Qt::AlignCenter);
        specL->setWordWrap(true);
        specL->setText(
            "<span style='font-size:11px; color:#b0a0d0;'>✦ " + ci.special + "</span>"
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
        cLay->addWidget(statsL,   0, Qt::AlignCenter);
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

    root->addWidget(header);
    root->addWidget(selectionLabel);
    root->addLayout(cardsRow);
    root->addLayout(btnRow);

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
    lay->setSpacing(30);

    QLabel* title = new QLabel("SELECT DIFFICULTY");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 32px; font-weight: 900;
        color: #ffffff; letter-spacing: 6px;
        font-family: "Impact", "Arial Black", sans-serif;
    )");

    QLabel* sub = new QLabel("Choose your challenge");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 13px; color: #7a7a99; letter-spacing: 2px;");

    // Easy button
    QPushButton* btnEasy = new QPushButton("⚔  EASY");
    btnEasy->setFixedSize(220, 60);
    btnEasy->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #1a7a3a, stop:1 #3dba6e);
            color: #fff; font-size: 18px; font-weight: bold;
            border: none; border-radius: 10px; letter-spacing: 2px;
        }
        QPushButton:hover { background: #3dba6e; }
    )");

    // Hard button
    QPushButton* btnHard = new QPushButton("💀  HARD");
    btnHard->setFixedSize(220, 60);
    btnHard->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #8b0000, stop:1 #d94f4f);
            color: #fff; font-size: 18px; font-weight: bold;
            border: none; border-radius: 10px; letter-spacing: 2px;
        }
        QPushButton:hover { background: #d94f4f; }
    )");

    QLabel* easyDesc = new QLabel("Normal speed · Enemy uses basic attacks only");
    easyDesc->setAlignment(Qt::AlignCenter);
    easyDesc->setStyleSheet("font-size: 11px; color: #3dba6e;");

    QLabel* hardDesc = new QLabel("2× faster timer · Enemy uses special when HP < 30%");
    hardDesc->setAlignment(Qt::AlignCenter);
    hardDesc->setStyleSheet("font-size: 11px; color: #d94f4f;");

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
    lay->addSpacing(10);
    lay->addWidget(btnEasy, 0, Qt::AlignCenter);
    lay->addWidget(easyDesc);
    lay->addSpacing(6);
    lay->addWidget(btnHard, 0, Qt::AlignCenter);
    lay->addWidget(hardDesc);
    lay->addSpacing(20);
    lay->addWidget(btnBack, 0, Qt::AlignLeft);

    stack->addWidget(difficultyPage);
}

// Helper: actually launch the game after difficulty is chosen
void MainWindow::startBattle()
{
    Character* enemy = nullptr;
    int enemyType = rand() % 3;
    if      (enemyType == 0) enemy = new Warrior("Enemy");
    else if (enemyType == 1) enemy = new Mage("Enemy");
    else                     enemy = new Archer("Enemy");

    gameManager->startGame(selectedCharacter, enemy);

    static const QStringList classNames = {"Warrior", "Mage", "Archer"};
    static const QStringList enemyNames = {"Warrior", "Mage", "Archer"};
    lblPlayerClass->setText(classNames[selectedType]);
    lblEnemyClass->setText(enemyNames[enemyType]);

    QPixmap pm = makeArcadeSprite(selectedType, 64, 0);
    if (playerPortraitLabel) playerPortraitLabel->setPixmap(pm);
    QPixmap enemyPm = makeArcadeSprite(enemyType, 64, 0);
    if (enemyPortraitLabel) enemyPortraitLabel->setPixmap(enemyPm);

    drawGrid();

    QPixmap playerPm = makeArcadeSprite(selectedType, CELL - 8, 0);
    playerToken = scene->addPixmap(playerPm);
    playerToken->setZValue(2);

    QPixmap enemyPixmap = makeArcadeSprite(enemyType, CELL - 8, 0);
    enemyToken = scene->addPixmap(enemyPixmap);
    enemyToken->setZValue(2);

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
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(10);

    QHBoxLayout* topBar = new QHBoxLayout();
    lblTurnInfo = new QLabel("Your turn — move or attack");
    lblTurnInfo->setStyleSheet("font-size: 13px; color: #7c5cbf; font-weight: bold;");
    lblScore = new QLabel("Score: 0");
    lblScore->setStyleSheet("font-size: 13px; color: #d4a017;");
    topBar->addWidget(lblTurnInfo);
    topBar->addStretch();
    topBar->addWidget(lblScore);

    QHBoxLayout* midRow = new QHBoxLayout();
    midRow->setSpacing(16);

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

    root->addLayout(topBar);
    root->addLayout(midRow);

    stack->addWidget(gamePage);
    gamePage->setFocusPolicy(Qt::StrongFocus);
}

// ─── Helper: flash a character token with an attack pose then restore idle ──
void MainWindow::flashAttackPose(bool isPlayer, int pose) {
    Character* ch = isPlayer ? gameManager->getPlayer() : gameManager->getEnemy();
    if (!ch) return;

    QGraphicsPixmapItem*& token = isPlayer ? playerToken : enemyToken;
    QLabel*& portrait = isPlayer ? playerPortraitLabel : enemyPortraitLabel;
    int type = isPlayer ? selectedType : 0;  // enemy is always Warrior

    if (!token) return;

    // Show attack/special sprite
    QPixmap attackPm = makeArcadeSprite(type, CELL - 8, pose);
    token->setPixmap(attackPm);
    if (portrait) portrait->setPixmap(makeArcadeSprite(type, 64, pose));

    // Restore idle sprite after short delay
    QTimer::singleShot(400, this, [=]() {
        if (token) token->setPixmap(makeArcadeSprite(type, CELL - 8, 0));
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

    int newRow = enemyRow;
    int newCol = enemyCol;

    if (std::abs(dr) >= std::abs(dc)) {
        newRow += (dr > 0 ? 1 : -1);
    } else {
        newCol += (dc > 0 ? 1 : -1);
    }

    grid->moveCharacter(enemy, newRow, newCol);

    if (grid->isAdjacent(enemy->getGridX(), enemy->getGridY(), playerRow, playerCol)) {
        // Hard mode: use special if enemy HP < 30% of max
        if (hardMode && enemy->getCurrentHealth() < 0.3 * enemy->getMaxHealth()) {
            player->takeDamage(enemy->specialAbility());
            flashAttackPose(false, 2);
        } else {
            player->takeDamage(enemy->attack());
            flashAttackPose(false, 1);
        }
    }

    updateTokenPositions();
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
}


void MainWindow::updateTokenPositions()
{
    if (!gameManager || !gameManager->getPlayer() || !gameManager->getEnemy()) return;

    Character* p = gameManager->getPlayer();
    Character* e = gameManager->getEnemy();

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
                // Flash attack pose on player token + portrait
                flashAttackPose(true, 1);
                lblTurnInfo->setText("⚔ Attack! Hit for " +
                    QString::number(dmg) + " damage.");
                updateHUD();
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
                // Flash special pose on player token + portrait
                flashAttackPose(true, 2);
                lblTurnInfo->setText("✦ Special! Hit for " +
                    QString::number(dmg) + " damage!");
                updateHUD();
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
        if (specialCooldown > 0) specialCooldown--;
        updateTokenPositions();
        updateHUD();
        lblTurnInfo->setText("Your turn — move or attack");
    }
}
