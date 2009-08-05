/* Copyright (c) 2005 by CodeSourcery, LLC.  All rights reserved. */

/** @file    vsip/core/matvec.hpp
    @author  Don McCoy
    @date    2005-09-18
    @brief   VSIPL++ Library: [math.matvec]

    Matrix and Vector Operations 
*/

#ifndef VSIP_CORE_MATVEC_HPP
#define VSIP_CORE_MATVEC_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/vector.hpp>
#include <vsip/matrix.hpp>
#include <vsip/core/promote.hpp>
#include <vsip/core/fns_elementwise.hpp>
#include <vsip/core/dispatch.hpp>
#include <vsip/core/impl_tags.hpp>
#ifndef VSIP_IMPL_REF_IMPL
# include <vsip/opt/dispatch.hpp>
# ifdef VSIP_IMPL_CBE_SDK
#  include <vsip/opt/cbe/cml/matvec.hpp>
# endif
# ifdef VSIP_IMPL_HAVE_BLAS
#  include <vsip/opt/lapack/matvec.hpp>
# endif
# ifdef VSIP_IMPL_HAVE_SAL
#  include <vsip/opt/sal/eval_misc.hpp>
# endif
# ifdef VSIP_IMPL_HAVE_CUDA
#  include <vsip/opt/cuda/matvec.hpp>
# endif
#endif
#if VSIP_IMPL_HAVE_CVSIP
# include <vsip/core/cvsip/matvec.hpp>
#endif
#include <vsip/core/math_enum.hpp>
#include <vsip/core/profile.hpp>

