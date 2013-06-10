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

#include <OOBase/Memory.h>

#include "Token.h"
#include "Decoder.h"
#include "IO.h"

class IOState
{
public:
	static IOState* create(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& fname, unsigned int version = (unsigned int)-1);
	static IOState* create(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& entity_name, unsigned int version, const OOBase::LocalString& repl_text);

	void destroy();

	void init(OOBase::LocalString& strEncoding, bool& standalone);
	void init();

	unsigned char next_char();
	bool is_eof() const;
	void rappend(const OOBase::LocalString& str);
	void push(unsigned char c);
	unsigned int get_version();
	bool is_file() const;

	OOBase::LocalString m_fname;
	size_t              m_col;
	size_t              m_line;
	IOState*            m_next;
	bool                m_auto_pop;

private:
	IOState(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& fname, unsigned int version);
	IOState(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& entity_name, unsigned int version, const OOBase::LocalString& repl_text);

	~IOState();

	IOState(const IOState&);
	IOState& operator = (const IOState&);

	// These are the private members used by Ragel
	int           m_cs;
	unsigned char m_char;

	OOBase::AllocatorInstance& m_allocator;

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

	unsigned char operator * () const
	{
		return m_char;
	}

	void set_decoder(Decoder::eType type);
	void set_encoding(Token& token, OOBase::LocalString& str);
	void init(bool entity, OOBase::LocalString& strEncoding, bool& standalone);
	unsigned char get_char(bool& from_input);
	void switch_encoding(OOBase::LocalString& strEncoding);
	void set_version(Token& token);

	Decoder*       m_decoder;
	Decoder::eType m_decoder_type;
	IO*            m_io;
	bool           m_eof;
	bool           m_preinit;
	Token          m_input;
	unsigned int   m_version;
};

#endif // IOSTATE_H_INCLUDED_
