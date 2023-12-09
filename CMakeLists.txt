cmake_minimum_required(VERSION 3.10)

# set the project name
project(index)

# set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_SYSTEM_NAME Windows)
# set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
# set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# get all source files
file(GLOB_RECURSE OBJECTS_SOURCES src/objects/*.cpp)
file(GLOB_RECURSE SCENE_SOURCES src/scenes/*.cpp)
file(GLOB_RECURSE CHARLIE_SOURCES /home/ethanscharlie/Projects/Code/Github/charlie2d/source/*.cpp)

# add the executable
add_executable(index
    src/main.cpp
    ${OBJECTS_SOURCES}
    ${SCENE_SOURCES}
    ${CHARLIE_SOURCES}
)
 
# add include directories
target_include_directories(index PUBLIC
    /usr/include/SDL2
    /usr/include/nlohmann
    #/home/ethanscharlie/Projects/Code/C++/Charlie2D/include
    /home/ethanscharlie/Projects/Code/Github/charlie2d/include
    ${PROJECT_SOURCE_DIR}/include 
    ${PROJECT_SOURCE_DIR}/src/engine 
    ${PROJECT_SOURCE_DIR}/src/objects 
    ${PROJECT_SOURCE_DIR}/src/scenes 
    ${PROJECT_SOURCE_DIR}/src
)

find_library(CHARLIE_LIB charlie2D PATHS /home/ethanscharlie/Projects/Code/Github/charlie2d/build)

# link libraries
target_link_libraries(index
    # ${CHARLIE_LIB}
    SDL2
    SDL2_ttf
    SDL2_mixer
)

# copy image directory to build directory
file(COPY ${CMAKE_SOURCE_DIR}/img DESTINATION ${CMAKE_BINARY_DIR})

# set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

if (EMSCRIPTEN)
  # Enable Emscripten-specific flags
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_IMAGE=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s SDL2_IMAGE_FORMATS=['png']")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_TTF=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_MIXER=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_MIXER=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS=\"['_main', '_on_resize']\"")
  #set_target_properties(index PROPERTIES LINK_FLAGS "-s EXPORTED_FUNCTIONS='_on_resize'")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXTRA_EXPORTED_RUNTIME_METHODS=\"['ccall']\"")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I /usr/include/nlohmann")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --use-preload-plugins")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --shell-file ../minimal.html")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file ${CMAKE_SOURCE_DIR}/img@/img")
else()
    target_link_libraries(index
        SDL2_image
)
endif()

