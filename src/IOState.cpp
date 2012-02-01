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
		m_io(new (std::nothrow) IO()),
		m_eof(false)
{
	if (!m_io)
		throw "Out of memory";

	int err = m_io->open(fname.c_str());
	if (err != 0)
		m_eof = true;

	unsigned char bom[4] = {0};
	size_t push_back = 0;

	while (!is_eof() && push_back < 4)
		bom[push_back++] = m_io->get_char();

	if (push_back >= 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[0] == 0xBF)
		push_back = 0;
	else if (push_back == 4)
	{
		if (bom[0] == 0x00 && bom[1] == 0x00)
		{
			if (bom[2] == 0xFE && bom[3] == 0xFF)
			{
				m_decoder = Decoder::create(Decoder::UTF32BE);
				push_back = 0;
			}
			else if (bom[2] == 0x00 && bom[3] == 0x3C)
				m_decoder = Decoder::create(Decoder::UTF32BE);
		}
		else if (bom[2] == 0x00 && bom[3] == 0x00)
		{
			if (bom[0] == 0xFF && bom[1] == 0xFE)
			{
				m_decoder = Decoder::create(Decoder::UTF32LE);
				push_back = 0;
			}
			else if (bom[0] == 0x3C && bom[1] == 0x00)
				m_decoder = Decoder::create(Decoder::UTF32LE);
		}
		else if (bom[0] == 0xFE && bom[1] == 0xFF && (bom[2] != 0x00 || bom[3] != 0x00))
		{
			m_decoder = Decoder::create(Decoder::UTF16BE);
			bom[1] = bom[3];
			bom[0] = bom[2];
			push_back = 2;
		}
		else if (bom[0] == 0x00 && bom[1] == 0x3C && bom[2] == 0x00 && bom[3] == 0x3F)
			m_decoder = Decoder::create(Decoder::UTF16BE);
		else if (bom[0] == 0xFF && bom[1] == 0xFE && (bom[2] != 0x00 || bom[3] != 0x00))
		{
			m_decoder = Decoder::create(Decoder::UTF16LE);
			bom[1] = bom[3];
			bom[0] = bom[2];
			push_back = 2;
		}
		else if (bom[0] == 0x3C && bom[1] == 0x00 && bom[2] == 0x3F && bom[3] == 0x00)
			m_decoder = Decoder::create(Decoder::UTF16LE);
		else if (bom[0] == 0x4C && bom[1] == 0x6F && bom[2] == 0xA7 && bom[3] == 0x94)
			m_decoder = Decoder::create(Decoder::EBCDIC);
	}

	while (push_back > 0)
		m_bom_input.push(bom[--push_back]);

	if (pre_space)
	{
		void* TODO; // Need to handle pre_space
		m_bom_input.push(' ');
	}
}

IOState::IOState(const OOBase::String& entity_name, const OOBase::String& repl_text) :
		m_fname(entity_name),
		m_col(0),
		m_line(1),
		m_next(NULL),
		m_auto_pop(false),
		m_decoder(NULL),
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

void IOState::clear_decoder()
{
	delete m_decoder;
	m_decoder = NULL;
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
			if (!m_bom_input.empty())
				c = m_bom_input.pop();
			else
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
