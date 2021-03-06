#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
set(CTEST_PROJECT_NAME "MIDAScpp")
set(CTEST_NIGHTLY_START_TIME "21:00:00 EDT")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "www.cdash.org")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=MIDAS")
set(CTEST_DROP_SITE_CDASH TRUE)
set(CTEST_CDASH_VERSION "1.6")
set(CTEST_CDASH_QUERY_VERSION TRUE)

# use old trigger stuff so that cmake 2.4 and below will not 
# get errors on trigger
SET (TRIGGER_SITE 
  "http://public.kitware.com/cgi-bin/Submit-CMake-TestingResults.cgi")
