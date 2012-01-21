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

#include <OOBase/GlobalNew.h>
#include <OOBase/String.h>
#include <OOBase/HashTable.h>

#include "Token.h"
#include "Decoder.h"
#include "IO.h"

class IOState
{
public:
	IOState(const char* fname);
	~IOState();

	unsigned char get_char();
	bool is_eof() const;
	static void pop(IOState*& io);

	OOBase::String m_fname;
	size_t         m_col;
	size_t         m_line;
	Decoder*       m_decoder;
	IOState*       m_next;

private:
	IOState(const IOState&);
	IOState& operator = (const IOState&);

	IO*            m_io;
};

// Callbacks

OOBase::String resolve_url(const OOBase::String& strBase, const OOBase::String& strPublicId, const OOBase::String& strSystemId);

class Tokenizer
{
public:
	Tokenizer();
	~Tokenizer();
	
	void load(const char* fname);

	enum TokenType
	{
		Error = 0,
		End = 1,
		DocumentStandalone,
		DocumentVersion,
		DocumentEncoding,
		DocTypeStart,
		DocTypeEnd,
		ElementStart,
		ElementEnd,
		AttributeName,
		AttributeValue,
		Text,
		PiTarget,
		PiData,
		Comment,
		CData,
	};

	TokenType next_token(OOBase::String& strToken);

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

	IOState* m_io;

	OOBase::HashTable<OOBase::String,OOBase::String> m_int_gen_entities;
	OOBase::HashTable<OOBase::String,OOBase::String> m_int_param_entities;

	struct External
	{
		OOBase::String m_strPublicId;
		OOBase::String m_strSystemId;
		OOBase::String m_strNData;
	};
	OOBase::HashTable<OOBase::String,External> m_ext_gen_entities;
	OOBase::HashTable<OOBase::String,External> m_ext_param_entities;
		
	// These are the private members used by Ragel
	Tokenizer& operator ++ ()
	{ 
		next_char();
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

	bool operator == (bool pe) const
	{
		return (pe || m_io == NULL || m_io->is_eof());
	}
		
	unsigned char operator * () const
	{ 
		return m_char; 
	}
	
	bool encoding(OOBase::String& str);
	void decoder(Decoder::eType type);

	void pre_push();

	void external_doctype();
	bool do_doctype();

	void do_init();

	unsigned char next_char_i();
	void next_char();
	void set_token(OOBase::String& str, size_t offset = 0);
	
	void general_entity();
	void param_entity();
	bool subst_entity();
	void subst_char(int base);
};

#endif // TOKENIZER_H_INCLUDED_
