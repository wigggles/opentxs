# Copyright (c) Monetas AG, 2014

if(PYTHON OR PERL OR RUBY OR TCL OR JAVA OR PHP)
  find_package(SWIG REQUIRED)
  include(${SWIG_USE_FILE})
  set(CMAKE_SWIG_FLAGS "")
endif()


include_directories(SYSTEM
  ${CMAKE_CURRENT_SOURCE_DIR}/../deps/
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/ext
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/api
)


if(PYTHON)
  find_package(PythonLibs)
  include_directories(${PYTHON_INCLUDE_PATH})
  set_source_files_properties(OTAPI.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(otapi-python python OTAPI.i)
  swig_link_libraries(otapi-python ${PYTHON_LIBRARIES})
endif()


if(PERL)
  find_package(PerlLibs)
  include_directories(${PERL_INCLUDE_PATH})
  set_source_files_properties(OTAPI.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(otapi-perl perl OTAPI.i)
  swig_link_libraries(otapi-perl ${PERL_LIBRARIES})
endif()


if(RUBY)
  find_package(Ruby)
  include_directories(${RUBY_INCLUDE_PATH})
  set_source_files_properties(OTAPI.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(otapi-ruby ruby OTAPI.i)
  swig_link_libraries(otapi-ruby ${RUBY_LIBRARIES})
endif()


if(TCL)
  find_package(TCL)
  include_directories(${TCL_INCLUDE_PATH})
  set_source_files_properties(OTAPI.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(otapi-tcl tcl OTAPI.i)
  swig_link_libraries(otapi-tcl ${TCL_LIBRARIES})
endif()


if(JAVA)
  find_package(Java)
  include_directories(${JAVA_INCLUDE_PATH})
  set_source_files_properties(OTAPI.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(otapi-java java OTAPI.i)
  swig_link_libraries(otapi-java ${JAVA_LIBRARIES})
endif()


if(PHP)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
  find_package(PHP5)
  include_directories(${PHP5_INCLUDE_DIR} ${PHP5_MAIN_INCLUDE_DIR} ${PHP5_ZEND_INCLUDE_DIR} ${PHP5_TSRM_INCLUDE_DIR})
  set_source_files_properties(OTAPI.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(otapi-php php OTAPI.i)
  swig_link_libraries(otapi-php ${PHP_LIBRARIES})
endif()


if(GO)
  ##
endif()


if(D)
  ##
endif()


if(CSHARP)
  ##
endif()
