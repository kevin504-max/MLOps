# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/kevin_lucas/esp/v5.4/esp-idf/components/bootloader/subproject"
  "/home/kevin_lucas/esp/v5.4/esp-idf/tcc/MLOps/build/bootloader"
  "/home/kevin_lucas/esp/v5.4/esp-idf/tcc/MLOps/build/bootloader-prefix"
  "/home/kevin_lucas/esp/v5.4/esp-idf/tcc/MLOps/build/bootloader-prefix/tmp"
  "/home/kevin_lucas/esp/v5.4/esp-idf/tcc/MLOps/build/bootloader-prefix/src/bootloader-stamp"
  "/home/kevin_lucas/esp/v5.4/esp-idf/tcc/MLOps/build/bootloader-prefix/src"
  "/home/kevin_lucas/esp/v5.4/esp-idf/tcc/MLOps/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/kevin_lucas/esp/v5.4/esp-idf/tcc/MLOps/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/kevin_lucas/esp/v5.4/esp-idf/tcc/MLOps/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
