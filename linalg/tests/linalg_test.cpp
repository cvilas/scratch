#include "linalg.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>

using namespace linalg;

struct World : Frame {};
struct Base : Frame {};
struct Sensor : Frame {};
struct A : PointTag {};
struct B : PointTag {};
struct C : PointTag {};

template<typename L, typename R>
concept Addable = requires(L l, R r) { l + r; };

template<typename L, typename R>
concept Subtractable = requires(L l, R r) { l - r; };

template<typename L, typename R>
concept Multipliable = requires(L l, R r) { l * r; };

static_assert(FrameType<World> && PointType<World> && PointType<A>);
static_assert(!FrameType<A> && !FrameType<int>);
static_assert(!std::is_convertible_v<Vector<World>, Vector<Base>>);
static_assert(!std::is_constructible_v<Vector<World>, Vector<Base>>);
static_assert(!std::is_constructible_v<Displacement<World, A, B>, Vector<World>>);
static_assert(!Addable<Vector<World>, Vector<Base>>);
static_assert(!Subtractable<Position<World, A>, Position<Base, B>>);
static_assert(!Addable<Position<World, A>, Position<World, B>>);
static_assert(!Addable<Displacement<World, A, B>, Displacement<World, A, C>>);
static_assert(!Addable<Displacement<World, A, B>, Displacement<Base, B, C>>);
static_assert(!Addable<Position<World, A>, Displacement<World, B, C>>);
static_assert(!Multipliable<Rotation<World, Base>, Rotation<Sensor, World>>);
static_assert(!Multipliable<Rotation<World, Base>, Vector<World>>);
static_assert(!Multipliable<Rotation<World, Base>, Position<Base, A>>);
static_assert(!Multipliable<Transform<World, Base>, Transform<Sensor, World>>);
static_assert(!Multipliable<Transform<World, Base>, Position<Sensor, A>>);
static_assert(!std::is_constructible_v<Transform<World, Base>,
    Rotation<World, Base>, Displacement<Base, World, Base>>);
static_assert(!std::is_constructible_v<Transform<World, Base>,
    Rotation<World, Base>, Displacement<World, Base, World>>);
static_assert(!std::is_constructible_v<Rotation<World, Base>, Matrix3>);
static_assert(std::is_same_v<decltype(Rotation<World, Base>{} * Rotation<Base, Sensor>{}),
                             Rotation<World, Sensor>>);
static_assert(std::is_same_v<decltype(Transform<World, Base>{}.inverse()),
                             Transform<Base, World>>);
static_assert(std::is_same_v<decltype(Rotation<World, Base>{} * Displacement<Base, A, B>{}),
                             Displacement<World, A, B>>);
static_assert(std::is_same_v<decltype(Transform<World, Base>{} * Displacement<Base, A, B>{}),
                             Displacement<World, A, B>>);
static_assert(std::is_same_v<decltype(Position<World, B>{} - Position<World, A>{}),
                             Displacement<World, A, B>>);
static_assert(std::is_same_v<decltype(-Displacement<World, A, B>{}),
                             Displacement<World, B, A>>);

constexpr auto ab = Displacement<World, A, B>{{1., 2., 3.}};
constexpr auto bc = Displacement<World, B, C>{{4., 5., 6.}};
static_assert((ab + bc).coordinates() == Coordinates{5., 7., 9.});
static_assert(((ab + bc) - bc).coordinates() == ab.coordinates());
static_assert((-ab).coordinates() == Coordinates{-1., -2., -3.});
constexpr auto position_a = Position<World, A>{{2., 3., 4.}};
static_assert(((position_a + ab) - ab).coordinates() == position_a.coordinates());
constexpr auto v = Vector<World>{{1., 2., 3.}};
static_assert((2. * v - v).coordinates() == v.coordinates());
static_assert(v.dot(v) == 14.);
static_assert(Vector<World>{{1., 0., 0.}}.cross(Vector<World>{{0., 1., 0.}}).coordinates()
              == Coordinates{0., 0., 1.});
