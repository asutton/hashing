// Copyright (c) 2016 Andrew Sutton
// All rights reserved

#include "hashing.hpp"

#include <iostream>
#include <iomanip>


using namespace origin;


std::ostream&
operator<<(std::ostream& os, std::vector<byte> const& buf)
{
  os << std::hex << std::setfill('0');
  unsigned int n = 0;
  for (byte c : buf)
  {
      os << std::setw(2) << (unsigned)c << ' ';
      if (++n == 16)
      {
          os << '\n';
          n = 0;
      }
  }
  os << '\n';
  os << std::dec << std::setfill(' ');
  return os;
}


// Hash for a vector.
template<Hash_algorithm H, Hashable_with<H> T, typename A>
void
hash_append(H& h, std::vector<T, A> const& v)
{
  hash_append(h, v.begin(), v.end());
}


// Hash for a string.
template<Hash_algorithm H, Hashable_with<H> C, typename T, typename A>
void
hash_append(H& h, std::basic_string<C, T, A> const& s)
{
  hash_append(h, s.begin(), s.end());
}


int
main()
{
  debug_hasher h;

  hash_append(h, 5);
  hash_append(h, true);
  hash_append(h, 3.1415);

  std::vector<int> v {1, 2, 3};
  hash_append(h, v);

  std::string s = "abcdef";
  hash_append(h, s);

  // NOTE: Currently broken because of an apparent bug in template
  // argument deduction, possibly caused by substitutingn through a
  // concept.
  // std::vector<std::string> vs { "hello", "goodbye" };
  // hash_append(h, vs);

  std::cout << h.value() << '\n';
}
