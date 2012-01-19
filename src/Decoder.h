/*
 * Decoder.h
 *
 *  Created on: 19 Jan 2012
 *      Author: taylorr
 */

#ifndef DECODER_H_INCLUDED_
#define DECODER_H_INCLUDED_

class Decoder
{
public:
	enum eType
	{
		UTF32LE,
		UTF32BE,
		UTF16LE,
		UTF16BE,
		EBCDIC
	};

	static Decoder* create(eType type);

	virtual unsigned char next(unsigned char c) = 0;

protected:
	Decoder() {}

private:
	Decoder(const Decoder&);
	Decoder& operator =(const Decoder&);
};


#endif /* DECODER_H_INCLUDED_ */
