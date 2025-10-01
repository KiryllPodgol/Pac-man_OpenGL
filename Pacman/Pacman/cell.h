#ifndef CELL_H
#define CELL_H

enum CellType {
    EMPTY,
    WALL,
    COIN,
    WHITEPOINT  
};

class Cell {
public:
    int x, y;
    CellType type;

    Cell(int x = 0, int y = 0, CellType type = EMPTY) : x(x), y(y), type(type) {}

    bool isWalkable() const {
        return type == EMPTY || type == COIN || type == WHITEPOINT;
    }
};

#endif