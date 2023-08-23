# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/cam/Desktop/CMOD/build/memflow/src/memflow"
  "/home/cam/Desktop/CMOD/build/memflow/src/memflow"
  "/home/cam/Desktop/CMOD/build/memflow"
  "/home/cam/Desktop/CMOD/build/memflow/tmp"
  "/home/cam/Desktop/CMOD/build/memflow/src/memflow-stamp"
  "/home/cam/Desktop/CMOD/build/memflow/src"
  "/home/cam/Desktop/CMOD/build/memflow/src/memflow-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/cam/Desktop/CMOD/build/memflow/src/memflow-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/cam/Desktop/CMOD/build/memflow/src/memflow-stamp${cfgdir}") # cfgdir has leading slash
endif()
