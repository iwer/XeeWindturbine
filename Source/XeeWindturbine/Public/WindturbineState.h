// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "Engine.h"

UENUM()
enum EWindTurbineState {
    Stopped,
    SafetyStopped,
    Starting,
    Running,
    Stopping,
    SafetyStopping
};
