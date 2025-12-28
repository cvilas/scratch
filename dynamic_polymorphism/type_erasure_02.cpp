// Nice explanation of Type Erasure design by Klaus Iglberger
// From https://youtu.be/m3UmABVf55g
//
// See also https://www.modernescpp.com/index.php/type-erasure/
//
// Type erasure is a type-safe generic way to provide one interface for different types, without 
// the need for a common base class.

#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <fstream>
#include <filesystem>

// Shape abstraction using Type Erasure 
// - can do the same as the Shape classes implemented in a virtual polymorphic hierarchy
// - is faster due to fewer indirections
// - is a value (can be copied, moved)
//   - can create std::vector<Shape> by value, not pointers
//   - no need for vector<variant<..>> of concrete shapes 
// - simplifies surrounding code. Concrete shapes (See 'Circle', 'Square')
//   - have no virtual functions
//   - no dependencies 
// - clear architecture boundary (see architecture boundary) 
//   - your code can be extended, and implements details 
//   - my code (that uses your code to achieve something) is fixed
// - less memory management, inheritance, pointers, virtual functions (only one indirection)
// - better performance

class Shape {
public:
    template<typename ConcreteShape, typename DrawStrategy>
    Shape(ConcreteShape shape, DrawStrategy ds) 
    : pimpl_{ std::make_unique<ShapeModel<ConcreteShape, DrawStrategy>>(shape, ds)}{}

    void draw() const {pimpl_->draw(); }

private:

    // - All the virtual functions factored out into this class
    // - Private base class for internal purposes
    class ShapeConcept {
        public:
        virtual ~ShapeConcept() = default;
        virtual void draw() const = 0;
    };

    // Encapsulates model of a shape and how to draw it
    template<typename ConcreteShape, typename DrawStrategy>
    class ShapeModel : public ShapeConcept {
    public:
        explicit ShapeModel(ConcreteShape shape, DrawStrategy ds) : shape_{shape}, drawer_{ds} {}
        void draw() const override { drawer_(shape_); }
    private:
        ConcreteShape shape_;
        DrawStrategy drawer_;
    };

    std::unique_ptr<ShapeConcept> pimpl_;
};

using Shapes = std::vector<Shape>;

using ShapesFactory = std::function<Shapes(std::string_view)>;

// draw using value semantics, no pointers
void drawAllShapes(Shapes const& shapes) {
    for(auto const& sh : shapes) {
        sh.draw();
    }
}

// no pointers, no base classes
void createAndDrawShapes( ShapesFactory const& factory, std::string_view filename) {
    auto shapes = factory(filename);
    drawAllShapes(shapes);
}

// ---- My code: Fixed abstraction that exploits your code for use-case-specific details
//------------------------------------------------------------------------------------------------ Architectural boundary
// ---- Your code: implements details and extensions 

// A concrete shape. simple. no class hierarchy, no virtual functions, no dependencies
class Circle {
public:
    explicit Circle(double r) : radius_(r) {}
    auto radius() const -> double { return radius_; }
private:
    double radius_{};
};

// Another concrete shape
class Square {
public:
    explicit Square(double s) : side_{s} {};
    auto side() const -> double { return side_; }
private:
    double side_{};
};


// Drawing strategy for my types. Can be extended to new types. value semantics. no virtual call operators
class OpenGLDrawer {
public:
    explicit OpenGLDrawer( /* color, texture, transparency, etc */) {}
    void operator()(Circle const& circle) const{}
    void operator()(Square const& square) const{}
private:
    // data members for color, texture, transparency, etc
};

// Detail: Factory class for my types. Can be extended to new types. 
// no dependencies, value semantics.
class MyShapesFactory {
public:
    Shapes operator()(std::filesystem::path filename) const {
        Shapes shapes{};
        std::string shape{};
        std::ifstream iss{ filename };
        while(iss >> shape) {
            if(shape == "circle"){
                double r{};
                iss >> r;
                shapes.emplace_back( Circle(r), OpenGLDrawer{});
            }
            else if(shape == "square") {
                double side{};
                iss >> side;
                shapes.emplace_back(Square(side), OpenGLDrawer{});
            }
            else {
                break;
            }
        }
        return shapes;
    }
};

int main() {
    MyShapesFactory factory{};
    createAndDrawShapes(factory, "shapes.txt");
    return 0;
}

// comparing std::variant with type-erasure approach
//
// std::variant                | Type Erasure
//-----------------------------|----------------------------
// Functional programming      | Object-oriented programming
// Fixed set of types          | Open set of types
// Open set of operations      | Closed set of operations
// Best for low level of       | Best for higher levels of 
// architecture (impl details) | architecture

