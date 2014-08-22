# Copyright (c) Monetas AG, 2014

if(PYTHON OR PERL OR RUBY OR TCL OR JAVA OR PHP)
  find_package(SWIG 3.0.0 REQUIRED)
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
  set_source_files_properties(opentxs.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(opentxs-python python opentxs.i)
  swig_link_libraries(opentxs-python ${PYTHON_LIBRARIES})
  target_link_libraries(_opentxs-python opentxs-api)
  set_target_properties(_opentxs-python
                      PROPERTIES OUTPUT_NAME _opentxs)
  string(REGEX REPLACE "\\.[0-9]+$" "" PYTHON_VERSION "${PYTHONLIBS_VERSION_STRING}")
  install(TARGETS _opentxs-python
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION}/site-packages
          COMPONENT python)
  install(FILES
          ${CMAKE_CURRENT_BINARY_DIR}/opentxs.py
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION}/site-packages
          COMPONENT python)
endif()


if(PERL)
  find_package(PerlLibs)
  include_directories(${PERL_INCLUDE_PATH})
  set_source_files_properties(opentxs.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(opentxs-perl perl opentxs.i)
  swig_link_libraries(opentxs-perl ${PERL_LIBRARIES})
  target_link_libraries(_opentxs-perl opentxs-api)
  set_target_properties(_opentxs-perl
                      PROPERTIES OUTPUT_NAME _opentxs)
  install(TARGETS _opentxs-perl
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT perl)
endif()


if(RUBY)
  find_package(Ruby)
  include_directories(${RUBY_INCLUDE_PATH})
  set_source_files_properties(opentxs.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(opentxs-ruby ruby opentxs.i)
  swig_link_libraries(opentxs-ruby ${RUBY_LIBRARIES})
  target_link_libraries(_opentxs-ruby opentxs-api)
  set_target_properties(_opentxs-ruby
                      PROPERTIES OUTPUT_NAME _opentxs)
  install(TARGETS _opentxs-ruby
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT ruby)
endif()


if(TCL)
  find_package(TCL)
  include_directories(${TCL_INCLUDE_PATH})
  set_source_files_properties(opentxs.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(opentxs-tcl tcl opentxs.i)
  swig_link_libraries(opentxs-tcl ${TCL_LIBRARIES})
  target_link_libraries(_opentxs-tcl opentxs-api)
  set_target_properties(_opentxs-tcl
                      PROPERTIES OUTPUT_NAME _opentxs)
  install(TARGETS _opentxs-tcl
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT tcl)
endif()


if(JAVA)
  find_package(Java)
  include_directories(${JAVA_INCLUDE_PATH})
  set_source_files_properties(opentxs.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(opentxs-java java opentxs.i)
  swig_link_libraries(opentxs-java ${JAVA_LIBRARIES})
  target_link_libraries(_opentxs-java opentxs-api)
  set_target_properties(_opentxs-java
                      PROPERTIES OUTPUT_NAME _opentxs)
  install(TARGETS _opentxs-java
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT java)
endif()


if(PHP)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
  find_package(PHP5)
  include_directories(${PHP5_INCLUDE_DIR} ${PHP5_MAIN_INCLUDE_DIR} ${PHP5_ZEND_INCLUDE_DIR} ${PHP5_TSRM_INCLUDE_DIR})
  set_source_files_properties(opentxs.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(opentxs-php php opentxs.i)
  swig_link_libraries(opentxs-php ${PHP_LIBRARIES})
  target_link_libraries(_opentxs-php opentxs-api)
  set_target_properties(_opentxs-php
                      PROPERTIES OUTPUT_NAME _opentxs)
  install(TARGETS _opentxs-php
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT php)
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
