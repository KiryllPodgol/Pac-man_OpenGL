#ifndef PACMAN_H
#define PACMAN_H

#include "gameMap.h"
#include <cmath>

class Pacman {
private:
    float x, y;
    float startX, startY;
    int dx, dy;
    int nextDx, nextDy;
    float speed;
    int lives;

public:
    Pacman(float startX = 0, float startY = 0)
        : x(startX), y(startY), startX(startX), startY(startY),
        dx(0), dy(0), nextDx(0), nextDy(0),
        speed(0.1f), lives(1) {  // ИЗМЕНЕНО: 1 жизнь вместо 3
    }

    void setDirection(int ndx, int ndy) {
        nextDx = ndx;
        nextDy = ndy;
    }

    void update(const GameMap& map) {
        // Всегда пытаемся сменить направление (буферизация ввода)
        if ((nextDx != 0 || nextDy != 0)) {
            int testX = static_cast<int>(std::round(x)) + nextDx;  // ИСПРАВЛЕНО: округляем текущую позицию
            int testY = static_cast<int>(std::round(y)) + nextDy;

            if (map.canMove(testX, testY)) {
                dx = nextDx;
                dy = nextDy;
            }
            nextDx = 0;
            nextDy = 0;
        }

        // Движение
        if (dx != 0 || dy != 0) {
            float newX = x + dx * speed;
            float newY = y + dy * speed;

            // Правильная проверка следующей клетки
            int currentCellX = static_cast<int>(std::round(x));
            int currentCellY = static_cast<int>(std::round(y));
            int targetCellX = static_cast<int>(std::round(newX));  // ИСПРАВЛЕНО: округляем новую позицию
            int targetCellY = static_cast<int>(std::round(newY));

            // Если мы остаемся в той же клетке ИЛИ переходим в проходимую клетку
            if ((currentCellX == targetCellX && currentCellY == targetCellY) ||
                map.canMove(targetCellX, targetCellY)) {
                x = newX;
                y = newY;
            }
            else {
                // Если уперлись в стену - останавливаемся и выравниваем позицию
                dx = 0;
                dy = 0;
                x = std::round(x);
                y = std::round(y);
            }
        }
    }

    void die() {
        lives--;
    }

    void resetPosition(float sx, float sy) {
        x = sx;
        y = sy;
        startX = sx;
        startY = sy;
        dx = 0;
        dy = 0;
        nextDx = 0;
        nextDy = 0;
    }

    bool isAlive() const { return lives > 0; }
    float getX() const { return x; }
    float getY() const { return y; }
    void setSpeed(float s) { speed = s; }
};

#endif