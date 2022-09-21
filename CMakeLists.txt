add_library(DiskViewer SHARED disktypes.h diskviewer.cpp diskviewer.h DiskViewer_global.h fsoperations.cpp fsoperations.h nativefunc.cpp nativefunc.h nativetypes.h ntdir.cpp ntdir.h)
add_compile_definitions(DISKVIEWER_LIBRARY)

target_include_directories(DiskViewer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
