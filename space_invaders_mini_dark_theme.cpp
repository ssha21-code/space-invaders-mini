#include "raylib.h"
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <atomic>

#define MY_BLACK CLITERAL(Color){2, 0, 26, 255}
#define MY_DARKGREEN (Color){0, 41, 6, 255}
#define MY_DARKRED (Color){41, 11, 0, 255}

constexpr int WINDOW_WIDTH = 1000;
constexpr int WINDOW_HEIGHT = 800;
int velocity = 8;
bool shouldSpawnEnemies = true;
bool startOfNextStage = false;

static std::mt19937 rng(std::random_device{}());
static std::bernoulli_distribution random_bool(0.5);
std::atomic<bool> timer_tick(false);
std::atomic<bool> running(true);
int enemy_shooting_cooldown = 300;

void tick_timer() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(enemy_shooting_cooldown));
        timer_tick = true;
    }
}

void showVictoryScreen() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), MY_DARKGREEN);
    int victory_text_width = MeasureText("VICTORY!", 72);
    int victory_posX = (GetScreenWidth() - victory_text_width) / 2;
    DrawText("VICTORY!", victory_posX, 300, 72, WHITE);
    int text1_text_width = MeasureText("Thanks for playing!", 40);
    int text1_posX = (GetScreenWidth() - text1_text_width) / 2;
    DrawText("Thanks for playing!", text1_posX, 450, 40, WHITE);
    int text2_text_width = MeasureText("Made by: @ssha21-code", 40);
    int text2_posX = (GetScreenWidth() - text2_text_width) / 2;
    DrawText("Made by: @ssha21-code", text2_posX, 520, 40, WHITE);
}
void showDeathScreen() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), MY_DARKRED);
    int death_text_width = MeasureText("YOU DIED!", 72);
    int death_posX = (GetScreenWidth() - death_text_width) / 2;
    DrawText("YOU DIED!", death_posX, 300, 72, WHITE);
}

class Turret {
public: 
    float radius = 10;
    int centerX;
    int centerY;
    bool isPlayerTurret;
    bool shouldBeRemoved = false;
    Turret() {

    }
    void update() {
        if (!isPlayerTurret) {
            centerY += velocity;
        } else {
            centerY -= velocity;
        }
        
    }
    void draw() {
        DrawCircle(centerX, centerY, radius, isPlayerTurret ? DARKBLUE : ORANGE);
    }
    float getRadius() const {
        return radius;
    }
    Vector2 getCenter() const {
        Vector2 center = {(float)centerX, (float)centerY};
        return center;
    }
};

class Player {
public: 
    float width = 150;
    float height = 30;
    float posX = (GetScreenWidth() - width) / 2;
    float posY = GetScreenHeight() - 100;
    float triangle_tip = posY - 20;
    float center = posX + (width / 2);
    int hp = 25;
    Vector2 polygon_points[7];
    Player() {
    
    }
    void paint() {
        center = posX + (width / 2);
        DrawRectangle(posX, posY, width, height, BLUE);
        //Color laser_color = {255, 0, 0, 125};
        //DrawRectangle(center - 5, 0, center - posX - (width / 2) + 10, posY, laser_color);
        DrawTriangle(
            (Vector2){center, triangle_tip},           
            (Vector2){posX + 50, posY},                       
            (Vector2){posX + 100, posY},                      
            BLUE
        );
        DrawTriangle(
            (Vector2){posX, posY + height}, 
            (Vector2){posX, posY + height + 35}, 
            (Vector2){posX + 20, posY + height}, 
            BLUE
        );
        DrawTriangle(
            (Vector2){posX + width, posY + height}, 
            (Vector2){posX + width - 20, posY + height}, 
            (Vector2){posX + width, posY + height + 35}, 
            BLUE
        );
    }
    void moveLeft() {
        if (posX > 0) {
            posX -= velocity;
        }
        center = posX + (width / 2);
    }
    void moveRight() {
        if (posX + width < GetScreenWidth()) {
            posX += velocity;
        }
        center = posX + (width / 2);
    }
    Rectangle getRectangle() {
        Rectangle rect = {posX, posY, width, height};
        return rect;
    }
    void checkCollisions(std::vector<Turret>& turrets) {
        static std::uniform_int_distribution<int> one_in_four{1, 5};
        static std::uniform_int_distribution<int> one_in_three{1, 3};
        for (auto &turret: turrets) {
            if (CheckCollisionCircleRec(turret.getCenter(), turret.getRadius(), getRectangle()) && !turret.isPlayerTurret) {
                int damage = one_in_four(rng);
                turret.shouldBeRemoved = true;
                if (hp > 0) {
                    hp -= damage;
                }
                
            }
        }
        
    }
};

enum class Stage {
    STAGE_ONE = 0, 
    STAGE_TWO, 
    STAGE_THREE,
    VICTORY,
    DEFEAT,
};

