#include "raylib.h"
#include <cmath>
#include <cstdio>

// ============================================================
// VOID//SIGNAL
// Procedural 2D sci-fi exploration demo
//
// CONTROLS
// WASD  - Move
// E     - Interact / Scavenge
// ENTER - Continue / Submit
// 0-9   - Cipher input
// ESC   - Pause / Close
// ============================================================

float ClampFloat(float value, float minValue, float maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

enum GameState
{
    MENU,
    BOOT,
    PLAYING,
    TERMINAL,
    CIPHER,
    DIALOGUE,
    PAUSE,
    ENDING
};

struct Player
{
    Vector2 pos;
    int facing;
    float energy;
};

static float Distance(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;

    return sqrtf(dx * dx + dy * dy);
}

static void DrawCentered(const char *text, int y, int size, Color color)
{
    int width = MeasureText(text, size);

    DrawText(
        text,
        (1000 - width) / 2,
        y,
        size,
        color
    );
}

static void DrawPanel(int x, int y, int w, int h)
{
    DrawRectangle(
        x,
        y,
        w,
        h,
        Color{5, 12, 14, 245}
    );

    DrawRectangleLines(
        x,
        y,
        w,
        h,
        GREEN
    );

    DrawLine(
        x + 15,
        y + 45,
        x + w - 15,
        y + 45,
        DARKGREEN
    );
}

// ============================================================
// ROBOT
// ============================================================

static void DrawRobot(
    Vector2 p,
    int facing,
    float time
)
{
    // Shadow
    DrawEllipse(
        (int)p.x,
        (int)p.y + 23,
        24,
        8,
        Fade(BLACK, 0.5f)
    );

    // Body
    DrawRectangle(
        (int)p.x - 18,
        (int)p.y - 15,
        36,
        38,
        Color{90, 100, 102, 255}
    );

    // Head
    DrawRectangle(
        (int)p.x - 20,
        (int)p.y - 35,
        40,
        25,
        Color{125, 135, 135, 255}
    );

    // Head outline
    DrawRectangleLines(
        (int)p.x - 20,
        (int)p.y - 35,
        40,
        25,
        GREEN
    );

    // Eye blink
    bool blinking =
        fmodf(time, 4.0f) > 3.82f;

    if (!blinking)
    {
        DrawRectangle(
            (int)p.x + facing * 7 - 3,
            (int)p.y - 28,
            7,
            5,
            GREEN
        );

        DrawCircle(
            (int)p.x + facing * 7,
            (int)p.y - 26,
            8,
            Fade(GREEN, 0.12f)
        );
    }

    // Antenna
    DrawLine(
        (int)p.x,
        (int)p.y - 35,
        (int)p.x,
        (int)p.y - 45,
        GREEN
    );

    DrawCircle(
        (int)p.x,
        (int)p.y - 48,
        3,
        GREEN
    );

    // Legs
    DrawRectangle(
        (int)p.x - 15,
        (int)p.y + 20,
        9,
        12,
        DARKGRAY
    );

    DrawRectangle(
        (int)p.x + 6,
        (int)p.y + 20,
        9,
        12,
        DARKGRAY
    );
}

// ============================================================
// STARFIELD
// ============================================================

static void DrawStars(float time)
{
    for (int i = 0; i < 80; i++)
    {
        float x = (i * 137) % 1000;
        float y = (i * 71) % 600;

        float twinkle =
            0.35f +
            0.35f *
            sinf(time * 2.0f + i);

        DrawCircle(
            (int)x,
            (int)y,
            (i % 3 == 0) ? 2 : 1,
            Fade(LIGHTGRAY, twinkle)
        );
    }
}

// ============================================================
// BLACK HOLE
// ============================================================

static void DrawBlackHole(
    int x,
    int y,
    float time
)
{
    float pulse =
        (sinf(time * 1.5f) + 1.0f) * 0.5f;

    DrawCircle(
        x,
        y,
        100 + pulse * 10,
        Fade(ORANGE, 0.035f)
    );

    DrawCircle(
        x,
        y,
        80 + pulse * 8,
        Fade(ORANGE, 0.06f)
    );

    DrawCircle(
        x,
        y,
        58 + pulse * 5,
        Fade(ORANGE, 0.10f)
    );

    for (int i = 0; i < 3; i++)
    {
        float radius =
            45 + i * 12;

        float angle =
            time * (0.7f + i * 0.2f);

        Vector2 a =
        {
            x + cosf(angle) * radius,
            y + sinf(angle) * radius * 0.45f
        };

        Vector2 b =
        {
            x + cosf(angle + PI) * radius,
            y + sinf(angle + PI) * radius * 0.45f
        };

        DrawLineEx(
            a,
            b,
            3,
            Fade(ORANGE, 0.55f)
        );
    }

    DrawCircle(
        x,
        y,
        34,
        BLACK
    );
}

// ============================================================
// TERMINAL
// ============================================================

static void DrawTerminal(
    Vector2 p,
    float time
)
{
    DrawRectangle(
        (int)p.x - 35,
        (int)p.y - 50,
        70,
        100,
        Color{35, 42, 43, 255}
    );

    DrawRectangle(
        (int)p.x - 27,
        (int)p.y - 38,
        54,
        38,
        BLACK
    );

    DrawRectangleLines(
        (int)p.x - 27,
        (int)p.y - 38,
        54,
        38,
        GREEN
    );

    int lines =
        2 + ((int)(time * 2) % 3);

    for (int i = 0; i < lines; i++)
    {
        DrawLine(
            (int)p.x - 22,
            (int)p.y - 30 + i * 8,
            (int)p.x + 15,
            (int)p.y - 30 + i * 8,
            Fade(GREEN, 0.6f)
        );
    }
}

// ============================================================
// CACHE
// ============================================================

static void DrawCache(Vector2 p)
{
    DrawRectangle(
        (int)p.x - 28,
        (int)p.y - 20,
        56,
        40,
        Color{115, 75, 35, 255}
    );

    DrawRectangleLines(
        (int)p.x - 28,
        (int)p.y - 20,
        56,
        40,
        ORANGE
    );

    DrawLine(
        (int)p.x - 20,
        (int)p.y,
        (int)p.x + 20,
        (int)p.y,
        ORANGE
    );
}

// ============================================================
// SCRAP
// ============================================================

static void DrawScrap(Vector2 p)
{
    DrawCircle(
        (int)p.x,
        (int)p.y,
        20,
        GRAY
    );

    DrawCircle(
        (int)p.x - 15,
        (int)p.y + 5,
        10,
        DARKGRAY
    );

    DrawRectangle(
        (int)p.x + 8,
        (int)p.y - 18,
        8,
        30,
        LIGHTGRAY
    );
}

// ============================================================
// DOOR
// ============================================================

static void DrawDoor(
    Vector2 p,
    bool open
)
{
    DrawRectangle(
        (int)p.x - 25,
        (int)p.y - 70,
        50,
        140,
        open ? DARKGREEN : MAROON
    );

    DrawRectangleLines(
        (int)p.x - 25,
        (int)p.y - 70,
        50,
        140,
        open ? GREEN : RED
    );

    DrawText(
        open ? "OPEN" : "LOCK",
        (int)p.x - 18,
        (int)p.y - 5,
        10,
        WHITE
    );
}

// ============================================================
// HUD
// ============================================================

static void DrawHUD(
    int level,
    int power,
    int circuits,
    int metal,
    int data
)
{
    DrawRectangle(
        0,
        0,
        1000,
        65,
        Color{3, 8, 9, 245}
    );

    DrawText(
        TextFormat(
            "VOID//SIGNAL   //   LEVEL %d",
            level
        ),
        25,
        20,
        22,
        GREEN
    );

    DrawText(
        TextFormat(
            "PWR %d   CIR %d   MET %d   DATA %d",
            power,
            circuits,
            metal,
            data
        ),
        610,
        23,
        16,
        GREEN
    );
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    const int screenWidth = 1000;
    const int screenHeight = 600;

    InitWindow(
        screenWidth,
        screenHeight,
        "VOID//SIGNAL"
    );

    SetTargetFPS(60);

    GameState state = MENU;

    Player player =
    {
        {150, 330},
        1,
        100
    };

    int level = 1;

    int power = 0;
    int circuits = 0;
    int metal = 0;
    int data = 0;

    int cipherInput = 0;
    int cipherAnswer = 7;

    int dialoguePage = 0;

    bool cache1 = false;
    bool cache2 = false;
    bool cache3 = false;
    bool cache4 = false;
    bool cache5 = false;

    bool scrap1 = false;
    bool scrap2 = false;
    bool scrap3 = false;

    bool terminal1 = false;
    bool terminal2 = false;
    bool terminal3 = false;

    bool door1 = false;
    bool door2 = false;
    bool door3 = false;
    bool door4 = false;

    bool gameComplete = false;

    float stateTimer = 0;
    float shake = 0;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float time = GetTime();

        stateTimer += dt;

        // ====================================================
        // MENU
        // ====================================================

        if (state == MENU)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                state = BOOT;
                stateTimer = 0;
            }
        }

        // ====================================================
        // BOOT
        // ====================================================

        else if (state == BOOT)
        {
            if (
                IsKeyPressed(KEY_ENTER) ||
                stateTimer > 9.0f
            )
            {
                state = DIALOGUE;
                dialoguePage = 0;
                stateTimer = 0;
            }
        }

        // ====================================================
        // PLAYING
        // ====================================================

        else if (state == PLAYING)
        {
            if (IsKeyPressed(KEY_ESCAPE))
            {
                state = PAUSE;
            }

            Vector2 oldPos = player.pos;

            if (IsKeyDown(KEY_W))
                player.pos.y -= 190 * dt;

            if (IsKeyDown(KEY_S))
                player.pos.y += 190 * dt;

            if (IsKeyDown(KEY_A))
            {
                player.pos.x -= 190 * dt;
                player.facing = -1;
            }

            if (IsKeyDown(KEY_D))
            {
                player.pos.x += 190 * dt;
                player.facing = 1;
            }

            // Keep player inside room
            player.pos.x =
                ClampFloat(
                    player.pos.x,
                    80.0f,
                    920.0f
                );

            player.pos.y =
                ClampFloat(
                    player.pos.y,
                    100.0f,
                    500.0f
                );

            player.energy -= dt * 0.35f;

            if (player.energy < 0)
                player.energy = 0;

            // =================================================
            // LEVEL 1
            // =================================================

            if (level == 1)
            {
                Vector2 terminal =
                {
                    700,
                    260
                };

                Vector2 cache =
                {
                    760,
                    400
                };

                Vector2 scrap =
                {
                    330,
                    420
                };

                if (
                    Distance(player.pos, terminal) < 80 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    terminal1 = true;
                    state = TERMINAL;
                }

                if (
                    Distance(player.pos, cache) < 80 &&
                    !cache1 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    cipherAnswer = 7;
                    cipherInput = 0;
                    state = CIPHER;
                }

                if (
                    Distance(player.pos, scrap) < 70 &&
                    !scrap1 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    scrap1 = true;
                    metal += 2;
                    shake = 0.2f;
                }

                if (
                    player.pos.x > 850 &&
                    player.pos.y > 230 &&
                    player.pos.y < 390 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    if (
                        cache1 &&
                        power >= 1 &&
                        circuits >= 1 &&
                        metal >= 2
                    )
                    {
                        door1 = true;

                        level = 2;

                        player.pos =
                        {
                            130,
                            330
                        };

                        state = DIALOGUE;
                        dialoguePage = 1;
                    }
                }
            }

            // =================================================
            // LEVEL 2
            // =================================================

            else if (level == 2)
            {
                Vector2 terminal =
                {
                    240,
                    180
                };

                Vector2 cache =
                {
                    700,
                    180
                };

                Vector2 scrap =
                {
                    350,
                    430
                };

                if (
                    Distance(player.pos, terminal) < 80 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    terminal2 = true;
                    state = TERMINAL;
                }

                if (
                    Distance(player.pos, cache) < 80 &&
                    !cache2 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    cipherAnswer = 4;
                    cipherInput = 0;
                    state = CIPHER;
                }

                if (
                    Distance(player.pos, scrap) < 70 &&
                    !scrap2 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    scrap2 = true;
                    metal += 2;
                }

                if (
                    player.pos.x > 850 &&
                    player.pos.y > 250 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    if (
                        cache2 &&
                        circuits >= 2 &&
                        metal >= 4
                    )
                    {
                        door2 = true;

                        level = 3;

                        player.pos =
                        {
                            130,
                            330
                        };

                        state = DIALOGUE;
                        dialoguePage = 2;
                    }
                }
            }

            // =================================================
            // LEVEL 3
            // =================================================

            else if (level == 3)
            {
                Vector2 terminal =
                {
                    250,
                    170
                };

                Vector2 cache =
                {
                    750,
                    170
                };

                Vector2 scrap =
                {
                    350,
                    430
                };

                if (
                    Distance(player.pos, terminal) < 80 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    terminal3 = true;
                    state = TERMINAL;
                }

                if (
                    Distance(player.pos, cache) < 80 &&
                    !cache3 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    cipherAnswer = 6;
                    cipherInput = 0;
                    state = CIPHER;
                }

                if (
                    Distance(player.pos, scrap) < 70 &&
                    !scrap3 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    scrap3 = true;
                    metal += 2;
                }

                if (
                    player.pos.x > 850 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    if (
                        cache3 &&
                        data >= 1
                    )
                    {
                        door3 = true;

                        level = 4;

                        player.pos =
                        {
                            130,
                            330
                        };

                        state = DIALOGUE;
                        dialoguePage = 3;
                    }
                }
            }

            // =================================================
            // LEVEL 4
            // =================================================

            else if (level == 4)
            {
                Vector2 cache =
                {
                    700,
                    180
                };

                Vector2 terminal =
                {
                    300,
                    420
                };

                if (
                    Distance(player.pos, cache) < 80 &&
                    !cache4 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    cipherAnswer = 3;
                    cipherInput = 0;
                    state = CIPHER;
                }

                if (
                    Distance(player.pos, terminal) < 80 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    state = TERMINAL;
                }

                // Gravity distortion
                player.pos.x +=
                    sinf(time * 2.0f) *
                    8.0f *
                    dt;

                if (
                    player.pos.x > 850 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    if (cache4)
                    {
                        door4 = true;

                        level = 5;

                        player.pos =
                        {
                            130,
                            330
                        };

                        state = DIALOGUE;
                        dialoguePage = 4;
                    }
                }
            }

            // =================================================
            // LEVEL 5
            // =================================================

            else if (level == 5)
            {
                Vector2 cache =
                {
                    700,
                    180
                };

                Vector2 escapePod =
                {
                    850,
                    330
                };

                if (
                    Distance(player.pos, cache) < 80 &&
                    !cache5 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    cipherAnswer = 9;
                    cipherInput = 0;
                    state = CIPHER;
                }

                if (
                    Distance(player.pos, escapePod) < 100 &&
                    IsKeyPressed(KEY_E)
                )
                {
                    if (
                        cache5 &&
                        power >= 1 &&
                        circuits >= 3 &&
                        metal >= 6 &&
                        data >= 2
                    )
                    {
                        gameComplete = true;
                        state = ENDING;
                        stateTimer = 0;
                    }
                }
            }

            if (player.energy <= 10)
                player.energy = 100;
        }

        // ====================================================
        // TERMINAL
        // ====================================================

        else if (state == TERMINAL)
        {
            if (IsKeyPressed(KEY_ESCAPE))
            {
                state = PLAYING;
            }

            if (
                IsKeyPressed(KEY_E) ||
                IsKeyPressed(KEY_ENTER)
            )
            {
                if (level == 1)
                {
                    power = 1;
                    data = 1;
                }
                else if (level == 2)
                {
                    circuits++;
                }
                else if (level == 3)
                {
                    data++;
                }
                else if (level == 4)
                {
                    power = 1;
                }

                state = PLAYING;
            }
        }

        // ====================================================
        // CIPHER
        // ====================================================

        else if (state == CIPHER)
        {
            if (IsKeyPressed(KEY_ZERO))
                cipherInput = 0;

            if (IsKeyPressed(KEY_ONE))
                cipherInput = 1;

            if (IsKeyPressed(KEY_TWO))
                cipherInput = 2;

            if (IsKeyPressed(KEY_THREE))
                cipherInput = 3;

            if (IsKeyPressed(KEY_FOUR))
                cipherInput = 4;

            if (IsKeyPressed(KEY_FIVE))
                cipherInput = 5;

            if (IsKeyPressed(KEY_SIX))
                cipherInput = 6;

            if (IsKeyPressed(KEY_SEVEN))
                cipherInput = 7;

            if (IsKeyPressed(KEY_EIGHT))
                cipherInput = 8;

            if (IsKeyPressed(KEY_NINE))
                cipherInput = 9;

            if (IsKeyPressed(KEY_ENTER))
            {
                if (cipherInput == cipherAnswer)
                {
                    if (level == 1)
                    {
                        cache1 = true;
                        power = 1;
                        circuits = 1;
                    }
                    else if (level == 2)
                    {
                        cache2 = true;
                        circuits++;
                    }
                    else if (level == 3)
                    {
                        cache3 = true;
                        data++;
                    }
                    else if (level == 4)
                    {
                        cache4 = true;
                        data++;
                    }
                    else if (level == 5)
                    {
                        cache5 = true;
                        circuits++;
                    }

                    state = PLAYING;
                }
                else
                {
                    cipherInput = 0;
                    shake = 0.3f;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE))
                state = PLAYING;
        }

        // ====================================================
        // DIALOGUE
        // ====================================================

        else if (state == DIALOGUE)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                dialoguePage++;

                if (dialoguePage >= 5)
                {
                    state = PLAYING;
                    stateTimer = 0;
                }
            }
        }

        // ====================================================
        // PAUSE
        // ====================================================

        else if (state == PAUSE)
        {
            if (IsKeyPressed(KEY_ESCAPE))
                state = PLAYING;
        }

        // ====================================================
        // ENDING
        // ====================================================

        else if (state == ENDING)
        {
            if (
                IsKeyPressed(KEY_ENTER) &&
                stateTimer > 5
            )
            {
                state = MENU;

                level = 1;

                power = 0;
                circuits = 0;
                metal = 0;
                data = 0;

                cache1 = false;
                cache2 = false;
                cache3 = false;
                cache4 = false;
                cache5 = false;

                scrap1 = false;
                scrap2 = false;
                scrap3 = false;

                gameComplete = false;

                player.pos =
                {
                    150,
                    330
                };

                player.facing = 1;
                player.energy = 100;

                stateTimer = 0;
            }
        }

        // ====================================================
        // SCREEN SHAKE
        // ====================================================

        if (shake > 0)
            shake -= dt;

        float sx = 0;
        float sy = 0;

        if (shake > 0)
        {
            sx =
                sinf(time * 50) * 4;

            sy =
                cosf(time * 45) * 4;
        }

        // ====================================================
        // DRAW
        // ====================================================

        BeginDrawing();

        ClearBackground(BLACK);

        // ====================================================
        // MENU
        // ====================================================

        if (state == MENU)
        {
            DrawStars(time);

            DrawBlackHole(
                760,
                230,
                time
            );

            DrawCentered(
                "VOID//SIGNAL",
                150,
                70,
                GREEN
            );

            DrawCentered(
                "A CREWSHIP SURVIVAL STORY",
                235,
                18,
                DARKGREEN
            );

            DrawCentered(
                "UNIT 07 // RECOVERY SYSTEM",
                290,
                16,
                GREEN
            );

            if (
                ((int)(time * 2)) % 2 == 0
            )
            {
                DrawCentered(
                    "[ ENTER ]",
                    390,
                    25,
                    GREEN
                );
            }

            DrawCentered(
                "A SIGNAL IS STILL WAITING.",
                500,
                15,
                GRAY
            );
        }

        // ====================================================
        // BOOT
        // ====================================================

        else if (state == BOOT)
        {
            DrawStars(time);

            DrawBlackHole(
                760,
                230,
                time
            );

            DrawText(
                "CREWSHIP OS v4.7",
                60,
                60,
                30,
                GREEN
            );

            DrawText(
                "UNIT 07 // CORE RECOVERY",
                60,
                110,
                18,
                GREEN
            );

            float progress =
                ClampFloat(
                    stateTimer / 9.0f,
                    0.0f,
                    1.0f
                );

            DrawRectangle(
                60,
                160,
                500,
                18,
                DARKGREEN
            );

            DrawRectangle(
                60,
                160,
                (int)(500 * progress),
                18,
                GREEN
            );

            DrawText(
                "SYSTEM RECOVERY",
                60,
                200,
                18,
                GREEN
            );

            if (stateTimer > 1)
                DrawText(
                    "> HULL INTEGRITY: 12%",
                    60,
                    250,
                    18,
                    RED
                );

            if (stateTimer > 2)
                DrawText(
                    "> NAVIGATION: OFFLINE",
                    60,
                    280,
                    18,
                    RED
                );

            if (stateTimer > 3)
                DrawText(
                    "> COMMUNICATION: LOST",
                    60,
                    310,
                    18,
                    RED
                );

            if (stateTimer > 4)
                DrawText(
                    "> BLACK HOLE EVENT: CONFIRMED",
                    60,
                    340,
                    18,
                    ORANGE
                );

            if (stateTimer > 5)
                DrawText(
                    "> CREWSHIP: DESTROYED",
                    60,
                    370,
                    18,
                    RED
                );

            if (stateTimer > 6)
                DrawText(
                    "> UNIT 07: ONLINE",
                    60,
                    400,
                    18,
                    GREEN
                );

            if (stateTimer > 7)
                DrawText(
                    "> MEMORY CORE: DAMAGED",
                    60,
                    430,
                    18,
                    ORANGE
                );

            if (stateTimer > 8)
                DrawText(
                    "> WAKE UP.",
                    60,
                    475,
                    28,
                    GREEN
                );

            DrawText(
                "ENTER - SKIP",
                60,
                550,
                14,
                DARKGREEN
            );
        }

        // ====================================================
        // DIALOGUE
        // ====================================================

        else if (state == DIALOGUE)
        {
            ClearBackground(
                Color{3, 8, 10, 255}
            );

            DrawStars(time);

            DrawPanel(
                120,
                110,
                760,
                380
            );

            DrawText(
                "RECOVERED TRANSMISSION",
                155,
                140,
                24,
                GREEN
            );

            const char *text = "";

            if (dialoguePage == 0)
            {
                text =
                    "UNIT 07...\n\n"
                    "If you can hear this, the ship is gone.\n"
                    "The black hole tore through the outer decks.\n\n"
                    "We scattered emergency caches before impact.\n"
                    "Find them. Recover what you can.\n\n"
                    "Then find a way home.";
            }
            else if (dialoguePage == 1)
            {
                text =
                    "CACHE 01 RECOVERED.\n\n"
                    "The emergency door is responding.\n"
                    "But something is wrong.\n\n"
                    "The crew didn't abandon the ship.\n"
                    "They were trying to reach the archive.";
            }
            else if (dialoguePage == 2)
            {
                text =
                    "DEAD DECK REPORT.\n\n"
                    "Gravity systems are failing.\n"
                    "The ship is being pulled toward the anomaly.\n\n"
                    "Someone left a message in the archive.\n"
                    "Maybe it explains what happened.";
            }
            else if (dialoguePage == 3)
            {
                text =
                    "ARCHIVE ENTRY FOUND.\n\n"
                    "The black hole wasn't random.\n\n"
                    "The crew detected a repeating signal\n"
                    "coming from inside the anomaly.\n\n"
                    "They went looking for its source.";
            }
            else
            {
                text =
                    "FINAL DECK.\n\n"
                    "The signal is still transmitting.\n\n"
                    "The crew may be gone.\n"
                    "But their message survived.\n\n"
                    "Unit 07, finish what we started.";
            }

            int y = 195;

            int start = 0;
            int len = 0;

            for (int i = 0;; i++)
            {
                if (
                    text[i] == '\n' ||
                    text[i] == '\0'
                )
                {
                    char buffer[256];

                    int count = len;

                    if (count > 250)
                        count = 250;

                    for (int j = 0; j < count; j++)
                    {
                        buffer[j] =
                            text[start + j];
                    }

                    buffer[count] = '\0';

                    DrawText(
                        buffer,
                        160,
                        y,
                        18,
                        GREEN
                    );

                    y += 30;

                    if (text[i] == '\0')
                        break;

                    start = i + 1;
                    len = 0;
                }
                else
                {
                    len++;
                }
            }

            DrawText(
                "[ ENTER ] CONTINUE",
                680,
                455,
                16,
                GREEN
            );
        }

        // ====================================================
        // PLAYING
        // ====================================================

        else if (state == PLAYING)
        {
            BeginScissorMode(
                (int)sx,
                (int)sy,
                screenWidth,
                screenHeight
            );

            ClearBackground(
                Color{7, 11, 14, 255}
            );

            DrawStars(time);

            // Ship room
            DrawRectangle(
                45,
                75,
                910,
                455,
                Color{18, 24, 27, 255}
            );

            // Floor
            DrawRectangle(
                45,
                430,
                910,
                100,
                Color{11, 15, 17, 255}
            );

            // Floor panels
            for (int x = 50; x < 950; x += 55)
            {
                DrawLine(
                    x,
                    430,
                    x,
                    530,
                    DARKGRAY
                );
            }

            // Flickering lights
            for (int x = 100; x < 900; x += 120)
            {
                float flicker =
                    0.45f +
                    0.25f *
                    sinf(time * 7 + x);

                DrawRectangle(
                    x,
                    82,
                    70,
                    5,
                    Fade(GREEN, flicker)
                );
            }

            // =================================================
            // LEVEL 1
            // =================================================

            if (level == 1)
            {
                DrawBlackHole(
                    760,
                    190,
                    time
                );

                DrawTerminal(
                    {700, 260},
                    time
                );

                if (!cache1)
                    DrawCache(
                        {760, 400}
                    );

                if (!scrap1)
                    DrawScrap(
                        {330, 420}
                    );

                DrawDoor(
                    {900, 310},
                    door1
                );

                DrawText(
                    cache1 ?
                    "REPAIR THE EMERGENCY DOOR" :
                    "SEARCH THE CREW CACHE",
                    300,
                    55,
                    18,
                    GREEN
                );

                if (
                    Distance(
                        player.pos,
                        {700, 260}
                    ) < 85
                )
                {
                    DrawText(
                        "[E] TERMINAL",
                        650,
                        330,
                        15,
                        GREEN
                    );
                }

                if (
                    Distance(
                        player.pos,
                        {760, 400}
                    ) < 85 &&
                    !cache1
                )
                {
                    DrawText(
                        "[E] DECODE CACHE",
                        690,
                        455,
                        15,
                        GREEN
                    );
                }

                if (
                    Distance(
                        player.pos,
                        {330, 420}
                    ) < 75 &&
                    !scrap1
                )
                {
                    DrawText(
                        "[E] SCAVENGE",
                        290,
                        470,
                        15,
                        GREEN
                    );
                }
            }

            // =================================================
            // LEVEL 2
            // =================================================

            else if (level == 2)
            {
                DrawBlackHole(
                    760,
                    160,
                    time
                );

                DrawTerminal(
                    {240, 180},
                    time
                );

                if (!cache2)
                    DrawCache(
                        {700, 180}
                    );

                if (!scrap2)
                    DrawScrap(
                        {350, 430}
                    );

                DrawDoor(
                    {900, 330},
                    door2
                );

                DrawText(
                    cache2 ?
                    "RESTORE THE DEAD DECK" :
                    "FIND NAVIGATION CACHE",
                    330,
                    55,
                    18,
                    GREEN
                );

                if (
                    Distance(
                        player.pos,
                        {240, 180}
                    ) < 85
                )
                {
                    DrawText(
                        "[E] TERMINAL",
                        190,
                        245,
                        15,
                        GREEN
                    );
                }

                if (
                    Distance(
                        player.pos,
                        {700, 180}
                    ) < 85 &&
                    !cache2
                )
                {
                    DrawText(
                        "[E] DECODE CACHE",
                        650,
                        230,
                        15,
                        GREEN
                    );
                }

                if (
                    Distance(
                        player.pos,
                        {350, 430}
                    ) < 75 &&
                    !scrap2
                )
                {
                    DrawText(
                        "[E] SCAVENGE",
                        300,
                        475,
                        15,
                        GREEN
                    );
                }
            }

            // =================================================
            // LEVEL 3
            // =================================================

            else if (level == 3)
            {
                DrawBlackHole(
                    760,
                    180,
                    time
                );

                DrawTerminal(
                    {250, 170},
                    time
                );

                if (!cache3)
                    DrawCache(
                        {750, 170}
                    );

                if (!scrap3)
                    DrawScrap(
                        {350, 430}
                    );

                DrawDoor(
                    {900, 330},
                    door3
                );

                DrawText(
                    "THE ARCHIVE",
                    420,
                    55,
                    20,
                    GREEN
                );

                if (
                    Distance(
                        player.pos,
                        {250, 170}
                    ) < 85
                )
                {
                    DrawText(
                        "[E] ACCESS ARCHIVE",
                        180,
                        235,
                        15,
                        GREEN
                    );
                }

                if (
                    Distance(
                        player.pos,
                        {750, 170}
                    ) < 85 &&
                    !cache3
                )
                {
                    DrawText(
                        "[E] DECODE RECORD",
                        680,
                        220,
                        15,
                        GREEN
                    );
                }

                if (
                    Distance(
                        player.pos,
                        {350, 430}
                    ) < 75 &&
                    !scrap3
                )
                {
                    DrawText(
                        "[E] SCAVENGE",
                        300,
                        475,
                        15,
                        GREEN
                    );
                }
            }

            // =================================================
            // LEVEL 4
            // =================================================

            else if (level == 4)
            {
                DrawBlackHole(
                    780,
                    300,
                    time * 1.5f
                );

                if (!cache4)
                    DrawCache(
                        {700, 180}
                    );

                DrawTerminal(
                    {300, 420},
                    time
                );

                DrawDoor(
                    {900, 300},
                    door4
                );

                DrawText(
                    "GRAVITY WELL",
                    410,
                    55,
                    20,
                    ORANGE
                );

                DrawText(
                    "THE SHIP IS FALLING APART.",
                    355,
                    95,
                    15,
                    RED
                );

                if (
                    Distance(
                        player.pos,
                        {700, 180}
                    ) < 85 &&
                    !cache4
                )
                {
                    DrawText(
                        "[E] DECODE SIGNAL",
                        640,
                        225,
                        15,
                        GREEN
                    );
                }

                if (
                    Distance(
                        player.pos,
                        {300, 420}
                    ) < 85
                )
                {
                    DrawText(
                        "[E] STABILIZE SYSTEM",
                        235,
                        475,
                        15,
                        GREEN
                    );
                }
            }

            // =================================================
            // LEVEL 5
            // =================================================

            else if (level == 5)
            {
                DrawBlackHole(
                    500,
                    190,
                    time * 1.2f
                );

                if (!cache5)
                    DrawCache(
                        {700, 180}
                    );

                // Escape pod
                DrawEllipse(
                    850,
                    330,
                    55,
                    85,
                    Color{55, 65, 68, 255}
                );

                DrawEllipseLines(
                    850,
                    330,
                    55,
                    85,
                    GREEN
                );

                DrawText(
                    "ESCAPE POD",
                    810,
                    430,
                    14,
                    GREEN
                );

                DrawText(
                    "LAST SIGNAL",
                    420,
                    55,
                    20,
                    GREEN
                );

                if (!cache5)
                {
                    DrawText(
                        "RECOVER THE FINAL CACHE",
                        345,
                        95,
                        15,
                        GREEN
                    );
                }
                else
                {
                    DrawText(
                        "ESCAPE POD READY",
                        385,
                        95,
                        15,
                        GREEN
                    );
                }

                if (
                    Distance(
                        player.pos,
                        {700, 180}
                    ) < 85 &&
                    !cache5
                )
                {
                    DrawText(
                        "[E] DECODE FINAL CACHE",
                        650,
                        225,
                        15,
                        GREEN
                    );
                }

                if (
                    Distance(
                        player.pos,
                        {850, 330}
                    ) < 110
                )
                {
                    DrawText(
                        "[E] ENTER ESCAPE POD",
                        770,
                        455,
                        15,
                        GREEN
                    );
                }
            }

            // Robot
            DrawRobot(
                player.pos,
                player.facing,
                time
            );

            // HUD
            DrawHUD(
                level,
                power,
                circuits,
                metal,
                data
            );

            // Energy
            DrawText(
                "ENERGY",
                25,
                555,
                13,
                GREEN
            );

            DrawRectangle(
                85,
                555,
                150,
                10,
                DARKGREEN
            );

            DrawRectangle(
                85,
                555,
                (int)(
                    150 *
                    (player.energy / 100.0f)
                ),
                10,
                GREEN
            );

            EndScissorMode();
        }

        // ====================================================
        // TERMINAL SCREEN
        // ====================================================

        else if (state == TERMINAL)
        {
            ClearBackground(BLACK);

            DrawPanel(
                120,
                70,
                760,
                460
            );

            DrawText(
                "CREWSHIP TERMINAL",
                155,
                100,
                30,
                GREEN
            );

            DrawText(
                "> CONNECTION ESTABLISHED",
                160,
                155,
                18,
                GREEN
            );

            if (level == 1)
            {
                DrawText(
                    "RECOVERED LOG // CAPTAIN",
                    160,
                    210,
                    18,
                    GREEN
                );

                DrawText(
                    "The anomaly wasn't supposed to be here.",
                    180,
                    255,
                    17,
                    GREEN
                );

                DrawText(
                    "We detected a signal inside it.",
                    180,
                    285,
                    17,
                    GREEN
                );

                DrawText(
                    "Then everything went dark.",
                    180,
                    315,
                    17,
                    GREEN
                );
            }
            else if (level == 2)
            {
                DrawText(
                    "DEAD DECK SYSTEMS",
                    160,
                    210,
                    18,
                    GREEN
                );

                DrawText(
                    "POWER ROUTING FAILURE.",
                    180,
                    255,
                    17,
                    RED
                );

                DrawText(
                    "Navigation cache marked for recovery.",
                    180,
                    285,
                    17,
                    GREEN
                );
            }
            else if (level == 3)
            {
                DrawText(
                    "ARCHIVE DATABASE",
                    160,
                    210,
                    18,
                    GREEN
                );

                DrawText(
                    "CLASSIFIED CREW RECORD.",
                    180,
                    255,
                    17,
                    ORANGE
                );

                DrawText(
                    "The signal responds to us.",
                    180,
                    285,
                    17,
                    GREEN
                );

                DrawText(
                    "It knows we're here.",
                    180,
                    315,
                    17,
                    GREEN
                );
            }
            else
            {
                DrawText(
                    "SYSTEM STABILIZATION",
                    160,
                    210,
                    18,
                    GREEN
                );

                DrawText(
                    "Gravity distortion increasing.",
                    180,
                    255,
                    17,
                    RED
                );

                DrawText(
                    "Escape route calculated.",
                    180,
                    285,
                    17,
                    GREEN
                );
            }

            DrawText(
                "[ENTER / E] CLOSE",
                160,
                465,
                16,
                DARKGREEN
            );
        }

        // ====================================================
        // CIPHER SCREEN
        // ====================================================

        else if (state == CIPHER)
        {
            ClearBackground(
                Color{2, 7, 8, 255}
            );

            DrawPanel(
                180,
                90,
                640,
                420
            );

            DrawText(
                "ENCRYPTED CACHE",
                225,
                125,
                30,
                GREEN
            );

            DrawText(
                "CREW CIPHER PROTOCOL",
                225,
                170,
                16,
                DARKGREEN
            );

            DrawText(
                "SIGNAL:",
                225,
                230,
                20,
                GREEN
            );

            DrawText(
                TextFormat(
                    "%d",
                    cipherAnswer
                ),
                330,
                225,
                25,
                ORANGE
            );

            DrawText(
                "ENTER DECRYPTION KEY",
                225,
                280,
                20,
                GREEN
            );

            DrawRectangle(
                225,
                320,
                200,
                55,
                BLACK
            );

            DrawRectangleLines(
                225,
                320,
                200,
                55,
                GREEN
            );

            DrawText(
                TextFormat(
                    "%d",
                    cipherInput
                ),
                310,
                330,
                30,
                GREEN
            );

            DrawText(
                "NUMBER KEYS 0-9",
                225,
                410,
                15,
                DARKGREEN
            );

            DrawText(
                "ENTER = DECRYPT",
                225,
                440,
                15,
                GREEN
            );

            DrawText(
                "ESC = CANCEL",
                225,
                465,
                15,
                DARKGREEN
            );
        }

        // ====================================================
        // PAUSE
        // ====================================================

        else if (state == PAUSE)
        {
            ClearBackground(
                Color{5, 8, 10, 255}
            );

            DrawCentered(
                "PAUSED",
                180,
                55,
                GREEN
            );

            DrawCentered(
                "VOID//SIGNAL",
                250,
                18,
                DARKGREEN
            );

            DrawCentered(
                "[ ESC ] RESUME",
                360,
                20,
                GREEN
            );
        }

        // ====================================================
        // ENDING
        // ====================================================

        else if (state == ENDING)
        {
            DrawStars(time);

            DrawBlackHole(
                700,
                250,
                time
            );

            float fade =
                ClampFloat(
                    stateTimer / 5.0f,
                    0.0f,
                    1.0f
                );

            DrawCentered(
                "ESCAPE SYSTEM ONLINE",
                110,
                32,
                Fade(GREEN, fade)
            );

            if (stateTimer > 1)
            {
                DrawCentered(
                    "NAVIGATION RESTORED",
                    180,
                    20,
                    GREEN
                );
            }

            if (stateTimer > 2)
            {
                DrawCentered(
                    "CREW SIGNAL: LOST",
                    220,
                    20,
                    RED
                );
            }

            if (stateTimer > 3)
            {
                DrawCentered(
                    "UNIT 07: ALIVE",
                    270,
                    28,
                    GREEN
                );
            }

            if (stateTimer > 5)
            {
                DrawCentered(
                    "The escape pod leaves the wreck.",
                    350,
                    18,
                    GREEN
                );

                DrawCentered(
                    "Behind you, the black hole consumes the ship.",
                    385,
                    18,
                    GREEN
                );

                DrawCentered(
                    "But the crew's final signal survives.",
                    420,
                    18,
                    GREEN
                );
            }

            if (stateTimer > 7)
            {
                DrawCentered(
                    "\"UNIT 07... IF YOU'RE STILL OUT THERE...\"",
                    475,
                    16,
                    DARKGREEN
                );
            }

            if (stateTimer > 9)
            {
                DrawCentered(
                    "[ ENTER ] RETURN TO TITLE",
                    540,
                    18,
                    GREEN
                );
            }
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}