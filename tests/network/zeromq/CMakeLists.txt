# Copyright (c) Monetas AG, 2014

set(name unittests-opentxs-network-zeromq)

set(cxx-sources
  Test_Message.cpp
  Test_MultipartMessage.cpp
  Test_FrameIterator.cpp
  Test_ReplySocket.cpp
  Test_RequestSocket.cpp
  Test_RequestReply.cpp
  Test_PublishSocket.cpp
  Test_SubscribeSocket.cpp
  Test_PublishSubscribe.cpp
)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${GTEST_INCLUDE_DIRS}
)

add_executable(${name} ${cxx-sources})
target_link_libraries(${name} opentxs ${GTEST_BOTH_LIBRARIES} ${ZMQ_LIBRARIES})
set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)
add_test(${name} ${PROJECT_BINARY_DIR}/tests/${name} --gtest_output=xml:gtestresults.xml)
