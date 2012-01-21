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

#include "Tokenizer.h"

IOState::IOState(const char* fname) :
		m_col(0),
		m_line(1),
		m_decoder(NULL),
		m_next(NULL),
		m_io(new (std::nothrow) IO())
{
	int err = m_fname.assign(fname);
	if (err != 0)
		throw "Out of memory";

	m_io->open(fname);
}

IOState::~IOState()
{
	delete m_decoder;
	delete m_io;
	delete m_next;
}

unsigned char IOState::get_char()
{
	unsigned char c = '\0';
	if (m_io)
	{
		bool again = false;
		do
		{
			c = m_io->get_char();

			if (m_decoder)
				c = m_decoder->next(c,again);
		}
		while(again);
	}

	return c;
}

bool IOState::is_eof() const
{
	return (m_io == NULL || m_io->is_eof());
}

void IOState::pop(IOState*& io)
{
	IOState* n = io;
	io = io->m_next;
	n->m_next = NULL;
	delete n;
}

Tokenizer::Tokenizer() : 
		m_stack(NULL), 
		m_stacksize(0),
		m_char('\0'),
		m_io(NULL)
{
	// Preload the predefined internal entities
	struct predef
	{
		const char* k;
		const char* v;
	};

	static const predef predefined[] =
	{
		{"lt", "&#38;#60;"},
		{"gt", "&#62;"},
		{"amp", "&#38;#38;"},
		{"apos", "&#39;"},
		{"quot", "&#34;"},
		{NULL,NULL}
	};

	for (const struct predef* p = predefined;p->k != NULL;++p)
	{
		OOBase::String strK,strV;
		int err = strK.assign(p->k);
		if (err == 0)
			err = strV.assign(p->v);

		if (err == 0)
			err = m_int_gen_entities.insert(strK,strV);

		if (err != 0)
			throw "Out of memory";
	}
}

Tokenizer::~Tokenizer()
{
	OOBase::HeapAllocator::free(m_stack);
}

void Tokenizer::load(const char* fname)
{
	delete m_io;
	m_io = NULL;

	do_init();
	
	m_input.clear();
	m_token.clear();
	m_entity_name.clear();
	m_entity.clear();
	m_system.clear();
	m_public.clear();

	m_io = new (std::nothrow) IOState(fname);

	next_char();
}

void Tokenizer::pre_push()
{
	if (!m_stack)
	{
		m_stacksize = 256;
		m_stack = static_cast<int*>(OOBase::HeapAllocator::allocate(m_stacksize*sizeof(int)));
	}
	else if (m_top == m_stacksize-1)
	{
		int* new_stack = static_cast<int*>(OOBase::HeapAllocator::reallocate(m_stack,m_stacksize*2*sizeof(int)));
		m_stack = new_stack;
		m_stacksize *= 2;
	}
}

unsigned char Tokenizer::next_char_i()
{
	unsigned char c = '\0';
	
	if (m_input.empty())
		c = m_io->get_char();
	else
		c = m_input.pop();
		
	return c;
}

void Tokenizer::next_char()
{
	unsigned char c = next_char_i();
	if (c == '\r')
	{
		c = '\n';
		
		unsigned char n = next_char_i();
		if (n != '\n')
			m_input.push(n);
	}
	
	if (c == '\n')
	{
		++m_io->m_line;
		m_io->m_col = 0;
	}

	if (c != 0)
		++m_io->m_col;

	m_char = c;
}

void Tokenizer::decoder(Decoder::eType type)
{
	// Apply a temporary decoder to the input stream, before reaching the real encoding value
	free(m_io->m_decoder);
	m_io->m_decoder = Decoder::create(type);
}

bool Tokenizer::encoding(OOBase::String& str)
{
	size_t len = 0;
	const char* val = m_token.pop(len);

	if (len)
	{
		// Drop any decoder and use the real decoder
		delete m_io->m_decoder;
		m_io->m_decoder = NULL;

		if (!m_io->m_next)
		{
			int err = str.assign(val,len);
			if (err != 0)
				throw "Out of memory";

			return true;
		}
	}

	return false;
}

void Tokenizer::general_entity()
{
	OOBase::String strName;
	m_entity_name.pop(strName);

	size_t val_len = 0;
	const char* val = m_token.pop(val_len);

	if (val_len)
	{
		// Internal general entity
		OOBase::String strVal;
		int err = strVal.assign(val,val_len);
		if (err == 0)
			err = m_int_gen_entities.insert(strName,strVal);

		if (err != 0)
			throw "Out of memory";
	}
	else
	{
		// Unparsed entity or external parsed entity
		struct External ext;
		m_entity.pop(ext.m_strNData);
		m_system.pop(ext.m_strSystemId);
		m_public.pop(ext.m_strPublicId);

		int err = m_ext_gen_entities.insert(strName,ext);
		if (err != 0)
			throw "Out of memory";
	}
}

