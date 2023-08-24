# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(MimicWindowsServerTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(MimicWindowsServer_test source/MimicWindowsServer_test.cpp)
target_link_libraries(
    MimicWindowsServer_test PRIVATE
    MimicWindowsServer_lib
    Catch2::Catch2WithMain
)
target_compile_features(MimicWindowsServer_test PRIVATE cxx_std_17)

catch_discover_tests(MimicWindowsServer_test)

# ---- End-of-file commands ----

add_folders(Test)
