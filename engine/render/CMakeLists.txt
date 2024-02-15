SET(files_render
	window.cc
	window.h
	RenderBasic.cc
	RenderBasic.h
	shader.cc
	shader.h
	camera.cc
	camera.h
	computeshader.cc
	computeshader.h
    )
SOURCE_GROUP("render" FILES ${files_render})
	
SET(files_pch ../config.h ../config.cc)
SOURCE_GROUP("pch" FILES ${files_pch})
ADD_LIBRARY(render STATIC ${files_render} ${files_pch})
TARGET_PCH(render ../)
ADD_DEPENDENCIES(render exts imgui glew glfw glm_static)
TARGET_LINK_LIBRARIES(render PUBLIC engine exts glew glfw imgui ${OPENGL_LIBS} glm_static)