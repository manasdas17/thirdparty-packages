/* Copyright (c) 2005 by CodeSourcery, LLC.  All rights reserved. */

/** @file    vsip/core/expr/binary_operators.hpp
    @author  Stefan Seefeld
    @date    2005-03-10
    @brief   VSIPL++ Library: Operators to be used with expression templates.

    This file declares Operators to be used with expression templates.
*/

#ifndef VSIP_CORE_EXPR_BINARY_OPERATORS_HPP
#define VSIP_CORE_EXPR_BINARY_OPERATORS_HPP

/***********************************************************************
  Included Files
***********************************************************************/

#include <vsip/core/refcount.hpp>
#include <vsip/core/expr/operations.hpp>
#include <vsip/core/expr/binary_block.hpp>
#include <vsip/core/expr/scalar_block.hpp>
#include <vsip/core/view_traits.hpp>
#include <vsip/core/promote.hpp>

namespace vsip
{
namespace impl
{
/***********************************************************************
 Overloaded Operators
***********************************************************************/

/// View_promotion allows the compiler to instantiate
/// binary operators for views that differ in their
/// constness only.
/// A 'type' member is only provided in the special case
/// where the two view template parameters are identical,
/// and thus template parameter substitution will fail
/// in general. 
/// This ('SFINAE') principle makes the compiler consider
/// the binary operator template only for matching view
/// types.
template <typename LView, typename RView>
struct View_promotion
{
  // No 'type' member here as LView and RView are incompatible in general.
};

/// Specialization for View_promotion with two identical
/// View class templates.
template <typename View>
struct View_promotion<View, View>
{
  typedef View type;
};

/// Binary_operator_return_type encapsulates the inference
/// of a binary operator's return type from its arguments.
template <template <typename, typename> class LView,
 	  typename LType,
	  typename LBlock,
 	  template <typename, typename> class RView,
 	  typename RType,
	  typename RBlock,
          template <typename, typename> class Operator,
	  typename LTag = typename Is_view_type<LView<LType, LBlock> >::type,
	  typename RTag = typename Is_view_type<RView<RType, RBlock> >::type>
struct Binary_operator_return_type
{
  typedef typename Promotion<LType, RType>::type value_type;

  typedef const Binary_expr_block<LView<LType, LBlock>::dim, Operator,
				LBlock, LType, RBlock, RType> block_type;


