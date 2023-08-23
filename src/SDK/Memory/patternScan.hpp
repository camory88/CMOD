#ifndef PATTERNSCANNER_HPP
#define PATTERNSCANNER_HPP


#include <cstring>

typedef uint8_t *PBYTE;
typedef uint8_t BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef WORD *PWORD;

#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) (INRANGE(x, '0', '9') ? (x - '0') : ((x & (~0x20)) - 'A' + 0xa))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))

inline bool isMatch(const PBYTE addr, const PBYTE pat, const PBYTE msk)
{
	size_t n = 0;
	while (addr[n] == pat[n] || msk[n] == (BYTE)'?')
	{
		if (!msk[++n])
		{
			return true;
		}
	}
	return false;
}


// Credits: learn_more, stevemk14ebr
inline size_t findPattern(const PBYTE rangeStart, size_t len, const char *pattern)
{
	size_t l = strlen(pattern);
	PBYTE patt_base = static_cast<PBYTE>(malloc(l >> 1));
	PBYTE msk_base = static_cast<PBYTE>(malloc(l >> 1));
	PBYTE pat = patt_base;
	PBYTE msk = msk_base;
	if (pat && msk)
	{
		l = 0;
		while (*pattern)
		{
			if (*pattern == ' ')
				pattern++;
			if (!*pattern)
				break;
			if (*(PBYTE)pattern == (BYTE)'\?')
			{
				*pat++ = 0;
				*msk++ = '?';
				pattern += ((*(PWORD)pattern == (WORD)'\?\?') ? 2 : 1);
			}
			else
			{
				*pat++ = getByte(pattern);
				*msk++ = 'x';
				pattern += 2;
			}
			l++;
		}
		*msk = 0;
		pat = patt_base;
		msk = msk_base;
		for (size_t n = 0; n < (len - l); ++n)
		{
			if (isMatch(rangeStart + n, patt_base, msk_base))
			{
				free(patt_base);
				free(msk_base);
				return n;
			}
		}
		free(patt_base);
		free(msk_base);
	}
	return 0;
}


#endif // PATTERNSCANNER_HPP