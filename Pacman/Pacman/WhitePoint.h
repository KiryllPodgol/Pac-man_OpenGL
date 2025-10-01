#ifndef WHITEPOINT_H
#define WHITEPOINT_H

#include "cell.h"

namespace WhitePoint {
    inline Cell createWhitePointCell(int x, int y) {
        return Cell(x, y, WHITEPOINT);
    }
}

#endif