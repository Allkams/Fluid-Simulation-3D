PROJECT(fluidsim)
FILE(GLOB project_headers code/*.h)
FILE(GLOB project_sources code/*.cc)

SET(files_project ${project_headers} ${project_sources})

SOURCE_GROUP("fluidsim" FILES ${files_project})

SET(simulation_folder "code/simulations")

FILE(GLOB simulation_headers ${simulation_folder}/*.h)
FILE(GLOB simulation_sources ${simulation_folder}/*.cc)

SET(files_simulation ${simulation_headers} ${simulation_sources})

SOURCE_GROUP("fluidsim\\simulations" FILES ${files_simulation})

SET(all_files ${files_project} ${files_simulation})

ADD_EXECUTABLE(fluidsim ${all_files})

TARGET_LINK_LIBRARIES(fluidsim core render physics misc)
ADD_DEPENDENCIES(fluidsim core render physics misc)

IF(MSVC)
    set_property(TARGET fluidsim PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
ENDIF()