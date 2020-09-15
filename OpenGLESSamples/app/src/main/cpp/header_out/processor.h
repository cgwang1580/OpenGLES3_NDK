//
// Created by wcg3031 on 2020/4/1.
//

#pragma once

#include <MotionState.h>

typedef void* 	PHandle;

int onSurfaceCreated (PHandle *ppProcessorHandle, const int effectType = 0);
int onSurfaceChanged (const PHandle pProcessorHandle, const int width, const int height);
int setMotionState (const PHandle pProcessorHandle, MotionState const motionState);
int onDrawFrame (const PHandle pProcessorHandle);
int getTextureFromFrameBuffer (const PHandle pProcessorHandle);
int onSurfaceDestroyed (PHandle *ppProcessorHandle);

