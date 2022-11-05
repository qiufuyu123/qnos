#include"string.h"
#include"inttypes.h"
void strcpy(char *d, const char *s)
{
	while(*s) {
		*d++ = *s++;
	}
	*d = 0;
}

void strncpy(char *d, const char *s, unsigned length)
{
	while(*s && length--) {
		*d++ = *s++;
	}
	*d = 0;
}

int strcmp(const char *a, const char *b)
{
	while(1) {
		if(*a < *b) {
			return -1;
		} else if(*a > *b) {
			return 1;
		} else if(*a == 0) {
			return 0;
		} else {
			a++;
			b++;
		}
	}
}

int strncmp(const char *a, const char *b, unsigned length)
{
	while(length > 0) {
		if(*a < *b) {
			return -1;
		} else if(*a > *b) {
			return 1;
		} else if(*a == 0) {
			return 0;
		} else {
			a++;
			b++;
			length--;
		}
	}
	return 0;
}

unsigned strlen(const char *s)
{
	unsigned len = 0;
	while(*s) {
		len++;
		s++;
	}
	return len;
}

char *strrev(char *s)
{
	unsigned start = 0;
	unsigned end = strlen(s) - 1;
	char swap;

	while(start < end) {
		swap = s[start];
		s[start] = s[end];
		s[end] = swap;

		start++;
		end--;
	}

	return s;
}

char *strcat(char *d, const char *s)
{
	strcpy(d + strlen(d), s);
	return d;
}

const char *strchr(const char *s, char ch)
{
	while(*s) {
		if(*s == ch)
			return s;
		s++;
	}
	return 0;
}

char *strtok(char *s, const char *delim)
{
	static char *oldword = 0;
	char *word;

	if(!s)
		s = oldword;

	while(*s && strchr(delim, *s))
		s++;

	if(!*s) {
		oldword = s;
		return 0;
	}

	word = s;
	while(*s && !strchr(delim, *s))
		s++;

	if(*s) {
		*s = 0;
		oldword = s + 1;
	} else {
		oldword = s;
	}

	return word;
}


void strtoupper(char *name)
{
	while(*name) {
		if(*name >= 'a' && *name <= 'z') {
			*name -= 'a' - 'A';
		}
		name++;
	}
}

void strtolower(char *name)
{
	while(*name) {
		if(*name >= 'A' && *name <= 'Z') {
			*name += 'a' - 'A';
		}
		name++;
	}
}

int str2int(const char *s, int *d)
{
	int val = 0;
	for(; *s; ++s) {
		val *= 10;
		if(*s > '9' || *s < '0') {
			return 0;
		}
		val += (*s - '0');
	}
	*d = val;
	return 1;
}

void memset(void *vd, char value, unsigned length)
{
	char *d = vd;
	while(length) {
		*d = value;
		length--;
		d++;
	}
}

void memcpy(void *vd, const void *vs, unsigned length)
{
	char *d = vd;
	const char *s = vs;
	while(length) {
		*d = *s;
		d++;
		s++;
		length--;
	}
}

char *uint_to_string(uint32_t u, char *s)
{
	uint32_t f, d, i;

	f = 1;
	i = 0;
	while((u / (f * 10)) > 0) {
		f *= 10;
	}
	while(f > 0) {
		d = u / f;
		s[i] = '0' + d;
		u = u % f;
		f = f / 10;
		i++;
	}
	s[i] = 0;
	return s;
}
void *memclr(void *dest, int32_t count) {
    memset(dest, 0, count);
    return dest;
}
int32_t memcmp(const void *in1, const void *in2, uint32_t count) {
    const int8_t *ip1 = in1;
    const int8_t *ip2 = in2;
    for (; count; count--)
        if (*ip1++ != *ip2++)
            return 1;
    return 0;
}