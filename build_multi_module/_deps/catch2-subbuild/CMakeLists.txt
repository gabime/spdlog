# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 4.2.1)

# Reject any attempt to use a toolchain file. We must not use one because
# we could be downloading it here. If the CMAKE_TOOLCHAIN_FILE environment
# variable is set, the cache variable will have been initialized from it.
unset(CMAKE_TOOLCHAIN_FILE CACHE)
unset(ENV{CMAKE_TOOLCHAIN_FILE})

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(catch2-populate NONE)


# Pass through things we've already detected in the main project to avoid
# paying the cost of redetecting them again in ExternalProject_Add()
set(GIT_EXECUTABLE [==[D:/Program Files/Git/cmd/git.exe]==])
set(Git_VERSION [==[2.53.0.windows.1]==])
set_property(GLOBAL PROPERTY _CMAKE_FindGit_GIT_EXECUTABLE_VERSION
  [==[D:/Program Files/Git/cmd/git.exe;2.53.0.windows.1]==]
)


include(ExternalProject)
ExternalProject_Add(catch2-populate
                     "UPDATE_DISCONNECTED" "False" "GIT_REPOSITORY" "https://github.com/catchorg/Catch2.git" "EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR" "GIT_TAG" "53d0d913a422d356b23dd927547febdf69ee908"
                    SOURCE_DIR          "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/_deps/catch2-src"
                    BINARY_DIR          "D:/workspace/兼职项目/4.Solo Coder/spdlog/build_multi_module/_deps/catch2-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


