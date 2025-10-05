#ifndef GHOST_H
#define GHOST_H

#include "gameMap.h"
#include "pacman.h"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iostream>

enum GhostColor {
    RED,    // Blinky
    PINK,   // Pinky
    CYAN,   // Inky
    ORANGE  // Clyde
};

class Ghost {
private:
    float x, y;
    float speed;
    int dx, dy;
    bool vulnerable;
    float respawnX, respawnY;
    GhostColor color;
    int frightenedTimer;
    int modeTimer;
    bool inScatterMode;
    int scatterChaseCycle; // Счётчик циклов scatter/chase
    bool modeJustChanged;  // Флаг смены режима для принудительного разворота

    // Получаем целочисленные координаты текущей клетки
    int getCurrentTileX() const { return static_cast<int>(std::round(x)); }
    int getCurrentTileY() const { return static_cast<int>(std::round(y)); }

    // Проверяем, находится ли призрак в центре клетки
    bool isAtIntersection() const {
        return std::abs(x - getCurrentTileX()) < 0.05f &&
            std::abs(y - getCurrentTileY()) < 0.05f;
    }

    // Выравниваем позицию к центру клетки
    void alignToGrid() {
        x = getCurrentTileX();
        y = getCurrentTileY();
    }

    // Получаем целевую позицию для преследования
    std::pair<int, int> getChaseTarget(const Pacman& pacman) {
        int pacmanX = static_cast<int>(std::round(pacman.getX()));
        int pacmanY = static_cast<int>(std::round(pacman.getY()));
        int pacmanDx = pacman.getDirectionX();
        int pacmanDy = pacman.getDirectionY();

        switch (color) {
        case RED: // Blinky - прямое преследование
            return { pacmanX, pacmanY };

        case PINK: // Pinky - на 4 клетки впереди Пакмана
            return { pacmanX + pacmanDx * 4, pacmanY + pacmanDy * 4 };

        case CYAN: // Inky - зеркальная позиция относительно Blinky
        {
            // Более точная версия Inky
            int blinkyX = getCurrentTileX(); // Примерно позиция Blinky
            int blinkyY = getCurrentTileY();
            int targetX = pacmanX + pacmanDx * 2;
            int targetY = pacmanY + pacmanDy * 2;
            return { targetX + (targetX - blinkyX), targetY + (targetY - blinkyY) };
        }

        case ORANGE: // Clyde - преследует на расстоянии, убегает вблизи
        {
            float distance = std::sqrt(std::pow(x - pacman.getX(), 2) +
                std::pow(y - pacman.getY(), 2));
            if (distance < 8.0f) {
                return getScatterTarget(); // Убегает в свой scatter-угол
            }
            else {
                return { pacmanX, pacmanY };
            }
        }
        }
        return { pacmanX, pacmanY };
    }

    // Получаем целевую позицию для режима scatter
    std::pair<int, int> getScatterTarget() {
        switch (color) {
        case RED:    return { 25, -3 };   // Правый верхний (за картой)
        case PINK:   return { 2, -3 };    // Левый верхний (за картой)
        case CYAN:   return { 27, 30 };   // Правый нижний (за картой)
        case ORANGE: return { 0, 30 };    // Левый нижний (за картой)
        }
        return { 0, 30 };
    }

    // Проверяем специальные ограничения для туннелей
    bool isRestrictedTunnel(int tileX, int tileY) const {
        // Желтые клетки где нельзя поворачивать вверх
        return (tileY == 17 && (tileX == 12 || tileX == 15));
    }

    // Выбираем лучшее направление движения согласно оригинальной механике
    void chooseBestDirection(const GameMap& map, const Pacman& pacman) {
        if (!isAtIntersection()) {
            return;
        }

        alignToGrid();

        // Принудительный разворот при смене режима (кроме выхода из frightened)
        if (modeJustChanged && !vulnerable) {
            dx = -dx;
            dy = -dy;
            modeJustChanged = false;
            return;
        }

        std::pair<int, int> target;
        if (vulnerable) {
            // В режиме испуга - случайное движение
            chooseRandomDirection(map);
            return;
        }
        else if (inScatterMode) {
            target = getScatterTarget();
        }
        else {
            target = getChaseTarget(pacman);
        }

        // Все возможные направления с приоритетами
        std::vector<std::pair<int, int>> directions = {
            {0, -1},  // Вверх (наивысший приоритет)
            {-1, 0},   // Влево
            {0, 1},    // Вниз  
            {1, 0}     // Вправо (наименьший приоритет)
        };

        std::vector<std::pair<int, int>> validDirections;
        std::vector<float> distances;

        int currentX = getCurrentTileX();
        int currentY = getCurrentTileY();

        for (const auto& dir : directions) {
            int newDx = dir.first;
            int newDy = dir.second;

            // Запрещаем обратное направление (кроме случаев когда нет выбора)
            if (newDx == -dx && newDy == -dy) {
                continue;
            }

            // Проверяем специальные ограничения для туннелей
            if (isRestrictedTunnel(currentX, currentY) && newDy == -1) { // Вверх
                continue;
            }

            // Проверяем возможность движения
            int testX = currentX + newDx;
            int testY = currentY + newDy;

            if (map.canMove(testX, testY)) {
                // Вычисляем расстояние по прямой до цели
                float distance = std::sqrt(std::pow(target.first - testX, 2) +
                    std::pow(target.second - testY, 2));
                validDirections.push_back({ newDx, newDy });
                distances.push_back(distance);
            }
        }

        if (!validDirections.empty()) {
            // Находим минимальное расстояние
            float minDistance = *std::min_element(distances.begin(), distances.end());

            // Выбираем первое направление с минимальным расстоянием (учитывая приоритет)
            for (size_t i = 0; i < validDirections.size(); ++i) {
                if (distances[i] == minDistance) {
                    dx = validDirections[i].first;
                    dy = validDirections[i].second;
                    break;
                }
            }
        }
        else {
            // Если нет других путей, пробуем идти назад
            if (map.canMove(currentX - dx, currentY - dy)) {
                dx = -dx;
                dy = -dy;
            }
        }
    }

