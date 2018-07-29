#!/bin/sh

mkdir -p generated
rm -rf generated/*
protoc --proto_path=. --cpp_out=generated/ *.proto
