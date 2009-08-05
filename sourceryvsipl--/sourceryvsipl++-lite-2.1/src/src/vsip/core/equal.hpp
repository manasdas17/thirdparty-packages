/* Copyright (c) 2006 by CodeSourcery, LLC.  All rights reserved. */

/** @file    vsip/core/equal.cpp
    @author  Stefan Seefeld
    @date    2006-04-20
    @brief   VSIPL++ Library: Compare various objects for numeric equality.
*/

#ifndef VSIP_CORE_EQUAL_HPP
#define VSIP_CORE_EQUAL_HPP

#include <vsip/core/fns_scalar.hpp>

namespace vsip
{
namespace impl
{

/// Compare two floating-point values for equality.
///
/// Algorithm from:
///    www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm

template <typename T>
inline bool
almost_equal(T A, T B, T rel_epsilon = 1e-4, T abs_epsilon = 1e-6)
{
  if (fn::mag(A - B) < abs_epsilon)
    return true;

  T relative_error;

  if (fn::mag(B) > fn::mag(A))
    relative_error = fn::mag((A - B) / B);
  else
    relative_error = fn::mag((B - A) / A);

  return (relative_error <= rel_epsilon);
}

} // namespace vsip::impl
} // namespace vsip

#endif
