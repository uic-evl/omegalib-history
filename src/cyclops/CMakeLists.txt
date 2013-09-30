add_definitions( -D_CRT_SECURE_NO_WARNINGS -DCY_EXPORTING)
set(SRCS 
		Entity.cpp
		EffectNode.cpp
		AnimatedObject.cpp
		LineSet.cpp
		Light.cpp
		Material.cpp
		MaterialParser.cpp
		ModelLoader.cpp
		ModelGeometry.cpp
		Shapes.cpp
		Skybox.cpp
		SceneLoader.cpp
		SceneManager.cpp
		StaticObject.cpp
		Text3D.cpp
		Uniforms.cpp
		cyclopsPythonApi.cpp)
    
set(HEADERS 
		${CMAKE_SOURCE_DIR}/include/cyclops/Entity.h
		${CMAKE_SOURCE_DIR}/include/cyclops/EffectNode.h
		${CMAKE_SOURCE_DIR}/include/cyclops/AnimatedObject.h
		${CMAKE_SOURCE_DIR}/include/cyclops/LineSet.h
		${CMAKE_SOURCE_DIR}/include/cyclops/Light.h
		${CMAKE_SOURCE_DIR}/include/cyclops/Material.h
		${CMAKE_SOURCE_DIR}/include/cyclops/MaterialParser.h
		${CMAKE_SOURCE_DIR}/include/cyclops/ModelLoader.h
		${CMAKE_SOURCE_DIR}/include/cyclops/ModelGeometry.h
		${CMAKE_SOURCE_DIR}/include/cyclops/cyclopsConfig.h
		${CMAKE_SOURCE_DIR}/include/cyclops/Shapes.h
		${CMAKE_SOURCE_DIR}/include/cyclops/Text3D.h
		${CMAKE_SOURCE_DIR}/include/cyclops/Skybox.h
		${CMAKE_SOURCE_DIR}/include/cyclops/StaticObject.h
		${CMAKE_SOURCE_DIR}/include/cyclops/SceneLoader.h
		${CMAKE_SOURCE_DIR}/include/cyclops/SceneManager.h
		${CMAKE_SOURCE_DIR}/include/cyclops/Uniforms.h)
		
###############################################################################
# Setup compile info
add_library( cyclops SHARED ${SRCS} ${HEADERS})

source_group(osg REGULAR_EXPRESSION osg/)		
target_link_libraries(cyclops
	omega 
	omegaToolkit
    omegaOsg)
