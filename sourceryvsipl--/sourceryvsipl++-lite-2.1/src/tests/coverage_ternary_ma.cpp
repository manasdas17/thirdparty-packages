/* Copyright (c) 2005, 2006, 2007 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    tests/coverage_ternary_ma.hpp
    @author  Jules Bergmann
    @date    2005-09-13
    @brief   VSIPL++ Library: Coverage tests for ma ternary expressions.
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
  Ternary Operator Tests
***********************************************************************/

TEST_TERNARY(ma, ma, *, +, *, +)

/// Test ternary 'cma' (and nested conj(*)/+) expressions

struct Test_cma
{
  template <typename View1,
	    typename View2,
	    typename View3,
	    typename View4>
  static void
  exec(
    View1 view1,
    View2 view2,
    View3 view3,
    View4 view4)	// Result
  {
    length_type size = get_size(view4);
    test_assert(Is_scalar<View1>::value || get_size(view1) == size);
    test_assert(Is_scalar<View2>::value || get_size(view2) == size);
    test_assert(Is_scalar<View3>::value || get_size(view3) == size);

    typedef typename Value_type_of<View1>::type T1;
    typedef typename Value_type_of<View2>::type T2;
    typedef typename Value_type_of<View3>::type T3;
    typedef typename Value_type_of<View4>::type T4;

    for (index_type i=0; i<get_size(view1); ++i)
      put_nth(view1, i, T1(2*i + 1));
    for (index_type i=0; i<get_size(view2); ++i)
      put_nth(view2, i, T2(3*i + 2));
    for (index_type i=0; i<get_size(view3); ++i)
      put_nth(view3, i, T3(5*i + 3));
  
    view4 = ma(conj(view1), view2, view3);

    for (index_type i=0; i<get_size(view4); ++i)
    {
      T4 expected = conj(T1(Is_scalar<View1>::value ? (2*0+1) : (2*i+1)))
	          *      T2(Is_scalar<View2>::value ? (3*0+2) : (3*i+2))
	          +      T3(Is_scalar<View3>::value ? (5*0+3) : (5*i+3));
      test_assert(equal(get_nth(view4, i), T4(expected)));
    }
    
    view4 = T4();
    view4 = conj(view1) * view2 + view3;

    for (index_type i=0; i<get_size(view4); ++i)
    {
      T4 expected = conj(T1(Is_scalar<View1>::value ? (2*0+1) : (2*i+1)))
	          *      T2(Is_scalar<View2>::value ? (3*0+2) : (3*i+2))
	          +      T3(Is_scalar<View3>::value ? (5*0+3) : (5*i+3));
      test_assert(equal(get_nth(view4, i), T4(expected)));
    }
  }
};



/***********************************************************************
  Main
***********************************************************************/

int
main(int argc, char** argv)
{
  vsipl init(argc, argv);

  test_ternary<Test_ma>();

  // special coverage.
  vector_cases4<Test_cma, complex<float>,  complex<float>, complex<float> >();
}
