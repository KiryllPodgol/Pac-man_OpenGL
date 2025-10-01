#ifndef COIN_H
#define COIN_H

#include "cell.h"

namespace Coin {
    inline Cell createCoinCell(int x, int y) {
        return Cell(x, y, COIN);
    }
}

#endif