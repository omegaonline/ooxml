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

IOState::IOState(const OOBase::String& fname) :
		m_fname(fname),
		m_col(0),
		m_line(1),
		m_decoder(NULL),
		m_next(NULL),
		m_auto_pop(false),
		m_io(new (std::nothrow) IO()),
		m_eof(false)
{
	if (!m_io)
		throw "Out of memory";

	m_io->open(fname.c_str());
}

IOState::IOState(const OOBase::String& entity_name, const OOBase::String& repl_text) :
		m_fname(entity_name),
		m_col(0),
		m_line(1),
		m_decoder(NULL),
		m_next(NULL),
		m_auto_pop(false),
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
