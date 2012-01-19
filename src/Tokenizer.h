///////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2011 Rick Taylor
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

#ifndef TOKENIZER_H_INCLUDED_
#define TOKENIZER_H_INCLUDED_

#include <OOBase/String.h>

#include "Token.h"
#include "Decoder.h"
#include "IO.h"

class Tokenizer
{
public:
	Tokenizer();
	~Tokenizer();
	
	void load(const char* fname);
	void next_token();

private:
	// Ragel members
	int    m_cs;
	int*   m_stack;
	size_t m_top;
	
	size_t        m_stacksize;
	unsigned char m_char;

	Token m_input;
	Token m_token;
	Token m_entity_name;
	Token m_entity;
	Token m_system;
	Token m_public;

	struct IOInfo
	{
		OOBase::String m_fname;
		size_t         m_col;
		size_t         m_line;
		IO*            m_io;
		Decoder*       m_decoder;
		IOInfo*        m_prev;
	};
	IOInfo* m_io;
		
	// These are the private members used by Ragel
	Tokenizer& operator ++ ()
	{ 
		next_char();
		return *this;
	}
		
	bool operator == (unsigned char c) const 
	{ 
		return (m_char == c); 
	}
		
	bool operator != (unsigned char c) const 
	{ 
		return (m_char != c); 
	}
		
	unsigned char operator * () const
	{ 
		return m_char; 
	}
	
	struct IOInfo* new_io(const char* fname);

	void encoding();
	void decoder(Decoder::eType type);

	void pre_push();

	void external_doctype();
	bool do_doctype();
	void end_doctype();
		
	void do_init();
	bool do_exec();
	
	unsigned char next_char_i();
	void next_char();
	void token(const char* s, size_t offset = 0);
	
	void general_entity();
	void param_entity();
	void subst_entity();
	void subst_char();
	void subst_hex();
};

#endif // TOKENIZER_H_INCLUDED_
