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
	IOState(const OOBase::String& fname);
	IOState(const OOBase::String& entity_name, const OOBase::String& repl_text);
	~IOState();

	void init(OOBase::String& strEncoding, bool& standalone);
	void init();

	unsigned char next_char();
	bool is_eof() const;
	void rappend(const OOBase::String& str);
	void push(unsigned char c);

	OOBase::String m_fname;
	size_t         m_col;
	size_t         m_line;
	IOState*       m_next;
	bool           m_auto_pop;

private:
	IOState(const IOState&);
	IOState& operator = (const IOState&);

	// These are the private members used by Ragel
	int           m_cs;
	unsigned char m_char;

	IOState& operator += (int)
	{
		m_char = next_char();
		return *this;
	}

	IOState& operator ++ (int)
	{
		m_char = next_char();
		return *this;
	}

	struct EndOfFile
	{
		int unused;
	};

	bool operator == (const EndOfFile&) const
	{
		return (m_io == NULL || m_io->is_eof());
	}

	unsigned char operator * () const
	{
		return m_char;
	}

	void set_decoder(Decoder::eType type);
	void set_encoding(Token& token, OOBase::String& str);
	void init(bool entity, OOBase::String& strEncoding, bool& standalone);
	unsigned char get_char(bool& from_input);
	void switch_encoding(OOBase::String& strEncoding);

	Decoder*       m_decoder;
	Decoder::eType m_decoder_type;
	IO*            m_io;
	bool           m_eof;
	Token          m_input;
};

#endif // IOSTATE_H_INCLUDED_
