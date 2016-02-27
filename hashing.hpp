// Copyright (c) 2016 Andrew Sutton
// All rights reserved

#ifndef ORIGIN_HASHING_HPP
#define ORIGIN_HASHING_HPP

// An implementation Howard Hinnant's "Types don't know #" proposal [1],
// with the following differences:
//
//  - A hash algorithm returns its "value" by calling value(). Forcing
//    an explicit conversion seemed a bit heavy-handed for extracting
//    the hash value.
//
//  - The contiguously_hashable trait is re-conceived as trivially
//    comparable since that seems to entail the latter. EoP would
//    probably call this "uniquely represented".
//
//  - Added iterator overloads of hash_append.
//
//  - Some of the concept names have changed a bit.
//
// [1] http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html

#include <origin/generic.hpp>
#include <origin/functional.hpp>
#include <origin/iterator.hpp>

#include <vector>


namespace origin
{

// -------------------------------------------------------------------------- //
// Memory and objects

// A byte is the fundamental unit of storage.
using byte = unsigned char;


// Returns the underlying storage of an object.
template<typename T>
byte const* storage(T const& t)
{
  return std::addressof(t);
}


// -------------------------------------------------------------------------- //
// Trivially comparable types

template<typename T> struct trivially_comparable;


// A type alias for the trivially_comparable trait.
template<typename T>
using trivially_comparable_t = typename trivially_comparable<T>::type;


// A constant value for the trivially_comparable trait.
template<typename T>
constexpr bool trivially_comparable_v = trivially_comparable_t<T>::value;


// Returns true if the type T is trivially comparable.
//
// NOTE: Floating point types are not trivially comparable since
// there are multiple representations of 0.0.
template<typename T>
struct trivially_comparable : std::false_type { };

// All arithmetic types are contiguously hashable.
template<origin::Integral_type T>
struct trivially_comparable<T> : std::true_type { };

// All pointer types are contiguously hashable.
template<typename T>
struct trivially_comparable<T*> : std::true_type { };

// All array types are contiguously hashable if their underlying
// type is contiguously hashable.
//
// NOTE: We dont' really use this determination for the
// hash_append functions.
template<typename T, std::size_t N>
struct trivially_comparable<T[N]>
  : trivially_comparable_t<std::remove_extent_t<T>>
{ };

template<typename T>
struct trivially_comparable<T[]>
  : trivially_comparable_t<std::remove_extent_t<T>>
{ };


// Determines if T is trivially comparable.
template<typename T>
concept bool Trivially_comparable()
{
  return trivially_comparable_v<T>;
}


// -------------------------------------------------------------------------- //
// Hash algorithms


// A hash halgorithm is a function that can take a sequence of
// bytes as an argument. Every hash halgorithm has a result type
// to which it can be implicitly converted.
template<typename H>
concept bool
Hash_algorithm()
{
  return Copyable<H>() && requires(H h, void const* p, std::size_t n) {
    // A sequence of bytes can be appended to the function.
    h(p, n);

    // A hash value can be retrieved from the object. The
    // result type is unspecified.
    h.value();
  };
}


// The debug hasher simply records the bytes appended by each
// hash function to an underlying byte sequence (i.e., vector).
//
// FIXME: Move this into a sub-library.
struct debug_hasher
{
  using value_type = std::vector<byte>;

  void operator()(void const* key, std::size_t len) noexcept
  {
    byte const* buf = static_cast<byte const*>(key);
    buf_.insert(buf_.end(), buf, buf + len);
  }

  value_type value()
  {
    return buf_;
  }

  std::vector<byte> buf_;
};


// -------------------------------------------------------------------------- //
// Hash append

// Hash append for trivially comparable T.
//
// This is only defined for scalar types.
template<Hash_algorithm H, Trivially_comparable T>
inline void
hash_append(H& h, T const& t)
{
  h(std::addressof(t), sizeof(t));
}


// Hash append for floating point T.
//
// Guarantee that 0 values have the same hash code.
template<Hash_algorithm H, origin::Floating_point_type T>
inline void
hash_append(H& h, T t)
{
  if (t == 0)
    t = 0;
  h(std::addressof(t), sizeof(t));
}


// hash_append for arrays of trivially comparable T.
template<Hash_algorithm H, Trivially_comparable T, std::size_t N>
inline void
hash_append(H& h, T const (&a)[N])
{
  h(a, sizeof(a));
}


// A type is hashable with a hash algorith iff hash_append
// is valid for those arguments. The hashed value type T is
// given as the first template argument which allows the concept
// to be used like this:
//
//    template<Hash_algorithm H, Hashable_with<H> T>
//    void
//    hash_append(H&, /* some type involving T */);
//
template<typename T, typename H>
concept bool
Hashable_with()
{
  return Hash_algorithm<H>() && requires(H& h, T const& t) {
    hash_append(h, t);
  };
}


// Just flips the types of the concept above.
template<typename H, typename T>
concept bool
Hashable_type()
{
  return Hashable_with<T, H>();
}


// hash_append for arrays of non-trivially comparable type.
template<Hash_algorithm H, Hashable_with<H> T, std::size_t N>
inline void
hash_append(H& h, T const (&a)[N])
{
  for (T const& x : a)
    hash_append(h, a);
}


// Variadic hash_append.
template<Hash_algorithm H, Hashable_with<H> T0, Hashable_with<H> T1, Hashable_with<H>... Ts>
inline void
hash_append(H& h, T0 const& t0, T1 const& t1, Ts const&... ts)
{
  hash_append(h, t0);
  hash_append(h, t1, ts...);
}


// Itertor range hash_append.
template<Hash_algorithm H, Forward_iterator I>
  requires Hashable_type<H, Value_type<I>>()
inline void
hash_append(H& h, I first, I last)
{
  while (first != last) {
    hash_append(h, *first);
    ++first;
  }
}


// Optimization for trivially comparable types.
template<Hash_algorithm H, Trivially_comparable T>
inline void
hash_append(H& h, T const* first, T const* last)
{
  h(first, last - first);
}


// -------------------------------------------------------------------------- //
// Universal hash functions

// The universal hash function produces a hash value for objects
// that can be hashed with that algorithm.
template<Hash_algorithm H>
struct hash
{
  using result_type = Result_type<H>;

  template<Hashable_with<H> T>
  result_type operator()(T const& t) const noexcept
  {
      H hasher;
      hash_append(hasher, t);
      return hasher;
  }
};



} // namespace origin



#endif
