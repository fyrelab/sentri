/*
  def.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_TESTING_DEF_H_
#define SRC_TESTING_DEF_H_

#ifdef assert
#undef assert
#endif

#ifdef DEV

#include <iostream>

#define assert(expr) do { \
  if (!(expr)) { \
    std::cerr << "Assert violated in " << __FILE__ << " line " << __LINE__ << ": `" #expr "`" << std::endl; \
  } \
} while (0);

#define nassert(expr, note) do { \
  if (!(expr)) { \
    std::cerr << "Assert violated in " << __FILE__ << " line " << __LINE__ \
              << ": `" #expr "` > " << note << " <" << std::endl; \
  } \
} while (0);

#else

#define assert(expr)
#define nassert(expr, note)

#endif  // DEV


#endif  // SRC_TESTING_DEF_H_
