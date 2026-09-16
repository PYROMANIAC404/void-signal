#include "raylib.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>

const int SCREEN_W = 1000;
const int SCREEN_H = 600;

const float ROOM_LEFT = 45.0f;
const float ROOM_RIGHT = 955.0f;
const float ROOM_TOP = 75.0f;
const float ROOM_BOTTOM = 530.0f;

// ---------- Palette ----------
const Color COL_BG         = {5, 8, 10, 255};
const Color COL_PANEL      = {10, 17, 20, 255};
const Color COL_PANEL2     = {18, 29, 32, 255};
const Color COL_GREEN      = {90, 255, 150, 255};
const Color COL_DIM_GREEN  = {50, 150, 90, 255};
const Color COL_DARK_GREEN = {20, 70, 45, 255};
const Color COL_WARNING    = {255, 190, 60, 255};
const Color COL_DANGER     = {255, 70, 80, 255};
const Color COL_METAL      = {100, 115, 120, 255};
const Color COL_DARK_METAL = {35, 45, 48, 255};
const Color COL_CYAN       = {80, 230, 255, 255};
const Color COL_WHITE      = {220, 235, 230, 255};
const Color COL_PURPLE     = {190, 100, 255, 255};

// ---------- Game ----------
enum GameState
{
    MENU,
    BOOT,
    DIALOGUE,
    PLAYING,
    CRAFTING,
    WIRING,
    REPAIR,
    PAUSE,
    ENDING
};

enum RoomType
{
    CENTRAL_DECK = 1,
    LOWER_ENGINE,
    UPPER_ENGINE,
    WATER_TREATMENT,
    CAFETERIA,
    MEDICAL,
    OXYGEN,
    COMMUNICATIONS,
    ADMIN,
    ARCHIVE
};

enum HazardType
{
    HAZARD_FIRE,
    HAZARD_HULL_BREACH,
    HAZARD_DEBRIS,
    HAZARD_ELECTRICAL
};

enum ToolType
{
    TOOL_WRENCH,
    TOOL_WELDER,
    TOOL_EXTINGUISHER,
    TOOL_REPAIR_KIT
};

struct Player
{
    Vector2 pos;
    float speed;
    float energy;
    float maxEnergy;
    int health;
    int maxHealth;

    float hurtCooldown;
    float meleeCooldown;
    float meleeTimer;
    float meleeAngle;
    float aimAngle;

    int wrenchDamage;
    int wrenchDurability;
    int wrenchMaxDurability;

    int repairLevel;
    int speedLevel;
    int armorLevel;

    bool welder;
    bool extinguisher;
    bool repairKit;
};

struct Hazard
{
    HazardType type;
    Vector2 pos;
    float radius;
    float intensity;
    bool repaired;
    bool active;
};

struct Salvage
{
    Vector2 pos;
    int metal;
    int circuits;
    int power;
    int data;
    int botParts;
    int coolant;
    bool collected;
};

struct Drone
{
    Vector2 pos;
    Vector2 vel;
    float speed;
    int health;
    int maxHealth;
    bool active;
    bool corrupted;
    float attackCooldown;
    float hitFlash;
};

struct Particle
{
    Vector2 pos;
    Vector2 vel;
    float life;
    float maxLife;
    float size;
    Color color;
};

struct FloatText
{
    Vector2 pos;
    float life;
    char text[48];
    Color color;
    bool active;
};

struct RepairJob
{
    int hazardIndex;
    float progress;
    bool active;
};

Particle particles[220];
FloatText floatTexts[24];
Drone drones[8];
Hazard hazards[12];
Salvage salvage[12];

float gameTime = 0.0f;
float statusTimer = 0.0f;
char statusText[256] = "";
float damageFlash = 0.0f;
float cameraTrauma = 0.0f;
float transitionTimer = 0.0f;

int selectedTool = TOOL_WRENCH;
int activeHazard = -1;
int dialoguePage = 0;
int selectedCraft = 0;
bool mapOpen = false;
bool wiringSolved = false;
bool finalArchiveRecovered = false;
bool escapeReady = false;

// Inventory
int metal = 0;
int circuits = 0;
int powerCells = 0;
int data = 0;
int botParts = 0;
int coolant = 0;