class Enemy {
public:
    float width = 100;
    float height = 20;
    float posX;
    float posY;
    bool shouldBeDestroyed = false;
    int center;
    int hp;
    Enemy() {
        
    }
    void draw() {
        DrawRectangle(posX, posY, width, height, RED);
        DrawTriangle(
            (Vector2){posX + 50, posY + height + 15},
            (Vector2){posX + 70, posY + height},
            (Vector2){posX + 30, posY + height},
            RED
        );
        DrawTriangle(
            (Vector2){posX, posY - 25}, 
            (Vector2){posX, posY}, 
            (Vector2){posX + 15, posY}, 
            RED
        );
        DrawTriangle(
            (Vector2){posX + width, posY - 25}, 
            (Vector2){posX + width - 15, posY}, 
            (Vector2){posX + width, posY}, 
            RED
        );
        char hp_label[4];
        sprintf(hp_label, "%d", hp);
        DrawText(hp_label, posX + (width / 2 - 10), posY, 30, WHITE);
    }
    Rectangle getRectangle() {
        Rectangle rectangle = {posX, posY, width, height};
        return rectangle;
    }

};

class SpawnTurrets {
public: 
    std::vector<Turret> turrets = {};
    void addTurretsEnemy(const Enemy enemy){
        Turret turret = Turret();
        turret.centerX = enemy.center;
        turret.centerY = enemy.posY + enemy.height + 20;
        turret.isPlayerTurret = false;
        turrets.push_back(turret);
    }
    void addTurretsPlayer(const Player player) {
        Turret turret = Turret();
        turret.centerX = player.center;
        turret.centerY = player.posY - player.height - 20;
        turret.isPlayerTurret = true;
        turrets.push_back(turret);
    }
    void paint() {
        for (auto &turret: turrets) {
            turret.draw();
        }
    }
    void update() {
        for (auto &turret: turrets) {
            turret.update();
            if (turret.centerY - turret.radius > GetScreenHeight() || turret.centerY + turret.radius < 0) {
                turret.shouldBeRemoved = true;
            }
        }
    }
    void checkForRemoval() {
        for (int i = 0; i < turrets.size(); i++) {
            if (turrets.at(i).shouldBeRemoved) {
                turrets.erase(turrets.begin() + i);
                i--;
            }
        }
    }
    const std::vector<Turret>& getTurrets() const {
        return turrets;
    }
    std::vector<Turret>& getTurretsModify() { 
        return turrets; 
    }
};

class SpawnEnemies {
public: 
    std::vector<Enemy> enemies = {};
    int spawn_number = 0;
    Stage stage = Stage::STAGE_ONE;
    SpawnEnemies() {

    }
    std::vector<Enemy>& getEnemies() {
        return enemies;
    }
    void setSpawnNumber() {
        switch (stage) {
            case Stage::STAGE_ONE:
                spawn_number = 5;
                enemy_shooting_cooldown = 300;
                break;
            case Stage::STAGE_TWO:
                spawn_number = 10;
                enemy_shooting_cooldown = 250;
                break;
            case Stage::STAGE_THREE:
                spawn_number = 15;
                enemy_shooting_cooldown = 200;
                break;
            default:
                spawn_number = 1;
        }
    }
    void addEnemies() {
        for (int i = 0; i < spawn_number; i++) {
            enemies.push_back(Enemy());
        }
    }
    void setEnemiesPosition() {
        for (int i = 0; i < spawn_number; i++) {
            switch (stage) {
                case Stage::STAGE_ONE: {
                    enemies.at(i).posX = 225 + (110 * i);
                    enemies.at(i).posY = 100;
                    enemies.at(i).center = enemies.at(i).posX + (enemies.at(i).width / 2);
                    enemies.at(i).hp = 2;
                    break;
                }
                case Stage::STAGE_TWO: {
                    if (i > 4) {
                        enemies.at(i).posY = 180;
                        enemies.at(i).posX = 225 + (110 * (i - 5));
                    }
                    if (i <= 4) {
                        enemies.at(i).posY = 100;
                        enemies.at(i).posX = 225 + (110 * i);
                    }
                    enemies.at(i).center = enemies.at(i).posX + (enemies.at(i).width / 2);
                    enemies.at(i).hp = 3;
                    break;
                }
                case Stage::STAGE_THREE: {
                    if (i > 9) {
                        enemies.at(i).posY = 260;
                        enemies.at(i).posX = 225 + (110 * (i - 10));
                    } else if (i > 4) {
                        enemies.at(i).posY = 180;
                        enemies.at(i).posX = 225 + (110 * (i - 5));
                    }
                    if (i <= 4) {
                        enemies.at(i).posY = 100;
                        enemies.at(i).posX = 225 + (110 * i);
                    }
                    enemies.at(i).center = enemies.at(i).posX + (enemies.at(i).width / 2);
                    enemies.at(i).hp = 4;
                    break;
                }
                case Stage::VICTORY: {
                    enemies.clear();
                    break;
                }
            }
        }
    }
    void paintEnemies() {
        for (auto &e : enemies) {
            e.draw();
        }
    }
    
