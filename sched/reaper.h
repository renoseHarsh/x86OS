#pragma once

#include "thread.h"

void init_reaper();
void reaper_add(Thread *t);
