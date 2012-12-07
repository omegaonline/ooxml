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

#include <OOBase/Stack.h>
#include <OOBase/ArenaAllocator.h>
#include <OOBase/Set.h>

static size_t passed = 0;
static size_t failed = 0;

static const int verbose = 0;

static bool do_wf_test(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& strURI, bool fail_expected)
{
	OOBase::Stack<OOBase::LocalString,OOBase::AllocatorInstance> elements(allocator);
	OOBase::Set<OOBase::LocalString,OOBase::AllocatorInstance> attributes(allocator);

	Tokenizer tok(allocator);

	tok.load(strURI);

	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::LocalString strToken(allocator);
		tok_type = tok.next_token(strToken,verbose);

		if (tok_type == Tokenizer::ElementStart)
		{
			elements.push(strToken);
			attributes.clear();
		}
		else if (tok_type == Tokenizer::AttributeName)
		{
			if (attributes.exists(strToken))
				return false;

			attributes.insert(strToken);
		}
		else if (tok_type == Tokenizer::ElementEnd)
		{
			OOBase::LocalString strE(allocator);
			elements.pop(&strE);

			if (!strToken.empty() && strE != strToken)
				return false;
		}
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);

	if (tok_type == Tokenizer::Error && !fail_expected)
		printf("\nSyntax error at %s, line %lu, col %lu\n",tok.get_location().c_str(),tok.get_line(),tok.get_column());

	return (tok_type == Tokenizer::End);
}

static bool do_valid_test(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& strURI, bool fail_expected)
{
	OOBase::Stack<OOBase::LocalString,OOBase::AllocatorInstance> elements(allocator);
	OOBase::Set<OOBase::LocalString,OOBase::AllocatorInstance> attributes(allocator);
	OOBase::LocalString strDocType(allocator);
	bool root = true;

	Tokenizer tok(allocator);

	tok.load(strURI);

	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::LocalString strToken(allocator);
		tok_type = tok.next_token(strToken,verbose);

		if (tok_type == Tokenizer::DocTypeStart)
		{
			strDocType = strToken;
		}
		else if (tok_type == Tokenizer::ElementStart)
		{
			if (root)
			{
				if (strDocType.empty())
				{
					if (!fail_expected)
						printf("No DOCTYPE\n");
					return false;
				}

				if (strToken != strDocType)
				{
					if (!fail_expected)
						printf("Mismatched root element\n");
					return false;
				}

				root = false;
			}

			elements.push(strToken);
			attributes.clear();
		}
		else if (tok_type == Tokenizer::AttributeName)
		{
			if (attributes.exists(strToken))
				return false;

			attributes.insert(strToken);
		}
		else if (tok_type == Tokenizer::ElementEnd)
		{
			OOBase::LocalString strE(allocator);
			elements.pop(&strE);

			if (!strToken.empty() && strE != strToken)
				return false;
		}
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);

	return (tok_type == Tokenizer::End);
}

static bool pass()
{
	printf("[OK]\n");
	++passed;
	return true;
}

static bool fail()
{
	printf("[Fail] ");
	++failed;
	return false;
}

static bool do_valid_test(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& strURI)
{
	if (do_valid_test(allocator,strURI,false))
		return pass();
	else if (do_wf_test(allocator,strURI,false))
	{
		printf("[Well-formed] ");
		++passed;
		return false;
	}
	else
		return fail();
}

static bool do_invalid_test(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& strURI)
{
	if (!do_wf_test(allocator,strURI,false))
		return fail();
	else if (!do_valid_test(allocator,strURI,true))
		return pass();
	else
	{
		printf("[Well-formed] ");
		++passed;
		return false;
	}
}

static bool do_not_wf_test(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& strURI)
{
	if (do_wf_test(allocator,strURI,true))
		return fail();
	else
		return pass();
}

static bool do_error_test(OOBase::AllocatorInstance& allocator, const OOBase::LocalString& strURI)
{
	if (!do_wf_test(allocator,strURI,true))
		return pass();

	printf("[No error]\n");
	return true;
}

