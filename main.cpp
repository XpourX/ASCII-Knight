/**
*
* Solution to course project # 10
* Introduction to programming course
* Faculty of Mathematics and Informatics of Sofia University
* Winter semester 2025/2026
*
* @author Yusmen Ismail Osman
* @idnumber 5MI0600666
* @compiler GCC
*
* <Main game file - ASCII Knight game implementation>
*
*/

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>

using namespace std;

// ========================================
// CONSTANTS AND CONFIGURATION
// ========================================

// Arena dimensions
const int ARENA_WIDTH = 120;
const int ARENA_HEIGHT = 30;

// Player constants
const int PLAYER_MAX_HP = 5;
const int PLAYER_JUMP_VELOCITY = -4;
const int PLAYER_MAX_FALL_SPEED = 5;

// Physics constants
const int GRAVITY = 1;

// Combat constants
const int ATTACK_COOLDOWN_FRAMES = 10;
const int ATTACK_DURATION_LONG = 20;
const int ATTACK_DURATION_SHORT = 2;

// Enemy AI constants
const int CHASE_RANGE = 10;
const int JUMP_RANGE = 25;
const int FLIER_DESCENT_INTERVAL = 180;
const int FLIER_DESCENT_AMOUNT = 3;
const int BOSS_ATTACK_INTERVAL = 30;
const int BOSS_WINDUP_TIME = 10;
const int BOSS_AOE_RANGE = 5;
const int BOSS_DAMAGE = 3;
const int BOSS_MAX_HP = 5;

// Wave constants
const int MAX_WAVES = 5;
const int WAVE_DELAY_MS = 2000;
const int WAVE_ENEMY_INCREMENT_MIN = 2;
const int WAVE_ENEMY_INCREMENT_MAX = 4;

// Frame timing
const int FRAME_DELAY_MS = 16;

// ========================================
// STRUCTURES
// ========================================

// Player structure
struct Player {
    int x;
    int y;
    int hp;
    int velocityY;
    bool isOnGround;
    bool canDoubleJump;
};

// Attack structure
struct Attack {
    bool isActive;
    char direction;
    int x;
    int y;
    int framesRemaining;
};

// Enemy structure
struct Enemy {
    char type;
    int x;
    int y;
    int velocityX;
    int velocityY;
    int hp;
    bool isActive;
    bool isOnGround;
    int aiTimer; // For Flier descent timing, Boss attack timing
    char surface; // For Crawler: 'f'=floor, 'r'=right wall, 'l'=left wall, 'c'=ceiling
    int edgeWrapStep; // For Crawler edge wrapping: 0=normal, 1-4=wrapping steps
    int attackState; // For Boss: 0=walking, 1=winding up, 2=attacking
    int windupTimer; // For Boss windup countdown
};

// Global variables
char arena[ARENA_HEIGHT][ARENA_WIDTH];
Player player;
Attack currentAttack;
int attackCooldown = 0;
int combatStyle = 1; // 1 = cooldown-based, 2 = duration-based spam

// ========================================
// GLOBAL VARIABLES
// ========================================

// Enemy management
Enemy* enemies = nullptr;
int enemyCount = 0;
int enemyCountToSpawn = 0;
int enemyCapacity = 10;

// Wave management
int currentWave = 1;
int totalEnemiesFromPreviousWaves = 0;
bool waveInProgress = false;

// ========================================
// FUNCTION DECLARATIONS
// ========================================

// Core game functions
void runGameLoop();
void processInput();
void updateGame();

// Menu and initialization
void showCombatMenu();
void initializeArena();
void initializePlayer();
void initializeEnemies();

// Wave management
void spawnWave(int waveNumber);
bool isWaveComplete();

// Rendering functions
void render();
void renderHUD();
void renderArena();
char getCharAtPosition(int i, int j, bool& shouldColor, char& colorChar);
bool tryRenderAttack(int i, int j, char& outChar);
bool tryRenderEnemy(int i, int j, char& outChar, char& colorChar);

// Console utility
void moveCursorToTopLeft();
void hideCursor();
void setColorForEnemy(char type);
void resetConsoleColor();

// Physics and collision
bool isColliding(int x, int y);
void applyGravity();
void applyEnemyGravity(Enemy& enemy);

// Player systems
void updatePlayer();
void performAttack(char direction);
void updateAttack();

// Enemy systems
void addEnemy(char type, int x, int y);
void removeEnemy(int index);
void updateEnemies();
void updateEnemyAI();
void updateWalkerAI(Enemy& enemy);
void updateJumperAI(Enemy& enemy);
void updateFlierAI(Enemy& enemy);
void updateCrawlerAI(Enemy& enemy);
void updateBossAI(Enemy& enemy);
void handleCrawlerFloorMode(Enemy& enemy);
void handleCrawlerRightWallMode(Enemy& enemy);
void handleCrawlerLeftWallMode(Enemy& enemy);
void handleCrawlerCeilingMode(Enemy& enemy);
void handleCrawlerEdgeWrap(Enemy& enemy);

// Combat systems
bool isEnemyHitByAttack(Enemy& enemy);
void checkAttackHits();
void checkPlayerEnemyCollision();

// Cleanup
void cleanupEnemies();

// ========================================
// MAIN FUNCTION
// ========================================

int main() {
    srand((unsigned)time(nullptr));

    hideCursor();
    showCombatMenu();
    initializeArena();
    initializePlayer();
    initializeEnemies();

    currentAttack.isActive = false;

    runGameLoop();

    cleanupEnemies();
    return 0;
}

// ========================================
// CORE GAME LOOP
// ========================================

// Main game loop - handles wave progression and win/loss conditions
void runGameLoop() {
    spawnWave(currentWave);
    waveInProgress = true;

    while (true) {
        // Check if current wave is complete
        if (waveInProgress && isWaveComplete()) {
            waveInProgress = false;
            currentWave++;

            if (currentWave <= MAX_WAVES) {
                Sleep(WAVE_DELAY_MS);
                spawnWave(currentWave);
                waveInProgress = true;
            }
        }

        // Victory condition: all waves complete
        if (currentWave > MAX_WAVES && enemyCount == 0) {
            moveCursorToTopLeft();
            cout << "\n\n";
            cout << "        YOU WIN!\n";
            cout << "    Press any key to exit...\n";
            _getch();
            break;
        }

        // Defeat condition: HP depleted
        if (player.hp <= 0) {
            moveCursorToTopLeft();
            cout << "\n\n";
            cout << "        GAME OVER!\n";
            cout << "    Press any key to exit...\n";
            _getch();
            break;
        }

        updateGame();
        processInput();

        Sleep(FRAME_DELAY_MS);
    }
}

