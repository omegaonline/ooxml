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

#include "IO.h"

#include <errno.h>

IO::IO() : m_f(NULL), m_eof(true)
{ }

IO::~IO()
{
	if (m_f)
		fclose(m_f);
}

int IO::open(const char* fname)
{
	int err = 0;
	m_f = fopen(fname,"rb");
	if (m_f)
		m_eof = (feof(m_f) != 0);
	else
		err = errno;

	return err;
}

bool IO::is_eof() const
{
	return m_eof;
}

unsigned char IO::get_char()
{
	unsigned char c = '\0';
	if (m_f)
	{
		size_t r = fread(&c,sizeof(c),1,m_f);
		if (r == 0)
			m_eof = true;
		else if (r == (size_t)-1)
			throw "IO Error";
	}
	return c;
}
