
set(name unittests-opentxs-network-zeromq)

set(cxx-sources
  ${PROJECT_SOURCE_DIR}/tests/main.cpp
  Test_DealerReply.cpp
  Test_DealerRouter.cpp
  Test_DealerSocket.cpp
  Test_Frame.cpp
  Test_FrameIterator.cpp
  Test_FrameSection.cpp
  Test_Message.cpp
  Test_PublishSocket.cpp
  Test_PublishSubscribe.cpp
  Test_ReplyCallback.cpp
  Test_ReplySocket.cpp
  Test_RequestReply.cpp
  Test_RequestRouter.cpp
  Test_RequestSocket.cpp
  Test_RouterDealer.cpp
  Test_RouterSocket.cpp
  Test_SubscribeSocket.cpp
  ${PROJECT_SOURCE_DIR}/tests/OTTestEnvironment.cpp
)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/tests
  ${GTEST_INCLUDE_DIRS}
)

add_executable(${name} ${cxx-sources})
target_link_libraries(${name} opentxs ${GTEST_BOTH_LIBRARIES} ${ZMQ_LIBRARIES})
set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)
add_test(${name} ${PROJECT_BINARY_DIR}/tests/${name} --gtest_output=xml:gtestresults.xml)
