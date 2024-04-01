// C++23 example demonstrating CRTP for non-virtual polymorphism
// From 'A Journey Into Non-Virtual Polymorphism in C++ - Rudyard Merriam - CppCon 2023'
// https://youtu.be/xpomlTd41hg

#include <print>
#include <vector>
#include <variant>
#include <memory>

struct Shape {
    template<typename T>
    void draw(this T&& self) { self.draw_impl(); }
};

struct Rectangle : public Shape {
private:
    friend Shape;
    void draw_impl() const { std::println("rectangle"); }
};

struct Triangle : public Shape {
private:
    friend Shape;
    void draw_impl() const { std::println("triangle"); }
};

int main() {
    using Shapes = std::variant<Rectangle, Triangle>;
    std::vector<Shapes> shapes;
    shapes.emplace_back(Rectangle{});
    shapes.emplace_back(Triangle{});
    for(const auto& s : shapes) {
        std::visit([](const auto& sh){ sh.draw(); }, s);
    }
    return EXIT_SUCCESS;
}
