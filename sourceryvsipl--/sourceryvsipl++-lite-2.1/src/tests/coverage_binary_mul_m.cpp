/* Copyright (c) 2005, 2006, 2007 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    tests/coverage_binary_mul_m.hpp
    @author  Jules Bergmann
    @date    2005-09-13
    @brief   VSIPL++ Library: Coverage tests for binary expressions.
                              multiply/matrix
*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <iostream>

#include <vsip/support.hpp>
#include <vsip/initfin.hpp>
#include <vsip/vector.hpp>
#include <vsip/math.hpp>
#include <vsip/random.hpp>

#include <vsip_csl/test.hpp>
#include <vsip_csl/test-storage.hpp>
#include "coverage_common.hpp"

using namespace std;
using namespace vsip;
using namespace vsip_csl;


/***********************************************************************
  Definitions
***********************************************************************/

TEST_BINARY_OP(mul,  *,  *,  anyval)



/***********************************************************************
  Main
***********************************************************************/

int
main(int argc, char** argv)
{
  vsipl init(argc, argv);

  matrix_cases3<Test_mul, float, float>();
  matrix_cases3<Test_mul, complex<float>, complex<float> >();

  return EXIT_SUCCESS;
}
