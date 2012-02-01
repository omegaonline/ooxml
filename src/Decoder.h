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

	static Decoder* create(eType type);

	virtual unsigned char next(unsigned char c, bool& again) = 0;

protected:
	Decoder() {}

private:
	Decoder(const Decoder&);
	Decoder& operator =(const Decoder&);
};


#endif /* DECODER_H_INCLUDED_ */