// Process all player input
void processInput() {
    if (_kbhit()) {
        char ch = _getch();

        // ESC to exit
        if (ch == 27) {
            player.hp = 0;
            return;
        }

        // Movement controls
        if ((ch == 'a' || ch == 'A') && player.x > 1) {
            player.x--;
        }
        if ((ch == 'd' || ch == 'D') && player.x < ARENA_WIDTH - 2) {
            player.x++;
        }

        // Jump controls (single and double jump)
        if (ch == 'w' || ch == 'W') {
            if (player.isOnGround) {
                player.velocityY = PLAYER_JUMP_VELOCITY;
                player.isOnGround = false;
                player.canDoubleJump = true;
            } else if (player.canDoubleJump) {
                player.velocityY = PLAYER_JUMP_VELOCITY;
                player.canDoubleJump = false;
            }
        }

        // Attack controls (four directions)
        if (ch == 'i' || ch == 'I') {
            performAttack('i');
        }
        if (ch == 'j' || ch == 'J') {
            performAttack('j');
        }
        if (ch == 'k' || ch == 'K') {
            performAttack('k');
        }
        if (ch == 'l' || ch == 'L') {
            performAttack('l');
        }
    }
}

// Update all game state (physics, AI, collisions)
void updateGame() {
    updatePlayer();
    updateAttack();
    updateEnemies();
    checkAttackHits();
    checkPlayerEnemyCollision();

    moveCursorToTopLeft();
    render();
}

// ========================================
// UTILITY FUNCTIONS
// ========================================

void setColorForEnemy(char type) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    switch (type) {
        case 'E': // Walker
            SetConsoleTextAttribute(hConsole, 10); // Light green
            break;
        case 'J': // Jumper
            SetConsoleTextAttribute(hConsole, 14); // Yellow
            break;
        case 'F': // Flier
            SetConsoleTextAttribute(hConsole, 11); // Cyan
            break;
        case 'C': // Crawler
            SetConsoleTextAttribute(hConsole, 13); // Magenta
            break;
        case 'B': // Boss
            SetConsoleTextAttribute(hConsole, 12); // Light red
            break;
        default:
            SetConsoleTextAttribute(hConsole, 7); // Default white
    }
}

// Reset color back to normal
void resetConsoleColor() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

// ========================================
// MENU SYSTEM
// ========================================

void showCombatMenu() {
    system("cls");
    cout << "\n\n";
    cout << "        =================================\n";
    cout << "              ASCII KNIGHT GAME\n";
    cout << "        =================================\n\n";
    cout << "        Select Combat Style:\n\n";
    cout << "        1. Cooldown-based (current style)\n";
    cout << "           - Attacks require cooldown between uses\n";
    cout << "           - Strategic timing required\n\n";
    cout << "        2. Duration-based spam\n";
    cout << "           - Attacks last for a short time\n";
    cout << "           - Can be spammed rapidly\n\n";
    cout << "        Enter your choice (1 or 2): ";

    char choice;
    while (true) {
        choice = _getch();
        if (choice == '1' || choice == '2') {
            combatStyle = choice - '0';
            cout << choice << "\n\n";
            cout << "        Combat style selected! Starting game...\n";
            Sleep(1500);
            system("cls");
            break;
        }
    }
}

// ========================================
// ARENA SYSTEM
// ========================================

void initializeArena() {
    // Clear arena
    for (int i = 0; i < ARENA_HEIGHT; i++) {
        for (int j = 0; j < ARENA_WIDTH; j++) {
            arena[i][j] = ' ';
        }
    }

    // Create border walls
    for (int j = 0; j < ARENA_WIDTH; j++) {
        arena[0][j] = '#';
        arena[ARENA_HEIGHT - 1][j] = '#';
    }

    for (int i = 0; i < ARENA_HEIGHT; i++) {
        arena[i][0] = '#';
        arena[i][ARENA_WIDTH - 1] = '#';
    }


    for (int j = 5; j < 50; j++) arena[ARENA_HEIGHT - 6][j] = '=';
    for (int j = 30; j < 60; j++) arena[ARENA_HEIGHT - 12][j] = '=';
    for (int j = 50; j < 90; j++) arena[ARENA_HEIGHT - 18][j] = '=';
 
}

// ========================================
// PLAYER SYSTEM
// ========================================

void initializePlayer() {
    player.x = ARENA_WIDTH / 2;
    player.y = ARENA_HEIGHT - 2;
    player.hp = PLAYER_MAX_HP;
    player.velocityY = 0;
    player.isOnGround = false;
    player.canDoubleJump = false;
}

void updatePlayer() {
    applyGravity();
}

// ========================================
// PHYSICS SYSTEM
// ========================================

