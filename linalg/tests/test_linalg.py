"""Compile/run the positive tests and independently compile each invalid expression."""

import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
PRELUDE = """
#include "linalg.hpp"
using namespace linalg;
struct World : Frame {};
struct Base : Frame {};
struct Camera : Frame {};
struct A : PointTag {};
struct B : PointTag {};
struct C : PointTag {};
"""

INVALID = {
    "mixed_basis_sum": "Vector<World>{} + Vector<Base>{}",
    "implicit_relabel": "Vector<World> v = Vector<Base>{}",
    "explicit_relabel": "Vector<World>{Vector<Base>{}}",
    "rotation_chain": "Rotation<World, Base>{} * Rotation<Camera, World>{}",
    "rotation_vector": "Rotation<World, Base>{} * Vector<Camera>{}",
    "rotation_displacement": "Rotation<World, Base>{} * Displacement<Camera, A, B>{}",
    "rotation_position": "Rotation<World, Base>{} * Position<Base, A>{}",
    "transform_chain": "Transform<World, Base>{} * Transform<Camera, World>{}",
    "transform_position": "Transform<World, Base>{} * Position<Camera, A>{}",
    "transform_vector": "Transform<World, Base>{} * Vector<Camera>{}",
    "transform_displacement": "Transform<World, Base>{} * Displacement<Camera, A, B>{}",
    "translation_basis": (
        "Transform<World, Base>{Rotation<World, Base>{}, Displacement<Base, World, Base>{}}"
    ),
    "translation_endpoints": (
        "Transform<World, Base>{Rotation<World, Base>{}, Displacement<World, Base, World>{}}"
    ),
    "translation_unrelated_origin": (
        "Transform<World, Base>{Rotation<World, Base>{}, Displacement<World, A, Base>{}}"
    ),
    "disconnected_endpoints": "Displacement<World, A, B>{} + Displacement<World, A, C>{}",
    "displacement_basis": "Displacement<World, A, B>{} + Displacement<Base, B, C>{}",
    "position_sum": "Position<World, A>{} + Position<World, B>{}",
    "position_difference_basis": "Position<World, A>{} - Position<Base, B>{}",
    "position_displacement_tail": "Position<World, A>{} + Displacement<World, B, C>{}",
    "position_displacement_basis": "Position<World, A>{} + Displacement<Base, A, B>{}",
    "mixed_basis_dot": "Vector<World>{}.dot(Vector<Base>{})",
    "mixed_basis_cross": "Vector<World>{}.cross(Vector<Base>{})",
    "point_used_as_frame": "Vector<A>{}",
    "untyped_frame": "Rotation<int, Base>{}",
    "displacement_to_vector": "Vector<World>{Displacement<World, A, B>{}}",
    "mutable_coordinates": "Vector<World>{}.coordinates()[0] = 1.",
    "unchecked_matrix": "Rotation<World, Base>{Matrix3{}}",
}


class FrameSafeLinalgTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="frame-safe-linalg-")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.build = Path(cls.temp.name)
        cls.compiler = shlex.split(os.environ.get("CXX", "g++"))
        cls.flags = ["-std=c++20", "-Wall", "-Wextra", "-Werror", "-pedantic", "-I", str(ROOT)]

    def compile(self, source, output=None):
        if output is None:
            # A warning must never count as a type-system rejection.
            flags = ["-std=c++20", "-pedantic-errors", "-I", str(ROOT), "-fsyntax-only"]
            command = self.compiler + flags + [str(source)]
        else:
            command = self.compiler + self.flags + [str(source), "-o", str(output)]
        return subprocess.run(
            command,
            text=True,
            capture_output=True,
            check=False,
        )

    def test_numeric_and_static_contracts(self):
        binary = self.build / "linalg_test"
        result = self.compile(ROOT / "tests" / "linalg_test.cpp", binary)
        self.assertEqual(result.returncode, 0, result.stderr)
        subprocess.run([str(binary)], check=True)

    def test_example(self):
        binary = self.build / "linalg"
        result = self.compile(ROOT / "linalg.cpp", binary)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = subprocess.run([str(binary)], text=True, capture_output=True, check=True)
        self.assertEqual(
            result.stdout,
            "Tool in world: (1, 4, 0)\n"
            "Tool-to-target displacement in end effector: (1, -1, 0)\n",
        )

    def test_invalid_expressions(self):
        # A passing control prevents a missing compiler/header from masquerading as rejection.
        source = self.build / "control.cpp"
        source.write_text(PRELUDE + "\nint main() {}\n")
        result = self.compile(source)
        self.assertEqual(result.returncode, 0, result.stderr)
        for name, expression in INVALID.items():
            with self.subTest(name=name):
                source = self.build / f"{name}.cpp"
                source.write_text(PRELUDE + f"\nint main() {{ {expression}; }}\n")
                result = self.compile(source)
                self.assertNotEqual(result.returncode, 0, f"Unexpectedly compiled: {expression}")
                self.assertIn("error:", result.stderr)


if __name__ == "__main__":
    unittest.main()
