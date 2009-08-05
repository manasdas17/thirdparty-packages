/* Copyright (c) 2007 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/

/** @file    tests/regressions/par_maxval.cpp
    @author  Jules Bergmann
    @date    2006-01-29
    @brief   VSIPL++ Library: Test Maxval of distributed expression.
*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <algorithm>
#include <iostream>

#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/vector.hpp>
#include <vsip/math.hpp>
#include <vsip/parallel.hpp>

#include <vsip_csl/test.hpp>
#include <vsip_csl/test-storage.hpp>

#include "test_common.hpp"

using namespace std;
using namespace vsip;
using vsip_csl::equal;



/***********************************************************************
  Definitions
***********************************************************************/

template <typename MapT>
void
test_maxval()
{
  typedef float T;
  typedef Dense<1, T, row1_type, MapT> block_type;
  typedef Vector<T, block_type > view_type;

  MapT map;

  length_type size = 16;
  view_type   view(size, map);
  Index<1>    idx;
  int         k = 1;
  T           maxv;

  setup(view, 1);

  maxv = maxval(view, idx);

  test_assert(equal(maxv, T((size-1)*k)));
  test_assert(idx[0] == (size-1));


  maxv = maxval(magsq(view), idx);

  test_assert(equal(maxv, sq(T((size-1)*k))));
  test_assert(idx[0] == (size-1));
}



template <typename       T,
	  typename       MapT,
	  dimension_type Dim>
void
test_maxval_nd(Domain<Dim> const& dom)
{
  typedef typename Default_order<Dim>::type order_type;
  Storage<Dim, T, order_type, MapT> stor(dom, T(1));
  Index<Dim> idx;

  T mv = maxval(stor.view, idx);

  test_assert(mv == T(1));
}



/***********************************************************************
  Main
***********************************************************************/

int
main(int argc, char** argv)
{
  vsipl init(argc, argv);

  test_maxval<Local_map>();
  test_maxval<Map<> >();
  test_maxval<Global_map<1> >();



  // A bug in generic_par_idx_op prevented index-reductions such as
  // maxval from working on distributed views with dimension greater
  // than 1.

  test_maxval_nd<float, Local_map>(Domain<1>(5));		// OK
  test_maxval_nd<float, Local_map>(Domain<2>(5, 8));		// OK
  test_maxval_nd<float, Local_map>(Domain<3>(4, 6, 8));		// OK

  test_maxval_nd<float, Map<> >(Domain<1>(5));			// OK
  test_maxval_nd<float, Map<> >(Domain<2>(5, 8));		// error
  test_maxval_nd<float, Map<> >(Domain<3>(4, 6, 8));		// error

  test_maxval_nd<float, Global_map<1> >(Domain<1>(5));		// OK
  test_maxval_nd<float, Global_map<2> >(Domain<2>(5, 8));	// error
  test_maxval_nd<float, Global_map<3> >(Domain<3>(4, 6, 8));	// error

  return 0;
}
