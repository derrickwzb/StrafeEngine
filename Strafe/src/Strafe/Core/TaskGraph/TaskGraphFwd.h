#pragma once
#include "Strafe/Core/Utils/RefCounting.h"

class BaseGraphTask;

//convenient typedef for reference counted pte t a graph event
using GraphEventRef = RefCountPtr<class GraphEvent>;