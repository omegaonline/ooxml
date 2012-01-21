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

int main( int argc, char* argv[] )
{
	Tokenizer tok;

	tok.load(argv[1]);

	Tokenizer::TokenType tok_type;
		
	do
	{
		OOBase::String strToken;
		tok_type = tok.next_token(strToken);

		printf("%d: %s\n",(int)tok_type,strToken.c_str());
	}
	while (tok_type != Tokenizer::End && tok_type != Tokenizer::Error);
		
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
