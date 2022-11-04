cmake_minimum_required (VERSION 3.8)

project (Client)

set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB _root_files LIST_DIRECTORIES false "${_src_root_path}/*.h" "${_src_root_path}/*.cpp" "${_src_root_path}/.c")

add_executable (Client 
	${_root_files}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Client PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(Client NocturnLib)
