/*
 * IO.cpp
 *
 *  Created on: 19 Jan 2012
 *      Author: taylorr
 */

#include "IO.h"

#include <errno.h>

IO::IO() : m_f(NULL)
{ }

IO::~IO()
{
	if (m_f)
		fclose(m_f);
}

int IO::open(const char* fname)
{
	m_f = fopen(fname,"rb");
	return errno;
}

unsigned char IO::get_char()
{
	unsigned char c = '\0';
	if (m_f)
		fread(&c,sizeof(c),1,m_f);
	return c;
}
