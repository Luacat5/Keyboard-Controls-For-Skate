cmake_minimum_required(VERSION 3.15)  # 4.1 is not valid, max is 3.x
project(skate_controls)


add_subdirectory(json)

add_subdirectory(ViGEmClient)

add_executable( skate_controls 
    src/main.cpp 
    src/config.cpp
    src/logic.cpp 
    src/mapping.cpp
    src/hooks.cpp
    )

# Link against ViGEmClient and required Windows system libs
target_link_libraries(skate_controls
    PRIVATE
        ViGEmClient
        winhttp
        shell32
        setupapi
)

target_include_directories(skate_controls PRIVATE ViGEmClient/include)
target_link_libraries(skate_controls PRIVATE nlohmann_json::nlohmann_json)
