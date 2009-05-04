/*
 *
 * tests/test-reducePath.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/utils/FileUtils.H>

#include <iostream>

using namespace nucleo ;

void
test(const char *p) {
  std::cerr << p << std::endl ;
  std::cerr << reducePath(p) << std::endl << std::endl ;
}

int
main(int argc, char **argv) {
  test("/home/roussel/CodeBase/quadro") ; 
  test("/home/roussel/./CodeBase/quadro") ; 
  test("/home/roussel/CodeBuild/../CodeBase/quadro") ; 
  test("../etc/passwd") ; 
  test("/../etc/passwd") ; 
  test("/home/../etc/passwd") ; 
  test("/home/roussel/../../etc/passwd") ; 
  test("/home/roussel/../../../etc/passwd") ; 
  test("/home/roussel/..") ; 
  test("/home/roussel/../..") ; 
  test("/home/roussel/../../..") ; 
  test("/home/roussel/../../../..") ; 
}
