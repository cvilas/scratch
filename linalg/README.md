# Frame-safe coordinate transformations

This dependency-free C++20 proof of concept extends `linalg.cpp` with real 3D
arithmetic and compile-time coordinate semantics. Tags occupy no storage in the
numeric objects. Include `linalg.hpp` to use it.

The convention follows Paul Furgale's
[Representing Robot Pose: The Good, the Bad, and the Ugly](https://rpg.ifi.uzh.ch/docs/teaching/2025/FurgaleTutorial.pdf).
The basis and endpoint interpretation is also documented in
[wave_geometry's frame semantics](https://github.com/wavelab/wave_geometry/blob/master/docs/frame_semantics.md),
an existing, more extensive implementation of this idea.

## Mathematical notation as types

| Math | C++ | Meaning |
|---|---|---|
| ${}_E v$ | `Vector<E>` | Free vector expressed in E |
| ${}_E r_{AB}$ | `Displacement<E, A, B>` | Directed displacement **from A to B**, expressed in E |
| ${}_E r_{EP}$ | `Position<E, P>` | Position of point P relative to E's origin, expressed in E |
| $C_{AB}$ | `Rotation<A, B>` | Re-express B coordinates in A |
| $T_{AB}$ | `Transform<A, B>` | Map B-relative positions to A-relative positions |

Derive coordinate-system tags from `Frame` and landmark tags from `PointTag`.
A frame is also a point tag denoting its origin. Define distinct tags for
distinct physical points (e.g. left camera and right camera); inheriting one
user-defined frame from another does not make their coordinates compatible.

Rotations and transforms use **destination, source** order, as in the original
prototype. Displacements use **basis, tail, head** order. This distinction is
intentional: rotating a displacement changes its basis, not its endpoints.
These are passive coordinate changes, not active motion of a physical point.

## Algebra

```cpp
using namespace linalg;
struct World : Frame {};
struct Base : Frame {};
struct Camera : Frame {};
struct Landmark : PointTag {};

auto C_WB = Rotation<World, Base>::from_matrix(
    {{{0., -1., 0.}, {1., 0., 0.}, {0., 0., 1.}}});
auto T_WB = Transform<World, Base>{
    C_WB, Displacement<World, World, Base>{{1., 2., 0.}}};
auto T_BC = Transform<Base, Camera>{
    Rotation<Base, Camera>{}, Displacement<Base, Base, Camera>{{1., 0., 0.}}};
auto T_WC = T_WB * T_BC;                       // Transform<World, Camera>
auto p_C = Position<Camera, Landmark>{{1., 0., 0.}};
auto p_W = T_WC * p_C;                        // Position<World, Landmark>, (1,4,0)
auto recovered = T_WC.inverse() * p_W;         // Position<Camera, Landmark>
auto direction_W = T_WC * Vector<Camera>{{1., 0., 0.}}; // (0,1,0), no translation
```

The rigid transform stores $C_{AB}$ and ${}_A r_{AB}$, so
${}_A r_{AP} = C_{AB} {}_B r_{BP} + {}_A r_{AB}$.
Composition is $T_{AC}=T_{AB}T_{BC}$, with translation
${}_A r_{AC}={}_A r_{AB}+C_{AB}{}_B r_{BC}$.
The inverse has rotation $C_{AB}^{T}$ and translation
$-C_{AB}^{T}{}_A r_{AB}$.

Free vectors support same-basis addition/subtraction, scalar multiplication,
dot products and cross products. Displacements support ordered head-to-tail
addition: `Displacement<E,A,B> + Displacement<E,B,C>` yields
`Displacement<E,A,C>`. Negation reverses endpoints; subtraction of displacements
with a common head uses this same chaining rule. This small API deliberately
requires path order rather than implementing every commuted form of addition.

Subtracting `Position<E,A>` from `Position<E,B>` yields
`Displacement<E,A,B>`. Adding that displacement to `Position<E,A>` yields
`Position<E,B>`. Positions cannot be added or implicitly treated as vectors.
Both rotations and transforms re-express displacements without translation.
Only transforms act on positions: a rotation alone cannot change their origin.

For example, these **do not compile**:

```cpp
Vector<World>{} + Vector<Base>{};                  // Different bases
Rotation<World, Base>{} * Rotation<Camera, World>{}; // Broken frame chain
T_WC * Position<Base, Landmark>{};                 // Wrong input frame
Transform<World, Base>{
    C_WB, Displacement<World, Base, World>{}};      // Reversed translation
p_W + p_W;                                       // Position + position
```

## Construction boundary and limits

Raw coordinates enter through explicit constructors; `coordinates()` and
`matrix()` expose read-only numeric storage. There is no implicit relabeling,
untyped arithmetic fallback, or inheritance from `std::array`.
`Rotation{}` represents aligned axes, including between distinct frame tags;
`Transform{}` additionally asserts coincident origins. These defaults are
explicit geometric assumptions, not facts inferred about the robot.

`Rotation::from_matrix()` checks finite elements, orthonormality and determinant
+1 (absolute tolerance `1e-10`), throwing `std::invalid_argument` otherwise.
Matrices are row-major and act on column vectors; transpose implements inverse.
Composition/inversion preserve the rotation structure without revalidating it;
long chains still accumulate floating-point drift.

The compiler proves **compatibility of declared semantics**, not the physical
truth of labels, calibration, units, timestamps, or numeric accuracy. It cannot
know that raw coordinates supplied as `Vector<World>` actually came from Base.
Explicitly extracting coordinates and reconstructing a differently tagged object
is a deliberate escape hatch. Position/displacement constructors likewise trust
the asserted endpoint labels and values. All coordinates use `double` and a
consistent, caller-chosen length unit; they are not checked for finiteness.
This is not yet a general linear algebra, unit, uncertainty, or Lie-group library.

## Run

From the repository root:

```sh
cd linalg
g++ -std=c++20 -Wall -Wextra -pedantic linalg.cpp -o /tmp/linalg
/tmp/linalg
python3 tests/test_linalg.py
```

The test runner uses only Python's standard library and a C++20 compiler (`CXX`
overrides `g++`). It runs numerical/static-assert tests and the example, then
compiles each incompatible expression independently and requires rejection.
A passing compilation control ensures missing headers/tooling cannot count as
successful negative tests. Build artifacts live in an automatically cleaned
temporary directory; tests remain effective even when `NDEBUG` is defined.