bool isColliding(int x, int y) {
    if (x < 0 || x >= ARENA_WIDTH || y < 0 || y >= ARENA_HEIGHT) {
        return true;
    }
    return arena[y][x] == '#' || arena[y][x] == '=';
}
bool isEnemyHitByAttack(Enemy& enemy) {
    if (!currentAttack.isActive) return false;

    // Determine attack hitbox
    int hitX[3], hitY[3];
    int hitCount = 0;

    if (currentAttack.direction == 'i') { // Up
        hitY[0] = hitY[1] = hitY[2] = currentAttack.y;
        hitX[0] = currentAttack.x;
        hitX[1] = currentAttack.x + 1;
        hitX[2] = currentAttack.x + 2;
        hitCount = 3;
    } else if (currentAttack.direction == 'j') { // Left
        hitX[0] = hitX[1] = hitX[2] = currentAttack.x;
        hitY[0] = currentAttack.y;
        hitY[1] = currentAttack.y + 1;
        hitY[2] = currentAttack.y + 2;
        hitCount = 3;
    } else if (currentAttack.direction == 'k') { // Down
        hitY[0] = hitY[1] = hitY[2] = currentAttack.y;
        hitX[0] = currentAttack.x;
        hitX[1] = currentAttack.x + 1;
        hitX[2] = currentAttack.x + 2;
        hitCount = 3;
    } else if (currentAttack.direction == 'l') { // Right
        hitX[0] = hitX[1] = hitX[2] = currentAttack.x;
        hitY[0] = currentAttack.y;
        hitY[1] = currentAttack.y + 1;
        hitY[2] = currentAttack.y + 2;
        hitCount = 3;
    }

    // Check collision
    for (int h = 0; h < hitCount; h++) {
        if (enemy.type == 'B') {
            // Boss 3x3 hitbox
            if (hitX[h] >= enemy.x - 1 && hitX[h] <= enemy.x + 1 &&
                hitY[h] >= enemy.y - 1 && hitY[h] <= enemy.y + 1) {
                return true;
            }
        } else {
            // Normal 1x1 enemy
            if (enemy.x == hitX[h] && enemy.y == hitY[h]) {
                return true;
            }
        }
    }

    return false;
}


// Apply gravity to player - handles falling, jumping, and platform collision
void applyGravity() {
    player.velocityY += GRAVITY;

    if (player.velocityY > PLAYER_MAX_FALL_SPEED) {
        player.velocityY = PLAYER_MAX_FALL_SPEED;
    }

    // Falling downward
    if (player.velocityY > 0) {
        // Move down one pixel at a time to avoid falling through platforms
        for (int i = 0; i < player.velocityY; i++) {
            int nextY = player.y + 1;

            if (isColliding(player.x, nextY)) {
                player.velocityY = 0;
                player.isOnGround = true;
                player.canDoubleJump = false;
                break;
            } else {
                player.y = nextY;
                player.isOnGround = false;
            }
        }
    }
    // Jumping upward
    else if (player.velocityY < 0) {
        for (int i = 0; i < -player.velocityY; i++) {
            int nextY = player.y - 1;

            if (nextY < 0) {
                player.velocityY = 0;
                break;
            }

            char cell = arena[nextY][player.x];

            // Only walls block upward movement (can jump through platforms)
            if (cell == '#') {
                player.velocityY = 0;
                break;
            } else {
                player.y = nextY;
                player.isOnGround = false;
            }
        }
    }
    // Standing still - check if ground still exists below
    else {
        if (!isColliding(player.x, player.y + 1)) {
            player.isOnGround = false;
        }
    }
}

// ========================================
// ATTACK SYSTEM
// ========================================

// Initiate a directional attack (i=up, j=left, k=down, l=right)
void performAttack(char direction) {
    // Combat Style 1: Cooldown-based (strategic timing)
    if (combatStyle == 1) {
        if (attackCooldown > 0 || currentAttack.isActive) {
            return;
        }

        currentAttack.isActive = true;
        currentAttack.direction = direction;
        currentAttack.framesRemaining = ATTACK_DURATION_LONG;

        // Position attack relative to player based on direction
        switch (direction) {
            case 'i': // Up
                currentAttack.x = player.x - 1;
                currentAttack.y = player.y - 2;
                break;
            case 'j': // Left
                currentAttack.x = player.x - 2;
                currentAttack.y = player.y - 1;
                break;
            case 'k': // Down
                currentAttack.x = player.x - 1;
                currentAttack.y = player.y + 1;
                break;
            case 'l': // Right
                currentAttack.x = player.x + 1;
                currentAttack.y = player.y - 1;
                break;
        }

        attackCooldown = ATTACK_COOLDOWN_FRAMES;
    }
    else if (combatStyle == 2) {
        currentAttack.isActive = true;
        currentAttack.direction = direction;
        currentAttack.framesRemaining = ATTACK_DURATION_SHORT;

        // Set attack position based on direction
        switch (direction) {
            case 'i': // Up
                currentAttack.x = player.x - 1;
                currentAttack.y = player.y - 2;
                break;
            case 'j': // Left
                currentAttack.x = player.x - 2;
                currentAttack.y = player.y - 1;
                break;
            case 'k': // Down
                currentAttack.x = player.x - 1;
                currentAttack.y = player.y + 1;
                break;
            case 'l': // Right
                currentAttack.x = player.x + 1;
                currentAttack.y = player.y - 1;
                break;
        }

        attackCooldown = 0;
    }
}

void updateAttack() {
    if (currentAttack.isActive) {
        currentAttack.framesRemaining--;
        if (currentAttack.framesRemaining <= 0) {
            currentAttack.isActive = false;
        }
    }

    if (combatStyle == 1 && attackCooldown > 0) {
        attackCooldown--;
    }
}

// ========================================
// CONSOLE & RENDERING SYSTEM
// ========================================

void moveCursorToTopLeft() {
    COORD pos = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hideCursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    GetConsoleCursorInfo(console, &info);
    info.bVisible = false;
    SetConsoleCursorInfo(console, &info);
}

// Main render function - displays HUD and arena
void render() {
    renderHUD();
    renderArena();
}

// Render the heads-up display (HP and wave info)
void renderHUD() {
    cout << "HP: " << player.hp << " | Wave: " << currentWave << "/" << MAX_WAVES << "\n";
}

// Render the entire game arena
void renderArena() {
    for (int i = 0; i < ARENA_HEIGHT; i++) {
        for (int j = 0; j < ARENA_WIDTH; j++) {
            bool shouldColor = false;
            char colorChar = ' ';
            char ch = getCharAtPosition(i, j, shouldColor, colorChar);

            if (shouldColor) {
                setColorForEnemy(colorChar);
                cout << ch;
                resetConsoleColor();
            } else {
                cout << ch;
            }
        }
        cout << '\n';
    }
}

// Determine which character should be displayed at position (i, j)
char getCharAtPosition(int i, int j, bool& shouldColor, char& colorChar) {
    char ch = ' ';

    // Try to render attack first (highest priority)
    if (tryRenderAttack(i, j, ch)) {
        return ch;
    }

    // Try to render enemy second
    if (tryRenderEnemy(i, j, ch, colorChar)) {
        shouldColor = true;
        return ch;
    }

    // Render player
    if (i == player.y && j == player.x) {
        return '@';
    }

    // Default to arena tiles
    return arena[i][j];
}

