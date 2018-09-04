# Set headers
set(header_path "${TBABM_SOURCE_DIR}/include/TBABM")
set(header ${header_path}/Individual.h
		   ${header_path}/Household.h
		   ${header_path}/HouseholdGen.h
		   ${header_path}/TBABM.h)

# Set Demographic files
set(demographic_path "${TBABM_SOURCE_DIR}/src/Demographic")
set(demographic ${demographic_path}/event-Birth.cpp
				${demographic_path}/event-ChangeAgeGroup.cpp
				${demographic_path}/event-CreateHousehold.cpp
				${demographic_path}/event-CreatePopulation.cpp
				${demographic_path}/event-Death.cpp
				${demographic_path}/event-Divorce.cpp
				${demographic_path}/event-LeaveHousehold.cpp
				${demographic_path}/event-Marriage.cpp
				${demographic_path}/event-Matchmaking.cpp
				${demographic_path}/event-NewHouseholds.cpp
				${demographic_path}/event-Pregnancy.cpp
				${demographic_path}/event-SingleToLooking.cpp
				${demographic_path}/event-Survey.cpp
				${demographic_path}/event-UpdateHouseholds.cpp
				${demographic_path}/event-UpdatePyramid.cpp
				${demographic_path}/helper-InitialMortalityCheck.cpp)

set(hiv_path "${TBABM_SOURCE_DIR}/src/HIV")
set(hiv ${hiv_path}/event-ARTGuidelineChange.cpp
		${hiv_path}/event-ARTInitiate.cpp
		${hiv_path}/event-HIVInfection.cpp
		${hiv_path}/event-MortalityCheck.cpp
		${hiv_path}/event-VCTDiagnosis.cpp
		${hiv_path}/helper-ARTEligible.cpp
		${hiv_path}/helper-HIVInfectionCheck.cpp)		

# Set source files
set(src TBABM.cpp ${demographic} ${hiv} HouseholdGen.cpp)

# Require C++14 compilation
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add_compile_options("-c")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -c")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
# add_compile_options("${CMAKE_CXX_FLAGS} -c")
# set(CMAKE_LINK_LIBRARY_FLAG "-l")

# Include SimulationLib
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

# Configure config.hpp.in
# configure_file("config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/config_impl.hpp")

# Define library target
# add_library(TBABM ${header} ${src})

add_executable(TBABMTest test.cpp TBABM.cpp HouseholdGen.cpp ${src})
target_link_libraries(TBABMTest PUBLIC SimulationLib StatisticalDistributionsLib)

# target_include_directories(TBABM PUBLIC
  # $<BUILD_INTERFACE:${TBABM_SOURCE_DIR}/include> # for headers when building
  # $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for config_impl.hpp when building
  # $<INSTALL_INTERFACE:${include_dest}> # for client in install mode
  # $<INSTALL_INTERFACE:${lib_dest}>) # for config_impl.hpp in install mode

# target_link_libraries(TBABM PUBLIC SimulationLib StatisticalDistributionsLib)

# install(TARGETS TBABM EXPORT TBABM DESTINATION "${lib_dest}")
# install(FILES ${header} DESTINATION "${include_dest}")
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config_impl.hpp DESTINATION "${include_dest}")
