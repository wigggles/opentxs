
set(SWIG_INCLUDE_DIRS 
  SYSTEM
  ${CMAKE_CURRENT_SOURCE_DIR}/../deps/
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/ext
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/api
)

if(PYTHON)
  subdirs(python)
endif(PYTHON)

if(CSHARP)
  subdirs(csharp)
endif(CSHARP)

if(JAVA)
  subdirs(java)
endif(JAVA)

if(PHP)
  subdirs(php)
endif(PHP)

if(PERL)
  #subdirs(perl)
  message("Perl module currently breaks the build so is disabled.")
  message("See ./wrappers/CMakeLists.txt:28 to manually enable.")
endif(PERL)