// Try to render attack at position - returns true if attack rendered
bool tryRenderAttack(int i, int j, char& outChar) {
    if (!currentAttack.isActive) {
        return false;
    }

    // Attack upward (3-char horizontal: /-\)
    if (currentAttack.direction == 'i' && i == currentAttack.y) {
        if (j == currentAttack.x) {
            outChar = '/';
            return true;
        } else if (j == currentAttack.x + 1) {
            outChar = '-';
            return true;
        } else if (j == currentAttack.x + 2) {
            outChar = '\\';
            return true;
        }
    }
    // Attack left (3-char vertical)
    else if (currentAttack.direction == 'j' && j == currentAttack.x) {
        if (i == currentAttack.y) {
            outChar = '/';
            return true;
        } else if (i == currentAttack.y + 1) {
            outChar = '|';
            return true;
        } else if (i == currentAttack.y + 2) {
            outChar = '\\';
            return true;
        }
    }
    // Attack downward (3-char horizontal: \_/)
    else if (currentAttack.direction == 'k' && i == currentAttack.y) {
        if (j == currentAttack.x) {
            outChar = '\\';
            return true;
        } else if (j == currentAttack.x + 1) {
            outChar = '_';
            return true;
        } else if (j == currentAttack.x + 2) {
            outChar = '/';
            return true;
        }
    }
    // Attack right (3-char vertical)
    else if (currentAttack.direction == 'l' && j == currentAttack.x) {
        if (i == currentAttack.y) {
            outChar = '\\';
            return true;
        } else if (i == currentAttack.y + 1) {
            outChar = '|';
            return true;
        } else if (i == currentAttack.y + 2) {
            outChar = '/';
            return true;
        }
    }

    return false;
}

// Try to render enemy at position - returns true if enemy rendered
bool tryRenderEnemy(int i, int j, char& outChar, char& colorChar) {
    for (int e = 0; e < enemyCount; e++) {
        if (!enemies[e].isActive) continue;

        // Boss is 3x3, centered at (x, y)
        if (enemies[e].type == 'B') {
            // Render Boss body (3x3)
            if (j >= enemies[e].x - 1 && j <= enemies[e].x + 1 &&
                i >= enemies[e].y - 1 && i <= enemies[e].y + 1) {
                outChar = 'B';
                colorChar = 'B';
                return true;
            }

            // Boss AOE warning during windup (11x11 area)
            if (enemies[e].attackState == 1) {
                if (j >= enemies[e].x - 5 && j <= enemies[e].x + 5 &&
                    i >= enemies[e].y - 5 && i <= enemies[e].y + 5) {
                    // Only show * in empty positions
                    if (arena[i][j] == ' ' &&
                        !(j >= enemies[e].x - 1 && j <= enemies[e].x + 1 &&
                          i >= enemies[e].y - 1 && i <= enemies[e].y + 1) &&
                        !(i == player.y && j == player.x)) {
                        outChar = '*';
                        colorChar = 'B';
                        return true;
                    }
                }
            }
        }
        // Normal 1x1 enemies
        else if (enemies[e].x == j && enemies[e].y == i) {
            outChar = enemies[e].type;
            colorChar = enemies[e].type;
            return true;
        }
    }

    return false;
}

// ========================================
// ENEMY MANAGEMENT SYSTEM
// ========================================

// Initialize the dynamic enemy array
void initializeEnemies() {
    enemies = new Enemy[enemyCapacity];
    enemyCount = 0;
}

// Add a new enemy to the arena - dynamically expands array if needed
void addEnemy(char type, int x, int y) {
    // Expand array if capacity reached (double the size)
    if (enemyCount >= enemyCapacity) {
        int newCapacity = enemyCapacity * 2;
        Enemy* newEnemies = new Enemy[newCapacity];

        for (int i = 0; i < enemyCount; i++) {
            newEnemies[i] = enemies[i];
        }

        delete[] enemies;
        enemies = newEnemies;
        enemyCapacity = newCapacity;
    }

    enemies[enemyCount].type = type;
    enemies[enemyCount].x = x;
    enemies[enemyCount].y = y;
    enemies[enemyCount].hp = (type == 'B') ? BOSS_MAX_HP : 1;
    enemies[enemyCount].velocityX = (rand() % 2 == 0) ? 1 : -1;
    enemies[enemyCount].velocityY = 0;
    enemies[enemyCount].isActive = true;
    enemies[enemyCount].isOnGround = false;
    enemies[enemyCount].aiTimer = 0;
    enemies[enemyCount].surface = 'f';
    enemies[enemyCount].edgeWrapStep = 0;
    enemies[enemyCount].attackState = 0;
    enemies[enemyCount].windupTimer = 0;
    enemyCount++;
}

void removeEnemy(int index) {
    if (index < 0 || index >= enemyCount) return;

    // Shift all enemies after this one back
    for (int i = index; i < enemyCount - 1; i++) {
        enemies[i] = enemies[i + 1];
    }
    enemyCount--;
}

// ========================================
// ENEMY PHYSICS
// ========================================

