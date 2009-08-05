/* Copyright (c) 2006, 2008 by CodeSourcery.  All rights reserved.

   This file is available for license from CodeSourcery, Inc. under the terms
   of a commercial license and under the GPL.  It is not part of the VSIPL++
   reference implementation and is not available under the BSD license.
*/
/** @file    vsip/opt/sal/qr.hpp
    @author  Assem Salama
    @date    2006-04-17
    @brief   VSIPL++ Library: QR linear system solver using SAL.

*/

#ifndef VSIP_IMPL_SAL_SOLVER_QR_HPP
#define VSIP_IMPL_SAL_SOLVER_QR_HPP

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
#include <vsip/core/temp_buffer.hpp>
#include <vsip/core/working_view.hpp>
#include <vsip/core/fns_elementwise.hpp>
#include <vsip/core/solver/common.hpp>
#include <sal.h>


/***********************************************************************
  Declarations
***********************************************************************/

namespace vsip
{
namespace impl
{

// SAL QR decomposition
#define VSIP_IMPL_SAL_QR_DEC( T, SAL_T, SALFCN )	\
inline void						\
sal_mat_qr_dec(						\
  T* a, int tcols_a,					\
  T* r, int tcols_r,					\
  int m, int n)						\
{							\
  SALFCN((SAL_T*)a,tcols_a,				\
         (SAL_T*)r,tcols_r,				\
	 m,n);						\
}

#define VSIP_IMPL_SAL_QR_DEC_SPLIT( T, SAL_T, SALFCN )	\
inline void						\
sal_mat_qr_dec(						\
  std::pair<T*,T*> const& a, int tcols_a,		\
  std::pair<T*,T*> const& r, int tcols_r,		\
  int m, int n)						\
{							\
  SALFCN((SAL_T*)&a,tcols_a,				\
         (SAL_T*)&r,tcols_r,				\
	 m,n);						\
}

VSIP_IMPL_SAL_QR_DEC      (float,          float,          matmgs_dqr)
VSIP_IMPL_SAL_QR_DEC      (complex<float>, COMPLEX,       cmatmgs_dqr)
VSIP_IMPL_SAL_QR_DEC_SPLIT(float,          COMPLEX_SPLIT, zmatmgs_dqr)

#undef VSIP_IMPL_SAL_QR_DEC
#undef VSIP_IMPL_SAL_QR_DEC_SPLIT

// SAL QR r solver
#define VSIP_IMPL_SAL_QR_RSOL( T, SAL_T, SALFCN )	\
inline void						\
sal_mat_qr_rsol(					\
  T* r, int tcols_r,					\
  T* b, T* x,						\
  int n)						\
{							\
  SALFCN((SAL_T*)r,tcols_r,				\
         (SAL_T*)b,					\
	 (SAL_T*)x,n);					\
}

#define VSIP_IMPL_SAL_QR_RSOL_SPLIT( T, SAL_T, SALFCN )	\
inline void						\
sal_mat_qr_rsol(					\
  std::pair<T*,T*> const& r, int tcols_r,		\
  std::pair<T*,T*> const& b,				\
  std::pair<T*,T*> const& x,				\
  int n)						\
{							\
  SALFCN((SAL_T*)&r, tcols_r,				\
         (SAL_T*)&b,					\
	 (SAL_T*)&x, n);				\
}

VSIP_IMPL_SAL_QR_RSOL      (float,          float,          matmgs_sr)
VSIP_IMPL_SAL_QR_RSOL      (complex<float>, COMPLEX,       cmatmgs_sr)
VSIP_IMPL_SAL_QR_RSOL_SPLIT(float,          COMPLEX_SPLIT, zmatmgs_sr)

#undef VSIP_IMPL_SAL_QR_RSOL
#undef VSIP_IMPL_SAL_QR_RSOL_SPLIT

// SAL QR rhr solver
#define VSIP_IMPL_SAL_QR_RHSOL( T, SAL_T, SALFCN )	\
inline void						\
sal_mat_qr_rhsol(					\
  T* r, int tcols_r,					\
  T* b, T* x,						\
  int n)						\
{							\
  SALFCN((SAL_T*)r,tcols_r,				\
         (SAL_T*)b,					\
	 (SAL_T*)x,n);					\
}

// SAL QR rhr solver
#define VSIP_IMPL_SAL_QR_RHSOL_SPLIT( T, SAL_T, SALFCN )\
inline void						\
sal_mat_qr_rhsol(					\
  std::pair<T*,T*> const& r,				\
  int tcols_r,						\
  std::pair<T*,T*> const& b,				\
  std::pair<T*,T*> const& x,				\
  int n)						\
{							\
  SALFCN((SAL_T*)&r, tcols_r,				\
         (SAL_T*)&b,					\
	 (SAL_T*)&x, n);				\
}

VSIP_IMPL_SAL_QR_RHSOL      (float,          float,          matmgs_srhr)
VSIP_IMPL_SAL_QR_RHSOL      (complex<float>, COMPLEX,       cmatmgs_srhr)
VSIP_IMPL_SAL_QR_RHSOL_SPLIT(float,          COMPLEX_SPLIT, zmatmgs_srhr)

#undef VSIP_IMPL_SAL_QR_RHSOL
#undef VSIP_IMPL_SAL_QR_RHSOL_SPLIT

// A structure that tells us if sal qr impl is available
// for certain types
template <>
struct Is_qrd_impl_avail<Mercury_sal_tag, float>
{
  static bool const value = true;
};

template <>
struct Is_qrd_impl_avail<Mercury_sal_tag, complex<float> >
{
  static bool const value = true;
};



/// Qrd implementation using Mercury SAL library.

/// Requires:
///   T to be a value type supported by SAL's QR routines
///   BLOCKED is not used (it is used by the Lapack QR implementation
///      class).

template <typename T,
	  bool     Blocked>
class Qrd_impl<T, Blocked, Mercury_sal_tag>
{
  // SAL input matrix must be in ROW major form. Sal supports both interleaved
  // and split complex formats
  typedef vsip::impl::dense_complex_type   complex_type;
  typedef Storage<complex_type, T>         cp_storage_type;
  typedef typename cp_storage_type::type   ptr_type;

