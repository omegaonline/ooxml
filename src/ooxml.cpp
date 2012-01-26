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

static size_t passed = 0;
static size_t failed = 0;

static bool do_test(const OOBase::String& strURI)
{
	Tokenizer tok;

	tok.load(strURI.c_str());

	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::String strToken;
		tok_type = tok.next_token(strToken);
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

static bool do_valid_test(const OOBase::String& strURI)
{
	if (!do_test(strURI))
		return fail();
	else
		return pass();
}

static bool do_invalid_test(const OOBase::String& strURI)
{
	if (!do_test(strURI))
		return fail();
	else
		return pass();
}

static bool do_not_wf_test(const OOBase::String& strURI)
{
	if (do_test(strURI))
		return fail();
	else
		return pass();
}

static bool do_error_test(const OOBase::String& strURI)
{
	if (do_test(strURI))
		return fail();
	else
		return pass();
}

static void do_test(Tokenizer& tok, const OOBase::String& strBase)
{
	OOBase::String strType,strText;
	OOBase::String strURI = strBase;
	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::String strToken;
		tok_type = tok.next_token(strToken);

		if (tok_type == Tokenizer::AttributeName)
		{
			if (strToken == "ID")
			{
				if (tok.next_token(strToken) == Tokenizer::AttributeValue)
					printf("Test: %s...",strToken.c_str());
			}
			else if (strToken == "TYPE")
			{
				if (tok.next_token(strToken) == Tokenizer::AttributeValue)
					strType = strToken;
			}
			else if (strToken == "URI")
			{
				if (tok.next_token(strToken) == Tokenizer::AttributeValue)
					strURI.append(strToken.c_str());
			}
		}
		else if (tok_type == Tokenizer::Text)
		{
			strText.append(strToken.c_str());
		}
		else if (tok_type == Tokenizer::ElementEnd && strToken == "TEST")
		{
			bool ret = false;
			if (strType == "valid")
				ret = do_valid_test(strURI);
			else if (strType == "invalid")
				ret = do_invalid_test(strURI);
			else if (strType == "not-wf")
				ret = do_not_wf_test(strURI);
			else if (strType == "error")
				ret = do_error_test(strURI);
			else
				return;

			if (!ret && !strText.empty())
				printf("%s\n",strText.c_str());

			return;
		}
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);
}

static void do_test_cases(Tokenizer& tok, const OOBase::String& strParent)
{
	OOBase::String strBase = strParent;
	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::String strToken;
		tok_type = tok.next_token(strToken);

		if (tok_type == Tokenizer::AttributeName)
		{
			if (strToken == "xml:base")
			{
				if (tok.next_token(strToken) == Tokenizer::AttributeValue)
					strBase.append(strToken.c_str());
			}
			else if (strToken == "PROFILE")
			{
				if (tok.next_token(strToken) == Tokenizer::AttributeValue)
					printf("\nRunning test cases: %s\n",strToken.c_str());
			}
		}
		else if (tok_type == Tokenizer::ElementStart)
		{
			if (strToken == "TESTCASES")
				do_test_cases(tok,strBase);
			else if (strToken == "TEST")
				do_test(tok,strBase);
		}
		else if (tok_type == Tokenizer::ElementEnd && strToken == "TESTCASES")
			return;
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);
}

static void do_test_suite(Tokenizer& tok, const OOBase::String& strParent)
{
	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::String strToken;
		tok_type = tok.next_token(strToken);

		if (tok_type == Tokenizer::AttributeName && strToken == "PROFILE")
		{
			if (tok.next_token(strToken) == Tokenizer::AttributeValue)
				printf("\nRunning suite: %s\n",strToken.c_str());
		}
		else if (tok_type == Tokenizer::ElementStart && strToken == "TESTCASES")
			do_test_cases(tok,strParent);
		else if (tok_type == Tokenizer::ElementEnd && strToken == "TESTSUITE")
			return;
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);
}

int main( int argc, char* argv[] )
{
	OOBase::String path,file;
	OOBase::Paths::SplitDirAndFilename(argv[1],path,file);
	OOBase::Paths::AppendDirSeparator(path);

	Tokenizer tok;

	tok.load(argv[1]);

	Tokenizer::TokenType tok_type;
	do
	{
		OOBase::String strToken;
		tok_type = tok.next_token(strToken,0);

		if (tok_type == Tokenizer::ElementStart && strToken == "TESTSUITE")
			do_test_suite(tok,path);
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);

	printf("\n%lu passed, %lu failed\n",passed,failed);
		
	return 0;
}

OOBase::String resolve_url(const OOBase::String& strBase, const OOBase::String& strPublicId, const OOBase::String& strSystemId)
{
	OOBase::String path,file;
	OOBase::Paths::SplitDirAndFilename(strBase.c_str(),path,file);
	OOBase::Paths::AppendDirSeparator(path);
	path.append(strSystemId.c_str());
	return path;
}
