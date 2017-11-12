
#pragma once

#include <iostream>
#include <functional>

namespace IP
{

typedef std::basic_ifstream< char, std::char_traits< char > > IFStream;
typedef std::basic_ofstream< char, std::char_traits< char > > OFStream;
typedef std::basic_fstream< char, std::char_traits< char > > FStream;
typedef std::basic_istream< char, std::char_traits< char > > IStream;
typedef std::basic_ostream< char, std::char_traits< char > > OStream;
typedef std::basic_iostream< char, std::char_traits< char > > IOStream;
typedef std::istreambuf_iterator< char, std::char_traits< char > > IStreamBufIterator;

} // namespace IP