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

#include "Token.h"

class Tokenizer
{
public:
	Tokenizer();
	~Tokenizer();
	
	void init();
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
	
	void encoding(const char*)
	{}

	void pre_push();
		
	void do_init();
	bool do_exec();
	
	unsigned char get_char();
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
