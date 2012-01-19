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
		m_decoder(NULL)
{
}

Tokenizer::~Tokenizer()
{
	OOBase::HeapAllocator::free(m_stack);

	delete m_decoder;
}

void Tokenizer::init()
{
	do_init();
	
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
	{
		c = get_char();

		while (m_decoder)
		{
			c = m_decoder->next(c);
			if (c != '\0')
				break;
		}
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
	
	m_char = c;
}

void Tokenizer::decoder(Decoder::eType type)
{
	// Apply a temporary decoder to the input stream, before reaching the real encoding value

	free(m_decoder);
	m_decoder = Decoder::create(type);
}

void Tokenizer::encoding()
{
	size_t len = 0;
	unsigned char* val = m_token.copy(len);

	if (len)
	{
		// Internal general entity
		printf("Encoding: %.*s\n",(int)len,val);
	}

	// Drop any decoder and use the real decoder
	delete m_decoder;
	m_decoder = NULL;
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
