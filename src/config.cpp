#include "config.h"

json config;

bool loadConfig(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Could not open config file: " << path << std::endl;
        return false;
    }

    f >> config;
    return true;
}