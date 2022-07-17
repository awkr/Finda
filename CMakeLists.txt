cmake_minimum_required(VERSION 3.20)

project(Finda)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-x objective-c++")
# set(CMAKE_EXE_LINKER_FLAGS
#     "-framework CoreGraphics -framework QuartzCore -framework UIKit"
# )

set(BUNDLE_IDENTIFIER "com.bilibili.finda")
set(CODE_SIGN_IDENTITY "iPhone Developer")
set(DEPLOYMENT_TARGET 9.0)
set(SAMPLE_NAME "Finda")

set(EXECUTABLE_NAME ${SAMPLE_NAME})
set(PRODUCT_NAME ${SAMPLE_NAME})
set(MACOSX_BUNDLE_BUNDLE_NAME ${BUNDLE_IDENTIFIER})
set(MACOSX_BUNDLE_EXECUTABLE_NAME ${SAMPLE_NAME})
set(MACOSX_BUNDLE_GUI_IDENTIFIER ${BUNDLE_IDENTIFIER})
set(MACOSX_BUNDLE_INFO_STRING ${BUNDLE_IDENTIFIER})
set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
set(MACOSX_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET})

file(GLOB_RECURSE Headers "src/*.h" "src/*.hpp")
file(GLOB_RECURSE Sources "src/*.m" "src/*.mm" "src/*.cpp")
file(GLOB_RECURSE Resources "resources/*.storyboard")

find_library(UIKit_Lib UIKit)
find_library(Foundation_Lib Foundation)
find_library(QuartzCore_Lib QuartzCore)
find_library(OpenGLES_Lib OpenGLES)
find_library(CoreGraphics_Lib CoreGraphics)

add_executable(${SAMPLE_NAME} MACOSX_BUNDLE ${Headers} ${Sources} ${Resources})

target_link_libraries(${SAMPLE_NAME} ${UIKit_Lib} ${Foundation_Lib} ${QuartzCore_Lib} ${OpenGLES_Lib} ${CoreGraphics_Lib})

set_target_properties(${SAMPLE_NAME} PROPERTIES
    XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "ALL"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET}
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CODE_SIGN_IDENTITY}
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "FTSW9V26U6"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist.in
    RESOURCE "${Resources}"
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
)
