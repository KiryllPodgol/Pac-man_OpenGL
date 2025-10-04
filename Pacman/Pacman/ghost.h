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
            // Упрощенная версия - преследование с небольшим смещением
            return { pacmanX - pacmanDx * 2, pacmanY - pacmanDy * 2 };
        }

        case ORANGE: // Clyde - преследует на расстоянии, убегает вблизи
        {
            float distance = std::sqrt(std::pow(x - pacman.getX(), 2) +
                std::pow(y - pacman.getY(), 2));
            if (distance < 8.0f) {
                return { 1, 1 }; // Убегает в левый нижний угол
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
        case RED:    return { 17, 19 }; // Правый верхний
        case PINK:   return { 1, 19 };  // Левый верхний
        case CYAN:   return { 17, 1 };  // Правый нижний
        case ORANGE: return { 1, 1 };   // Левый нижний
        }
        return { 1, 1 };
    }

    // Выбираем лучшее направление движения
    void chooseBestDirection(const GameMap& map, const Pacman& pacman) {
        if (!isAtIntersection()) {
            return; // Меняем направление только на пересечениях
        }

        alignToGrid();

        std::pair<int, int> target;
        if (vulnerable) {
            // В уязвимом режиме - случайное движение
            chooseRandomDirection(map);
            return;
        }
        else if (inScatterMode) {
            target = getScatterTarget();
        }
        else {
            target = getChaseTarget(pacman);
        }

        // Все возможные направления
        std::vector<std::pair<int, int>> directions = {
            {1, 0}, {-1, 0}, {0, 1}, {0, -1}
        };

        std::vector<std::pair<int, int>> validDirections;
        std::vector<float> distances;

        for (const auto& dir : directions) {
            int newDx = dir.first;
            int newDy = dir.second;

            // Пропускаем обратное направление (кроме случаев когда нет выбора)
            if (newDx == -dx && newDy == -dy) {
                continue;
            }

            // Проверяем возможность движения
            int testX = getCurrentTileX() + newDx;
            int testY = getCurrentTileY() + newDy;

            if (map.canMove(testX, testY)) {
                float distance = std::sqrt(std::pow(target.first - testX, 2) +
                    std::pow(target.second - testY, 2));
                validDirections.push_back({ newDx, newDy });
                distances.push_back(distance);
            }
        }

        if (!validDirections.empty()) {
            // Выбираем направление с минимальным расстоянием до цели
            auto minIt = std::min_element(distances.begin(), distances.end());
            int bestIndex = std::distance(distances.begin(), minIt);
            dx = validDirections[bestIndex].first;
            dy = validDirections[bestIndex].second;
        }
        else {
            // Если нет других путей, пробуем идти назад
            if (map.canMove(getCurrentTileX() - dx, getCurrentTileY() - dy)) {
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
            {1, 0}, {-1, 0}, {0, 1}, {0, -1}
        };

        for (const auto& dir : directions) {
            int testX = getCurrentTileX() + dir.first;
            int testY = getCurrentTileY() + dir.second;

            if (map.canMove(testX, testY)) {
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
                std::cout << "Ghost is no longer vulnerable" << std::endl;
            }
        }

        // Переключение между scatter и chase режимами
        modeTimer--;
        if (modeTimer <= 0) {
            inScatterMode = !inScatterMode;
            modeTimer = inScatterMode ? 350 : 1000; // Scatter: 7 сек, Chase: 20 сек
        }
    }

public:
    Ghost(float startX, float startY, GhostColor ghostColor)
        : x(startX), y(startY), speed(0.08f), dx(0), dy(0),
        vulnerable(false), respawnX(startX), respawnY(startY),
        color(ghostColor), frightenedTimer(0), modeTimer(350),
        inScatterMode(true) {
    }

    void update(const GameMap& map, const Pacman& pacman) {
        updateMode();
        chooseBestDirection(map, pacman);

        // Движение
        float newX = x + dx * speed;
        float newY = y + dy * speed;

        // Проверяем возможность движения в выбранном направлении
        if (dx != 0 || dy != 0) {
            if (map.canMove(newX, newY)) {
                x = newX;
                y = newY;
            }
            else {
                // Если не можем двигаться, останавливаемся и выравниваем
                alignToGrid();
                dx = 0;
                dy = 0;
            }
        }
    }

    void setVulnerable(bool isVulnerable) {
        if (isVulnerable) {
            vulnerable = true;
            frightenedTimer = 180; // 6 секунд уязвимости
           
            dx = -dx;
            dy = -dy;
        }
        else {
            vulnerable = false;
            frightenedTimer = 0;
        }
    }

    bool isVulnerable() const {
        return vulnerable;
    }

    GhostColor getColor() const {
        return color;
    }

    void respawn(int mapWidth, int mapHeight) {
        // Возвращаем призрака на стартовую позицию с небольшим случайным смещением
        x = respawnX + (rand() % 3 - 1) * 0.5f;
        y = respawnY + (rand() % 3 - 1) * 0.5f;
        dx = 0;
        dy = 0;
        vulnerable = false;
        inScatterMode = true;
        modeTimer = 350;
        frightenedTimer = 0;

        // Выравниваем к сетке
        alignToGrid();
    }

    float getX() const { return x; }
    float getY() const { return y; }

    void resetPosition(float newX, float newY) {
        x = newX;
        y = newY;
        dx = 0;
        dy = 0;
        vulnerable = false;
        inScatterMode = true;
        modeTimer = 350;
        frightenedTimer = 0;
    }

    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }

    // Для отладки - получение текущего направления
    int getDirectionX() const { return dx; }
    int getDirectionY() const { return dy; }
};

#endif