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

#ifndef TOKEN_H_INCLUDED_
#define TOKEN_H_INCLUDED_

#include <OOBase/GlobalNew.h>
#include <OOBase/String.h>

class Token
{
public:
	Token();
	~Token();

	bool empty() const;

	void push(unsigned char c);

	unsigned char pop();
	const char* pop(size_t& len);
	void pop(OOBase::String& str);

	void clear();

	void rappend(const char* sz);

private:
	Token(const Token&);
	Token& operator = (const Token&);

	unsigned char* m_buffer;
	size_t         m_alloc;
	size_t         m_len;
};

#endif // TOKEN_H_INCLUDED_
