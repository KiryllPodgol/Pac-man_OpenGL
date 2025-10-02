#ifndef GAME_H
#define GAME_H

#include "pacman.h"
#include "ghost.h"
#include "gameMap.h"
#include <vector>
#include <ctime>
#include <algorithm>

class Game {
private:
    Pacman pacman;
    std::vector<Ghost> ghosts;
    GameMap map;
    int level;
    int score;
    int highScore;
    bool gameOver;
    bool levelComplete;
    bool gameStarted;
    bool powerMode;
    int powerModeTimer;
    bool ghostsVulnerable; 
    int flashTimer; 

public:
    Game(int width, int height) :
        map(width, height),
        pacman(width / 2.0f, 1.0f),
        level(1),
        score(0),
        highScore(0),
        gameOver(false),
        levelComplete(false),
        gameStarted(false),
        powerMode(false),
        powerModeTimer(0),
        ghostsVulnerable(false),
        flashTimer(0)
    {
        initializeGhosts();
    }

    void initializeGhosts() {
        ghosts.clear();
        int mapWidth = map.getWidth();
        int mapHeight = map.getHeight();

        ghosts.push_back(Ghost(1.0f, mapHeight - 2.0f));
        ghosts.push_back(Ghost(mapWidth - 2.0f, mapHeight - 2.0f));
        ghosts.push_back(Ghost(mapWidth / 2.0f, mapHeight - 2.0f));
    }

    void startGame() {
        if (!gameStarted && !gameOver) {
            gameStarted = true;
        }
    }

    void update() {
        if (gameOver || levelComplete || !gameStarted) return;

      
        if (powerMode) {
            powerModeTimer--;
            flashTimer++;

            
            if (flashTimer >= 10) {
                flashTimer = 0;
            }

            if (powerModeTimer <= 0) {
                powerMode = false;
                ghostsVulnerable = false;
            }
        }

        pacman.update(map);
        checkCoinCollection();
        checkPowerPointCollection();

        for (auto& ghost : ghosts) {
            ghost.update(map, pacman);
        }

        checkCollisions();
        checkLevelCompletion();
    }

    void checkCollisions() {
        for (auto& ghost : ghosts) {
            if (static_cast<int>(pacman.getX()) == static_cast<int>(ghost.getX()) &&
                static_cast<int>(pacman.getY()) == static_cast<int>(ghost.getY())) {

                if (ghostsVulnerable && ghost.isVulnerable()) {
                    // Пакман ест призрака
                    ghost.setVulnerable(false);
                    ghost.respawn(map.getWidth(), map.getHeight());
                    score += 200;
                    highScore = std::max(score, highScore);
                }
                else if (!ghostsVulnerable) {
                    // Обычный режим - Пакман умирает
                    pacman.die();
                    if (!pacman.isAlive()) {
                        gameOver = true;
                    }
                    pacman.resetPosition(map.getWidth() / 2.0f, 1.0f);
                    initializeGhosts();
                    powerMode = false;
                    ghostsVulnerable = false;
                }
                break;
            }
        }
    }

    void checkCoinCollection() {
        int pacmanX = static_cast<int>(pacman.getX() + 0.5f);
        int pacmanY = static_cast<int>(pacman.getY() + 0.5f);

        if (map.hasCoin(pacmanX, pacmanY)) {
            map.collectCoin(pacmanX, pacmanY);
            score += 10;
            highScore = std::max(score, highScore);
        }
    }

    void checkPowerPointCollection() {
        int pacmanX = static_cast<int>(pacman.getX() + 0.5f);
        int pacmanY = static_cast<int>(pacman.getY() + 0.5f);

        if (map.hasPowerPoint(pacmanX, pacmanY)) {
            map.collectCoin(pacmanX, pacmanY);
            score += 50;
            highScore = std::max(score, highScore);
            activatePowerMode();
        }
    }

    void activatePowerMode() {
        powerMode = true;
        powerModeTimer = 300; // 300 кадров = ~10 секунд
        ghostsVulnerable = true;
        flashTimer = 0;

        // Делаем всех призраков уязвимыми
        for (auto& ghost : ghosts) {
            ghost.setVulnerable(true);
        }
    }

    void checkLevelCompletion() {
        if (map.countRemainingCoins() == 0) {
            levelComplete = true;
        }
    }

    void setPacmanDirection(int dx, int dy) {
        if (gameOver || levelComplete) return;
        pacman.setDirection(dx, dy);
    }

    void nextLevel() {
        if (!levelComplete) return;
        level++;
        levelComplete = false;
        gameStarted = false;
        powerMode = false;
        powerModeTimer = 0;
        ghostsVulnerable = false;
        map.initializeClassicMap();
        pacman.resetPosition(map.getWidth() / 2.0f, 1.0f);
        initializeGhosts();
        pacman.setSpeed(0.1f + level * 0.01f);
    }

    void restart() {
        level = 1;
        score = 0;
        gameOver = false;
        levelComplete = false;
        gameStarted = false;
        powerMode = false;
        powerModeTimer = 0;
        ghostsVulnerable = false;
        map.initializeClassicMap();
        pacman.resetPosition(map.getWidth() / 2.0f, 1.0f);
        initializeGhosts();
        pacman.setSpeed(0.1f);
    }

    // Геттеры
    const Pacman& getPacman() const { return pacman; }
    const std::vector<Ghost>& getGhosts() const { return ghosts; }
    const GameMap& getMap() const { return map; }
    int getLevel() const { return level; }
    int getScore() const { return score; }
    int getHighScore() const { return highScore; }
    bool isGameOver() const { return gameOver; }
    bool isLevelComplete() const { return levelComplete; }
    bool isGameStarted() const { return gameStarted; }
    bool isPowerMode() const { return powerMode; }
    bool areGhostsVulnerable() const { return ghostsVulnerable; }
    int getFlashTimer() const { return flashTimer; }
};

#endif