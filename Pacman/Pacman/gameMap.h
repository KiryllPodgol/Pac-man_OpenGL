#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "cell.h"
#include "coin.h"
#include <vector>
#include <cstdlib>
#include <ctime>

class GameMap {
private:
    std::vector<std::vector<Cell>> grid;
    int width, height;

public:
    GameMap(int w, int h) : width(w), height(h) {
        grid.resize(height, std::vector<Cell>(width));
        std::srand(std::time(0));
        initializeClassicMap();
    }
   
    void initializeClassicMap() {
        
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                grid[i][j] = Cell(j, i, EMPTY);
            }
        }
        createClassicWalls();
        createCoins();
        createPowerPoints(); 
    }

    void createClassicWalls() {
        // Внешние стены
        for (int i = 0; i < height; i++) {
            grid[i][0].type = WALL;
            grid[i][width - 1].type = WALL;
        }
        for (int j = 0; j < width; j++) {
            grid[0][j].type = WALL;
            grid[height - 1][j].type = WALL;
        }

        // Простая классическая карта Pac-Man
        // Вертикальные стены
        for (int i = 1; i <= 5; i++) {
            grid[i][3].type = WALL;
            grid[i][width - 4].type = WALL;
        }

        for (int i = height - 6; i <= height - 2; i++) {
            grid[i][3].type = WALL;
            grid[i][width - 4].type = WALL;
        }

        // Угловые блоки
        grid[5][5].type = WALL;
        grid[5][6].type = WALL;
        grid[6][5].type = WALL;

        grid[5][width - 6].type = WALL;
        grid[5][width - 7].type = WALL;
        grid[6][width - 6].type = WALL;

        grid[height - 6][5].type = WALL;
        grid[height - 6][6].type = WALL;
        grid[height - 7][5].type = WALL;

        grid[height - 6][width - 6].type = WALL;
        grid[height - 6][width - 7].type = WALL;
        grid[height - 7][width - 6].type = WALL;
    }

    void createCoins() {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (grid[i][j].type == EMPTY) {
                    grid[i][j] = Coin::createCoinCell(j, i);
                }
            }
        }
    }

    void createPowerPoints() {
        // Размещаем POWER_POINT в четырех углах карты (как в оригинальном Pac-Man)
        grid[1][1].type = POWER_POINT;                    // Левый верхний угол
        grid[1][width - 2].type = POWER_POINT;            // Правый верхний угол
        grid[height - 2][1].type = POWER_POINT;           // Левый нижний угол
        grid[height - 2][width - 2].type = POWER_POINT;   // Правый нижний угол
    }

    bool canMove(float fx, float fy) const {
        int x = static_cast<int>(fx);
        int y = static_cast<int>(fy);
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return false;
        }
        return grid[y][x].isWalkable();
    }

    void collectCoin(int x, int y) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            if (grid[y][x].type == COIN || grid[y][x].type == POWER_POINT) {
                grid[y][x].type = EMPTY;
            }
        }
    }

    bool hasCoin(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return false;
        }
        return grid[y][x].type == COIN;
    }

    bool hasPowerPoint(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return false;
        }
        return grid[y][x].type == POWER_POINT;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    const std::vector<std::vector<Cell>>& getGrid() const {
        return grid;
    }

    int countRemainingCoins() const {
        int count = 0;
        for (const auto& row : grid) {
            for (const auto& cell : row) {
                if (cell.type == COIN || cell.type == POWER_POINT) count++;
            }
        }
        return count;
    }
};

#endif