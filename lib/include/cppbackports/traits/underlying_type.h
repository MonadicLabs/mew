/* Copyright (c) 2016, Pollard Banknote Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef PBL_CPP_TRAITS_UNDERLYING_TYPE_H
#define PBL_CPP_TRAITS_UNDERLYING_TYPE_H

#ifndef CPP11
#include "is_enum.h"

namespace cpp11
{
namespace details
{
template< typename, bool B >
struct underlying_type_helper
{
	typedef unsigned long type;
};

template< typename E >
struct underlying_type_helper< E, true >
{
	typedef long type;
};
}

template< typename E, typename Enable = void >
struct underlying_type;

template< typename E >
struct underlying_type< E, typename enable_if< is_enum< E >::value >::type >
{
	typedef typename details::underlying_type_helper < E, static_cast< E >( -1 ) < static_cast< E > (0) > ::type type;
};

}

#else
#ifndef CPP14
namespace cpp14
{
template< class T >
using underlying_type_t = typename std::underlying_type< T >::type;
}
#endif
#endif // ifndef CPP11
#endif // PBL_CPP_TRAITS_UNDERLYING_TYPE_H