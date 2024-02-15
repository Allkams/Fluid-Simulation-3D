PROJECT(testproject)
FILE(GLOB project_headers code/*.h)
FILE(GLOB project_sources code/*.cc)

SET(files_project ${project_headers} ${project_sources})

SOURCE_GROUP("testproject" FILES ${files_project})

ADD_EXECUTABLE(testproject ${files_project})

TARGET_LINK_LIBRARIES(testproject core render physics misc)
ADD_DEPENDENCIES(testproject core render physics misc)

IF(MSVC)
    set_property(TARGET testproject PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
ENDIF()