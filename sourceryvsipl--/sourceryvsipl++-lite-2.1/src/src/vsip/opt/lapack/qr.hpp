/* Copyright (c) 2005, 2006, 2008 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    vsip/opt/lapack/qr.hpp
    @author  Jules Bergmann
    @date    2005-08-19
    @brief   VSIPL++ Library: QR Linear system solver using Lapack.

*/

#ifndef VSIP_OPT_LAPACK_QR_HPP
#define VSIP_OPT_LAPACK_QR_HPP

#if VSIP_IMPL_REF_IMPL
# error "vsip/opt files cannot be used as part of the reference impl."
#endif

/***********************************************************************
  Included Files
***********************************************************************/

#include <algorithm>

#include <vsip/support.hpp>
#include <vsip/matrix.hpp>
#include <vsip/core/math_enum.hpp>
#include <vsip/opt/lapack/bindings.hpp>
#include <vsip/core/temp_buffer.hpp>
#include <vsip/core/working_view.hpp>
#include <vsip/core/solver/common.hpp>



/***********************************************************************
  Declarations
***********************************************************************/

namespace vsip
{

namespace impl
{

// Specialize Is_lud_impl_avail to indicate what types Lapack QR
// solver supports.  It supports all BLAS types.

template <typename T>
struct Is_qrd_impl_avail<Lapack_tag, T>
{
  static bool const value = blas::Blas_traits<T>::valid;
};



/// QR decomposition implementation class.  Common functionality for
/// qrd by-value and by-reference classes.

template <typename T,
	  bool     Blocked>
class Qrd_impl<T, Blocked, Lapack_tag>
  : Compile_time_assert<blas::Blas_traits<T>::valid>
{
  // BLAS/LAPACK require complex data to be in interleaved format.
  typedef Layout<2, col2_type, Stride_unit_dense, Cmplx_inter_fmt> data_LP;
  typedef Fast_block<2, T, data_LP> data_block_type;

  // Qrd types supported.
protected:
  static bool const supports_qrd_saveq1  = true;
  static bool const supports_qrd_saveq   = true;
  static bool const supports_qrd_nosaveq = true;

  // Constructors, copies, assignments, and destructors.
public:
  Qrd_impl(length_type, length_type, storage_type)
    VSIP_THROW((std::bad_alloc));
  Qrd_impl(Qrd_impl const&)
    VSIP_THROW((std::bad_alloc));

  Qrd_impl& operator=(Qrd_impl const&) VSIP_NOTHROW;
  ~Qrd_impl() VSIP_NOTHROW;

  // Accessors.
public:
  length_type  rows()     const VSIP_NOTHROW { return m_; }
  length_type  columns()  const VSIP_NOTHROW { return n_; }
  storage_type qstorage() const VSIP_NOTHROW { return st_; }

  // Solve systems.
public:
  template <typename Block>
  bool decompose(Matrix<T, Block>) VSIP_NOTHROW;

protected:
  template <mat_op_type       tr,
	    product_side_type ps,
	    typename          Block0,
	    typename          Block1>
  bool impl_prodq(const_Matrix<T, Block0>, Matrix<T, Block1>)
    VSIP_NOTHROW;

  template <mat_op_type       tr,
	    typename          Block0,
	    typename          Block1>
  bool impl_rsol(const_Matrix<T, Block0>, T const, Matrix<T, Block1>)
    VSIP_NOTHROW;

  template <typename          Block0,
	    typename          Block1>
  bool impl_covsol(const_Matrix<T, Block0>, Matrix<T, Block1>)
    VSIP_NOTHROW;

  template <typename          Block0,
	    typename          Block1>
  bool impl_lsqsol(const_Matrix<T, Block0>, Matrix<T, Block1>)
    VSIP_NOTHROW;

  // Member data.
private:
  typedef std::vector<T, Aligned_allocator<T> > vector_type;

  length_type  m_;			// Number of rows.
  length_type  n_;			// Number of cols.
  storage_type st_;			// Q storage type

  Matrix<T, data_block_type> data_;	// Factorized QR matrix
  vector_type tau_;			// Additional info on Q
  length_type geqrf_lwork_;		// size of workspace needed for geqrf
  vector_type geqrf_work_;		// workspace for geqrf
};



/***********************************************************************
  Definitions
***********************************************************************/

template <typename T,
	  bool     Blocked>
Qrd_impl<T, Blocked, Lapack_tag>::Qrd_impl(
  length_type  rows,
  length_type  cols,
  storage_type st
  )
VSIP_THROW((std::bad_alloc))
  : m_          (rows),
    n_          (cols),
    st_         (st),
    data_       (m_, n_),
    tau_        (n_),
    geqrf_lwork_(n_ * lapack::geqrf_blksize<T>(m_, n_)),
    geqrf_work_ (geqrf_lwork_)
{
  assert(m_ > 0 && n_ > 0 && m_ >= n_);
  assert(st_ == qrd_nosaveq || st_ == qrd_saveq || st_ == qrd_saveq1);
}



template <typename T,
	  bool     Blocked>
Qrd_impl<T, Blocked, Lapack_tag>::Qrd_impl(Qrd_impl const& qr)
VSIP_THROW((std::bad_alloc))
  : m_          (qr.m_),
    n_          (qr.n_),
    st_         (qr.st_),
    data_       (m_, n_),
    tau_        (n_),
    geqrf_lwork_(qr.geqrf_lwork_),
    geqrf_work_ (geqrf_lwork_)
{
  data_ = qr.data_;
  for (index_type i=0; i<n_; ++i)
    tau_[i] = qr.tau_[i];
}



template <typename T,
	  bool     Blocked>
Qrd_impl<T, Blocked, Lapack_tag>::~Qrd_impl()
  VSIP_NOTHROW
{
}



/// Decompose matrix M into QR form.
///
/// Requires
///   M to be a full rank, modifiable matrix of ROWS x COLS.

template <typename T,
	  bool     Blocked>
template <typename Block>
bool
Qrd_impl<T, Blocked, Lapack_tag>::decompose(Matrix<T, Block> m)
  VSIP_NOTHROW
{
  assert(m.size(0) == m_ && m.size(1) == n_);

  int lwork   = geqrf_lwork_;

  assign_local(data_, m);

  Ext_data<data_block_type> ext(data_.block());

  // lda is m_.
  if (Blocked)
     lapack::geqrf(m_, n_, ext.data(), m_, &tau_[0], &geqrf_work_[0], lwork);
  else
     lapack::geqr2(m_, n_, ext.data(), m_, &tau_[0], &geqrf_work_[0], lwork);

  // FLOPS:
  // scalar : (4/3) n^3          if m == n
  //        : (2/3) n^2 (3m - n) if m > n
  //        : (2/3) m^2 (3n - m) if m < n
  // complex: 4*
  

  assert((length_type)lwork <= geqrf_lwork_);

  return true;
}



/// Compute product of Q and b
///
/// qstoarge   | ps        | tr         | product | b (in) | x (out)
/// qrd_saveq1 | mat_lside | mat_ntrans | Q b     | (n, p) | (m, p)
/// qrd_saveq1 | mat_lside | mat_trans  | Q' b    | (m, p) | (n, p)
/// qrd_saveq1 | mat_lside | mat_herm   | Q* b    | (m, p) | (n, p)
///
/// qrd_saveq1 | mat_rside | mat_ntrans | b Q     | (p, m) | (p, n)
/// qrd_saveq1 | mat_rside | mat_trans  | b Q'    | (p, n) | (p, m)
/// qrd_saveq1 | mat_rside | mat_herm   | b Q*    | (p, n) | (p, m)
///
/// qrd_saveq  | mat_lside | mat_ntrans | Q b     | (m, p) | (m, p)
/// qrd_saveq  | mat_lside | mat_trans  | Q' b    | (m, p) | (m, p)
/// qrd_saveq  | mat_lside | mat_herm   | Q* b    | (m, p) | (m, p)
///
/// qrd_saveq  | mat_rside | mat_ntrans | b Q     | (p, m) | (p, m)
/// qrd_saveq  | mat_rside | mat_trans  | b Q'    | (p, m) | (p, m)
/// qrd_saveq  | mat_rside | mat_herm   | b Q*    | (p, m) | (p, m)

template <typename T,
	  bool     Blocked>
template <mat_op_type       tr,
	  product_side_type ps,
	  typename          Block0,
	  typename          Block1>
bool
Qrd_impl<T, Blocked, Lapack_tag>::impl_prodq(
  const_Matrix<T, Block0> b,
  Matrix<T, Block1>       x)
  VSIP_NOTHROW
{
  assert(this->qstorage() == qrd_saveq1 || this->qstorage() == qrd_saveq);

  char        side;
  char        trans;
  length_type q_rows;
  length_type q_cols;
  length_type k_reflectors = n_;
  int         mqr_lwork;

  if (qstorage() == qrd_saveq1)
  {
    q_rows = m_;
    q_cols = n_;
  }
  else // (qstorage() == qrd_saveq1)
  {
    q_rows = m_;
    q_cols = m_;
  }

  if (tr == mat_trans)
  {
    trans = 't';
    std::swap(q_rows, q_cols);
  }
  else if (tr == mat_herm)
  {
    trans = 'c';
    std::swap(q_rows, q_cols);
  }
  else // if (tr == mat_ntrans)
  {
    trans = 'n';
  }
  
  if (ps == mat_lside)
  {
    assert(b.size(0) == q_cols);
    assert(x.size(0) == q_rows);
    assert(b.size(1) == x.size(1));
    side = 'l';
    mqr_lwork = b.size(1);
  }
  else // (ps == mat_rside)
  {
    assert(b.size(1) == q_rows);
    assert(x.size(1) == q_cols);
    assert(b.size(0) == x.size(0));
    side = 'r';
    mqr_lwork = b.size(0);
  }

  Matrix<T, data_block_type> b_int(b.size(0), b.size(1));
  assign_local(b_int, b);

  int blksize   = lapack::mqr_blksize<T>(side, trans,
					 b.size(0), b.size(1), k_reflectors);
  mqr_lwork *= blksize;
  Temp_buffer<T> mqr_work(mqr_lwork);

  {
    Ext_data<data_block_type> b_ext(b_int.block());
    Ext_data<data_block_type> a_ext(data_.block());

    lapack::mqr(side,
		trans,
		b.size(0), b.size(1),
		k_reflectors,
		a_ext.data(), m_,
		&tau_[0],
		b_ext.data(), b.size(0),
		mqr_work.data(), mqr_lwork);
		
  }
  assign_local(x, b_int);

  return true;
}



/// Solve op(R) x = alpha b

template <typename T,
	  bool     Blocked>
template <mat_op_type tr,
	  typename    Block0,
	  typename    Block1>
bool
Qrd_impl<T, Blocked, Lapack_tag>::impl_rsol(
  const_Matrix<T, Block0> b,
  T const                 alpha,
  Matrix<T, Block1>       x)
  VSIP_NOTHROW
{
  assert(b.size(0) == n_);
  assert(b.size(0) == x.size(0));
  assert(b.size(1) == x.size(1));

  char trans;

  switch(tr)
  {
  case mat_trans:
    // assert(Is_scalar<T>::value);
    trans = 't';
    break;
  case mat_herm:
    // assert(Is_complex<T>::value);
    trans = 'c';
    break;
  default:
    trans = 'n';
    break;
  }

  Matrix<T, data_block_type> b_int(b.size(0), b.size(1));
  assign_local(b_int, b);
  

  {
    Ext_data<data_block_type> a_ext(data_.block());
    Ext_data<data_block_type> b_ext(b_int.block());
      
    blas::trsm('l',		// R appears on [l]eft-side
	       'u',		// R is [u]pper-triangular
	       trans,		// 
	       'n',		// R is [n]ot unit triangular
	       b.size(0), b.size(1),
	       alpha,
	       a_ext.data(), m_,
	       b_ext.data(), b_ext.stride(1));

    // FLOPS:
    // scalar : (4/3) n^3          if m == n
    //        : (2/3) n^2 (3m - n) if m > n
    //        : (2/3) m^2 (3n - m) if m < n
    // complex: 4*
  }
  assign_local(x, b_int);

  return true;
}



/// Solve covariance system for x:
///   A' A X = B

template <typename T,
	  bool     Blocked>
template <typename Block0,
	  typename Block1>
bool
Qrd_impl<T, Blocked, Lapack_tag>::impl_covsol(
  const_Matrix<T, Block0> b,
  Matrix<T, Block1>       x)
  VSIP_NOTHROW
{
  length_type b_rows = b.size(0);
  length_type b_cols = b.size(1);
  T alpha = T(1);

  assert(b_rows == n_);

  // Solve A' A x = b

  // Equiv to solve: R' R x = b
  // First solve:    R' b_1 = b
  // Then solve:     R x = b_1

  Matrix<T, data_block_type> b_int(b_rows, b_cols);
  assign_local(b_int, b);

  {
    Ext_data<data_block_type> b_ext(b_int.block());
    Ext_data<data_block_type> a_ext(data_.block());

    // First solve: R' b_1 = b

    blas::trsm('l',	// R' appears on [l]eft-side
	       'u',	// R is [u]pper-triangular
	       blas::Blas_traits<T>::trans, // [c]onj/[t]ranspose (conj(R'))
	       'n',	// R is [n]ot unit triangular
	       b_rows, b_cols,
	       alpha,
	       a_ext.data(), m_,
	       b_ext.data(), b_rows);

    // Then solve: R x = b_1
    
    blas::trsm('l',	// R appears on [l]eft-side
	       'u',	// R is [u]pper-triangular
	       'n',	// [n]o-op (R)
	       'n',	// R is [n]ot unit triangular
	       b_rows, b_cols,
	       alpha,
	       a_ext.data(), m_,
	       b_ext.data(), b_rows);
  }

  assign_local(x, b_int);

  return true;
}



/// Solve linear least squares problem for x:
///   min_x norm-2( A x - b )

template <typename T,
	  bool     Blocked>
template <typename          Block0,
	  typename          Block1>
bool
Qrd_impl<T, Blocked, Lapack_tag>::impl_lsqsol(
  const_Matrix<T, Block0> b,
  Matrix<T, Block1>       x)
  VSIP_NOTHROW
{
  length_type p = b.size(1);

  assert(b.size(0) == m_);
  assert(x.size(0) == n_);
  assert(x.size(1) == p);

  length_type c_rows = m_;
  length_type c_cols = p;
  
  int blksize   = lapack::mqr_blksize<T>('l', blas::Blas_traits<T>::trans,
					 c_rows, c_cols, n_);
  int mqr_lwork = c_cols*blksize;
  Temp_buffer<T> mqr_work(mqr_lwork);

  // Solve  A X = B  for X
  //
  // 0. factor:             QR X = B
  //    mult by Q'        Q'QR X = Q'B
  //    simplify             R X = Q'B
  //
  // 1. compute C = Q'B:     R X = C
  // 2. solve for X:         R X = C

  Matrix<T, data_block_type> c(c_rows, c_cols);
  assign_local(c, b);

  {
    Ext_data<data_block_type> c_ext(c.block());
    Ext_data<data_block_type> a_ext(data_.block());

    // 1. compute C = Q'B:     R X = C

    lapack::mqr('l',				// Q' on [l]eft (C = Q' B)
		blas::Blas_traits<T>::trans,	// [t]ranspose (Q')
		c_rows, c_cols, 
		n_,			// No. elementary reflectors in Q
		a_ext.data(), m_,
		&tau_[0],
		c_ext.data(), c_rows,
		mqr_work.data(), mqr_lwork);
		
    // 2. solve for X:         R X = C
    //      R (n, n)
    //      X (n, p)
    //      C (m, p)
    // Since R is (n, n), we treat C as an (n, p) matrix.

    blas::trsm('l',	// R appears on [l]eft-side
	       'u',	// R is [u]pper-triangular
	       'n',	// [n]o op (R)
	       'n',	// R is [n]ot unit triangular
	       n_, c_cols,
	       T(1),
	       a_ext.data(), m_,
	       c_ext.data(), c_rows);
  }

  assign_local(x, c(Domain<2>(n_, p)));

  return true;
}

} // namespace vsip::impl

} // namespace vsip


#endif // VSIP_OPT_LAPACK_QR_HPP