  typedef Layout<2, row2_type, Stride_unit_dense, complex_type> data_LP;
  typedef Fast_block<2, T, data_LP> data_block_type;

  typedef Layout<2, col2_type, Stride_unit_dense, complex_type> t_data_LP;
  typedef Fast_block<2, T, t_data_LP> t_data_block_type;

  // Qrd types supported.
protected:
  static bool const supports_qrd_saveq1  = true;
  static bool const supports_qrd_saveq   = false;
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

  Matrix<T, t_data_block_type> data_;	// Factorized QR(mxn) matrix
  Matrix<T, t_data_block_type> t_data_;	// Factorized QR(mxn) matrix transposed
  Matrix<T, data_block_type> r_data_;	// Factorized R(nxn) matrix
  Matrix<T, data_block_type> rt_data_;	// Factorized R(nxn) matrix transposed
};



/***********************************************************************
  Definitions
***********************************************************************/

template <typename T,
	  bool     Blocked>
Qrd_impl<T, Blocked, Mercury_sal_tag>::Qrd_impl(
  length_type  rows,
  length_type  cols,
  storage_type st
  )
VSIP_THROW((std::bad_alloc))
  : m_          (rows),
    n_          (cols),
    st_         (st),
    data_       (m_, n_),
    t_data_     (n_, m_),
    r_data_     (n_, n_),
    rt_data_    (n_, n_)
{
  assert(m_ > 0 && n_ > 0 && m_ >= n_);
  assert(st_ == qrd_nosaveq || st_ == qrd_saveq || st_ == qrd_saveq1);

  // SAL only provides a thin-QR decomposition.
  if (st_ == qrd_saveq)
    VSIP_IMPL_THROW(impl::unimplemented(
	      "Qrd does not support full storage of Q (qrd_saveq) when using SAL"));
}



template <typename T,
	  bool     Blocked>
Qrd_impl<T, Blocked, Mercury_sal_tag>::Qrd_impl(Qrd_impl const& qr)
VSIP_THROW((std::bad_alloc))
  : m_          (qr.m_),
    n_          (qr.n_),
    st_         (qr.st_),
    data_       (m_, n_),
    t_data_     (n_, m_),
    r_data_     (n_, n_),
    rt_data_    (n_, n_)
{
  data_ = qr.data_;
}



template <typename T,
	  bool     Blocked>
Qrd_impl<T, Blocked, Mercury_sal_tag>::~Qrd_impl()
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
Qrd_impl<T, Blocked, Mercury_sal_tag>::decompose(Matrix<T, Block> m)
  VSIP_NOTHROW
{
  assert(m.size(0) == m_ && m.size(1) == n_);

  assign_local(data_, m);

  Ext_data<t_data_block_type> ext(data_.block());
  Ext_data<data_block_type> r_ext(r_data_.block());

  sal_mat_qr_dec(ext.data(), m_,
                 r_ext.data(), n_, 
		 m_, n_);

  return true;
}



/// Compute product of Q and b
/// 
/// If qstorage == qrd_saveq1, Q is MxN.
/// If qstorage == qrd_saveq,  Q is MxM.
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
Qrd_impl<T, Blocked, Mercury_sal_tag>::impl_prodq(
  const_Matrix<T, Block0> b,
  Matrix<T, Block1>       x)
  VSIP_NOTHROW
{
  assert(this->qstorage() == qrd_saveq1 || this->qstorage() == qrd_saveq);

  length_type q_rows;
  length_type q_cols;

  if (qstorage() == qrd_saveq1)
  {
    q_rows = m_;
    q_cols = n_;
  }
  else // (qstorage() == qrd_saveq1)
  {
    assert(0);
    q_rows = m_;
    q_cols = m_;
  }

  // do we need a transpose?
  if(tr == mat_trans || tr == mat_herm) 
  {
    t_data_ = data_.transpose();
    std::swap(q_rows, q_cols);
  }
  if(tr == mat_herm) 
  {
    t_data_ = impl::impl_conj(t_data_);
    std::swap(q_rows, q_cols);
  }

  // left or right?
  if(ps == mat_lside) 
  {
    assert(b.size(0) == q_cols);
    assert(x.size(0) == q_rows);
    assert(b.size(1) == x.size(1));

    generic_prod(tr == mat_trans || tr == mat_herm ? t_data_ : data_, b, x);
  }
  else
  {
    assert(b.size(1) == q_rows);
    assert(x.size(1) == q_cols);
    assert(b.size(0) == x.size(0));

    generic_prod(b, tr == mat_trans || tr == mat_herm ? t_data_ : data_, x);
  }

  return true;
}



/// Solve op(R) x = alpha b

template <typename T,
	  bool     Blocked>
template <mat_op_type tr,
	  typename    Block0,
	  typename    Block1>
bool
Qrd_impl<T, Blocked, Mercury_sal_tag>::impl_rsol(
  const_Matrix<T, Block0> b,
  T const                 alpha,
  Matrix<T, Block1>       x)
  VSIP_NOTHROW
{
  assert(b.size(0) == n_);
  assert(b.size(0) == x.size(0));
  assert(b.size(1) == x.size(1));

  Matrix<T, t_data_block_type> b_int(b.size(0), b.size(1));
  Matrix<T, t_data_block_type> x_int(x.size(0), x.size(1));

  if(tr == mat_ntrans)
  {
    assign_local(b_int, b);

    // multiply b by alpha
    b_int *= alpha;

    {
      Ext_data<t_data_block_type> b_ext(b_int.block());
      Ext_data<t_data_block_type> x_ext(x_int.block());
      Ext_data<data_block_type>   r_ext(r_data_.block());

      ptr_type b_ptr = b_ext.data();
      ptr_type x_ptr = x_ext.data();
      
      for(length_type i=0;i < b.size(1);i++)
      {
	sal_mat_qr_rsol(r_ext.data(), n_,
			cp_storage_type::offset(b_ptr,i*n_),
			cp_storage_type::offset(x_ptr,i*n_),
			n_);
      }
    }
    assign_local(x,x_int);
  }
  else
  {
    rt_data_ = r_data_(Domain<2>(Domain<1>(n_-1, -1, n_),
				 Domain<1>(n_-1, -1, n_)));

    Domain<2> flip(Domain<1>(b.size(0)-1, -1, b.size(0)),
		   Domain<1>(b.size(1)-1, -1, b.size(1)));

    assign_local(b_int, b(flip));

    // multiply b by alpha
    b_int *= alpha;

    if(tr == mat_herm) rt_data_ = impl::impl_conj(rt_data_);

    {
      Ext_data<t_data_block_type> b_ext(b_int.block());
      Ext_data<t_data_block_type> x_ext(x_int.block());
      Ext_data<data_block_type>   r_ext(rt_data_.block());

      ptr_type b_ptr = b_ext.data();
      ptr_type x_ptr = x_ext.data();

      // It turns out if I want to solve R'x=b, I need to read everything
      // backwards! That is why I'm remapping the matrixes using negative
      // strides.

      for(length_type i=0;i < b.size(1);i++)
      {
        sal_mat_qr_rsol(r_ext.data(), n_,
		      cp_storage_type::offset(b_ptr,i*n_),
		      cp_storage_type::offset(x_ptr,i*n_),
		      n_);
      }
    }

    // X is now backwards too. Have to flip it around!
    assign_local(x, x_int(flip));
  }
  
  return true;
}



/// Solve covariance system for x:
///   A' A X = B

template <typename T,
	  bool     Blocked>
template <typename Block0,
	  typename Block1>
bool
Qrd_impl<T, Blocked, Mercury_sal_tag>::
impl_covsol(const_Matrix<T, Block0> b, Matrix<T, Block1> x)
  VSIP_NOTHROW
{
  Matrix<T, t_data_block_type> b_int(b.size(0), b.size(1));
  Matrix<T, t_data_block_type> x_int(x.size(0), x.size(1));

  assign_local(b_int, b);

  {
    Ext_data<t_data_block_type> b_ext(b_int.block());
    Ext_data<t_data_block_type> x_ext(x_int.block());
    Ext_data<data_block_type>   r_ext(r_data_.block());

    ptr_type b_ptr = b_ext.data();
    ptr_type x_ptr = x_ext.data();

    // Because SAL only wants x and b as vectors, we have to look at each
    // column.
    for(length_type i=0;i<b.size(1);i++)
    {
      sal_mat_qr_rhsol(r_ext.data(), n_,
		      cp_storage_type::offset(b_ptr,i*n_),
		      cp_storage_type::offset(x_ptr,i*n_),
		      n_);
    }
  }

  assign_local(x, x_int);

  return true;
}



/// Solve linear least squares problem for x:
///   min_x norm-2( A x - b )

template <typename T,
	  bool     Blocked>
template <typename Block0,
	  typename Block1>
bool
Qrd_impl<T, Blocked, Mercury_sal_tag>::
impl_lsqsol(const_Matrix<T, Block0> b, Matrix<T, Block1> x)
  VSIP_NOTHROW
{
  length_type p = b.size(1);

  assert(b.size(0) == m_);
  assert(x.size(0) == n_);
  assert(x.size(1) == p);
 
  Matrix<T, t_data_block_type> x_int(x.size(0), x.size(1));

  // C will be Q'b, Q' is nxm, so, c is nxb.size(1)
  Matrix<T, t_data_block_type> c_int(n_, b.size(1));



  // Solve  A X = B  for X
  //
  // 0. factor:             QR X = B
  //    mult by Q'        Q'QR X = Q'B
  //    simplify             R X = Q'B
  //
  // 1. compute C = Q'B:     R X = C
  // 2. solve for X:         R X = C

  t_data_ = data_.transpose();
  if (Is_complex<T>::value) t_data_ = impl::impl_conj(t_data_);
  generic_prod(t_data_,b,c_int);

  assign_local(x_int, x);

  // Ok, now, solve Rx=C
  {
    Ext_data<t_data_block_type> x_ext(x_int.block());
    Ext_data<t_data_block_type> c_ext(c_int.block());
    Ext_data<data_block_type>   r_ext(r_data_.block());

    ptr_type c_ptr = c_ext.data();
    ptr_type x_ptr = x_ext.data();

    // Because SAL only wants x and b as vectors, we have to look at each
    // column.
    for(length_type i=0;i<x.size(1);i++)
    {
      sal_mat_qr_rsol(r_ext.data(), n_,
		      cp_storage_type::offset(c_ptr,i*n_),
		      cp_storage_type::offset(x_ptr,i*n_),
		      n_);
    }
  }

  assign_local(x,x_int);

  return true;
}

} // namespace vsip::impl
} // namespace vsip

#endif // VSIP_IMPL_SAL_SOLVER_QR_HPP
