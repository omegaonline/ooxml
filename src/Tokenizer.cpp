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
#include "IOState.h"

Tokenizer::Tokenizer(OOBase::AllocatorInstance& allocator) :
		m_allocator(allocator),
		m_cs(0),
		m_stack(NULL),
		m_top(0),
		m_stacksize(0),
		m_char('\0'),
		m_token(allocator),
		m_entity_name(allocator),
		m_entity(allocator),
		m_system(allocator),
		m_public(allocator),
		m_internal_doctype(true),
		m_standalone(false),
		m_strEncoding(allocator),
		m_io(NULL),
		m_int_param_entities(allocator),
		m_int_gen_entities(allocator),
		m_ext_gen_entities(allocator),
		m_ext_param_entities(allocator)
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

	for (const predef* p = predefined;p->k != NULL;++p)
	{
		OOBase::LocalString strKey(m_allocator),strValue(m_allocator);
		int err = strKey.assign(p->k);
		if (err == 0)
			err = strValue.assign(p->v);

		if (err != 0)
			throw "Out of memory";

		m_int_gen_entities.insert(strKey,InternalEntity(strValue,false));
	}
}

Tokenizer::~Tokenizer()
{
	m_allocator.free(m_stack);

	while (m_io)
		io_pop();
}

void Tokenizer::load(const OOBase::LocalString& fname)
{
	while (m_io)
		io_pop();

	do_init();
	
	m_token.clear();
	m_entity_name.clear();
	m_entity.clear();
	m_system.clear();
	m_public.clear();

	m_internal_doctype = true;

	m_io = IOState::create(m_allocator,fname);

	m_io->init(m_strEncoding,m_standalone);

	next_char();
}

size_t Tokenizer::get_column() const
{
	if (m_io)
		return m_io->m_col;
	else
		return 0;
}

size_t Tokenizer::get_line() const
{
	if (m_io)
		return m_io->m_line;
	else
		return 0;
}

unsigned int Tokenizer::get_version() const
{
	if (m_io)
		return m_io->get_version();
	else
		return 0;
}

OOBase::LocalString Tokenizer::get_location() const
{
	if (m_io)
		return m_io->m_fname;
	else
		return OOBase::LocalString(m_allocator);
}

void Tokenizer::pre_push()
{
	if (!m_stack)
	{
		m_stacksize = 256;
		m_stack = static_cast<int*>(m_allocator.allocate(m_stacksize*sizeof(int),OOBase::alignment_of<int>::value));
	}
	else if (m_top == m_stacksize-1)
	{
		int* new_stack = static_cast<int*>(m_allocator.reallocate(m_stack,m_stacksize*2*sizeof(int),OOBase::alignment_of<int>::value));
		m_stack = new_stack;
		m_stacksize *= 2;
	}

	if (!m_stack)
		throw "Out of memory";
}

bool Tokenizer::operator == (const EndOfFile&) const
{
	return (m_io == NULL || m_io->is_eof());
}

