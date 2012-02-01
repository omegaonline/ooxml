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

Tokenizer::Tokenizer() : 
		m_stack(NULL), 
		m_stacksize(0),
		m_char('\0'),
		m_internal_doctype(true),
		m_io(NULL)
{
	struct predef
	{
		const char* k;
		const char* v;
	};

	static const predef predefined[] =
	{
		{"lt", "&#60;" },
		{"gt", "&#62;" },
		{"amp", "&#38;" },
		{"apos", "&#39;" },
		{"quot", "&#34;" },
		{NULL,NULL}
	};

	for (const struct predef* p = predefined;p->k != NULL;++p)
	{
		OOBase::String strKey,strValue;
		int err = strKey.assign(p->k);
		if (err == 0)
			err = strValue.assign(p->v);

		if (err != 0)
			throw "Out of memory";

		m_int_gen_entities.insert(strKey,strValue);
	}
}

Tokenizer::~Tokenizer()
{
	OOBase::HeapAllocator::free(m_stack);
}

void Tokenizer::load(const OOBase::String& fname)
{
	delete m_io;
	m_io = NULL;

	do_init();
	
	m_token.clear();
	m_entity_name.clear();
	m_entity.clear();
	m_system.clear();
	m_public.clear();

	m_internal_doctype = true;

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

void Tokenizer::next_char()
{
	m_char = '\0';

	while (m_io)
	{
		m_char = m_io->next_char();
		if (m_io->is_eof() && m_io->m_auto_pop)
			io_pop();
		else
			break;
	}
}

void Tokenizer::decoder(Decoder::eType type)
{
	// Apply a temporary decoder to the input stream, before reaching the real encoding value
	if (m_io)
	{
		free(m_io->m_decoder);
		m_io->m_decoder = Decoder::create(type);
	}

	printf("WILL FAIL - NO DECODER!\n");
}

void Tokenizer::encoding(ParseState& pe)
{
	size_t len = 0;
	const char* val = m_token.pop(len);

	if (len && m_io)
	{
		// Drop any decoder and use the real decoder
		delete m_io->m_decoder;
		m_io->m_decoder = NULL;

		if (!m_io->m_next)
		{
			int err = pe.m_strToken.assign(val,len);
			if (err != 0)
				throw "Out of memory";

			pe.m_type = Tokenizer::DocumentEncoding;
			pe.m_halt = true;

			if (pe.m_strToken != "UTF-8" && pe.m_strToken != "utf-8")
				printf("WILL FAIL - NO DECODER FOR %s\n",pe.m_strToken.c_str());
		}
	}
}

void Tokenizer::general_entity()
{
	OOBase::String strName,strVal,strSysLiteral;
	m_entity_name.pop(strName);
	m_system.pop(strSysLiteral);
	m_token.pop(strVal);

	if (strSysLiteral.empty())
	{
		// Internal general entity
		int err = m_int_gen_entities.insert(strName,strVal);
		if (err != 0 && err != EEXIST)
			throw "Out of memory";
	}
	else
	{
		// Unparsed entity or external parsed entity
		struct External ext;
		ext.m_strSystemId = strSysLiteral;
		m_entity.pop(ext.m_strNData);
		m_public.pop(ext.m_strPublicId);

		int err = m_ext_gen_entities.insert(strName,ext);
		if (err != 0 && err != EEXIST)
			throw "Out of memory";
	}
}

void Tokenizer::param_entity()
{
	OOBase::String strName,strVal,strSysLiteral;
	m_entity_name.pop(strName);
	m_system.pop(strSysLiteral);
	m_token.pop(strVal);

	if (strSysLiteral.empty())
	{
		// Internal parameter entity
		int err = m_int_param_entities.insert(strName,strVal);
		if (err != 0 && err != EEXIST)
			throw "Out of memory";
	}
	else
	{
		// External parameter entity
		struct External ext;
		ext.m_strSystemId = strSysLiteral;
		m_public.pop(ext.m_strPublicId);

		int err = m_ext_param_entities.insert(strName,ext);
		if (err != 0 && err != EEXIST)
			throw "Out of memory";
	}
}

void Tokenizer::bypass_entity()
{
	OOBase::String strEnt;
	m_entity.pop(strEnt);

	External* pExt = m_ext_gen_entities.find(strEnt);
	if (pExt && !pExt->m_strNData.empty())
		throw "Unparsed entity reference in entity value";

	m_token.push('&');
	for (const char* sz = strEnt.c_str();*sz != '\0';++sz)
		m_token.push(*sz);

	m_token.push(';');
}

void Tokenizer::check_entity_recurse(const OOBase::String& strEnt)
{
	for (IOState* io = m_io;io != NULL; io=io->m_next)
	{
		if (io->m_fname == strEnt)
			throw "Recursive entity!";
	}
}

unsigned int Tokenizer::subst_content_entity()
{
	OOBase::String strEnt;
	m_entity.pop(strEnt);

	unsigned int r = 0;
	IOState* n = NULL;

	OOBase::String* pInt = m_int_gen_entities.find(strEnt);
	if (pInt)
	{
		if (!pInt->empty())
		{
			OOBase::String strFull;
			int err = strFull.concat("&",strEnt.c_str());
			if (err == 0)
				err = strFull.append(";");
			if (err != 0)
				throw "Out of memory";

			check_entity_recurse(strFull);

			n = new (std::nothrow) IOState(strFull,*pInt);
			if (!n)
				throw "Out of memory";

			r = 1;
		}
	}
	else
	{
		External* pExt = m_ext_gen_entities.find(strEnt);
		if (!pExt)
			throw "Unrecognized entity";

		if (!pExt->m_strNData.empty())
		{
			void* TODO; // Unparsed entity handling
		}
		else
		{
			OOBase::String strExt = resolve_url(m_io->m_fname,pExt->m_strPublicId,pExt->m_strSystemId);

			check_entity_recurse(strExt);

			// Start pulling from external source
			n = new (std::nothrow) IOState(strExt);
			if (!n)
				throw "Out of memory";

			r = 2;
		}
	}

	if (n)
	{
		n->m_next = m_io;
		m_io = n;
	}

	return r;
}

bool Tokenizer::subst_attr_entity()
{
	OOBase::String strEnt;
	m_entity.pop(strEnt);

	OOBase::String* pInt = m_int_gen_entities.find(strEnt);
	if (!pInt)
		throw "External entity in attribute value";

	if (!pInt->empty())
	{
		OOBase::String strFull;
		int err = strFull.concat("&",strEnt.c_str());
		if (err == 0)
			err = strFull.append(";");
		if (err != 0)
			throw "Out of memory";

		check_entity_recurse(strFull);

		IOState* n = new (std::nothrow) IOState(strFull,*pInt);
		if (!n)
			throw "Out of memory";

		n->m_next = m_io;
		m_io = n;
	}

	return (!pInt->empty());
}

unsigned int Tokenizer::subst_pentity()
{
	OOBase::String strEnt;
	m_entity.pop(strEnt);

	unsigned int r = 0;
	IOState* n = NULL;

	if (m_internal_doctype)
		throw "PE in Internal Subset";

	OOBase::String* pInt = m_int_param_entities.find(strEnt);
	if (pInt)
	{
		if (!pInt->empty())
		{
			OOBase::String strFull;
			int err = strFull.concat("%",strEnt.c_str());
			if (err == 0)
				err = strFull.append(";");
			if (err != 0)
				throw "Out of memory";

			check_entity_recurse(strFull);

			n = new (std::nothrow) IOState(strFull,*pInt);
			if (!n)
				throw "Out of memory";

			r = 1;
		}
	}
	else
	{
		External* pExt = m_ext_param_entities.find(strEnt);
		if (!pExt)
			throw "Unrecognized entity";

		OOBase::String strExt = resolve_url(m_io->m_fname,pExt->m_strPublicId,pExt->m_strSystemId);

		check_entity_recurse(strExt);

		// Start pulling from external source
		n = new (std::nothrow) IOState(strExt);
		if (!n)
			throw "Out of memory";

		r = 2;
	}

	if (n)
	{
		n->m_next = m_io;
		m_io = n;
	}

	return r;
}

bool Tokenizer::include_pe(bool auto_pop)
{
	OOBase::String strEnt;
	m_entity.pop(strEnt);

	bool r = false;
	IOState* n = NULL;

	OOBase::String* pInt = m_int_param_entities.find(strEnt);
	if (pInt)
	{
		OOBase::String strFull;
		int err = strFull.concat("%",strEnt.c_str());
		if (err == 0)
			err = strFull.append(";");
		if (err != 0)
			throw "Out of memory";

		check_entity_recurse(strFull);

		n = new (std::nothrow) IOState(strFull,*pInt);
		if (!n)
			throw "Out of memory";

		n->m_auto_pop = auto_pop;
	}
	else
	{
		External* pExt = m_ext_param_entities.find(strEnt);
		if (!pExt)
			throw "Unrecognized entity";

		OOBase::String strExt = resolve_url(m_io->m_fname,pExt->m_strPublicId,pExt->m_strSystemId);

		check_entity_recurse(strExt);

		// Start pulling from external source
		n = new (std::nothrow) IOState(strExt);
		if (!n)
			throw "Out of memory";

		n->m_auto_pop = auto_pop;

		r = true;
	}

	if (n)
	{
		if (m_io)
			m_io->push(' ');

		n->m_next = m_io;
		m_io = n;
		m_io->push(' ');
	}

	return r;
}

void Tokenizer::subst_char(int base)
{
	OOBase::String strEntity;
	m_entity.pop(strEntity);

	unsigned long v = strtoul(strEntity.c_str(),NULL,base);

	// Check for Char (for xml 1.0)
	if ((v <= 0x1F && v != 0x9 && v != 0xA && v != 0xD) ||
		(v >= 0xD800 && v <= 0xDFFF) ||
		(v >= 0xFFFE && v <= 0xFFFF) ||
		v > 0x10FFFF)
	{
		throw "Invalid Char";
	}

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
	else
	{
		m_token.push(static_cast<unsigned char>(v >> 18) | 0xf0);
		m_token.push(static_cast<unsigned char>((v & 0x3f000) >> 12) | 0x80);
		m_token.push(static_cast<unsigned char>((v & 0xfc0) >> 6) | 0x80);
		m_token.push(static_cast<unsigned char>(v & 0x3f) | 0x80);
	}
}

void Tokenizer::set_token(ParseState& pe, enum TokenType type, size_t offset, bool allow_empty)
{
	size_t len = 0;
	const char* tok = m_token.pop(len);

	if (offset > len)
		offset = len;

	int err = pe.m_strToken.assign(tok,len - offset);
	if (err != 0)
		throw "Out of memory";

	if (allow_empty || len > offset)
	{
		pe.m_type = type;
		pe.m_halt = true;
	}
}

void Tokenizer::external_doctype()
{
	OOBase::String strSystemId;
	m_system.pop(strSystemId);

	OOBase::String strPublicId;
	m_public.pop(strPublicId);

	// We cheat and use m_next here
	m_io->m_next = new (std::nothrow) IOState(resolve_url(m_io->m_fname,strPublicId,strSystemId));
}

bool Tokenizer::do_doctype()
{
	// We cheat and use m_next here
	if (m_io && m_io->m_next)
	{
		IOState* n = m_io->m_next;
		m_io->m_next = NULL;
		n->m_next = m_io;
		m_io = n;

		m_internal_doctype = false;

		return true;
	}

	return false;
}

void Tokenizer::io_pop()
{
	if (m_io)
	{
		IOState* n = m_io;
		m_io = m_io->m_next;
		n->m_next = NULL;
		delete n;
	}
}