void applyEnemyGravity(Enemy& enemy) {
    enemy.velocityY += GRAVITY;
    if (enemy.velocityY > PLAYER_MAX_FALL_SPEED) {
        enemy.velocityY = PLAYER_MAX_FALL_SPEED;
    }

    // Apply vertical movement
    if (enemy.velocityY > 0) {
        // Falling
        for (int i = 0; i < enemy.velocityY; i++) {
            int nextY = enemy.y + 1;
            bool collision = false;

            // Boss is 3x3, check all bottom tiles
            if (enemy.type == 'B') {
                for (int dx = -1; dx <= 1; dx++) {
                    if (isColliding(enemy.x + dx, nextY + 1)) {
                        collision = true;
                        break;
                    }
                }
            } else {
                collision = isColliding(enemy.x, nextY);
            }

            if (collision) {
                enemy.velocityY = 0;
                enemy.isOnGround = true;
                break;
            }

            enemy.y = nextY;
            enemy.isOnGround = false;

            // --- Use centralized attack collision ---
            if (currentAttack.isActive && isEnemyHitByAttack(enemy)) {
                enemy.hp--;
                if (enemy.hp <= 0) enemy.isActive = false;
                currentAttack.isActive = false;
            }
        }
    }
    else if (enemy.velocityY < 0) {
        // Moving up (jumping)
        for (int i = 0; i < -enemy.velocityY; i++) {
            int nextY = enemy.y - 1;
            bool collision = false;

            // Boss 3x3 top collision
            if (enemy.type == 'B') {
                if (nextY - 1 < 0) collision = true;
                else {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (arena[nextY - 1][enemy.x + dx] == '#') {
                            collision = true;
                            break;
                        }
                    }
                }
            } else {
                if (nextY < 0 || arena[nextY][enemy.x] == '#') collision = true;
            }

            if (collision) {
                enemy.velocityY = 0;
                break;
            }

            enemy.y = nextY;
            enemy.isOnGround = false;

            // --- Use centralized attack collision ---
            if (currentAttack.isActive && isEnemyHitByAttack(enemy)) {
                enemy.hp--;
                if (enemy.hp <= 0) enemy.isActive = false;
                currentAttack.isActive = false;
            }
        }
    }
    else {
        // Not moving vertically, check if still on ground
        bool hasGround = false;

        if (enemy.type == 'B') {
            for (int dx = -1; dx <= 1; dx++) {
                if (isColliding(enemy.x + dx, enemy.y + 2)) {
                    hasGround = true;
                    break;
                }
            }
        } else {
            hasGround = isColliding(enemy.x, enemy.y + 1);
        }

        if (!hasGround) enemy.isOnGround = false;
    }
}


// ========================================
// ENEMY AI - INDIVIDUAL BEHAVIORS
// ========================================

void updateWalkerAI(Enemy& enemy) {
    int distanceX = (player.x > enemy.x) ? (player.x - enemy.x) : (enemy.x - player.x);
    int distanceY = (player.y > enemy.y) ? (player.y - enemy.y) : (enemy.y - player.y);

    if (distanceX < CHASE_RANGE && distanceY < CHASE_RANGE) {
        if (player.x < enemy.x) {
            enemy.velocityX = -1;
        } else if (player.x > enemy.x) {
            enemy.velocityX = 1;
        }
    }

    int nextX = enemy.x + enemy.velocityX;

    if (nextX < 1 || nextX >= ARENA_WIDTH - 1 || isColliding(nextX, enemy.y)) {
        enemy.velocityX = -enemy.velocityX;
    } else {
        if (!isColliding(nextX, enemy.y + 1)) {
            enemy.velocityX = -enemy.velocityX;
        } else {
            enemy.x = nextX;
        }
    }
}

void updateJumperAI(Enemy& enemy) {
    int distanceX = (player.x > enemy.x) ? (player.x - enemy.x) : (enemy.x - player.x);
    int distanceY = (player.y > enemy.y) ? (player.y - enemy.y) : (enemy.y - player.y);

    if (distanceX < JUMP_RANGE && distanceY < JUMP_RANGE && enemy.isOnGround) {
        enemy.velocityY = PLAYER_JUMP_VELOCITY;
        enemy.isOnGround = false;
    }

    if (distanceX < CHASE_RANGE && distanceY < CHASE_RANGE) {
        if (player.x < enemy.x) {
            enemy.velocityX = -1;
        } else if (player.x > enemy.x) {
            enemy.velocityX = 1;
        }
    }

    int nextX = enemy.x + enemy.velocityX;

    if (nextX < 1 || nextX >= ARENA_WIDTH - 1 || isColliding(nextX, enemy.y)) {
        enemy.velocityX = -enemy.velocityX;
    } else {
        if (!isColliding(nextX, enemy.y + 1)) {
            enemy.velocityX = -enemy.velocityX;
        } else {
            enemy.x = nextX;
        }
    }
}

void updateFlierAI(Enemy& enemy) {
    int nextX = enemy.x + enemy.velocityX;

    bool enemyAhead = false;
    for (int j = 0; j < enemyCount; j++) {
        if (enemies[j].isActive && enemies[j].x == nextX && enemies[j].y == enemy.y) {
            if (&enemies[j] != &enemy) {
                enemyAhead = true;
                break;
            }
        }
    }

    if (nextX < 1 || nextX >= ARENA_WIDTH - 1) {
        enemy.velocityX = -enemy.velocityX;
    } else if (isColliding(nextX, enemy.y) || enemyAhead) {
        int obstaclesAbove = 0;
        int obstaclesBelow = 0;

        for (int dy = 1; dy <= 3; dy++) {
            if (isColliding(enemy.x, enemy.y - dy)) obstaclesAbove++;
            if (isColliding(enemy.x, enemy.y + dy)) obstaclesBelow++;
        }

        if (obstaclesAbove < obstaclesBelow) {
            if (!isColliding(enemy.x, enemy.y - 1) && enemy.y > 1) {
                enemy.y--;
            } else {
                enemy.velocityX = -enemy.velocityX;
            }
        } else {
            if (!isColliding(enemy.x, enemy.y + 1) && enemy.y < ARENA_HEIGHT - 2) {
                enemy.y++;
            } else {
                enemy.velocityX = -enemy.velocityX;
            }
        }
    } else {
        enemy.x = nextX;
    }

    enemy.aiTimer++;
    if (enemy.aiTimer >= FLIER_DESCENT_INTERVAL) {
        if (enemy.y < player.y) {
            for (int dy = 1; dy <= FLIER_DESCENT_AMOUNT; dy++) {
                if (!isColliding(enemy.x, enemy.y + dy)) {
                    enemy.y++;
                } else {
                    break;
                }
            }
        } else if (enemy.y > player.y) {
            for (int dy = 1; dy <= FLIER_DESCENT_AMOUNT; dy++) {
                if (!isColliding(enemy.x, enemy.y - dy)) {
                    enemy.y--;
                } else {
                    break;
                }
            }
        }
        enemy.aiTimer = 0;
    }
}

