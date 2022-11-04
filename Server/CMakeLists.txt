project(Server)

# Create the executable
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    NocturnLib
)

target_compile_options(Server PUBLIC "/Zc:__cplusplus")
set_property(TARGET Server PROPERTY CXX_STANDARD 20)