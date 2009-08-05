/* Copyright (c) 2005, 2006 by CodeSourcery, LLC.  All rights reserved. */

/** @file    vsip/core/cvsip/cholesky.hpp
    @author  Assem Salama
    @date    2006-11-15
    @brief   VSIPL++ Library: Cholesky linear system solver using cvsip.

*/

#ifndef VSIP_CORE_CVSIP_CHOLESKY_HPP
#define VSIP_CORE_CVSIP_CHOLESKY_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <algorithm>

#include <vsip/support.hpp>
#include <vsip/matrix.hpp>
#include <vsip/core/math_enum.hpp>
#include <vsip/core/temp_buffer.hpp>
#include <vsip/core/working_view.hpp>
#include <vsip/core/fns_elementwise.hpp>
#include <vsip/core/solver/common.hpp>
#include <vsip/core/cvsip/solver.hpp>
#include <vsip/core/cvsip/view.hpp>

/***********************************************************************
  Declarations
***********************************************************************/

namespace vsip
{
namespace impl
{

/// Cholesky factorization implementation class.  Common functionality
/// for chold by-value and by-reference classes.

template <typename T>
class Chold_impl<T,Cvsip_tag>
  : impl::Compile_time_assert<cvsip::Solver_traits<T>::valid>
{
  typedef cvsip::Solver_traits<T> traits;
  typedef vsip::impl::dense_complex_type   complex_type;
  typedef Layout<2, row2_type, Stride_unit_dense, complex_type> data_LP;
  typedef Fast_block<2, T, data_LP> data_block_type;

  // Constructors, copies, assignments, and destructors.
public:
  Chold_impl(mat_uplo uplo, length_type length)
    : uplo_(uplo),
      length_(length),
      data_(length_, length_),
      cvsip_data_(data_.block().impl_data(), length_, length_, true),
      chol_(traits::chol_create(length_, uplo_))
  { assert(length_ > 0);}

  Chold_impl(Chold_impl const& chol)
    : uplo_(chol.uplo_),
    length_(chol.length_),
    data_(length_, length_),
    cvsip_data_(data_.block().impl_data(), length_, length_, true),
    chol_(traits::chol_create(length_, uplo_))
  { data_ = chol.data_;}

  ~Chold_impl() { traits::chol_destroy(chol_);}

  mat_uplo    uplo()  const VSIP_NOTHROW { return uplo_;}
  length_type length()const VSIP_NOTHROW { return length_;}

  /// Form Cholesky factorization of matrix A
  ///
  /// Requires
  ///   A to be a square matrix, either
  ///     symmetric positive definite (T real), or
  ///     hermitian positive definite (T complex).
  ///
  /// FLOPS:
  ///   real   : (1/3) n^3
  ///   complex: (4/3) n^3
  template <typename Block>
  bool decompose(Matrix<T, Block> m) VSIP_NOTHROW
  {
    assert(m.size(0) == length_ && m.size(1) == length_);

    cvsip_data_.block().release(false);
    assign_local(data_, m);
    cvsip_data_.block().admit(true);
    return !traits::chol_decompose(chol_, cvsip_data_.ptr());
  }

protected:

  /// Solve A x = b (where A previously given to decompose)

  template <typename Block0, typename Block1>
  bool impl_solve(const_Matrix<T, Block0> b, Matrix<T, Block1> x) VSIP_NOTHROW
  {
    typedef typename Block_layout<Block0>::order_type order_type;
    typedef typename Block_layout<Block0>::complex_type complex_type;
    typedef Layout<2, order_type, Stride_unit_dense, complex_type> data_LP;
    typedef Fast_block<2, T, data_LP, Local_map> block_type;

    assert(b.size(0) == length_);
    assert(b.size(0) == x.size(0) && b.size(1) == x.size(1));

    Matrix<T, block_type> b_int(b.size(0), b.size(1));
    assign_local(b_int, b);
    {
      Ext_data<block_type> b_ext(b_int.block());
      cvsip::View<2,T,true>
        cvsip_b_int(b_ext.data(),0,b_ext.stride(0),b_ext.size(0),
                    b_ext.stride(1),b_ext.size(1));

      cvsip_b_int.block().admit(true);
      traits::chol_solve(chol_, cvsip_b_int.ptr());
      cvsip_b_int.block().release(true);
    }
    assign_local(x, b_int);
    return true;
  }

  // Member data.
private:
  typedef std::vector<float, Aligned_allocator<float> > vector_type;

  Chold_impl& operator=(Chold_impl const&) VSIP_NOTHROW;

  mat_uplo     uplo_;			// A upper/lower triangular
  length_type  length_;			// Order of A.

  Matrix<T, data_block_type> data_;	// Factorized Cholesky matrix (A)
  cvsip::View<2,T,true>      cvsip_data_;
  typename traits::chol_type *chol_;
};



// The CVSIP Cholesky solver supports all CVSIP types.

template <typename T>
struct Is_chold_impl_avail<Cvsip_tag, T>
{
  static bool const value = cvsip::Solver_traits<T>::valid;
};

} // namespace vsip::impl
} // namespace vsip

#endif