    void chooseRandomDirection(const GameMap& map) {
        if (!isAtIntersection()) {
            return;
        }

        alignToGrid();

        std::vector<std::pair<int, int>> validDirections;
        std::vector<std::pair<int, int>> directions = {
            {0, -1}, {-1, 0}, {0, 1}, {1, 0} // С приоритетами
        };

        int currentX = getCurrentTileX();
        int currentY = getCurrentTileY();

        for (const auto& dir : directions) {
            int testX = currentX + dir.first;
            int testY = currentY + dir.second;

            if (map.canMove(testX, testY)) {
                // В режиме испуга игнорируем ограничения туннелей
                validDirections.push_back(dir);
            }
        }

        if (!validDirections.empty()) {
            auto randomDir = validDirections[rand() % validDirections.size()];
            dx = randomDir.first;
            dy = randomDir.second;
        }
    }

    void updateMode() {
        if (vulnerable) {
            frightenedTimer--;
            if (frightenedTimer <= 0) {
                vulnerable = false;
                // При выходе из frightened НЕ разворачиваемся
            }
        }

        // Обновляем таймер режима
        modeTimer--;

        // Волны scatter/chase согласно оригинальной механике
        if (modeTimer <= 0) {
            if (inScatterMode) {
                // Завершился scatter-режим, переходим в chase
                inScatterMode = false;
                scatterChaseCycle++;

                // Устанавливаем длительность chase-режима
                if (scatterChaseCycle < 4) {
                    modeTimer = 20 * 60; // 20 секунд (60 FPS)
                }
                else {
                    modeTimer = -1; // Бесконечный chase
                }
            }
            else {
                // Завершился chase-режим, переходим в scatter
                inScatterMode = true;

                // Устанавливаем длительность scatter-режима
                if (scatterChaseCycle < 2) {
                    modeTimer = 7 * 60; // 7 секунд
                }
                else {
                    modeTimer = 5 * 60; // 5 секунд
                }
            }

            modeJustChanged = true;
        }
    }

public:
    Ghost(float startX, float startY, GhostColor ghostColor)
        : x(startX), y(startY - 3), speed(0.08f), dx(0), dy(0),
        vulnerable(false), respawnX(startX), respawnY(startY - 3),
        color(ghostColor), frightenedTimer(0), modeTimer(7 * 60),
        inScatterMode(true), scatterChaseCycle(0), modeJustChanged(false) {
    }

    void update(const GameMap& map, const Pacman& pacman) {
        updateMode();
        chooseBestDirection(map, pacman);

        // Движение
        float newX = x + dx * speed;
        float newY = y + dy * speed;

        if (dx != 0 || dy != 0) {
            if (map.canMove(newX, newY)) {
                x = newX;
                y = newY;
            }
            else {
                alignToGrid();
                dx = 0;
                dy = 0;
            }
        }
    }

    void setVulnerable(bool isVulnerable) {
        if (isVulnerable && !vulnerable) {
            vulnerable = true;
            frightenedTimer = 6 * 60;
            // При входе в frightened разворачиваемся
            dx = -dx;
            dy = -dy;
        }
        else if (!isVulnerable && vulnerable) {
            vulnerable = false;
            frightenedTimer = 0;
            // При выходе из frightened НЕ разворачиваемся
        }
    }

    bool isVulnerable() const { return vulnerable; }
    GhostColor getColor() const { return color; }
    float getX() const { return x; }
    float getY() const { return y; }
    int getDirectionX() const { return dx; }
    int getDirectionY() const { return dy; }

    void respawn(int mapWidth, int mapHeight) {
        x = respawnX;
        y = respawnY;
        dx = 0;
        dy = 0;
        vulnerable = false;
        inScatterMode = true;
        modeTimer = 7 * 60;
        frightenedTimer = 0;
        scatterChaseCycle = 0;
        modeJustChanged = false;
        alignToGrid();
    }

    void resetPosition(float newX, float newY) {
        x = newX;
        y = newY - 3;
        dx = 0;
        dy = 0;
        vulnerable = false;
        inScatterMode = true;
        modeTimer = 7 * 60;
        frightenedTimer = 0;
        scatterChaseCycle = 0;
        modeJustChanged = false;
    }

    void setSpeed(float newSpeed) { speed = newSpeed; }
};

#endif