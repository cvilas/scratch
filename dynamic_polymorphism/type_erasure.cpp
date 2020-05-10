// Experimenting with the type-erasure based solution to implementing dynamic
// polymorphism. See C++ London talk: https://youtu.be/t_0JhHVh_Kg

#include <cstdio>
#include <memory>
#include <vector>

class Vector3d;
class Quaternion;

class Circle {
public:
  explicit Circle(double rad) : radius_(rad) {}
  double getRadius() const noexcept { return radius_; }

private:
  double radius_;
};

void translate(Circle const &, Vector3d const &) { printf("translate circle\n"); };

void rotate(Circle const &, Quaternion const &) { printf("rotate circle\n"); }

void draw(Circle const &) { printf("draw circle\n"); }

class Square {
public:
  explicit Square(double s) : side_(s) {}
  double getSide() const noexcept { return side_; }

private:
  double side_;
};

void translate(Square const &, Vector3d const &) { printf("translate square\n"); }

void rotate(Square const &, Quaternion const &) { printf("rotate square\n"); }

void draw(Square const &) { printf("draw square\n"); }

class Shape {
private:
  struct Concept {
    virtual ~Concept() {}
    virtual void do_translate(Vector3d const &v) const = 0;
    virtual void do_rotate(Quaternion const &q) const = 0;
    virtual void do_draw() const = 0;
  };

  template <typename T> 
  struct Model : Concept {
    Model(T const &value) : object_(value) {}

    void do_translate(Vector3d const &v) const override { translate(object_, v); }

    void do_rotate(Quaternion const &q) const override { rotate(object_, q); }

    void do_draw() const override { draw(object_); }

    T object_;
  };

  std::unique_ptr<Concept> pimpl_;

  friend void translate(Shape &shape, Vector3d const &v) {
    shape.pimpl_->do_translate(v);
  }

  friend void rotate(Shape &shape, Quaternion const &v) {
    shape.pimpl_->do_rotate(v);
  }

  friend void draw(Shape const &shape) { shape.pimpl_->do_draw(); }

public:
  template <typename T> 
  Shape(T const &x) : pimpl_(new Model<T>(x)) {}

  Shape(Shape const &s) = delete;
  Shape(Shape &&s) : pimpl_(std::move(s.pimpl_)) {}
  Shape &operator=(Shape const &s) = delete;
  Shape &operator=(Shape &&s) {
    pimpl_ = std::move(s.pimpl_);
    return *this;
  }
};

void draw(std::vector<Shape> const &shapes) {
  for (auto const &shape : shapes) {
    draw(shape);
  }
}

int main() {
  using Shapes = std::vector<Shape>;

  Shapes shapes;
  shapes.push_back(Circle{2.0});
  shapes.push_back(Square{1.5});

  draw(shapes);

  return 0;
}
