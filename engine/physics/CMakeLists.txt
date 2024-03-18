SET(files_physics
	physicsWorld.cc
	physicsWorld.h
	kernels.cc
	kernels.h
    )
SOURCE_GROUP("physics" FILES ${files_physics})
	
SET(files_pch ../config.h ../config.cc)
SOURCE_GROUP("pch" FILES ${files_pch})
ADD_LIBRARY(physics STATIC ${files_physics} ${files_pch})
TARGET_PCH(physics ../)
ADD_DEPENDENCIES(physics glew)
TARGET_LINK_LIBRARIES(physics PUBLIC engine exts glew soloud)