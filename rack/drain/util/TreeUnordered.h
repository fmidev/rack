#ifndef DRAIN_TREE_UNORDERED_H
#define DRAIN_TREE_UNORDERED_H "2.0"  // use: std::string version(DRAIN_TREE_UNORDERED);

#define DRAIN_TREE_CONTAINER(k,v) std::list<std::pair<k,v> >

#define DRAIN_TREE_NAME UnorderedTree
#include "Tree.h"
#undef  DRAIN_TREE_NAME

#define DRAIN_TREE_NAME UnorderedMultiTree
#define DRAIN_TREE_MULTI
#include "Tree.h"
#undef  DRAIN_TREE_NAME
#undef  DRAIN_TREE_MULTI

#undef  DRAIN_TREE_CONTAINER

#endif
