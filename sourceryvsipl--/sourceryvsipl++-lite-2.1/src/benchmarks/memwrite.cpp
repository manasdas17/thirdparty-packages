/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    benchmarks/memwrite.cpp
    @author  Jules Bergmann
    @date    2006-10-12
    @brief   VSIPL++ Library: Benchmark for memory write bandwidth.

*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/math.hpp>
#include <vsip/random.hpp>
#include <vsip/opt/profile.hpp>
#include <vsip/opt/diag/eval.hpp>

#include <vsip_csl/test.hpp>
#include "loop.hpp"

using namespace vsip;
using namespace vsip_csl;



/***********************************************************************
  VSIPL++ memwrite
***********************************************************************/

template <typename T>
struct t_memwrite1 : Benchmark_base
{
  char const* what() { return "t_memwrite1"; }
  int ops_per_point(length_type)  { return 1; }
  int riob_per_point(length_type) { return 0; }
  int wiob_per_point(length_type) { return 1*sizeof(T); }
  int mem_per_point(length_type)  { return 1*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
  {
    Vector<T>   view(size, T());
    T           val = T(1);
    
    vsip::impl::profile::Timer t1;
    
    marker1_start();
    t1.start();
    for (index_type l=0; l<loop; ++l)
      view = val;
    t1.stop();
    marker1_stop();

    for(index_type i=0; i<size; ++i)
      test_assert(equal(view.get(i), val));
    
    time = t1.delta();
  }

  void diag()
  {
    length_type const size = 256;

    Vector<T>   view(size, T());
    T           val = T(1);

    vsip::impl::diagnose_eval_list_std(view, val);
  }
};



// explicit loop

template <typename T>
struct t_memwrite_expl : Benchmark_base
{
  char const* what() { return "t_memwrite_expl"; }
  int ops_per_point(length_type)  { return 1; }
  int riob_per_point(length_type) { return 0; }
  int wiob_per_point(length_type) { return 1*sizeof(T); }
  int mem_per_point(length_type)  { return 1*sizeof(T); }

  void operator()(length_type size, length_type loop, float& time)
  {
    Vector<T>   view(size, T());
    T           val = T(1);
    
    vsip::impl::profile::Timer t1;
    
    t1.start();
    for (index_type l=0; l<loop; ++l)
      for (index_type i=0; i<size; ++i)
	view.put(i, val);
    t1.stop();

    for(index_type i=0; i<size; ++i)
      test_assert(equal(view.get(i), val));
    
    time = t1.delta();
  }
};






void
defaults(Loop1P&)
{
}



int
test(Loop1P& loop, int what)
{
  switch (what)
  {
  case   1: loop(t_memwrite1<float>()); break;
  case   2: loop(t_memwrite_expl<float>()); break;

  default: return 0;
  }
  return 1;
}