namespace vsip
{
namespace impl
{
namespace dispatcher
{


#ifndef VSIP_IMPL_REF_IMPL
template<>
struct List<Op_prod_vv_outer>
{
  typedef Make_type_list<Cml_tag, Blas_tag, Mercury_sal_tag, 
    Cvsip_tag, Generic_tag>::type type;
};
#endif

/// Generic evaluator for outer product
template <typename T1,
	  typename Block0,
	  typename Block1,
	  typename Block2>
struct Evaluator<Op_prod_vv_outer, Generic_tag,
                 void(Block0&, T1, Block1 const&, Block2 const&)>
{
  static bool const ct_valid = true;
  static bool rt_valid(Block0&, T1, Block1 const&, Block2 const&) 
    { return true; }

  static void exec(Block0& r, T1 alpha, Block1 const& a, Block2 const& b)
  {
    assert(a.size(1, 0) == r.size(2, 0));
    assert(b.size(1, 0) == r.size(2, 1));

    // r(i, j) = alpha * a(i) * b(j)
    for ( index_type i = a.size(); i-- > 0; )
      for ( index_type j = b.size(); j-- > 0; )
        r.put( i, j, alpha * a.get(i) * b.get(j) );
  }
};

/// Generic evaluator for outer product (with conjugate)
template <typename T1,
	  typename Block0,
	  typename Block1,
	  typename Block2>
struct Evaluator<Op_prod_vv_outer, Generic_tag,
                 void(Block0&, std::complex<T1>, Block1 const&, Block2 const&)>
{
  static bool const ct_valid = true;
  static bool rt_valid(Block0&, std::complex<T1>, Block1 const&, Block2 const&) 
    { return true; }

  static void exec(Block0& r, std::complex<T1> alpha, Block1 const& a, Block2 const& b)
  {
    assert(a.size(1, 0) == r.size(2, 0));
    assert(b.size(1, 0) == r.size(2, 1));

    // r(i, j) = alpha * a(i) * b(j)
    for ( index_type i = a.size(); i-- > 0; )
      for ( index_type j = b.size(); j-- > 0; )
        r.put( i, j, alpha * a.get(i) * conj(b.get(j)) );
  }
};



#ifndef VSIP_IMPL_REF_IMPL
template<>
struct List<Op_prod_vv_dot>
{
  typedef Make_type_list<Cml_tag, Cuda_tag, Blas_tag, Mercury_sal_tag, 
    Cvsip_tag, Generic_tag>::type type;
};
#endif

/// Generic evaluator for vector-vector dot-product.
template <typename T,
	  typename Block0,
	  typename Block1>
struct Evaluator<Op_prod_vv_dot, Generic_tag,
                 T(Block0 const&, Block1 const&)>
{
  static bool const ct_valid = true;
  static bool rt_valid(Block0 const&, Block1 const&) { return true; }

  static T exec(Block0 const& a, Block1 const& b)
  {
    assert(a.size(1, 0) == b.size(1, 0));

    T r = T();
    for ( index_type i = 0; i < a.size(); ++i )
      r += a.get(i) * b.get(i);
    return r;
  }
};


#ifndef VSIP_IMPL_REF_IMPL
template<>
struct List<Op_prod_gemp>
{
  typedef Make_type_list<Cml_tag, Blas_tag, Mercury_sal_tag, 
    Cvsip_tag, Generic_tag>::type type;
};
#endif

/// Generic evaluator for general product
template <typename T1,
	  typename T2,
	  typename Block0,
	  typename Block1,
	  typename Block2>
struct Evaluator<Op_prod_gemp, Generic_tag,
                 void(Block0&, T1, Block1 const&, Block2 const&, T2)>
{
  static bool const ct_valid = true;
  static bool rt_valid(Block0&, T1, Block1 const&, Block2 const&, T2) 
    { return true; }

  static void exec(Block0& c, T1 alpha, Block1 const& a, 
    Block2 const& b, T2 beta)
  {
    assert( a.size(2, 0) == c.size(2, 0) );
    assert( b.size(2, 1) == c.size(2, 1) );
    assert( a.size(2, 1) == b.size(2, 0) );  
    
    // c(i,j) = alpha * a(i) * b(j) + beta * c(i,j)
    for ( index_type i = a.size(2, 0); i-- > 0; )
    {
      for ( index_type j = b.size(2, 1); j-- > 0; )
      {
        T1 dot = T1();
        for ( index_type k = 0; k < a.size(2, 1); ++k )
          dot += a.get(i, k) * b.get(k, j);

        c.put(i, j, alpha * dot + beta * c.get(i, j));
      }
    }
  }
};

} // namespace dispatcher



/// Outer product dispatch
template <typename T0,
	  typename T1,
	  typename T2,
	  typename Block0,
	  typename Block1,
	  typename Block2>
void
outer(
  T0 alpha, 
  const_Vector<T0, Block0> a,
  const_Vector<T1, Block1> b,
  Matrix<T2, Block2>       r)
{
  assert(r.size(0) == a.size());
  assert(r.size(1) == b.size());
#ifdef VSIP_IMPL_REF_IMPL
  dispatcher::Evaluator<dispatcher::Op_prod_vv_outer, Cvsip_tag, 
    void(Block2&, T0, Block0 const&, Block1 const&)>::exec
    (r.block(), alpha, a.block(), b.block());
#else
  impl::dispatch<dispatcher::Op_prod_vv_outer, void,
    Block2&, T0, Block0 const&, Block1 const&>
    (r.block(), alpha, a.block(), b.block());
#endif
}


// Dot product dispatch.  This is intentionally not named 'dot' to avoid
// conflicting with vsip::dot, which shares the same signature, forcing
// the user to resolve the call themselves.
template <typename T0, typename T1, typename Block0, typename Block1>
typename Promotion<T0, T1>::type
impl_dot(
  const_Vector<T0, Block0> v,
  const_Vector<T1, Block1> w) VSIP_NOTHROW
{
  typedef typename Promotion<T0, T1>::type return_type;
  return_type r(0);

#ifdef VSIP_IMPL_REF_IMPL
  r = dispatcher::Evaluator<dispatcher::Op_prod_vv_dot, Cvsip_tag,
    return_type(Block0 const&, Block1 const&) >::exec
    (v.block(), w.block());
#else
  r = impl::dispatch<dispatcher::Op_prod_vv_dot, return_type,
    Block0 const&, Block1 const&>
    (v.block(), w.block());
#endif
  return r;
};


/// Generic vector-vector kron
template <typename T0,
          typename T1,
          typename T2,
          typename Block1,
          typename Block2>
const_Matrix<typename Promotion<T0, typename Promotion<T1, T2>::type>::type>
impl_kron( T0 alpha, Vector<T1, Block1> v, Vector<T2, Block2> w)
    VSIP_NOTHROW
{
  typedef Matrix<typename Promotion<T0, 
    typename Promotion<T1, T2>::type>::type> return_type;
  return_type r(w.size(), v.size());

  for (index_type i = w.size(); i-- > 0;)
    for (index_type j = v.size(); j-- > 0;)
      r.put(i, j, alpha * w.get(i) * v.get(j));

  return r;
}

/// Generic matrix-matrix kron
template <typename T0,
          typename T1,
          typename T2,
          typename Block1,
          typename Block2>
const_Matrix<typename Promotion<T0, typename Promotion<T1, T2>::type>::type>
impl_kron( T0 alpha, Matrix<T1, Block1> v, Matrix<T2, Block2> w)
    VSIP_NOTHROW
{
  typedef Matrix<typename Promotion<T0, 
    typename Promotion<T1, T2>::type>::type> return_type;
  const length_type row_size = v.size(0) * w.size(0);
  const length_type col_size = v.size(1) * w.size(1);

  return_type r( row_size, col_size );
  for ( index_type i = v.size(0); i-- > 0; )
    for ( index_type j = w.size(0); j-- > 0; )
      for ( index_type k = v.size(1); k-- > 0; )
        for ( index_type l = w.size(1); l-- > 0; ) 
        {
          T0 val = alpha * v.get(i,k) * w.get(j,l);
          r.put( j + (i * w.size(0)), l + (k * w.size(1)), val );
        }

  return r;
}


/// General matrix product dispatch
template <typename T0,
	  typename T1,
	  typename T2,
	  typename T4,
	  typename Block1,
	  typename Block2,
	  typename Block4>
void
gemp(
  T0 alpha, const_Matrix<T1, Block1> a,
  const_Matrix<T2, Block2> b, T0 beta, Matrix<T4, Block4> c) 
{
  assert(c.size(0) == a.size(0));
  assert(c.size(1) == b.size(1));

#ifdef VSIP_IMPL_REF_IMPL
  dispatcher::Evaluator<dispatcher::Op_prod_gemp, Cvsip_tag,
    void(Block4&, T0, Block1 const&, Block2 const&, T0)>::exec
    (c.block(), alpha, a.block(), b.block(), beta);
#else
  impl::dispatch<dispatcher::Op_prod_gemp, void,
    Block4&, T0, Block1 const&, Block2 const&, T0> 
    (c.block(), alpha, a.block(), b.block(), beta);
#endif
}

/// General matrix sum
template <typename T0, typename T1, typename T3, typename T4,
  typename Block1, typename Block4>
void 
gems( T0 alpha, const_Matrix<T1, Block1> A, T3 beta, Matrix<T4, Block4> C) 
{
  assert( A.size(0) == C.size(0) );
  assert( A.size(1) == C.size(1) );
  C = alpha * A + beta * C;
}



/// Class to perform transpose or hermetian (conjugate-transpose),
/// depending on value type.

/// Primary case - perform transpose.
template <typename T,
	  typename Block>
struct Trans_or_herm
{
  typedef typename const_Matrix<T, Block>::transpose_type result_type;

