/*
 * IO.cpp
 *
 *  Created on: 19 Jan 2012
 *      Author: taylorr
 */

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
	m_f = fopen(fname,"rb");
	if (m_f)
		m_eof = (feof(m_f) != 0);

	return errno;
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
		if (fread(&c,sizeof(c),1,m_f) != 1)
			m_eof = (feof(m_f) != 0);
	}
	return c;
}
