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

#include <OOBase/Memory.h>

IOState::IOState(const char* fname) :
		m_col(0),
		m_line(1),
		m_decoder(NULL),
		m_next(NULL),
		m_io(new IO())
{
	int err = m_fname.assign(fname);
	if (err != 0)
		throw "Buggered";

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

	m_io = new IOState(fname);

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

	++m_io->m_col;
	m_char = c;
}

void Tokenizer::decoder(Decoder::eType type)
{
	// Apply a temporary decoder to the input stream, before reaching the real encoding value
	free(m_io->m_decoder);
	m_io->m_decoder = Decoder::create(type);
}

void Tokenizer::encoding()
{
	size_t len = 0;
	unsigned char* val = m_token.copy(len);

	if (len)
	{
		// Drop any decoder and use the real decoder
		delete m_io->m_decoder;
		m_io->m_decoder = NULL;

		printf("document.encoding: %.*s\n",(int)len,val);
	}
}

void Tokenizer::general_entity()
{
	OOBase::String strName;
	m_entity_name.copy(strName);

	size_t val_len = 0;
	unsigned char* val = m_token.copy(val_len);

	if (val_len)
	{
		// Internal general entity
		OOBase::String strVal;
		strVal.assign(reinterpret_cast<char*>(val),val_len);

		m_int_gen_entities.insert(strName,strVal);
	}
	else
	{
		// Unparsed entity or external parsed entity
		struct External ext;
		m_entity.copy(ext.m_strNData);
		m_system.copy(ext.m_strSystemId);
		m_public.copy(ext.m_strPublicId);

		m_ext_gen_entities.insert(strName,ext);
	}
}

void Tokenizer::param_entity()
{
	OOBase::String strName;
	m_entity_name.copy(strName);

	size_t val_len = 0;
	unsigned char* val = m_token.copy(val_len);

	if (val_len)
	{
		// Internal parameter entity
		OOBase::String strVal;
		strVal.assign(reinterpret_cast<char*>(val),val_len);

		m_int_param_entities.insert(strName,strVal);
	}
	else
	{
		// External parameter entity
		struct External ext;
		m_system.copy(ext.m_strSystemId);
		m_public.copy(ext.m_strPublicId);

		m_ext_param_entities.insert(strName,ext);
	}
}

void Tokenizer::subst_entity()
{
	size_t len = 0;
	unsigned char* val = m_entity.copy(len);

	char szBuf[128] = {0};
	sprintf(szBuf,"&%.*s;",(int)len,val);

	for (size_t i=0;szBuf[i] != '\0';++i)
		m_token.push(szBuf[i]);
}

void Tokenizer::subst_char()
{
	//printf("CHARREF: '");
	//m_entity.dump();
	//printf("'\n");
	
	size_t len = 0;
	unsigned char* val = m_entity.copy(len);

	m_token.push('1');
}

void Tokenizer::subst_hex()
{
	size_t len = 0;
	unsigned char* val = m_entity.copy(len);

	printf("CHARREF: '0x%.*s'\n",(int)len,val);
}

void Tokenizer::token(const char* s, size_t offset)
{
	size_t len = 0;
	unsigned char* tok = m_token.copy(len);

	if (len > offset)
		printf("%s: %.*s\n",s,(int)(len - offset),tok);
	else
		printf("%s\n",s);
}

void Tokenizer::next_token()
{
	if (!do_exec())
		printf("Syntax error in %s, line %ld, column %ld.\n",m_io->m_fname.c_str(),m_io->m_line,m_io->m_col);
}

void Tokenizer::external_doctype()
{
	OOBase::String strSystemId;
	m_system.copy(strSystemId);

	OOBase::String strPublicId;
	m_public.copy(strPublicId);

	// We cheat and use m_next here
	m_io->m_next = new IOState(resolve_url(m_io->m_fname,strPublicId,strSystemId).c_str());
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

	printf("doctype.end\n");
	return false;
}

void Tokenizer::end_doctype()
{
	IOState::pop(m_io);

	printf("doctype.end\n");
}
