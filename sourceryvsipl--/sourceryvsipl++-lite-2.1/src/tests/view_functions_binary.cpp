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

  TEST_BINARY(add, scalar_f, 2.f, 2.f)
  TEST_BINARY(atan2, scalar_f, 0.5f, 0.5f)
  TEST_BINARY(band, int, 2, 4)
  TEST_BINARY(bor, int, 2, 4)
  TEST_BINARY(bxor, int, 2, 4)
  TEST_BINARY(div, scalar_f, 5.f, 2.f)
  TEST_BINARY_RETN(eq, scalar_f, bool, 4.f, 4.f)
  TEST_BINARY_RETN(eq, scalar_f, bool, 4.f, 3.f)
  TEST_BINARY(fmod, scalar_f, 5.f, 2.f)
  TEST_BINARY_RETN(ge, scalar_f, bool, 2.f, 5.f)
  TEST_BINARY_RETN(ge, scalar_f, bool, 5.f, 2.f)
  TEST_BINARY_RETN(gt, scalar_f, bool, 2.f, 5.f)
  TEST_BINARY_RETN(gt, scalar_f, bool, 5.f, 2.f)
  TEST_BINARY(jmul, cscalar_f, cscalar_f(5.f, 2.f), cscalar_f(2.f, 2.f))
  TEST_BINARY_RETN(land, int, bool, 1, 2)
  TEST_BINARY_RETN(land, int, bool, 1, 0)
  TEST_BINARY_RETN(land, int, bool, 0, 2)
  TEST_BINARY_RETN(le, scalar_f, bool, 2.f, 5.f)
  TEST_BINARY_RETN(le, scalar_f, bool, 5.f, 2.f)
  TEST_BINARY_RETN(lt, scalar_f, bool, 2.f, 5.f)
  TEST_BINARY_RETN(lt, scalar_f, bool, 5.f, 2.f)
  TEST_BINARY_RETN(lor, int, bool, 4, 2)
  TEST_BINARY_RETN(lor, int, bool, 0, 2)
  TEST_BINARY_RETN(lor, int, bool, 4, 0)
  TEST_BINARY_RETN(lxor, int, bool, 4, 2)
  TEST_BINARY_RETN(lxor, int, bool, 0, 2)
  TEST_BINARY_RETN(lxor, int, bool, 4, 0)
  TEST_BINARY(max, scalar_f, 4.f, 2.f)
  TEST_BINARY(maxmg, scalar_f, 4.f, 2.f)
  TEST_BINARY(maxmgsq, scalar_f, 4.f, 2.f)
  TEST_BINARY(min, scalar_f, 4.f, 2.f)
  TEST_BINARY(minmg, scalar_f, 4.f, 2.f)
  TEST_BINARY(minmgsq, scalar_f, 4.f, 2.f)
  TEST_BINARY(mul, scalar_f, 4.f, 2.f)
  TEST_BINARY_RETN(ne, scalar_f, bool, 4.f, 4.f)
  TEST_BINARY_RETN(ne, scalar_f, bool, 4.f, 3.f)
  TEST_BINARY(pow, scalar_f, 4.f, 2.f)
  TEST_BINARY(sub, scalar_f, 4.f, 2.f)
}
