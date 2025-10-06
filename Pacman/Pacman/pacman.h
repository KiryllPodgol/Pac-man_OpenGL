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
    float mouthAngle;
    float mouthSpeed;
    bool mouthOpening;
    float rotationY;

public:
    Pacman(float startX = 0, float startY = 0)
        : x(startX), y(startY), startX(startX), startY(startY),
        dx(0), dy(0), nextDx(0), nextDy(0),
        speed(0.1f), lives(1),
        mouthAngle(0.0f), mouthSpeed(0.08f), mouthOpening(true),
        rotationY(0) {
    }

    void setDirection(int ndx, int ndy) {
        nextDx = ndx;
        nextDy = ndy;

        
        if (ndx == 1) rotationY = 0;      // Вправо
        else if (ndx == -1) rotationY = 180; // Влево
        else if (ndy == 1) rotationY = 90;   // Вверх
        else if (ndy == -1) rotationY = 270; // Вниз
    }

    void update(const GameMap& map) {
        // Анимация рта
        updateMouthAnimation();

        // Всегда пытаемся сменить направление (буферизация ввода)
        if ((nextDx != 0 || nextDy != 0)) {
            int testX = static_cast<int>(std::round(x)) + nextDx;
            int testY = static_cast<int>(std::round(y)) + nextDy;

            if (map.canMove(testX, testY)) {
                dx = nextDx;
                dy = nextDy;

                
                if (dx == 1) rotationY = 0;      // Вправо
                else if (dx == -1) rotationY = 180; // Влево
                else if (dy == 1) rotationY = 90;   // Вверх
                else if (dy == -1) rotationY = 270; // Вниз
            }
            nextDx = 0;
            nextDy = 0;
        }

        // Движение
        if (dx != 0 || dy != 0) {
            float newX = x + dx * speed;
            float newY = y + dy * speed;

            int currentCellX = static_cast<int>(std::round(x));
            int currentCellY = static_cast<int>(std::round(y));
            int targetCellX = static_cast<int>(std::round(newX));
            int targetCellY = static_cast<int>(std::round(newY));

            if ((currentCellX == targetCellX && currentCellY == targetCellY) ||
                map.canMove(targetCellX, targetCellY)) {
                x = newX;
                y = newY;
            }
            else {
                dx = 0;
                dy = 0;
                x = std::round(x);
                y = std::round(y);
            }
        }
    }

    void updateMouthAnimation() {
        if (mouthOpening) {
            mouthAngle += mouthSpeed;
            if (mouthAngle >= 1.0f) {
                mouthOpening = false;
            }
        }
        else {
            mouthAngle -= mouthSpeed;
            if (mouthAngle <= 0.05f) {
                mouthOpening = true;
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
        mouthAngle = 0.0f;
        mouthOpening = true;
        rotationY = 0;
    }

    bool isAlive() const { return lives > 0; }
    float getX() const { return x; }
    float getY() const { return y; }
    void setSpeed(float s) { speed = s; }

    // Геттеры для анимации рта и направления
    float getMouthAngle() const { return mouthAngle; }
    int getDirectionX() const { return dx; }
    int getDirectionY() const { return dy; }

    // Геттеры для 3D
    float getRotationY() const { return rotationY; }
    float getSpeed() const { return speed; }
};

#endif