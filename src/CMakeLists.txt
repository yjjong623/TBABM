cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(TBABM)

# Include SimulationLib
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

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

set(tbabm_path "${TBABM_SOURCE_DIR}")
set(tbabm ${tbabm_path}/TBABM.cpp
		  ${tbabm_path}/test.cpp
		  ${tbabm_path}/MasterData.cpp)

# Set source files
set(src ${tbabm} ${demographic} ${hiv} ${tb} ${individual} ${household})

include_directories(${Boost_INCLUDE_DIRS})

configure_file("RunTBABM" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RunTBABM" COPYONLY)
configure_file("household_structure.csv" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/household_structure.csv" COPYONLY)

add_executable(TBABM ${src})
target_compile_features(TBABM PUBLIC cxx_std_14)
target_link_libraries(TBABM PUBLIC SimulationLib StatisticalDistributionsLib ${CMAKE_THREAD_LIBS_INIT})
