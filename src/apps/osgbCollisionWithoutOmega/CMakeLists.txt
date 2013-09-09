###################################################################################################
# Shi
# Source files
set(SRCS 
		collision.cpp)
    
###################################################################################################
# Setup compile info
add_executable(osgbCollisionWithoutOmega ${SRCS})
set_target_properties(osgbCollisionWithoutOmega PROPERTIES FOLDER osgBulletExamples)
target_link_libraries(osgbCollisionWithoutOmega
	omega
	omegaToolkit
	omegaOsg
	${BULLET_LIBS}
	${OSGBULLET_LIBS})
# Yin