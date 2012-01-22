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

#ifndef IO_H_
#define IO_H_

// This is a dummy
#include <stdlib.h>
#include <stdio.h>

class IO
{
public:
	IO();
	~IO();

	int open(const char* fname);
	unsigned char get_char();
	bool is_eof() const;

private:
	IO(const IO&);
	IO& operator = (const IO&);

	FILE* m_f;
	bool m_eof;
};

#endif /* IO_H_ */
