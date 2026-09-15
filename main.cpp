#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <string>
#include <cmath>

// --- ENUMS & STRUCTS ---
enum WeaponType { PISTOL = 0, SHOTGUN = 1, RIFLE = 2 };

struct Player {
    Vector2 pos;
    Vector2 vel;
    float speed;
    int health;
    int maxHealth;
    float shield;
    float maxShield;
    float shieldRegenTimer;
    float energy;
    float maxEnergy;
    WeaponType weapon;
    int ammo[3];
    int maxAmmo[3];
    float shootCooldown;
    float reloadTimer;
    float aimAngle;
    int sentienceExp;
};

struct Projectile {
    Vector2 pos;
    Vector2 vel;
    float damage;
    bool isEnemy;
    float life;
    bool active;
    Color color;
};

struct Drone {
    Vector2 pos;
    float speed;
    int health;
    int maxHealth;
    float shootTimer;
    bool active;
};

struct Inventory {
    int scrapMetal;
    int powerCells;
    int circuits;
    int alienFragments;
    int researchDisks;
    bool hasPlasmaTorch;
    bool hasShieldGenerator;
    bool hasImprovedServos;
    bool hasExtinguisher;
    bool hasRadiationDampener;
};

struct HullBreach {
    Vector2 pos;
    float radius;
    float suction;
    bool isPatched;
};

struct ElectricFire {
    Vector2 pos;
    float radius;
    bool isExtinguished;
};

