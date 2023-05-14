#ifndef DRAIN_TREE_ORDERED_H
#define DRAIN_TREE_ORDERED_H  // use: std::string version(DRAIN_TREE_ORDERED);


#include <map>

#define DRAIN_TREE_ORDERED

#define DRAIN_TREE_NAME OrderedTree
#define DRAIN_TREE_CONTAINER(k,v) std::map<k,v>
#include "Tree.h"
#undef  DRAIN_TREE_NAME
#undef  DRAIN_TREE_CONTAINER

#define DRAIN_TREE_NAME OrderedMultiTree
#define DRAIN_TREE_MULTI
#define DRAIN_TREE_CONTAINER(k,v) std::multimap<k,v>
#include "Tree.h"
#undef  DRAIN_TREE_NAME
#undef  DRAIN_TREE_MULTI
#undef  DRAIN_TREE_CONTAINER

#undef  DRAIN_TREE_ORDERED

#endif
