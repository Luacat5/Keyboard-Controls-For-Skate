
#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>


using json = nlohmann::json;

extern json config;

bool loadConfig(const std::string& path);