static void do_test(OOBase::AllocatorInstance& allocator, Tokenizer& tok, const OOBase::LocalString& strBase)
{
	OOBase::LocalString strType(allocator),strText(allocator);
	OOBase::LocalString strURI = strBase;
	OOBase::LocalString strEdition(allocator);
	OOBase::LocalString strNamespace(allocator);
	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::LocalString strToken(allocator);
		tok_type = tok.next_token(strToken,verbose);

		if (tok_type == Tokenizer::AttributeName)
		{
			if (strToken == "ID")
			{
				if (tok.next_token(strToken,verbose) == Tokenizer::AttributeValue)
					printf("Test: %s...",strToken.c_str());
			}
			else if (strToken == "TYPE")
			{
				if (tok.next_token(strToken,verbose) == Tokenizer::AttributeValue)
					strType = strToken;
			}
			else if (strToken == "URI")
			{
				if (tok.next_token(strToken,verbose) == Tokenizer::AttributeValue)
					strURI.append(strToken.c_str());
			}
			else if (strToken == "EDITION")
			{
				if (tok.next_token(strToken,verbose) == Tokenizer::AttributeValue)
					strEdition = strToken;
			}
			else if (strToken == "NAMESPACE")
			{
				if (tok.next_token(strToken,verbose) == Tokenizer::AttributeValue)
					strNamespace = strToken;
			}
		}
		else if (tok_type == Tokenizer::Text)
		{
			strText.append(strToken.c_str());
		}
		else if (tok_type == Tokenizer::ElementEnd && strToken == "TEST")
		{
			if (strNamespace == "no")
				printf("Skipping, test only applies to non-namespace parsers\n");
			else if (!strEdition.empty() && strEdition.find('5') == OOBase::LocalString::npos)
				printf("Skipping, test only applies to editions %s\n",strEdition.c_str());
			else
			{
				bool ret = false;
				if (strType == "valid")
					ret = do_valid_test(tok.get_allocator(),strURI);
				else if (strType == "invalid")
					ret = do_invalid_test(tok.get_allocator(),strURI);
				else if (strType == "not-wf")
					ret = do_not_wf_test(tok.get_allocator(),strURI);
				else if (strType == "error")
					ret = do_error_test(tok.get_allocator(),strURI);
				else
					return;

				if (!ret && !strText.empty())
					printf("%s\n\n",strText.c_str());
			}

			return;
		}
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);
}

static void do_test_cases(OOBase::AllocatorInstance& allocator, Tokenizer& tok, const OOBase::LocalString& strParent)
{
	OOBase::LocalString strBase = strParent;
	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::LocalString strToken(allocator);
		tok_type = tok.next_token(strToken,verbose);

		if (tok_type == Tokenizer::AttributeName)
		{
			if (strToken == "xml:base")
			{
				if (tok.next_token(strToken,verbose) == Tokenizer::AttributeValue)
					strBase.append(strToken.c_str());
			}
			else if (strToken == "PROFILE")
			{
				if (tok.next_token(strToken,verbose) == Tokenizer::AttributeValue)
					printf("\nRunning test cases: %s\n",strToken.c_str());
			}
		}
		else if (tok_type == Tokenizer::ElementStart)
		{
			if (strToken == "TESTCASES")
				do_test_cases(allocator,tok,strBase);
			else if (strToken == "TEST")
				do_test(allocator,tok,strBase);
		}
		else if (tok_type == Tokenizer::ElementEnd && strToken == "TESTCASES")
			return;
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);
}

static void do_test_suite(OOBase::AllocatorInstance& allocator, Tokenizer& tok, const OOBase::LocalString& strParent)
{
	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::LocalString strToken(allocator);
		tok_type = tok.next_token(strToken,verbose);

		if (tok_type == Tokenizer::AttributeName && strToken == "PROFILE")
		{
			if (tok.next_token(strToken,verbose) == Tokenizer::AttributeValue)
				printf("\nRunning suite: %s\n",strToken.c_str());
		}
		else if (tok_type == Tokenizer::ElementStart && strToken == "TESTCASES")
			do_test_cases(allocator,tok,strParent);
		else if (tok_type == Tokenizer::ElementEnd && strToken == "TESTSUITE")
			return;
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);
}

static void split_dir_and_fname(const char* path, OOBase::LocalString& dir, OOBase::LocalString& file)
{
	const char* s = strrchr(path,'/');
	if (!s)
	{
		dir.assign(path);
		if (dir.c_str()[dir.length()-1] != '/')
			dir.append("/",1);
	}
	else
	{
		dir.assign(path,s - path + 1);
		file.assign(s+1);
	}
}

int main( int argc, char* argv[] )
{
	//OOBase::StackAllocator<4096> allocator;
	OOBase::ArenaAllocator allocator;
	Tokenizer tok(allocator);

	OOBase::LocalString path(allocator),file(allocator);
	split_dir_and_fname(argv[1],path,file);

	OOBase::LocalString strLoad(allocator);
	strLoad.assign(argv[1]);

	tok.load(strLoad);

	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::LocalString strToken(allocator);
		tok_type = tok.next_token(strToken,verbose);

		if (tok_type == Tokenizer::ElementStart && strToken == "TESTSUITE")
			do_test_suite(allocator,tok,path);
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);

	if (tok_type == Tokenizer::Error)
		printf("\nSyntax error at %s, line %lu, col %lu\n",tok.get_location().c_str(),tok.get_line(),tok.get_column());

	printf("\n%lu passed, %lu failed\n",passed,failed);
		
	return 0;
}

OOBase::LocalString resolve_url(const OOBase::LocalString& strBase, const OOBase::LocalString& strPublicId, const OOBase::LocalString& strSystemId)
{
	OOBase::LocalString path(strBase.get_allocator()),file(strBase.get_allocator());
	split_dir_and_fname(strBase.c_str(),path,file);

	path.append(strSystemId.c_str());
	return path;
}
