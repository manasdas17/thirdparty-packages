/* Copyright (c) 2006 by CodeSourcery.  All rights reserved. */

/** @file    vsip/opt/simd/vma_ip_csc.cpp
    @author  Jules Bergmann
    @date    2006-11-17
    @brief   VSIPL++ Library: SIMD element-wise vector multiplication.

*/

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/opt/simd/vma_ip_csc.hpp>



/***********************************************************************
  Definitions
***********************************************************************/

namespace vsip
{
namespace impl
{
namespace simd
{

#if !VSIP_IMPL_INLINE_LIBSIMD

template <typename T>
void
vma_ip_cSC(
  std::complex<T> const& a,
  T const*               B,
  std::complex<T>*       R,
  int                    n)
{
  static bool const Is_vectorized =
    Is_algorithm_supported<std::complex<T>, false, Alg_vma_ip_cSC>::value;
  Simd_vma_ip_cSC<std::complex<T>, Is_vectorized>::exec(a, B, R, n);
}

template void vma_ip_cSC(std::complex<float> const&, float const*,
			 std::complex<float>*, int);
template void vma_ip_cSC(std::complex<double> const&, double const*,
			 std::complex<double>*, int);

#endif

} // namespace vsip::impl::simd
} // namespace vsip::impl
} // namespace vsip
