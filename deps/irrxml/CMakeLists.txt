# Copyright (c) Monetas AG, 2014

cmake_minimum_required(VERSION 2.8)

### Build lucre as library
set(sources
      irrXML.cpp
   )

add_library(irrxml
	    STATIC
	    ${sources}
)
