#pragma once

#include <SDL.h>
#include <spdlog/spdlog.h>

#include <ostream>

/**
 * Gets a reference to the stdout logger
 *
 * @return Reference to stdout logger
 */
std::shared_ptr<spdlog::logger> out();

/**
 * Gets a reference to the stderr logger
 *
 * @return Reference to stderr logger
 */
std::shared_ptr<spdlog::logger> err();

/**
 * Generic function to clamp a number inside the given min and max
 *
 * @param num Number to clamp
 * @param min Minimum value for number
 * @param max Maximum value for numver
 * @return Whether number was clamped
 */
template <typename T>
bool clamp(T& num, T min, T max) {
  T clamped = std::max(min, std::min(num, max));
  if (clamped != num) {
    num = clamped;
    return true;
  }
  return false;
}

/**
 * A class representing a point in space
 */
class Point {
 public:
  int x, y;

  Point() : x(0), y(0) {}
  Point(int newX, int newY) : x(newX), y(newY) {}

  /**
   * Move the point by the given (dx, dy)
   *
   * @param dx Distance to move x coordinate
   * @param dy Distance to move y coordinate
   */
  void move(int dx, int dy) {
    x += dx;
    y += dy;
  }

  /**
   * Move the point by the given point's (dx, dy)
   *
   * @param p Distance to move
   */
  void move(Point p) {
    x += p.x;
    y += p.y;
  }

  bool operator==(Point p) { return x == p.x && y == p.y; }
};

/**
 * A special case of Point that ensures its coordinates are always
 * within the specified boundaries
 */
class BoundedPoint : public Point {
 public:
  int xMin, xMax, yMin, yMax;

  BoundedPoint() : Point(0, 0), xMin(0), xMax(0), yMin(0), yMax(0) {}
  BoundedPoint(int newXMin, int newXMax, int newYMin, int newYMax)
      : Point(0, 0),
        xMin(newXMin),
        xMax(newXMax),
        yMin(newYMin),
        yMax(newYMax) {}
  BoundedPoint(int newX, int newY, int newXMin, int newXMax, int newYMin,
               int newYMax)
      : Point(newX, newY),
        xMin(newXMin),
        xMax(newXMax),
        yMin(newYMin),
        yMax(newYMax) {}

  /**
   * Move the point by the given (dx, dy) but clamp movement
   * to the point's boundaries
   *
   * @param dx Distance to move x coordinate
   * @param dy Distance to move y coordinate
   */
  void move(int dx, int dy) {
    Point::move(dx, dy);
    clamp<int>(x, xMin, xMax);
    clamp<int>(y, yMin, yMax);
  }

  /**
   * Update point's position and clamp to boundaries
   *
   * @param newX New x coordinate of point
   * @param newY New y coordinate of point
   */
  void setPosition(int newX, int newY) {
    x = newX;
    y = newY;
    clamp<int>(x, xMin, xMax);
    clamp<int>(y, yMin, yMax);
  }
};

/**
 * Represents a rectangle with a top left position and
 * a width and height
 */
struct Rect {
 public:
  int x, y;
  int w, h;

  Rect() : x(0), y(0), w(0), h(0) {}
  Rect(int newX, int newY, int newW, int newH)
      : x(newX), y(newY), w(newW), h(newH) {}

  Rect operator=(Rect rect) {
    x = rect.x;
    y = rect.y;
    w = rect.w;
    h = rect.h;
    return *this;
  }

  /**
   * Move the rect by the given (dx, dy)
   *
   * @param dx Distance to move x coordinate
   * @param dy Distance to move y coordinate
   */
  void move(int dx, int dy) {
    x += dx;
    y += dy;
  }

  bool operator==(Rect r) {
    return x == r.x && y == r.y && w == r.w && h == r.h;
  }

  bool operator!=(Rect r) { return !(*this == r); }
};