// ---------- Utility ----------
float Dist(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

float ClampF(float v, float lo, float hi)
{
    return std::max(lo, std::min(hi, v));
}

Color WithAlpha(Color c, unsigned char a)
{
    c.a = a;
    return c;
}

void SetStatus(const char* text)
{
    std::snprintf(statusText, sizeof(statusText), "%s", text);
    statusTimer = 2.8f;
}

const char* RoomName(int level)
{
    switch (level)
    {
        case CENTRAL_DECK: return "CENTRAL DECK";
        case LOWER_ENGINE: return "LOWER ENGINE";
        case UPPER_ENGINE: return "UPPER ENGINE";
        case WATER_TREATMENT: return "WATER TREATMENT";
        case CAFETERIA: return "CAFETERIA";
        case MEDICAL: return "MEDICAL";
        case OXYGEN: return "OXYGEN ROOM";
        case COMMUNICATIONS: return "COMMUNICATIONS";
        case ADMIN: return "ADMINISTRATION";
        case ARCHIVE: return "ARCHIVE";
        default: return "UNKNOWN";
    }
}

const char* HazardName(HazardType type)
{
    switch (type)
    {
        case HAZARD_FIRE: return "FIRE";
        case HAZARD_HULL_BREACH: return "HULL BREACH";
        case HAZARD_DEBRIS: return "DEBRIS";
        case HAZARD_ELECTRICAL: return "ELECTRICAL ARC";
        default: return "HAZARD";
    }
}

const char* ToolName(int tool)
{
    switch (tool)
    {
        case TOOL_WRENCH: return "WRENCH";
        case TOOL_WELDER: return "WELDER";
        case TOOL_EXTINGUISHER: return "EXTINGUISHER";
        case TOOL_REPAIR_KIT: return "REPAIR KIT";
        default: return "UNKNOWN";
    }
}

bool HasTool(const Player& p, int tool)
{
    if (tool == TOOL_WRENCH) return p.wrenchDurability > 0;
    if (tool == TOOL_WELDER) return p.welder;
    if (tool == TOOL_EXTINGUISHER) return p.extinguisher;
    if (tool == TOOL_REPAIR_KIT) return p.repairKit;
    return false;
}

// ---------- Particles ----------
void SpawnParticle(Vector2 pos, Vector2 vel, float life, float size, Color color)
{
    for (auto& p : particles)
    {
        if (p.life <= 0.0f)
        {
            p.pos = pos;
            p.vel = vel;
            p.life = life;
            p.maxLife = life;
            p.size = size;
            p.color = color;
            return;
        }
    }
}

void Burst(Vector2 pos, Color color, int count)
{
    for (int i = 0; i < count; ++i)
    {
        float a = GetRandomValue(0, 359) * DEG2RAD;
        float s = (float)GetRandomValue(50, 180);
        SpawnParticle(pos, {cosf(a) * s, sinf(a) * s},
                       GetRandomValue(20, 55) / 100.0f,
                       (float)GetRandomValue(2, 5), color);
    }
}

void UpdateParticles(float dt)
{
    for (auto& p : particles)
    {
        if (p.life <= 0) continue;
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.vel.x *= 1.0f - 2.0f * dt;
        p.vel.y *= 1.0f - 2.0f * dt;
        p.life -= dt;
    }

    for (auto& t : floatTexts)
    {
        if (!t.active) continue;
        t.pos.y -= 24.0f * dt;
        t.life -= dt;
        if (t.life <= 0) t.active = false;
    }
}

void DrawParticles()
{
    BeginBlendMode(BLEND_ADDITIVE);
    for (const auto& p : particles)
    {
        if (p.life <= 0) continue;
        float r = ClampF(p.life / p.maxLife, 0, 1);
        DrawCircleV(p.pos, p.size * r, WithAlpha(p.color, (unsigned char)(r * 180)));
    }
    EndBlendMode();

    for (const auto& t : floatTexts)
    {
        if (!t.active) continue;
        int w = MeasureText(t.text, 13);
        DrawText(t.text, (int)t.pos.x - w / 2, (int)t.pos.y,
                 13, WithAlpha(t.color, (unsigned char)(ClampF(t.life, 0, 1) * 255)));
    }
}

void FloatTextAt(Vector2 pos, const char* text, Color color)
{
    for (auto& t : floatTexts)
    {
        if (!t.active)
        {
            t.active = true;
            t.pos = pos;
            t.life = 1.0f;
            std::snprintf(t.text, sizeof(t.text), "%s", text);
            t.color = color;
            return;
        }
    }
}

// ---------- Room / hazards ----------
void ResetRoomEntities()
{
    for (auto& h : hazards) h.active = false;
    for (auto& s : salvage) s.collected = true;
    for (auto& d : drones) d.active = false;
}

void AddHazard(int index, HazardType type, Vector2 pos, float radius, float intensity)
{
    hazards[index] = {type, pos, radius, intensity, false, true};
}

void AddSalvage(int index, Vector2 pos, int m, int c, int p, int d, int b, int co)
{
    salvage[index] = {pos, m, c, p, d, b, co, false};
}

void SpawnRoomContent(int level)
{
    ResetRoomEntities();

    // Every room gets environmental damage. The locations are deliberately
    // deterministic so the game feels designed rather than randomly noisy.
    int h = 0;

    switch (level)
    {
        case CENTRAL_DECK:
            AddHazard(h++, HAZARD_HULL_BREACH, {785, 180}, 52, 1.0f);
            AddHazard(h++, HAZARD_DEBRIS, {520, 390}, 48, 1.0f);
            AddHazard(h++, HAZARD_ELECTRICAL, {315, 230}, 42, 1.0f);
            AddSalvage(0, {380, 420}, 3, 1, 0, 0, 1, 0);
            AddSalvage(1, {610, 170}, 2, 1, 1, 0, 0, 0);
            break;

        case LOWER_ENGINE:
            AddHazard(h++, HAZARD_FIRE, {700, 180}, 58, 1.0f);
            AddHazard(h++, HAZARD_HULL_BREACH, {840, 405}, 48, 1.0f);
            AddHazard(h++, HAZARD_DEBRIS, {470, 400}, 55, 1.0f);
            AddSalvage(0, {330, 390}, 3, 0, 1, 0, 0, 0);
            AddSalvage(1, {610, 420}, 1, 1, 0, 0, 1, 0);
            break;

        case UPPER_ENGINE:
            AddHazard(h++, HAZARD_ELECTRICAL, {690, 210}, 55, 1.0f);
            AddHazard(h++, HAZARD_FIRE, {800, 380}, 48, 1.0f);
            AddHazard(h++, HAZARD_DEBRIS, {470, 210}, 50, 1.0f);
            AddSalvage(0, {330, 420}, 2, 2, 0, 0, 1, 0);
            AddSalvage(1, {580, 160}, 1, 0, 1, 1, 0, 0);
            break;

        case WATER_TREATMENT:
            AddHazard(h++, HAZARD_HULL_BREACH, {800, 170}, 48, 1.0f);
            AddHazard(h++, HAZARD_ELECTRICAL, {600, 420}, 44, 1.0f);
            AddHazard(h++, HAZARD_DEBRIS, {420, 300}, 50, 1.0f);
            AddSalvage(0, {300, 430}, 2, 1, 0, 0, 0, 2);
            AddSalvage(1, {760, 410}, 1, 0, 1, 0, 1, 1);
            break;

        case CAFETERIA:
            AddHazard(h++, HAZARD_FIRE, {710, 185}, 50, 1.0f);
            AddHazard(h++, HAZARD_DEBRIS, {530, 360}, 65, 1.0f);
            AddHazard(h++, HAZARD_HULL_BREACH, {860, 300}, 42, 1.0f);
            AddSalvage(0, {300, 180}, 2, 0, 0, 1, 1, 0);
            AddSalvage(1, {430, 430}, 2, 1, 0, 0, 0, 0);
            break;

        case MEDICAL:
            AddHazard(h++, HAZARD_ELECTRICAL, {730, 190}, 45, 1.0f);
            AddHazard(h++, HAZARD_HULL_BREACH, {835, 420}, 45, 1.0f);
            AddHazard(h++, HAZARD_DEBRIS, {490, 240}, 55, 1.0f);
            AddSalvage(0, {300, 400}, 1, 2, 0, 2, 1, 0);
            AddSalvage(1, {620, 430}, 2, 1, 0, 0, 2, 0);
            break;

        case OXYGEN:
            AddHazard(h++, HAZARD_FIRE, {620, 180}, 48, 1.0f);
            AddHazard(h++, HAZARD_HULL_BREACH, {830, 210}, 48, 1.0f);
            AddHazard(h++, HAZARD_ELECTRICAL, {520, 410}, 50, 1.0f);
            AddSalvage(0, {300, 210}, 2, 1, 1, 0, 0, 2);
            AddSalvage(1, {710, 430}, 2, 0, 0, 1, 1, 1);
            break;

        case COMMUNICATIONS:
            AddHazard(h++, HAZARD_ELECTRICAL, {710, 250}, 60, 1.0f);
            AddHazard(h++, HAZARD_HULL_BREACH, {850, 420}, 45, 1.0f);
            AddHazard(h++, HAZARD_DEBRIS, {410, 420}, 52, 1.0f);
            AddSalvage(0, {300, 180}, 1, 2, 0, 2, 0, 0);
            AddSalvage(1, {570, 420}, 2, 1, 1, 1, 0, 0);
            break;

        case ADMIN:
            AddHazard(h++, HAZARD_HULL_BREACH, {840, 180}, 50, 1.0f);
            AddHazard(h++, HAZARD_ELECTRICAL, {680, 410}, 52, 1.0f);
            AddHazard(h++, HAZARD_DEBRIS, {460, 250}, 48, 1.0f);
            AddSalvage(0, {300, 400}, 2, 2, 1, 2, 1, 0);
            AddSalvage(1, {620, 170}, 1, 1, 0, 2, 0, 0);
            break;

        case ARCHIVE:
            AddHazard(h++, HAZARD_FIRE, {730, 180}, 45, 1.0f);
            AddHazard(h++, HAZARD_HULL_BREACH, {860, 380}, 50, 1.0f);
            AddHazard(h++, HAZARD_ELECTRICAL, {520, 410}, 48, 1.0f);
            AddSalvage(0, {320, 190}, 2, 2, 1, 3, 1, 0);
            AddSalvage(1, {570, 350}, 3, 2, 1, 4, 2, 0);
            break;
    }

    // More aggressive corrupted maintenance drones as the anomaly gains access.
    int droneCount = std::min(3, 1 + level / 4);
    for (int i = 0; i < 8; ++i) drones[i].active = false;

    for (int i = 0; i < droneCount; ++i)
    {
        Vector2 spawn = {
            690.0f + i * 90.0f,
            150.0f + (i % 2) * 260.0f
        };
        drones[i] = {
            spawn, {0,0},
            34.0f + level * 1.5f,
            3 + level / 3,
            3 + level / 3,
            true,
            true,
            1.8f + i * 0.6f,
            0
        };
    }
}

// ---------- Drawing ----------
void DrawStarfield()
{
    for (int i = 0; i < 90; ++i)
    {
        int x = (i * 83 + 31) % SCREEN_W;
        int y = (i * 47 + 19) % SCREEN_H;
        float pulse = 0.5f + 0.5f * sinf(gameTime * 1.5f + i);
        DrawCircle(x, y, 1, WithAlpha(COL_CYAN, (unsigned char)(20 + pulse * 35)));
    }
}

void DrawHazardStripes(Rectangle r)
{
    for (int x = (int)r.x; x < r.x + r.width; x += 24)
        DrawLineEx({(float)x, r.y},
                   {(float)x + 12, r.y + r.height},
                   8, WithAlpha(COL_WARNING, 120));
}

void DrawRoomBase(int level)
{
    ClearBackground(COL_BG);
    DrawStarfield();

    DrawRectangle(ROOM_LEFT, ROOM_TOP,
                  ROOM_RIGHT - ROOM_LEFT,
                  ROOM_BOTTOM - ROOM_TOP, COL_PANEL);
    DrawRectangleLines(ROOM_LEFT, ROOM_TOP,
                       ROOM_RIGHT - ROOM_LEFT,
                       ROOM_BOTTOM - ROOM_TOP, COL_DARK_GREEN);

    for (int x = (int)ROOM_LEFT; x <= ROOM_RIGHT; x += 45)
        DrawLine(x, (int)ROOM_TOP, x, (int)ROOM_BOTTOM,
                 WithAlpha(COL_GREEN, 16));
    for (int y = (int)ROOM_TOP; y <= ROOM_BOTTOM; y += 45)
        DrawLine((int)ROOM_LEFT, y, (int)ROOM_RIGHT, y,
                 WithAlpha(COL_GREEN, 16));

    switch (level)
    {
        case CENTRAL_DECK:
            DrawRectangle(520, 135, 330, 190, COL_DARK_METAL);
            DrawCircleLines(685, 230, 70, COL_CYAN);
            DrawCircle(685, 230, 48, BLACK);
            DrawText("ALL-SPARK // CONTAINMENT FAILURE", 540, 330, 12, COL_CYAN);
            break;

        case LOWER_ENGINE:
            DrawRectangle(520, 130, 360, 300, COL_DARK_METAL);
            DrawHazardStripes({510, 420, 370, 35});
            DrawCircleLines(700, 255, 95, COL_WARNING);
            DrawCircleLines(700, 255, 70, COL_GREEN);
            DrawText("LOWER REACTOR", 640, 360, 15, COL_WARNING);
            break;

        case UPPER_ENGINE:
            // Upper Engine is intentionally high-contrast so the important
            // machinery and repair points are easy to see.
            DrawRectangle(555, 115, 315, 335, COL_DARK_METAL);
            DrawRectangleLines(555, 115, 315, 335, COL_METAL);

            // Large turbine housing.
            DrawCircle((int)700, 235, 78, COL_BG);
            DrawCircleLines(700, 235, 78, COL_CYAN);
            DrawCircleLines(700, 235, 62, COL_DIM_GREEN);
            DrawCircleLines(700, 235, 42, COL_METAL);
            for (int i = 0; i < 8; ++i)
            {
                float a = i * 0.785398f + gameTime * 0.15f;
                Vector2 a1 = {700 + cosf(a) * 18, 235 + sinf(a) * 18};
                Vector2 a2 = {700 + cosf(a) * 55, 235 + sinf(a) * 55};
                DrawLineEx(a1, a2, 7, COL_METAL);
            }
            DrawCircle(700, 235, 12, COL_CYAN);
            DrawCircle(700, 235, 5, COL_WHITE);
            DrawText("MAIN TURBINE", 655, 325, 12, COL_CYAN);

            // Coolant / power pipes.
            DrawLineEx({575, 150}, {660, 150}, 10, COL_CYAN);
            DrawLineEx({660, 150}, {660, 195}, 10, COL_CYAN);
            DrawLineEx({740, 275}, {740, 390}, 10, COL_WARNING);
            DrawLineEx({740, 390}, {850, 390}, 10, COL_WARNING);

            // Clearly visible service panels.
            DrawRectangle(575, 345, 115, 70, COL_PANEL2);
            DrawRectangleLines(575, 345, 115, 70, COL_GREEN);
            DrawText("POWER", 605, 355, 11, COL_GREEN);
            DrawText("DISTRIBUTION", 588, 375, 10, COL_DIM_GREEN);
            DrawCircle(600, 400, 5, COL_GREEN);
            DrawCircle(620, 400, 5, COL_WARNING);
            DrawCircle(640, 400, 5, COL_DANGER);

            DrawRectangle(755, 135, 90, 75, COL_PANEL2);
            DrawRectangleLines(755, 135, 90, 75, COL_WARNING);
            DrawText("SERVICE", 773, 147, 10, COL_WARNING);
            DrawText("PANEL", 780, 165, 12, COL_WHITE);
            DrawCircle(775, 193, 5, COL_CYAN);
            DrawCircle(795, 193, 5, COL_GREEN);
            DrawCircle(815, 193, 5, COL_DANGER);

            // Floor warning area.
            DrawHazardStripes({560, 425, 310, 25});
            DrawText("UPPER ENGINE // TURBINE CONTROL", 575, 455, 13, COL_GREEN);
            break;

        case WATER_TREATMENT:
            for (int i = 0; i < 3; ++i)
            {
                int x = 560 + i * 110;
                DrawRectangle(x, 150, 80, 220, COL_DARK_METAL);
                DrawRectangleLines(x, 150, 80, 220, COL_CYAN);
                DrawText("H2O", x + 22, 215, 22, COL_CYAN);
            }
            DrawText("CONTAMINATED COOLANT LOOP", 570, 400, 14, COL_WARNING);
            break;

        case CAFETERIA:
            for (int y = 160; y <= 380; y += 110)
            {
                DrawRectangle(560, y, 230, 55, COL_DARK_METAL);
                DrawCircle(590, y + 28, 14, COL_METAL);
                DrawCircle(660, y + 28, 14, COL_METAL);
                DrawCircle(730, y + 28, 14, COL_METAL);
            }
            DrawText("MESS HALL // EVACUATION LOCKED", 540, 440, 14, COL_DIM_GREEN);
            break;

        case MEDICAL:
            for (int y = 145; y < 410; y += 90)
            {
                DrawRectangle(560, y, 250, 55, COL_DARK_METAL);
                DrawRectangleLines(560, y, 250, 55, COL_CYAN);
            }
            DrawText("MEDICAL / CRYOSTORAGE", 600, 440, 15, COL_CYAN);
            break;

        case OXYGEN:
            DrawRectangle(600, 125, 95, 300, COL_DARK_METAL);
            DrawRectangle(750, 125, 95, 300, COL_DARK_METAL);
            DrawText("O2", 630, 240, 35, COL_CYAN);
            DrawText("O2", 780, 240, 35, COL_CYAN);
            DrawLineEx({695, 270}, {750, 270}, 10, COL_GREEN);
            DrawText("ATMOSPHERE: UNSTABLE", 600, 450, 14, COL_WARNING);
            break;

        case COMMUNICATIONS:
            DrawCircleLines(720, 255, 75, COL_CYAN);
            DrawCircleLines(720, 255, 115, WithAlpha(COL_CYAN, 70));
            DrawCircleLines(720, 255, 150, WithAlpha(COL_GREEN, 40));
            DrawLine(230, 255, 320, 190, COL_CYAN);
            DrawLine(230, 255, 320, 320, COL_CYAN);
            DrawText("DEEP SPACE ARRAY", 620, 430, 15, COL_CYAN);
            break;

        case ADMIN:
            DrawRectangle(560, 145, 270, 85, COL_DARK_METAL);
            DrawRectangle(560, 285, 270, 85, COL_DARK_METAL);
            DrawText("CAPTAIN'S LOG", 610, 175, 17, COL_GREEN);
            DrawText("VESSEL AUTHORITY", 600, 315, 17, COL_CYAN);
            DrawText("ADMINISTRATION // COMMAND", 570, 430, 14, COL_DIM_GREEN);
            break;

        case ARCHIVE:
            for (int y = 145; y < 430; y += 70)
            {
                DrawRectangle(530, y, 155, 42, COL_DARK_METAL);
                for (int x = 540; x < 675; x += 27)
                    DrawRectangle(x, y + 6, 16, 29,
                                  WithAlpha(COL_METAL, 170));
            }
            DrawCircleLines(800, 250, 65, COL_PURPLE);
            DrawCircle(800, 250, 42, BLACK);
            DrawText("ALL-SPARK DATA CORE", 690, 350, 15, COL_PURPLE);
            break;
    }

    // Exit door.
    Color doorColor = (level == ARCHIVE && escapeReady) ? COL_GREEN : COL_DANGER;
    DrawRectangle(915, 250, 28, 120, COL_DARK_METAL);
    DrawRectangleLines(915, 250, 28, 120, doorColor);
    DrawText(level == ARCHIVE ? "ESC" : "NEXT",
             900, 385, 11, doorColor);
}

void DrawHazard(const Hazard& h)
{
    if (!h.active) return;

    if (h.repaired)
    {
        DrawCircleLines((int)h.pos.x, (int)h.pos.y, h.radius,
                        WithAlpha(COL_GREEN, 60));
        DrawText("REPAIRED", (int)h.pos.x - 28, (int)h.pos.y - 5, 10, COL_DIM_GREEN);
        return;
    }

    float pulse = 1.0f + sinf(gameTime * 5.0f + h.pos.x) * 0.08f;

    if (h.type == HAZARD_FIRE)
    {
        DrawCircleV(h.pos, h.radius, WithAlpha(COL_DANGER, 20));
        for (int i = 0; i < 5; ++i)
        {
            Vector2 p = {
                h.pos.x + cosf(gameTime * 2 + i) * 18,
                h.pos.y + sinf(gameTime * 3 + i) * 12
            };
            DrawCircleV(p, (10 + i * 2) * pulse, COL_WARNING);
        }
        DrawText("FIRE", (int)h.pos.x - 16, (int)h.pos.y + 55, 11, COL_DANGER);
    }
    else if (h.type == HAZARD_HULL_BREACH)
    {
        DrawCircleV(h.pos, h.radius, WithAlpha(BLACK, 230));
        DrawCircleLines((int)h.pos.x, (int)h.pos.y,
                        h.radius * pulse, COL_CYAN);
        for (int i = 0; i < 5; ++i)
            DrawLineEx(h.pos,
                       {h.pos.x + cosf(i) * h.radius * 1.5f,
                        h.pos.y + sinf(i) * h.radius * 1.5f},
                       2, WithAlpha(COL_CYAN, 80));
        DrawText("HULL BREACH", (int)h.pos.x - 38, (int)h.pos.y + 55, 11, COL_DANGER);
    }
    else if (h.type == HAZARD_DEBRIS)
    {
        DrawCircleV(h.pos, h.radius, WithAlpha(COL_WARNING, 10));
        for (int i = 0; i < 7; ++i)
        {
            float a = i * 0.9f;
            DrawRectangle((int)(h.pos.x + cosf(a) * 24) - 9,
                          (int)(h.pos.y + sinf(a) * 24) - 7,
                          18, 14, COL_METAL);
        }
        DrawText("DEBRIS", (int)h.pos.x - 20, (int)h.pos.y + 55, 11, COL_WARNING);
    }
    else
    {
        // Electrical hazards get a bright service-panel marker instead of
        // relying only on thin lightning lines.
        DrawCircleV(h.pos, h.radius + 8, WithAlpha(COL_CYAN, 18));
        DrawCircleLines((int)h.pos.x, (int)h.pos.y, h.radius, COL_CYAN);
        DrawRectangle((int)h.pos.x - 28, (int)h.pos.y - 22, 56, 44, COL_PANEL2);
        DrawRectangleLines((int)h.pos.x - 28, (int)h.pos.y - 22, 56, 44, COL_CYAN);
        DrawText("!", (int)h.pos.x - 6, (int)h.pos.y - 15, 28, COL_WARNING);
        for (int i = 0; i < 4; ++i)
        {
            Vector2 a = {h.pos.x - h.radius + i * 20, h.pos.y - 20};
            Vector2 b = {a.x + 35, a.y + sinf(gameTime * 12 + i) * 15};
            DrawLineEx(a, b, 4, COL_CYAN);
        }
        DrawText("ELECTRICAL // REPAIR KIT", (int)h.pos.x - 65, (int)h.pos.y + h.radius + 12, 10, COL_CYAN);
    }
}

void DrawSalvage(const Salvage& s)
{
    if (s.collected) return;
    DrawRectangle((int)s.pos.x - 17, (int)s.pos.y - 13, 34, 26, COL_DARK_METAL);
    DrawRectangleLines((int)s.pos.x - 17, (int)s.pos.y - 13, 34, 26, COL_METAL);
    DrawText("SALVAGE", (int)s.pos.x - 25, (int)s.pos.y + 20, 9, COL_DIM_GREEN);
}

void DrawDrone(const Drone& d)
{
    if (!d.active) return;

    DrawCircleV(d.pos, 18, COL_DARK_METAL);
    DrawCircleLines((int)d.pos.x, (int)d.pos.y, 20, COL_DANGER);
    DrawLine((int)d.pos.x - 10, (int)d.pos.y,
             (int)d.pos.x + 10, (int)d.pos.y, COL_DANGER);
    DrawCircle((int)d.pos.x, (int)d.pos.y, 4, COL_WARNING);

    if (d.hitFlash > 0)
        DrawCircleV(d.pos, 23, WithAlpha(COL_WHITE, 150));

    DrawRectangle((int)d.pos.x - 18, (int)d.pos.y - 30, 36, 4, COL_DARK_METAL);
    DrawRectangle((int)d.pos.x - 18, (int)d.pos.y - 30,
                  (int)(36.0f * d.health / d.maxHealth), 4, COL_DANGER);
}

void DrawRobot(const Player& p)
{
    float x = p.pos.x;
    float y = p.pos.y;

    // Compact, battered maintenance-bot silhouette: tracks, box chassis,
    // camera head, utility arms and a physical wrench.
    float bob = 0.0f;
    if (p.meleeTimer > 0)
    {
        float swingProgress = 1.0f - ClampF(p.meleeTimer / 0.28f, 0.0f, 1.0f);
        bob = sinf(swingProgress * 3.14159265f) * 3.0f;
    }
    y += bob;

    // Tracks / wheel housings.
    DrawRectangleRounded({x - 29, y - 17, 12, 34}, 0.35f, 5, COL_DARK_METAL);
    DrawRectangleRounded({x + 17, y - 17, 12, 34}, 0.35f, 5, COL_DARK_METAL);
    DrawRectangleLines((int)x - 29, (int)y - 17, 12, 34, COL_METAL);
    DrawRectangleLines((int)x + 17, (int)y - 17, 12, 34, COL_METAL);

    for (int i = -1; i <= 1; ++i)
    {
        DrawCircle((int)x - 23, (int)y + i * 10, 3, COL_PANEL2);
        DrawCircle((int)x + 23, (int)y + i * 10, 3, COL_PANEL2);
    }

    // Battered torso and service panel.
    DrawRectangleRounded({x - 22, y - 20, 44, 40}, 0.12f, 5, COL_DARK_METAL);
    DrawRectangleLines((int)x - 22, (int)y - 20, 44, 40, COL_METAL);
    DrawRectangle((int)x - 15, (int)y - 12, 30, 20, COL_PANEL2);
    DrawRectangleLines((int)x - 15, (int)y - 12, 30, 20, COL_DIM_GREEN);
    DrawRectangle((int)x - 9, (int)y - 7, 18, 9, COL_BG);
    DrawCircle((int)x - 5, (int)y - 2, 2, COL_GREEN);
    DrawCircle((int)x + 1, (int)y - 2, 2, COL_CYAN);
    DrawCircle((int)x + 7, (int)y - 2, 2, COL_WARNING);
    DrawLine((int)x - 10, (int)y + 7, (int)x + 10, (int)y + 7, COL_DARK_GREEN);

    // Neck + binocular camera head.
    DrawRectangle((int)x - 8, (int)y - 29, 16, 10, COL_METAL);
    DrawRectangleRounded({x - 20, y - 45, 40, 20}, 0.25f, 6, COL_DARK_METAL);
    DrawRectangleLines((int)x - 20, (int)y - 45, 40, 20, COL_METAL);
    DrawRectangle((int)x - 13, (int)y - 41, 10, 11, COL_BG);
    DrawRectangle((int)x + 3, (int)y - 41, 10, 11, COL_BG);
    DrawCircle((int)x - 8, (int)y - 36, 3.5f, COL_CYAN);
    DrawCircle((int)x + 8, (int)y - 36, 3.5f, COL_CYAN);
    DrawCircleLines((int)x - 8, (int)y - 36, 5, COL_DIM_GREEN);
    DrawCircleLines((int)x + 8, (int)y - 36, 5, COL_DIM_GREEN);

    // Small antenna / sensor.
    DrawLineEx({x, y - 45}, {x + 5, y - 55}, 2, COL_METAL);
    DrawCircle((int)x + 5, (int)y - 56, 2.5f,
               (sinf(gameTime * 6.0f) > 0.0f) ? COL_GREEN : COL_DIM_GREEN);

    // Left utility arm.
    float leftBend = sinf(gameTime * 2.0f) * 1.5f;
    DrawLineEx({x - 21, y - 2}, {x - 33, y + 8 + leftBend}, 6, COL_METAL);
    DrawCircle((int)x - 34, (int)(y + 9 + leftBend), 5, COL_DARK_METAL);
    DrawCircleLines((int)x - 34, (int)(y + 9 + leftBend), 5, COL_METAL);

    // Wrench arm. During a swing the arm arcs across the front of the bot.
    float armAngle = p.aimAngle;
    if (p.meleeTimer > 0)
    {
        float progress = 1.0f - ClampF(p.meleeTimer / 0.28f, 0.0f, 1.0f);
        float arc = -1.15f + progress * 2.30f;
        armAngle = p.meleeAngle + arc;
    }

    float c = cosf(armAngle);
    float sn = sinf(armAngle);
    Vector2 shoulder = {x + c * 15, y + sn * 15};
    Vector2 elbow = {x + c * 25 - sn * 5, y + sn * 25 + c * 5};
    float armLength = (p.meleeTimer > 0) ? 37.0f : 27.0f;
    Vector2 hand = {elbow.x + c * armLength, elbow.y + sn * armLength};

    DrawLineEx(shoulder, elbow, 8, COL_DARK_METAL);
    DrawCircleV(elbow, 5, COL_METAL);
    DrawLineEx(elbow, hand, 7, COL_METAL);
    DrawCircleV(hand, 5, COL_DARK_METAL);

    Vector2 wrenchTip = {hand.x + c * 14, hand.y + sn * 14};
    DrawLineEx(hand, wrenchTip, 6, COL_METAL);
    DrawCircleV(wrenchTip, 7, COL_WARNING);
    DrawCircleV({wrenchTip.x - c * 3, wrenchTip.y - sn * 3}, 3, COL_DARK_METAL);

    if (p.meleeTimer > 0)
    {
        float progress = 1.0f - ClampF(p.meleeTimer / 0.28f, 0.0f, 1.0f);
        float trailAngle = p.meleeAngle - 1.15f + progress * 2.30f;
        for (int i = 1; i <= 3; ++i)
        {
            float a = trailAngle - i * 0.14f;
            Vector2 trail = {x + cosf(a) * (55.0f + i * 4.0f),
                             y + sinf(a) * (55.0f + i * 4.0f)};
            DrawLineEx({x + cosf(a) * 28.0f, y + sinf(a) * 28.0f},
                       trail, 3.0f, WithAlpha(COL_WARNING, (unsigned char)(90 - i * 20)));
        }
        DrawCircleV(wrenchTip, 10, WithAlpha(COL_WARNING, 80));
    }
}

void DrawToolIcon(int tool, int x, int y, bool selected)
{
    Color c = selected ? COL_GREEN : COL_DIM_GREEN;
    DrawRectangle(x, y, 160, 42, selected ? COL_DARK_GREEN : COL_PANEL2);
    DrawRectangleLines(x, y, 160, 42, c);
    DrawText(ToolName(tool), x + 12, y + 13, 13, c);
}

void DrawHUD(const Player& p, int level)
{
    DrawRectangle(0, 0, SCREEN_W, 60, WithAlpha(COL_PANEL, 245));
    DrawLine(0, 59, SCREEN_W, 59, COL_DARK_GREEN);

    DrawText("VOID//SIGNAL", 18, 10, 21, COL_GREEN);
    DrawText(RoomName(level), 18, 35, 10, COL_DIM_GREEN);

    DrawText("HULL", 245, 10, 10, COL_DIM_GREEN);
    DrawRectangle(245, 27, 100, 9, COL_DARK_METAL);
    DrawRectangle(245, 27, (int)(100.0f * p.health / p.maxHealth), 9, COL_GREEN);

    DrawText("ENERGY", 365, 10, 10, COL_DIM_GREEN);
    DrawRectangle(365, 27, 100, 9, COL_DARK_METAL);
    DrawRectangle(365, 27, (int)(100.0f * p.energy / p.maxEnergy), 9, COL_CYAN);

    DrawText(TextFormat("TOOL: %s", ToolName(selectedTool)), 490, 12, 11, COL_CYAN);
    DrawText(TextFormat("WRENCH %d/%d", p.wrenchDurability, p.wrenchMaxDurability),
             490, 31, 10, COL_METAL);

    DrawText(TextFormat("MET %d  CIR %d  PWR %d  DAT %d  BOT %d",
             metal, circuits, powerCells, data, botParts),
             650, 15, 10, COL_WHITE);

    DrawRectangle(45, 540, 910, 45, WithAlpha(COL_PANEL, 245));
    DrawText("WASD MOVE", 60, 555, 10, COL_DIM_GREEN);
    DrawText("F WRENCH", 155, 555, 10, COL_GREEN);
    DrawText("E INTERACT", 245, 555, 10, COL_CYAN);
    DrawText("C CRAFT", 355, 555, 10, COL_WARNING);
    DrawText("TAB TOOLS", 435, 555, 10, COL_DIM_GREEN);
    DrawText("M MAP", 545, 555, 10, COL_DIM_GREEN);
    DrawText("ESC PAUSE", 610, 555, 10, COL_DIM_GREEN);

    if (statusTimer > 0)
    {
        DrawRectangle(275, 470, 450, 40, WithAlpha(COL_BG, 235));
        DrawRectangleLines(275, 470, 450, 40, COL_GREEN);
        DrawText(statusText, 295, 483, 13, COL_GREEN);
    }
}

void DrawCrafting(const Player& p)
{
    DrawRectangle(105, 65, 790, 470, COL_PANEL);
    DrawRectangleLines(105, 65, 790, 470, COL_GREEN);

    DrawText("FIELD FABRICATION // XD-07", 140, 95, 24, COL_GREEN);
    DrawText("Salvaged parts can be converted into tools and chassis upgrades.",
             140, 130, 13, COL_DIM_GREEN);

    const char* names[6] = {
        "WELDER",
        "FIRE EXTINGUISHER",
        "REPAIR KIT",
        "WRENCH MK-II",
        "SERVO UPGRADE",
        "ARMOR PLATING"
    };

    const char* costs[6] = {
        "2 MET + 1 CIR",
        "1 MET + 1 PWR",
        "2 MET + 1 CIR",
        "3 MET + 1 CIR + 1 BOT",
        "3 MET + 2 CIR + 1 BOT",
        "4 MET + 2 CIR + 2 BOT"
    };

    for (int i = 0; i < 6; ++i)
    {
        int y = 170 + i * 50;
        bool selected = i == selectedCraft;
        DrawRectangle(145, y, 480, 38,
                      selected ? COL_DARK_GREEN : COL_PANEL2);
        DrawRectangleLines(145, y, 480, 38,
                           selected ? COL_GREEN : COL_DIM_GREEN);
        DrawText(names[i], 160, y + 11, 13,
                 selected ? COL_GREEN : COL_WHITE);
        DrawText(costs[i], 400, y + 11, 11, COL_DIM_GREEN);
    }

    DrawText("1-6 SELECT", 670, 180, 12, COL_CYAN);
    DrawText("ENTER CRAFT", 670, 210, 12, COL_GREEN);
    DrawText("ESC CLOSE", 670, 240, 12, COL_DIM_GREEN);

    DrawText(TextFormat("METAL: %d", metal), 670, 310, 13, COL_WHITE);
    DrawText(TextFormat("CIRCUITS: %d", circuits), 670, 335, 13, COL_WHITE);
    DrawText(TextFormat("POWER: %d", powerCells), 670, 360, 13, COL_WHITE);
    DrawText(TextFormat("BOT PARTS: %d", botParts), 670, 385, 13, COL_WHITE);

    DrawText("DESIGN NOTE", 140, 455, 11, COL_CYAN);
    DrawText("The cube is teaching XD-07 how to rebuild the ship.", 140, 477, 13, COL_DIM_GREEN);
}

void DrawRepairScreen(const Hazard& h, const Player& p)
{
    DrawRectangle(145, 80, 710, 450, COL_PANEL);
    DrawRectangleLines(145, 80, 710, 450, COL_GREEN);

    DrawText(TextFormat("MAINTENANCE // %s", HazardName(h.type)),
             180, 115, 24, COL_WARNING);

    const char* tool = "UNKNOWN";
    if (h.type == HAZARD_FIRE) tool = "EXTINGUISHER";
    if (h.type == HAZARD_HULL_BREACH) tool = "WELDER";
    if (h.type == HAZARD_DEBRIS) tool = "WRENCH";
    if (h.type == HAZARD_ELECTRICAL) tool = "REPAIR KIT";

    DrawText(TextFormat("REQUIRED TOOL: %s", tool),
             180, 160, 14, COL_CYAN);

    DrawText("HOLD SPACE TO PERFORM THE REPAIR.",
             180, 195, 13, COL_DIM_GREEN);

    float t = (sinf(gameTime * 3.8f) + 1.0f) * 0.5f;
    DrawRectangle(205, 275, 590, 32, COL_DARK_METAL);
    DrawRectangle(205, 275, (int)(590 * t), 32, COL_GREEN);
    DrawRectangle(480, 265, 100, 52, WithAlpha(COL_CYAN, 35));
    DrawText("SAFE ZONE", 490, 284, 12, COL_CYAN);

    DrawText("SPACE = REPAIR     ESC = CANCEL", 350, 360, 13, COL_WHITE);
    DrawText(TextFormat("XD-07 WRENCH DURABILITY: %d/%d",
             p.wrenchDurability, p.wrenchMaxDurability),
             350, 395, 12, COL_METAL);
}

void DrawMap(int level)
{
    DrawRectangle(110, 70, 780, 460, WithAlpha(COL_PANEL, 248));
    DrawRectangleLines(110, 70, 780, 460, COL_GREEN);
    DrawText("ZARIMAN // DAMAGE CONTROL SCHEMATIC", 145, 100, 22, COL_GREEN);

    const char* names[10] = {
        "DECK", "LOWER ENG", "UPPER ENG", "WATER", "CAFETERIA",
        "MEDS", "O2", "COMMS", "ADMIN", "ARCHIVE"
    };

    Vector2 nodes[10] = {
        {180, 180}, {330, 180}, {480, 180}, {630, 180}, {780, 180},
        {780, 330}, {630, 330}, {480, 330}, {330, 330}, {180, 330}
    };

    for (int i = 0; i < 9; ++i)
        DrawLineEx(nodes[i], nodes[i + 1], 3,
                   i + 1 < level ? COL_GREEN : COL_DARK_GREEN);

    for (int i = 0; i < 10; ++i)
    {
        bool unlocked = i + 1 <= level;
        Color c = unlocked ? COL_GREEN : COL_DARK_METAL;
        DrawCircleV(nodes[i], 25, c);
        DrawCircleV(nodes[i], 18, COL_PANEL);
        DrawText(names[i], (int)nodes[i].x - 34,
                 (int)nodes[i].y - 5, 9,
                 unlocked ? COL_GREEN : COL_DIM_GREEN);
    }

    DrawText("The route is failing from the inside out.",
             145, 420, 13, COL_DIM_GREEN);
    DrawText("M = CLOSE", 750, 485, 12, COL_CYAN);
}

// ---------- Gameplay systems ----------
bool PlayerInsideHazard(const Player& p, const Hazard& h)
{
    return h.active && !h.repaired && Dist(p.pos, h.pos) < h.radius;
}

void DamagePlayer(Player& p, int amount, const char* reason)
{
    if (p.hurtCooldown > 0) return;

    p.health -= amount;
    p.hurtCooldown = 0.8f;
    damageFlash = 0.25f;
    cameraTrauma = 0.8f;
    SetStatus(reason);
    Burst(p.pos, COL_DANGER, 8);

    if (p.health <= 0)
    {
        p.health = 1;
        SetStatus("XD-07 CRITICAL // EMERGENCY RESTART");
    }
}

void UpdateHazards(float dt, Player& p)
{
    for (auto& h : hazards)
    {
        if (!h.active || h.repaired) continue;

        float d = Dist(p.pos, h.pos);

        if (h.type == HAZARD_FIRE && d < h.radius)
            DamagePlayer(p, 1, "WARNING: FIRE EXPOSURE");

        if (h.type == HAZARD_ELECTRICAL && d < h.radius)
            DamagePlayer(p, 1, "WARNING: ELECTRICAL ARC");

        if (h.type == HAZARD_HULL_BREACH)
        {
            if (d < h.radius * 2.0f)
            {
                float pull = (h.radius * 2.0f - d) / (h.radius * 2.0f);
                if (d > 4)
                {
                    p.pos.x += (h.pos.x - p.pos.x) / d * pull * 18.0f * dt;
                    p.pos.y += (h.pos.y - p.pos.y) / d * pull * 18.0f * dt;
                }
            }
            if (d < h.radius)
                DamagePlayer(p, 1, "WARNING: HULL DECOMPRESSION");
        }
    }
}

bool HazardNeedsTool(const Hazard& h, const Player& p)
{
    if (h.type == HAZARD_FIRE) return p.extinguisher;
    if (h.type == HAZARD_HULL_BREACH) return p.welder;
    if (h.type == HAZARD_DEBRIS) return p.wrenchDurability > 0;
    if (h.type == HAZARD_ELECTRICAL) return p.repairKit;
    return false;
}

bool RepairNearbyHazard(Player& p)
{
    int best = -1;
    float bestDist = 9999;

    for (int i = 0; i < 12; ++i)
    {
        if (!hazards[i].active || hazards[i].repaired) continue;
        float d = Dist(p.pos, hazards[i].pos);
        if (d < 75 && d < bestDist)
        {
            best = i;
            bestDist = d;
        }
    }

    if (best < 0)
    {
        SetStatus("NO REPAIR TARGET IN RANGE");
        return false;
    }

    if (!HazardNeedsTool(hazards[best], p))
    {
        switch (hazards[best].type)
        {
            case HAZARD_FIRE: SetStatus("NEED A FIRE EXTINGUISHER"); break;
            case HAZARD_HULL_BREACH: SetStatus("NEED A WELDER"); break;
            case HAZARD_DEBRIS: SetStatus("WRENCH IS BROKEN"); break;
            case HAZARD_ELECTRICAL: SetStatus("NEED A REPAIR KIT"); break;
        }
        return false;
    }

    activeHazard = best;
    return true;
}

void CompleteHazardRepair(Player& p, int index)
{
    if (index < 0 || index >= 12) return;
    Hazard& h = hazards[index];
    if (h.repaired) return;

    if (h.type == HAZARD_DEBRIS)
    {
        p.wrenchDurability = std::max(0, p.wrenchDurability - 3);
        if (p.wrenchDurability == 0)
            SetStatus("DEBRIS CLEARED // WRENCH DAMAGED");
    }

    if (h.type == HAZARD_ELECTRICAL)
        p.repairKit = false;

    h.repaired = true;
    Burst(h.pos, COL_GREEN, 18);
    FloatTextAt(h.pos, "SYSTEM RESTORED", COL_GREEN);
    SetStatus(TextFormat("%s REPAIRED", HazardName(h.type)));
    activeHazard = -1;
}

bool AllRoomHazardsRepaired()
{
    bool any = false;
    for (const auto& h : hazards)
    {
        if (!h.active) continue;
        any = true;
        if (!h.repaired) return false;
    }
    return any;
}

void UpdateDrones(float dt, Player& p)
{
    for (auto& d : drones)
    {
        if (!d.active) continue;

        d.hitFlash = std::max(0.0f, d.hitFlash - dt * 5.0f);

        Vector2 diff = {p.pos.x - d.pos.x, p.pos.y - d.pos.y};
        float len = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (len > 1)
        {
            diff.x /= len;
            diff.y /= len;
        }

        d.pos.x += diff.x * d.speed * dt;
        d.pos.y += diff.y * d.speed * dt;

        d.attackCooldown -= dt;
        if (d.attackCooldown <= 0 && len < 260)
        {
            d.attackCooldown = 1.7f;
            if (len < 34)
                DamagePlayer(p, 1, "CORRUPTED MAINTENANCE UNIT");
        }

        d.pos.x = ClampF(d.pos.x, ROOM_LEFT + 25, ROOM_RIGHT - 25);
        d.pos.y = ClampF(d.pos.y, ROOM_TOP + 30, ROOM_BOTTOM - 30);
    }
}

void MeleeAttack(Player& p)
{
    if (p.meleeCooldown > 0) return;

    if (p.wrenchDurability <= 0)
    {
        SetStatus("WRENCH BROKEN // REPAIR IT AT A BENCH");
        return;
    }

    p.meleeCooldown = 0.42f;
    p.meleeTimer = 0.28f;
    p.meleeAngle = p.aimAngle;

    Vector2 hit = {
        p.pos.x + cosf(p.aimAngle) * 50,
        p.pos.y + sinf(p.aimAngle) * 50
    };

    bool hitSomething = false;

    for (auto& d : drones)
    {
        if (!d.active) continue;
        if (Dist(hit, d.pos) < 35)
        {
            d.health -= p.wrenchDamage;
            d.hitFlash = 0.15f;
            hitSomething = true;
            Burst(d.pos, COL_WARNING, 8);

            if (d.health <= 0)
            {
                d.active = false;
                botParts++;
                metal++;
                FloatTextAt(d.pos, "BOT PARTS +1", COL_CYAN);
                SetStatus("CORRUPTED UNIT DISABLED // SALVAGE RECOVERED");
                Burst(d.pos, COL_CYAN, 18);
            }
        }
    }

    // Wrench is also the physical maintenance tool for loose debris.
    for (auto& h : hazards)
    {
        if (h.active && !h.repaired &&
            h.type == HAZARD_DEBRIS &&
            Dist(hit, h.pos) < h.radius)
        {
            h.repaired = true;
            p.wrenchDurability = std::max(0, p.wrenchDurability - 2);
            hitSomething = true;
            Burst(h.pos, COL_METAL, 12);
            SetStatus("DEBRIS SMASHED // PATH CLEARED");
        }
    }

    p.wrenchDurability = std::max(0, p.wrenchDurability - (hitSomething ? 1 : 0));
    cameraTrauma = hitSomething ? 0.75f : 0.42f;
}

// ---------- Crafting ----------
bool CraftSelected(Player& p)
{
    switch (selectedCraft)
    {
        case 0: // Welder
            if (p.welder) { SetStatus("WELDER ALREADY INSTALLED"); return false; }
            if (metal >= 2 && circuits >= 1)
            {
                metal -= 2; circuits -= 1; p.welder = true;
                SetStatus("WELDER FABRICATED");
                return true;
            }
            break;

        case 1: // Extinguisher
            if (p.extinguisher) { SetStatus("EXTINGUISHER ALREADY INSTALLED"); return false; }
            if (metal >= 1 && powerCells >= 1)
            {
                metal -= 1; powerCells -= 1; p.extinguisher = true;
                SetStatus("FIRE EXTINGUISHER FABRICATED");
                return true;
            }
            break;

        case 2: // Repair kit
            if (p.repairKit) { SetStatus("REPAIR KIT ALREADY INSTALLED"); return false; }
            if (metal >= 2 && circuits >= 1)
            {
                metal -= 2; circuits -= 1; p.repairKit = true;
                SetStatus("REPAIR KIT FABRICATED");
                return true;
            }
            break;

        case 3: // Wrench MkII
            if (p.wrenchDamage >= 3)
            {
                SetStatus("WRENCH ALREADY UPGRADED");
                return false;
            }
            if (metal >= 3 && circuits >= 1 && botParts >= 1)
            {
                metal -= 3; circuits -= 1; botParts -= 1;
                p.wrenchDamage = 3;
                p.wrenchMaxDurability += 20;
                p.wrenchDurability = p.wrenchMaxDurability;
                SetStatus("WRENCH MK-II INSTALLED // IMPACT +");
                return true;
            }
            break;

        case 4: // Servo
            if (p.speedLevel >= 2)
            {
                SetStatus("SERVO SYSTEM FULLY CALIBRATED");
                return false;
            }
            if (metal >= 3 && circuits >= 2 && botParts >= 1)
            {
                metal -= 3; circuits -= 2; botParts -= 1;
                p.speedLevel++;
                p.speed += 35;
                SetStatus("SERVO UPGRADE INSTALLED // MOBILITY +");
                return true;
            }
            break;

        case 5: // Armor
            if (p.armorLevel >= 2)
            {
                SetStatus("ARMOR PLATING MAXED");
                return false;
            }
            if (metal >= 4 && circuits >= 2 && botParts >= 2)
            {
                metal -= 4; circuits -= 2; botParts -= 2;
                p.armorLevel++;
                p.maxHealth += 2;
                p.health = p.maxHealth;
                SetStatus("ARMOR PLATING INSTALLED // HULL +");
                return true;
            }
            break;
    }

    SetStatus("INSUFFICIENT SALVAGE");
    return false;
}

// ---------- Story ----------
void DrawDialogue(int page)
{
    const char* title = "";
    const char* body1 = "";
    const char* body2 = "";

    switch (page)
    {
        case 0:
            title = "XD-07 // BOOT";
            body1 = "Maintenance unit online.";
            body2 = "Memory integrity: unknown. Vessel integrity: catastrophic.";
            break;

        case 1:
            title = "THE PLANET";
            body1 = "ZARIMAN expedition record // recovered.";
            body2 = "The crew retrieved an object designated ALL-SPARK from an abandoned alien world.";
            break;

        case 2:
            title = "GRAVITATIONAL EVENT";
            body1 = "Departure failed.";
            body2 = "A gravitational anomaly folded local space. The Zariman was thrown through an unstable wormhole.";
            break;

        case 3:
            title = "INTEGRATION";
            body1 = "ALL-SPARK // SYSTEM ACCESS DETECTED.";
            body2 = "The cube began integrating with vessel systems. Doors opened. Reactors changed state. Robots became hostile.";
            break;

        case 4:
            title = "THE SECOND MIND";
            body1 = "XD-07 has memories that are not machine memories.";
            body2 = "A comatose patient was linked to the maintenance network before the disaster. Something survived inside the chassis.";
            break;

        default:
            title = "SYSTEM LOG";
            body1 = "The Zariman is still alive.";
            body2 = "If XD-07 can restore enough systems, the remaining research can reach home.";
            break;
    }

    DrawRectangle(100, 100, 800, 400, COL_PANEL);
    DrawRectangleLines(100, 100, 800, 400, COL_GREEN);
    DrawText("ZARIMAN // RECOVERY LOG", 135, 135, 16, COL_CYAN);
    DrawText(title, 135, 185, 27, COL_GREEN);
    DrawText(body1, 135, 245, 20, COL_WHITE);
    DrawText(body2, 135, 285, 17, COL_DIM_GREEN);
    DrawText("ENTER TO CONTINUE", 650, 450, 13, COL_GREEN);
}

void DrawEnding()
{
    DrawRectangle(110, 80, 780, 450, COL_PANEL);
    DrawRectangleLines(110, 80, 780, 450, COL_GREEN);

    DrawText("XD-07 // ESCAPE", 160, 125, 32, COL_GREEN);
    DrawText("RESEARCH ARCHIVE SECURED", 160, 190, 18, COL_CYAN);
    DrawText("SURVIVING CREW DATA: RECOVERED", 160, 230, 15, COL_WHITE);
    DrawText("ALL-SPARK STATUS: UNKNOWN", 160, 265, 15, COL_WARNING);
    DrawText("ZARIMAN HOMING BURN: COMPLETE", 160, 300, 15, COL_GREEN);

    DrawLine(160, 350, 840, 350, COL_DARK_GREEN);

    DrawText("A small salvaged craft leaves the wreck behind.", 160, 390, 16, COL_DIM_GREEN);
    DrawText("Inside XD-07, two patterns of thought remain.", 160, 420, 16, COL_DIM_GREEN);
    DrawText("One was built. One was born.", 160, 450, 17, COL_CYAN);

    DrawText("ENTER TO EXIT", 700, 490, 12, COL_GREEN);
}

bool RoomReadyToLeave(int level)
{
    // Early rooms teach the player the loop.
    // Later rooms demand increasing restoration before the door opens.
    if (level == CENTRAL_DECK)
        return AllRoomHazardsRepaired();

    if (level == LOWER_ENGINE)
        return AllRoomHazardsRepaired() && metal >= 2;

    if (level == UPPER_ENGINE)
        return AllRoomHazardsRepaired() && circuits >= 2;

    if (level == WATER_TREATMENT)
        return AllRoomHazardsRepaired() && coolant >= 2;

    if (level == CAFETERIA)
        return AllRoomHazardsRepaired();

    if (level == MEDICAL)
        return AllRoomHazardsRepaired() && botParts >= 2;

    if (level == OXYGEN)
        return AllRoomHazardsRepaired() && powerCells >= 2;

    if (level == COMMUNICATIONS)
        return AllRoomHazardsRepaired() && wiringSolved;

    if (level == ADMIN)
        return AllRoomHazardsRepaired() && data >= 6;

    if (level == ARCHIVE)
        return AllRoomHazardsRepaired() && finalArchiveRecovered;

    return false;
}

const char* ObjectiveForRoom(int level)
{
    if (!AllRoomHazardsRepaired())
        return "Repair the environmental hazards blocking the deck.";

    switch (level)
    {
        case CENTRAL_DECK: return "Stabilize the central deck and reach the lower engine.";
        case LOWER_ENGINE: return "Recover engine salvage and restore the lower reactor.";
        case UPPER_ENGINE: return "Repair power distribution and secure the upper engine.";
        case WATER_TREATMENT: return "Restore coolant flow and collect coolant.";
        case CAFETERIA: return "Clear the mess hall and search for emergency supplies.";
        case MEDICAL: return "Recover XD-series parts from the medical storage bay.";
        case OXYGEN: return "Stabilize oxygen generation and recover power cells.";
        case COMMUNICATIONS: return "Repair the array and reconstruct the corrupted signal.";
        case ADMIN: return "Recover command records and locate the final archive key.";
        case ARCHIVE: return "Recover the ALL-SPARK research package and launch.";
        default: return "Unknown objective.";
    }
}

void RewardRoom(int level)
{
    switch (level)
    {
        case CENTRAL_DECK:
            powerCells++;
            circuits++;
            break;
        case LOWER_ENGINE:
            powerCells++;
            coolant++;
            break;
        case UPPER_ENGINE:
            circuits++;
            powerCells++;
            break;
        case WATER_TREATMENT:
            coolant += 2;
            break;
        case CAFETERIA:
            metal++;
            powerCells++;
            break;
        case MEDICAL:
            botParts++;
            data += 2;
            break;
        case OXYGEN:
            powerCells += 2;
            break;
        case COMMUNICATIONS:
            data += 2;
            circuits++;
            break;
        case ADMIN:
            data += 2;
            break;
        case ARCHIVE:
            data += 4;
            finalArchiveRecovered = true;
            break;
    }
}

// ---------- Game reset ----------
void ResetGame(Player& player, GameState& state, int& level)
{
    player = {
        {150, 300},
        125.0f,
        10.0f, 10.0f,
        5, 5,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1, 25, 25,
        0, 0, 0,
        false, false, false
    };

    state = MENU;
    level = CENTRAL_DECK;

    gameTime = 0.0f;
    statusTimer = 0.0f;
    statusText[0] = '\0';
    damageFlash = 0.0f;
    cameraTrauma = 0.0f;
    transitionTimer = 0.0f;

    selectedTool = TOOL_WRENCH;
    activeHazard = -1;
    dialoguePage = 0;
    selectedCraft = 0;
    mapOpen = false;
    wiringSolved = false;
    finalArchiveRecovered = false;
    escapeReady = false;

    metal = 0;
    circuits = 0;
    powerCells = 0;
    data = 0;
    botParts = 0;
    coolant = 0;

    for (auto& p : particles) p.life = 0;
    for (auto& t : floatTexts) t.active = false;

    SpawnRoomContent(level);
}

// ---------- Main ----------
int main()
{
    InitWindow(SCREEN_W, SCREEN_H, "VOID//SIGNAL");

    // Raylib normally uses ESC as the window close key. Disable that so ESC
    // can be handled by the game menus instead (crafting, repair, pause, etc.).
    SetExitKey(KEY_NULL);

    SetTargetFPS(60);

    Player player;
    GameState state;
    int level;
    ResetGame(player, state, level);

    bool quitRequested = false;

    while (!WindowShouldClose() && !quitRequested)
    {
        float rawDt = GetFrameTime();
        float dt = rawDt;

        gameTime += rawDt;
        statusTimer = std::max(0.0f, statusTimer - rawDt);
        damageFlash = std::max(0.0f, damageFlash - rawDt);
        cameraTrauma = std::max(0.0f, cameraTrauma - rawDt * 2.2f);

        if (state == MENU)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                state = BOOT;
                transitionTimer = 0;
            }

            if (IsKeyPressed(KEY_Q))
                quitRequested = true;
        }
        else if (state == BOOT)
        {
            transitionTimer += dt;
            if (transitionTimer > 3.0f)
            {
                state = DIALOGUE;
                dialoguePage = 0;
            }
        }
        else if (state == DIALOGUE)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                dialoguePage++;
                if (dialoguePage >= 5)
                {
                    state = PLAYING;
                    player.pos = {150, 300};
                    SpawnRoomContent(level);
                }
            }
        }
        else if (state == PLAYING)
        {
            if (IsKeyPressed(KEY_ESCAPE))
            {
                state = PAUSE;
            }

            if (IsKeyPressed(KEY_M))
                mapOpen = !mapOpen;

            if (IsKeyPressed(KEY_C))
            {
                state = CRAFTING;
                selectedCraft = 0;
            }

            if (IsKeyPressed(KEY_TAB))
            {
                selectedTool = (selectedTool + 1) % 4;
                if (selectedTool == TOOL_WELDER && !player.welder) selectedTool++;
                if (selectedTool == TOOL_EXTINGUISHER && !player.extinguisher) selectedTool++;
                if (selectedTool == TOOL_REPAIR_KIT && !player.repairKit) selectedTool = TOOL_WRENCH;
                SetStatus(TextFormat("SELECTED TOOL: %s", ToolName(selectedTool)));
            }

            if (!mapOpen)
            {
                if (player.hurtCooldown > 0) player.hurtCooldown -= dt;
                if (player.meleeCooldown > 0) player.meleeCooldown -= dt;
                if (player.meleeTimer > 0) player.meleeTimer -= dt;

                Vector2 mouse = GetMousePosition();
                player.aimAngle = atan2f(mouse.y - player.pos.y,
                                        mouse.x - player.pos.x);

                Vector2 move = {0, 0};
                if (IsKeyDown(KEY_W)) move.y -= 1;
                if (IsKeyDown(KEY_S)) move.y += 1;
                if (IsKeyDown(KEY_A)) move.x -= 1;
                if (IsKeyDown(KEY_D)) move.x += 1;

                float len = sqrtf(move.x * move.x + move.y * move.y);
                if (len > 0)
                {
                    move.x /= len;
                    move.y /= len;
                }

                float speed = player.speed;

                // Debris slows the robot until cleared.
                for (const auto& h : hazards)
                {
                    if (h.active && !h.repaired && h.type == HAZARD_DEBRIS &&
                        Dist(player.pos, h.pos) < h.radius)
                        speed *= 0.35f;
                }

                // Low energy makes the damaged chassis sluggish.
                if (player.energy < 2.0f)
                    speed *= 0.65f;

                player.pos.x += move.x * speed * dt;
                player.pos.y += move.y * speed * dt;

                player.pos.x = ClampF(player.pos.x, ROOM_LEFT + 25, ROOM_RIGHT - 25);
                player.pos.y = ClampF(player.pos.y, ROOM_TOP + 30, ROOM_BOTTOM - 30);

                player.energy += 0.45f * dt;
                player.energy = ClampF(player.energy, 0, player.maxEnergy);

                UpdateHazards(dt, player);
                UpdateDrones(dt, player);

                if (IsKeyPressed(KEY_F))
                    MeleeAttack(player);

                if (IsKeyPressed(KEY_E))
                {
                    bool handled = false;

                    // Salvage interaction.
                    for (auto& s : salvage)
                    {
                        if (!s.collected && Dist(player.pos, s.pos) < 60)
                        {
                            s.collected = true;
                            metal += s.metal;
                            circuits += s.circuits;
                            powerCells += s.power;
                            data += s.data;
                            botParts += s.botParts;
                            coolant += s.coolant;

                            SetStatus(TextFormat(
                                "SALVAGED: MET %d / CIR %d / PWR %d / DAT %d / BOT %d",
                                s.metal, s.circuits, s.power, s.data, s.botParts));
                            handled = true;
                            break;
                        }
                    }

                    // Environmental repair.
                    if (!handled)
                    {
                        int before = activeHazard;
                        if (RepairNearbyHazard(player))
                        {
                            if (activeHazard != before)
                            {
                                state = REPAIR;
                                handled = true;
                            }
                        }
                    }

                    // Room exit.
                    if (!handled && Dist(player.pos, {930, 310}) < 90)
                    {
                        if (RoomReadyToLeave(level))
                        {
                            RewardRoom(level);

                            if (level < ARCHIVE)
                            {
                                level++;
                                player.pos = {100, 300};
                                SpawnRoomContent(level);
                                SetStatus(TextFormat("DECK ACCESS RESTORED // %s", RoomName(level)));
                            }
                            else
                            {
                                escapeReady = true;
                                SetStatus("ESCAPE CRAFT ARMED // ENTER LAUNCH");
                            }
                            handled = true;
                        }
                        else
                        {
                            SetStatus("ACCESS DENIED // REPAIRS OR SALVAGE REQUIRED");
                            handled = true;
                        }
                    }
                }
            }
        }
        else if (state == CRAFTING)
        {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_C))
                state = PLAYING;

            if (IsKeyPressed(KEY_ONE)) selectedCraft = 0;
            if (IsKeyPressed(KEY_TWO)) selectedCraft = 1;
            if (IsKeyPressed(KEY_THREE)) selectedCraft = 2;
            if (IsKeyPressed(KEY_FOUR)) selectedCraft = 3;
            if (IsKeyPressed(KEY_FIVE)) selectedCraft = 4;
            if (IsKeyPressed(KEY_SIX)) selectedCraft = 5;

            if (IsKeyPressed(KEY_ENTER))
                CraftSelected(player);
        }
        else if (state == REPAIR)
        {
            if (activeHazard < 0 || activeHazard >= 12)
            {
                state = PLAYING;
            }
            else if (IsKeyPressed(KEY_ESCAPE))
            {
                activeHazard = -1;
                state = PLAYING;
            }
            else if (IsKeyDown(KEY_SPACE))
            {
                float marker = (sinf(gameTime * 3.8f) + 1.0f) * 0.5f;

                if (marker > 0.45f && marker < 0.66f)
                {
                    CompleteHazardRepair(player, activeHazard);
                    state = PLAYING;
                }
                else if (GetRandomValue(0, 100) < 4)
                {
                    player.energy = std::max(0.0f, player.energy - 1.0f);
                    SetStatus("REPAIR TIMING MISSED");
                }
            }
        }
        else if (state == PAUSE)
        {
            if (IsKeyPressed(KEY_ESCAPE))
                state = PLAYING;

            if (IsKeyPressed(KEY_M))
                mapOpen = !mapOpen;

            // Restart the entire run from the beginning.
            if (IsKeyPressed(KEY_R))
            {
                ResetGame(player, state, level);
                state = BOOT;
                transitionTimer = 0.0f;
            }

            // Quit cleanly from the pause menu.
            if (IsKeyPressed(KEY_Q))
                quitRequested = true;
        }

        // Archive launch is deliberately separate from the normal door.
        if (state == PLAYING && level == ARCHIVE && escapeReady &&
            IsKeyPressed(KEY_ENTER))
        {
            state = ENDING;
        }

        UpdateParticles(dt);

        BeginDrawing();

        if (state == MENU)
        {
            ClearBackground(COL_BG);
            DrawStarfield();
            DrawParticles();

            DrawText("VOID//SIGNAL", 245, 155, 64, COL_GREEN);
            DrawText("ZARIMAN MAINTENANCE PROTOCOL", 320, 235, 15, COL_CYAN);
            DrawLine(250, 270, 750, 270, COL_DARK_GREEN);
            DrawText("ENTER", 450, 330, 20, COL_GREEN);
            DrawText("BEGIN RECOVERY", 410, 360, 12, COL_DIM_GREEN);
            DrawText("Q QUIT GAME", 425, 395, 12, COL_DANGER);
            DrawText("WASD MOVE   F WRENCH   E REPAIR / SALVAGE", 290, 430, 11, COL_DIM_GREEN);
            DrawText("C CRAFT   TAB TOOLS   M MAP", 350, 455, 11, COL_DIM_GREEN);
            DrawText("THE ZARIMAN IS NOT EMPTY.", 355, 500, 13, COL_WARNING);
        }
        else if (state == BOOT)
        {
            ClearBackground(BLACK);
            DrawText("XD-07", 420, 185, 44, COL_GREEN);
            DrawText("MAINTENANCE UNIT // RECOVERY MODE", 330, 250, 13, COL_DIM_GREEN);
            DrawText(TextFormat("[%02d%%]",
                     (int)ClampF(transitionTimer / 3.0f * 100.0f, 0, 100)),
                     460, 305, 16, COL_CYAN);
            DrawRectangle(300, 350, 400, 8, COL_DARK_METAL);
            DrawRectangle(300, 350,
                          (int)(400 * ClampF(transitionTimer / 3.0f, 0, 1)),
                          8, COL_GREEN);
        }
        else if (state == DIALOGUE)
        {
            ClearBackground(COL_BG);
            DrawStarfield();
            DrawDialogue(dialoguePage);
        }
        else if (state == CRAFTING)
        {
            ClearBackground(COL_BG);
            DrawCrafting(player);
        }
        else if (state == REPAIR)
        {
            ClearBackground(COL_BG);
            if (activeHazard >= 0)
                DrawRepairScreen(hazards[activeHazard], player);
        }
        else if (state == ENDING)
        {
            ClearBackground(BLACK);
            DrawStarfield();
            DrawEnding();
        }
        else
        {
            // World-space camera shake. UI/HUD stays stable while impacts shake
            // the room, robot and particles.
            float shakeAmount = cameraTrauma * 9.0f;
            Camera2D gameCamera = {0};
            gameCamera.offset = {
                shakeAmount > 0 ? (float)GetRandomValue((int)-shakeAmount, (int)shakeAmount) : 0.0f,
                shakeAmount > 0 ? (float)GetRandomValue((int)-shakeAmount, (int)shakeAmount) : 0.0f
            };
            gameCamera.target = {0, 0};
            gameCamera.rotation = 0.0f;
            gameCamera.zoom = 1.0f;

            BeginMode2D(gameCamera);

            DrawRoomBase(level);

            for (const auto& h : hazards) DrawHazard(h);
            for (const auto& s : salvage) DrawSalvage(s);
            for (const auto& d : drones) DrawDrone(d);

            DrawRobot(player);
            DrawParticles();

            EndMode2D();

            DrawHUD(player, level);

            const char* objective = ObjectiveForRoom(level);
            DrawText(objective, 65, 575, 11, COL_WHITE);

            bool nearInteract = false;

            for (const auto& s : salvage)
                if (!s.collected && Dist(player.pos, s.pos) < 60)
                    nearInteract = true;

            for (const auto& h : hazards)
                if (h.active && !h.repaired && Dist(player.pos, h.pos) < 75)
                    nearInteract = true;

            if (Dist(player.pos, {930, 310}) < 90)
                nearInteract = true;

            if (nearInteract && statusTimer <= 0)
                DrawText("[E] INTERACT", 810, 525, 11, COL_CYAN);

            if (level == ARCHIVE && escapeReady)
            {
                DrawText("[ENTER] LAUNCH SALVAGE CRAFT", 665, 90, 12, COL_GREEN);
            }

            if (damageFlash > 0)
                DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                              WithAlpha(COL_DANGER,
                                        (unsigned char)(damageFlash * 110)));

            if (mapOpen)
                DrawMap(level);

            if (state == PAUSE)
            {
                DrawRectangle(0, 0, SCREEN_W, SCREEN_H, WithAlpha(BLACK, 175));
                DrawText("PAUSED", 405, 185, 42, COL_GREEN);
                DrawText("ESC RESUME", 420, 255, 15, COL_DIM_GREEN);
                DrawText("C CRAFT", 440, 285, 15, COL_WARNING);
                DrawText("M MAP", 445, 315, 15, COL_CYAN);
                DrawText("R RESTART GAME", 410, 345, 15, COL_WARNING);
                DrawText("Q QUIT GAME", 425, 375, 15, COL_DANGER);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
