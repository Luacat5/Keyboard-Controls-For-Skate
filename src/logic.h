#pragma once

#include "config.h"
#include "mapping.h"
#include "hooks.h"

#include <iostream>
#include <Xinput.h>
#include <VigEm/Client.h>
#include <unordered_map>
#include <windows.h>
#include <fstream>


// Example main loop function
bool initController();
void mainLogicLoop();