void updateEnemyAI() {
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].isActive) continue;

        Enemy& enemy = enemies[i];

        if (enemy.type == 'E') {
            updateWalkerAI(enemy);
        } else if (enemy.type == 'J') {
            updateJumperAI(enemy);
        } else if (enemy.type == 'F') {
            updateFlierAI(enemy);
        } else if (enemy.type == 'C') {
            // Crawler: Sticks to surfaces (floor, walls, ceiling) following complete surface logic

            // Handle edge wrapping multi-step movement
            if (enemy.edgeWrapStep > 0) {
                // Steps 1-4: Floor to Ceiling wrapping
                if (enemy.edgeWrapStep >= 1 && enemy.edgeWrapStep <= 4) {
                    int originalDir = (enemy.edgeWrapStep == 1 || enemy.edgeWrapStep == 4) ? 1 : -1;
                    if (enemy.velocityX == 0) {
                        // Determine direction from wrap step
                        originalDir = (enemy.edgeWrapStep == 1 || enemy.edgeWrapStep == 4) ? -1 : 1;
                    }

                    if (enemy.edgeWrapStep == 1) {
                        // Step 1: Move one space in original direction
                        enemy.x += (enemy.velocityX != 0) ? enemy.velocityX : originalDir;
                        enemy.edgeWrapStep = 2;
                    } else if (enemy.edgeWrapStep == 2) {
                        // Step 2: Move one space down
                        enemy.y++;
                        enemy.edgeWrapStep = 3;
                    } else if (enemy.edgeWrapStep == 3) {
                        // Step 3: Move one more space down
                        enemy.y++;
                        enemy.edgeWrapStep = 4;
                    } else if (enemy.edgeWrapStep == 4) {
                        // Step 4: Move one space back and switch to ceiling mode
                        int wrapDir = (enemy.velocityX != 0) ? -enemy.velocityX : -originalDir;
                        enemy.x += wrapDir;
                        enemy.surface = 'c';
                        enemy.velocityX = wrapDir;
                        enemy.edgeWrapStep = 0; // Done wrapping
                    }
                }
                // Steps 5-8: Ceiling to Floor wrapping (reverse of floor to ceiling)
                else if (enemy.edgeWrapStep >= 5 && enemy.edgeWrapStep <= 8) {
                    int originalDir = (enemy.edgeWrapStep == 5 || enemy.edgeWrapStep == 8) ? 1 : -1;
                    if (enemy.velocityX == 0) {
                        // Determine direction from wrap step
                        originalDir = (enemy.edgeWrapStep == 5 || enemy.edgeWrapStep == 8) ? -1 : 1;
                    }

                    if (enemy.edgeWrapStep == 5) {
                        // Step 5: Move one space in original direction
                        enemy.x += (enemy.velocityX != 0) ? enemy.velocityX : originalDir;
                        enemy.edgeWrapStep = 6;
                    } else if (enemy.edgeWrapStep == 6) {
                        // Step 6: Move one space UP
                        enemy.y--;
                        enemy.edgeWrapStep = 7;
                    } else if (enemy.edgeWrapStep == 7) {
                        // Step 7: Move one more space UP
                        enemy.y--;
                        enemy.edgeWrapStep = 8;
                    } else if (enemy.edgeWrapStep == 8) {
                        // Step 8: Move one space back and switch to floor mode
                        int wrapDir = (enemy.velocityX != 0) ? -enemy.velocityX : -originalDir;
                        enemy.x += wrapDir;
                        enemy.surface = 'f';
                        enemy.velocityX = wrapDir;
                        enemy.edgeWrapStep = 0; // Done wrapping
                    }
                }
                continue; // Skip normal movement this frame
            }

            if (enemy.surface == 'f') {
                // ===== FLOOR MODE =====
                // Current state: surface below at (x, y+1)
                // Movement: horizontal (velocityX = ±1)

                int nextX = enemy.x + enemy.velocityX;

                // Case 1A: Wall blocking ahead (includes boundary walls)
                if (nextX <= 0 || nextX >= ARENA_WIDTH - 1 || isColliding(nextX, enemy.y)) {
                    // Hit a wall - transition to climbing it
                    if (enemy.velocityX > 0) {
                        enemy.surface = 'r';
                        enemy.velocityY = -1; // Climb up
                        enemy.velocityX = 0;
                    } else {
                        enemy.surface = 'l';
                        enemy.velocityY = -1; // Climb up
                        enemy.velocityX = 0;
                    }
                }
                // Case 1B: Floor continues
                else if (isColliding(nextX, enemy.y + 1)) {
                    // Floor exists below, move forward
                    enemy.x = nextX;
                }
                // Case 1C: Platform edge - move around the edge to get underneath
                else {
                    // Check if there's a wall ahead that we should climb instead
                    if (enemy.velocityX > 0 && isColliding(nextX + 1, enemy.y)) {
                        // Wall to the right of the edge, climb it
                        enemy.surface = 'r';
                        enemy.velocityY = -1;
                        enemy.velocityX = 0;
                    } else if (enemy.velocityX < 0 && isColliding(nextX - 1, enemy.y)) {
                        // Wall to the left of the edge, climb it
                        enemy.surface = 'l';
                        enemy.velocityY = -1;
                        enemy.velocityX = 0;
                    } else {
                        // No wall, start edge wrapping sequence
                        // Example: crawler at (20,10), platform at (20,9)
                        // Moving right: (20,10) -> (21,10) -> (21,11) -> (21,12) -> (20,12)
                        // This will happen over 4 frames
                        enemy.edgeWrapStep = 1;
                    }
                }
            }
            else if (enemy.surface == 'r') {
                // ===== RIGHT WALL MODE =====
                // Current state: surface right at (x+1, y)
                // Movement: vertical (velocityY = ±1)

                int nextY = enemy.y + enemy.velocityY;

                // Check for transitions FIRST before boundaries
                // Check if wall still exists to the right at next position
                bool wallContinues = (nextY > 0 && nextY < ARENA_HEIGHT - 1 && isColliding(enemy.x + 1, nextY));
                bool pathBlocked = (nextY > 0 && nextY < ARENA_HEIGHT - 1 && isColliding(enemy.x, nextY));

                // Case 2A: Path blocked by obstacle
                if (pathBlocked) {
                    enemy.velocityY = -enemy.velocityY; // Turn around
                }
                // Case 2B: Wall continues
                else if (wallContinues) {
                    // Wall exists, move along it
                    enemy.y = nextY;
                }
                // Wall ends OR boundary reached - check for transitions
                else {
                    // Case 2C: Going up - check for ceiling
                    if (enemy.velocityY < 0) {
                        if (isColliding(enemy.x, enemy.y - 1)) {
                            // Ceiling exists, transition to it
                            enemy.surface = 'c';
                            enemy.velocityX = -1; // Move left (away from wall)
                            enemy.velocityY = 0;
                        } else {
                            enemy.velocityY = -enemy.velocityY; // Turn around
                        }
                    }
                    // Case 2D: Going down - check for floor
                    else {
                        if (isColliding(enemy.x, enemy.y + 1)) {
                            // Floor exists, transition to it
                            enemy.surface = 'f';
                            enemy.velocityX = -1; // Move left (away from wall)
                            enemy.velocityY = 0;
                        } else {
                            enemy.velocityY = -enemy.velocityY; // Turn around
                        }
                    }
                }
            }
            else if (enemy.surface == 'l') {
                // ===== LEFT WALL MODE =====
                // Current state: surface left at (x-1, y)
                // Movement: vertical (velocityY = ±1)

                int nextY = enemy.y + enemy.velocityY;

                // Check for transitions FIRST before boundaries
                // Check if wall still exists to the left at next position
                bool wallContinues = (nextY > 0 && nextY < ARENA_HEIGHT - 1 && isColliding(enemy.x - 1, nextY));
                bool pathBlocked = (nextY > 0 && nextY < ARENA_HEIGHT - 1 && isColliding(enemy.x, nextY));

                // Case 3A: Path blocked by obstacle
                if (pathBlocked) {
                    enemy.velocityY = -enemy.velocityY; // Turn around
                }
                // Case 3B: Wall continues
                else if (wallContinues) {
                    // Wall exists, move along it
                    enemy.y = nextY;
                }
                // Wall ends OR boundary reached - check for transitions
                else {
                    // Case 3C: Going up - check for ceiling
                    if (enemy.velocityY < 0) {
                        if (isColliding(enemy.x, enemy.y - 1)) {
                            // Ceiling exists, transition to it
                            enemy.surface = 'c';
                            enemy.velocityX = 1; // Move right (away from wall)
                            enemy.velocityY = 0;
                        } else {
                            enemy.velocityY = -enemy.velocityY; // Turn around
                        }
                    }
                    // Case 3D: Going down - check for floor
                    else {
                        if (isColliding(enemy.x, enemy.y + 1)) {
                            // Floor exists, transition to it
                            enemy.surface = 'f';
                            enemy.velocityX = 1; // Move right (away from wall)
                            enemy.velocityY = 0;
                        } else {
                            enemy.velocityY = -enemy.velocityY; // Turn around
                        }
                    }
                }
            }
            else if (enemy.surface == 'c') {
                // ===== CEILING MODE =====
                // Current state: surface above at (x, y-1)
                // Movement: horizontal (velocityX = ±1)

                int nextX = enemy.x + enemy.velocityX;

                // Case 4A: Wall blocking ahead (includes boundary walls)
                if (nextX <= 0 || nextX >= ARENA_WIDTH - 1 || isColliding(nextX, enemy.y)) {
                    // Hit a wall - transition to climbing down
                    if (enemy.velocityX > 0) {
                        enemy.surface = 'r';
                        enemy.velocityY = 1; // Descend
                        enemy.velocityX = 0;
                    } else {
                        enemy.surface = 'l';
                        enemy.velocityY = 1; // Descend
                        enemy.velocityX = 0;
                    }
                }
                // Case 4B: Ceiling continues
                else if (isColliding(nextX, enemy.y - 1)) {
                    // Ceiling exists above, move forward
                    enemy.x = nextX;
                }
                // Case 4C: Ceiling edge - move around the edge to get on top
                else {
                    // Check if there's a wall ahead that we should climb instead
                    if (enemy.velocityX > 0 && isColliding(nextX + 1, enemy.y)) {
                        // Wall to the right of the edge, climb it
                        enemy.surface = 'r';
                        enemy.velocityY = 1; // Descend down the wall
                        enemy.velocityX = 0;
                    } else if (enemy.velocityX < 0 && isColliding(nextX - 1, enemy.y)) {
                        // Wall to the left of the edge, climb it
                        enemy.surface = 'l';
                        enemy.velocityY = 1; // Descend down the wall
                        enemy.velocityX = 0;
                    } else {
                        // No wall, start edge wrapping sequence (ceiling to floor)
                        // Example: crawler at (20,12), ceiling at (20,11)
                        // Moving right: (20,12) -> (21,12) -> (21,11) -> (21,10) -> (20,10)
                        // This will happen over 4 frames (using steps 5-8)
                        enemy.edgeWrapStep = 5; // Use 5-8 for ceiling wrapping
                    }
                }
            }
        } else if (enemy.type == 'B') {
            // Boss: AOE attack system
            // State 0: Walking normally
            // State 1: Winding up (5 seconds)
            // State 2: Attack triggered (1 frame)

            if (enemy.attackState == 0) {
                // Walking state - normal movement
                enemy.aiTimer++;
                const int ATTACK_INTERVAL = 30;

                if (enemy.aiTimer >= ATTACK_INTERVAL) {
                    // Start windup
                    enemy.attackState = 1;
                    enemy.windupTimer = 10;
                    enemy.aiTimer = 0;
                } else {
                    // Normal walking behavior
                    int nextX = enemy.x + enemy.velocityX;

                    // Check if next position is valid (Boss is 3x3, so check all tiles)
                    bool canMove = true;
                    if (nextX - 1 < 1 || nextX + 1 >= ARENA_WIDTH - 1) {
                        canMove = false; // Hit boundary
                    } else {
                        // Check if any part of Boss would collide
                        for (int dy = -1; dy <= 1; dy++) {
                            if (isColliding(nextX - 1, enemy.y + dy) ||
                                isColliding(nextX, enemy.y + dy) ||
                                isColliding(nextX + 1, enemy.y + dy)) {
                                canMove = false;
                                break;
                            }
                        }
                    }

                    if (!canMove) {
                        // Hit wall, turn around
                        enemy.velocityX = -enemy.velocityX;
                    } else {
                        // Check if there's ground ahead for all bottom tiles
                        bool hasGround = false;
                        for (int dx = -1; dx <= 1; dx++) {
                            if (isColliding(nextX + dx, enemy.y + 2)) {
                                hasGround = true;
                                break;
                            }
                        }

                        if (!hasGround) {
                            // No ground ahead, turn around
                            enemy.velocityX = -enemy.velocityX;
                        } else {
                            // Safe to move
                            enemy.x = nextX;
                        }
                    }
                }
            } else if (enemy.attackState == 1) {
                // Winding up - Boss stops moving, countdown timer
                enemy.windupTimer--;
                if (enemy.windupTimer <= 0) {
                    // Trigger attack
                    enemy.attackState = 2;
                }
            } else if (enemy.attackState == 2) {
                // Attack frame - deal damage in 11x11 area
                // Check if player is in AOE range (5 tiles from Boss center)
                int distX = (player.x > enemy.x) ? (player.x - enemy.x) : (enemy.x - player.x);
                int distY = (player.y > enemy.y) ? (player.y - enemy.y) : (enemy.y - player.y);

                if (distX <= 5 && distY <= 5) {
                    // Player is in AOE, deal 3 damage
                    player.hp -= 3;
                }

                // Return to walking state
                enemy.attackState = 0;
                enemy.aiTimer = 0; // Reset 10-second timer
            }
        }
    }
}

