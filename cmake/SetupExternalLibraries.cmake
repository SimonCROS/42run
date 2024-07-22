# ---------------------------------------------------------------------------------
# GLM Configuration
# ---------------------------------------------------------------------------------
set(GLM_BUILD_LIBRARY ON)

# ---------------------------------------------------------------------------------
# Add libraries subdirectories
# ---------------------------------------------------------------------------------
add_subdirectory(${EXTERNAL_LIBRARIES_DIR}/glm EXCLUDE_FROM_ALL)
add_subdirectory(${EXTERNAL_LIBRARIES_DIR}/glad EXCLUDE_FROM_ALL)
add_subdirectory(${EXTERNAL_LIBRARIES_DIR}/stb EXCLUDE_FROM_ALL)
add_subdirectory(${EXTERNAL_LIBRARIES_DIR}/tinygltf EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------------
# Find OpenGL and glfw
# ---------------------------------------------------------------------------------
find_package(OpenGL 4.1 REQUIRED)
find_package(glfw3 3.3 REQUIRED)
