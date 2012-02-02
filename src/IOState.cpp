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

#include "IOState.h"

IOState::IOState(const OOBase::String& fname, bool pre_space) :
		m_fname(fname),
		m_col(0),
		m_line(1),
		m_next(NULL),
		m_auto_pop(false),
		m_decoder(NULL),
		m_decoder_type(Decoder::None),
		m_io(new (std::nothrow) IO()),
		m_eof(false)
{
	if (!m_io)
		throw "Out of memory";

	int err = m_io->open(fname.c_str());
	if (err != 0)
		m_eof = true;

	if (pre_space)
	{
		void* TODO; // Need to handle pre_space

	}
}

IOState::IOState(const OOBase::String& entity_name, const OOBase::String& repl_text) :
		m_fname(entity_name),
		m_col(0),
		m_line(1),
		m_next(NULL),
		m_auto_pop(false),
		m_decoder(NULL),
		m_decoder_type(Decoder::None),
		m_io(NULL),
		m_eof(repl_text.empty())
{
	m_input.rappend(repl_text);
}

IOState::~IOState()
{
	delete m_decoder;
	delete m_io;
	delete m_next;
}

void IOState::set_decoder(Decoder::eType type)
{
	if (m_decoder_type != type)
	{
		delete m_decoder;
		m_decoder = Decoder::create(type);
		m_decoder_type = type;
	}
}

Decoder::eType IOState::get_decoder() const
{
	return m_decoder_type;
}

void IOState::set_encoder(const OOBase::String& str)
{
	if (m_encoding != str && !m_encoding.empty() && !str.empty())
		throw "Invalid encoding";

	if (m_encoding != str)
	{
		delete m_decoder;
		m_decoder = NULL;

		m_encoding = str;

		if (str != "UTF-8" && str != "utf-8")
			printf("WILL FAIL - NO DECODER FOR %s\n",str.c_str());
	}
}

OOBase::String IOState::get_encoder() const
{
	return m_encoding;
}

unsigned char IOState::get_char()
{
	unsigned char c = '\0';
	if (!m_input.empty())
	{
		c = m_input.pop();
	}
	else if (m_io)
	{
		bool again = false;
		do
		{
			c = m_io->get_char();

			if (m_decoder)
				c = m_decoder->next(c,again);
		}
		while(again);

		m_eof = m_io->is_eof();
	}
	else
		m_eof = true;

	return c;
}

void IOState::push(unsigned char c)
{
	m_input.push(c);
}

unsigned char IOState::next_char()
{
	unsigned char c = get_char();
	if (c == '\r')
	{
		c = '\n';

		unsigned char n = get_char();
		if (n != '\n')
			m_input.push(n);
	}

	if (c == '\n')
	{
		++m_line;
		m_col = 0;
	}

	if (c != '\0')
		++m_col;

	return c;
}

void IOState::rappend(const OOBase::String& str)
{
	m_input.rappend(str);
}

bool IOState::is_eof() const
{
	return m_eof;
}
