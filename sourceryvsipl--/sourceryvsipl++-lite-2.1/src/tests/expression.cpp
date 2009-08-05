/* Copyright (c) 2005, 2006, 2007, 2008 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    tests/expression.cpp
    @author  Stefan Seefeld
    @date    2005-01-31
    @brief   VSIPL++ Library: Unit tests for expression block API.

    This file has unit tests for the block expression API.
*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/initfin.hpp>
#include <vsip/math.hpp>
#include <vsip/dense.hpp>
#include <vsip/map.hpp>
#include <vsip_csl/test.hpp>
#include "block_interface.hpp"

using namespace std;
using namespace vsip;
using namespace vsip_csl;
using namespace impl;

/***********************************************************************
  Definitions
***********************************************************************/

// initialize elements according to block(i) = a * i + b
template <typename Block>
void ramp(Block& block, index_type a = 1, index_type b = 1)
{
  for (index_type i = 0; i != block.size(); ++i)
    block.put(i, static_cast<typename Block::value_type>(a * i + b));
}

template <template <typename> class Operation,
	  typename Block>
void
test_unary_expr_1d()
{
  length_type const size = 3;

  Block o(size);

  ramp(o);

  Unary_expr_block<1, Operation, Block, typename Block::value_type> expr(o);

  for (index_type i = 0; i != size; ++i)
    test_assert(equal(expr.get(i),
		 Operation<typename Block::value_type>::apply(o.get(i))));
}

template <template <typename, typename> class Operation,
	  typename LBlock,
	  typename RBlock>
void
test_binary_expr_1d()
{
  typedef typename Promotion<typename LBlock::value_type,
                             typename RBlock::value_type>::type value_type;

  length_type const size = 3;

  LBlock d1(size);
  RBlock d2(size);

  ramp(d1);
  ramp(d2, 2);

  Binary_expr_block<1, Operation,
		  LBlock, value_type,
		  RBlock, value_type> expr(d1, d2);

  for (index_type i = 0; i != size; ++i)
    test_assert(equal(expr.get(i),
		 Operation<typename LBlock::value_type,
		           typename RBlock::value_type>::apply(d1.get(i),
							       d2.get(i))));
}

template<typename Expression>
void
evaluate(Dense<1>& result,
	 Expression const& expr,
	 length_type length)
{
  for (index_type i = 0; i < length; ++i)
    result.put(i, expr.get(i));
}

void
test_1d()
{
  Dense<1> d1(Domain<1>(3));
  d1.put(0, 0);
  d1.put(1, 1);
  d1.put(2, 2);
  Dense<1> d2(Domain<1>(3));
  d2.put(0, 3);
  d2.put(1, 4);
  d2.put(2, 5);
  Dense<1> d3(Domain<1>(3));

  // Unary expression block tests

  Unary_expr_block<1, op::Minus, Dense<1>, Dense<1>::value_type> unary(d1);

  block_1d_interface_test(unary);

  test_unary_expr_1d<op::Plus, Dense<1> >();
  test_unary_expr_1d<op::Minus, Dense<1> >();

  // Binary expression block tests

  Binary_expr_block<1, op::Add, 
                  Dense<1>, Dense<1>::value_type,
                  Dense<1>, Dense<1>::value_type> binary(d1, d2);

  block_1d_interface_test(binary);

  test_binary_expr_1d<op::Add, Dense<1>, Dense<1> >();
  test_binary_expr_1d<op::Sub, Dense<1>, Dense<1> >();
  test_binary_expr_1d<op::Mult, Dense<1>, Dense<1> >();
  test_binary_expr_1d<op::Div, Dense<1>, Dense<1> >();

  // Scalar block tests

  Scalar_block<1, float> scalar(5.);

  block_1d_interface_test(scalar);

  test_assert(scalar.size() == 0);
  test_assert(scalar.get(0) == 5.);
  test_assert(scalar.get(1) == 5.);
  test_assert(scalar.get(2) == 5.);
}

int
main(int argc, char** argv)
{
  vsip::vsipl init(argc, argv);

  test_1d();
}
