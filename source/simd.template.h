//
// SPDX-License-Identifier: MIT
// Copyright (C) 2020 - 2022 by the ryujin authors
//

#pragma once

#include "simd.h"

DEAL_II_DISABLE_EXTRA_DIAGNOSTICS
#if DEAL_II_COMPILER_VECTORIZATION_LEVEL >= 1 && defined(__SSE2__)
#define VCL_NAMESPACE vcl
#include "../simd-math/vectorclass.h"
#include "../simd-math/vectormath_exp.h"
#undef VCL_NAMESPACE
#endif
DEAL_II_ENABLE_EXTRA_DIAGNOSTICS

namespace ryujin
{
  /*
   * pow implementation for scalar float and double:
   */

#if DEAL_II_COMPILER_VECTORIZATION_LEVEL >= 1 && defined(__SSE2__) &&          \
    defined(WITH_CUSTOM_POW)

  template <>
  // DEAL_II_ALWAYS_INLINE inline
  float pow(const float x, const float b)
  {
    /* Use a custom pow implementation instead of std::pow(): */
    return vcl::pow(vcl::Vec4f(x), b).extract(0);
  }


  template <>
  // DEAL_II_ALWAYS_INLINE inline
  double pow(const double x, const double b)
  {
    /* Use a custom pow implementation instead of std::pow(): */
    return vcl::pow(vcl::Vec2d(x), b).extract(0);
  }

#else

  template <>
  // DEAL_II_ALWAYS_INLINE inline
  float pow(const float x, const float b)
  {
    // Call generic std::pow() implementation
    return std::pow(x, b);
  }


  template <>
  // DEAL_II_ALWAYS_INLINE inline
  double pow(const double x, const double b)
  {
    // Call generic std::pow() implementation
    return std::pow(x, b);
  }
#endif


  namespace
  {
    /*
     * A type trait to select the correct VCL type:
     */
    template <typename T, std::size_t width>
    struct VectorClassType {
    };

#if DEAL_II_COMPILER_VECTORIZATION_LEVEL >= 3 && defined(__AVX512F__)
    template <>
    struct VectorClassType<float, 16> {
      using value_type = vcl::Vec16f;
    };

    template <>
    struct VectorClassType<double, 8> {
      using value_type = vcl::Vec8d;
    };
#endif

#if DEAL_II_COMPILER_VECTORIZATION_LEVEL >= 2 && defined(__AVX__)
    template <>
    struct VectorClassType<float, 8> {
      using value_type = vcl::Vec8f;
    };

    template <>
    struct VectorClassType<double, 4> {
      using value_type = vcl::Vec4d;
    };
#endif

#if DEAL_II_COMPILER_VECTORIZATION_LEVEL >= 1 && defined(__SSE2__)
    template <>
    struct VectorClassType<float, 4> {
      using value_type = vcl::Vec4f;
    };

    template <>
    struct VectorClassType<double, 2> {
      using value_type = vcl::Vec2d;
    };
#endif
  } // namespace

  /*
   * Convert a dealii::VectorizedArray to a VCL container type:
   */
  template <typename T, std::size_t width>
  DEAL_II_ALWAYS_INLINE inline typename VectorClassType<T, width>::value_type
  to_vcl(const dealii::VectorizedArray<T, width> x)
  {
    return typename VectorClassType<T, width>::value_type(x.data);
  }


  /*
   * Convert a VCL container type to a dealii::VectorizedArray:
   */
  template <typename T, std::size_t width>
  DEAL_II_ALWAYS_INLINE inline dealii::VectorizedArray<T, width>
  from_vcl(typename VectorClassType<T, width>::value_type x)
  {
    dealii::VectorizedArray<T, width> result;
    result.data = x;
    return result;
  }


  /*
   * Specialized pow implementation for VectorizedArray:
   */

  template <typename T, std::size_t width>
  // DEAL_II_ALWAYS_INLINE inline
  dealii::VectorizedArray<T, width>
  pow(const dealii::VectorizedArray<T, width> x, const T b)
  {
    return from_vcl<T, width>(vcl::pow(to_vcl(x), b));
  }


  template <typename T, std::size_t width>
  // DEAL_II_ALWAYS_INLINE inline
  dealii::VectorizedArray<T, width>
  pow(const dealii::VectorizedArray<T, width> x,
      const dealii::VectorizedArray<T, width> b)
  {
    return from_vcl<T, width>(vcl::pow(to_vcl(x), to_vcl(b)));
  }

  namespace
  {
    template <typename T, std::size_t width>
    struct FC {
    };

    template <std::size_t width>
    struct FC<double, width> {

      // There is no Vec2f, so make sure to use at least Vec4f...
      static constexpr std::size_t float_width = (width == 2 ? 4 : width);

      static DEAL_II_ALWAYS_INLINE inline
      typename VectorClassType<float, float_width>::value_type
      to_float(typename VectorClassType<double, width>::value_type x)
      {
        return vcl::to_float(x);
      }
      static DEAL_II_ALWAYS_INLINE inline
      typename VectorClassType<double, width>::value_type
      to_double(typename VectorClassType<float, float_width>::value_type x)
      {
        if constexpr (width == 2) {
          const vcl::Vec4d temp = vcl::to_double(x);
          return vcl::Vec2d(temp.extract(0), temp.extract(1));
        } else {
          return vcl::to_double(x);
        }
      }
    };

    template <std::size_t width>
    struct FC<float, width> {
      static DEAL_II_ALWAYS_INLINE inline
      typename VectorClassType<float, width>::value_type
      to_float(typename VectorClassType<float, width>::value_type x)
      {
        return x;
      }
      static DEAL_II_ALWAYS_INLINE inline
      typename VectorClassType<float, width>::value_type
      to_double(typename VectorClassType<float, width>::value_type x)
      {
        return x;
      }
    };
  } // namespace

  /*
   * The "fast_pow" family of approximate pow() functions
   */

  template <Bias B, typename T>
  // DEAL_II_ALWAYS_INLINE inline
  T fast_pow(const T x, const T b)
  {
    return ryujin::pow(float(x), float(b));
  }


  template <Bias B, typename T, std::size_t width>
  // DEAL_II_ALWAYS_INLINE inline
  dealii::VectorizedArray<T, width>
  fast_pow(const dealii::VectorizedArray<T, width> x, const T b)
  {
    return from_vcl<T, width>(FC<T, width>::to_double(
        vcl::pow(FC<T, width>::to_float(to_vcl(x)), float(b))));
  }


  template <Bias B, typename T, std::size_t width>
  // DEAL_II_ALWAYS_INLINE inline
  dealii::VectorizedArray<T, width>
  fast_pow(const dealii::VectorizedArray<T, width> x,
      const dealii::VectorizedArray<T, width> b)
  {
    return from_vcl<T, width>(FC<T, width>::to_double(vcl::pow(
        FC<T, width>::to_float(to_vcl(x)), FC<T, width>::to_float(to_vcl(b)))));
  }


} // namespace ryujin
