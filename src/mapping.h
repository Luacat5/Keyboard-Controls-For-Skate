#pragma once

#include <unordered_map>
#include <fstream>
#include <windows.h>

#include "config.h"


bool GetKeyDown(std::string);
bool processKeyEvent(int, WPARAM);
std::unordered_map< int, std::string> MakeKeyMap();