  static result_type
  exec(const_Matrix<T, Block> m) VSIP_NOTHROW
  {
    return m.transpose();
  }
};

/// Complex specialization - perform hermetian.
template <typename T,
	  typename Block>
struct Trans_or_herm<complex<T>, Block>
{
  typedef typename const_Matrix<complex<T>, Block>::transpose_type 
      transpose_type;
  typedef impl::Unary_func_view<impl::conj_functor, transpose_type> 
      functor_type;
  typedef typename functor_type::result_type result_type;

  static result_type
  exec(const_Matrix<complex<T>, Block> m) VSIP_NOTHROW
  {
    return functor_type::apply(m.transpose());
  } 
};

/// Perform transpose or hermetian, depending on value type.
template <typename T,
	  typename Block>
inline
typename Trans_or_herm<T, Block>::result_type
trans_or_herm(const_Matrix<T, Block> m)
{
  return Trans_or_herm<T, Block>::exec(m);
};



/// Generalized class used to invoke the correct matrix operator
template <mat_op_type OpT,
          typename    T,
          typename    Block>
struct Apply_mat_op;

/// 'No transpose' matrix operator
template <typename T,
          typename Block>
struct Apply_mat_op<mat_ntrans, T, Block>
{
  typedef const_Matrix<T, Block> result_type;

