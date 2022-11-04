cmake_minimum_required (VERSION 3.8)

project("Nocturn-Server" VERSION 1.0.0)

add_subdirectory ("Server")
add_subdirectory ("Client")
add_subdirectory ("NocturnLib")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)