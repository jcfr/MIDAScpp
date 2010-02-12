#ifndef MIDASSTANDARDINCLUDES_H
#define MIDASSTANDARDINCLUDES_H

#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <map>
#include <list>
#include <set>
#include <deque>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <kwsys/SystemTools.hxx>

// For resource_uuid.resource_type_id
#define MIDAS_RESOURCE_BITSTREAM  0
#define MIDAS_RESOURCE_BUNDLE     1
#define MIDAS_RESOURCE_ITEM       2
#define MIDAS_RESOURCE_COLLECTION 3
#define MIDAS_RESOURCE_COMMUNITY  4
#define MIDAS_RESOURCE_PROJECT    5
#define MIDAS_RESOURCE_ERROR     -1
#define MIDAS_RESOURCE_NONE      -2

// For marking local resources dirty
#define MIDAS_DIRTY_MODIFIED 1
#define MIDAS_DIRTY_ADDED    2
#define MIDAS_DIRTY_REMOVED  3

#endif
