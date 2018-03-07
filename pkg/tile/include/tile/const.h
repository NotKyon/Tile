#ifndef TILE_CONST_H
#define TILE_CONST_H

#ifndef SHADERS_ENABLED
# define SHADERS_ENABLED 1
#endif

#ifndef GLFW_ENABLED
# ifdef _WIN32
#  define GLFW_ENABLED 0
# else
#  define GLFW_ENABLED 1
# endif
#endif

#if !GLFW_ENABLED && !defined( _WIN32 )
# error GLFW must be enabled on this platform.
#endif

#if _WIN32_WINNT < 0x0501
# define _WIN32_WINNT 0x0501
#endif

#if GLFW_ENABLED
# include <GLFW/glfw3.h>
#elif defined( _WIN32 )
# undef WIN32_LEAN_AND_MEAN /* prevent warning about redefine below */
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
# undef min
# undef max
#endif

#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glext.h>
#else
# include <GL/gl.h>
# include <GL/glext.h>
#endif

#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <fcntl.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <unistd.h>
# include <time.h>
# include <sys/time.h>
#endif

#if defined( __APPLE__ )
# include <mach/mach.h>
# include <mach/mach_time.h>
#endif

#ifdef __cplusplus
# define TILE_EXTRNC_ENTER extern "C" {
# define TILE_EXTRNC_LEAVE }
#else
# define TILE_EXTRNC_ENTER
# define TILE_EXTRNC_LEAVE
#endif


TILE_EXTRNC_ENTER

/*
 * ==========================================================================
 *
 *	TYPEDEFS
 *
 * ==========================================================================
 */

#define TL_VECTOR_SIZE    16
#define TL_CACHELINE_SIZE 64

#if 1
# if defined( _MSC_VER ) || defined( __INTEL_COMPILER )
#  define TL_VECTOR_ALIGNED    __declspec(align(TL_VECTOR_SIZE))
#  define TL_CACHELINE_ALIGNED __declspec(align(TL_CACHELINE_SIZE))
# else
#  define TL_VECTOR_ALIGNED    __attribute__((aligned(TL_VECTOR_SIZE)))
#  define TL_CACHELINE_ALIGNED __attribute__((aligned(TL_CACHELINE_SIZE)))
# endif
#else
# define TL_VECTOR_ALIGNED
# define TL_CACHELINE_ALIGNED
#endif

#if defined( _MSC_VER ) || defined( __INTEL_COMPILER )
typedef   signed __int64 TlSInt64;
typedef unsigned __int64 TlUInt64;
#else
# include <stdint.h>
typedef  int64_t     TlSInt64;
typedef uint64_t     TlUInt64;
#endif

typedef unsigned int TlUInt;

typedef signed char  TlS8;
typedef signed short TlS16;
typedef signed int   TlS32;
typedef TlSInt64       TlS64;

typedef unsigned char  TlU8;
typedef unsigned short TlU16;
typedef unsigned int   TlU32;
typedef TlUInt64         TlU64;

typedef unsigned int   TlBool;
#undef TRUE
#undef FALSE
#define TRUE           1
#define FALSE          0

typedef void(*TlFn_t)();
typedef union { TlFn_t fn; void *p; } TlFnPtr;

#ifndef bitfield_t_defined
# define bitfield_t_defined 1
typedef size_t bitfield_t;
#endif

#ifndef __has_builtin
# define __has_builtin(Builtin_) 0
#endif


/*
 * ==========================================================================
 *
 *	API
 *
 * ==========================================================================
 */

#ifndef TL_DEBUG_ENABLED
# if defined( AX_DEBUG_ENABLED )
#  define TL_DEBUG_ENABLED AX_DEBUG_ENABLED
# elif defined(_DEBUG) || defined( __DEBUG__ ) || defined( MK_DEBUG )
#  define TL_DEBUG_ENABLED 1
# else
#  define TL_DEBUG_ENABLED 0
# endif
#endif

