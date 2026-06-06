// SPDX-License-Identifier: Apache-2.0

#include "override.h"

static OverrideDir_t s_dir = OVR_NONE;

void Override_Init(void)
{
    s_dir = OVR_NONE;
}

void Override_Set(OverrideDir_t dir)
{
    s_dir = dir;
}

OverrideDir_t Override_Get(void)
{
    return s_dir;
}
