#pragma once

#include <SFML/Graphics.hpp>

#include <array>
#include <experimental/optional>
#include <utility>

namespace detail {

// TODO(jsvana): make this not do crappy new/delete

/*
 * Business logic of the quadtree
 */
template <class T>
struct QuadtreeNode {
 public:
  const std::size_t MAX_ENTRIES = 4;

  sf::FloatRect dimensions;

  bool leaf = true;

  /*
   * Structure is:
   *
   *   I | II
   * ----+----
   * III | IV
   *
   */
  std::array<QuadtreeNode<T>*, 4> children;

  std::list<T> entries;

  QuadtreeNode(sf::FloatRect dim) : dimensions(dim) {}

  ~QuadtreeNode() {
    for (int i = 0; i < 4; i++) {
      delete children[i];
    }
  }

  /**
   * Splits node into four quadrants and moves entries into proper quadrants
   */
  void split() {
    auto newDim = dimensions;
    newDim.width = dimensions.width / 2;
    newDim.height = dimensions.height / 2;
    const auto x = dimensions.left;
    const auto y = dimensions.top;

    children[0] = new QuadtreeNode<T>(newDim);
    newDim.left = x + newDim.width;
    children[1] = new QuadtreeNode<T>(newDim);
    newDim.left = x;
    newDim.top = y + newDim.height;
    children[2] = new QuadtreeNode<T>(newDim);
    newDim.left = x + newDim.width;
    children[3] = new QuadtreeNode<T>(newDim);

    leaf = false;

    while (!entries.empty()) {
      auto entry = entries.back();
      children[quadrant(entry)]->insert(entry);
      entries.pop_back();
    }
  }

  /**
   * Finds the quadtree quadrant for the value
   *
   * Here is where we assume T has `getDimensions()`
   *
   * @param value Value to find quadrant for
   * @return Quadrant for value
   */
  int quadrant(const T& value) {
    const auto midX = dimensions.left + dimensions.width / 2;
    const auto midY = dimensions.top + dimensions.height / 2;

    // TODO(jsvana): pass dimension func to avoid this
    const auto valueDim = value->getDimensions();

    return (valueDim.left >= midX) + (valueDim.top >= midY) * 2;
  }

  /**
   * Inserts the given value into the tree, splitting and creating children as
   * necessary
   *
   * @param value Value to insert
   */
  void insert(T value) {
    if (entries.size() == MAX_ENTRIES) {
      split();
    }

    if (leaf) {
      entries.push_back(value);
    } else {
      children[quadrant(value)]->insert(value);
    }
  }

  /**
   * Fetches all entries near the given value
   *
   * @param value Value to find entries around
   * @return List of nearby entities
   */
  std::list<T> get(const T value) {
    if (leaf) {
      return entries;
    }

    return children[quadrant(value)]->get(value);
  }
};

}  // namespace detail

/*
 * Used for collision detection.
 * Assumes T has method `getDimensions()` that returns `sf::FloatRect`
 */
template <class T>
class Quadtree {
 private:
  sf::FloatRect dimensions_;

  detail::QuadtreeNode<T>* root_ = nullptr;

 public:
  Quadtree(sf::FloatRect dim) : dimensions_(dim) {}

  /**
   * Clears nodes in the quadtree
   */
  void clear() {
    if (root_) {
      delete root_;
    }
    root_ = new detail::QuadtreeNode<T>(dimensions_);
  }

  /**
   * Inserts the given value into the tree, splitting and creating children as
   * necessary
   *
   * @param value Value to insert
   */
  void insert(T value) {
    if (!root_) {
      return;
    }

    root_->insert(value);
  }

  /**
   * Fetches all entries near the given value
   *
   * @param value Value to find entries around
   * @return List of nearby entities
   */
  std::list<T> get(const T value) {
    if (!root_) {
      return std::list<T>();
    }
    return root_->get(value);
  }
};
