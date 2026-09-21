# CMake generated Testfile for 
# Source directory: C:/EndlessProjects/BootstrapForge/Tests
# Build directory: C:/EndlessProjects/BootstrapForge/out/build/vs2022-debug/Tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[BootstrapForge.ArchitectureDiffer]=] "C:/EndlessProjects/BootstrapForge/out/build/vs2022-debug/bin/Debug/BootstrapForge.Tests.exe")
  set_tests_properties([=[BootstrapForge.ArchitectureDiffer]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/EndlessProjects/BootstrapForge/Tests/CMakeLists.txt;82;add_test;C:/EndlessProjects/BootstrapForge/Tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[BootstrapForge.ArchitectureDiffer]=] "C:/EndlessProjects/BootstrapForge/out/build/vs2022-debug/bin/Release/BootstrapForge.Tests.exe")
  set_tests_properties([=[BootstrapForge.ArchitectureDiffer]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/EndlessProjects/BootstrapForge/Tests/CMakeLists.txt;82;add_test;C:/EndlessProjects/BootstrapForge/Tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[BootstrapForge.ArchitectureDiffer]=] "C:/EndlessProjects/BootstrapForge/out/build/vs2022-debug/bin/MinSizeRel/BootstrapForge.Tests.exe")
  set_tests_properties([=[BootstrapForge.ArchitectureDiffer]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/EndlessProjects/BootstrapForge/Tests/CMakeLists.txt;82;add_test;C:/EndlessProjects/BootstrapForge/Tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[BootstrapForge.ArchitectureDiffer]=] "C:/EndlessProjects/BootstrapForge/out/build/vs2022-debug/bin/RelWithDebInfo/BootstrapForge.Tests.exe")
  set_tests_properties([=[BootstrapForge.ArchitectureDiffer]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/EndlessProjects/BootstrapForge/Tests/CMakeLists.txt;82;add_test;C:/EndlessProjects/BootstrapForge/Tests/CMakeLists.txt;0;")
else()
  add_test([=[BootstrapForge.ArchitectureDiffer]=] NOT_AVAILABLE)
endif()
