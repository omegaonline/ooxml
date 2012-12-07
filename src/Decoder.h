///////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2012 Rick Taylor
//
// This file is part of OOXML, the Omega Online XML library.
//
// OOXML is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OOXML is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with OOXML.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef DECODER_H_INCLUDED_
#define DECODER_H_INCLUDED_

#include <OOBase/GlobalNew.h>

class Decoder
{
public:
	enum eType
	{
		None = 0,
		UTF32LE,
		UTF32BE,
		UTF16LE,
		UTF16BE,
		EBCDIC
	};

	static Decoder* create(OOBase::AllocatorInstance& allocator, eType type);

	template <typename T>
	static void destroy(OOBase::AllocatorInstance& allocator, T*& p)
	{
		if (p)
		{
			p->~T();
			allocator.free(p);
			p = NULL;
		}
	}

	virtual unsigned char next(unsigned char c, bool& again) = 0;

protected:
	virtual ~Decoder() {};
	Decoder() {}

private:
	Decoder(const Decoder&);
	Decoder& operator =(const Decoder&);

	template <typename T>
	static T* create_i(OOBase::AllocatorInstance& allocator)
	{
		void* p = allocator.allocate(sizeof(T),OOBase::alignof<T>::value);
		if (!p)
			throw "Out of memory";

		return ::new (p) T();
	}
};


#endif /* DECODER_H_INCLUDED_ */