void Tokenizer::param_entity()
{
	OOBase::String strName;
	m_entity_name.pop(strName);

	size_t val_len = 0;
	const char* val = m_token.pop(val_len);

	if (val_len)
	{
		// Internal parameter entity
		OOBase::String strVal;
		int err = strVal.assign(val,val_len);
		if (err == 0)
			err = m_int_param_entities.insert(strName,strVal);

		if (err != 0)
			throw "Out of memory";
	}
	else
	{
		// External parameter entity
		struct External ext;
		m_system.pop(ext.m_strSystemId);
		m_public.pop(ext.m_strPublicId);

		int err = m_ext_param_entities.insert(strName,ext);
		if (err != 0)
			throw "Out of memory";
	}
}

bool Tokenizer::subst_entity()
{
	OOBase::String strEnt,strReplace;
	m_entity.pop(strEnt);

	OOBase::String* pInt = m_int_gen_entities.find(strEnt);
	if (pInt)
		strReplace = *pInt;
	else
	{
		External* pExt = m_ext_gen_entities.find(strEnt);
		if (!pExt)
			throw "Unrecognized entity";
		else if (!pExt->m_strNData.empty())
		{
			void* TODO; // Unparsed entity handling
			strReplace = strEnt;
		}
		else
		{
			// Start pulling from external source
			IOState* n = new (std::nothrow) IOState(resolve_url(m_io->m_fname,pExt->m_strPublicId,pExt->m_strSystemId).c_str());
			n->m_next = m_io;
			m_io = n;
			return true;
		}
	}

	m_input.rappend(strReplace);
	return false;
}

void Tokenizer::subst_char(int base)
{
	OOBase::String strEntity;
	m_entity.pop(strEntity);

	unsigned long v = strtoul(strEntity.c_str(),NULL,base);

	// Check for Char (for xml 1.0)
	if (v <= 0x1F && (v != 0x9 || v != 0xA || v != 0xD))
		throw "Invalid Char";

	// Now recode v as UTF-8...
	if (v <= 0x7f)
		m_token.push(static_cast<unsigned char>(v));
	else if (v <= 0x7FF)
	{
		m_token.push(static_cast<unsigned char>(v >> 6) | 0xc0);
		m_token.push(static_cast<unsigned char>(v & 0x3f) | 0x80);
	}
	else if (v <= 0xFFFF)
	{
		m_token.push(static_cast<unsigned char>(v >> 12) | 0xe0);
		m_token.push(static_cast<unsigned char>((v & 0xfc0) >> 6) | 0x80);
		m_token.push(static_cast<unsigned char>(v & 0x3f) | 0x80);
	}
	else if (v <= 0x10FFFF)
	{
		m_token.push(static_cast<unsigned char>(v >> 18) | 0xf0);
		m_token.push(static_cast<unsigned char>((v & 0x3f000) >> 12) | 0x80);
		m_token.push(static_cast<unsigned char>((v & 0xfc0) >> 6) | 0x80);
		m_token.push(static_cast<unsigned char>(v & 0x3f) | 0x80);
	}
	else
	{
		m_token.push('\xEF');
		m_token.push('\xBF');
		m_token.push('\xBD');
	}
}

void Tokenizer::set_token(OOBase::String& str, size_t offset)
{
	size_t len = 0;
	const char* tok = m_token.pop(len);

	if (offset > len)
		offset = len;

	int err = str.assign(tok,len - offset);
	if (err != 0)
		throw "Out of memory";
}

void Tokenizer::external_doctype()
{
	OOBase::String strSystemId;
	m_system.pop(strSystemId);

	OOBase::String strPublicId;
	m_public.pop(strPublicId);

	// We cheat and use m_next here
	m_io->m_next = new (std::nothrow) IOState(resolve_url(m_io->m_fname,strPublicId,strSystemId).c_str());
}

bool Tokenizer::do_doctype()
{
	// We cheat and use m_next here
	if (m_io->m_next)
	{
		IOState* n = m_io->m_next;
		m_io->m_next = NULL;
		n->m_next = m_io;
		m_io = n;

		return true;
	}

	return false;
}
