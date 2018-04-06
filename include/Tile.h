#pragma once
#include <vector>

class Tile {
private:
    vec2 center;
    vec2 pilot;
    int shape;

public:
    // Constructor with shape s
    Tile(vec cen, int sh);
    void rotateTile();
}
