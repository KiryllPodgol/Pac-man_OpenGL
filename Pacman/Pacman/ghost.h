#ifndef GHOST_H
#define GHOST_H

#include "gameMap.h"
#include "pacman.h"
#include <cmath>
#include <vector>
#include <cstdlib> // для rand()

class Ghost {
private:
    float x, y;
    float speed;
    int dx, dy; // Текущее направление движения
    float visionRadius; // Дальность "зрения" призрака
    bool vulnerable; // Уязвим ли призрак
    float respawnX, respawnY; // Позиция возрождения

    // Движение в погоне за Пакманом
    void chasePacman(const GameMap& map, const Pacman& pacman) {
        float pacmanX = pacman.getX();
        float pacmanY = pacman.getY();

        int chase_dx = 0;
        int chase_dy = 0;

        // Выбираем направление для погони
        if (std::abs(pacmanX - x) > std::abs(pacmanY - y)) {
            chase_dx = (pacmanX > x) ? 1 : -1;
        }
        else {
            chase_dy = (pacmanY > y) ? 1 : -1;
        }

        // Пытаемся двигаться в этом направлении ТОЛЬКО ЕСЛИ ПУТЬ СВОБОДЕН
        if (chase_dx != 0 && map.canMove(x + chase_dx, y)) {
            dx = chase_dx;
            dy = 0;
        }
        else if (chase_dy != 0 && map.canMove(x, y + chase_dy)) {
            dx = 0;
            dy = chase_dy;
        }
        // Если выбранное направление заблокировано, продолжаем текущее или ищем альтернативу
        else if (!map.canMove(x + dx, y + dy)) {
            findAlternativeDirection(map);
        }
    }

    // Бегство от Пакмана (в уязвимом режиме)
    void fleeFromPacman(const GameMap& map, const Pacman& pacman) {
        float pacmanX = pacman.getX();
        float pacmanY = pacman.getY();

        int flee_dx = 0;
        int flee_dy = 0;

        // Выбираем направление противоположное Пакману
        if (std::abs(pacmanX - x) > std::abs(pacmanY - y)) {
            flee_dx = (pacmanX > x) ? -1 : 1;
        }
        else {
            flee_dy = (pacmanY > y) ? -1 : 1;
        }

        // Пытаемся двигаться в направлении бегства
        if (flee_dx != 0 && map.canMove(x + flee_dx, y)) {
            dx = flee_dx;
            dy = 0;
        }
        else if (flee_dy != 0 && map.canMove(x, y + flee_dy)) {
            dx = 0;
            dy = flee_dy;
        }
        // Если направление бегства заблокировано, ищем альтернативу
        else if (!map.canMove(x + dx, y + dy)) {
            findAlternativeDirection(map);
        }
    }

    // Поиск альтернативного направления
    void findAlternativeDirection(const GameMap& map) {
        std::vector<std::pair<int, int>> possibleMoves;
        if (map.canMove(x + 1, y)) possibleMoves.push_back({ 1, 0 });
        if (map.canMove(x - 1, y)) possibleMoves.push_back({ -1, 0 });
        if (map.canMove(x, y + 1)) possibleMoves.push_back({ 0, 1 });
        if (map.canMove(x, y - 1)) possibleMoves.push_back({ 0, -1 });

        if (!possibleMoves.empty()) {
            auto move = possibleMoves[rand() % possibleMoves.size()];
            dx = move.first;
            dy = move.second;
        }
        else {
            dx = 0;
            dy = 0;
        }
    }

    // Случайное патрулирование
    void randomMove(const GameMap& map) {
        // Если уперлись в стену или стоим на месте, ищем новый путь
        if (!map.canMove(x + dx, y + dy) || (dx == 0 && dy == 0)) {
            findAlternativeDirection(map);
        }
    }

public:
    Ghost(float startX = 0, float startY = 0)
        : x(startX), y(startY), speed(0.07f), dx(0), dy(0), visionRadius(8.0f),
        vulnerable(false), respawnX(startX), respawnY(startY) {
    }

    void update(const GameMap& map, const Pacman& pacman) {
        float pacmanX = pacman.getX();
        float pacmanY = pacman.getY();

        bool canSeePacman = false;

        // Проверяем зрение по горизонтали
        if (static_cast<int>(y) == static_cast<int>(pacmanY) && std::abs(x - pacmanX) < visionRadius) {
            canSeePacman = true;
        }
        // Проверяем зрение по вертикали
        if (static_cast<int>(x) == static_cast<int>(pacmanX) && std::abs(y - pacmanY) < visionRadius) {
            canSeePacman = true;
        }

        if (vulnerable) {
            // В уязвимом режиме призрак убегает от Пакмана
            if (canSeePacman) {
                fleeFromPacman(map, pacman);
            }
            else {
                randomMove(map);
            }
        }
        else {
            // Обычный режим
            if (canSeePacman) {
                chasePacman(map, pacman);
            }
            else {
                randomMove(map);
            }
        }

        // Проверяем стену перед каждым движением
        float newX = x + dx * speed;
        float newY = y + dy * speed;

        if (map.canMove(newX, newY)) {
            x = newX;
            y = newY;
        }
        else {
            // Если движение заблокировано, останавливаемся и выравниваем позицию
            x = std::round(x);
            y = std::round(y);
            dx = 0;
            dy = 0;
        }
    }

    // Установка уязвимости
    void setVulnerable(bool isVulnerable) {
        vulnerable = isVulnerable;
    }

    bool isVulnerable() const {
        return vulnerable;
    }

    // Возрождение призрака после съедения
    void respawn(int mapWidth, int mapHeight) {
        x = respawnX;
        y = respawnY;
        dx = 0;
        dy = 0;
        vulnerable = false;
    }

    float getX() const { return x; }
    float getY() const { return y; }

    // Добавим метод для сброса позиции
    void resetPosition(float newX, float newY) {
        x = newX;
        y = newY;
        dx = 0;
        dy = 0;
        vulnerable = false;
    }

    // Установка скорости (для разных уровней сложности)
    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }
};

#endif