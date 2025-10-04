#ifndef GAME_H
#define GAME_H

#include "pacman.h"
#include "ghost.h"
#include "gameMap.h"
#include <vector>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <iostream>

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

        // Размещаем призраков в разных стартовых позициях
        ghosts.push_back(Ghost(9.0f, 10.0f, RED));    // Blinky - центр
        ghosts.push_back(Ghost(8.0f, 10.0f, PINK));   // Pinky  
        ghosts.push_back(Ghost(10.0f, 10.0f, CYAN));  // Inky
        ghosts.push_back(Ghost(9.0f, 9.0f, ORANGE));  // Clyde

        // Даем им начальные направления
        ghosts[0].setSpeed(0.08f); // Blinky
        ghosts[1].setSpeed(0.075f); // Pinky  
        ghosts[2].setSpeed(0.07f); // Inky
        ghosts[3].setSpeed(0.065f); // Clyde
    }

    void startGame() {
        if (!gameStarted && !gameOver) {
            gameStarted = true;
        }
    }

    void update() {
        if (gameOver || levelComplete || !gameStarted) return;

        // Обновляем таймер режима силы и мигания
        if (powerMode) {
            powerModeTimer--;
            flashTimer++;

            // Мигание каждые 10 кадров
            if (flashTimer >= 10) {
                flashTimer = 0;
            }

            if (powerModeTimer <= 0) {
                powerMode = false;
                ghostsVulnerable = false;
                std::cout << "POWER MODE ENDED!" << std::endl;
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
        float pacmanX = pacman.getX();
        float pacmanY = pacman.getY();

        for (auto& ghost : ghosts) {
            float ghostX = ghost.getX();
            float ghostY = ghost.getY();

            // Проверяем столкновение по области
            float distance = std::sqrt(std::pow(pacmanX - ghostX, 2) + std::pow(pacmanY - ghostY, 2));
            float collisionRadius = 0.7f;

            if (distance < collisionRadius) {
                if (ghostsVulnerable) {
                    // В режиме силы Пакман ест призраков
                    std::cout << "Pacman ate ghost! Score +200" << std::endl;
                    ghost.respawn(map.getWidth(), map.getHeight());
                    score += 200;
                    highScore = std::max(score, highScore);
                }
                else {
                    // Обычный режим - Пакман умирает
                    std::cout << "Pacman died!" << std::endl;
                    pacman.die();
                    if (!pacman.isAlive()) {
                        gameOver = true;
                        std::cout << "GAME OVER!" << std::endl;
                    }
                    pacman.resetPosition(map.getWidth() / 2.0f, 1.0f);

                    // Сбрасываем всех призраков
                    for (auto& g : ghosts) {
                        g.resetPosition(g.getX(), g.getY());
                    }
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

        std::cout << "POWER MODE ACTIVATED! Ghosts are vulnerable for 10 seconds." << std::endl;

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