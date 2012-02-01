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

#ifndef IOSTATE_H_INCLUDED_
#define IOSTATE_H_INCLUDED_

#include <OOBase/GlobalNew.h>

#include "Token.h"
#include "Decoder.h"
#include "IO.h"

class IOState
{
public:
	IOState(const OOBase::String& fname, bool pre_space);
	IOState(const OOBase::String& entity_name, const OOBase::String& repl_text);
	~IOState();

	unsigned char next_char();
	bool is_eof() const;
	void rappend(const OOBase::String& str);
	void push(unsigned char c);
	void clear_decoder();

	OOBase::String m_fname;
	size_t         m_col;
	size_t         m_line;
	IOState*       m_next;
	bool           m_auto_pop;

private:
	IOState(const IOState&);
	IOState& operator = (const IOState&);

	unsigned char get_char();

	Decoder*       m_decoder;
	IO*            m_io;
	bool           m_eof;
	Token          m_input;
	Token          m_bom_input;
};

#endif // IOSTATE_H_INCLUDED_
