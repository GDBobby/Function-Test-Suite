include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.23)

project(TestMathSuite VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT LAB_PATH)
  set(LAB_PATH C:/Projects/LinearAlgebra)
  message(STATUS "LAB_PATH not specified in .env.cmake, using ${LAB_PATH}")
endif()
add_subdirectory(${LAB_PATH} ${CMAKE_BINARY_DIR}/LinearAlgebra-build)

if(NOT DX9_PATH)
  set(DX9_PATH C:/Projects/collab/Duelists/Stable/sdk/dx9)
  message(STATUS "DX9_PATH not specified in .env.cmake, using ${DX9_PATH}")
endif()


file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.*)

add_executable(${PROJECT_NAME} ${SOURCES})

option(USE_SSE_INTERNAL "Enable SSE optimizations" OFF)
option(USE_AVX2_INTERNAL "Enable AVX2 optimizations" ON)
  if(USE_SSE_INTERNAL)
      message(STATUS "Enabling SSE ")
      add_compile_definitions(LAB_USING_SSE)
      if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
      else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
    endif()
  elseif(USE_AVX2_INTERNAL)
      message(STATUS "Enabling AVX2")
      add_compile_definitions(LAB_USING_AVX2)
      if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
      else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
      endif()
  endif()

add_compile_definitions(LAB_ROW_MAJOR)

message(STATUS "including directories")
target_include_directories(${PROJECT_NAME} PUBLIC
	${DX9_PATH}/Include
  ${LAB_PATH}
)
message(STATUS "linking directories")
target_link_directories(${PROJECT_NAME} PUBLIC
	${DX9_PATH}/lib/x64
)

message(STATUS "linking libs")
target_link_libraries(${PROJECT_NAME}
	$<$<CONFIG:Debug>:d3dx9d.lib>
	$<$<CONFIG:Release>:d3dx9.lib>
)

	
# Create source groups to maintain file structure in Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})