#
# Copyright (c) 2014-23, Lawrence Livermore National Security, LLC
# and Kripke project contributors. See the Kripke/COPYRIGHT file for details.
# 
# SPDX-License-Identifier: (BSD-3-Clause)
#

set(RAJA_COMPILER "RAJA_COMPILER_CLANG" CACHE STRING "")

set(CMAKE_C_COMPILER   "/usr/apps/gnu/clang/2017.06.06/llnl/bin/mpiclang" CACHE PATH "")
set(CMAKE_CXX_COMPILER "/usr/apps/gnu/clang/2017.06.06/llnl/bin/mpiclang++" CACHE PATH "")
set(CMAKE_LINKER       "/usr/apps/gnu/clang/2017.06.06/llnl/bin/mpiclang++" CACHE PATH "")

set(CMAKE_CXX_FLAGS "-stdlib=libc++" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -ffast-math" CACHE STRING "")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "")

set(ENABLE_OPENMP On CACHE BOOL "")
set(ENABLE_MPI On CACHE BOOL "")

# For LLNL TCE packages
set(ENABLE_MPI_WRAPPER Off CACHE BOOL "")


