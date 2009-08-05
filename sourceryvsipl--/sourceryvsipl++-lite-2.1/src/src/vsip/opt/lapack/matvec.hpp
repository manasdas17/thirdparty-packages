/* Copyright (c) 2005, 2006 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    vsip/opt/lapack/matvec.hpp
    @author  Jules Bergmann
    @date    2005-10-11
    @brief   VSIPL++ Library: BLAS evaluators (for use in general dispatch).

*/

#ifndef VSIP_OPT_LAPACK_MATVEC_HPP
#define VSIP_OPT_LAPACK_MATVEC_HPP

#if VSIP_IMPL_REF_IMPL
# error "vsip/opt files cannot be used as part of the reference impl."
#endif

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/opt/dispatch.hpp>
#include <vsip/opt/lapack/bindings.hpp>



/***********************************************************************
  Declarations
***********************************************************************/

namespace vsip
{
namespace impl
{
namespace dispatcher
{

/// BLAS evaluator for vector-vector outer product
template <typename T1,
	  typename Block0,
	  typename Block1,
	  typename Block2>
struct Evaluator<Op_prod_vv_outer, Blas_tag,
                 void(Block0&, T1, Block1 const&, Block2 const&)>
{
  static bool const ct_valid = 
    impl::blas::Blas_traits<T1>::valid &&
    Type_equal<T1, typename Block1::value_type>::value &&
    Type_equal<T1, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0;

  static bool rt_valid(Block0& /*r*/, T1 /*alpha*/,
    Block1 const& a, Block2 const& b)
  {
    typedef typename Block_layout<Block1>::order_type order1_type;

    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    return (ext_a.stride(0) == 1) && (ext_b.stride(0) == 1);
  }

  static void exec(Block0& r, T1 alpha, Block1 const& a, Block2 const& b)
  {
    assert(a.size(1, 0) == r.size(2, 0));
    assert(b.size(1, 0) == r.size(2, 1));

    typedef typename Block_layout<Block0>::order_type order0_type;

    Ext_data<Block0> ext_r(const_cast<Block0&>(r));
    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    if (Type_equal<order0_type, row2_type>::value)
    {
      // Use identity:
      //   R = A B     <=>     trans(R) = trans(B) trans(A)

      blas::ger( 
        b.size(1, 0), a.size(1, 0),     // int m, int n,
        alpha,                          // T alpha,
        ext_b.data(), ext_b.stride(0),  // T *x, int incx,
        ext_a.data(), ext_a.stride(0),  // T *y, int incy,
        ext_r.data(), r.size(2, 1)      // T *a, int lda
      );
    }
    else if (Type_equal<order0_type, col2_type>::value)
    {
      blas::ger( 
        a.size(1, 0), b.size(1, 0),     // int m, int n,
        alpha,                          // T alpha,
        ext_a.data(), ext_a.stride(0),  // T *x, int incx,
        ext_b.data(), ext_b.stride(0),  // T *y, int incy,
        ext_r.data(), r.size(2, 0)      // T *a, int lda
      );
    }
    else
      assert(0);
  }
};

template <typename T1,
	  typename Block0,
	  typename Block1,
	  typename Block2>
struct Evaluator<Op_prod_vv_outer, Blas_tag,
                 void(Block0&, std::complex<T1>, Block1 const&, Block2 const&)>
{
  static bool const ct_valid = 
    impl::blas::Blas_traits<std::complex<T1> >::valid &&
    Type_equal<std::complex<T1>, typename Block1::value_type>::value &&
    Type_equal<std::complex<T1>, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0 &&
    // check that format is interleaved.
    !Is_split_block<Block0>::value &&
    !Is_split_block<Block1>::value &&
    !Is_split_block<Block2>::value;

  static bool rt_valid(Block0& /*r*/, std::complex<T1> /*alpha*/, 
    Block1 const& a, Block2 const& b)
  {
    typedef typename Block_layout<Block1>::order_type order1_type;

    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    return (ext_a.stride(0) == 1) && (ext_b.stride(0) == 1);
  }

  static void exec(Block0& r, std::complex<T1> alpha, 
    Block1 const& a, Block2 const& b)
  {
    assert(a.size(1, 0) == r.size(2, 0));
    assert(b.size(1, 0) == r.size(2, 1));

    typedef typename Block_layout<Block0>::order_type order0_type;
    typedef typename Block_layout<Block1>::order_type order1_type;
    typedef typename Block_layout<Block2>::order_type order2_type;

    Ext_data<Block0> ext_r(const_cast<Block0&>(r));
    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    if (Type_equal<order0_type, row2_type>::value)
    {
      // BLAS does not have a function that will conjugate the first 
      // vector and allow us to take advantage of the identity:
      //   R = A B*     <=>     trans(R) = trans(B*) trans(A)
      // This requires a manual conjugation after calling the library 
      // function.

      blas::gerc( 
        b.size(1, 0), a.size(1, 0),     // int m, int n,
        std::complex<T1>(1),            // T alpha,
        ext_b.data(), ext_b.stride(0),  // T *x, int incx,
        ext_a.data(), ext_a.stride(0),  // T *y, int incy,
        ext_r.data(), r.size(2, 1)      // T *a, int lda
      );

      for ( index_type i = 0; i < r.size(2, 0); ++i )
	for ( index_type j = 0; j < r.size(2, 1); ++j )
	  r.put(i, j, alpha * conj(r.get(i, j)));

    }
    else if (Type_equal<order0_type, col2_type>::value)
    {
      blas::gerc( 
        a.size(1, 0), b.size(1, 0),     // int m, int n,
        alpha,                          // T alpha,
        ext_a.data(), ext_a.stride(0),  // T *x, int incx,
        ext_b.data(), ext_b.stride(0),  // T *y, int incy,
        ext_r.data(), r.size(2, 0)      // T *a, int lda
      );
    }
    else
      assert(0);
  }
};


/// BLAS evaluator for vector-vector dot-product (non-conjugated).
template <typename T,
          typename Block0,
          typename Block1>
struct Evaluator<Op_prod_vv_dot, Blas_tag,
                 T(Block0 const&, Block1 const&)>
{
  static bool const ct_valid = 
    impl::blas::Blas_traits<T>::valid &&
    Type_equal<T, typename Block0::value_type>::value &&
    Type_equal<T, typename Block1::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0 &&
    // check that format is interleaved.
    !Is_split_block<Block0>::value &&
    !Is_split_block<Block1>::value;

  static bool rt_valid(Block0 const&, Block1 const&) { return true; }

  static T exec(Block0 const& a, Block1 const& b)
  {
    assert(a.size(1, 0) == b.size(1, 0));

    Ext_data<Block0> ext_a(a);
    Ext_data<Block1> ext_b(b);

    T r = blas::dot(a.size(1, 0),
                    ext_a.data(), ext_a.stride(0),
                    ext_b.data(), ext_b.stride(0));

    return r;
  }
};


/// BLAS evaluator for vector-vector dot-product (conjugated).
template <typename T,
          typename Block0,
          typename Block1>
struct Evaluator<Op_prod_vv_dot, Blas_tag,
                 std::complex<T>(Block0 const&, 
                   Unary_expr_block<1, conj_functor, Block1, std::complex<T> > const&)>
{
  static bool const ct_valid = 
    impl::blas::Blas_traits<complex<T> >::valid &&
    Type_equal<complex<T>, typename Block0::value_type>::value &&
    Type_equal<complex<T>, typename Block1::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0 &&
    // check that format is interleaved.
    !Is_split_block<Block0>::value &&
    !Is_split_block<Block1>::value;

  static bool rt_valid(
    Block0 const&, 
    Unary_expr_block<1, impl::conj_functor, Block1, complex<T> > const&)
  { return true; }

  static complex<T> exec(
    Block0 const& a, 
    Unary_expr_block<1, impl::conj_functor, Block1, complex<T> > const& b)
  {
    assert(a.size(1, 0) == b.size(1, 0));

    Ext_data<Block0> ext_a(const_cast<Block0&>(a));
    Ext_data<Block1> ext_b(const_cast<Block1&>(b.op()));

    return blas::dotc(a.size(1, 0),
                      ext_b.data(), ext_b.stride(0),
                      ext_a.data(), ext_a.stride(0));
    // Note:
    //   BLAS    cdotc(x, y)  => conj(x) * y, while 
    //   VSIPL++ cvjdot(x, y) => x * conj(y)
  }
};


/// BLAS evaluator for matrix-vector product
template <typename Block0,
          typename Block1,
          typename Block2>
struct Evaluator<Op_prod_mv, Blas_tag,
                 void(Block0&, Block1 const&, Block2 const&)>
{
  typedef typename Block0::value_type T;

  static bool const ct_valid = 
    impl::blas::Blas_traits<T>::valid &&
    Type_equal<T, typename Block1::value_type>::value &&
    Type_equal<T, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0 &&
    // check that format is interleaved.
    !Is_split_block<Block0>::value &&
    !Is_split_block<Block1>::value &&
    !Is_split_block<Block2>::value;

  static bool rt_valid(Block0& /*r*/, Block1 const& a, Block2 const& b)
  {
    typedef typename Block_layout<Block1>::order_type order1_type;

    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    // Note: gemm is used for row type and b is restricted to unit stride.
    // gemv is used for col type and can handle any stride for b.
    bool is_a_row = Type_equal<order1_type, row2_type>::value;
    return is_a_row ? ((ext_a.stride(1) == 1) && (ext_b.stride(0) == 1))
                    : (ext_a.stride(0) == 1);
  }

  static void exec(Block0& r, Block1 const& a, Block2 const& b)
  {
    assert(a.size(2, 1) == b.size(1, 0));

    typedef typename Block0::value_type RT;

    typedef typename Block_layout<Block0>::order_type order0_type;
    typedef typename Block_layout<Block1>::order_type order1_type;
    typedef typename Block_layout<Block2>::order_type order2_type;

    Ext_data<Block0> ext_r(const_cast<Block0&>(r));
    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    if (Type_equal<order1_type, row2_type>::value)
    {
      // Use identity:
      //   R = A B      <=>     trans(R) = trans(B) trans(A)
      // to evaluate row-major matrix result with BLAS.

      char transa   = 'n';           // already transposed
      char transb   = 'n';

      blas::gemm(
        transa, transb,
        1,                  // M
        a.size(2, 0),       // N
        a.size(2, 1),       // K
        1.0,                // alpha
        ext_b.data(), 1,    // vector, first dim is implicitly 1
        ext_a.data(), a.size(2, 1),
        0.0,                // beta
        ext_r.data(), 1     // vector, first dim is implicitly 1
      );
    }
    else if (Type_equal<order1_type, col2_type>::value)
    {
      char transa   = 'n';

      blas::gemv( 
        transa,                          // char trans,
        a.size(2, 0), a.size(2, 1),      // int m, int n,
        1.0,                             // T alpha,
        ext_a.data(), ext_a.stride(1),   // T *a, int lda,
        ext_b.data(), ext_b.stride(0),   // T *x, int incx,
        0.0,                             // T beta,
        ext_r.data(), ext_r.stride(0)    // T *y, int incy)
      );
    }
    else assert(0);
  }
};


/// BLAS evaluator for vector-matrix product
template <typename Block0,
          typename Block1,
          typename Block2>
struct Evaluator<Op_prod_vm, Blas_tag,
                 void(Block0&, Block1 const&, Block2 const&)>
{
  typedef typename Block0::value_type T;

  static bool const ct_valid = 
    impl::blas::Blas_traits<T>::valid &&
    Type_equal<T, typename Block1::value_type>::value &&
    Type_equal<T, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0 &&
    // check that format is interleaved.
    !Is_split_block<Block0>::value &&
    !Is_split_block<Block1>::value &&
    !Is_split_block<Block2>::value;

  static bool rt_valid(Block0& /*r*/, Block1 const& a, Block2 const& b)
  {
    typedef typename Block_layout<Block2>::order_type order2_type;

    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    // Note: gemv is used for row type and can handle any stride for a.
    // gemm is used for col type and a is restricted to unit stride.
    // 
    bool is_b_row = Type_equal<order2_type, row2_type>::value;
    return is_b_row ? (ext_b.stride(1) == 1) 
                    : ((ext_b.stride(0) == 1) && (ext_a.stride(0) == 1));
  }

  static void exec(Block0& r, Block1 const& a, Block2 const& b)
  {
    assert(a.size(1, 0) == b.size(2, 0));

    typedef typename Block0::value_type RT;

    typedef typename Block_layout<Block0>::order_type order0_type;
    typedef typename Block_layout<Block1>::order_type order1_type;
    typedef typename Block_layout<Block2>::order_type order2_type;

    Ext_data<Block0> ext_r(const_cast<Block0&>(r));
    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    if (Type_equal<order2_type, row2_type>::value)
    {
      // Use identity:
      //   R = A B      <=>     trans(R) = trans(B) trans(A)
      // to evaluate row-major matrix result with BLAS.

      char transa   = 'n';

      blas::gemv( 
        transa,                          // char trans,
        b.size(2, 1), b.size(2, 0),      // int m, int n,
        1.0,                             // T alpha,
        ext_b.data(), ext_b.stride(0),   // T *a, int lda,
        ext_a.data(), ext_a.stride(0),   // T *x, int incx,
        0.0,                             // T beta,
        ext_r.data(), ext_r.stride(0)    // T *y, int incy)
      );
    }
    else if (Type_equal<order2_type, col2_type>::value)
    {
      char transa   = 'n';
      char transb   = 'n';

      blas::gemm(
        transa, transb,
        1,                  // M
        b.size(2, 1),       // N
        b.size(2, 0),       // K
        1.0,                // alpha
        ext_a.data(), 1,    // vector, first dim is implicitly 1
        ext_b.data(), b.size(2, 0),
        0.0,                // beta
        ext_r.data(), 1     // vector, first dim is implicitly 1
      );
    }
    else assert(0);
  }
};


/// BLAS evaluator for matrix-matrix products.
template <typename Block0,
          typename Block1,
          typename Block2>
struct Evaluator<Op_prod_mm, Blas_tag,
                 void(Block0&, Block1 const&, Block2 const&)>
{
  typedef typename Block0::value_type T;

  static bool const is_block0_interleaved =
    !Is_complex<typename Block0::value_type>::value ||
    Type_equal<typename Block_layout<Block0>::complex_type,
	       Cmplx_inter_fmt>::value;
  static bool const is_block1_interleaved =
    !Is_complex<typename Block1::value_type>::value ||
    Type_equal<typename Block_layout<Block1>::complex_type,
	       Cmplx_inter_fmt>::value;
  static bool const is_block2_interleaved =
    !Is_complex<typename Block2::value_type>::value ||
    Type_equal<typename Block_layout<Block2>::complex_type,
	       Cmplx_inter_fmt>::value;

  static bool const ct_valid = 
    impl::blas::Blas_traits<T>::valid &&
    Type_equal<T, typename Block1::value_type>::value &&
    Type_equal<T, typename Block2::value_type>::value &&
    // check that data is interleaved
    is_block0_interleaved && is_block1_interleaved && is_block2_interleaved &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0;

  static bool rt_valid(Block0& /*r*/, Block1 const& a, Block2 const& b)
  {
    typedef typename Block_layout<Block1>::order_type order1_type;
    typedef typename Block_layout<Block2>::order_type order2_type;

    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    bool is_a_row = Type_equal<order1_type, row2_type>::value;
    bool is_b_row = Type_equal<order2_type, row2_type>::value;

    return ( ext_a.stride(is_a_row ? 1 : 0) == 1 &&
             ext_b.stride(is_b_row ? 1 : 0) == 1 );
  }

  static void exec(Block0& r, Block1 const& a, Block2 const& b)
  {
    typedef typename Block0::value_type RT;

    typedef typename Block_layout<Block0>::order_type order0_type;
    typedef typename Block_layout<Block1>::order_type order1_type;
    typedef typename Block_layout<Block2>::order_type order2_type;

    Ext_data<Block0> ext_r(const_cast<Block0&>(r));
    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    if (Type_equal<order0_type, row2_type>::value)
    {
      bool is_a_row = Type_equal<order1_type, row2_type>::value;
      char transa   = is_a_row ? 'n' : 't';
      int  lda      = is_a_row ? ext_a.stride(0) : ext_a.stride(1);

      bool is_b_row = Type_equal<order2_type, row2_type>::value;
      char transb   = is_b_row ? 'n' : 't';
      int  ldb      = is_b_row ? ext_b.stride(0) : ext_b.stride(1);

      // Use identity:
      //   R = A B      <=>     trans(R) = trans(B) trans(A)
      // to evaluate row-major matrix result with BLAS.

      blas::gemm(transb, transa,
                 b.size(2, 1),  // N
                 a.size(2, 0),  // M
                 a.size(2, 1),  // K
                 1.0,           // alpha
                 ext_b.data(), ldb,
                 ext_a.data(), lda,
                 0.0,           // beta
                 ext_r.data(), ext_r.stride(0));
    }
    else if (Type_equal<order0_type, col2_type>::value)
    {
      bool is_a_col = Type_equal<order1_type, col2_type>::value;
      char transa   = is_a_col ? 'n' : 't';
      int  lda      = is_a_col ? ext_a.stride(1) : ext_a.stride(0);

      bool is_b_col = Type_equal<order2_type, col2_type>::value;
      char transb   = is_b_col ? 'n' : 't';
      int  ldb      = is_b_col ? ext_b.stride(1) : ext_b.stride(0);

      blas::gemm(transa, transb,
                 a.size(2, 0),  // M
                 b.size(2, 1),  // N
                 a.size(2, 1),  // K
                 1.0,           // alpha
                 ext_a.data(), lda,
                 ext_b.data(), ldb,
                 0.0,           // beta
                 ext_r.data(), ext_r.stride(1));
    }
    else assert(0);
  }
};


/// BLAS evaluator for generalized matrix-matrix products.
template <typename T1,
	  typename T2,
	  typename Block0,
          typename Block1,
          typename Block2>
struct Evaluator<Op_prod_gemp, Blas_tag,
                 void(Block0&, T1, Block1 const&, Block2 const&, T2)>
{
  typedef typename Block0::value_type T;

  static bool const ct_valid = 
    impl::blas::Blas_traits<T>::valid &&
    Type_equal<T, typename Block1::value_type>::value &&
    Type_equal<T, typename Block2::value_type>::value &&
    // check that direct access is supported
    Ext_data_cost<Block0>::value == 0 &&
    Ext_data_cost<Block1>::value == 0 &&
    Ext_data_cost<Block2>::value == 0 &&
    // check that format is interleaved.
    !Is_split_block<Block0>::value &&
    !Is_split_block<Block1>::value &&
    !Is_split_block<Block2>::value;

  static bool rt_valid(Block0&, T1, Block1 const& a, Block2 const& b, T2)
  {
    typedef typename Block_layout<Block1>::order_type order1_type;
    typedef typename Block_layout<Block2>::order_type order2_type;

    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    bool is_a_row = Type_equal<order1_type, row2_type>::value;
    bool is_b_row = Type_equal<order2_type, row2_type>::value;

    return ( ext_a.stride(is_a_row ? 1 : 0) == 1 &&
             ext_b.stride(is_b_row ? 1 : 0) == 1 );
  }

  static void exec(Block0& r, T1 alpha, Block1 const& a, 
    Block2 const& b, T2 beta)
  {
    typedef typename Block0::value_type RT;

    typedef typename Block_layout<Block0>::order_type order0_type;
    typedef typename Block_layout<Block1>::order_type order1_type;
    typedef typename Block_layout<Block2>::order_type order2_type;

    Ext_data<Block0> ext_r(const_cast<Block0&>(r));
    Ext_data<Block1> ext_a(const_cast<Block1&>(a));
    Ext_data<Block2> ext_b(const_cast<Block2&>(b));

    if (Type_equal<order0_type, row2_type>::value)
    {
      bool is_a_row = Type_equal<order1_type, row2_type>::value;
      char transa   = is_a_row ? 'n' : 't';
      int  lda      = is_a_row ? ext_a.stride(0) : ext_a.stride(1);

      bool is_b_row = Type_equal<order2_type, row2_type>::value;
      char transb   = is_b_row ? 'n' : 't';
      int  ldb      = is_b_row ? ext_b.stride(0) : ext_b.stride(1);

      // Use identity:
      //   R = A B      <=>     trans(R) = trans(B) trans(A)
      // to evaluate row-major matrix result with BLAS.

      blas::gemm(transb, transa,
                 b.size(2, 1),  // N
                 a.size(2, 0),  // M
                 a.size(2, 1),  // K
                 alpha,         // alpha
                 ext_b.data(), ldb,
                 ext_a.data(), lda,
                 beta,          // beta
                 ext_r.data(), ext_r.stride(0));
    }
    else if (Type_equal<order0_type, col2_type>::value)
    {
      bool is_a_col = Type_equal<order1_type, col2_type>::value;
      char transa   = is_a_col ? 'n' : 't';
      int  lda      = is_a_col ? ext_a.stride(1) : ext_a.stride(0);

      bool is_b_col = Type_equal<order2_type, col2_type>::value;
      char transb   = is_b_col ? 'n' : 't';
      int  ldb      = is_b_col ? ext_b.stride(1) : ext_b.stride(0);

      blas::gemm(transa, transb,
                 a.size(2, 0),  // M
                 b.size(2, 1),  // N
                 a.size(2, 1),  // K
                 alpha,         // alpha
                 ext_a.data(), lda,
                 ext_b.data(), ldb,
                 beta,          // beta
                 ext_r.data(), ext_r.stride(1));
    }
    else assert(0);
  }
};


} // namespace vsip::impl::dispatcher
} // namespace vsip::impl
} // namespace vsip

#endif // VSIP_OPT_LAPACK_MATVEC_HPP