  static result_type
  exec(const_Matrix<T, Block> m) VSIP_NOTHROW
    {
      return m;
    }
};

/// 'Transpose' matrix operator
template <typename T,
          typename Block>
struct Apply_mat_op<mat_trans, T, Block>
{
  typedef typename const_Matrix<T, Block>::transpose_type result_type;

  static result_type
  exec(const_Matrix<T, Block> m) VSIP_NOTHROW
    {
      return m.transpose();
    }
};

/// 'Hermitian' matrix operator for non-complex types (results in transpose)
template <typename T,
          typename Block>
struct Apply_mat_op<mat_herm, T, Block>
{
  typedef typename const_Matrix<T, Block>::transpose_type result_type;

  static result_type
  exec(const_Matrix<T, Block> m) VSIP_NOTHROW
    {
      return impl::trans_or_herm(m);
    }
};

/// 'Hermitian' matrix operator for complex types (results in conjugate 
/// transpose)
template <typename T,
          typename Block>
struct Apply_mat_op<mat_herm, complex<T>, Block>
{
  typedef typename const_Matrix<complex<T>, Block>::transpose_type 
      transpose_type;
  typedef impl::Unary_func_view<impl::conj_functor, transpose_type> 
      functor_type;
  typedef typename functor_type::result_type result_type;

  static result_type
  exec(const_Matrix<complex<T>, Block> m) VSIP_NOTHROW
    {
      return impl::trans_or_herm(m);
    }
};

/// 'Conjugate' matrix operator for non-complex types (results in identity)
template <typename T,
          typename Block>
struct Apply_mat_op<mat_conj, T, Block>
{
  typedef const_Matrix<T, Block> result_type;

  static result_type
  exec(const_Matrix<T, Block> m) VSIP_NOTHROW
    {
      return m;
    }
};

/// 'Conjugate' matrix operator for complex types
template <typename T,
          typename Block>
struct Apply_mat_op<mat_conj, complex<T>, Block>
{
  typedef impl::Unary_func_view<impl::conj_functor, 
    const_Matrix<complex<T>, Block> > functor_type;
  typedef typename functor_type::result_type result_type;

