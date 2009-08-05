/* Copyright (c) 2005, 2006, 2007 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    tests/view_functions_binary.hpp
    @author  Stefan Seefeld
    @date    2005-03-16
    @brief   VSIPL++ Library: Unit tests for binary View expressions.

    This file contains unit tests for View expressions.
*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <cassert>
#include <complex>
#include <iostream>
#include <vsip/initfin.hpp>
#include <vsip/vector.hpp>
#include <vsip/matrix.hpp>
#include <vsip/dense.hpp>
#include <vsip/math.hpp>
#include <vsip_csl/test.hpp>

#include "view_functions.hpp"

using namespace std;
using namespace vsip;
using namespace vsip_csl;
using namespace impl;



int
main(int argc, char** argv)
{
  vsip::vsipl init(argc, argv);

  TEST_UNARY(acos, scalar_f, 0.5f)
  TEST_UNARY_RETN(arg, cscalar_f, scalar_f, cscalar_f(4.f, 2.f))
  TEST_UNARY(asin, scalar_f, 0.5f)
  TEST_UNARY(atan, scalar_f, 0.5f)
  TEST_UNARY(bnot, int, 4)
  TEST_UNARY(ceil, scalar_f, 1.6f)
  TEST_UNARY(conj, cscalar_f, 4.f)
  TEST_UNARY(cos, scalar_f, 4.f)
  TEST_UNARY(cosh, scalar_f, 4.f)
  TEST_UNARY_RETN(euler, scalar_f, cscalar_f, 4.f)
  TEST_UNARY(exp, scalar_f, 4.f)
  TEST_UNARY(exp10, scalar_f, 4.f)
  TEST_UNARY(floor, scalar_f, 4.f)
  TEST_UNARY_RETN(imag, cscalar_f, scalar_f, cscalar_f(4.f, 2.f))
  TEST_UNARY_RETN(lnot, int, bool, 4)
  TEST_UNARY_RETN(lnot, int, bool, 0)
  TEST_UNARY(log, scalar_f, 4.f)
  TEST_UNARY(log10, scalar_f, 4.f)
  TEST_UNARY(mag, scalar_f, -2.f)
  TEST_UNARY(magsq, scalar_f, -2.f)
  TEST_UNARY(neg, scalar_f, 4.f)
  TEST_UNARY_RETN(real, cscalar_f, scalar_f, cscalar_f(4.f, 2.f))
  TEST_UNARY(recip, scalar_f, 2.f)
  TEST_UNARY(rsqrt, scalar_f, 2.f)
  TEST_UNARY(sin, scalar_f, 2.f)
  TEST_UNARY(sinh, scalar_f, 2.f)
  TEST_UNARY(sq, scalar_f, 2.f)
  TEST_UNARY(sqrt, scalar_f, 2.f)
  TEST_UNARY(tan, scalar_f, 2.f)
  TEST_UNARY(tanh, scalar_f, 2.f)
}
