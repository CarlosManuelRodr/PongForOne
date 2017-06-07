cmake_minimum_required(VERSION 2.8)

project(PongForOne)

find_package(PkgConfig REQUIRED)
pkg_search_module(SFML REQUIRED sfml-all)

add_executable(${PROJECT_NAME} 
	main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${SFML_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})

file(COPY "resources/" DESTINATION "resources/")
