/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    tests/selgen-ramp.cpp
    @author  Jules Bergmann
    @date    2005-08-15
    @brief   VSIPL++ Library: Unit tests for ramp.
*/

/***********************************************************************
  Included Files
***********************************************************************/


#include <iostream>
#include <cassert>
#include <vsip/support.hpp>
#include <vsip/initfin.hpp>
#include <vsip/vector.hpp>
#include <vsip/selgen.hpp>

#include <vsip_csl/test.hpp>

using namespace std;
using namespace vsip;
using namespace vsip_csl;


/***********************************************************************
  Definitions
***********************************************************************/

template <typename T>
void
test_ramp(T a, T b, length_type len)
{
  Vector<T> vec = ramp(a, b, len);

  for (index_type i=0; i<len; ++i)
    test_assert(equal(a + T(i)*b,
		 vec.get(i)));
}



void
ramp_cases()
{
  test_ramp<int>(0,  1, 10);
  test_ramp<int>(5, -2, 10);

  test_ramp<unsigned int>(0, 1, 10);
  test_ramp<unsigned int>(5, 2, 10);

  test_ramp<float>(0.f,   1.f, 10);
  test_ramp<float>(1.5f, -1.f, 10);

  test_ramp<complex<float> >(complex<float>(), complex<float>(1.f, 0.f), 10);
  test_ramp<complex<float> >(complex<float>(), complex<float>(-1.f, 0.5f), 10);
}



int
main(int argc, char** argv)
{
  vsipl init(argc, argv);

  ramp_cases();
}
