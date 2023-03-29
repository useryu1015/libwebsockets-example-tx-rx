# CMake generated Testfile for 
# Source directory: /home/project_xlian/WDD/websocket/libwebsockets-x86/minimal-examples/client/hello_world
# Build directory: /home/project_xlian/WDD/websocket/libwebsockets-x86/build/minimal-examples/client/hello_world
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
ADD_TEST(mss-lws-minimal-ss-hello_world "/usr/bin/valgrind" "--tool=memcheck" "--leak-check=yes" "--num-callers=20" "/home/project_xlian/WDD/websocket/libwebsockets-x86/build/bin/lws-minimal-ss-hello_world")
SET_TESTS_PROPERTIES(mss-lws-minimal-ss-hello_world PROPERTIES  TIMEOUT "40" WORKING_DIRECTORY "/home/project_xlian/WDD/websocket/libwebsockets-x86/minimal-examples/client/hello_world")
