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

#include "Token.h"

#include <OOBase/Memory.h>

Token::Token() :
		m_buffer(NULL),
		m_alloc(0),
		m_len(0)
{
}

Token::~Token()
{
	OOBase::HeapAllocator::free(m_buffer);
}

bool Token::empty() const
{
	return (m_len == 0);
}

unsigned char Token::pop()
{
	unsigned char r = '\0';
	if (m_len && m_buffer)
		r = m_buffer[--m_len];

	return r;
}

void Token::push(unsigned char c)
{
	if (!m_buffer)
	{
		m_buffer = static_cast<unsigned char*>(OOBase::HeapAllocator::allocate(32));
		if (m_buffer)
			m_alloc = 32;
	}
	else if (m_len + 1 > m_alloc)
	{
		unsigned char* new_buffer = static_cast<unsigned char*>(OOBase::HeapAllocator::reallocate(m_buffer,m_alloc * 2));
		if (new_buffer)
		{
			m_alloc *= 2;
			m_buffer = new_buffer;
		}
	}

	m_buffer[m_len++] = c;
}

void Token::clear()
{
	m_len = 0;
}

unsigned char* Token::copy(size_t& len)
{
	len = m_len;
	m_len = 0;
	return m_buffer;
}
