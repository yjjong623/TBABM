# Set headers
set(header_path "${TBABM_SOURCE_DIR}/include/TBABM")
set(header ${header_path}/Individual.h
		   ${header_path}/Household.h
		   ${header_path}/HouseholdGen.h
		   ${header_path}/TBABM.h)

# Set source files
set(src TBABM.cpp)

# Require C++14 compilation
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include SimulationLib
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)

# Configure config.hpp.in
# configure_file("config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/config_impl.hpp")

# Define library target
add_library(TBABM ${header} ${src})

target_include_directories(TBABM PUBLIC
  $<BUILD_INTERFACE:${TBABM_SOURCE_DIR}/include> # for headers when building
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for config_impl.hpp when building
  $<INSTALL_INTERFACE:${include_dest}> # for client in install mode
  $<INSTALL_INTERFACE:${lib_dest}>) # for config_impl.hpp in install mode

target_link_libraries(TBABM PUBLIC SimulationLib StatisticalDistributionsLib)

install(TARGETS TBABM EXPORT TBABM DESTINATION "${lib_dest}")
install(FILES ${header} DESTINATION "${include_dest}")
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config_impl.hpp DESTINATION "${include_dest}")
