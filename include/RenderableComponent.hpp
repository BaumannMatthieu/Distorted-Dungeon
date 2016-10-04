#pragma once

#include <memory>

#include "../include/Component.h"
#include "../include/renderable.hpp"

struct RenderableComponent : public Component {
	DrawablePtr			m_renderable;
};

using RenderableComponentPtr = std::shared_ptr<RenderableComponent>;