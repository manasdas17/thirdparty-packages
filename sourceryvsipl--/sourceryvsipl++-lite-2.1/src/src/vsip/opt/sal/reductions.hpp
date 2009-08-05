/* Copyright (c) 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    vsip/opt/sal/reductions.hpp
    @author  Jules Bergmann
    @date    2006-10-04
    @brief   VSIPL++ Library: Wrappers to bridge with Mercury SAL
             reduction functions.
*/

#ifndef VSIP_IMPL_SAL_REDUCTIONS_HPP
#define VSIP_IMPL_SAL_REDUCTIONS_HPP

#if VSIP_IMPL_REF_IMPL
# error "vsip/opt files cannot be used as part of the reference impl."
#endif

/***********************************************************************
  Included Files
***********************************************************************/

#include <sal.h>

#include <vsip/support.hpp>
#include <vsip/core/coverage.hpp>
#include <vsip/opt/sal/bridge_util.hpp>



/***********************************************************************
  Declarations
***********************************************************************/

namespace vsip
{
namespace impl
{
namespace sal
{

#define VSIP_IMPL_SAL_INLINE inline

/***********************************************************************
  Reduction Functions
***********************************************************************/

#define VSIP_IMPL_SAL_RDX_V(FCN, T, SALFCN)				\
VSIP_IMPL_SAL_INLINE void						\
FCN(									\
  Sal_vector<T> const& A,						\
  T&                   value,						\
  length_type          len)						\
{									\
  VSIP_IMPL_COVER_FCN("RDX_V", SALFCN)					\
  SALFCN(A.ptr, A.stride, &value, len, 0);				\
}

VSIP_IMPL_SAL_RDX_V(sumval, float,  svex)
VSIP_IMPL_SAL_RDX_V(sumval, double, svedx)

VSIP_IMPL_SAL_RDX_V(sumsqval, float,  svesqx)
VSIP_IMPL_SAL_RDX_V(sumsqval, double, svesqdx)

VSIP_IMPL_SAL_RDX_V(meanval, float,  meanvx)
VSIP_IMPL_SAL_RDX_V(meanval, double, meanvdx)

VSIP_IMPL_SAL_RDX_V(meanmagsqval, float,  measqvx)
VSIP_IMPL_SAL_RDX_V(meanmagsqval, double, measqvdx)


/***********************************************************************
  Reduction-Index Functions
***********************************************************************/

#define VSIP_IMPL_SAL_V_IDX(FCN, T, SALFCN)				\
VSIP_IMPL_SAL_INLINE void						\
FCN(									\
  Sal_vector<T> const& A,						\
  T&                   value,						\
  int&                 idx,						\
  length_type          len)						\
{									\
  VSIP_IMPL_COVER_FCN("RDX_V_IDX", SALFCN)				\
  SALFCN(A.ptr, A.stride, &value, &idx, len, 0);			\
}

VSIP_IMPL_SAL_V_IDX(maxval, float,  maxvix)
VSIP_IMPL_SAL_V_IDX(maxval, double, maxvidx)

VSIP_IMPL_SAL_V_IDX(minval, float,  minvix)
VSIP_IMPL_SAL_V_IDX(minval, double, minvidx)

VSIP_IMPL_SAL_V_IDX(maxmgval, float,  maxmgvix)
// not in SAL      (maxmgval, double, maxmgvidx)

VSIP_IMPL_SAL_V_IDX(minmgval, float,  minmgvix)
// not in SAL      (minmgval, double, minmgvidx)

#undef VSIP_IMPL_SAL_V_IDX

#undef VSIP_IMPL_SAL_INLINE

} // namespace vsip::impl::sal
} // namespace vsip::impl
} // namespace vsip

#endif // VSIP_IMPL_SAL_REDUCTIONS_HPP