  static result_type
  exec(const_Matrix<complex<T>, Block> m) VSIP_NOTHROW
    {
      return conj(m);
    }
};


/// Convenience function to use matrix operator classes
template <mat_op_type OpT,
          typename    T,
          typename    Block>
typename Apply_mat_op<OpT, T, Block>::result_type
apply_mat_op(const_Matrix<T, Block> m)
{
  return Apply_mat_op<OpT, T, Block>::exec(m);
}


/// Generic cumulative vector sum
template <dimension_type d,
          typename T0,
          typename T1,
          typename Block0,
          typename Block1>
void
cumsum(
  const_Vector<T0, Block0> v,
  Vector<T1, Block1> w) 
    VSIP_NOTHROW
{
  //  Effects: w has values equaling the cumulative sum of values in v. 
  //
  //  If View is Vector, d is ignored and, for 
  //    0 <= i < v.size(), 
  //      w.get(i) equals the sum over 0 <= j <= i of v.get(j)
  assert( v.size() == w.size() );

  T1 sum = T0();
  for ( index_type i = 0; i < v.size(); ++i )
  {
    sum += v.get(i);
    w.put(i, sum);
  }
}

/// Generic cumulative matrix sum
template <dimension_type d,
          typename T0,
          typename T1,
          typename Block0,
          typename Block1>
void
cumsum(
  const_Matrix<T0, Block0> v,
  Matrix<T1, Block1> w) 
    VSIP_NOTHROW
{
  if ( d == 0 )
  {
    //  If View is Matrix and d == 0, then, for 
    //    0 <= m < v.size(0) and 0 <= i < v.size(1),
    //      w.get(m, i) equals the sum over 0 <= j <= i of v.get(m, j).

    for ( index_type m = 0; m < v.size(0); ++m )
    {
      T1 sum = T0();
      for ( index_type i = 0; i < v.size(1); ++i )
      {
        sum += v.get(m, i);
        w.put(m, i, sum);
      }
    }
  }
  else
  if ( d == 1 )
  {
    //  If View is Matrix and d == 1, then, for 
    //    0 <= i < v.size(0) and 0 <= n < v.size(1), 
    //      w.get(i, n) equals the sum over 0 <= j <= i of v.get(j, n).

    for ( index_type n = 0; n < v.size(1); ++n )
    {
      T1 sum = T0();
      for ( index_type i = 0; i < v.size(0); ++i )
      {
        sum += v.get(i, n);
        w.put(i, n, sum);
      }
    }
  }
}


/// Generic modulation of a vector by a complex frequency
template <typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename Block0,
          typename Block1>
T1
modulate(
  const_Vector<T0, Block0> v,
  T1 nu,
  T2 phi,
  Vector<complex<T3>, Block1> w)
    VSIP_NOTHROW
{
  // Requires: The only specializations which must be supported are those 
  // having T0, T1, T2, and T3 all scalar f or alternatively T0 the same 
  // as cscalar f and T1, T2, and T3 all scalar f.

  assert(v.size() == w.size());
  assert((Type_equal<typename impl::Scalar_of<T0>::type, T3>::value));
  assert((Type_equal<T1, T3>::value));
  assert((Type_equal<T2, T3>::value));

  // Effects: For 0 <= i < v.size(), w.get(i) has a value equaling 
  // the product of v.get(i) and the exponential of the product of 
  // j (sqrt(-1)) and i * nu + phi

  for ( index_type i = v.size(); i-- > 0; )
  {
    complex<T3> phase( 0, i * nu + phi );
    w.put(i, v.get(i) * exp(phase));
  }

  // Returns: v.size() * nu + phi (the phase needed for processing
  // a series frame by frame)

  return v.size() * nu + phi;
}

} // namespace impl



/***********************************************************************
  Functions
***********************************************************************/


/// dot products  [math.matvec.dot]

/// dot
template <typename T0, typename T1, typename Block0, typename Block1>
typename Promotion<T0, T1>::type
dot(
  const_Vector<T0, Block0> v,
  const_Vector<T1, Block1> w) VSIP_NOTHROW
{
#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  typedef typename Promotion<T0, T1>::type result_type;
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<result_type>::tag("dot", impl::extent(v)),
     impl::matvec::Op_count_dot<result_type>::value(impl::extent(v)) );
#endif
  return impl::impl_dot(v, w);
}


/// cvjdot
template <typename T0, typename T1, typename Block0, typename Block1>
typename Promotion<complex<T0>, complex<T1> >::type
cvjdot(
  const_Vector<complex<T0>, Block0> v,
  const_Vector<complex<T1>, Block1> w) VSIP_NOTHROW
{
#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  typedef typename Promotion<T0, T1>::type result_type;
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<result_type>::tag("cvjdot", impl::extent(v)),
     impl::matvec::Op_count_cvjdot<result_type>::value(impl::extent(v)));
#endif
  return impl::impl_dot(v, conj(w));
}

 
/// Transpositions  [math.matvec.transpose]

/// transpose
template <typename T, typename Block>
typename const_Matrix<T, Block>::transpose_type
trans(const_Matrix<T, Block> m) VSIP_NOTHROW
{
#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<T>::tag("trans", impl::extent(m)));
#endif
  return m.transpose();
}

/// conjugate transpose
template <typename T, typename Block>
typename impl::Unary_func_view<impl::conj_functor,
  typename const_Matrix<complex<T>,
  Block>::transpose_type>::result_type
herm(const_Matrix<complex<T>, Block> m) VSIP_NOTHROW
{
  typedef typename const_Matrix<complex<T>, Block>::transpose_type 
    transpose_type;
  typedef impl::Unary_func_view<impl::conj_functor, transpose_type> 
    functor_type;

#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<complex<T> >::tag("herm", impl::extent(m)),
     impl::matvec::Op_count_herm<complex<T> >::value(impl::extent(m)));
#endif
  return functor_type::apply(m.transpose());
} 


/// Kronecker tensor product  [math.matvec.kron]

/// kronecker product
template <typename T0,
          typename T1,
          typename T2,
          template <typename, typename> class const_View,
          typename Block1,
          typename Block2>
