cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(TBABM)

# Include SimulationLib
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads)

# Require C++14 compilation
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CLANG_DEFAULT_CXX_STDLIB "libc++")
set(CMAKE_BUILD_TYPE Debug)

# Set Demographic files
set(demographic_path "${TBABM_SOURCE_DIR}/Demographic")
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
				${demographic_path}/event-ExogenousBirth.cpp
				${demographic_path}/helper-InitialEvents.cpp
				${demographic_path}/helper-SurveyUtils.cpp)

set(individual_path "${TBABM_SOURCE_DIR}/Individual")
set(individual ${individual_path}/IndividualTypes.cpp)

set(household_path "${TBABM_SOURCE_DIR}/Household")
set(household ${household_path}/Household.cpp
			  ${household_path}/HouseholdGen.cpp)

set(hiv_path "${TBABM_SOURCE_DIR}/HIV")
set(hiv ${hiv_path}/event-ARTGuidelineChange.cpp
		${hiv_path}/event-ARTInitiate.cpp
		${hiv_path}/event-HIVInfection.cpp
		${hiv_path}/event-MortalityCheck.cpp
		${hiv_path}/event-VCTDiagnosis.cpp
		${hiv_path}/helper-ARTEligible.cpp
		${hiv_path}/helper-HIVInfectionCheck.cpp)

set(tb_path "${TBABM_SOURCE_DIR}/TB")
set(tb ${tb_path}/TB.cpp
	   ${tb_path}/TBTypes.cpp)

# Set source files
set(src test.cpp TBABM.cpp ${demographic} ${hiv} ${tb} ${individual} ${household})

include_directories(${Boost_INCLUDE_DIRS})

add_executable(TBABM ${src})
target_link_libraries(TBABM PUBLIC SimulationLib StatisticalDistributionsLib ${CMAKE_THREAD_LIBS_INIT})