    void checkForRemoval() {
        for (int i = 0; i < enemies.size(); i++) {
            if (enemies.at(i).shouldBeDestroyed) {
                enemies.erase(enemies.begin() + i);
                i--;
            }
        }
    }
    void checkCollision(SpawnTurrets &spawn_turrets) {
        auto &turrets = spawn_turrets.getTurretsModify();
        for (auto &enemy: enemies) {
            if (enemy.shouldBeDestroyed) continue;
            for (auto &turret: turrets) {
                if (CheckCollisionCircleRec(turret.getCenter(), turret.getRadius(), enemy.getRectangle())) {
                    if (!turret.isPlayerTurret) continue;
                    if (turret.isPlayerTurret) {
                        bool destroyed = false;
                        switch (stage) {
                            case Stage::STAGE_ONE: {
                                enemy.hp--;
                                destroyed = (enemy.hp <= 0);
                                break;
                            }
                            case Stage::STAGE_TWO: {
                                enemy.hp--;
                                destroyed = (enemy.hp <= 0);
                                break;
                            }
                            case Stage::STAGE_THREE: {
                                enemy.hp--;
                                destroyed = (enemy.hp <= 0);
                                break;
                            }
                        }
                        if (destroyed) enemy.shouldBeDestroyed = true;
                        turret.shouldBeRemoved = true;
                        break;
                    }
                }
            }
        }
    }
    bool setStage() {
        if (enemies.empty()) {
            switch (stage) {
                case Stage::STAGE_ONE: {
                    stage = Stage::STAGE_TWO;
                    startOfNextStage = true;
                    return true;
                }
                case Stage::STAGE_TWO: {
                    stage = Stage::STAGE_THREE;
                    startOfNextStage = true;
                    return true;
                }
                case Stage::STAGE_THREE: {
                    stage = Stage::VICTORY;
                    startOfNextStage = true;
                    return true;
                }
                default: {
                    return false;
                }
            }
        }
        return false;
    }
};



int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Shooter Game");
    SetTargetFPS(60);
    std::thread timer_thread(tick_timer);
    Player player = Player();
    SpawnEnemies spawn_enemies = SpawnEnemies();
    SpawnTurrets spawn_turrets = SpawnTurrets();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(MY_BLACK);
        if (IsKeyDown(KEY_A)) player.moveLeft();
        if (IsKeyDown(KEY_D)) player.moveRight();
        if (IsKeyPressed(KEY_SPACE)) {
            spawn_turrets.addTurretsPlayer(player);
        }
        if (timer_tick) {
            timer_tick = false;
            auto &enemies = spawn_enemies.getEnemies();
            if (!enemies.empty()) {
                std::uniform_int_distribution<std::size_t> enemy_chooser(0, enemies.size() - 1);
                std::size_t idx = enemy_chooser(rng);
                Enemy &enemy = enemies.at(idx);
                spawn_turrets.addTurretsEnemy(enemy);
            }
        }
        if (shouldSpawnEnemies) {
            spawn_enemies.setSpawnNumber();
            spawn_enemies.addEnemies();
            spawn_enemies.setEnemiesPosition();
            shouldSpawnEnemies = false;
        }
        char hp_label[16];
        sprintf(hp_label, "Health: %d", player.hp);
        DrawText(hp_label, 25, 80, 36, WHITE);
        spawn_turrets.update();
        spawn_turrets.paint();
        spawn_turrets.checkForRemoval();
        player.paint();
        player.checkCollisions(spawn_turrets.getTurretsModify());
        spawn_enemies.paintEnemies();
        if (spawn_enemies.setStage()) {
            shouldSpawnEnemies = true;
        }
        if (startOfNextStage) {
            switch (spawn_enemies.stage) {
                case Stage::STAGE_ONE: {
                    player.hp = 30;
                    break;
                }
                case Stage::STAGE_TWO: {
                    player.hp = 50;
                    break;
                }
                case Stage::STAGE_THREE: {
                    player.hp = 100;
                    break;
                }
            }
            spawn_turrets.getTurretsModify().clear();
            startOfNextStage = false;
        }
        if (spawn_enemies.stage == Stage::VICTORY) {
            player.posX = 0;
            showVictoryScreen();
        }
        if (player.hp <= 0) {
            player.hp = 0;
            showDeathScreen();
        }
        spawn_enemies.checkCollision(spawn_turrets);
        spawn_enemies.checkForRemoval();
        EndDrawing();
    }

    CloseWindow();
    running = false;
    timer_thread.join();
    std::abort();
    return 0;
}