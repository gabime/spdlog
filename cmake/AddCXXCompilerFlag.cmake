# - Adds a compiler flag globally
#
#  add_cxx_compiler_flag(<FLAG> [<VARIANT>])
#
# - Example
#
# include(AddCXXCompilerFlag)
# add_cxx_compiler_flag(-Wall)
# add_cxx_compiler_flag(-no-strict-aliasing RELEASE)
# Requires CMake 2.6+

if(__add_cxx_compiler_flag)
  return()
endif()
set(__add_cxx_compiler_flag INCLUDED)

function(add_cxx_compiler_flag FLAG)
    if(ARGC GREATER 1)
        set(VARIANT ${ARGV1})
        string(TOUPPER "_${VARIANT}" VARIANT)
    else()
        set(VARIANT "")
    endif()
    set(CMAKE_CXX_FLAGS${VARIANT} "${CMAKE_CXX_FLAGS${VARIANT}} ${FLAG}" PARENT_SCOPE)
endfunction()