constexpr auto identity = Transform<World, World>{};
static_assert((identity * position_a).coordinates() == position_a.coordinates());
static_assert((identity.inverse() * v).coordinates() == v.coordinates());
static_assert(sizeof(Vector<World>) == sizeof(Coordinates));
static_assert(sizeof(Displacement<World, A, B>) == sizeof(Coordinates));
static_assert(sizeof(Position<World, A>) == sizeof(Coordinates));
static_assert(sizeof(Rotation<World, Base>) == sizeof(Matrix3));

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void near(const Coordinates& actual, const Coordinates& expected) {
    for (std::size_t i = 0; i < 3; ++i) {
        check(std::isfinite(actual[i]) && std::abs(actual[i] - expected[i]) < 1e-12,
              "Unexpected coordinates");
    }
}

template<typename Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const std::invalid_argument&) {
        return;
    }
    check(false, "Expected invalid rotation to be rejected");
}

int main() {
    const auto yaw = Rotation<World, Base>::from_matrix(
        {{{0., -1., 0.}, {1., 0., 0.}, {0., 0., 1.}}});
    const auto roll = Rotation<Base, Sensor>::from_matrix(
        {{{1., 0., 0.}, {0., 0., -1.}, {0., 1., 0.}}});
    const auto composed_rotation = yaw * roll;
    near((yaw * Vector<Base>{{1., 2., 3.}}).coordinates(), {-2., 1., 3.});
    near((composed_rotation * Vector<Sensor>{{1., 2., 3.}}).coordinates(), {3., 1., 2.});
    near((composed_rotation.inverse() *
          (composed_rotation * Vector<Sensor>{{1., 2., 3.}})).coordinates(), {1., 2., 3.});

    const auto world_from_base = Transform<World, Base>{
        yaw, Displacement<World, World, Base>{{10., 20., 30.}}};
    const auto base_from_sensor = Transform<Base, Sensor>{
        roll, Displacement<Base, Base, Sensor>{{1., 2., 3.}}};
    const auto world_from_sensor = world_from_base * base_from_sensor;
    const auto point = Position<Sensor, A>{{4., 5., 6.}};
    near(world_from_sensor.translation().coordinates(), {8., 21., 33.});
    near((world_from_sensor * point).coordinates(), {14., 25., 38.});
    near((world_from_base * (base_from_sensor * point)).coordinates(), {14., 25., 38.});
    near((world_from_sensor.inverse() * (world_from_sensor * point)).coordinates(),
         point.coordinates());
    near(((world_from_sensor * world_from_sensor.inverse()) *
          Position<World, A>{{4., 5., 6.}}).coordinates(), point.coordinates());
    near(((world_from_sensor.inverse() * world_from_sensor) * point).coordinates(),
         point.coordinates());

    const auto displacement = Displacement<Sensor, A, B>{{4., 5., 6.}};
    near((world_from_sensor * displacement).coordinates(), {6., 4., 5.});
    near((world_from_sensor * Vector<Sensor>{{4., 5., 6.}}).coordinates(), {6., 4., 5.});
    const auto point_b = Position<Sensor, B>{{8., 10., 12.}};
    near(((world_from_sensor * point_b) - (world_from_sensor * point)).coordinates(),
         (world_from_sensor * (point_b - point)).coordinates());
    near((world_from_sensor.inverse() *
          (world_from_sensor * displacement)).coordinates(), displacement.coordinates());

    const double angle = 0.37;
    const auto arbitrary = Rotation<World, Base>::from_matrix(
        {{{std::cos(angle), -std::sin(angle), 0.},
          {std::sin(angle), std::cos(angle), 0.}, {0., 0., 1.}}});
    near((arbitrary.inverse() * (arbitrary * Vector<Base>{{1., 2., 3.}})).coordinates(),
         {1., 2., 3.});

    const Matrix3 identity_matrix = Rotation<World, Base>{}.matrix();
    auto invalid = identity_matrix;
    invalid[0][0] = -1.;
    rejects([&] { Rotation<World, Base>::from_matrix(invalid); });
    invalid[0][0] = 2.;
    rejects([&] { Rotation<World, Base>::from_matrix(invalid); });
    invalid = {};
    rejects([&] { Rotation<World, Base>::from_matrix(invalid); });
    invalid = identity_matrix;
    invalid[0][1] = 0.1;
    rejects([&] { Rotation<World, Base>::from_matrix(invalid); });
    for (double bad : {std::numeric_limits<double>::quiet_NaN(),
                       std::numeric_limits<double>::infinity(),
                       -std::numeric_limits<double>::infinity(),
                       std::numeric_limits<double>::max()}) {
        invalid = identity_matrix;
        invalid[0][0] = bad;
        rejects([&] { Rotation<World, Base>::from_matrix(invalid); });
    }
}