#ifndef TL_ASSERT_ENABLED
# if TL_DEBUG_ENABLED
#  define TL_ASSERT_ENABLED 1
# else
#  define TL_ASSERT_ENABLED 0
# endif
#endif

/* Assert */
#ifdef _MSC_VER
# if _MSC_VER >= 1800
#  define __func__ __FUNCTION__
# endif
#endif

#define TL_GENERIC_BREAKPOINT() \
	do {\
		volatile int *p_ = (volatile int *)0x11;\
		*p = *p;\
	} while(0)

#ifndef TL_BREAKPOINT
# if __has_builtin(__builtin_trap) || ( defined( __GNUC__ ) && !defined( __clang__ ) )
#  define TL_BREAKPOINT() __builtin_trap()
# elif defined( _MSC_VER )
#  define TL_BREAKPOINT() __debugbreak()
# else
#  define TL_BREAKPOINT() TL_GENERIC_BREAKPOINT()
# endif
#endif

#ifndef TL_UNLIKELY
# if __has_builtin(__builtin_expect) || ( defined( __GNUC__ ) && !defined( __clang__ ) )
#  define TL_UNLIKELY(X_) (__builtin_expect(!!(X_), 0))
# else
#  define TL_UNLIKELY(X_) (X_)
# endif
#endif

#ifndef TL_ASSERT
# if TL_ASSERT_ENABLED
#  define TL_ASSERT(X_) \
	do {\
		if( TL_UNLIKELY( !( X_ ) ) ) {\
			tlError( __FILE__, __LINE__, __func__, "TL_ASSERT(%s) failed!", #X_ );\
			TL_BREAKPOINT();\
		}\
	} while(0)
# else
#  define TL_ASSERT(X_) do{((void)0);}while(0)
# endif
#endif

/* tlReport */
#define TL_TYPE_ERR "ERROR"
#define TL_TYPE_WRN "WARNING"

void tlReportV(const char *type, const char *file, int line, const char *func, const char *message, va_list args);
void tlReport(const char *type, const char *file, int line, const char *func, const char *message, ...);

void tlWarn(const char *file, int line, const char *func, const char *message, ...);
void tlError(const char *file, int line, const char *func, const char *message, ...);

void tlWarnMessage(const char *message, ...);
void tlErrorMessage(const char *message, ...);

void tlWarnFile(const char *file, int line, const char *message, ...);
void tlErrorFile(const char *file, int line, const char *message, ...);

void tlErrorFileExit(const char *file, int line, const char *message, ...);
void tlErrorExit(const char *message, ...);

/* tlMemory */
#define tlAllocStruct(x) ((x *)tlMemory((void *)0, sizeof(x)))
void *tlAlloc(size_t n);
void *tlAllocZero(size_t n);
void *tlAllocArray(size_t nmemb, size_t size);
void *tlAllocArrayZero(size_t nmemb, size_t size);
void *tlReallocArray(void *p, size_t nmemb, size_t size);
void *tlReallocArrayZero(void *p, size_t oldnmemb, size_t nmemb, size_t size);
void *tlFree(void *p);

void *tlMemory(void *p, size_t n);

char *tlDuplicateN(const char *src, size_t srcn);
char *tlDuplicate(const char *src);

char *tlCopyN(char *dst, const char *src, size_t srcn);
char *tlCopy(char *dst, const char *src);

char *tlAppendNChar(char *dst, const char *src, size_t srcn, char ch);
char *tlAppendN(char *dst, const char *src, size_t srcn);
char *tlAppend(char *dst, const char *src);

char *tlTrimAppendChar(char *dst, const char *src, char ch);
char *tlTrimAppend(char *dst, const char *src);

char *tlStrCpy(char *dst, size_t dstn, const char *src);
char *tlStrCpyN(char *dst, size_t dstn, const char *src, size_t srcn);

char *tlStrCat(char *dst, size_t dstn, const char *src);
char *tlStrCatN(char *dst, size_t dstn, const char *src, size_t srcn);

TILE_EXTRNC_LEAVE

#endif

