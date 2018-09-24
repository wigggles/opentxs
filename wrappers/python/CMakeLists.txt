message("** Python Wrapper")

set(OTX_WRAPPER_NAME "opentxs-python")

set(CMAKE_SWIG_FLAGS "")

find_package(SWIG 3.0.0 REQUIRED)
include(${SWIG_USE_FILE})

include_directories(
  ${SWIG_INCLUDE_DIRS}
)

# PYTHONLIBS_FOUND           - have the Python libs been found
# PYTHON_LIBRARIES           - path to the python library
# PYTHON_INCLUDE_PATH        - path to where Python.h is found (deprecated)
# PYTHON_INCLUDE_DIRS        - path to where Python.h is found
# PYTHON_DEBUG_LIBRARIES     - path to the debug library (deprecated)
# PYTHONLIBS_VERSION_STRING  - version of the Python libs found (since CMake 2.8.8)

set(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 2.7)

# find_package(PythonInterp)

if(PYTHON)
  find_package(PythonLibs)
elseif(PYTHON3)
  if(APPLE)
    message(STATUS "Using a hack to get Python wrappers building on a brewed Mac for Python3.5" )
    message(STATUS "When you actually build opentxs, the build will probably fail at the end on the python wrapper." )
    message(STATUS "It's because -bundle is being added instead of -dynamiclib" )
    message(STATUS "Go into the build/wrappers folder and find that spot, and change it to -dynamiclib" )
    message(STATUS "It will probably be in this file:  python/CMakeFiles/_opentxs-python.dir/link.txt")
    set(PYTHON_EXECUTABLE /usr/local/bin/python3)
    set(PYTHON_INCLUDE_DIR /usr/local/Cellar/python3/3.5.1/Frameworks/Python.framework/Versions/3.5/include/python3.5m)
    set(PYTHON_INCLUDE_DIRS /usr/local/Cellar/python3/3.5.1/Frameworks/Python.framework/Versions/3.5/include/python3.5m)
    set(PYTHON_INCLUDE_PATH /usr/local/Cellar/python3/3.5.1/Frameworks/Python.framework/Versions/3.5/include/python3.5m)
    set(PYTHON_LIBRARY /usr/local/Cellar/python3/3.5.1/Frameworks/Python.framework/Versions/3.5/lib/libpython3.5m.dylib)
    set(PYTHON_LIBRARIES /usr/local/Cellar/python3/3.5.1/Frameworks/Python.framework/Versions/3.5/lib/libpython3.5m.dylib)
    set(PYTHON_LIBRARY_DEBUG /usr/local/Cellar/python3/3.5.1/Frameworks/Python.PYTHON_LIBRARY_RELEASE**: framework/Versions/3.5/lib/libpython3.5.dylib)
    set(PYTHON_DEBUG_LIBRARIES /usr/local/Cellar/python3/3.5.1/Frameworks/Python.PYTHON_LIBRARY_RELEASE**: framework/Versions/3.5/lib/libpython3.5.dylib)
  else(APPLE)
    set(PYTHON_EXECUTABLE /usr/bin/python3.6)
    set(PYTHON_INCLUDE_DIR /usr/include/python3.6)
    set(PYTHON_INCLUDE_DIRS /usr/include/python3.6)
    set(PYTHON_INCLUDE_PATH /usr/include/python3.6)
    set(PYTHON_LIBRARY /usr/lib/x86_64-linux-gnu/libpython3.6m.so)
    set(PYTHON_LIBRARIES /usr/lib/x86_64-linux-gnu/libpython3.6m.so)
    set(PYTHON_LIBRARY_DEBUG /usr/lib/x86_64-linux-gnu/libpython3.6m.so)
  endif(APPLE)
elseif(PYTHON2)
  find_package(PythonLibs 2.7 REQUIRED)
endif(PYTHON)

message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")

include_directories(
  ${PYTHON_INCLUDE_PATH}
)

set_source_files_properties(OTX_.i PROPERTIES CPLUSPLUS ON)
swig_add_module(${OTX_WRAPPER_NAME} python OTX_.i)
swig_link_libraries(${OTX_WRAPPER_NAME} ${PYTHON_LIBRARIES})

set(MOD_NAME ${SWIG_MODULE_${OTX_WRAPPER_NAME}_REAL_NAME})

target_link_libraries(${MOD_NAME} ${OTX_LINK_LIBS})
set_target_properties(${MOD_NAME}
                      PROPERTIES OUTPUT_NAME _opentxs)
set_lib_property(${MOD_NAME})
string(REGEX REPLACE "\\.[0-9]+$" "" PYTHON_VERSION "${PYTHONLIBS_VERSION_STRING}")
install(TARGETS ${MOD_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION}/site-packages
        COMPONENT python)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/opentxs.py
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION}/site-packages
        COMPONENT python)

