set(PROJECT_NAME VulkanRayTracing)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "Camera.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "Maths.cpp"
    "Maths.h"
    "Matrix2.cpp"
    "Matrix2.h"
    "Matrix3.cpp"
    "Matrix3.h"
    "Matrix4.cpp"
    "Matrix4.h"
    "Plane.cpp"
    "Plane.h"
    "Quaternion.cpp"
    "Quaternion.h"
    "Vector2.cpp"
    "Vector2.h"
    "Vector3.cpp"
    "Vector3.h"
    "Vector4.cpp"
    "Vector4.h"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
target_precompile_headers(${PROJECT_NAME} PRIVATE
    <memory>
    <unordered_set>
    <vector>
    <string>
    <fstream>
    <iostream>
    <string>
    <iosfwd>
    <set>
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
    )
endif()