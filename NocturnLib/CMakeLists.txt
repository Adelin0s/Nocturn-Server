project(NocturnLibrary)

set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}/Nocturn")

file(GLOB _root_files		LIST_DIRECTORIES false "${_src_root_path}/*.h" "${_src_root_path}/*.cpp" "${_src_root_path}/.hpp")
file(GLOB _platform_files	LIST_DIRECTORIES false "${_src_root_path}/Platform/*.h" "${_src_root_path}/Platform/*.cpp" "${_src_root_path}/Platform/*.hpp")

add_library(${PROJECT_NAME} 
	${_root_files}
	${_platform_files}
)
add_library(NocturnLib ALIAS ${PROJECT_NAME})

source_group("Nocturn" FILES ${_root_files})
source_group("Nocturn/Platform" FILES ${_platform_files})

target_include_directories( ${PROJECT_NAME} PUBLIC 
	${PROJECT_SOURCE_DIR}/
)

target_compile_options(NocturnLibrary PUBLIC "/Zc:__cplusplus")
set_property(TARGET NocturnLibrary PROPERTY CXX_STANDARD 20)