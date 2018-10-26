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
project(SpdlogBench CXX)

if(NOT TARGET spdlog)
  # Stand-alone build
  find_package(spdlog CONFIG REQUIRED)
endif()

find_package(Threads REQUIRED)

add_executable(bench bench.cpp)
target_link_libraries(bench spdlog::spdlog Threads::Threads)

add_executable(async_bench async_bench.cpp)
target_link_libraries(async_bench spdlog::spdlog Threads::Threads)

add_executable(latency latency.cpp)
set(CMAKE_CXX_STANDARD_LIBRARIES -lbenchmark)
target_link_libraries(latency spdlog::spdlog Threads::Threads)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")
