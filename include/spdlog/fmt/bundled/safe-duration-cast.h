/*
 * For conversion between std::chrono::durations without undefined
 * behaviour or erroneous results.
 * This is a stripped down version of duration_cast, for inclusion in fmt.
 * See https://github.com/pauldreik/safe_duration_cast
 *
 * Copyright Paul Dreik 2019
 *
 * This file is licensed under the fmt license, see format.h
 */

#include <chrono>
#include <cmath>
#include <limits>
#include <type_traits>

#include "format.h"

FMT_BEGIN_NAMESPACE

namespace safe_duration_cast {

template <typename To, typename From,
          FMT_ENABLE_IF(!std::is_same<From, To>::value &&
                        std::numeric_limits<From>::is_signed ==
                            std::numeric_limits<To>::is_signed)>
FMT_CONSTEXPR To lossless_integral_conversion(const From from, int& ec) {
  ec = 0;
  using F = std::numeric_limits<From>;
  using T = std::numeric_limits<To>;
  static_assert(F::is_integer, "From must be integral");
  static_assert(T::is_integer, "To must be integral");

  // A and B are both signed, or both unsigned.
  if (F::digits <= T::digits) {
    // From fits in To without any problem.
  } else {
    // From does not always fit in To, resort to a dynamic check.
    if (from < T::min() || from > T::max()) {
      // outside range.
      ec = 1;
      return {};
    }
  }
  return static_cast<To>(from);
}

/**
 * converts From to To, without loss. If the dynamic value of from
 * can't be converted to To without loss, ec is set.
 */
template <typename To, typename From,
          FMT_ENABLE_IF(!std::is_same<From, To>::value &&
                        std::numeric_limits<From>::is_signed !=
                            std::numeric_limits<To>::is_signed)>
FMT_CONSTEXPR To lossless_integral_conversion(const From from, int& ec) {
  ec = 0;
  using F = std::numeric_limits<From>;
  using T = std::numeric_limits<To>;
  static_assert(F::is_integer, "From must be integral");
  static_assert(T::is_integer, "To must be integral");

  if (F::is_signed && !T::is_signed) {
    // From may be negative, not allowed!
    if (from < 0) {
      ec = 1;
      return {};
    }

    // From is positive. Can it always fit in To?
    if (F::digits <= T::digits) {
      // yes, From always fits in To.
    } else {
      // from may not fit in To, we have to do a dynamic check
      if (from > static_cast<From>(T::max())) {
        ec = 1;
        return {};
      }
    }
  }

  if (!F::is_signed && T::is_signed) {
    // can from be held in To?
    if (F::digits < T::digits) {
      // yes, From always fits in To.
    } else {
      // from may not fit in To, we have to do a dynamic check
      if (from > static_cast<From>(T::max())) {
        // outside range.
        ec = 1;
        return {};
      }
    }
  }

  // reaching here means all is ok for lossless conversion.
  return static_cast<To>(from);

}  // function

template <typename To, typename From,
          FMT_ENABLE_IF(std::is_same<From, To>::value)>
FMT_CONSTEXPR To lossless_integral_conversion(const From from, int& ec) {
  ec = 0;
  return from;
}  // function

// clang-format off
/**
 * converts From to To if possible, otherwise ec is set.
 *
 * input                            |    output
 * ---------------------------------|---------------
 * NaN                              | NaN
 * Inf                              | Inf
 * normal, fits in output           | converted (possibly lossy)
 * normal, does not fit in output   | ec is set
 * subnormal                        | best effort
 * -Inf                             | -Inf
 */
// clang-format on
template <typename To, typename From,
          FMT_ENABLE_IF(!std::is_same<From, To>::value)>
FMT_CONSTEXPR To safe_float_conversion(const From from, int& ec) {
  ec = 0;
  using T = std::numeric_limits<To>;
  static_assert(std::is_floating_point<From>::value, "From must be floating");
  static_assert(std::is_floating_point<To>::value, "To must be floating");

  // catch the only happy case
  if (std::isfinite(from)) {
    if (from >= T::lowest() && from <= T::max()) {
      return static_cast<To>(from);
    }
    // not within range.
    ec = 1;
    return {};
  }

  // nan and inf will be preserved
  return static_cast<To>(from);
}  // function

template <typename To, typename From,
          FMT_ENABLE_IF(std::is_same<From, To>::value)>
FMT_CONSTEXPR To safe_float_conversion(const From from, int& ec) {
  ec = 0;
  static_assert(std::is_floating_point<From>::value, "From must be floating");
  return from;
}

/**
 * safe duration cast between integral durations
 */
template <typename To, typename FromRep, typename FromPeriod,
          FMT_ENABLE_IF(std::is_integral<FromRep>::value),
          FMT_ENABLE_IF(std::is_integral<typename To::rep>::value)>
To safe_duration_cast(std::chrono::duration<FromRep, FromPeriod> from,
                      int& ec) {
  using From = std::chrono::duration<FromRep, FromPeriod>;
  ec = 0;
  // the basic idea is that we need to convert from count() in the from type
  // to count() in the To type, by multiplying it with this:
  using Factor = std::ratio_divide<typename From::period, typename To::period>;

  static_assert(Factor::num > 0, "num must be positive");
  static_assert(Factor::den > 0, "den must be positive");

  // the conversion is like this: multiply from.count() with Factor::num
  // /Factor::den and convert it to To::rep, all this without
  // overflow/underflow. let's start by finding a suitable type that can hold
  // both To, From and Factor::num
  using IntermediateRep =
      typename std::common_type<typename From::rep, typename To::rep,
                                decltype(Factor::num)>::type;

  // safe conversion to IntermediateRep
  IntermediateRep count =
      lossless_integral_conversion<IntermediateRep>(from.count(), ec);
  if (ec) {
    return {};
  }
  // multiply with Factor::num without overflow or underflow
  if (Factor::num != 1) {
    constexpr auto max1 =
        std::numeric_limits<IntermediateRep>::max() / Factor::num;
    if (count > max1) {
      ec = 1;
      return {};
    }
    constexpr auto min1 =
        std::numeric_limits<IntermediateRep>::min() / Factor::num;
    if (count < min1) {
      ec = 1;
      return {};
    }
    count *= Factor::num;
  }

  // this can't go wrong, right? den>0 is checked earlier.
  if (Factor::den != 1) {
    count /= Factor::den;
  }
  // convert to the to type, safely
  using ToRep = typename To::rep;
  const ToRep tocount = lossless_integral_conversion<ToRep>(count, ec);
  if (ec) {
    return {};
  }
  return To{tocount};
}

/**
 * safe duration_cast between floating point durations
 */
template <typename To, typename FromRep, typename FromPeriod,
          FMT_ENABLE_IF(std::is_floating_point<FromRep>::value),
          FMT_ENABLE_IF(std::is_floating_point<typename To::rep>::value)>
To safe_duration_cast(std::chrono::duration<FromRep, FromPeriod> from,
                      int& ec) {
  using From = std::chrono::duration<FromRep, FromPeriod>;
  ec = 0;
  if (std::isnan(from.count())) {
    // nan in, gives nan out. easy.
    return To{std::numeric_limits<typename To::rep>::quiet_NaN()};
  }
  // maybe we should also check if from is denormal, and decide what to do about
  // it.

  // +-inf should be preserved.
  if (std::isinf(from.count())) {
    return To{from.count()};
  }

  // the basic idea is that we need to convert from count() in the from type
  // to count() in the To type, by multiplying it with this:
  using Factor = std::ratio_divide<typename From::period, typename To::period>;

  static_assert(Factor::num > 0, "num must be positive");
  static_assert(Factor::den > 0, "den must be positive");

  // the conversion is like this: multiply from.count() with Factor::num
  // /Factor::den and convert it to To::rep, all this without
  // overflow/underflow. let's start by finding a suitable type that can hold
  // both To, From and Factor::num
  using IntermediateRep =
      typename std::common_type<typename From::rep, typename To::rep,
                                decltype(Factor::num)>::type;

  // force conversion of From::rep -> IntermediateRep to be safe,
  // even if it will never happen be narrowing in this context.
  IntermediateRep count =
      safe_float_conversion<IntermediateRep>(from.count(), ec);
  if (ec) {
    return {};
  }

  // multiply with Factor::num without overflow or underflow
  if (Factor::num != 1) {
    constexpr auto max1 = std::numeric_limits<IntermediateRep>::max() /
                          static_cast<IntermediateRep>(Factor::num);
    if (count > max1) {
      ec = 1;
      return {};
    }
    constexpr auto min1 = std::numeric_limits<IntermediateRep>::lowest() /
                          static_cast<IntermediateRep>(Factor::num);
    if (count < min1) {
      ec = 1;
      return {};
    }
    count *= static_cast<IntermediateRep>(Factor::num);
  }

  // this can't go wrong, right? den>0 is checked earlier.
  if (Factor::den != 1) {
    using common_t = typename std::common_type<IntermediateRep, intmax_t>::type;
    count /= static_cast<common_t>(Factor::den);
  }

  // convert to the to type, safely
  using ToRep = typename To::rep;

  const ToRep tocount = safe_float_conversion<ToRep>(count, ec);
  if (ec) {
    return {};
  }
  return To{tocount};
}

}  // namespace safe_duration_cast

FMT_END_NAMESPACE
