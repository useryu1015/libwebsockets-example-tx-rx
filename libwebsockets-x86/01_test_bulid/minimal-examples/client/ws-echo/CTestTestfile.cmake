# CMake generated Testfile for 
# Source directory: /home/project_xlian/WDD/websocket/libwebsockets-x86/minimal-examples/client/ws-echo
# Build directory: /home/project_xlian/WDD/websocket/libwebsockets-x86/build/minimal-examples/client/ws-echo
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
ADD_TEST(mssws_echo-warmcat "/usr/bin/valgrind" "--tool=memcheck" "--leak-check=yes" "--num-callers=20" "/home/project_xlian/WDD/websocket/libwebsockets-x86/build/bin/lws-minimal-ss-ws-echo")
SET_TESTS_PROPERTIES(mssws_echo-warmcat PROPERTIES  TIMEOUT "40" WORKING_DIRECTORY "/home/project_xlian/WDD/websocket/libwebsockets-x86/minimal-examples/client/ws-echo")
