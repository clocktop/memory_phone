# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/clocktop/pico-sdk/tools/elf2uf2"
  "/Users/clocktop/Documents/Memory-Phone/pico-code/memory_phone/build/elf2uf2"
  "/Users/clocktop/Documents/Memory-Phone/pico-code/memory_phone/build/elf2uf2"
  "/Users/clocktop/Documents/Memory-Phone/pico-code/memory_phone/build/elf2uf2/tmp"
  "/Users/clocktop/Documents/Memory-Phone/pico-code/memory_phone/build/elf2uf2/src/ELF2UF2Build-stamp"
  "/Users/clocktop/Documents/Memory-Phone/pico-code/memory_phone/build/elf2uf2/src"
  "/Users/clocktop/Documents/Memory-Phone/pico-code/memory_phone/build/elf2uf2/src/ELF2UF2Build-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/clocktop/Documents/Memory-Phone/pico-code/memory_phone/build/elf2uf2/src/ELF2UF2Build-stamp/${subDir}")
endforeach()
