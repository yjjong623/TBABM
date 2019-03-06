cmake_minimum_required(VERSION 3.3)
project (TBABM)

find_package(Boost REQUIRED)
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)

# Define library version
# set(TBABM_VERSION_MAJOR 0 CACHE STRING "major version" FORCE)
# set(TBABM_VERSION_MINOR 1 CACHE STRING "minor version" FORCE)
# set(TBABM_VERSION ${TBABM_VERSION_MAJOR}.${TBABM_VERSION_MINOR} CACHE STRING "version" FORCE)

# Define install destinations
# set(include_dest "include/TBABM-${TBABM_VERSION}")
# set(main_lib_dest "lib/TBABM-${TBABM_VERSION}")
# set(lib_dest ${main_lib_dest}/${CMAKE_BUILD_TYPE})

# Define options here

# Add subdirectories
include_directories(${Boost_INCLUDE_DIRS})
add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

# install(FILES TBABMConfig.cmake DESTINATION ${main_lib_dest})
# install(EXPORT TBABM DESTINATION ${lib_dest})
