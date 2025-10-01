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
        // Очищаем карту
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                grid[i][j] = Cell(j, i, EMPTY);
            }
        }
        createClassicWalls();
        createCoins();
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

        // Простые внутренние препятствия для примера
        if (width > 5 && height > 5) {
            grid[height / 2][3].type = WALL;
            grid[height / 2][4].type = WALL;
            grid[height / 2][5].type = WALL;
            grid[height / 2][width - 4].type = WALL;
            grid[height / 2][width - 5].type = WALL;
            grid[height / 2][width - 6].type = WALL;
        }
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

    bool canMove(float fx, float fy) const {
        int x = static_cast<int>(fx);
        int y = static_cast<int>(fy);
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return false;
        }
        return grid[y][x].isWalkable();
    }

    void collectCoin(int x, int y) {
        if (x >= 0 && x < width && y >= 0 && y < height && grid[y][x].type == COIN) {
            grid[y][x].type = EMPTY;
        }
    }

    bool hasCoin(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return false;
        }
        return grid[y][x].type == COIN;
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
                if (cell.type == COIN) count++;
            }
        }
        return count;
    }
};

#endif