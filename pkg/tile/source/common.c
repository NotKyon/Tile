#include <tile/const.h>

/*
 * ==========================================================================
 *
 *	REPORT
 *
 * ==========================================================================
 */
void tlReportV(const char *type, const char *file, int line, const char *func,
const char *message, va_list args) {
	int e;

	e = errno;

	fflush(stdout);

	if (file) {
		fprintf(stderr, "[%s", file);
		if (line)
			fprintf(stderr, "(%i)", line);

		if (func)
			fprintf(stderr, " %s", func);

		fprintf(stderr, "] ");
	} else if(func)
		fprintf(stderr, "%s: ", func);

	fprintf(stderr, "%s: ", type);

	vfprintf(stderr, message, args);

	if (e) {
#if _MSC_VER
		char errbuf[4096];

		if (strerror_s(errbuf, sizeof(errbuf), e) != 0)
			errbuf[0] = '\0';

		fprintf(stderr, ": %s (%i)", errbuf, e);
#else
		fprintf(stderr, ": %s (%i)", strerror(e), e);
#endif
	}

	fprintf(stderr, "\n");
	fflush(stderr);
}
void tlReport(const char *type, const char *file, int line, const char *func,
const char *message, ...) {
	va_list args;

	va_start(args, message);
	tlReportV(type, file, line, func, message, args);
	va_end(args);
}

void tlWarn(const char *file, int line, const char *func, const char *message,
...) {
	va_list args;

	va_start(args, message);
	tlReportV(TL_TYPE_WRN, file, line, func, message, args);
	va_end(args);
}
void tlError(const char *file, int line, const char *func, const char *message,
...) {
	va_list args;

	va_start(args, message);
	tlReportV(TL_TYPE_ERR, file, line, func, message, args);
	va_end(args);
}

void tlWarnMessage(const char *message, ...) {
	va_list args;

	va_start(args, message);
	tlReportV(TL_TYPE_WRN, (const char *)0, 0, (const char *)0, message, args);
	va_end(args);
}
void tlErrorMessage(const char *message, ...) {
	va_list args;

	va_start(args, message);
	tlReportV(TL_TYPE_ERR, (const char *)0, 0, (const char *)0, message, args);
	va_end(args);
}

void tlWarnFile(const char *file, int line, const char *message, ...) {
	va_list args;

	va_start(args, message);
	tlReportV(TL_TYPE_WRN, file, line, (const char *)0, message, args);
	va_end(args);
}
void tlErrorFile(const char *file, int line, const char *message, ...) {
	va_list args;

	va_start(args, message);
	tlReportV(TL_TYPE_ERR, file, line, (const char *)0, message, args);
	va_end(args);
}

void tlErrorFileExit(const char *file, int line, const char *message, ...) {
	va_list args;

	va_start(args, message);
	tlReportV(TL_TYPE_ERR, file, line, (const char *)0, message, args);
	va_end(args);

	exit(EXIT_FAILURE);
}
void tlErrorExit(const char *message, ...) {
	va_list args;

	va_start(args, message);
	tlReportV(TL_TYPE_ERR, (const char *)0, 0, (const char *)0, message, args);
	va_end(args);

	exit(EXIT_FAILURE);
}

/*
 * ==========================================================================
 *
 *	MEMORY
 *
 * ==========================================================================
 */
void *tlAlloc(size_t n) {
	void *p;

	if (!n)
		return (void *)0;

	p = malloc(n);
	if (!p) {
		TL_BREAKPOINT();
		tlErrorExit("Failed to allocate memory");
	}

	return p;
}
void *tlAllocZero(size_t n) {
	void *p;

	p = calloc(1, n);
	if (!p) {
		TL_BREAKPOINT();
		tlErrorExit("Failed to allocate memory");
	}

	memset(p, 0, n);

	return p;
}
void *tlAllocArray(size_t nmemb, size_t size) {
	if( nmemb*size < nmemb ) {
		TL_BREAKPOINT();
		tlErrorExit("Requested too much memory");
	}

	if( !( p = tlAlloc(nmemb*size) ) ) {
		return (void *)0;
	}

	return p;
}
void *tlAllocArrayZero(size_t nmemb, size_t size) {
	if( nmemb*size < nmemb ) {
		TL_BREAKPOINT();
		tlErrorExit("Requested too much memory");
	}

	if( !( p = tlAllocZero(nmemb*size) ) ) {
		return (void *)0;
	}

	return p;
}
void *tlReallocArray(void *p, size_t nmemb, size_t size) {
#ifdef __OpenBSD__
	void *q;

	if( !( q = reallocarray(p, nmemb, size) ) ) {
		TL_BREAKPOINT();
		tlErrorExit("Failed to reallocate memory");
	}

	return q;
#else
	void *q;

	if( nmemb*size < nmemb ) {
		TL_BREAKPOINT();
		tlErrorExit("Requested too much memory");
	}

	if( !p ) {
		return tlAlloc( nmemb*size );
	}

	if( !( q = realloc(p, nmemb*size) ) ) {
		TL_BREAKPOINT();
		tlErrorExit("Failed to reallocate memory");
	}

	return q;
#endif
}
void *tlReallocArrayZero(void *p, size_t oldnmemb, size_t nmemb, size_t size) {
#ifdef __OpenBSD__
	void *q;

	if( !( q = recallocarray(p, oldnmemb, nmemb, size) ) ) {
		TL_BREAKPOINT();
		tlErrorExit("Failed to reallocate memory");
	}

	return q;
#else
	void *q;

	if( !p ) {
		return tlAllocArrayZero(nmemb, size);
	}

	if( nmemb*size < nmemb ) {
		TL_BREAKPOINT();
		tlErrorExit("Requested too much memory");
	}

	if( !( q = realloc(p, nmemb*size) ) ) {
		TL_BREAKPOINT();
		tlErrorExit("Failed to reallocate memory");
	}

	if( oldnmemb < nmemb ) {
		const size_t offset = oldnmemb*size;
		const size_t nzeros = ( nmemb - oldnmemb )*size;

		memset((void*)(((char*)q)+offset), 0, nzeros);
	}

	return q;
#endif
}
void *tlFree(void *p) {
	if (!p)
		return (void *)0;

	free(p);
	return (void *)0;
}

