#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <stdexcept>

namespace linalg {

struct PointTag {};
struct Frame : PointTag {};

template<typename T>
concept PointType = std::derived_from<T, PointTag>;

template<typename T>
concept FrameType = std::derived_from<T, Frame>;

using Coordinates = std::array<double, 3>;
using Matrix3 = std::array<Coordinates, 3>;

namespace detail {

constexpr Coordinates add(const Coordinates& a, const Coordinates& b) {
    return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

constexpr Coordinates negate(const Coordinates& a) {
    return {-a[0], -a[1], -a[2]};
}

constexpr Coordinates scale(const Coordinates& a, double s) {
    return {a[0] * s, a[1] * s, a[2] * s};
}

constexpr double dot(const Coordinates& a, const Coordinates& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

constexpr Coordinates multiply(const Matrix3& a, const Coordinates& b) {
    return {dot(a[0], b), dot(a[1], b), dot(a[2], b)};
}

constexpr Matrix3 transpose(const Matrix3& a) {
    return {{{a[0][0], a[1][0], a[2][0]},
             {a[0][1], a[1][1], a[2][1]},
             {a[0][2], a[1][2], a[2][2]}}};
}

constexpr Matrix3 multiply(const Matrix3& a, const Matrix3& b) {
    const auto columns = transpose(b);
    Matrix3 result{};
    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t col = 0; col < 3; ++col) {
            result[row][col] = dot(a[row], columns[col]);
        }
    }
    return result;
}

} // namespace detail

template<FrameType ExpressedIn>
class Vector {
public:
    explicit constexpr Vector(Coordinates coordinates = {}) : coordinates_(coordinates) {}

    constexpr const Coordinates& coordinates() const { return coordinates_; }

    constexpr Vector operator+(const Vector& other) const {
        return Vector{detail::add(coordinates_, other.coordinates_)};
    }

    constexpr Vector operator-() const { return Vector{detail::negate(coordinates_)}; }
    constexpr Vector operator-(const Vector& other) const { return *this + -other; }
    constexpr Vector operator*(double s) const { return Vector{detail::scale(coordinates_, s)}; }

    friend constexpr Vector operator*(double s, const Vector& v) { return v * s; }

    constexpr double dot(const Vector& other) const {
        return detail::dot(coordinates_, other.coordinates_);
    }

    constexpr Vector cross(const Vector& other) const {
        const auto& a = coordinates_;
        const auto& b = other.coordinates_;
        return Vector{{a[1] * b[2] - a[2] * b[1],
                       a[2] * b[0] - a[0] * b[2],
                       a[0] * b[1] - a[1] * b[0]}};
    }

private:
    Coordinates coordinates_;
};

// _E r_AB: the displacement from point A to point B, expressed in E.
template<FrameType ExpressedIn, PointType Tail, PointType Head>
class Displacement {
public:
    explicit constexpr Displacement(Coordinates coordinates = {}) : coordinates_(coordinates) {}

    constexpr const Coordinates& coordinates() const { return coordinates_; }

    template<PointType Next>
    constexpr Displacement<ExpressedIn, Tail, Next>
    operator+(const Displacement<ExpressedIn, Head, Next>& other) const {
        return Displacement<ExpressedIn, Tail, Next>{
            detail::add(coordinates_, other.coordinates())};
    }

    constexpr Displacement<ExpressedIn, Head, Tail> operator-() const {
        return Displacement<ExpressedIn, Head, Tail>{detail::negate(coordinates_)};
    }

    template<PointType OtherTail>
    constexpr Displacement<ExpressedIn, Tail, OtherTail>
    operator-(const Displacement<ExpressedIn, OtherTail, Head>& other) const {
        return *this + -other;
    }

private:
    Coordinates coordinates_;
};

// _E r_EP: position of P relative to the origin of E, expressed in E.
template<FrameType ExpressedIn, PointType Point>
class Position {
public:
    explicit constexpr Position(Coordinates coordinates = {}) : coordinates_(coordinates) {}

    constexpr const Coordinates& coordinates() const { return coordinates_; }

    template<PointType Other>
    constexpr Displacement<ExpressedIn, Other, Point>
    operator-(const Position<ExpressedIn, Other>& other) const {
        return Displacement<ExpressedIn, Other, Point>{
            detail::add(coordinates_, detail::negate(other.coordinates()))};
    }

