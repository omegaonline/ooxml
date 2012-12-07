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
	Token(OOBase::AllocatorInstance& allocator);
	~Token();

	bool empty() const;

	void push(unsigned char c);
	void push(const char* sz);

	unsigned char pop();
	const char* pop(size_t& len);
	OOBase::LocalString pop_string();

	void clear();

	void rappend(const OOBase::LocalString& str);

private:
	Token(const Token&);
	Token& operator = (const Token&);

	OOBase::AllocatorInstance& m_allocator;

	unsigned char* m_buffer;
	size_t         m_alloc;
	size_t         m_len;
};

#endif // TOKEN_H_INCLUDED_
