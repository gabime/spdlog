# *************************************************************************/
# * Copyright (c) 2015 Ruslan Baratov.                                    */
# *                                                                       */
# * Permission is hereby granted, free of charge, to any person obtaining */
# * a copy of this software and associated documentation files (the       */
# * "Software"), to deal in the Software without restriction, including   */
# * without limitation the rights to use, copy, modify, merge, publish,   */
# * distribute, sublicense, and/or sell copies of the Software, and to    */
# * permit persons to whom the Software is furnished to do so, subject to */
# * the following conditions:                                             */
# *                                                                       */
# * The above copyright notice and this permission notice shall be        */
# * included in all copies or substantial portions of the Software.       */
# *                                                                       */
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
# * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
# * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
# * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
# * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
# * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
# * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
# *************************************************************************/

cmake_minimum_required(VERSION 3.1)
project(SpdlogExamples CXX)

if(TARGET spdlog)
    # If we're running this example as part of the primary spdlog applciation
    # then add an alias. This allows us to use the same "spdlog::spdlog"
    # below that a user would use (with the namespace)
    add_library(spdlog::spdlog ALIAS spdlog)
    add_library(spdlog::spdlog_header_only ALIAS spdlog_header_only)
else()
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

#---------------------------------------------------------------------------------------
# Example of using pre-compiled library
#---------------------------------------------------------------------------------------
add_executable(example example.cpp)
target_link_libraries(example spdlog::spdlog)

#---------------------------------------------------------------------------------------
# Example of using header-only library
#---------------------------------------------------------------------------------------
add_executable(example_header_only example.cpp)
target_link_libraries(example_header_only spdlog::spdlog_header_only)

# Create logs directory
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")
