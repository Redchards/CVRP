#ifndef COORDINATES_HXX
#define COORDINATES_HXX

#include <iostream>
#include <string>

struct Coordinates
{
    Coordinates(double x_, double y_) noexcept
    : x{x_},
      y{y_}
    {}
    
    Coordinates() noexcept
    : Coordinates(0, 0)
    {}
    
    Coordinates(const Coordinates&) = default;
    Coordinates(Coordinates&&) = default;
    
    Coordinates& operator=(const Coordinates&) = default;
    Coordinates& operator=(Coordinates&&) = default;
    
    double x;
    double y;
};

std::ostream& operator<<(std::ostream& os, Coordinates coord)
{
    os << "(" << std::to_string(coord.x) << ", " << std::to_string(coord.y) << ")";
    return os;
}

bool operator==(Coordinates c1, Coordinates c2)
{
    return c1.x == c2.x && c1.y == c2.y;
}

#endif // COORDINATES_HXX