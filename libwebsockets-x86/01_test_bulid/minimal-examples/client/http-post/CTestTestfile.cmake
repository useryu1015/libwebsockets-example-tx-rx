# CMake generated Testfile for 
# Source directory: /home/project_xlian/WDD/websocket/libwebsockets-x86/minimal-examples/client/http-post
# Build directory: /home/project_xlian/WDD/websocket/libwebsockets-x86/build/minimal-examples/client/http-post
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
ADD_TEST(msshttp-post-warmcat "/usr/bin/valgrind" "--tool=memcheck" "--leak-check=yes" "--num-callers=20" "/home/project_xlian/WDD/websocket/libwebsockets-x86/build/bin/lws-minimal-ss-http-post")
SET_TESTS_PROPERTIES(msshttp-post-warmcat PROPERTIES  TIMEOUT "40" WORKING_DIRECTORY "/home/project_xlian/WDD/websocket/libwebsockets-x86/minimal-examples/client/http-post")
