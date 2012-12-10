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

class IOState;

// Callbacks

OOBase::LocalString resolve_url(const OOBase::LocalString& strBase, const OOBase::LocalString& strPublicId, const OOBase::LocalString& strSystemId);

class Tokenizer
{
public:
	Tokenizer(OOBase::AllocatorInstance& allocator);
	~Tokenizer();
	
	void load(const OOBase::LocalString& fname);

	enum TokenType
	{
		Error = 0,
		End = 1,
		DocTypeStart = 2,
		DocTypeEnd = 3,
		ElementStart = 4,
		ElementEnd = 5,
		AttributeName = 6,
		AttributeValue = 7,
		Text = 8,
		PiTarget = 9,
		PiData = 10,
		Comment = 11,
		CData = 12
	};

	TokenType next_token(OOBase::LocalString& strToken, int verbose = 0);
	size_t get_column() const;
	size_t get_line() const;
	OOBase::LocalString get_location() const;
	unsigned int get_version() const;

	OOBase::AllocatorInstance& get_allocator() const
	{
		return m_allocator;
	}

private:
	OOBase::AllocatorInstance& m_allocator;

	// Ragel members
	int    m_cs;
	int*   m_stack;
	size_t m_top;
	
	size_t        m_stacksize;
	unsigned char m_char;

	Token m_token;
	Token m_entity_name;
	Token m_entity;
	Token m_system;
	Token m_public;
	bool  m_internal_doctype;
	bool  m_standalone;
	OOBase::LocalString m_strEncoding;

	IOState* m_io;

	OOBase::HashTable<OOBase::LocalString,OOBase::LocalString,OOBase::AllocatorInstance> m_int_param_entities;

	struct InternalEntity
	{
		InternalEntity(const OOBase::LocalString& strValue, bool extern_decl) :
			m_strValue(strValue), m_extern_decl(extern_decl)
		{}

		OOBase::LocalString m_strValue;
		bool                m_extern_decl;
	};
	OOBase::HashTable<OOBase::LocalString,InternalEntity,OOBase::AllocatorInstance> m_int_gen_entities;

	struct ExternalEntity
	{
		ExternalEntity(const OOBase::LocalString& strPublicId, const OOBase::LocalString& strSystemId, const OOBase::LocalString& strNData) :
			m_strPublicId(strPublicId), m_strSystemId(strSystemId), m_strNData(strNData)
		{}

		ExternalEntity(const OOBase::LocalString& strPublicId, const OOBase::LocalString& strSystemId) :
			m_strPublicId(strPublicId), m_strSystemId(strSystemId), m_strNData(strSystemId.get_allocator())
		{}

		OOBase::LocalString m_strPublicId;
		OOBase::LocalString m_strSystemId;
		OOBase::LocalString m_strNData;
	};
	OOBase::HashTable<OOBase::LocalString,ExternalEntity,OOBase::AllocatorInstance> m_ext_gen_entities;
	OOBase::HashTable<OOBase::LocalString,ExternalEntity,OOBase::AllocatorInstance> m_ext_param_entities;
		
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

	bool operator == (const EndOfFile&) const;

	struct ParseState
	{
		OOBase::LocalString& m_strToken;
		TokenType       m_type;
		bool            m_halt;

		ParseState(OOBase::LocalString& t) :
			m_strToken(t),
			m_type(Tokenizer::Error),
			m_halt(false)
		{}
	};

	bool operator == (const ParseState& pe) const;
		
	unsigned char operator * () const
	{ 
		return m_char; 
	}
	
	void pre_push();

	void external_doctype();
	bool do_doctype();

	void do_init();

	void next_char();
	void set_token(ParseState& pe, enum TokenType type, size_t offset = 0, bool allow_empty = true);
	OOBase::LocalString get_external_fname() const;
	void bypass_entity();
	void check_entity_recurse(const OOBase::LocalString& strEnt);
	void general_entity();
	void param_entity();
	bool subst_attr_entity();
	bool subst_content_entity();
	bool subst_pentity();
	void include_pe(bool auto_pop);
	void io_pop();
	void subst_char(int base);
};

#endif // TOKENIZER_H_INCLUDED_
