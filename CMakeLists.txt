cmake_minimum_required(VERSION 3.8)
project (firmware_upgrade  LANGUAGES C )



add_subdirectory(submodule/G_luaSerialPort)
add_subdirectory(submodule/libuv)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/lfk
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/submodule/G_luaSerialPort
    ${CMAKE_SOURCE_DIR}/submodule
)

file(GLOB src
         ${CMAKE_CURRENT_SOURCE_DIR}/*.c
		 ${CMAKE_CURRENT_SOURCE_DIR}/lfk/fw0/*.c
)


add_library(${PROJECT_NAME} SHARED ${src} )
add_executable(${PROJECT_NAME}_exe  ${src} )
target_link_libraries(${PROJECT_NAME} PUBLIC 
G_luaSerialPort
uv_a
)
target_link_libraries(${PROJECT_NAME}_exe PUBLIC 
G_luaSerialPort
uv_a
)
get_target_property(libs ${PROJECT_NAME} LINK_LIBRARIES)
message("${PROJECT_NAME} libs :${libs}")