# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 3.5)

### Build lucre as library
set(sources
      irrXML.cpp
   )

set(MODULE_NAME irrxml)

add_library(${MODULE_NAME}
	    OBJECT
	    ${sources}
)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