const_Matrix<typename Promotion<T0, typename Promotion<T1, T2>::type>::type>
kron( T0 alpha, const_View<T1, Block1> v, const_View<T2, Block2> w )
    VSIP_NOTHROW
{
#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  typedef typename Promotion<T0, typename Promotion<T1, T2>::type
    >::type result_type;
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<result_type>::tag("kron", impl::extent(v), 
                                                 impl::extent(w)),
     impl::matvec::Op_count_kron<impl::Dim_of_view<const_View>::dim, 
     result_type>::value(impl::extent(v), impl::extent(w)));
#endif
  return impl::impl_kron( alpha, v, w );
}


/// Outer product [math.matvec.outer]

/// outer product of two scalar vectors
template <typename T0,
          typename T1,
          typename T2,
          typename Block1,
          typename Block2>
const_Matrix<typename Promotion<T0, typename Promotion<T1, T2>::type>::type>
outer( T0 alpha, const_Vector<T1, Block1> v, const_Vector<T2, Block2> w )
    VSIP_NOTHROW
{
  typedef typename Promotion<T1, T2>::type return_type;
#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<return_type>::tag("outer", impl::extent(v), 
                                                 impl::extent(w)),
     impl::matvec::Op_count_outer<return_type>::value(impl::extent(v), 
                                                      impl::extent(w)));
#endif

  Matrix<return_type> r(v.size(), w.size(), return_type());

  impl::outer(alpha, v, w, r);

  return r;
}


/// Generalized Matrix operations [math.matvec.gem]

/// generalized matrix product
template <mat_op_type OpA,
          mat_op_type OpB,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename Block1,
          typename Block2,
          typename Block4>
void
  gemp(
    T0 alpha,
    const_Matrix<T1, Block1> A,
    const_Matrix<T2, Block2> B,
    T3 beta,
    Matrix<T4, Block4> C)
     VSIP_NOTHROW
{
#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<T4>::tag("gemp", impl::extent(A), 
                                        impl::extent(B)),
     impl::matvec::Op_count_gemp<T4>::value(impl::extent(A), 
                                            impl::extent(B), OpA, OpB));
#endif

  // equivalent to C = alpha * OpA(A) * OpB(B) + beta * C

  impl::gemp( alpha, 
              impl::apply_mat_op<OpA>(A), 
              impl::apply_mat_op<OpB>(B),
              beta, 
              C );
} 


/// Generalized matrix sum
template <mat_op_type OpA,
          typename T0,
          typename T1,
          typename T3,
          typename T4,
          typename Block1,
          typename Block4>
void
gems(
  T0 alpha,
  const_Matrix<T1, Block1> A,
  T3 beta,
  Matrix<T4, Block4> C) 
    VSIP_NOTHROW
{
#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<T4>::tag("gems", impl::extent(A)),
     impl::matvec::Op_count_gems<T4>::value(impl::extent(A), OpA));
#endif
  impl::gems(alpha, impl::apply_mat_op<OpA>(A), beta, C);
}


/// Miscellaneous functions [math.matvec.misc]

/// cumulative sum
template <dimension_type d,
          typename T0,
          typename T1,
          template <typename, typename> class const_View,
          template <typename, typename> class View,
          typename Block0,
          typename Block1>
void
cumsum(
  const_View<T0, Block0> v,
  View<T1, Block1> w) 
    VSIP_NOTHROW
{
#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  dimension_type const dim = impl::Dim_of_view<const_View>::dim;
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<T0>::tag("cumsum", impl::extent(v)),
     impl::matvec::Op_count_cumsum<dim, T0>::value(impl::extent(v)));
#endif

  impl::cumsum<d>(v, w);
}

/// modulate
template <typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename Block0,
          typename Block1>
T1
modulate(
  const_Vector<T0, Block0> v,
  T1 nu,
  T2 phi,
  Vector<complex<T3>, Block1> w)
    VSIP_NOTHROW
{
#if VSIP_IMPL_PROFILER & VSIP_IMPL_PROFILE_MASK_MATVEC
  impl::profile::Scope<impl::profile::matvec> scope
    (impl::matvec::Description<T0>::tag("modulate", impl::extent(v)),
     impl::matvec::Op_count_modulate<T0>::value(impl::extent(v)));
#endif
  return impl::modulate(v, nu, phi, w);
}

} // namespace vsip

#endif // VSIP_IMPL_MATVEC_HPP