void updateEnemies() {
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].isActive) continue;

        // Fliers and Crawlers don't obey gravity
        if (enemies[i].type != 'F' && enemies[i].type != 'C') {
            applyEnemyGravity(enemies[i]);
        }
    }

    updateEnemyAI();
}

// Check if active attack hits any enemies and apply damage
void checkAttackHits() {
    if (!currentAttack.isActive) return;

    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].isActive) continue;

        if (isEnemyHitByAttack(enemies[i])) {
            enemies[i].hp--;
            if (enemies[i].hp <= 0) {
                enemies[i].isActive = false;
            }
            currentAttack.isActive = false;
            break;
        }
    }

    // Clean up defeated enemies
    for (int i = enemyCount - 1; i >= 0; i--) {
        if (!enemies[i].isActive) {
            removeEnemy(i);
        }
    }
}


// Check for player-enemy collisions and apply damage
void checkPlayerEnemyCollision() {
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].isActive) continue;

        bool collision = false;

        // Boss has 3x3 hitbox
        if (enemies[i].type == 'B') {
            if (player.x >= enemies[i].x - 1 && player.x <= enemies[i].x + 1 &&
                player.y >= enemies[i].y - 1 && player.y <= enemies[i].y + 1) {
                collision = true;
            }
        }
        // Normal enemies have 1x1 hitbox
        else {
            if (enemies[i].x == player.x && enemies[i].y == player.y) {
                collision = true;
            }
        }

        if (collision) {
            player.hp--;

            // Regular enemies die on contact, Boss doesn't
            if (enemies[i].type != 'B') {
                enemies[i].isActive = false;
                removeEnemy(i);
                i--;
            }
        }
    }
}