void *tlMemory(void *p, size_t n) {
	void *q;

	if (!p)
		return tlAlloc(n);

	if (!n)
		return tlFree(p);

	q = realloc(p, n);
	if (!q)
		tlErrorExit("Failed to reallocate memory");

	return q;
}

char *tlDuplicateN(const char *src, size_t srcn) {
	size_t l;
	char *p;

	if (!src)
		return (char *)0;

	l = srcn ? srcn : strlen(src);

	p = (char *)tlAlloc(l + 1);
	memcpy((void *)p, (const void *)src, l);
	p[l] = 0;

	return p;
}
char *tlDuplicate(const char *src) {
	return tlDuplicateN(src, 0);
}

char *tlCopyN(char *dst, const char *src, size_t srcn) {
	size_t len;

	if (!src)
		return (char *)tlFree((void *)dst);

	len = srcn ? srcn : strlen(src);

	dst = (char *)tlMemory((void *)dst, len + 1);
	memcpy((void *)dst, (const char *)src, len);
	dst[len] = 0;

	return dst;
}
char *tlCopy(char *dst, const char *src) {
	return tlCopyN(dst, src, 0);
}

char *tlAppendNChar(char *dst, const char *src, size_t srcn, char ch) {
	size_t l1, l2, l3;

	if (!dst)
		return tlDuplicateN(src, srcn);

	l1 = strlen(dst);
	l2 = srcn ? srcn : strlen(src);
	l3 = ch!='\0' ? 1 : 0;

	dst = (char *)tlMemory((void *)dst, l1 + l2 + l3 + 1);
	memcpy(&dst[l1], (const void *)src, l2);
	dst[l1 + l2] = ch;
	if (ch != '\0')
		dst[l1 + l2 + l3] = '\0';

	return dst;
}
char *tlAppendN(char *dst, const char *src, size_t srcn) {
	return tlAppendNChar(dst, src, srcn, '\0');
}
char *tlAppend(char *dst, const char *src) {
	return tlAppendN(dst, src, 0);
}

char *tlTrimAppendChar(char *dst, const char *src, char ch) {
	const char *p, *q;

	for(p=src; *p<=' ' && *p!='\0'; p++);

	q = strchr(p, '\0');

	while(q > p) {
		if (*q++ > ' ')
			break;
	}

	if (!(q - p))
		return dst;

	return tlAppendNChar(dst, p, q - p, ch);
}
char *tlTrimAppend(char *dst, const char *src) {
	return tlTrimAppendChar(dst, src, '\0');
}

char *tlStrCpy(char *dst, size_t dstn, const char *src) {
	return tlStrCpyN(dst, dstn, src, 0);
}
char *tlStrCpyN(char *dst, size_t dstn, const char *src, size_t srcn) {
	size_t l;
	char *r;

	if (!dst) {
		tlError(__FILE__,__LINE__,__func__, "NULL destination string passed.");
		return (char *)0;
	}

	if (!src) {
		tlError(__FILE__,__LINE__,__func__, "NULL source string passed.");
		return (char *)0;
	}

	if (dstn < 2) {
		tlError(__FILE__,__LINE__,__func__, "No room in destination string.");
		return (char *)0;
	}

	l = srcn ? srcn : strlen(src);

	if (l + 1 > dstn) {
		l = dstn - 1;
		r = (char *)0;

		tlWarn(__FILE__,__LINE__,__func__, "Overflow prevented.");
	} else
		r = dst;

	if (l)
		memcpy((void *)dst, (const void *)src, l);

	dst[l] = 0;

	return r;
}

char *tlStrCat(char *dst, size_t dstn, const char *src) {
	return tlStrCatN(dst, dstn, src, 0);
}
char *tlStrCatN(char *dst, size_t dstn, const char *src, size_t srcn) {
	char *p;

	p = strchr(dst, '\0');

	p = tlStrCpyN(p, dstn - (p - dst), src, srcn);
	if (p != (char *)0)
		p = dst;

	return p;
}

