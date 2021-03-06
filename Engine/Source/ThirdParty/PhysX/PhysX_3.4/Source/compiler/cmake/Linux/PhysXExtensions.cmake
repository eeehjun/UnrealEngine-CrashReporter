#
# Build PhysXExtensions
#

SET(PHYSX_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../)

SET(LL_SOURCE_DIR ${PHYSX_SOURCE_DIR}/PhysXExtensions/src)

SET(PHYSXEXTENSIONS_PLATFORM_INCLUDES
	PRIVATE ${PHYSX_SOURCE_DIR}/Common/src/linux
)

SET(PHYSXEXTENSIONS_PLATFORM_SRC_FILES

)


# Use generator expressions to set config specific preprocessor definitions
SET(PHYSXEXTENSIONS_COMPILE_DEFS
	# NOTE: PX_BUILD_NUMBER - probably not good!
	# Common to all configurations
	${PHYSX_LINUX_COMPILE_DEFS};PX_BUILD_NUMBER=0;PX_PHYSX_STATIC_LIB;
)

if(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "debug")
	LIST(APPEND PHYSXEXTENSIONS_COMPILE_DEFS
		${PHYSX_LINUX_DEBUG_COMPILE_DEFS}
	)
elseif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "checked")
	LIST(APPEND PHYSXEXTENSIONS_COMPILE_DEFS
		${PHYSX_LINUX_CHECKED_COMPILE_DEFS}
	)
elseif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "profile")
	LIST(APPEND PHYSXEXTENSIONS_COMPILE_DEFS
		${PHYSX_LINUX_PROFILE_COMPILE_DEFS}
	)
elseif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "release")
	LIST(APPEND PHYSXEXTENSIONS_COMPILE_DEFS
		${PHYSX_LINUX_RELEASE_COMPILE_DEFS}
	)
else(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "debug")
	MESSAGE(FATAL_ERROR "Unknown configuration ${CMAKE_BUILD_TYPE}")
endif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "debug")


# include common PhysXExtensions settings
INCLUDE(../common/PhysXExtensions.cmake)

# enable -fPIC so we can link static libs with the editor
SET_TARGET_PROPERTIES(PhysXExtensions PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

TARGET_LINK_LIBRARIES(PhysXExtensions PUBLIC PsFastXml)
