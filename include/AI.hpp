#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Component.h"

struct AI : public Component {
};

using AIPtr = std::shared_ptr<AI>;