    template<PointType Other>
    constexpr Position<ExpressedIn, Other>
    operator+(const Displacement<ExpressedIn, Point, Other>& displacement) const {
        return Position<ExpressedIn, Other>{
            detail::add(coordinates_, displacement.coordinates())};
    }

    template<PointType Other>
    constexpr Position<ExpressedIn, Other>
    operator-(const Displacement<ExpressedIn, Other, Point>& displacement) const {
        return *this + -displacement;
    }

private:
    Coordinates coordinates_;
};

// C_AB maps B-expressed coordinates into A-expressed coordinates.
template<FrameType To, FrameType From>
class Rotation {
public:
    constexpr Rotation() = default;

    static Rotation from_matrix(const Matrix3& matrix) {
        constexpr double tolerance = 1e-10;
        for (const auto& row : matrix) {
            for (double value : row) {
                if (!std::isfinite(value)) {
                    throw std::invalid_argument("Rotation matrix must contain finite values");
                }
            }
        }
        const auto gram = detail::multiply(matrix, detail::transpose(matrix));
        for (std::size_t row = 0; row < 3; ++row) {
            for (std::size_t col = 0; col < 3; ++col) {
                const double expected = row == col ? 1.0 : 0.0;
                if (!std::isfinite(gram[row][col]) ||
                    std::abs(gram[row][col] - expected) > tolerance) {
                    throw std::invalid_argument("Rotation matrix must be orthonormal");
                }
            }
        }
        const double determinant =
            matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) -
            matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]) +
            matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);
        if (std::abs(determinant - 1.0) > tolerance) {
            throw std::invalid_argument("Rotation matrix must have determinant +1");
        }
        return Rotation{matrix};
    }

    constexpr const Matrix3& matrix() const { return matrix_; }

    template<FrameType Source>
    constexpr Rotation<To, Source> operator*(const Rotation<From, Source>& other) const {
        return Rotation<To, Source>{detail::multiply(matrix_, other.matrix())};
    }

    constexpr Vector<To> operator*(const Vector<From>& vector) const {
        return Vector<To>{detail::multiply(matrix_, vector.coordinates())};
    }

    template<PointType Tail, PointType Head>
    constexpr Displacement<To, Tail, Head>
    operator*(const Displacement<From, Tail, Head>& displacement) const {
        return Displacement<To, Tail, Head>{
            detail::multiply(matrix_, displacement.coordinates())};
    }

    constexpr Rotation<From, To> inverse() const {
        return Rotation<From, To>{detail::transpose(matrix_)};
    }

private:
    template<FrameType, FrameType>
    friend class Rotation;

    explicit constexpr Rotation(Matrix3 matrix) : matrix_(matrix) {}
    Matrix3 matrix_{{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};
};

// T_AB = [C_AB, _A r_AB; 0, 1].
template<FrameType To, FrameType From>
class Transform {
public:
    constexpr Transform() = default;

    constexpr Transform(Rotation<To, From> rotation, Displacement<To, To, From> translation)
        : rotation_(rotation), translation_(translation) {}

    constexpr const Rotation<To, From>& rotation() const { return rotation_; }
    constexpr const Displacement<To, To, From>& translation() const { return translation_; }

    template<FrameType Source>
    constexpr Transform<To, Source> operator*(const Transform<From, Source>& other) const {
        return Transform<To, Source>{
            rotation_ * other.rotation(),
            translation_ + rotation_ * other.translation()};
    }

    template<PointType Point>
    constexpr Position<To, Point> operator*(const Position<From, Point>& point) const {
        return Position<To, Point>{
            detail::add(detail::multiply(rotation_.matrix(), point.coordinates()),
                        translation_.coordinates())};
    }

    constexpr Vector<To> operator*(const Vector<From>& vector) const {
        return rotation_ * vector;
    }

    template<PointType Tail, PointType Head>
    constexpr Displacement<To, Tail, Head>
    operator*(const Displacement<From, Tail, Head>& displacement) const {
        return rotation_ * displacement;
    }

    constexpr Transform<From, To> inverse() const {
        const auto inverse_rotation = rotation_.inverse();
        return Transform<From, To>{inverse_rotation, inverse_rotation * -translation_};
    }

private:
    Rotation<To, From> rotation_;
    Displacement<To, To, From> translation_;
};

} // namespace linalg
