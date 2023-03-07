cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(UEditor VERSION 0.0.1 LANGUAGES CXX)

file(GLOB_RECURSE UEDITOR_SOURCE_FILES "src/*.h" "src/*.cpp")

add_executable(${PROJECT_NAME} ${UEDITOR_SOURCE_FILES})

set(UENGINE_UEDITOR_PROFILE ON CACHE BOOL "" FORCE)
set(UENGINE_BUILD_TESTS ON CACHE BOOL "" FORCE)
add_subdirectory("external/uengine")

target_link_libraries(${PROJECT_NAME} PRIVATE UEngine)

target_include_directories(${PROJECT_NAME} PRIVATE "src")

target_compile_definitions(${PROJECT_NAME} 
	PRIVATE 
		UEDITOR
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} 
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY "assets/" DESTINATION "assets")