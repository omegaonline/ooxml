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

	while (m_io != NULL)
	{
		struct IOInfo* n = m_io->m_prev;
		delete m_io->m_decoder;
		delete m_io->m_io;
		delete m_io;
		m_io = n;
	}
}

void Tokenizer::load(const char* fname)
{
	while (m_io != NULL)
	{
		struct IOInfo* n = m_io->m_prev;
		delete m_io->m_decoder;
		delete m_io->m_io;
		delete m_io;
		m_io = n;
	}

	do_init();
	
	m_input.clear();
	m_token.clear();
	m_entity_name.clear();
	m_entity.clear();
	m_system.clear();
	m_public.clear();

	m_io = new_io(fname);

	next_char();
}

Tokenizer::IOInfo* Tokenizer::new_io(const char* fname)
{
	IOInfo* io = new IOInfo;
	int err = io->m_fname.assign(fname);
	if (err != 0)
	{
		delete io;
		throw "Failed to assign string";
	}

	io->m_col = 0;
	io->m_line = 1;
	io->m_io = new IO;
	io->m_decoder = NULL;

	err = io->m_io->open(fname);
	if (err != 0)
	{
		delete io->m_io;
		delete io;
		throw "Failed to open file";
	}

	return io;
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
	{
		bool again = false;
		do
		{
			c = m_io->m_io->get_char();

			if (m_io->m_decoder)
				c = m_io->m_decoder->next(c,again);
		}
		while(again);
	}
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
	size_t val_len = 0;
	unsigned char* val = m_token.copy(val_len);

	if (val_len)
	{
		// Internal general entity
	}
	else
	{
		// External general entity
		size_t ndata_len = 0;
		unsigned char* ndata = m_entity.copy(ndata_len);
		if (ndata_len)
		{
			// Unparsed entity
		}
		else
		{
			// External parsed entity
		}
	}
}

void Tokenizer::param_entity()
{
	size_t val_len = 0;
	unsigned char* val = m_token.copy(val_len);

	if (val_len)
	{
		// Internal parameter entity
	}
	else
	{
		// External parameter entity
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
		printf("FAILED! %d\n",m_cs);
}

void Tokenizer::external_doctype()
{
	size_t sys_len = 0;
	unsigned char* sys_val = m_system.copy(sys_len);

	size_t pub_len = 0;
	unsigned char* pub_val = m_public.copy(pub_len);

	char szBuf[1024] = {0};
	strcpy(szBuf,m_io->m_fname.c_str());
	char* s = strrchr(szBuf,'/');
	if (s)
		*(s+1) = '\0';

	size_t len = sizeof(szBuf) - strlen(szBuf);
	memcpy(szBuf+strlen(szBuf),sys_val,sys_len >= len ? len-1 : sys_len);

	IOInfo* n = new_io(szBuf);
	int err = n->m_fname.assign(szBuf);

	// We cheat and use m_prev here
	m_io->m_prev = n;
}

bool Tokenizer::do_doctype()
{
	if (m_io->m_prev)
	{
		IOInfo* n = m_io->m_prev;
		m_io->m_prev = NULL;
		n->m_prev = m_io;
		m_io = n;

		return true;
	}

	printf("doctype.end\n");
	return false;
}

void Tokenizer::end_doctype()
{
	IOInfo* n = m_io;
	m_io = m_io->m_prev;
	delete n->m_decoder;
	delete n->m_io;
	delete n;

	printf("doctype.end\n");
}
