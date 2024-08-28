
#pragma once
#define GLM_FORCE_SSE2
#define GLM_ENABLE_EXPERIMENTAL

#ifdef STRAFE_DEBUG
	#define GLM_FORCE_MESSAGES
	#define GLM_FORCE_CTOR_INIT
#else
	#define GLM_FORCE_INLINE
#endif

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