// --- MAIN GAME ---
int main() {
    const int screenWidth = 1000;
    const int screenHeight = 650;
    InitWindow(screenWidth, screenHeight, "VOID//SIGNAL: Singularity Breach [C++ / Raylib]");
    InitAudioDevice();
    SetTargetFPS(60);

    // Initialize Player (Unit AX-07)
    Player player = {
        { 200.0f, 300.0f }, { 0.0f, 0.0f }, 200.0f,
        5, 5, 0.0f, 4.0f, 0.0f, 10.0f, 10.0f,
        PISTOL, { 12, 6, 30 }, { 12, 6, 30 },
        0.0f, 0.0f, 0.0f, 10
    };

    Inventory inv = { 4, 3, 2, 2, 1, false, false, false, false, false };

    // Environmental Hazards
    HullBreach breach = { { 750.0f, 200.0f }, 38.0f, 240.0f, false };
    ElectricFire fire = { { 620.0f, 400.0f }, 32.0f, false };
    Vector2 radZone = { 700.0f, 260.0f };
    float radRadius = 100.0f;

    // Entities
    std::vector<Projectile> projectiles;
    std::vector<Drone> drones;
    drones.push_back({ { 720.0f, 180.0f }, 45.0f, 3, 3, 2.0f, true });
    drones.push_back({ { 820.0f, 420.0f }, 40.0f, 3, 3, 3.0f, true });

    float singularityDist = 50000.0f;
    std::string statusMsg = "UNIT AX-07 AWAKENED: SINGULARITY PROXIMITY CRITICAL";

    // Room Bulkhead Boundaries
    const Rectangle room = { 50, 70, 900, 480 };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // 1. --- PLAYER MOVEMENT (Acceleration & Damping) ---
        Vector2 input = { 0, 0 };
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y -= 1;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y += 1;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x -= 1;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x += 1;

        if (Vector2Length(input) > 0) input = Vector2Normalize(input);

        float maxSpeed = inv.hasImprovedServos ? 270.0f : 190.0f;
        float accelRate = inv.hasImprovedServos ? 24.0f : 14.0f;

        Vector2 targetVel = Vector2Scale(input, maxSpeed);
        player.vel.x += (targetVel.x - player.vel.x) * fminf(1.0f, accelRate * dt);
        player.vel.y += (targetVel.y - player.vel.y) * fminf(1.0f, accelRate * dt);

        // Vacuum Suction from Unpatched Hull Breach
        if (!breach.isPatched) {
            Vector2 toBreach = Vector2Subtract(breach.pos, player.pos);
            float dist = Vector2Length(toBreach);
            if (dist < 260.0f && dist > 15.0f) {
                float pull = (breach.suction / (dist + 50.0f)) * 140.0f * dt;
                player.vel = Vector2Add(player.vel, Vector2Scale(Vector2Normalize(toBreach), pull));
            }
        }

        player.pos = Vector2Add(player.pos, Vector2Scale(player.vel, dt));

        // Wall collisions
        player.pos.x = Clamp(player.pos.x, room.x + 20, room.x + room.width - 20);
        player.pos.y = Clamp(player.pos.y, room.y + 20, room.y + room.height - 20);

        // Aiming
        Vector2 mouse = GetMousePosition();
        player.aimAngle = atan2f(mouse.y - player.pos.y, mouse.x - player.pos.x);

        // 2. --- CRAFTING SHORTCUTS ([C] to Fabricate Upgrades) ---
        if (IsKeyPressed(KEY_C)) {
            if (!inv.hasShieldGenerator && inv.scrapMetal >= 2 && inv.powerCells >= 1) {
                inv.scrapMetal -= 2; inv.powerCells -= 1;
                inv.hasShieldGenerator = true;
                player.shield = 4.0f;
                statusMsg = "CRAFTED: DEFLECTOR SHIELD GENERATOR [ONLINE]";
            } else if (!inv.hasPlasmaTorch && inv.scrapMetal >= 3 && inv.circuits >= 1) {
                inv.scrapMetal -= 3; inv.circuits -= 1;
                inv.hasPlasmaTorch = true;
                statusMsg = "CRAFTED: PLASMA CUTTER TORCH [WELD/CUT READY]";
            } else if (!inv.hasImprovedServos && inv.scrapMetal >= 2 && inv.circuits >= 2) {
                inv.scrapMetal -= 2; inv.circuits -= 2;
                inv.hasImprovedServos = true;
                statusMsg = "CRAFTED: OVERCLOCKED TREAD SERVOS (+35% SPEED)";
            } else if (!inv.hasExtinguisher && inv.scrapMetal >= 1 && inv.powerCells >= 1) {
                inv.scrapMetal -= 1; inv.powerCells -= 1;
                inv.hasExtinguisher = true;
                statusMsg = "CRAFTED: CRYO-FOAM SUPPRESSANT [USE WITH Q]";
            } else {
                statusMsg = "CRAFTING MATRIX: INSUFFICIENT SALVAGE ALLOY";
            }
        }

        // 3. --- ACTIONS & INTERACTIONS ---
        // Extinguisher [Q]
        if (IsKeyPressed(KEY_Q) && inv.hasExtinguisher) {
            float distFire = Vector2Distance(player.pos, fire.pos);
            if (distFire < 180.0f && !fire.isExtinguished) {
                fire.isExtinguished = true;
                inv.scrapMetal += 1;
                statusMsg = "FIRE SMOTHERED WITH CRYO-FOAM (+1 SCRAP)";
            }
        }

        // Interact [E] (Weld Breach)
        if (IsKeyPressed(KEY_E)) {
            float distBreach = Vector2Distance(player.pos, breach.pos);
            if (distBreach < 90.0f && !breach.isPatched) {
                if (inv.hasPlasmaTorch || inv.scrapMetal >= 1) {
                    breach.isPatched = true;
                    if (!inv.hasPlasmaTorch) inv.scrapMetal--;
                    player.sentienceExp += 40;
                    statusMsg = "HULL BREACH RESEALED // VACUUM SUCTION HALTED";
                } else {
                    statusMsg = "REQUIRES PLASMA TORCH OR 1 SCRAP TO WELD";
                }
            }
        }

        // Weapon Switching (1, 2, 3)
        if (IsKeyPressed(KEY_ONE)) player.weapon = PISTOL;
        if (IsKeyPressed(KEY_TWO)) player.weapon = SHOTGUN;
        if (IsKeyPressed(KEY_THREE)) player.weapon = RIFLE;

        // Reload [R]
        if (IsKeyPressed(KEY_R) && player.reloadTimer <= 0) {
            player.reloadTimer = 1.0f;
            statusMsg = "RELOADING WEAPON...";
        }

        if (player.reloadTimer > 0) {
            player.reloadTimer -= dt;
            if (player.reloadTimer <= 0) player.ammo[player.weapon] = player.maxAmmo[player.weapon];
        }

        // Shooting (Left Mouse)
        player.shootCooldown -= dt;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && player.shootCooldown <= 0 && player.reloadTimer <= 0) {
            if (player.ammo[player.weapon] > 0) {
                player.ammo[player.weapon]--;
                player.shootCooldown = (player.weapon == RIFLE) ? 0.12f : (player.weapon == SHOTGUN) ? 0.7f : 0.28f;

                int pellets = (player.weapon == SHOTGUN) ? 4 : 1;
                for (int i = 0; i < pellets; i++) {
                    float spread = (player.weapon == SHOTGUN) ? ((float)GetRandomValue(-15, 15) * DEG2RAD) : 0.0f;
                    Vector2 dir = { cosf(player.aimAngle + spread), sinf(player.aimAngle + spread) };
                    projectiles.push_back({ player.pos, Vector2Scale(dir, 480.0f), 1.0f, false, 1.4f, true, SKYBLUE });
                }
            }
        }

        // Shield Recharge
        if (inv.hasShieldGenerator) {
            if (player.shieldRegenTimer > 0) player.shieldRegenTimer -= dt;
            else if (player.shield < player.maxShield) player.shield += dt * 0.8f;
        }

        // Singularity Timer
        singularityDist = fmaxf(0.0f, singularityDist - (50000.0f / 360.0f) * dt);

        // 4. --- PROJECTILE SIMULATION ---
        for (auto& p : projectiles) {
            if (!p.active) continue;
            p.pos = Vector2Add(p.pos, Vector2Scale(p.vel, dt));
            p.life -= dt;
            if (p.life <= 0) p.active = false;

            // Hit drones
            if (!p.isEnemy) {
                for (auto& d : drones) {
                    if (d.active && CheckCollisionCircles(p.pos, 4, d.pos, 18)) {
                        p.active = false;
                        d.health -= (int)p.damage;
                        if (d.health <= 0) {
                            d.active = false;
                            inv.scrapMetal += 2;
                            player.sentienceExp += 25;
                            statusMsg = "CORRUPTED DRONE DESTROYED (+2 SCRAP, +25 EXP)";
                        }
                    }
                }
            }
        }

        // --- DRAWING ---
        BeginDrawing();
        ClearBackground(GetColor(0x050a08ff));

        // Deck Floor & Boundary
        DrawRectangleRec(room, GetColor(0x0a1612ff));
        DrawRectangleLinesEx(room, 3, GetColor(0x1a4030ff));

        // Radiation Hazard Hotspot
        DrawCircleV(radZone, radRadius, ColorAlpha(LIME, 0.12f));
        DrawCircleLines((int)radZone.x, (int)radZone.y, radRadius, ColorAlpha(LIME, 0.4f));
        DrawText("RAD HAZARD ZONE", (int)radZone.x - 55, (int)radZone.y - 10, 12, LIME);

        // Hull Breach
        if (!breach.isPatched) {
            DrawCircleV(breach.pos, breach.radius, BLACK);
            DrawCircleLines((int)breach.pos.x, (int)breach.pos.y, breach.radius, RED);
            DrawText("! HULL BREACH [E] WELD !", (int)breach.pos.x - 65, (int)breach.pos.y + 44, 11, RED);
        } else {
            DrawRectangle((int)breach.pos.x - 24, (int)breach.pos.y - 24, 48, 48, DARKGRAY);
            DrawRectangleLines((int)breach.pos.x - 24, (int)breach.pos.y - 24, 48, 48, GREEN);
            DrawText("SEALED", (int)breach.pos.x - 18, (int)breach.pos.y - 4, 10, GREEN);
        }

        // Electrical Fire
        if (!fire.isExtinguished) {
            DrawCircleV(fire.pos, fire.radius, ORANGE);
            DrawCircleV(fire.pos, fire.radius * 0.6f, YELLOW);
            DrawText("ELECTRICAL FIRE [Q]", (int)fire.pos.x - 55, (int)fire.pos.y + 36, 11, ORANGE);
        } else {
            DrawCircleV(fire.pos, 14, DARKBLUE);
            DrawText("COOLED", (int)fire.pos.x - 20, (int)fire.pos.y + 18, 10, SKYBLUE);
        }

        // Drones
        for (const auto& d : drones) {
            if (d.active) {
                DrawCircleV(d.pos, 18, DARKGRAY);
                DrawCircleLines((int)d.pos.x, (int)d.pos.y, 18, RED);
                DrawCircleV(d.pos, 5, RED);
            }
        }

        // Projectiles
        for (const auto& p : projectiles) {
            if (p.active) DrawCircleV(p.pos, 4, p.color);
        }

        // Robot Player (Unit AX-07)
        DrawRectangle((int)player.pos.x - 16, (int)player.pos.y - 14, 32, 28, GetColor(0x142820ff));
        DrawRectangleLines((int)player.pos.x - 16, (int)player.pos.y - 14, 32, 28, GREEN);
        DrawCircleV(player.pos, 4, SKYBLUE);

        // Aim line
        Vector2 barrelEnd = { player.pos.x + cosf(player.aimAngle) * 26, player.pos.y + sinf(player.aimAngle) * 26 };
        DrawLineEx(player.pos, barrelEnd, 3, GREEN);

        // Deflector Shield Bubble
        if (inv.hasShieldGenerator && player.shield > 0.0f) {
            DrawCircleLines((int)player.pos.x, (int)player.pos.y, 28, ColorAlpha(SKYBLUE, 0.7f));
        }

        // --- HUD OVERLAY ---
        DrawRectangle(0, 0, screenWidth, 48, GetColor(0x030806ee));
        DrawText("UNIT AX-07 // VOID PROTOCOL", 20, 14, 16, GREEN);

        DrawText(TextFormat("HP: %d/%d", player.health, player.maxHealth), 320, 14, 14, RED);
        if (inv.hasShieldGenerator) {
            DrawText(TextFormat("SHIELD: %.1f/%.1f", player.shield, player.maxShield), 410, 14, 14, SKYBLUE);
        }

        DrawText(TextFormat("AMMO: %d/%d", player.ammo[player.weapon], player.maxAmmo[player.weapon]), 570, 14, 14, YELLOW);
        DrawText(TextFormat("SINGULARITY: %.0f KM", singularityDist), 730, 14, 14, RED);

        // Bottom Bar
        DrawRectangle(0, screenHeight - 40, screenWidth, 40, GetColor(0x030806ee));
        DrawText(TextFormat("SALVAGE: Scrap: %d | Cells: %d | Circuits: %d | [C] Craft Upgrades",
            inv.scrapMetal, inv.powerCells, inv.circuits), 20, screenHeight - 26, 13, RAYWHITE);
        DrawText(statusMsg.c_str(), 520, screenHeight - 26, 12, GREEN);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}