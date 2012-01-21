/*
 * IO.h
 *
 *  Created on: 19 Jan 2012
 *      Author: taylorr
 */

#ifndef IO_H_
#define IO_H_

// This is a dummy
#include <stdlib.h>
#include <stdio.h>

class IO
{
public:
	IO();
	~IO();

	int open(const char* fname);
	unsigned char get_char();
	bool is_eof() const;

private:
	IO(const IO&);
	IO& operator = (const IO&);

	FILE* m_f;
	bool m_eof;
};

#endif /* IO_H_ */
