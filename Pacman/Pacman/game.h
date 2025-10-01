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

public:
    Game(int width, int height) :
        map(width, height),
        pacman(width / 2.0f, 1.0f),
        level(1),
        score(0),
        highScore(0),
        gameOver(false),
        levelComplete(false),
        gameStarted(false)
    {
        initializeGhosts();
    }

    void initializeGhosts() {
        ghosts.clear();
        int mapWidth = map.getWidth();
        int mapHeight = map.getHeight();

        // Ставим призраков в верхних углах и в центре вверху
        ghosts.push_back(Ghost(1.0f, mapHeight - 2.0f));                     // Верхний левый
        ghosts.push_back(Ghost(mapWidth - 2.0f, mapHeight - 2.0f));          // Верхний правый
        ghosts.push_back(Ghost(mapWidth / 2.0f, mapHeight - 2.0f));          // Верхний центр
    }

    void startGame() {
        if (!gameStarted && !gameOver) {
            gameStarted = true;
        }
    }

    void update() {
        if (gameOver || levelComplete || !gameStarted) return;

        pacman.update(map);
        checkCoinCollection();

        for (auto& ghost : ghosts) {
            ghost.update(map, pacman);
        }

        checkCollisions();
        checkLevelCompletion();
    }

    void checkCollisions() {
        for (const auto& ghost : ghosts) {
            if (static_cast<int>(pacman.getX()) == static_cast<int>(ghost.getX()) &&
                static_cast<int>(pacman.getY()) == static_cast<int>(ghost.getY())) {
                pacman.die();
                if (!pacman.isAlive()) {
                    gameOver = true;
                }

                pacman.resetPosition(map.getWidth() / 2.0f, map.getHeight() / 1.0f);
                initializeGhosts();
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
};

#endif