#pragma once
#include "node.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "transform.hpp"
#include "separator.hpp"
#include "cone.hpp"
#include "sphere.hpp"
#include "cylinder.hpp"
#include "cube.hpp"
#include "plane.hpp"
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Utility: Scene root
class Scene : public Node {
public:
    Scene() : Node("root") {}
};
