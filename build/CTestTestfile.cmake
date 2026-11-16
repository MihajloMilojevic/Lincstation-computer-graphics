# CMake generated Testfile for 
# Source directory: /home/mihajlo/Mihajlo/Projekti/Grafika-2D/Lincstation
# Build directory: /home/mihajlo/Mihajlo/Projekti/Grafika-2D/Lincstation/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(machine "/home/mihajlo/Mihajlo/Projekti/Grafika-2D/Lincstation/build/machine_test")
set_tests_properties(machine PROPERTIES  _BACKTRACE_TRIPLES "/home/mihajlo/Mihajlo/Projekti/Grafika-2D/Lincstation/CMakeLists.txt;59;add_test;/home/mihajlo/Mihajlo/Projekti/Grafika-2D/Lincstation/CMakeLists.txt;0;")
subdirs("_deps/glfw-build")
subdirs("_deps/glm-build")
subdirs("_deps/freetype-build")
