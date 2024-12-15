project(chip8_emulator)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})

# Add executable
add_executable(chip8
    src/main.cpp
    src/Display.cpp
    src/Chip8.cpp
)

# Link libraries
target_link_libraries(chip8 PRIVATE ${SDL2_LIBRARIES}) 