PROJECT(simulation)
FILE(GLOB project_headers code/*.h)
FILE(GLOB project_sources code/*.cc)

SET(files_project ${project_headers} ${project_sources})

SOURCE_GROUP("simulation" FILES ${files_project})

ADD_EXECUTABLE(simulation ${files_project})

TARGET_LINK_LIBRARIES(simulation core render physics misc)
ADD_DEPENDENCIES(simulation core render physics misc)

IF(MSVC)
    set_property(TARGET simulation PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
ENDIF()