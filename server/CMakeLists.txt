# CMakeList.txt : CMake project for Beej, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Server"
VERSION 0.1.0
LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Add source to this project's executable.
add_executable(server "./main.cpp" "src/interface.cpp" "include/interface.h")
target_link_libraries(server PRIVATE wsock32 ws2_32 common)
target_include_directories(server PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")