bool Tokenizer::operator == (const ParseState& pe) const
{
	return (pe.m_halt || m_io == NULL || m_io->is_eof());
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

void Tokenizer::general_entity()
{
	OOBase::LocalString strSysLiteral = m_system.pop_string();
	if (strSysLiteral.empty())
	{
		// Internal general entity
		int err = m_int_gen_entities.insert(m_entity_name.pop_string(),InternalEntity(m_token.pop_string(),!m_internal_doctype));
		if (err != 0 && err != EEXIST)
			throw "Out of memory";
	}
	else
	{
		// Unparsed entity or external parsed entity
		int err = m_ext_gen_entities.insert(m_entity_name.pop_string(),ExternalEntity(m_entity.pop_string(),strSysLiteral,m_public.pop_string()));
		if (err != 0 && err != EEXIST)
			throw "Out of memory";
	}
}

void Tokenizer::param_entity()
{
	OOBase::LocalString strSysLiteral = m_system.pop_string();
	if (strSysLiteral.empty())
	{
		// Internal parameter entity
		int err = m_int_param_entities.insert(m_entity_name.pop_string(),m_token.pop_string());
		if (err != 0 && err != EEXIST)
			throw "Out of memory";
	}
	else
	{
		// External parameter entity
		int err = m_ext_param_entities.insert(m_entity_name.pop_string(),ExternalEntity(m_public.pop_string(),strSysLiteral));
		if (err != 0 && err != EEXIST)
			throw "Out of memory";
	}
}

void Tokenizer::bypass_entity()
{
	OOBase::LocalString strEnt = m_entity.pop_string();

	OOBase::HashTable<OOBase::LocalString,ExternalEntity,OOBase::AllocatorInstance>::iterator i = m_ext_gen_entities.find(strEnt);
	if (i != m_ext_gen_entities.end())
	{
		if (!i->value.m_strNData.empty())
			throw "Unparsed entity reference in entity value";

		if (m_standalone)
		{
			printf("External in standalone document\n");
			throw "External in standalone document";
		}
	}

	m_token.push('&');
	for (const char* sz = strEnt.c_str();*sz != '\0';++sz)
		m_token.push(*sz);

	m_token.push(';');
}

void Tokenizer::check_entity_recurse(const OOBase::LocalString& strEnt)
{
	for (IOState* io = m_io;io != NULL; io=io->m_next)
	{
		if (io->m_fname == strEnt)
			throw "WFC: No Recursion";
	}
}

OOBase::LocalString Tokenizer::get_external_fname() const
{
	IOState* io=m_io;
	while (io != NULL && !io->is_file())
		io = io->m_next;

	return (io ? io->m_fname : OOBase::LocalString(m_allocator));
}

bool Tokenizer::subst_content_entity()
{
	OOBase::LocalString strEnt = m_entity.pop_string();

	IOState* n = NULL;

	OOBase::HashTable<OOBase::LocalString,InternalEntity,OOBase::AllocatorInstance>::iterator internal = m_int_gen_entities.find(strEnt);
	if (internal != m_int_gen_entities.end())
	{
		if (m_standalone && internal->value.m_extern_decl)
			throw "VC: External in standalone document";

		if (!internal->value.m_strValue.empty())
		{
			OOBase::LocalString strFull(m_allocator);
			int err = strFull.concat("&",strEnt.c_str());
			if (err == 0)
				err = strFull.append(";");
			if (err != 0)
				throw "Out of memory";

			check_entity_recurse(strFull);

			n = IOState::create(m_allocator,strFull,get_version(),internal->value.m_strValue);
		}
	}
	else
	{
		OOBase::HashTable<OOBase::LocalString,ExternalEntity,OOBase::AllocatorInstance>::iterator external = m_ext_gen_entities.find(strEnt);
		if (external == m_ext_gen_entities.end())
			throw "WFC: Entity Declared";

		if (m_standalone) // validity error only
			throw "VC: External in standalone document";

		if (!external->value.m_strNData.empty())
			throw "WFC: Parsed Entity";

		OOBase::LocalString strExt = resolve_url(get_external_fname(),external->value.m_strPublicId,external->value.m_strSystemId);

		check_entity_recurse(strExt);

		// Start pulling from external source
		n = IOState::create(m_allocator,strExt,get_version());

		n->init();
	}

	if (n)
	{
		n->m_next = m_io;
		m_io = n;
	}

	return (n != NULL);
}

bool Tokenizer::subst_attr_entity()
{
	OOBase::LocalString strEnt = m_entity.pop_string();

	OOBase::HashTable<OOBase::LocalString,InternalEntity,OOBase::AllocatorInstance>::iterator i = m_int_gen_entities.find(strEnt);
	if (i == m_int_gen_entities.end())
	{
		if (m_ext_param_entities.find(strEnt) != m_ext_param_entities.end())
			throw "WFC: No External Entity References";

		//if (m_standalone)
			throw "WFC: Entity Declared";

		// Otherwise a VC
		//return false;
	}

	if (m_standalone && i->value.m_extern_decl)
		throw "WFC: Entity Declared";

	if (!i->value.m_strValue.empty())
	{
		OOBase::LocalString strFull(m_allocator);
		int err = strFull.concat("&",strEnt.c_str());
		if (err == 0)
			err = strFull.append(";");
		if (err != 0)
			throw "Out of memory";

		check_entity_recurse(strFull);

		IOState* n = IOState::create(m_allocator,strFull,get_version(),i->value.m_strValue);

		n->m_next = m_io;
		m_io = n;
	}

	return (!i->value.m_strValue.empty());
}

bool Tokenizer::subst_pentity()
{
	OOBase::LocalString strEnt = m_entity.pop_string();

	IOState* n = NULL;

	if (m_internal_doctype)
		throw "WFC: PEs in Internal Subset";

	OOBase::HashTable<OOBase::LocalString,OOBase::LocalString,OOBase::AllocatorInstance>::iterator internal = m_int_param_entities.find(strEnt);
	if (internal != m_int_param_entities.end())
	{
		if (!internal->value.empty())
		{
			OOBase::LocalString strFull(m_allocator);
			int err = strFull.concat("%",strEnt.c_str());
			if (err == 0)
				err = strFull.append(";");
			if (err != 0)
				throw "Out of memory";

			check_entity_recurse(strFull);

			n = IOState::create(m_allocator,strFull,get_version(),internal->value);
		}
	}
	else
	{
		OOBase::HashTable<OOBase::LocalString,ExternalEntity,OOBase::AllocatorInstance>::iterator external = m_ext_param_entities.find(strEnt);
		if (external == m_ext_param_entities.end())
			throw "Unrecognized entity";

		OOBase::LocalString strExt = resolve_url(get_external_fname(),external->value.m_strPublicId,external->value.m_strSystemId);

		check_entity_recurse(strExt);

		// Start pulling from external source
		n = IOState::create(m_allocator,strExt,get_version());

		n->init();
	}

	if (n)
	{
		n->m_next = m_io;
		m_io = n;
	}

	return (n != NULL);
}

void Tokenizer::include_pe(bool auto_pop)
{
	OOBase::LocalString strEnt = m_entity.pop_string();

	IOState* n = NULL;

	OOBase::HashTable<OOBase::LocalString,OOBase::LocalString,OOBase::AllocatorInstance>::iterator internal = m_int_param_entities.find(strEnt);
	if (internal != m_int_param_entities.end())
	{
		OOBase::LocalString strFull(m_allocator);
		int err = strFull.concat("%",strEnt.c_str());
		if (err == 0)
			err = strFull.append(";");
		if (err != 0)
			throw "Out of memory";

		check_entity_recurse(strFull);

		n = IOState::create(m_allocator,strFull,get_version(),internal->value);

		n->m_auto_pop = auto_pop;
	}
	else
	{
		OOBase::HashTable<OOBase::LocalString,ExternalEntity,OOBase::AllocatorInstance>::iterator external = m_ext_param_entities.find(strEnt);
		if (external == m_ext_param_entities.end())
			throw "Unrecognized entity";

		OOBase::LocalString strExt = resolve_url(get_external_fname(),external->value.m_strPublicId,external->value.m_strSystemId);

		check_entity_recurse(strExt);

		// Start pulling from external source
		n = IOState::create(m_allocator,strExt,get_version());

		n->init();
		n->m_auto_pop = auto_pop;
	}

	if (n)
	{
		// Ensure there is 1 trailing space
		if (m_io)
			m_io->push(' ');

		// And one leading space
		n->push(' ');
		n->m_next = m_io;
		m_io = n;
	}
}

void Tokenizer::subst_char(int base)
{
	unsigned long v = strtoul(m_entity.pop_string().c_str(),NULL,base);

	// Check for Char
	if (get_version() == 1)
	{
		if (v == 0x00 ||
			(v >= 0xD800 && v <= 0xDFFF) ||
			(v >= 0xFFFE && v <= 0xFFFF) ||
			v > 0x10FFFF)
		{
			throw "WFC: Illegal Char";
		}
	}
	else if ((v <= 0x1F && v != 0x9 && v != 0xA && v != 0xD) ||
			(v >= 0xD800 && v <= 0xDFFF) ||
			(v >= 0xFFFE && v <= 0xFFFF) ||
			v > 0x10FFFF)
	{
		throw "WFC: Illegal Char";
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
	// We cheat and use m_next here
	m_io->m_next = IOState::create(m_allocator,resolve_url(m_io->m_fname,m_public.pop_string(),m_system.pop_string()),get_version());
	if (!m_io->m_next)
		throw "Out of memory";

	m_io->m_next->init();
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
		n->destroy();
	}
}
