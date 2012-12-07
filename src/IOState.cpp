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

IOState* IOState::create(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& fname, unsigned int version)
{
	void* p = allocator.allocate(sizeof(IOState),OOBase::alignof<IOState>::value);
	if (!p)
		throw "Out of memory";

	return ::new (p) IOState(allocator,fname,version);
}

IOState* IOState::create(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& entity_name, unsigned int version, const OOBase::LocalString& repl_text)
{
	void* p = allocator.allocate(sizeof(IOState),OOBase::alignof<IOState>::value);
	if (!p)
		throw "Out of memory";

	return ::new (p) IOState(allocator,entity_name,version,repl_text);
}

void IOState::destroy()
{
	OOBase::AllocatorInstance& a = m_allocator;
	this->~IOState();
	a.free(this);
}

IOState::IOState(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& fname, unsigned int version) :
		m_fname(fname),
		m_col(0),
		m_line(1),
		m_next(NULL),
		m_auto_pop(false),
		m_cs(0),
		m_char('\0'),
		m_allocator(allocator),
		m_decoder(NULL),
		m_decoder_type(Decoder::None),
		m_io(NULL),
		m_eof(false),
		m_preinit(true),
		m_input(allocator),
		m_version(version)
{
	void* p = allocator.allocate(sizeof(IO),OOBase::alignof<IO>::value);
	if (!p)
		throw "Out of memory";

	m_io = new (p) IO();

	int err = m_io->open(fname.c_str());
	if (err != 0)
		throw "IO Error";
}

IOState::IOState(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& entity_name, unsigned int version, const OOBase::LocalString& repl_text) :
		m_fname(entity_name),
		m_col(0),
		m_line(1),
		m_next(NULL),
		m_auto_pop(false),
		m_cs(0),
		m_char('\0'),
		m_allocator(allocator),
		m_decoder(NULL),
		m_decoder_type(Decoder::None),
		m_io(NULL),
		m_eof(repl_text.empty()),
		m_preinit(true),
		m_input(allocator),
		m_version(version)
{
	m_input.rappend(repl_text);
}

IOState::~IOState()
{
	Decoder::destroy(m_allocator,m_decoder);

	if (m_io)
	{
		m_io->~IO();
		m_allocator.free(m_io);
	}

	if (m_next)
		m_next->destroy();
}

void IOState::init(OOBase::LocalString& strEncoding, bool& standalone)
{
	init(false,strEncoding,standalone);
}

void IOState::init()
{
	OOBase::LocalString strEncoding(m_allocator);
	bool standalone;
	init(true,strEncoding,standalone);
}

void IOState::set_decoder(Decoder::eType type)
{
	if (m_decoder_type != type)
	{
		Decoder::destroy(m_allocator,m_decoder);
		m_decoder = Decoder::create(m_allocator,type);
		m_decoder_type = type;
	}
}

void IOState::set_encoding(Token& token, OOBase::LocalString& str)
{
	str = token.pop_string();
}

void IOState::switch_encoding(OOBase::LocalString& strEncoding)
{
	if (strEncoding.empty())
	{
		const char* sz = "UTF-8";
		switch (m_decoder_type)
		{
		case Decoder::UTF16BE:
			sz = "UTF-16BE";
			break;

		case Decoder::UTF16LE:
			sz = "UTF-16LE";
			break;

		case Decoder::UTF32BE:
		case Decoder::UTF32LE:
		case Decoder::EBCDIC:
			throw "Missing required encoding attribute";

		default:
			break;
		}

		int err = strEncoding.assign(sz);
		if (err != 0)
			throw "Out of memory";
	}

	if (strEncoding != "UTF-8" && strEncoding != "utf-8" && strEncoding != "UTF8" && strEncoding != "utf8")
		printf("NO DECODER FOR %s - WILL FAIL! ",strEncoding.c_str());
}

unsigned char IOState::get_char(bool& from_input)
{
	unsigned char c = '\0';
	if (!m_input.empty())
	{
		from_input = true;
		c = m_input.pop();
	}
	else if (m_io)
	{
		from_input = false;
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

bool IOState::is_file() const
{
	return (m_io != NULL);
}

unsigned int IOState::get_version()
{
	return m_version;
}

void IOState::push(unsigned char c)
{
	m_input.push(c);
}

unsigned char IOState::next_char()
{
	bool from_input = false;
	unsigned char c = get_char(from_input);

	if (!from_input)
	{
		if (c == '\r')
		{
			c = '\n';

			unsigned char n = get_char(from_input);
			if (!m_preinit && m_version == 1 && n == 0xC2)
			{
				unsigned char n2 = get_char(from_input);
				if (n2 != 0x85)
				{
					m_input.push(n2);
					m_input.push(n);
				}
			}
			else if (n != '\n')
				m_input.push(n);
		}
		else if (!m_preinit && m_version == 1)
		{
			if (c == 0xC2)
			{
				unsigned char n = get_char(from_input);
				if (n != 0x85)
					m_input.push(n);
				else
					c = '\n';
			}
			else if (c == 0xE2)
			{
				// e2 80 a8 = U+2028
				unsigned char n = get_char(from_input);
				if (n != 0x80)
					m_input.push(n);
				else
				{
					unsigned char n2 = get_char(from_input);
					if (n2 != 0xA8)
					{
						m_input.push(n2);
						m_input.push(n);
					}
					else
						c = '\n';
				}
			}
		}
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

void IOState::rappend(const OOBase::LocalString& str)
{
	m_input.rappend(str);
}

bool IOState::is_eof() const
{
	return m_eof;
}

void IOState::set_version(Token& token)
{
	unsigned long version = strtoul(token.pop_string().c_str(),NULL,10);

	if (m_version == (unsigned int)-1)
		m_version = version;
	else if (version > m_version)
		throw "Included external entity of higher version";
}
