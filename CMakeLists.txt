# Require minimum cmake version
cmake_minimum_required(VERSION 3.8...3.20)

# Define project
project(KLMC_SAC)

# Activate all available UI and vis drivers by default
# Can set WITH_GEANT4_UIVIS to OFF with cmake (-DWITH_GEANT4_UIVIS=OFF)
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)

# Find Geant4 package
if(WITH_GEANT4_UIVIS)
	find_package(Geant4 10.6.2 REQUIRED ui_all vis_all)
else()
	find_package(Geant4 10.6.2 REQUIRED)
endif()

# Find ROOT package
find_package(ROOT CONFIG REQUIRED)

# Set CMake build type to debug
set(CMAKE_BUILD_TYPE Debug)

# Setup Geant4 include directories and compile definitions
include(${Geant4_USE_FILE})

# Configure KLMC_SAC to use project, Geant4 headers, and ROOT headers
include_directories(${PROJECT_SOURCE_DIR}/include ${Geant4_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})

# Locate headers and sources for this project
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)

# Add the executable and link it to the Geant4 and ROOT libraries
add_executable(KLMC_SAC KLMC_SAC.cc ${headers} ${sources})
target_link_libraries(KLMC_SAC ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

# Add SAC to build
add_subdirectory(${PROJECT_SOURCE_DIR}/SAC) # Add SAC subdirectory to build
include_directories(${PROJECT_SOURCE_DIR}/SAC/include) # Specify SAC include directory
target_link_libraries(KLMC_SAC SAC) # Link with KLMC_SAC libraries

# Copy over macro files
file(COPY macros DESTINATION .)

# Install executable
install(TARGETS KLMC_SAC DESTINATION bin)
