/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    corr-2d.cpp
    @author  Jules Bergmann
    @date    2005-12-09
    @brief   VSIPL++ Library: Unit tests for [signal.correl] 2-D items.
*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/vector.hpp>
#include <vsip/signal.hpp>
#include <vsip/random.hpp>
#include <vsip/selgen.hpp>
#include <vsip/initfin.hpp>

#include <vsip_csl/test.hpp>
#include <vsip_csl/ref_corr.hpp>
#include <vsip_csl/error_db.hpp>

#define VERBOSE 0

#if VERBOSE
#  include <iostream>
#  include <vsip_csl/output.hpp>
#endif

using namespace std;
using namespace vsip;
using namespace vsip_csl;



/***********************************************************************
  Definitions
***********************************************************************/

/// Test general 1-D correlation.

template <typename            T,
	  support_region_type support>
void
test_corr(
  bias_type                bias,
  Domain<2> const&         M,		// reference size
  Domain<2> const&         N,		// input size
  length_type const        n_loop = 3)
{
  typedef typename vsip::impl::Scalar_of<T>::type scalar_type;
  typedef Correlation<const_Matrix, support, T> corr_type;

  length_type Mr = M[0].size();
  length_type Mc = M[1].size();
  length_type Nr = N[0].size();
  length_type Nc = N[1].size();

  length_type const Pr = ref::corr_output_size(support, Mr, Nr);
  length_type const Pc = ref::corr_output_size(support, Mc, Nc);

  corr_type corr(M, N);

  test_assert(corr.support()  == support);

  test_assert(corr.reference_size()[0].size() == Mr);
  test_assert(corr.reference_size()[1].size() == Mc);

  test_assert(corr.input_size()[0].size()     == Nr);
  test_assert(corr.input_size()[1].size()     == Nc);

  test_assert(corr.output_size()[0].size()    == Pr);
  test_assert(corr.output_size()[1].size()    == Pc);

  Rand<T> rand(0);

  Matrix<T> ref(Mr, Mc);
  Matrix<T> in(Nr, Nc);
  Matrix<T> out(Pr, Pc, T(100));
  Matrix<T> chk(Pr, Pc, T(101));

  for (index_type loop=0; loop<n_loop; ++loop)
  {
    if (loop == 0)
    {
      ref = T(1);
      for (index_type r=0; r<Nr; ++r)
	in.row(r) = ramp(T(0), T(1), Nc);
    }
    else if (loop == 1)
    {
      ref = rand.randu(Mr, Mc);
      for (index_type r=0; r<Nr; ++r)
	in.row(r) = ramp(T(0), T(1), Nc);
    }
    else
    {
      ref = rand.randu(Mr, Mc);
      in  = rand.randu(Nr, Nc);
    }

    corr(bias, ref, in, out);

    ref::corr(bias, support, ref, in, chk);

    double error = error_db(out, chk);

#if VERBOSE
    if (error > -100)
    {
      cout << "error = " << error
	   << "  (" << Pr << ", " << Pc << ")" << endl;
      for (index_type r=0; r<Pr; ++r)
      for (index_type c=0; c<Pc; ++c)
      {
	cout << r << ", " << c << ":  out = " << out.get(r, c)
	     << "  chk = " << chk.get(r, c)
	     << endl;
      }
      cout << "error = " << error << endl;
    }
#endif

    test_assert(error < -100);
  }
}



template <typename T>
void
corr_cases(Domain<2> const& M, Domain<2> const& N)
{
  test_corr<T, support_min>(biased,   M, N);
  test_corr<T, support_min>(unbiased, M, N);

  test_corr<T, support_same>(biased,   M, N);
  test_corr<T, support_same>(unbiased, M, N);

  test_corr<T, support_full>(biased,   M, N);
  test_corr<T, support_full>(unbiased, M, N);
}


template <typename T>
void
corr_cover()
{
  corr_cases<T>(Domain<2>(8, 8), Domain<2>(8, 8));

  corr_cases<T>(Domain<2>(1, 1), Domain<2>(32, 32));
  corr_cases<T>(Domain<2>(2, 4), Domain<2>(32, 32));
  corr_cases<T>(Domain<2>(2, 3), Domain<2>(32, 32));
  corr_cases<T>(Domain<2>(3, 2), Domain<2>(32, 32));

  corr_cases<T>(Domain<2>(1, 1), Domain<2>(16, 13));
  corr_cases<T>(Domain<2>(2, 4), Domain<2>(16, 13));
  corr_cases<T>(Domain<2>(2, 3), Domain<2>(16, 13));
  corr_cases<T>(Domain<2>(3, 2), Domain<2>(16, 13));

  corr_cases<T>(Domain<2>(1, 1), Domain<2>(13, 16));
  corr_cases<T>(Domain<2>(2, 4), Domain<2>(13, 16));
  corr_cases<T>(Domain<2>(2, 3), Domain<2>(13, 16));
  corr_cases<T>(Domain<2>(3, 2), Domain<2>(13, 16));
}



int
main(int argc, char** argv)
{
  vsipl init(argc, argv);

#if VSIP_IMPL_TEST_LEVEL == 0
  corr_cover<float>();
#else
  // Test user-visible correlation
  corr_cover<float>();
  corr_cover<complex<float> >();
#  if VSIP_IMPL_TEST_DOUBLE
  corr_cover<double>();
  corr_cover<complex<double> >();
#  endif // VSIP_IMPL_TEST_DOUBLE
#endif // VSIP_IMPL_TEST_LEVEL >= 1
}