  // The following projects the two view types onto their respective
  // const equivalent and promotes these to a return type.
  // Thus, Binary_operator_return_type(const_Matrix, Matrix) -> const_Matrix
  // but Binary_operator_return_type(Vector, Matrix) -> failure.
  typedef typename 
  View_promotion<typename ViewConversion<LView,
					value_type,
					block_type>::const_view_type,
		typename ViewConversion<RView,
					value_type,
					block_type>::const_view_type>::type 
					view_type;
};

template <template <typename, typename> class View,
 	  typename Type,
	  typename Block,
	  typename Scalar>
inline
typename Binary_operator_return_type<View, Type, Block,
				  View, Scalar,
				  Scalar_block<View<Type, Block>::dim,
					      Scalar>,
				  op::Add>::view_type
operator+(View<Type, Block> const& lhs, Scalar rhs)
{
  typedef Binary_operator_return_type<View, Type, Block,
                                   View, Scalar,
				   Scalar_block<View<Type, Block>::dim,
					       Scalar>,
                                   op::Add> type_trait;

  static dimension_type const dim = View<Type, Block>::dim;
  typedef typename type_trait::block_type block_type;
  typedef Scalar_block<dim, Scalar> scalar_block_type;
  typedef typename type_trait::view_type view_type;

  return view_type(block_type(lhs.block(),
			      scalar_block_type(rhs)));
}

template <template <typename, typename> class View,
 	  typename Type,
	  typename Block,
	  typename Scalar>
inline
typename Binary_operator_return_type<View, Scalar,
				  Scalar_block<View<Type, Block>::dim,
					      Scalar>,
				  View, Type, Block,
				  op::Add>::view_type
operator+(Scalar lhs, View<Type, Block> const& rhs)
{
  typedef Binary_operator_return_type<View, Scalar,
				   Scalar_block<View<Type, Block>::dim,
					       Scalar>,
                                   View, Type, Block,
                                   op::Add> type_trait;

  static dimension_type const dim = View<Type, Block>::dim;
  typedef Scalar_block<dim, Scalar> scalar_block_type;
  typedef typename type_trait::block_type block_type;
  typedef typename type_trait::view_type view_type;
  return view_type(block_type(scalar_block_type(lhs),
			      rhs.block()));
}

template <template <typename, typename> class LView,
 	  typename LType,
	  typename LBlock,
 	  template <typename, typename> class RView,
 	  typename RType,
	  typename RBlock>
inline
typename Binary_operator_return_type<LView, LType, LBlock,
				  RView, RType, RBlock,
				  op::Add>::view_type
operator+(LView<LType, LBlock> const& lhs,
	  RView<RType, RBlock> const& rhs)
{
  typedef Binary_operator_return_type<LView, LType, LBlock,
                                   RView, RType, RBlock,
                                   op::Add> type_trait;

  typedef typename type_trait::block_type block_type;
  typedef typename type_trait::view_type view_type;
  return view_type(block_type(lhs.block(), rhs.block()));
}

template <template <typename, typename> class View,
 	  typename Type,
	  typename Block,
	  typename Scalar>
inline
typename Binary_operator_return_type<View, Type, Block,
				  View, Scalar,
				  Scalar_block<View<Type, Block>::dim,
					      Scalar>,
				  op::Sub>::view_type
operator-(View<Type, Block> const& lhs, Scalar rhs)
{
  typedef Binary_operator_return_type<View, Type, Block,
                                   View, Scalar,
				   Scalar_block<View<Type, Block>::dim,
					       Scalar>,
                                   op::Sub> type_trait;

  static dimension_type const dim = View<Type, Block>::dim;
  typedef typename type_trait::block_type block_type;
  typedef Scalar_block<dim, Scalar> scalar_block_type;
  typedef typename type_trait::view_type view_type;

  return view_type(block_type(lhs.block(),
			      scalar_block_type(rhs)));
}

template <template <typename, typename> class View,
 	  typename Type,
	  typename Block,
	  typename Scalar>
inline
typename Binary_operator_return_type<View, Scalar,
				  Scalar_block<View<Type, Block>::dim,
					      Scalar>,
				  View, Type, Block,
				  op::Sub>::view_type
operator-(Scalar lhs, View<Type, Block> const& rhs)
{
  typedef Binary_operator_return_type<View, Scalar,
				   Scalar_block<View<Type, Block>::dim,
					       Scalar>,
                                   View, Type, Block,
                                   op::Sub> type_trait;

  static dimension_type const dim = View<Type, Block>::dim;
  typedef Scalar_block<dim, Scalar> scalar_block_type;
  typedef typename type_trait::block_type block_type;
  typedef typename type_trait::view_type view_type;
  return view_type(block_type(scalar_block_type(lhs),
			      rhs.block()));
}

template <template <typename, typename> class LView,
 	  typename LType,
	  typename LBlock,
 	  template <typename, typename> class RView,
 	  typename RType,
	  typename RBlock>
inline
typename Binary_operator_return_type<LView, LType, LBlock,
				  RView, RType, RBlock,
				  op::Sub>::view_type
operator-(LView<LType, LBlock> const& lhs,
	  RView<RType, RBlock> const& rhs)
{
  typedef Binary_operator_return_type<LView, LType, LBlock,
                                   RView, RType, RBlock,
                                   op::Sub> type_trait;

  typedef typename type_trait::block_type block_type;
  typedef typename type_trait::view_type view_type;
  return view_type(block_type(lhs.block(), rhs.block()));
}

template <template <typename, typename> class View,
 	  typename Type,
	  typename Block,
	  typename Scalar>
inline
typename Binary_operator_return_type<View, Type, Block,
				  View, Scalar,
				  Scalar_block<View<Type, Block>::dim,
					      Scalar>,
				  op::Mult>::view_type
operator*(View<Type, Block> const& lhs, Scalar rhs)
{
  typedef Binary_operator_return_type<View, Type, Block,
                                   View, Scalar,
				   Scalar_block<View<Type, Block>::dim,
					       Scalar>,
                                   op::Mult> type_trait;

  static dimension_type const dim = View<Type, Block>::dim;
  typedef typename type_trait::block_type block_type;
  typedef Scalar_block<dim, Scalar> scalar_block_type;
  typedef typename type_trait::view_type view_type;

  return view_type(block_type(lhs.block(),
			      scalar_block_type(rhs)));
}

template <template <typename, typename> class View,
 	  typename Type,
	  typename Block,
	  typename Scalar>
inline
typename Binary_operator_return_type<View, Scalar,
				  Scalar_block<View<Type, Block>::dim,
					      Scalar>,
				  View, Type, Block,
				  op::Mult>::view_type
operator*(Scalar lhs, View<Type, Block> const& rhs)
{
  typedef Binary_operator_return_type<View, Scalar,
				   Scalar_block<View<Type, Block>::dim,
					       Scalar>,
                                   View, Type, Block,
                                   op::Mult> type_trait;

  static dimension_type const dim = View<Type, Block>::dim;
  typedef Scalar_block<dim, Scalar> scalar_block_type;
  typedef typename type_trait::block_type block_type;
  typedef typename type_trait::view_type view_type;
  return view_type(block_type(scalar_block_type(lhs),
			      rhs.block()));
}

template <template <typename, typename> class LView,
 	  typename LType,
	  typename LBlock,
 	  template <typename, typename> class RView,
 	  typename RType,
	  typename RBlock>
inline
typename Binary_operator_return_type<LView, LType, LBlock,
				  RView, RType, RBlock,
				  op::Mult>::view_type
operator*(LView<LType, LBlock> const& lhs,
	  RView<RType, RBlock> const& rhs)
{
  typedef Binary_operator_return_type<LView, LType, LBlock,
                                   RView, RType, RBlock,
                                   op::Mult> type_trait;

  typedef typename type_trait::block_type block_type;
  typedef typename type_trait::view_type view_type;
  return view_type(block_type(lhs.block(), rhs.block()));
}

template <template <typename, typename> class View,
 	  typename Type,
	  typename Block,
	  typename Scalar>
inline
typename Binary_operator_return_type<View, Type, Block,
				  View, Scalar,
				  Scalar_block<View<Type, Block>::dim,
					      Scalar>,
				  op::Div>::view_type
operator/(View<Type, Block> const& lhs, Scalar rhs)
{
  typedef Binary_operator_return_type<View, Type, Block,
                                   View, Scalar,
				   Scalar_block<View<Type, Block>::dim,
					       Scalar>,
                                   op::Div> type_trait;

  static dimension_type const dim = View<Type, Block>::dim;
  typedef typename type_trait::block_type block_type;
  typedef Scalar_block<dim, Scalar> scalar_block_type;
  typedef typename type_trait::view_type view_type;

  return view_type(block_type(lhs.block(),
			      scalar_block_type(rhs)));
}

template <template <typename, typename> class View,
 	  typename Type,
	  typename Block,
	  typename Scalar>
inline
typename Binary_operator_return_type<View, Scalar,
				  Scalar_block<View<Type, Block>::dim,
					      Scalar>,
				  View, Type, Block,
				  op::Div>::view_type
operator/(Scalar lhs, View<Type, Block> const& rhs)
{
  typedef Binary_operator_return_type<View, Scalar,
				   Scalar_block<View<Type, Block>::dim,
					       Scalar>,
                                   View, Type, Block,
                                   op::Div> type_trait;

  static dimension_type const dim = View<Type, Block>::dim;
  typedef Scalar_block<dim, Scalar> scalar_block_type;
  typedef typename type_trait::block_type block_type;
  typedef typename type_trait::view_type view_type;
  return view_type(block_type(scalar_block_type(lhs),
			      rhs.block()));
}

template <template <typename, typename> class LView,
 	  typename LType,
	  typename LBlock,
 	  template <typename, typename> class RView,
 	  typename RType,
	  typename RBlock>
inline
typename Binary_operator_return_type<LView, LType, LBlock,
				  RView, RType, RBlock,
				  op::Div>::view_type
operator/(LView<LType, LBlock> const& lhs,
	  RView<RType, RBlock> const& rhs)
{
  typedef Binary_operator_return_type<LView, LType, LBlock,
                                   RView, RType, RBlock,
                                   op::Div> type_trait;

  typedef typename type_trait::block_type block_type;
  typedef typename type_trait::view_type view_type;
  return view_type(block_type(lhs.block(), rhs.block()));
}


} // namespace vsip::impl
} // namespace vsip

#endif
