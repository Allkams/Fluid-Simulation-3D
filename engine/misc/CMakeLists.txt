SET(files_misc
	temp.cc
	temp.h
    )
SOURCE_GROUP("misc" FILES ${files_misc})
	
SET(files_pch ../config.h ../config.cc)
SOURCE_GROUP("pch" FILES ${files_pch})
ADD_LIBRARY(misc STATIC ${files_misc} ${files_pch})
TARGET_PCH(misc ../)
ADD_DEPENDENCIES(misc glew)
TARGET_LINK_LIBRARIES(misc PUBLIC engine exts glew soloud)