// Free dynamically allocated enemy array
void cleanupEnemies() {
    if (enemies != nullptr) {
        delete[] enemies;
        enemies = nullptr;
    }
    enemyCount = 0;
    enemyCapacity = 0;
}

// ========================================
// WAVE MANAGEMENT
// ========================================

// Check if current wave is complete (all enemies defeated)
bool isWaveComplete() {
    return enemyCount == 0;
}

// Spawn enemies for a given wave number
void spawnWave(int waveNumber) {
    int enemiesToSpawn = 0;

    // Wave 1: Tutorial wave with basic enemies
    if (waveNumber == 1) {
        addEnemy('E', 20, ARENA_HEIGHT - 2);
        addEnemy('E', 100, ARENA_HEIGHT - 2);
        totalEnemiesFromPreviousWaves = 2;
        return;
    }
    // Final wave: Boss battle
    else if (waveNumber == MAX_WAVES) {
        int bossX = ARENA_WIDTH / 2;
        int bossY = ARENA_HEIGHT - 3;
        addEnemy('B', bossX, bossY);
        totalEnemiesFromPreviousWaves = 1;
        return;
    }
    // Intermediate waves: increasing difficulty
    else {
        int additionalEnemies = 2 + (rand() % 3);
        enemiesToSpawn = totalEnemiesFromPreviousWaves + additionalEnemies;
        totalEnemiesFromPreviousWaves = enemiesToSpawn;
    }

    char enemyTypes[] = {'E', 'J', 'F', 'C'};

    for (int i = 0; i < enemiesToSpawn; i++) {
        char type = enemyTypes[rand() % 4];
        int spawnX, spawnY;

        if (type == 'F') {
            // Fliers spawn in the air
            spawnY = 2 + rand() % (ARENA_HEIGHT / 2); // top half of arena
            spawnX = 1 + rand() % (ARENA_WIDTH - 2);
        }
        else {
            // Decide randomly: ground or platform
            bool spawnOnGround = (rand() % 2 == 0);

            if (spawnOnGround) {
                spawnY = ARENA_HEIGHT - 2;
                spawnX = 1 + rand() % (ARENA_WIDTH - 2);
            }
            else {
                // Choose a platform row
                int platformYs[] = {ARENA_HEIGHT - 6, ARENA_HEIGHT - 12, ARENA_HEIGHT - 18};
                spawnY = platformYs[rand() % 3];

                // Find valid X positions on this platform
                int validXs[ARENA_WIDTH];
                int count = 0;
                for (int x = 1; x < ARENA_WIDTH - 1; x++) {
                    if (arena[spawnY][x] == '=') validXs[count++] = x;
                }

                if (count == 0) {
                    // fallback to ground
                    spawnY = ARENA_HEIGHT - 2;
                    spawnX = 1 + rand() % (ARENA_WIDTH - 2);
                } else {
                    spawnX = validXs[rand() % count];
                }

                spawnY--; // spawn above platform
            }
        }

        addEnemy(type, spawnX, spawnY);
    }

}
