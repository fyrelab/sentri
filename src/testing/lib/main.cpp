/*
  main.cpp
  Copyright 2016 fyrelab
 */

#define BOOST_TEST_MODULE lib
#define BOOST_TEST_NO_MAIN
#include <boost/test/included/unit_test.hpp>
#include <random>
#include <iostream>

boost::unit_test::test_suite *init_function(int argc, char *argv[]) {
  // create test cases and suites and return a pointer to any enclosing
  // suite, or 0.
  return 0;
}

int main(int argc, char *argv[]) {
  return boost::unit_test::unit_test_main(init_function, argc, argv);
}
