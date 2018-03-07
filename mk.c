/*
 *	Mk
 *	Written by NotKyon (Aaron J. Miller) <nocannedmeat@gmail.com>, 2012-2016.
 *	Contributions from James P. O'Hagan <johagan12@gmail.com>, 2012-2016.
 *
 *	This tool is used to simplify the build process of simple interdependent
 *	programs/libraries that need to be (or would benefit from being) distributed
 *	without requiring make/scons/<other builder program>.
 *
 *	The entire program is written in one source file to make it easier to build.
 *	To build this program in debug mode, run the following in your terminal:
 *	$ gcc -g -D_DEBUG -W -Wall -pedantic -std=c99 -o mk mk.c
 *
 *	To build this program in release mode, run the following:
 *	$ gcc -O3 -fomit-frame-pointer -W -Wall -pedantic -std=c99 -o mk mk.c
 *
 *	Also, I apologize for the quality (or rather, the lack thereof) of the code.
 *	I intend to improve it over time, make some function names more consistent,
 *	and provide better internal source code documentation.
 *
 *	I did not attempt to compile with VC++. I do not believe "%zu" works in VC,
 *	so you may have to modify as necessary. Under GCC, I get absolutely no
 *	warnings for ISO C99; no extensions used.
 *
 *	LICENSE
 *	=======
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MK_VERSION_MAJOR 0
#define MK_VERSION_MINOR 7
#define MK_VERSION_PATCH 2

#define MK_VERSION ( MK_VERSION_MAJOR*10000 + MK_VERSION_MINOR*100 + MK_VERSION_PATCH )

#define MK_PPTOKSTR__IMPL(X_) #X_
#define MK_PPTOKSTR(X_) MK_PPTOKSTR__IMPL(X_)

#define MK_VERSION_STR MK_PPTOKSTR(MK_VERSION_MAJOR) "." MK_PPTOKSTR(MK_VERSION_MINOR) "." MK_PPTOKSTR(MK_VERSION_PATCH)

#ifndef MK_WINDOWS_ENABLED
# ifdef _WIN32
#  define MK_WINDOWS_ENABLED 1
# else
#  define MK_WINDOWS_ENABLED 0
# endif
#endif

#ifndef MK_WINDOWS_COLORS_ENABLED
# if MK_WINDOWS_ENABLED
#  define MK_WINDOWS_COLORS_ENABLED 1
# else
#  define MK_WINDOWS_COLORS_ENABLED 0
# endif
#endif
#if !MK_WINDOWS_ENABLED
# undef MK_WINDOWS_COLORS_ENABLED
# define MK_WINDOWS_COLORS_ENABLED 0
#endif

#ifdef _MSC_VER
# define MK_VC_VER _MSC_VER
#else
# define MK_VC_VER 0
#endif

#ifndef MK_HAS_DIRENT
# if !MK_VC_VER
#  define MK_HAS_DIRENT 1
# else
#  define MK_HAS_DIRENT 0
# endif
#endif
#ifndef MK_HAS_UNISTD
# if !MK_VC_VER
#  define MK_HAS_UNISTD 1
# else
#  define MK_HAS_UNISTD 0
# endif
#endif

#ifndef MK_HAS_EXECINFO
# if !MK_WINDOWS_ENABLED
#  define MK_HAS_EXECINFO 1
# else
#  define MK_HAS_EXECINFO 0
# endif
#endif

#ifndef MK_HAS_PWD
# if defined( __APPLE__ ) || defined( __linux__ ) || defined( __unix__ )
#  define MK_HAS_PWD 1
# else
#  define MK_HAS_PWD 0
# endif
#endif

#ifndef MK_PWD_HOMEPATH_DETECTION_ENABLED
# define MK_PWD_HOMEPATH_DETECTION_ENABLED 1
#endif
#if !MK_HAS_PWD
# undef  MK_PWD_HOMEPATH_DETECTION_ENABLED
# define MK_PWD_HOMEPATH_DETECTION_ENABLED 0
#endif


#ifndef MK_SECLIB
# if defined( __STDC_WANT_SECURE_LIB__ ) && defined( _MSC_VER )
#  define MK_SECLIB 1
# else
#  define MK_SECLIB 0
# endif
#endif

#if MK_WINDOWS_ENABLED
# if MK_VC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
# endif
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h>
#endif
#include <time.h>
#include <errno.h>
#include <stdio.h>
#if MK_HAS_DIRENT
# include <dirent.h>
#else
# include "dirent.h" /* user-provided replacement */
#endif
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#if MK_HAS_UNISTD
# include <unistd.h>
#else
# include <io.h>
# include <direct.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#if MK_HAS_EXECINFO
# include <execinfo.h>
#endif
#if MK_HAS_PWD
# include <pwd.h>
#endif

#ifndef MK_DEBUG_ENABLED
# if defined(_DEBUG)||defined(DEBUG)||defined(__debug__)||defined(__DEBUG__)
#  define MK_DEBUG_ENABLED 1
# else
#  define MK_DEBUG_ENABLED 0
# endif
#endif

#ifndef S_IFREG
# define S_IFREG 0100000
#endif
#ifndef S_IFDIR
# define S_IFDIR 0040000
#endif

#ifndef PATH_MAX
# if defined( _MAX_PATH )
#  define PATH_MAX _MAX_PATH
# elif defined( MAX_PATH )
#  define PATH_MAX MAX_PATH
# else
#  define PATH_MAX 512
# endif
#endif

#ifndef bitfield_t_defined
# define bitfield_t_defined 1
typedef size_t bitfield_t;
#endif

#ifndef MK_NORETURN
# if MK_VC_VER
#  define MK_NORETURN _declspec(noreturn)
# else
#  define MK_NORETURN __attribute__((noreturn))
# endif
#endif

#ifndef MK_CURFUNC
# if MK_VC_VER
#  define MK_CURFUNC __FUNCTION__
# else
#  define MK_CURFUNC __func__
# endif
#endif

#define MK_PPTOKCAT__IMPL(X,Y) X##Y
#define MK_PPTOKCAT(X,Y) MK_PPTOKCAT__IMPL(X,Y)

/*
================
MK_PROCESS_NEWLINE_CONCAT_ENABLED

Define to 1 to enable newline concatenation through the backslash character.
Example:

	INPUT
	-----
	Test\
	Text

	OUTPUT
	------
	TestText

================
*/
#ifndef MK_PROCESS_NEWLINE_CONCAT_ENABLED
# define MK_PROCESS_NEWLINE_CONCAT_ENABLED 1
#endif

/*

	TODO: bin, bin64
	TODO: "deploy" build to build both 32-bit and 64-bit (bin and bin64)

*/



#define MK_HOST_OS_MSWIN  0
#define MK_HOST_OS_CYGWIN 0
#define MK_HOST_OS_MACOSX 0
#define MK_HOST_OS_LINUX  0
#define MK_HOST_OS_UNIX   0

#if MK_WINDOWS_ENABLED
# if defined( __CYGWIN__ )
#  undef  MK_HOST_OS_CYGWIN
#  define MK_HOST_OS_CYGWIN 1
# else
#  undef  MK_HOST_OS_MSWIN
#  define MK_HOST_OS_MSWIN 1
# endif
#elif defined( __linux__ ) || defined( __linux )
# undef  MK_HOST_OS_LINUX
# define MK_HOST_OS_LINUX 1
#elif defined( __APPLE__ ) && defined( __MACH__ )
# undef  MK_HOST_OS_MACOSX
# define MK_HOST_OS_MACOSX 1
#elif defined( __unix__ ) || defined( __unix )
# undef  MK_HOST_OS_UNIX
# define MK_HOST_OS_UNIX 1
#else
# error "Unrecognized host OS."
#endif

#define MK_HOST_CPU_X86     0
#define MK_HOST_CPU_X86_64  0
#define MK_HOST_CPU_ARM     0
#define MK_HOST_CPU_AARCH64 0
#define MK_HOST_CPU_PPC     0
#define MK_HOST_CPU_MIPS    0

#if defined( __amd64__ ) || defined( __x86_64__ ) || defined( _M_X64 )
# undef  MK_HOST_CPU_X86_64
# define MK_HOST_CPU_X86_64 1
#elif defined( __i386__ ) || defined( _M_IX86 )
# undef  MK_HOST_CPU_X86
# define MK_HOST_CPU_X86 1
#elif defined( __aarch64__ )
# undef  MK_HOST_CPU_AARCH64
# define MK_HOST_CPU_AARCH64 1
#elif defined( __arm__ ) || defined( _ARM )
# undef  MK_HOST_CPU_ARM
# define MK_HOST_CPU_ARM 1
#elif defined( __powerpc__ ) || defined( __POWERPC__ ) || defined( __ppc__ )
# undef  MK_HOST_CPU_PPC
# define MK_HOST_CPU_PPC 1
#elif defined( _M_PPC ) || defined( _ARCH_PPC )
# undef  MK_HOST_CPU_PPC
# define MK_HOST_CPU_PPC 1
#elif defined( __mips__ ) || defined( __MIPS__ )
# undef  MK_HOST_CPU_MIPS
# define MK_HOST_CPU_MIPS 1
#else
# error "Unrecognized host CPU."
#endif


/*
===============================================================================

	Configuration Settings

===============================================================================
*/
#ifndef MK_DEFAULT_OBJDIR_BASE
# define MK_DEFAULT_OBJDIR_BASE ".mk-obj"
#endif

#ifndef MK_DEFAULT_DEBUG_SUFFIX
# define MK_DEFAULT_DEBUG_SUFFIX "-dbg"
#endif

#ifndef MK_DEFAULT_COLOR_MODE
# if MK_WINDOWS_COLORS_ENABLED
#  define MK_DEFAULT_COLOR_MODE Windows
# else
#  define MK_DEFAULT_COLOR_MODE ANSI
# endif
#endif
#define MK__DEFAULT_COLOR_MODE_IMPL MK_PPTOKCAT(kMkColorMode_, MK_DEFAULT_COLOR_MODE)

#ifndef MK_DEFAULT_RELEASE_CONFIG_NAME
# define MK_DEFAULT_RELEASE_CONFIG_NAME "release"
#endif
#ifndef MK_DEFAULT_DEBUG_CONFIG_NAME
# define MK_DEFAULT_DEBUG_CONFIG_NAME "debug"
#endif

/* Windows */
#ifndef MK_PLATFORM_OS_NAME_MSWIN
# define MK_PLATFORM_OS_NAME_MSWIN "mswin"
#endif
/* Universal Windows Program */
#ifndef MK_PLATFORM_OS_NAME_UWP
# define MK_PLATFORM_OS_NAME_UWP "uwp"
#endif
/* Windows (via Cygwin) */
#ifndef MK_PLATFORM_OS_NAME_CYGWIN
# define MK_PLATFORM_OS_NAME_CYGWIN "cygwin"
#endif
/* Mac OS X */
#ifndef MK_PLATFORM_OS_NAME_MACOSX
# define MK_PLATFORM_OS_NAME_MACOSX "apple"
#endif
/* (GNU/)Linux */
#ifndef MK_PLATFORM_OS_NAME_LINUX
# define MK_PLATFORM_OS_NAME_LINUX "linux"
#endif
/* Some UNIX variant (e.g., FreeBSD, Solaris) */
#ifndef MK_PLATFORM_OS_NAME_UNIX
# define MK_PLATFORM_OS_NAME_UNIX "unix"
#endif

/* x86 (32-bit) */
#ifndef MK_PLATFORM_CPU_NAME_X86
# define MK_PLATFORM_CPU_NAME_X86 "x86"
#endif
/* x86-64 (64-bit) */
#ifndef MK_PLATFORM_CPU_NAME_X64
# define MK_PLATFORM_CPU_NAME_X64 "x64"
#endif
/* ARM (32-bit) */
#ifndef MK_PLATFORM_CPU_NAME_ARM
# define MK_PLATFORM_CPU_NAME_ARM "arm"
#endif
/* ARM / AArch64 (64-bit) */
#ifndef MK_PLATFORM_CPU_NAME_AARCH64
# define MK_PLATFORM_CPU_NAME_AARCH64 "aarch64"
#endif
/* PowerPC (generic) */
#ifndef MK_PLATFORM_CPU_NAME_PPC
# define MK_PLATFORM_CPU_NAME_PPC "ppc"
#endif
/* MIPS (generic) */
#ifndef MK_PLATFORM_CPU_NAME_MIPS
# define MK_PLATFORM_CPU_NAME_MIPS "mips"
#endif
/* WebAssembly */
#ifndef MK_PLATFORM_CPU_NAME_WASM
# define MK_PLATFORM_CPU_NAME_WASM "wasm"
#endif

#ifndef MK_PLATFORM_OS_NAME
# if MK_HOST_OS_MSWIN
#  define MK_PLATFORM_OS_NAME MK_PLATFORM_OS_NAME_MSWIN
# elif MK_HOST_OS_CYGWIN
#  define MK_PLATFORM_OS_NAME MK_PLATFORM_OS_NAME_CYGWIN
# elif MK_HOST_OS_LINUX
#  define MK_PLATFORM_OS_NAME MK_PLATFORM_OS_NAME_LINUX
# elif MK_HOST_OS_MACOSX
#  define MK_PLATFORM_OS_NAME MK_PLATFORM_OS_NAME_MACOSX
# elif MK_HOST_OS_UNIX
#  define MK_PLATFORM_OS_NAME MK_PLATFORM_OS_NAME_UNIX
# else
#  error "MK_PLATFORM_OS_NAME: Unhandled OS."
# endif
#endif

#ifndef MK_PLATFORM_CPU_NAME
# if MK_HOST_CPU_X86
#  define MK_PLATFORM_CPU_NAME MK_PLATFORM_CPU_NAME_X86
# elif MK_HOST_CPU_X86_64
#  define MK_PLATFORM_CPU_NAME MK_PLATFORM_CPU_NAME_X64
# elif MK_HOST_CPU_ARM
#  define MK_PLATFORM_CPU_NAME MK_PLATFORM_CPU_NAME_ARM
# elif MK_HOST_CPU_AARCH64
#  define MK_PLATFORM_CPU_NAME MK_PLATFORM_CPU_NAME_AARCH64
# elif MK_HOST_CPU_PPC
#  define MK_PLATFORM_CPU_NAME MK_PLATFORM_CPU_NAME_PPC
# elif MK_HOST_CPU_MIPS
#  define MK_PLATFORM_CPU_NAME MK_PLATFORM_CPU_NAME_MIPS
# else
#  error "MK_PLATFORM_CPU_NAME: Unhandled CPU."
# endif
#endif

#ifndef MK_PLATFORM_DIR
# define MK_PLATFORM_DIR MK_PLATFORM_OS_NAME "-" MK_PLATFORM_CPU_NAME
#endif

/* -------------------------------------------------------------------------- */
void mk_dbg_out(const char *str);
void mk_dbg_outfv(const char *format, va_list args);
void mk_dbg_outf(const char *format, ...);

void mk_dbg_enter(const char *format, ...);
void mk_dbg_leave(void);

void mk_dbg_backtrace(void);

/* -------------------------------------------------------------------------- */
typedef enum {
	kMkSIO_Out,
	kMkSIO_Err,

	kMkNumSIO
} MkSIO_t;

#define MK_S_COLOR_BLACK         "^0"
#define MK_S_COLOR_BLUE          "^1"
#define MK_S_COLOR_GREEN         "^2"
#define MK_S_COLOR_CYAN          "^3"
#define MK_S_COLOR_RED           "^4"
#define MK_S_COLOR_MAGENTA       "^5"
#define MK_S_COLOR_BROWN         "^6"
#define MK_S_COLOR_LIGHT_GREY    "^7"
#define MK_S_COLOR_DARK_GREY     "^8"
#define MK_S_COLOR_LIGHT_BLUE    "^9"
#define MK_S_COLOR_LIGHT_GREEN   "^A"
#define MK_S_COLOR_LIGHT_CYAN    "^B"
#define MK_S_COLOR_LIGHT_RED     "^C"
#define MK_S_COLOR_LIGHT_MAGENTA "^D"
#define MK_S_COLOR_LIGHT_BROWN   "^E"
#define MK_S_COLOR_WHITE         "^F"

#define MK_S_COLOR_PURPLE        "^5"
#define MK_S_COLOR_YELLOW        "^E"
#define MK_S_COLOR_VIOLET        "^D"

#define MK_S_COLOR_RESTORE       "^&"

#define MK_COLOR_BLACK           0x0
#define MK_COLOR_BLUE            0x1
#define MK_COLOR_GREEN           0x2
#define MK_COLOR_CYAN            0x3
#define MK_COLOR_RED             0x4
#define MK_COLOR_MAGENTA         0x5
#define MK_COLOR_BROWN           0x6
#define MK_COLOR_LIGHT_GREY      0x7
#define MK_COLOR_DARK_GREY       0x8
#define MK_COLOR_LIGHT_BLUE      0x9
#define MK_COLOR_LIGHT_GREEN     0xA
#define MK_COLOR_LIGHT_CYAN      0xB
#define MK_COLOR_LIGHT_RED       0xC
#define MK_COLOR_LIGHT_MAGENTA   0xD
#define MK_COLOR_LIGHT_BROWN     0xE
#define MK_COLOR_WHITE           0xF

#define MK_COLOR_PURPLE          0x5
#define MK_COLOR_YELLOW          0xE
#define MK_COLOR_VIOLET          0xD

int mk_sys_isColoredOutputEnabled(void);
void mk_sys_initColoredOutput(void);

unsigned char mk_sys_getCurrColor(MkSIO_t sio);
void mk_sys_setCurrColor(MkSIO_t sio, unsigned char color);
void mk_sys_uncoloredPuts(MkSIO_t sio, const char *text, size_t len);
int mk_sys_charToColorCode(char c);
void mk_sys_puts(MkSIO_t sio, const char *text);
void mk_sys_printf(MkSIO_t sio, const char *format, ...);

void mk_sys_printStr(MkSIO_t sio, unsigned char color, const char *str);
void mk_sys_printUint(MkSIO_t sio, unsigned char color, unsigned int val);
void mk_sys_printInt(MkSIO_t sio, unsigned char color, int val);

/* -------------------------------------------------------------------------- */
void mk_log_errorMsg(const char *message);
MK_NORETURN void mk_log_fatalError(const char *message);

void mk_log_error(const char *file, unsigned int line, const char *func, const char *message);
void mk_log_errorAssert(const char *file, unsigned int line, const char *func, const char *message);

/* macros for MK_ASSERT errors */
#if MK_DEBUG_ENABLED
# undef MK_ASSERT
# undef MK_ASSERT_MSG
# define MK_ASSERT_MSG(x,m) \
	do {\
		if( !(x) ) {\
			mk_log_errorAssert( __FILE__, __LINE__, MK_CURFUNC, m );\
		}\
	} while(0)
# define MK_ASSERT(x) \
	MK_ASSERT_MSG((x),"MK_ASSERT(" #x ") failed!" )
#else
# undef MK_ASSERT
# undef MK_ASSERT_MSG
# define MK_ASSERT(x)       /*nothing*/
# define MK_ASSERT_MSG(x,m) /*nothing*/
#endif

/* -------------------------------------------------------------------------- */
#ifndef MK_MEM_LOCTRACE_ENABLED
# if MK_DEBUG_ENABLED
#  define MK_MEM_LOCTRACE_ENABLED 1
# else
#  define MK_MEM_LOCTRACE_ENABLED 0
# endif
#endif

#ifndef MK_LOG_MEMORY_ALLOCS_ENABLED
# define MK_LOG_MEMORY_ALLOCS_ENABLED 0
#endif

typedef void( *MkMem_Fini_fn_t )( void * );

enum
{
	/* do not zero out the memory (caller *WILL* initialize it immediately) */
	kMkMemF_Uninitialized = 1<<0
};

#define mk_mem_maybeAllocEx(cBytes_,uFlags_) (mk_mem__maybeAlloc((cBytes_),(uFlags_),__FILE__,__LINE__,MK_CURFUNC))
#define mk_mem_maybeAlloc(cBytes_)           (mk_mem__maybeAlloc((cBytes_),0,__FILE__,__LINE__,MK_CURFUNC))
#define mk_mem_allocEx(cBytes_,uFlags_)      (mk_mem__alloc((cBytes_),(uFlags_),__FILE__,__LINE__,MK_CURFUNC))
#define mk_mem_alloc(cBytes_)                (mk_mem__alloc((cBytes_),0,__FILE__,__LINE__,MK_CURFUNC))
#define mk_mem_dealloc(pBlock_)              (mk_mem__dealloc((void*)(pBlock_),__FILE__,__LINE__,MK_CURFUNC))
#define mk_mem_addRef(pBlock_)               (mk_mem__addRef((void*)(pBlock_),__FILE__,__LINE__,MK_CURFUNC))
#define mk_mem_attach(pBlock_,pSuperBlock_)  (mk_mem__attach((void*)(pBlock_),(void*)(pSuperBlock_),__FILE__,__LINE__,MK_CURFUNC))
#define mk_mem_detach(pBlock_)               (mk_mem__detach((void*)(pBlock_),__FILE__,__LINE__,MK_CURFUNC))
#define mk_mem_setFini(pBlock_,pfnFini_)     (mk_mem__setFini((void*)(pBlock_),(MkMem_Fini_fn_t)(pfnFini_)))
#define mk_mem_size(pBlock_)                 (mk_mem__size((const void*)(pBlock_)))

void *mk_mem__maybeAlloc( size_t cBytes, bitfield_t uFlags, const char *pszFile, unsigned int uLine, const char *pszFunction );
void *mk_mem__alloc( size_t cBytes, bitfield_t uFlags, const char *pszFile, unsigned int uLine, const char *pszFunction );
void *mk_mem__dealloc( void *pBlock, const char *pszFile, unsigned int uLine, const char *pszFunction );
void *mk_mem__addRef( void *pBlock, const char *pszFile, unsigned int uLine, const char *pszFunction );
void *mk_mem__attach( void *pBlock, void *pSuperBlock, const char *pszFile, unsigned int uLine, const char *pszFunction );
void *mk_mem__detach( void *pBlock, const char *pszFile, unsigned int uLine, const char *pszFunction );
void *mk_mem__setFini( void *pBlock, MkMem_Fini_fn_t pfnFini );
size_t mk_mem__size( const void *pBlock );

/* -------------------------------------------------------------------------- */
#ifndef MK_VA_MINSIZE
# define MK_VA_MINSIZE 1024
#endif

#define mk_com_memory(p_,n_) mk_com__memory((void*)(p_),(size_t)(n_),__FILE__,__LINE__,MK_CURFUNC)
#define mk_com_strdup(cstr_) mk_com__strdup((cstr_),__FILE__,__LINE__,MK_CURFUNC)

void *mk_com__memory( void *p, size_t n, const char *pszFile, unsigned int uLine, const char *pszFunction );
const char *mk_com_va(const char *format, ...);
size_t mk_com_strlen(const char *src);
size_t mk_com_strcat(char *buf, size_t bufn, const char *src);
size_t mk_com_strncat(char *buf, size_t bufn, const char *src, size_t srcn);
void mk_com_strcpy(char *dst, size_t dstn, const char *src);
void mk_com_strncpy(char *buf, size_t bufn, const char *src, size_t srcn);
char *mk_com__strdup(const char *cstr,const char *file,unsigned int line,const char *func);

char *mk_com_dup(char *dst, const char *src);
char *mk_com_dupn(char *dst, const char *src, size_t numchars);
char *mk_com_append(char *dst, const char *src);
const char *mk_com_extractDir(char *buf, size_t n, const char *filename);
void mk_com_substExt(char *dst, size_t dstn, const char *src, const char *ext);
int mk_com_shellf(const char *format, ...);
int mk_com_matchPath(const char *rpath, const char *apath);
int mk_com_getIntDate(void);

const char *mk_com_findArgEnd(const char *arg);
int mk_com_matchArg(const char *a, const char *b);
const char *mk_com_skipArg(const char *arg);
void mk_com_stripArgs(char *dst, size_t n, const char *src);
int mk_com_cmpPathChar(char a, char b);

int mk_com_relPath(char *dst, size_t dstn, const char *curpath, const char *abspath);
int mk_com_relPathCWD(char *dst, size_t dstn, const char *abspath);

const char *mk_com_getenv(char *dst, size_t dstn, const char *var);
int mk_com_strstarts(const char *src, const char *with);
int mk_com_strends(const char *src, const char *with);

void mk_com_fixpath(char *path);

/* -------------------------------------------------------------------------- */
#ifndef MK_DEFAULT_DEBUGLOG_FILENAME
# define MK_DEFAULT_DEBUGLOG_FILENAME "mk-debug.log"
#endif

typedef enum
{
	kMkColorMode_None,
	kMkColorMode_ANSI,
#if MK_WINDOWS_COLORS_ENABLED
	kMkColorMode_Windows,
#endif

	kMkNumColorModes
} MkColorMode_t;

const char *mk_opt_getObjdirBase(void);
const char *mk_opt_getDebugLogPath(void);
const char *mk_opt_getDebugSuffix(void);
MkColorMode_t mk_opt_getColorMode(void);
const char *mk_opt_getConfigName(void);

const char *mk_opt_getBuildGenPath(void);
const char *mk_opt_getBuildGenIncDir(void);

const char *mk_opt_getGlobalDir(void);
const char *mk_opt_getGlobalSharedDir(void);
const char *mk_opt_getGlobalVersionDir(void);

/* -------------------------------------------------------------------------- */
typedef struct MkStrList_s *MkStrList;

MkStrList mk_sl_new(void);
size_t mk_sl_getCapacity(MkStrList arr);
size_t mk_sl_getSize(MkStrList arr);
char **mk_sl_getData(MkStrList arr);
char *mk_sl_at(MkStrList arr, size_t i);
void mk_sl_set(MkStrList arr, size_t i, const char *cstr);
void mk_sl_clear(MkStrList arr);
void mk_sl_delete(MkStrList arr);
void mk_sl_deleteAll(void);
void mk_sl_resize(MkStrList arr, size_t n);
void mk_sl_pushBack(MkStrList arr, const char *cstr);
void mk_sl_popBack(MkStrList arr);
void mk_sl_print(MkStrList arr);
void mk_sl_debugPrint(MkStrList arr);
void mk_sl_sort(MkStrList arr);

void mk_sl_orderedSort(MkStrList arr, size_t *const buffer, size_t maxBuffer);
void mk_sl_indexedSort(MkStrList arr, const size_t *const buffer, size_t bufferLen);
void mk_sl_printOrderedBuffer(const size_t *const buffer, size_t bufferLen);
void mk_sl_makeUnique(MkStrList arr);

/* -------------------------------------------------------------------------- */
typedef struct MkBuffer_s *MkBuffer;

MkBuffer mk_buf_loadMemoryRange(const char *filename, const char *source, size_t len);
MkBuffer mk_buf_loadMemory(const char *filename, const char *source);
MkBuffer mk_buf_loadFile(const char *filename);
MkBuffer mk_buf_delete(MkBuffer text);

int mk_buf_setFilename(MkBuffer text, const char *filename);
int mk_buf_setFunction(MkBuffer text, const char *func);

const char *mk_buf_getFilename(const MkBuffer text);
const char *mk_buf_getFunction(const MkBuffer text);
size_t mk_buf_calculateLine(const MkBuffer text);

size_t mk_buf_getLength(const MkBuffer text);

void mk_buf_seek(MkBuffer text, size_t pos);
size_t mk_buf_tell(const MkBuffer text);
char *mk_buf_getPtr(MkBuffer text);

char mk_buf_read(MkBuffer text);
char mk_buf_peek(MkBuffer text);
char mk_buf_lookAhead(MkBuffer text, size_t offset);
int mk_buf_advanceIfCharEq(MkBuffer text, char ch);

void mk_buf_skip(MkBuffer text, size_t offset);
void mk_buf_skipWhite(MkBuffer text);
void mk_buf_skipLine(MkBuffer text);
int mk_buf_skipLineIfStartsWith(MkBuffer text, const char *pszCommentDelim);

int mk_buf_readLine(MkBuffer text, char *dst, size_t dstn);

void mk_buf_errorfv(MkBuffer text, const char *format, va_list args);
void mk_buf_errorf(MkBuffer text, const char *format, ...);

/* -------------------------------------------------------------------------- */
typedef struct stat MkStat_t;

void mk_fs_init(void);
void mk_fs_fini(void);

char *mk_fs_getCWD(char *cwd, size_t n);
int mk_fs_enter(const char *path);
void mk_fs_leave(void);
int mk_fs_isFile(const char *path);
int mk_fs_isDir(const char *path);
void mk_fs_makeDirs(const char *dirs);
char *mk_fs_realPath(const char *filename, char *resolvedname, size_t maxn);

DIR *mk_fs_openDir(const char *path);
DIR *mk_fs_closeDir(DIR *p);
struct dirent *mk_fs_readDir(DIR *d);
void mk_fs_remove(const char *path);

/* -------------------------------------------------------------------------- */
typedef struct MkGitInfo_s *MkGitInfo;

char *mk_git_findRoot( void );
char *mk_git_findBranchPath( const char *pszGitDir );
char *mk_git_getCommit( const char *pszBranchFile );
char *mk_git_getCommitTimestamp( const char *pszBranchFile, const MkStat_t *pStat );
MkGitInfo mk_git_loadInfo( void );
int mk_git_writeHeader( const char *pszHFilename, MkGitInfo pGitInfo );

int mk_git_generateInfo( void );

/* -------------------------------------------------------------------------- */
#if !MK_DEBUG_ENABLED
# undef  MK_DEBUG_DEPENDENCY_TRACKER_ENABLED
# define MK_DEBUG_DEPENDENCY_TRACKER_ENABLED 0
#else
# ifndef MK_DEBUG_DEPENDENCY_TRACKER_ENABLED
#  define MK_DEBUG_DEPENDENCY_TRACKER_ENABLED 0
# endif
#endif

typedef struct MkDep_s *MkDep;

MkDep mk_dep_new(const char *name);
void mk_dep_delete(MkDep dep);
void mk_dep_deleteAll(void);
const char *mk_dep_getFile(MkDep dep);
void mk_dep_push(MkDep dep, const char *name);
size_t mk_dep_getSize(MkDep dep);
const char *mk_dep_at(MkDep dep, size_t i);
MkDep mk_dep_find(const char *name);
void mk_dep_debugPrintAll(void);

/* -------------------------------------------------------------------------- */
int mk_mfdep_load(const char *filename);

/* -------------------------------------------------------------------------- */
typedef struct MkLib_s *MkLib;

MkLib mk_lib_new(void);
void mk_lib_delete(MkLib lib);
void mk_lib_deleteAll(void);
void mk_lib_setName(MkLib lib, const char *name);
void mk_lib_setFlags(MkLib lib, int sys, const char *flags);
const char *mk_lib_getName(MkLib lib);
const char *mk_lib_getFlags(MkLib lib, int sys);
MkLib mk_lib_prev(MkLib lib);
MkLib mk_lib_next(MkLib lib);
MkLib mk_lib_head(void);
MkLib mk_lib_tail(void);
MkLib mk_lib_find(const char *name);
MkLib mk_lib_lookup(const char *name);
void mk_lib_clearAllProcessed(void);
void mk_lib_clearProcessed(MkLib lib);
void mk_lib_setProcessed(MkLib lib);
int mk_lib_isProcessed(MkLib lib);

/* -------------------------------------------------------------------------- */
#if !MK_DEBUG_ENABLED
# undef  MK_DEBUG_AUTOLIBCONF_ENABLED
# define MK_DEBUG_AUTOLIBCONF_ENABLED 0
#else
# ifndef MK_DEBUG_AUTOLIBCONF_ENABLED
#  define MK_DEBUG_AUTOLIBCONF_ENABLED 1
# endif
#endif

typedef struct MkAutolink_s *MkAutolink;

MkAutolink mk_al_new(void);
void mk_al_delete(MkAutolink al);
void mk_al_deleteAll(void);
void mk_al_setHeader(MkAutolink al, int sys, const char *header);
void mk_al_setLib(MkAutolink al, const char *libname);
const char *mk_al_getHeader(MkAutolink al, int sys);
const char *mk_al_getLib(MkAutolink al);
MkAutolink mk_al_find(int sys, const char *header);
MkAutolink mk_al_lookup(int sys, const char *header);
const char *mk_al_autolink(int sys, const char *header);
void mk_al_managePackage_r(const char *libname, int sys, const char *incdir);

int mk_al_loadConfig( const char *filename );

/* -------------------------------------------------------------------------- */
#if !MK_DEBUG_ENABLED
# undef  MK_DEBUG_LIBDEPS_ENABLED
# define MK_DEBUG_LIBDEPS_ENABLED 0
#else
# ifndef MK_DEBUG_LIBDEPS_ENABLED
#  define MK_DEBUG_LIBDEPS_ENABLED 1
# endif
#endif

typedef struct MkProject_s *MkProject;

typedef enum {
	/* static library (lib<blah>.a) */
	kMkProjTy_StaticLib,
	/* dynamic library (<blah>.dll) */
	kMkProjTy_DynamicLib,
	/* (console) executable (<blah>.exe) */
	kMkProjTy_Program,
	/* (gui) executable (<blah>.exe) -- gets console in debug mode */
	kMkProjTy_Application
} MkProjectType_t;

typedef enum {
	kMkOS_MSWin,
	kMkOS_UWP,
	kMkOS_Cygwin,
	kMkOS_Linux,
	kMkOS_MacOSX,
	kMkOS_Unix,

	kMkNumOS
} MkOS_t;

typedef enum {
	kMkCPU_X86,
	kMkCPU_X86_64,
	kMkCPU_ARM,
	kMkCPU_AArch64,
	kMkCPU_PowerPC,
	kMkCPU_MIPS,
	kMkCPU_WebAssembly,

	kMkNumCPU
} MkCPU_t;

MkProject mk_prj_new(MkProject prnt);
void mk_prj_delete(MkProject proj);
void mk_prj_deleteAll(void);
MkProject mk_prj_rootHead(void);
MkProject mk_prj_rootTail(void);
MkProject mk_prj_getParent(MkProject proj);
MkProject mk_prj_head(MkProject proj);
MkProject mk_prj_tail(MkProject proj);
MkProject mk_prj_prev(MkProject proj);
MkProject mk_prj_next(MkProject proj);
void mk_prj_setName(MkProject proj, const char *name);
void mk_prj_setOutPath(MkProject proj, const char *path);
void mk_prj_setPath(MkProject proj, const char *path);
void mk_prj_setType(MkProject proj, int type);
const char *mk_prj_getName(MkProject proj);
const char *mk_prj_getPath(MkProject proj);
const char *mk_prj_getOutPath(MkProject proj);
int mk_prj_getType(MkProject proj);
void mk_prj_addLib(MkProject proj, const char *libname);
size_t mk_prj_numLibs(MkProject proj);
const char *mk_prj_libAt(MkProject proj, size_t i);
void mk_prj_addLinkFlags(MkProject proj, const char *flags);
const char *mk_prj_getLinkFlags(MkProject proj);
void mk_prj_appendExtraLibs(MkProject proj, const char *extras);
const char *mk_prj_getExtraLibs(MkProject proj);
void mk_prj_completeExtraLibs(MkProject proj, char *extras, size_t n);
void mk_prj_addSourceFile(MkProject proj, const char *src);
size_t mk_prj_numSourceFiles(MkProject proj);
const char *mk_prj_sourceFileAt(MkProject proj, size_t i);
void mk_prj_addTestSourceFile(MkProject proj, const char *src);
size_t mk_prj_numTestSourceFiles(MkProject proj);
const char *mk_prj_testSourceFileAt(MkProject proj, size_t i);
void mk_prj_addSpecialDir(MkProject proj, const char *dir);
size_t mk_prj_numSpecialDirs(MkProject proj);
const char *mk_prj_specialDirAt(MkProject proj, size_t i);
int mk_prj_isTarget(MkProject proj);
void mk_prj_printAll(MkProject proj, const char *margin);
void mk_prj_calcDeps(MkProject proj);
void mk_prj_calcLibFlags(MkProject proj);
MkProject mk_prj_find_r(MkProject from, const char *name);
MkProject mk_prj_find(const char *name);

/* -------------------------------------------------------------------------- */
int mk_prjfs_isSpecialDir(MkProject proj, const char *name);
int mk_prjfs_isIncDir(MkProject proj, const char *name);
int mk_prjfs_isLibDir(MkProject proj, const char *name);
int mk_prjfs_isTestDir(MkProject proj, const char *name);
int mk_prjfs_isDirOwner(const char *path);
void mk_prjfs_enumSourceFiles(MkProject proj, const char *srcdir);
void mk_prjfs_enumTestSourceFiles(MkProject proj, const char *srcdir);
int mk_prjfs_calcName(MkProject proj, const char *path, const char *file);
MkProject mk_prjfs_add(MkProject prnt, const char *path, const char *file, int type);
void mk_prjfs_findPackages(const char *pkgdir);
void mk_prjfs_findDynamicLibs(const char *dllsdir);
void mk_prjfs_findTools(const char *tooldir);
void mk_prjfs_findProjects(MkProject prnt, const char *srcdir);
void mk_prjfs_findRootDirs(MkStrList srcdirs, MkStrList incdirs, MkStrList libdirs, MkStrList pkgdirs, MkStrList tooldirs, MkStrList dllsdirs);
int mk_prjfs_makeObjDirs(MkProject proj);

/* -------------------------------------------------------------------------- */
#if !MK_DEBUG_ENABLED
# undef  MK_DEBUG_FIND_SOURCE_LIBS_ENABLED
# define MK_DEBUG_FIND_SOURCE_LIBS_ENABLED 0
#else
# ifndef MK_DEBUG_FIND_SOURCE_LIBS_ENABLED
#  define MK_DEBUG_FIND_SOURCE_LIBS_ENABLED 0
# endif
#endif

void mk_bld_initUnitTestArrays(void);
int mk_bld_findSourceLibs(MkStrList dst, int sys, const char *obj, const char *dep);
int mk_bld_shouldCompile(const char *obj);
int mk_bld_shouldLink(const char *bin, int numbuilds);
const char *mk_bld_getCompiler(int iscxx);
void mk_bld_getCFlags_warnings(char *flags, size_t nflags);
int mk_bld_getCFlags_standard(char *flags, size_t nflags, const char *filename);
void mk_bld_getCFlags_config(char *flags, size_t nflags, int projarch);
void mk_bld_getCFlags_platform(char *flags, size_t nflags, int projarch, int projsys, int usenative);
void mk_bld_getCFlags_projectType(char *flags, size_t nflags, int projtype);
void mk_bld_getCFlags_incDirs(char *flags, size_t nflags);
void mk_bld_getCFlags_defines(char *flags, size_t nflags, MkStrList defs);
void mk_bld_getCFlags_unitIO(char *flags, size_t nflags, const char *obj, const char *src);
const char *mk_bld_getCFlags(MkProject proj, const char *obj, const char *src);
void mk_bld_getDeps_r(MkProject proj, MkStrList deparray);
int mk_bld_doesLibDependOnLib(MkLib mainlib, MkLib deplib);
void mk_bld_sortDeps(MkStrList deparray);
const char *mk_bld_getProjDepLinkFlags(MkProject proj);
void mk_bld_getLibs(MkProject proj, char *dst, size_t n);
const char *mk_bld_getLFlags(MkProject proj, const char *bin, MkStrList objs);
void mk_bld_getObjName(MkProject proj, char *obj, size_t n, const char *src);
void mk_bld_getBinName(MkProject proj, char *bin, size_t n);
void mk_bld_unitTest(MkProject proj, const char *src);
void mk_bld_runTests(void);
void mk_bld_sortProjects(struct MkProject_s *proj);
void mk_bld_relinkDeps(MkProject proj);
int mk_bld_makeProject(MkProject proj);
int mk_bld_makeAllProjects(void);

/* -------------------------------------------------------------------------- */
void mk_front_pushSrcDir(const char *srcdir);
void mk_front_pushIncDir(const char *incdir);
void mk_front_pushLibDir(const char *libdir);
void mk_front_pushPkgDir(const char *pkgdir);
void mk_front_pushToolDir(const char *tooldir);
void mk_front_pushDynamicLibsDir(const char *dllsdir);

void mk_fs_unwindDirs(void);

void mk_main_init(int argc, char **argv);

/* -------------------------------------------------------------------------- */
MkStrList mk__g_targets = (MkStrList)0;
MkStrList mk__g_srcdirs = (MkStrList)0;
MkStrList mk__g_incdirs = (MkStrList)0;
MkStrList mk__g_libdirs = (MkStrList)0;
MkStrList mk__g_pkgdirs = (MkStrList)0;
MkStrList mk__g_tooldirs = (MkStrList)0;
MkStrList mk__g_dllsdirs = (MkStrList)0;

enum {
	kMkFlag_ShowHelp_Bit = 0x01,
	kMkFlag_ShowVersion_Bit = 0x02,
	kMkFlag_Verbose_Bit = 0x04,
	kMkFlag_Release_Bit = 0x08,
	kMkFlag_Rebuild_Bit = 0x10,
	kMkFlag_NoCompile_Bit = 0x20,
	kMkFlag_NoLink_Bit = 0x40,
	kMkFlag_LightClean_Bit = 0x80,
	kMkFlag_PrintHierarchy_Bit = 0x100,
	kMkFlag_Pedantic_Bit = 0x200,
	kMkFlag_Test_Bit = 0x400,
	kMkFlag_FullClean_Bit = 0x800
};
bitfield_t mk__g_flags = 0;
MkColorMode_t mk__g_flags_color = MK__DEFAULT_COLOR_MODE_IMPL;

/*
 *	========================================================================
 *	DEBUGGING CODE
 *	========================================================================
 *	Debug helpers.
 */

#if MK_DEBUG_ENABLED
FILE *mk__g_pDebugLog = (FILE *)0;
#endif
unsigned mk__g_cDebugIndents = 0;

static void mk_dbg__closeLog_f(void) {
#if MK_DEBUG_ENABLED
	if( !mk__g_pDebugLog ) {
		return;
	}

	fprintf(mk__g_pDebugLog, "\n\n[[== DEBUG LOG CLOSED ==]]\n\n");

	fclose(mk__g_pDebugLog);
	mk__g_pDebugLog = (FILE *)0;
#endif
}

/* write to the debug log, no formatting */
void mk_dbg_out(const char *str) {
#if MK_DEBUG_ENABLED
	static const char szTabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	static const unsigned cTabs = sizeof( szTabs ) - 1;
	unsigned cIndents;

	if( !mk__g_pDebugLog ) {
		time_t rawtime;
		struct tm *timeinfo;
		char szTimeBuf[ 128 ];

		mk__g_pDebugLog = fopen(mk_opt_getDebugLogPath(), "a+");
		if( !mk__g_pDebugLog ) {
			return;
		}

		szTimeBuf[ 0 ] = '\0';

		time( &rawtime );
		if( ( timeinfo = localtime( &rawtime ) ) != ( struct tm * )0 ) {
			strftime( szTimeBuf, sizeof( szTimeBuf ), " (%Y-%m-%d %H:%M:%S)", timeinfo );
		}

		fprintf(mk__g_pDebugLog, "\n\n[[== DEBUG LOG OPENED%s ==]]\n\n", szTimeBuf);
		fprintf(mk__g_pDebugLog, "#### Mk version: " MK_VERSION_STR " ****\n\n" );

		atexit(mk_dbg__closeLog_f);
	}

	cIndents = mk__g_cDebugIndents;
	while( cIndents > 0 ) {
		unsigned cIndentsToWrite;

		cIndentsToWrite = cIndents < cTabs ? cIndents : cTabs;
		fwrite(&szTabs[0], (size_t)cIndentsToWrite, 1, mk__g_pDebugLog);

		cIndents -= cIndentsToWrite;
	}

	fwrite(str, strlen(str), 1, mk__g_pDebugLog);
	fflush(mk__g_pDebugLog);
#else
	(void)str;
#endif
}

/* write to the debug log (va_args) */
void mk_dbg_outfv(const char *format, va_list args) {
#if MK_DEBUG_ENABLED
	static char buf[65536];

# if MK_SECLIB
	vsprintf_s(buf, sizeof(buf), format, args);
# else
	vsnprintf(buf, sizeof(buf), format, args);
	buf[sizeof(buf) - 1] = '\0';
# endif

	mk_dbg_out(buf);
#else
	(void)format;
	(void)args;
#endif
}
/* write to the debug log */
void mk_dbg_outf(const char *format, ...) {
	va_list args;

	va_start(args, format);
	mk_dbg_outfv(format, args);
	va_end(args);
}

/* enter a debug layer */
void mk_dbg_enter(const char *format, ...) {
	va_list args;

	mk_dbg_out("\n");
	va_start(args, format);
	mk_dbg_outfv(format, args);
	mk_dbg_out("\n");
	mk_dbg_out("{\n");
	va_end(args);

	++mk__g_cDebugIndents;
}
/* leave a debug layer */
void mk_dbg_leave(void) {
	MK_ASSERT( mk__g_cDebugIndents > 0 );

	if( mk__g_cDebugIndents > 0 ) {
		--mk__g_cDebugIndents;
	}

	mk_dbg_out("}\n\n");
}

/* backtrace support... */
void mk_dbg_backtrace(void) {
#if MK_WINDOWS_ENABLED
#else
	int i, n;
	void *buffer[PATH_MAX];
	char **strings;

	n = backtrace(buffer, sizeof(buffer)/sizeof(buffer[0]));
	if( n < 1 ) {
		return;
	}

	strings = backtrace_symbols(buffer, n);
	if( !strings ) {
		return;
	}

	mk_dbg_enter("BACKTRACE(%i)", n);
	for( i=0; i<n; i++ ) {
		mk_dbg_outf("%s\n", strings[i]);
	}
	mk_dbg_leave();

	free((void *)strings);
#endif
}

/*
 *	========================================================================
 *	COLORED PRINTING CODE
 *	========================================================================
 *	Display colored output.
 */

#if MK_WINDOWS_ENABLED
HANDLE mk__g_sioh[kMkNumSIO];
#endif
FILE *mk__g_siof[kMkNumSIO];

int mk_sys_isColoredOutputEnabled(void) {
	return mk__g_flags_color != kMkColorMode_None;
}

void mk_sys_initColoredOutput(void) {
#if MK_WINDOWS_ENABLED
	mk__g_sioh[kMkSIO_Out] = GetStdHandle(STD_OUTPUT_HANDLE);
	mk__g_sioh[kMkSIO_Err] = GetStdHandle(STD_ERROR_HANDLE);
#endif

	/*
	 * TODO: Make sure that colored output is possible on this terminal
	 */

	mk__g_siof[kMkSIO_Out] = stdout;
	mk__g_siof[kMkSIO_Err] = stderr;
}

unsigned char mk_sys_getCurrColor(MkSIO_t sio) {
#if MK_WINDOWS_ENABLED
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if( mk__g_sioh[sio] != INVALID_HANDLE_VALUE ) {
		if( mk__g_sioh[sio] && GetConsoleScreenBufferInfo(mk__g_sioh[sio], &csbi) ) {
			return csbi.wAttributes & 0xFF;
		}
	}
#else
	/*
	 * TODO: Support this on GNU/Linux...
	 */
	(void)sio;
#endif

	return 0x07;
}
void mk_sys_setCurrColor(MkSIO_t sio, unsigned char color) {
	/*
	 * MAP ORDER:
	 * Black, Blue, Green, Cyan, Red, Magenta, Yellow, Grey
	 *
	 * TERMINAL COLOR ORDER:
	 * Black=0, Red=1, Green=2, Yellow=3, Blue=4, Magenta=5, Cyan=6, Grey=7
	 */
	static const char *const mapF[16] = {
		"\x1b[30;22m", "\x1b[34;22m", "\x1b[32;22m", "\x1b[36;22m",
		"\x1b[31;22m", "\x1b[35;22m", "\x1b[33;22m", "\x1b[37;22m",
		"\x1b[30;1m", "\x1b[34;1m", "\x1b[32;1m", "\x1b[36;1m",
		"\x1b[31;1m", "\x1b[35;1m", "\x1b[33;1m", "\x1b[37;1m"
	};
#if 0
	static const char *const mapB[16] = {
		"\x1b[40m", "\x1b[41m", "\x1b[42m", "\x1b[43m",
		"\x1b[44m", "\x1b[45m", "\x1b[46m", "\x1b[47m",
		"\x1b[40;1m", "\x1b[41;1m", "\x1b[42;1m", "\x1b[43;1m",
		"\x1b[44;1m", "\x1b[45;1m", "\x1b[46;1m", "\x1b[47;1m"
	};
#endif

	switch( mk_opt_getColorMode() ) {
	case kMkColorMode_None:
		break;

	case kMkColorMode_ANSI:
		fwrite(mapF[color & 0x0F], strlen(mapF[color & 0x0F]), 1, mk__g_siof[sio]);
		break;

#if MK_WINDOWS_COLORS_ENABLED
	case kMkColorMode_Windows:
		if( mk__g_sioh[sio] != INVALID_HANDLE_VALUE ) {
			SetConsoleTextAttribute(mk__g_sioh[sio], (WORD)color);
		}
		break;
#endif

	case kMkNumColorModes:
		MK_ASSERT_MSG(0, "Invalid color mode!");
		break;
	}
}
void mk_sys_uncoloredPuts(MkSIO_t sio, const char *text, size_t len) {
	if( !len ) {
		len = strlen(text);
	}

	mk_dbg_outf("%.*s", len, text);

#if MK_WINDOWS_ENABLED && 0
	if( mk__g_sioh[sio] != INVALID_HANDLE_VALUE ) {
		WriteFile(mk__g_sioh[sio], text, len, NULL, NULL);
		return;
	}
#endif

	fwrite(text, len, 1, mk__g_siof[sio]);
}
int mk_sys_charToColorCode(char c) {
	if( c>='0' && c<='9' ) {
		return  0 + (int)(c - '0');
	}

	if( c>='A' && c<='F' ) {
		return 10 + (int)(c - 'A');
	}

	if( c>='a' && c<='f' ) {
		return 10 + (int)(c - 'a');
	}

	return -1;
}
void mk_sys_puts(MkSIO_t sio, const char *text) {
	unsigned char prevColor;
	const char *s, *e;
	int color;

	/* set due to potential bad input pulling garbage color */
	prevColor = mk_sys_getCurrColor(sio);

	s = text;
	while( 1 ) {
		/* write normal characters in one chunk */
		if( *s!='^' ) {
			e = strchr(s, '^');
			if( !e ) {
				e = strchr(s, '\0');
			}

			mk_sys_uncoloredPuts(sio, s, e - s);
			if( *e=='\0' ) {
				break;
			}

			s = e;
			continue;
		}

		/* must be a special character, treat it as such */
		color = mk_sys_charToColorCode(*++s);
		if( color != -1 ) {
			prevColor = mk_sys_getCurrColor(sio);
			mk_sys_setCurrColor(sio, (unsigned char)color);
		} else if(*s=='&') {
			color = (int)prevColor;
			prevColor = mk_sys_getCurrColor(sio);
			mk_sys_setCurrColor(sio, (unsigned char)color);
		} else if(*s=='^') {
			mk_sys_uncoloredPuts(sio, "^", 1);
		}

		s++;
	}
}
void mk_sys_printf(MkSIO_t sio, const char *format, ...) {
	static char buf[32768];
	va_list args;

	va_start(args, format);
#if MK_SECLIB
	vsprintf_s(buf, sizeof(buf), format, args);
#else
	vsnprintf(buf, sizeof(buf), format, args);
	buf[sizeof(buf) - 1] = '\0';
#endif
	va_end(args);

	mk_sys_puts(sio, buf);
}

void mk_sys_printStr(MkSIO_t sio, unsigned char color, const char *str) {
	unsigned char curColor;

	MK_ASSERT( str != (const char *)0 );

	curColor = mk_sys_getCurrColor(sio);
	mk_sys_setCurrColor(sio, color);
	mk_sys_uncoloredPuts(sio, str, 0);
	mk_sys_setCurrColor(sio, curColor);
}
void mk_sys_printUint(MkSIO_t sio, unsigned char color, unsigned int val) {
	char buf[64];

#if MK_SECLIB
	sprintf_s(buf, sizeof(buf), "%u", val);
#else
	snprintf(buf, sizeof(buf), "%u", val);
	buf[sizeof(buf) - 1] = '\0';
#endif

	mk_sys_printStr(sio, color, buf);
}
void mk_sys_printInt(MkSIO_t sio, unsigned char color, int val) {
	char buf[64];

#if MK_SECLIB
	sprintf_s(buf, sizeof(buf), "%i", val);
#else
	snprintf(buf, sizeof(buf), "%i", val);
	buf[sizeof(buf) - 1] = '\0';
#endif

	mk_sys_printStr(sio, color, buf);
}

/*
 *	========================================================================
 *	ERROR HANDLING CODE
 *	========================================================================
 *	Display errors in an appropriate manner.
 */

/* display a simple mk_log_error message */
void mk_log_errorMsg(const char *message) {
	unsigned char curColor;

	curColor = mk_sys_getCurrColor(kMkSIO_Err);
	mk_sys_printf(kMkSIO_Err, MK_S_COLOR_RED "ERROR" MK_S_COLOR_RESTORE ": %s", message);
	mk_sys_setCurrColor(kMkSIO_Err, curColor);
	if( errno ) {
		mk_sys_printf(kMkSIO_Err, ": %s [" MK_S_COLOR_MAGENTA "%d"
			MK_S_COLOR_RESTORE "]", strerror(errno), errno);
	}
	mk_sys_uncoloredPuts(kMkSIO_Err, "\n", 1);

	mk_dbg_backtrace();
}

/* exit with an mk_log_error message; if errno is set, display its mk_log_error */
MK_NORETURN void mk_log_fatalError(const char *message) {
	mk_log_errorMsg(message);
	exit(EXIT_FAILURE);
}

/* provide a general purpose mk_log_error, without terminating */
void mk_log_error(const char *file, unsigned int line, const char *func,
const char *message) {
	if( file ) {
		mk_sys_printStr(kMkSIO_Err, MK_COLOR_WHITE, file);
		mk_sys_uncoloredPuts(kMkSIO_Err, ":", 1);
		if( line ) {
			mk_sys_printUint(kMkSIO_Err, MK_COLOR_BROWN, line);
			mk_sys_uncoloredPuts(kMkSIO_Err, ":", 1);
		}
		mk_sys_uncoloredPuts(kMkSIO_Err, " ", 1);
	}

	mk_sys_puts(kMkSIO_Err, MK_S_COLOR_RED "ERROR" MK_S_COLOR_RESTORE ": ");

	if( func ) {
		unsigned char curColor;

		curColor = mk_sys_getCurrColor(kMkSIO_Err);
		mk_sys_puts(kMkSIO_Err, "in " MK_S_COLOR_GREEN);
		mk_sys_uncoloredPuts(kMkSIO_Err, func, 0);
		mk_sys_setCurrColor(kMkSIO_Err, curColor);
		mk_sys_uncoloredPuts(kMkSIO_Err, ": ", 2);
	}

	if( message ) {
		mk_sys_uncoloredPuts(kMkSIO_Err, message, 0);
	}

	if( (errno&&message) || !message ) {
		if( message ) {
			mk_sys_uncoloredPuts(kMkSIO_Err, ": ", 2);
		}
		mk_sys_uncoloredPuts(kMkSIO_Err, strerror(errno), 0);
		mk_sys_printf(kMkSIO_Err, "[" MK_S_COLOR_MAGENTA "%d" MK_S_COLOR_RESTORE "]",
			errno);
	}

	mk_sys_uncoloredPuts(kMkSIO_Err, "\n", 1);
	mk_dbg_backtrace();
}

/* provide an mk_log_error for a failed MK_ASSERT */
void mk_log_errorAssert(const char *file, unsigned int line, const char *func,
const char *message) {
	mk_log_error(file, line, func, message);
#if MK_WINDOWS_ENABLED
	fflush( stdout );
	fflush( stderr );

	if( IsDebuggerPresent() ) {
# if MK_VCVER
		__debugbreak();
# elif defined( __GNUC__ ) || defined( __clang__ )
		__builtin_trap();
# else
		DebugBreak();
# endif
	}
#endif
	/*exit(EXIT_FAILURE);*/
	abort();
}


/*
 *	========================================================================
 *	MEMORY CODE
 *	========================================================================
 *	Utility functions for managing memory. Allocated objects can have
 *	hierarchies of memory, such that when a "super" node is deallocated, its
 *	sub nodes are also deallocated. Destructor functions can be assigned as
 *	well to do other clean-up tasks. Reference counting is supported when
 *	ownership isn't as simple. (Objects allocated default to a reference
 *	count of one.)
 */

struct MkMem__Hdr_s
{
	struct MkMem__Hdr_s *pPrnt;
	struct MkMem__Hdr_s *pPrev, *pNext;
	struct MkMem__Hdr_s *pHead, *pTail;
	MkMem_Fini_fn_t pfnFini;
	size_t cRefs;
	size_t cBytes;
#if MK_MEM_LOCTRACE_ENABLED
	const char *pszFile;
	unsigned int uLine;
	const char *pszFunction;
#endif
};

static void mk_mem__unlink( struct MkMem__Hdr_s *pHdr )
{
	if( pHdr->pPrev != NULL ) {
		pHdr->pPrev->pNext = pHdr->pNext;
	} else if( pHdr->pPrnt != NULL ) {
		pHdr->pPrnt->pHead = pHdr->pNext;
	}

	if( pHdr->pNext != NULL ) {
		pHdr->pNext->pPrev = pHdr->pPrev;
	} else if( pHdr->pPrnt != NULL ) {
		pHdr->pPrnt->pTail = pHdr->pPrev;
	}

	pHdr->pPrnt = NULL;
	pHdr->pPrev = NULL;
	pHdr->pNext = NULL;
}

/*
================
mk_mem__maybeAlloc

Potentially allocate memory. (Returns NULL on failure.)
================
*/
void *mk_mem__maybeAlloc( size_t cBytes, bitfield_t uFlags, const char *pszFile, unsigned int uLine, const char *pszFunction )
{
	struct MkMem__Hdr_s *pHdr;
	void *p;

	pHdr = ( struct MkMem__Hdr_s * )malloc( sizeof( *pHdr ) + cBytes );
	if( !pHdr ) {
		return NULL;
	}

	p = ( void * )( pHdr + 1 );

	pHdr->pPrnt = NULL;
	pHdr->pPrev = NULL;
	pHdr->pNext = NULL;
	pHdr->pHead = NULL;
	pHdr->pTail = NULL;
	pHdr->pfnFini = NULL;
	pHdr->cRefs = 1;
	pHdr->cBytes = cBytes;
#if MK_MEM_LOCTRACE_ENABLED
	pHdr->pszFile = pszFile;
	pHdr->uLine = uLine;
	pHdr->pszFunction = pszFunction;
#else
	( void )pszFile;
	( void )uLine;
	( void )pszFunction;
#endif

	if( ( uFlags & kMkMemF_Uninitialized ) == 0 ) {
		memset( p, 0, cBytes );
	}

#if MK_LOG_MEMORY_ALLOCS_ENABLED
	mk_dbg_outf( "ALLOC: %s(%i) in %s: %p, %u;", pszFile, uLine, pszFunction, p,
		( unsigned int )cBytes );
#endif

	return p;
}
/*
================
mk_mem__alloc

Allocate memory without returning NULL. If an allocation fails, provide an error
then exit the process.
================
*/
void *mk_mem__alloc( size_t cBytes, bitfield_t uFlags, const char *pszFile, unsigned int uLine, const char *pszFunction )
{
	void *p;

	p = mk_mem__maybeAlloc( cBytes, uFlags, pszFile, uLine, pszFunction );
	if( !p ) {
		mk_log_fatalError( "Out of memory" );
	}

	return p;
}
/*
================
mk_mem__dealloc

Decrement the reference count of the memory block. If the reference count
reaches zero then deallocate all sub-blocks of memory. Throw an error if any of
the sub-blocks have a reference count greater than one.
================
*/
void *mk_mem__dealloc( void *pBlock, const char *pszFile, unsigned int uLine, const char *pszFunction )
{
	struct MkMem__Hdr_s *pHdr;

	if( !pBlock ) {
		return NULL;
	}

	MK_ASSERT( ( size_t )pBlock > 4096 );

	pHdr = ( struct MkMem__Hdr_s * )pBlock - 1;

#if MK_LOG_MEMORY_ALLOCS_ENABLED
	mk_dbg_outf( "DEALLOC: %s(%i) in %s: %s%p, %u (refcnt=%u);", pszFile, uLine,
		pszFunction, pHdr->pPrnt != NULL ? "[sub]" : "", pBlock,
		( unsigned int )pHdr->cBytes, ( unsigned int )pHdr->cRefs );
#endif

	MK_ASSERT( pHdr->cRefs > 0 );
	if( --pHdr->cRefs != 0 ) {
		return NULL;
	}

	while( pHdr->pHead != NULL ) {
		MK_ASSERT( pHdr->pHead->cRefs <= 1 );
		mk_mem__dealloc( ( void * )( pHdr->pHead + 1 ), pszFile, uLine, pszFunction );
	}

	if( pHdr->pfnFini != NULL ) {
		pHdr->pfnFini( pBlock );
	}

	mk_mem__unlink( pHdr );

	free( ( void * )pHdr );
	return NULL;
}
/*
================
mk_mem__addRef

Increase the reference count of a memory block. Returns the address passed in.
================
*/
void *mk_mem__addRef( void *pBlock, const char *pszFile, unsigned int uLine, const char *pszFunction )
{
	struct MkMem__Hdr_s *pHdr;

	MK_ASSERT( pBlock != NULL );

	pHdr = ( struct MkMem__Hdr_s * )pBlock - 1;

	++pHdr->cRefs;

#if MK_LOG_MEMORY_ALLOCS_ENABLED
	mk_dbg_outf( "MEM-ADDREF: %s(%i) in %s: %p, %u (refcnt=%u);", pszFile, uLine,
		pszFunction, pBlock, ( unsigned int )cBytes, ( unsigned int )pHdr->cRefs );
#else
	( void )pszFile;
	( void )uLine;
	( void )pszFunction;
#endif

	return pBlock;
}
/*
================
mk_mem__attach

Set a block to be the sub-block of some super-block. The sub-block will be
deallocated when the super-block is deallocated. The sub-block does NOT have its
reference count increased upon being attached to the super-block.

The super-block (pSuperBlock) cannot be NULL. To remove a block from an existing
super-block, call mk_mem__detach().

Returns the address of the sub-block (pBlock) passed in.
================
*/
void *mk_mem__attach( void *pBlock, void *pSuperBlock, const char *pszFile, unsigned int uLine, const char *pszFunction )
{
	struct MkMem__Hdr_s *pHdr;
	struct MkMem__Hdr_s *pSuperHdr;

	MK_ASSERT( pBlock != NULL );
	MK_ASSERT( pSuperBlock != NULL );

	pHdr = ( struct MkMem__Hdr_s * )pBlock - 1;
	pSuperHdr = ( struct MkMem__Hdr_s * )pSuperBlock - 1;

#if MK_LOG_MEMORY_ALLOCS_ENABLED
	mk_dbg_outf( "MEM-ATTACH: %s(%i) in %s: %p(%u)->%p(%u);",
		pszFile, uLine, pszFunction,
		pBlock, ( unsigned int )pBlock->cBytes,
		pSuperBlock, ( unsigned int )pSuperBlock->cBytes );
#else
	( void )pszFile;
	( void )uLine;
	( void )pszFunction;
#endif

	mk_mem__unlink( pHdr );

	pHdr->pPrnt = pSuperHdr;
	pHdr->pPrev = NULL;
	pHdr->pNext = pSuperHdr->pHead;
	if( pSuperHdr->pHead != NULL ) {
		pSuperHdr->pHead->pPrev = pHdr;
	} else {
		pSuperHdr->pTail = pHdr;
	}
	pSuperHdr->pHead = pHdr;

	return pBlock;
}
/*
================
mk_mem__detach

Remove a block from the super-block it is attached to, if any. This ensures the
block will not be deallocated when its super-block is allocated. (Useful when
new ownership is necessary.)

Only call this function if absolutely necessary. Changing or removing ownership
is discouraged.

Returns the address passed in (pBlock).
================
*/
void *mk_mem__detach( void *pBlock, const char *pszFile, unsigned int uLine, const char *pszFunction )
{
	struct MkMem__Hdr_s *pHdr;

	MK_ASSERT( pBlock != NULL );

	pHdr = ( struct MkMem__Hdr_s * )pBlock - 1;

#if MK_LOG_MEMORY_ALLOCS_ENABLED
	mk_dbg_outf( "MEM-DETACH: %s(%i) in %s: %p(%u);",
		pszFile, uLine, pszFunction,
		pBlock, ( unsigned int )pBlock->cBytes );
#else
	( void )pszFile;
	( void )uLine;
	( void )pszFunction;
#endif

	mk_mem__unlink( pHdr );
	return pBlock;
}
/*
================
mk_mem__setFini

Set the function to call when the program is finished with the block of memory.
(i.e., the clean-up / destructor function.)

Returns the address passed in (pBlock).
================
*/
void *mk_mem__setFini( void *pBlock, MkMem_Fini_fn_t pfnFini )
{
	struct MkMem__Hdr_s *pHdr;

	MK_ASSERT( pBlock != NULL );

	pHdr = ( struct MkMem__Hdr_s * )pBlock - 1;
	pHdr->pfnFini = pfnFini;

	return pBlock;
}
/*
================
mk_mem__size

Retrieve the size of a block of memory, in bytes.
================
*/
size_t mk_mem__size( const void *pBlock )
{
	const struct MkMem__Hdr_s *pHdr;

	if( !pBlock ) {
		return 0;
	}

	pHdr = ( const struct MkMem__Hdr_s * )pBlock - 1;

	return pHdr->cBytes;
}


/*
 *	========================================================================
 *	UTILITY CODE
 *	========================================================================
 *	A set of utility functions for making the rest of this easier.
 */

/*
================
mk_com__memory

manage memory; never worry about errors
================
*/
void *mk_com__memory( void *p, size_t n, const char *pszFile, unsigned int uLine, const char *pszFunction )
{
	errno = 0;

	if( n > 0 ) {
		void *q;

		q = mk_mem__alloc( n, 0, pszFile, uLine, pszFunction );

		if( p != NULL ) {
			size_t c;
#if MK_DEBUG_ENABLED
			const struct MkMem__Hdr_s *pHdr;

			pHdr = ( const struct MkMem__Hdr_s * )p - 1;
			MK_ASSERT_MSG( pHdr->cRefs == 1, "Cannot reallocate object: there are multiple references" );
			MK_ASSERT_MSG( pHdr->pHead == NULL, "Cannot reallocate object: it has associated objects" );
			MK_ASSERT_MSG( pHdr->pPrnt == NULL, "Cannot reallocate object: it is owned by another object" );
#endif

			c = mk_mem__size( p );
			memcpy( q, p, c < n ? c : n );
			mk_mem__dealloc( p, pszFile, uLine, pszFunction );
		}

		return q;
	}

	mk_mem__dealloc( p, pszFile, uLine, pszFunction );
	return NULL;
}

/*
================
mk_com_va

provide printf-style formatting on the fly
================
*/
const char *mk_com_va(const char *format, ...) {
	static char buf[65536];
	static size_t index = 0;
	va_list args;
	size_t n;
	char *p;

	p = &buf[index];

	va_start(args, format);
#if MK_SECLIB
	n = vsprintf_s(&buf[index], sizeof(buf) - index, format, args);
#else
	n = vsnprintf(&buf[index], sizeof(buf) - index, format, args);
	buf[sizeof(buf) - 1] = '\0';
#endif
	va_end(args);

	index += n;
	if( index + MK_VA_MINSIZE > sizeof(buf) ) {
		index = 0;
	}

	return p;
}

/* secure strlen */
size_t mk_com_strlen(const char *src)
{
	MK_ASSERT( src != (const char *)0 );
	return strlen(src);
}

/* secure strcat */
size_t mk_com_strcat(char *buf, size_t bufn, const char *src) {
	size_t index, len;

	index = mk_com_strlen(buf);
	len = mk_com_strlen(src);

	if( index+len >= bufn ) {
		mk_log_fatalError("detected overflow");
	}

	memcpy((void *)&buf[index], (const void *)src, len+1);
	return index+len;
}

/* secure strncat */
size_t mk_com_strncat(char *buf, size_t bufn, const char *src, size_t srcn) {
	size_t index, len;

	index = mk_com_strlen(buf);
	len = srcn ? srcn : mk_com_strlen(src);

	if( index+len >= bufn ) {
		mk_log_fatalError("detected overflow");
	}

	memcpy((void *)&buf[index], (const void *)src, len);
	buf[index + len] = '\0';

	return index+len;
}

/* secure strcpy */
void mk_com_strcpy(char *dst, size_t dstn, const char *src) {
	size_t i;

	for( i=0; src[i]; i++ ) {
		dst[i] = src[i];
		if( i==dstn-1 ) {
			mk_log_fatalError("detected overflow");
		}
	}

	dst[i] = 0;
}

/* secure strncpy */
void mk_com_strncpy(char *buf, size_t bufn, const char *src, size_t srcn) {
	if( srcn >= bufn ) {
		mk_log_fatalError("detected overflow");
	}

	strncpy(buf, src, srcn);
	buf[srcn] = 0;
}

/* secure strdup; uses mk_com_memory() */
char *mk_com__strdup(const char *cstr,const char *file,unsigned int line,const char *func) {
	size_t n;
	char *p;

	MK_ASSERT( cstr != (const char *)0 );

	n = strlen(cstr) + 1;

	p = (char *)mk_mem__alloc(n, kMkMemF_Uninitialized, file, line, func);
	memcpy((void *)p, (const void *)cstr, n);

	return p;
}

/* strdup() in-place alternative */
char *mk_com_dup(char *dst, const char *src) {
	size_t n;

	if( src ) {
		n = mk_com_strlen(src) + 1;

		dst = (char *)mk_com_memory((void *)dst, n);
		memcpy((void *)dst, (const void *)src, n);
	} else {
		dst = (char *)mk_com_memory((void *)dst, 0);
	}

	return dst;
}
char *mk_com_dupn(char *dst, const char *src, size_t numchars) {
	if( !numchars ) {
		numchars = mk_com_strlen(src);
	}

	if( src ) {
		dst = (char *)mk_com_memory((void *)dst, numchars + 1);
		memcpy((void *)dst, (const void *)src, numchars);
		dst[ numchars ] = '\0';
	} else {
		dst = (char *)mk_com_memory((void *)dst, 0);
	}

	return dst;
}

/* dynamic version of strcat() */
char *mk_com_append(char *dst, const char *src) {
	size_t l, n;

	if( !src ) {
		return dst;
	}

	l = dst ? mk_com_strlen(dst) : 0;
	n = mk_com_strlen(src) + 1;

	dst = (char *)mk_com_memory((void *)dst, l + n);
	memcpy((void *)&dst[l], (const void *)src, n);

	return dst;
}

/* extract the directory part of a filename */
const char *mk_com_extractDir(char *buf, size_t n, const char *filename) {
	const char *p;

	if( (p = strrchr(filename, '/')) != (const char *)0 ) {
		mk_com_strncpy(buf, n, filename, (p-filename) + 1);
		return &p[1];
	}

	*buf = 0;
	return filename;
}

/* copy the source string into the destination string, overwriting the
   extension (if present, otherwise appending) with 'ext' */
void mk_com_substExt(char *dst, size_t dstn, const char *src, const char *ext) {
	const char *p;

	MK_ASSERT( dst != (char *)0 );
	MK_ASSERT( dstn > 1 );
	MK_ASSERT( src != (const char *)0 );

	p = strrchr(src, '.');
	if( !p ) {
		p = strchr(src, 0);
	}

	MK_ASSERT( p != (const char *)0 );

	mk_com_strncpy(dst, dstn, src, p - src);
	mk_com_strcpy(&dst[p - src], dstn - (size_t)(p - src), ext);
}

/* run a command in the mk_com_shellf */
int mk_com_shellf(const char *format, ...) {
	static char cmd[16384];
	va_list args;

	va_start(args, format);
#if MK_SECLIB
	vsprintf_s(cmd, sizeof(cmd), format, args);
#else
	vsnprintf(cmd, sizeof(cmd), format, args);
	cmd[sizeof(cmd)-1] = 0;
#endif
	va_end(args);

#ifdef _WIN32
	{
		char *p, *e;

		e = strchr(cmd, ' ');
		if( !e ) {
			e = strchr(cmd, '\0');
		}

		for( p=&cmd[0]; p && p<e; p=strchr(p, '/') ) {
			if( *p=='/' ) {
				*p = '\\';
			}
		}
	}
#endif

	if( mk__g_flags & kMkFlag_Verbose_Bit ) {
		mk_sys_printStr(kMkSIO_Err, MK_COLOR_LIGHT_CYAN, "> ");
		mk_sys_printStr(kMkSIO_Err, MK_COLOR_CYAN, cmd);
		mk_sys_uncoloredPuts(kMkSIO_Err, "\n", 1);
	}

	fflush(mk__g_siof[kMkSIO_Err]);

	return system(cmd);
}

/* determine whether a given relative path is part of an absolute path */
int mk_com_matchPath(const char *rpath, const char *apath) {
	size_t rl, al;

	MK_ASSERT( rpath != (const char *)0 );
	MK_ASSERT( apath != (const char *)0 );

	rl = mk_com_strlen(rpath);
	al = mk_com_strlen(apath);

	if( rl > al ) {
		return 0;
	}

#if MK_WINDOWS_ENABLED
	if( _stricmp(&apath[al - rl], rpath) != 0 ) {
		return 0;
	}
#else
	if( strcmp(&apath[al - rl], rpath) != 0 ) {
		return 0;
	}
#endif

	if( al - rl > 0 ) {
		if( apath[al -rl - 1] != '/' ) {
			return 0;
		}
	}

	return 1;
}

/* retrieve the current date as an integral date -- YYYYMMDD, e.g., 20120223 */
int mk_com_getIntDate(void) {
	struct tm *p;
	time_t t;

	t = time(0);
	p = localtime(&t);

	return (p->tm_year + 1900)*10000 + (p->tm_mon + 1)*100 + p->tm_mday;
}

/* find the end of an argument within a string */
const char *mk_com_findArgEnd(const char *arg) {
	const char *p;

	if( *arg!='\"' ) {
		p = strchr(arg, ' ');
		if( !p ) {
			p = strchr(arg, '\0');
		}

		return p;
	}

	for( p=arg; *p!='\0'; p++ ) {
		if( *p=='\\' ) {
			p++;
			continue;
		}

		if( *p=='\"' ) {
			return p + 1;
		}
	}

	return p;
}

/* determine whether one argument in a string is equal to another */
int mk_com_matchArg(const char *a, const char *b) {
	const char *x, *y;

	x = mk_com_findArgEnd(a);
	y = mk_com_findArgEnd(b);

	if( x - a != y - b ) {
		return 0;
	}

	/*
	 * TODO: compare content of each argument, not the raw input
	 */

	return (int)(strncmp(a, b, x - a) == 0);
}

/* skip to next argument within a string */
const char *mk_com_skipArg(const char *arg) {
	const char *p;

	p = mk_com_findArgEnd(arg);
	while( *p<=' ' && *p!='\0' ) {
		p++;
	}

	if( *p=='\0' ) {
		return NULL;
	}

	return p;
}

/* puts the lotion on its skin; else it gets the hose again */
void mk_com_stripArgs(char *dst, size_t n, const char *src) {
	const char *p, *q, *next;

	mk_com_strcpy(dst, n, "");

	for( p=src; p!=NULL; p=next ) {
		next = mk_com_skipArg(p);

		for( q=src; q<p; q=mk_com_skipArg(q) ) {
			if( mk_com_matchArg(q, p) ) {
				break;
			}
		}

		if( q < p ) {
			continue;
		}

		q = next ? next : strchr(p, '\0');
		mk_com_strncat(dst, n, p, q - p);
	}
}

/* determine whether two characters of a path match */
int mk_com_cmpPathChar(char a, char b) {
#if MK_WINDOWS_ENABLED
	if( a>='A' && a<='Z' ) {
		a = a - 'A' + 'a';
	}
	if( b>='A' && b<='Z' ) {
		b = b - 'A' + 'a';
	}
	if( a=='\\' ) {
		a = '/';
	}
	if( b=='\\' ) {
		b = '/';
	}
#endif

	return a == b ? 1 : 0;
}

/* retrieve a relative path */
int mk_com_relPath(char *dst, size_t dstn, const char *curpath,
const char *abspath) {
	size_t i;

	MK_ASSERT( dst != (char *)0 );
	MK_ASSERT( dstn > 1 );
	MK_ASSERT( curpath != (const char *)0 );
	MK_ASSERT( abspath != (const char *)0 );

	i = 0;
	while( mk_com_cmpPathChar(curpath[i], abspath[i]) && curpath[i]!='\0' ) {
		i++;
	}

	if( curpath[i]=='\0' && abspath[i]=='\0' ) {
		mk_com_strcpy(dst, dstn, "");
	} else {
		mk_com_strcpy(dst, dstn, &abspath[i]);
	}

	return 1;
}

/* retrieve a relative path based on the current working directory */
int mk_com_relPathCWD(char *dst, size_t dstn, const char *abspath) {
	char cwd[PATH_MAX];

	if( !mk_fs_getCWD(cwd, sizeof(cwd)) ) {
		return 0;
	}

	return mk_com_relPath(dst, dstn, cwd, abspath);
}

/* retrieve an environment variable */
const char *mk_com_getenv(char *dst, size_t dstn, const char *var)
{
	const char *p;

	MK_ASSERT( dst != ( char * )0 );
	MK_ASSERT( dstn > 0 );
	MK_ASSERT( var != ( const char * )0 );

	if( !( p = getenv( var ) ) ) {
		*dst = '\0';
		return ( const char * )0;
	}

	mk_com_strcpy( dst, dstn, p );
	return dst;
}

/* determine whether a given string starts with another string */
int mk_com_strstarts(const char *src, const char *with)
{
	MK_ASSERT( src != ( const char * )0 );
	MK_ASSERT( with != ( const char * )0 );

	return strncmp( src, with, mk_com_strlen( with ) ) == 0;
}
/* determine whether a given string ends with another string */
int mk_com_strends(const char *src, const char *with)
{
	size_t cSrc, cWith;
	
	MK_ASSERT( src != ( const char * )0 );
	MK_ASSERT( with != ( const char * )0 );
	
	cSrc = mk_com_strlen( src );
	cWith = mk_com_strlen( with );
	
	if( cWith > cSrc ) {
		return 0;
	}
	
	return strcmp( src + ( cSrc - cWith ), with ) == 0;
}

/* perform in-place fixing of the path; sets directory separator to '/' on Windows */
void mk_com_fixpath(char *path)
{
#if MK_WINDOWS_ENABLED
	char *p;
	
	MK_ASSERT( path != ( char * )0 );
	
	p = path;
	while( ( p = strchr( p, '\\' ) ) != ( char * )0 ) {
		*p++ = '/';
	}
#endif

	( void )path;
}


/*
 *	========================================================================
 *	OPTIONS SYSTEM
 *	========================================================================
 *	Retrieve several options. In the future these are to be made more
 *	configurable.
 */

/* retrieve the object directory base (where intermediate files go) */
const char *mk_opt_getObjdirBase(void)
{
	return MK_DEFAULT_OBJDIR_BASE "/" MK_PLATFORM_DIR;
}
/* retrieve the path to the debug log */
const char *mk_opt_getDebugLogPath(void)
{
	return MK_DEFAULT_OBJDIR_BASE "/" MK_DEFAULT_DEBUGLOG_FILENAME;
}
/* retrieve the debug suffix used when naming debug target output */
const char *mk_opt_getDebugSuffix(void)
{
	return MK_DEFAULT_DEBUG_SUFFIX;
}
/* retrieve the color mode used when writing colored output */
MkColorMode_t mk_opt_getColorMode(void)
{
	return mk__g_flags_color;
}
/* retrieve the name of this configuration (debug, release) */
const char *mk_opt_getConfigName(void)
{
	return
		( mk__g_flags & kMkFlag_Release_Bit ) != 0
		? MK_DEFAULT_RELEASE_CONFIG_NAME
		: MK_DEFAULT_DEBUG_CONFIG_NAME;
}

/* retrieve the path to the "gen" directory */
const char *mk_opt_getBuildGenPath(void)
{
	return MK_DEFAULT_OBJDIR_BASE "/gen";
}
/* retrieve the path to the "build-generated" include directory */
const char *mk_opt_getBuildGenIncDir(void)
{
	return MK_DEFAULT_OBJDIR_BASE "/gen/include/build-generated";
}

/* find the path (on this platform) that the global directory (.mk/) should be stored within */
static const char *mk_opt__getGlobalContainerDir(void)
{
	static char szDir[ PATH_MAX ] = { '\0' };

	if( szDir[ 0 ] == '\0' ) {
#if MK_HAS_PWD
		const struct passwd *pwd;
#endif

#define MK__TRYENV(X_) \
	do { if( !szDir[0] ) {\
		mk_com_getenv( szDir, sizeof( szDir ), X_ );\
	}}while(0)

#if MK_PWD_HOMEPATH_DETECTION_ENABLED
		if( !szDir[0] && ( pwd = getpwuid( getuid() ) ) != ( const struct passwd * )0 ) {
			mk_com_strcpy( szDir, sizeof( szDir ), pwd->pw_dir );
		}
#endif
#if MK_WINDOWS_ENABLED
		MK__TRYENV( "LOCALAPPDATA" );
		MK__TRYENV( "USERPROFILE" );
		MK__TRYENV( "APPDATA" );
#endif
		MK__TRYENV( "HOME" );

		mk_com_fixpath( szDir );
		
		if( !szDir[ 0 ] ) {
			mk_com_strcpy( szDir, sizeof( szDir ), "./" );
		} else if( !mk_com_strends( szDir, "/" ) ) {
			mk_com_strcat( szDir, sizeof( szDir ), "/" );
		}
	}
	
	return szDir;
}

/* determine the path that the global directory (.mk/) should exist */
const char *mk_opt_getGlobalDir(void)
{
	static char szDir[ PATH_MAX ] = { '\0' };

	if( szDir[ 0 ] == '\0' ) {
		mk_com_strcpy( szDir, sizeof( szDir ), mk_opt__getGlobalContainerDir() );
		MK_ASSERT( mk_com_strends( szDir, "/" ) && "mk_opt__getGlobalContainerDir() didn't end path with '/'" );
		mk_com_strcat( szDir, sizeof( szDir ), ".mk/" );
	}
	
	return szDir;
}
/* retrieve the .mk/share/ directory, where version-agnostic mk data can be stored */
const char *mk_opt_getGlobalSharedDir(void)
{
	static char szDir[ PATH_MAX ] = { '\0' };
	
	if( szDir[ 0 ] == '\0' ) {
		mk_com_strcpy( szDir, sizeof( szDir ), mk_opt_getGlobalDir() );
		MK_ASSERT( mk_com_strends( szDir, "/" ) && "mk_opt_getGlobalDir() didn't end path with '/'" );
		mk_com_strcat( szDir, sizeof( szDir ), "share/" );
	}
	
	return szDir;
}
/* retrieve the .mk/<version>/ directory, where version-specific mk data can be stored */
const char *mk_opt_getGlobalVersionDir(void)
{
	static char szDir[ PATH_MAX ] = { '\0' };
	
	if( szDir[ 0 ] == '\0' ) {
		mk_com_strcpy( szDir, sizeof( szDir ), mk_opt_getGlobalDir() );
		MK_ASSERT( mk_com_strends( szDir, "/" ) && "mk_opt_getGlobalDir() didn't end path with '/'" );
		mk_com_strcat( szDir, sizeof( szDir ), MK_VERSION_STR "/" );
	}
	
	return szDir;
}

/*
 *	========================================================================
 *	DYNAMIC STRING ARRAY SYSTEM
 *	========================================================================
 *	Manages a dynamic array of strings. Functions similarly to the C++ STL's
 *	std::vector<std::string> class.
 */

struct MkStrList_s {
	size_t capacity;
	size_t size;
	char **data;

	struct MkStrList_s *prev, *next;
};
struct MkStrList_s *mk__g_arr_head = (struct MkStrList_s *)0;
struct MkStrList_s *mk__g_arr_tail = (struct MkStrList_s *)0;

/* create a new (empty) array */
MkStrList mk_sl_new(void) {
	MkStrList arr;

	arr = (MkStrList)mk_com_memory((void *)0, sizeof(*arr));

	arr->capacity = 0;
	arr->size = 0;
	arr->data = (char **)0;

	arr->next = (struct MkStrList_s *)0;
	if( (arr->prev = mk__g_arr_tail) != (struct MkStrList_s *)0 ) {
		mk__g_arr_tail->next = arr;
	} else {
		mk__g_arr_head = arr;
	}
	mk__g_arr_tail = arr;

	return arr;
}

/* retrieve the current amount of mk_com_memory allocated for the array */
size_t mk_sl_getCapacity(MkStrList arr) {
	MK_ASSERT( arr != (MkStrList)0 );

	return arr->capacity;
}

/* retrieve how many elements are in the array */
size_t mk_sl_getSize(MkStrList arr) {
	if( !arr ) {
		return 0;
	}

	return arr->size;
}

/* retrieve the data of the array */
char **mk_sl_getData(MkStrList arr) {
	MK_ASSERT( arr != (MkStrList)0 );

	return arr->data;
}

/* retrieve a single element of the array */
char *mk_sl_at(MkStrList arr, size_t i) {
	MK_ASSERT( arr != (MkStrList)0 );
	MK_ASSERT( i < arr->size );

	return arr->data[i];
}

/* set a single element of the array */
void mk_sl_set(MkStrList arr, size_t i, const char *cstr) {
	MK_ASSERT( arr != (MkStrList)0 );
	MK_ASSERT( i < arr->size );

	if( !cstr ) {
		arr->data[i] = (char *)mk_com_memory((void *)arr->data[i], 0);
	} else {
		arr->data[i] = mk_com_strdup(cstr);
	}
}

/* deallocate the internal mk_com_memory used by the array */
void mk_sl_clear(MkStrList arr) {
	size_t i;

	MK_ASSERT( arr != (MkStrList)0 );

	for( i=0; i<arr->size; i++ ) {
		arr->data[i] = (char *)mk_com_memory((void *)arr->data[i], 0);
	}

	arr->data = (char **)mk_com_memory((void *)arr->data, 0);
	arr->capacity = 0;
	arr->size = 0;
}

/* delete an array */
void mk_sl_delete(MkStrList arr) {
	if( !arr ) {
		return;
	}

	mk_sl_clear(arr);

	if( arr->prev ) {
		arr->prev->next = arr->next;
	}
	if( arr->next ) {
		arr->next->prev = arr->prev;
	}

	if( mk__g_arr_head==arr ) {
		mk__g_arr_head = arr->next;
	}
	if( mk__g_arr_tail==arr ) {
		mk__g_arr_tail = arr->prev;
	}

	mk_com_memory((void *)arr, 0);
}

/* delete all arrays */
void mk_sl_deleteAll(void) {
	while( mk__g_arr_head ) {
		mk_sl_delete(mk__g_arr_head);
	}
}

/* set the new size of an array */
void mk_sl_resize(MkStrList arr, size_t n) {
	size_t i;

	MK_ASSERT( arr != (MkStrList)0 );

	if( n > arr->capacity ) {
		i = arr->capacity;
		arr->capacity += 4096/sizeof(void *);
		/*arr->capacity += 32;*/

		arr->data = (char **)mk_com_memory((void *)arr->data,
			arr->capacity*sizeof(char *));

		memset((void *)&arr->data[i], 0, (arr->capacity - i)*sizeof(char *));
	}

	arr->size = n;
}

/* add an element to the array, resizing if necessary */
void mk_sl_pushBack(MkStrList arr, const char *cstr) {
	size_t i;

	MK_ASSERT( arr != (MkStrList)0 );

	i = mk_sl_getSize(arr);
	mk_sl_resize(arr, i+1);
	mk_sl_set(arr, i, cstr);
}
/* remove the last element in the array */
void mk_sl_popBack(MkStrList arr) {
	MK_ASSERT( arr != (MkStrList)0 );

	if( !arr->size ) {
		return;
	}

	mk_sl_set(arr, arr->size - 1, (const char *)0);
	--arr->size;
}

/* display the contents of an array */
void mk_sl_print(MkStrList arr) {
	size_t i, n;

	MK_ASSERT( arr != (MkStrList)0 );

	n = mk_sl_getSize(arr);
	for( i=0; i<n; i++ ) {
#if MK_WINDOWS_ENABLED
		printf("%2u. \"%s\"\n", (unsigned int)i, mk_sl_at(arr, i));
#else
		/* ISO C90 does not support the 'z' modifier; ignore... */
		printf("%2zu. \"%s\"\n", i, mk_sl_at(arr, i));
#endif
	}

	printf("\n");
}
void mk_sl_debugPrint(MkStrList arr) {
	size_t i, n;

	MK_ASSERT( arr != (MkStrList)0 );

	n = mk_sl_getSize(arr);
	for( i=0; i<n; i++ ) {
#if MK_WINDOWS_ENABLED
		mk_dbg_outf("%2u. \"%s\"\n", (unsigned int)i, mk_sl_at(arr, i));
#else
		/* ISO C90 does not support the 'z' modifier; ignore... */
		mk_dbg_outf("%2zu. \"%s\"\n", i, mk_sl_at(arr, i));
#endif
	}

	mk_dbg_outf("\n");
}

/* alphabetically sort an array */
static int _SL_Cmp(const void *a, const void *b) {
	return strcmp(*(char *const *)a, *(char *const *)b);
}

void mk_sl_sort(MkStrList arr) {
	if( arr->size < 2 ) {
		return;
	}

	qsort((void *)arr->data, arr->size, sizeof(char *), _SL_Cmp);
}

void mk_sl_orderedSort(MkStrList arr, size_t *const buffer, size_t maxBuffer) {
	size_t i, j;
	size_t n;

	MK_ASSERT( buffer != NULL );
	MK_ASSERT( maxBuffer >= arr->size );

	n = arr->size < maxBuffer ? arr->size : maxBuffer;

	for( i=0; i<n; i++ ) {
		buffer[i] = (size_t)arr->data[i];
	}

	mk_sl_sort(arr);

	for( i=0; i<n; i++ ) {
		for( j=0; j<n; j++ ) {
			if( buffer[i]==(size_t)arr->data[j] ) {
				buffer[i] = j;
				break;
			}
		}

		MK_ASSERT( j<n ); /* did exit from loop? */
	}
}
void mk_sl_indexedSort(MkStrList arr, const size_t *const buffer, size_t bufferLen) {
#if 1
	MkStrList backup;
	size_t i, n;

	/*
	 *	XXX: This is ugly.
	 *	FIXME: Ugly.
	 *	XXX: You can't fix ugly.
	 *	TODO: Buy a makeup kit for the beast.
	 *
	 *	In all seriousness, an in-place implementation should be written. This works for now,
	 *	though.
	 */

	MK_ASSERT( buffer != NULL );
	MK_ASSERT( bufferLen > 0 );

	backup = mk_sl_new();
	n = bufferLen < arr->size ? bufferLen : arr->size;
	for( i=0; i<n; i++ ) {
		mk_sl_pushBack(backup, mk_sl_at(arr, i));
	}

	for( i=0; i<n; i++ ) {
		mk_sl_set(arr, buffer[i], mk_sl_at(backup, i));
	}

	mk_sl_delete(backup);
#else
	char *tmp;
	size_t i;

	/*
	 *	FIXME: This partially works, but produces bad results.
	 */

	for( i=0; i<bufferLen; i++ ) {
		if( i > buffer[i] )
			continue;

		tmp = arr->data[buffer[i]];
		arr->data[buffer[i]] = arr->data[i];
		arr->data[i] = tmp;
	}
#endif
}
void mk_sl_printOrderedBuffer(const size_t *const buffer, size_t bufferLen) {
	size_t i;

	printf("array buffer %p (%u element%s):\n", (void *)buffer,
		(unsigned int)bufferLen, bufferLen==1?"":"s");
	for( i=0; i<bufferLen; i++ ) {
		printf("  %u\n", (unsigned int)buffer[i]);
	}
	printf("\n");
}

/* remove duplicate entries from an array */
void mk_sl_makeUnique(MkStrList arr) {
	const char *a, *b;
	size_t i, j, k, n;

	n = mk_sl_getSize(arr);
	for( i=0; i<n; i++ ) {
		if( !(a = mk_sl_at(arr, i)) ) {
			continue;
		}

		for( j=i+1; j<n; j++ ) {
			if( !(b = mk_sl_at(arr, j)) ) {
				continue;
			}

			if( strcmp(a, b)==0 ) {
				mk_sl_set(arr, j, (const char *)0);
			}
		}
	}

	arr->size = 0;
	for( i=0; i<n; ++i ) {
		if( arr->data[i]!=(char *)0 ) {
			arr->size = i + 1;
			continue;
		}

		j = i + 1;
		while( j < n && arr->data[ j ] == ( char * )0 ) {
			++j;
		}

		if( j == n ) {
			break;
		}

		for( k = 0; j + k < n; ++k ) {
			arr->data[i+k] = arr->data[j+k];
		}

		--i;
	}
}

/*
 *	========================================================================
 *	BUFFER MANAGEMENT CODE
 *	========================================================================
 *	Manage input source file buffers (similar in nature to flex's buffer).
 *	Keeps track of the file's name and the current line.
 */
struct MkBuffer_s {
	char *file;
	char *func;

	char *text;
	char *endPtr;
	char *ptr;

	size_t numLines;
	char **linePtrs;
};

static int mk_buf__copyRange(char **dst, const char *src, size_t len) {
	size_t n;
	char *p;

	if( !src ) {
		if( *dst ) {
			mk_com_memory((void *)*dst, 0);
			*dst = NULL;
		}

		return 1;
	}

	len = len ? len : strlen(src);
	n = len + 1;

	p = (char *)mk_com_memory((void *)*dst, n);
	if( !p ) {
		return 0;
	}

	*dst = p;

	memcpy(p, src, len);
	p[len] = '\0';

	return 1;
}
static int mk_buf__copy(char **dst, const char *src) {
	return mk_buf__copyRange(dst, src, 0);
}
#if 0
static int mk_buf__append(char **dst, const char *src, size_t *offset) {
	char *p;
	size_t len;

	if( (!src) || (!*dst && *offset>0) ) {
		return 0;
	}

	if( !*dst ) {
		return mk_buf__copy(dst, src);
	}

	len = strlen(src) + 1;

	p = (char *)(mk_com_memory((void *)*dst, *offset + len));
	if( !p ) {
		return 0;
	}

	*dst = p;

	memcpy(&((*dst)[*offset]), src, len);
	*offset += len - 1;

	return 1;
}
#endif

int mk_buf_setFilename(MkBuffer text, const char *filename) {
	return mk_buf__copy(&text->file, filename);
}
int mk_buf_setFunction(MkBuffer text, const char *func) {
	return mk_buf__copy(&text->func, func);
}

static int mk_buf__addLinePtr(MkBuffer text, char *ptr) {
	char **arr;
	size_t amt;

	if( !ptr ) {
		if( text->linePtrs ) {
			mk_com_memory((void *)(text->linePtrs), 0);
			text->linePtrs = NULL;
			text->numLines = 0;
		}

		return 1;
	}

	amt = sizeof(char *)*(text->numLines + 1);
	arr = (char **)mk_com_memory((void *)text->linePtrs, amt);
	if( !arr ) {
		return 0;
	}

	text->linePtrs = arr;
	text->linePtrs[text->numLines++] = ptr;

	return 1;
}

static MkBuffer mk_buf__alloc() {
	MkBuffer text;

	text = (MkBuffer )mk_com_memory(0, sizeof(*text));
	if( !text ) {
		return NULL;
	}

	text->file		= NULL;
	text->func		= NULL;

	text->text		= NULL;
	text->endPtr	= NULL;
	text->ptr		= NULL;

	text->numLines	= 0;
	text->linePtrs	= NULL;

	return text;
}
static void mk_buf__fini(MkBuffer text) {
	mk_buf__addLinePtr(text, NULL);

	mk_buf__copy(&text->file, NULL);
	mk_buf__copy(&text->func, NULL);
	text->ptr = NULL;
}

#if MK_PROCESS_NEWLINE_CONCAT_ENABLED
static const char *mk_buf__skipNewline(const char *p) {
	const char *e;

	e = NULL;
	if( *p=='\r' ) {
		e = ++p;
	}

	if( *p=='\n' ) {
		e = ++p;
	}

	if( *p=='\0' ) {
		e = p;
	}

	return e;
}
#endif
static int mk_buf__initFromMemory(MkBuffer text, const char *filename, const char *source, size_t len) {
#if MK_PROCESS_NEWLINE_CONCAT_ENABLED
	const char *s, *e, *next;
	const char *add;
#else
	int add;
#endif

	MK_ASSERT( !!text );
	MK_ASSERT( !!filename );
	MK_ASSERT( !!source );

	if( !mk_buf_setFilename(text, filename) ) {
		return 0;
	}

#if MK_PROCESS_NEWLINE_CONCAT_ENABLED
	if( !len ) {
		len = strlen( source );
	}

	text->text = mk_com_memory(0, len + 1);
	if( !text->text ) {
		return 0;
	}

	*text->text = '\0';
#else
	if( !mk_buf__copyRange(&text->text, source, len) ) {
		return 0;
	}
#endif

	mk_buf__addLinePtr(text, NULL);
	mk_buf__addLinePtr(text, text->text);

#if MK_PROCESS_NEWLINE_CONCAT_ENABLED
	text->ptr = text->text;
	s = source;
	for( e=s; *e; e=next ) {
		next = e + 1;
		add = NULL; /*this will point to AFTER the newline or AT the EOF*/

		if( *e=='\\' ) {
			add = mk_buf__skipNewline(e + 1);
			if( !add ) {
				continue;
			}
		} else {
			add = mk_buf__skipNewline(e);
			if( !add ) {
				continue;
			}

			e = add; /*want newlines in source when not escaped!*/
		}

		memcpy(text->ptr, s, e - s);
		text->ptr += e - s;

		s = add;
		next = add;

		if( !mk_buf__addLinePtr(text, text->ptr) ) {
			mk_buf__fini(text);
			return 0;
		}
	}

	*text->ptr = '\0';
#else
	for( text->ptr=text->text; *text->ptr!='\0'; text->ptr++ ) {
		add = 0;

		if( *text->ptr=='\r' ) {
			if( text->ptr[1]=='\n' ) {
				text->ptr++;
			}
			add = 1;
		}
		if( *text->ptr=='\n' ) {
			add = 1;
		}

		if( add && !mk_buf__addLinePtr(text, &text->ptr[1]) ) {
			mk_buf__fini(text);
			return 0;
		}
	}
#endif

	text->endPtr = text->ptr;
	text->ptr = text->text;

#if 0
	printf("******************************************************\n");
	printf("#### ORIGINAL ####\n");
	printf("%s\n", source);
	printf("******************************************************\n");
	printf("#### READ-IN ####\n");
	printf("%s\n", text->text);
	printf("******************************************************\n");
#endif

	return 1;
}
static int mk_buf__initFromFile(MkBuffer text, const char *filename) {
#if 0
		FILE *f;
		char *p, buf[8192];
		size_t offset;
		int r;

	#if MK_SECLIB
		if( fopen_s(&f, filename, "rb")!=0 ) {
			f = NULL;
		}
	#else
		f = fopen(filename, "rb");
	#endif
		if( !f ) {
			return 0;
		}

		p = NULL;
		offset = 0;
		r = 1;

		while( !feof(f) ) {
			int n;

			n = fread(buf, 1, sizeof(buf) - 1, f);
			if( n <= 0 ) {
				r = !ferror(f);
				break;
			}

			buf[ n ] = '\0';

			if( !mk_buf__append(&p, buf, &offset) ) {
				r = 0;
				break;
			}
		}

		fclose(f);

		if( !r || !mk_buf__initFromMemory(text, filename, p, 0) ) {
			mk_buf__copy(&p, NULL);
			return 0;
		}

		return 1;
#else
	FILE *f;
	char *p;
	size_t n;

# if MK_SECLIB
	if( fopen_s(&f, filename, "rb")!=0 ) {
		f = NULL;
	}
# else
	f = fopen(filename, "rb");
# endif
	if( !f ) {
		return 0;
	}

	fseek( f, 0, SEEK_END );
	n = ( size_t )ftell( f );

	fseek( f, 0, SEEK_SET );

	p = ( char * )mk_com_memory( NULL, n + 1 );
	if( !fread( ( void * )p, n, 1, f ) ) {
		mk_com_memory( ( void * )p, 0 );
		fclose( f );
		return 0;
	}

	fclose( f );
	f = NULL;

	p[ n ] = '\0';
	if( !mk_buf__initFromMemory(text, filename, p, 0) ) {
		mk_com_memory( ( void * )p, 0 );
		return 0;
	}

	mk_com_memory( ( void * )p, 0 );
	return 1;
#endif
}

MkBuffer mk_buf_loadMemoryRange(const char *filename, const char *source, size_t len) {
	MkBuffer text;

	text = mk_buf__alloc();
	if( !text ) {
		return NULL;
	}

	if( !mk_buf__initFromMemory(text, filename, source, len) ) {
		mk_com_memory((void *)text, 0);
		return NULL;
	}

	return text;
}
MkBuffer mk_buf_loadMemory(const char *filename, const char *source) {
	return mk_buf_loadMemoryRange(filename, source, 0);
}
MkBuffer mk_buf_loadFile(const char *filename) {
	MkBuffer text;

	text = mk_buf__alloc();
	if( !text ) {
		return NULL;
	}

	if( !mk_buf__initFromFile(text, filename) ) {
		mk_com_memory((void *)text, 0);
		return NULL;
	}

	return text;
}
MkBuffer mk_buf_delete(MkBuffer text) {
	if( !text ) {
		return NULL;
	}

	mk_buf__fini(text);
	mk_com_memory((void *)text, 0);

	return NULL;
}

const char *mk_buf_getFilename(const MkBuffer text) {
	return text->file;
}
const char *mk_buf_getFunction(const MkBuffer text) {
	return text->func;
}
size_t mk_buf_calculateLine(const MkBuffer text) {
	size_t i;

	for( i=0; i<text->numLines; i++ ) {
		if( text->ptr < text->linePtrs[i] ) {
			break;
		}
	}

	return i + 1;
}

size_t mk_buf_getLength(const MkBuffer text) {
	return text->endPtr - text->text;
}

void mk_buf_seek(MkBuffer text, size_t pos) {
	text->ptr = &text->text[pos];

	if( text->ptr > text->endPtr ) {
		text->ptr = text->endPtr;
	}
}
size_t mk_buf_tell(const MkBuffer text) {
	return text->ptr - text->text;
}
char *mk_buf_getPtr(MkBuffer text) {
	return text->ptr;
}

char mk_buf_read(MkBuffer text) {
	char c;

	c = *text->ptr++;
	if( text->ptr > text->endPtr ) {
		text->ptr = text->endPtr;
	}

	return c;
}
char mk_buf_peek(MkBuffer text) {
	return *text->ptr;
}
char mk_buf_lookAhead(MkBuffer text, size_t offset) {
	if( text->ptr + offset >= text->endPtr ) {
		return '\0';
	}

	return text->ptr[offset];
}
int mk_buf_advanceIfCharEq(MkBuffer text, char ch) {
	if( *text->ptr != ch ) {
		return 0;
	}

	if( ++text->ptr > text->endPtr ) {
		text->ptr = text->endPtr;
	}

	return 1;
}

void mk_buf_skip(MkBuffer text, size_t offset) {
	mk_buf_seek(text, text->ptr - text->text + offset);
}
void mk_buf_skipWhite(MkBuffer text) {
	while( (unsigned char)(*text->ptr)<=' ' && text->ptr<text->endPtr ) {
		text->ptr++;
	}
}
void mk_buf_skipLine(MkBuffer text) {
	int r;

	r = 0;
	while( text->ptr < text->endPtr ) {
		if( *text->ptr=='\r' ) {
			r = 1;
			text->ptr++;
		}
		if( *text->ptr=='\n' ) {
			r = 1;
			text->ptr++;
		}

		if( r ) {
			return;
		}

		text->ptr++;
	}
}

int mk_buf_skipLineIfStartsWith(MkBuffer text, const char *pszCommentDelim) {
	size_t n;

	MK_ASSERT( pszCommentDelim != ( const char * )0 );

	n = strlen( pszCommentDelim );
	if( text->ptr + n > text->endPtr ) {
		return 0;
	}

	if( strncmp( text->ptr, pszCommentDelim, ( int )n ) != 0 ) {
		return 0;
	}

	text->ptr += n;
	mk_buf_skipLine( text );

	return 1;
}

int mk_buf_readLine(MkBuffer text, char *dst, size_t dstn) {
	const char *s, *e;

	s = text->ptr;
	if( s==text->endPtr ) {
		return -1;
	}

	mk_buf_skipLine( text );
	e = text->ptr;

	if( e > s && *( e - 1 ) == '\n' ) {
		--e;
	}

	if( e > s && *( e - 1 ) == '\r' ) {
		--e;
	}

	mk_com_strncpy( dst, dstn, s, e - s );

	return (int)(e - s);
}

void mk_buf_errorfv(MkBuffer text, const char *format, va_list args) {
	char buf[ 4096 ];

#if __STDC_WANT_SECURE_LIB__
	vsprintf_s(buf, sizeof(buf), format, args);
#else
	vsnprintf(buf, sizeof(buf), format, args);
	buf[sizeof(buf) - 1] = '\0';
#endif

	mk_log_error(text->file, mk_buf_calculateLine(text), text->func, buf);
}
void mk_buf_errorf(MkBuffer text, const char *format, ...) {
	va_list args;

	va_start(args, format);
	mk_buf_errorfv(text, format, args);
	va_end(args);
}


/*
 *	========================================================================
 *	FILE SYSTEM MANAGEMENT CODE
 *	========================================================================
 *	This code deals with various file system related subjects. This includes
 *	making directories and finding where the executable is, etc.
 */

MkStrList mk__g_fs_dirstack = (MkStrList)0;

/* initialize file system */
void mk_fs_init(void) {
	atexit(mk_fs_fini);
}

/* deinitialize file system */
void mk_fs_fini(void) {
	mk_sl_delete(mk__g_fs_dirstack);
	mk__g_fs_dirstack = (MkStrList)0;
}

/* retrieve the current directory */
char *mk_fs_getCWD(char *cwd, size_t n) {
	char *p;

	MK_ASSERT( cwd != (char *)0 );
	MK_ASSERT( n > 1 );

#if MK_VC_VER
	if( !(p = _getcwd(cwd, n)) ) {
		mk_log_fatalError("getcwd() failed");
	}
#else
	if( !(p = getcwd(cwd, n)) ) {
		mk_log_fatalError("getcwd() failed");
	}
#endif

#if MK_WINDOWS_ENABLED
	while( (p=strchr(p,'\\')) != (char *)0 ) {
		*p = '/';
	}
#endif

	p = strrchr(cwd, '/');
	if( (p && *(p + 1) != '\0') || !p ) {
		mk_com_strcat(cwd, n, "/");
	}

	return cwd;
}

/* enter a directory (uses directory stack) */
int mk_fs_enter(const char *path) {
	char cwd[PATH_MAX];

	MK_ASSERT( path != ( const char * )0 );

	mk_dbg_enter("mk_fs_enter(\"%s\")", path);

	mk_fs_getCWD(cwd, sizeof(cwd));

	if( !mk__g_fs_dirstack ) {
		mk__g_fs_dirstack = mk_sl_new();
	}

	mk_sl_pushBack(mk__g_fs_dirstack, cwd);

	if( chdir(path)==-1 ) {
		mk_log_errorMsg(mk_com_va("chdir(^F\"%s\"^&) failed", path));
		if( chdir(cwd)==-1 ) {
			mk_log_fatalError("failed to restore current directory");
		}

		return 0;
	}

	return 1;
}

/* exit the current directory (uses directory stack) */
void mk_fs_leave(void) {
	size_t i;

	MK_ASSERT( mk__g_fs_dirstack != (MkStrList)0 );
	MK_ASSERT( mk_sl_getSize(mk__g_fs_dirstack) > 0 );

	mk_dbg_leave();

	i = mk_sl_getSize(mk__g_fs_dirstack)-1;

	if( chdir(mk_sl_at(mk__g_fs_dirstack, i))==-1 ) {
		mk_log_fatalError(mk_com_va("chdir(\"%s\") failed",
			mk_sl_at(mk__g_fs_dirstack, i)));
	}

	mk_sl_set(mk__g_fs_dirstack, i, (const char *)0);
	mk_sl_resize(mk__g_fs_dirstack, i);
}

/* determine whether the path specified is a file. */
int mk_fs_isFile(const char *path) {
	MkStat_t s;

	if( stat(path, &s)==-1 ) {
		return 0;
	}

	if( ~s.st_mode & S_IFREG ) {
		errno = s.st_mode & S_IFDIR ? EISDIR : EBADF;
		return 0;
	}

	return 1;
}

/* determine whether the path specified is a directory. */
int mk_fs_isDir(const char *path) {
	static char temp[PATH_MAX];
	MkStat_t s;
	const char *p;

	p = strrchr(path, '/');
	if( p && *(p + 1)=='\0' ) {
		mk_com_strncpy(temp, sizeof(temp), path, p - path);
		temp[p - path] = '\0';
		p = &temp[0];
	} else {
		p = path;
	}

	if( stat(p, &s)==-1 ) {
		return 0;
	}

	if( ~s.st_mode & S_IFDIR ) {
		errno = ENOTDIR;
		return 0;
	}

	return 1;
}

/* create a series of directories (e.g., a/b/c/d/...) */
void mk_fs_makeDirs(const char *dirs) {
	/*
	 *	! This is old code !
	 *	Just ignore bad practices, mmkay?
	 */
	const char *p;
	char buf[PATH_MAX], *path;
	int ishidden;

	/* ignore the root directory */
	if( dirs[0]=='/' ) {
		buf[0] = dirs[0];
		path = &buf[1];
		p = &dirs[1];
	} else if(dirs[1]==':' && dirs[2]=='/') {
		buf[0] = dirs[0];
		buf[1] = dirs[1];
		buf[2] = dirs[2];
		path = &buf[3];
		p = &dirs[3];
	} else {
		path = &buf[0];
		p = &dirs[0];
	}

	/* not hidden unless path component begins with '.' */
	ishidden = ( int )( dirs[0] == '.' );

	/* make each directory, one by one */
	while( 1 ) {
		if(*p=='/' || *p==0) {
			*path = 0;

			errno = 0;
#if MK_WINDOWS_ENABLED
			mkdir(buf);
#else
			mkdir(buf, 0740);
#endif
			if( errno && errno!=EEXIST ) {
				mk_log_fatalError(mk_com_va("couldn't create directory \"%s\"", buf));
			}

#ifdef _WIN32
			if( ishidden ) {
				SetFileAttributesA(buf, FILE_ATTRIBUTE_HIDDEN);
			}
#endif

			if( p[0] == '/' && p[1] == '.' ) {
				ishidden = 1;
			} else {
				ishidden = 0;
			}

			if( !(*path++ = *p++) ) {
				return;
			} else if(*p=='\0') { /* handle a '/' ending */
				return;
			}
		} else {
			*path++ = *p++;
		}

		if( path==&buf[sizeof(buf)-1] )
			mk_log_fatalError("path is too long");
	}
}

/* find the real path to a file */
#if MK_WINDOWS_ENABLED
char *mk_fs_realPath(const char *filename, char *resolvedname, size_t maxn) {
	static char buf[PATH_MAX];
	size_t i;
	DWORD r;

	if( !(r = GetFullPathNameA(filename, sizeof(buf), buf, (char **)0)) ) {
		errno = ENOSYS;
		return (char *)0;
	} else if(r >= (DWORD)maxn) {
		errno = ERANGE;
		return (char *)0;
	}

	for( i=0; i<sizeof(buf); i++ ) {
		if( !buf[i] ) {
			break;
		}

		if( buf[i]=='\\' ) {
			buf[i] = '/';
		}
	}

	if( buf[1]==':' ) {
		if( buf[0]>='A' && buf[0]<='Z' ) {
			buf[0] = buf[0] - 'A' + 'a';
		}
	}

	strncpy(resolvedname, buf, maxn-1);
	resolvedname[maxn-1] = 0;

	return resolvedname;
}
#else /*#elif __linux__||__linux||linux*/
char *mk_fs_realPath(const char *filename, char *resolvedname, size_t maxn) {
	static char buf[PATH_MAX + 1];

	if( maxn > PATH_MAX ) {
		if( !realpath(filename, resolvedname) ) {
			return (char *)0;
		}

		resolvedname[PATH_MAX] = 0;
		return resolvedname;
	}

	if( !realpath(filename, buf) ) {
		return (char *)0;
	}

	buf[PATH_MAX] = 0;
	strncpy(resolvedname, buf, maxn);

	if( mk_fs_isDir(resolvedname) ) {
		mk_com_strcat(resolvedname, maxn, "/");
	}

	resolvedname[maxn-1] = 0;
	return resolvedname;
}
#endif

DIR *mk_fs_openDir(const char *path) {
	static char buf[PATH_MAX];
	const char *p, *usepath;
	DIR *d;

	mk_dbg_outf("mk_fs_openDir(\"%s\")\n", path);

	p = strrchr(path, '/');
	if( p && *(p + 1) == '\0' ) {
		mk_com_strncpy(buf, sizeof(buf), path, (size_t)(p - path));
		usepath = buf;
	} else {
		usepath = path;
	}

	if( !( d = opendir(usepath) ) ) {
		const char *es;
		int e;

		e = errno;
		es = strerror( e );

		mk_dbg_outf( "\tKO: %i (%s)\n", e, es );
		return ( DIR * )0;
	}

	mk_dbg_outf( "\tOK: #%x\n", (unsigned)(size_t)d );
	return d;
}
DIR *mk_fs_closeDir(DIR *p) {
	int e;

	e = errno;
	mk_dbg_outf( "mk_fs_closeDir(#%x)\n", (unsigned)(size_t)p );
	if( p != ( DIR * )0 ) {
		closedir( p );
	}
	errno = e;

	return ( DIR * )0;
}
struct dirent *mk_fs_readDir(DIR *d) {
	struct dirent *dp;

	/* Okay... how is errno getting set? From what? */
	if( errno ) {
		mk_log_error(NULL, 0, "mk_fs_readDir", NULL);
	}

	/* TEMP-FIX: buggy version of readdir() sets errno? */
	dp = readdir(d);
	if( dp != ( struct dirent * )0 || errno == ENOTDIR ) {
		errno = 0;
	}

	return dp;
}

void mk_fs_remove(const char *path) {
	if( mk_fs_isDir( path ) ) {
		DIR *d;
		struct dirent *dp;
		char *filepath;
		char *filepath_namepart;

		filepath = ( char * )mk_com_memory( NULL, PATH_MAX );
		mk_com_strcpy( filepath, PATH_MAX, path );

		filepath_namepart = strchr( filepath, '\0' );
		if( filepath_namepart > filepath ) {
			char dirsep;

			dirsep = *( filepath_namepart - 1 );
			if( dirsep != '/' && dirsep != '\\' ) {
				*filepath_namepart++ = '/';
			}
		}

		d = mk_fs_openDir( path );
		while( ( dp = mk_fs_readDir( d ) ) != NULL ) {
			if( strcmp( dp->d_name, "." ) == 0 ) {
				continue;
			}
			if( strcmp( dp->d_name, ".." ) == 0 ) {
				continue;
			}

			mk_com_strcpy( filepath_namepart,
				PATH_MAX - ( size_t )filepath_namepart, dp->d_name );
			mk_fs_remove( filepath );
		}
		mk_fs_closeDir(d);

		filepath = ( char * )mk_com_memory( ( void * )filepath, 0 );

		mk_dbg_outf( "Deleting directory \"%s\"...\n", path );
		rmdir( path );
	} else {
		errno = 0;

		mk_dbg_outf( "Deleting file \"%s\"...\n", path );
		remove( path );
	}
}

/*
 *	=======================================================================
 *	GIT VERSION CONTROL INFO
 *	=======================================================================
 *	Retrieve information about the git repo from the perspective of the
 *	current directory. Header information can then be written to a
 *	gitinfo.h file that the project can include.
 *
 *	e.g., a project might include the gitinfo.h in one of their version
 *	`     source files (such as for an "about box" or debug logging) as
 *	`     follows:
 *
 *		#include <build-generated/gitinfo.h>
 *
 *	The file will define the following macros, if git is used:
 *
 *		BUILDGEN_GITINFO_BRANCH (e.g., "master")
 *		BUILDGEN_GITINFO_COMMIT (e.g., "9d4bf863ed1ea878fde62756eea2c6e85fc7a6de")
 *		BUILDGEN_GITINFO_TSTAMP (e.g., "2016-01-10 16:47:11")
 *
 *	If git is used, BUILDGEN_GITINFO_AVAILABLE will be defined to 1,
 *	otherwise it will be defined to 0.
 *
 *	It is planned to expand this in the future to also grab the tag of the
 *	branch, if any, which may be used for versioning. (As Swift's packages
 *	use.) Then, should zip/tar file generation be supported, said files can
 *	also use the version they're defined against.
 */

struct MkGitInfo_s
{
	char *pszBranchFile;
	char *pszCommit;
	char *pszTimestamp;
};

/* find the nearest ".git" directory at or below the current path */
char *mk_git_findRoot( void )
{
	/* szDir always ends with '/'; szGitDir is szDir + ".git" */
	char szDir[ PATH_MAX ], szGitDir[ PATH_MAX + 8 ];
	/* pszDirPos always points to the last '/' in szDir */
	char *pszDirPos;

	/* start searching from the current directory */
	if( !mk_fs_getCWD( szDir, ( int )sizeof( szDir ) - 1 ) ) {
		return ( char * )0;
	}

	/* find the nul terminator */
	pszDirPos = strchr( szDir, '\0' );

	/* quick sanity check */
	if( pszDirPos == &szDir[ 0 ] || !pszDirPos ) {
		mk_dbg_outf( "mk_git_findRoot: mk_fs_getCWD returned an empty directory path\n" );
		return ( char * )0;
	}

	/* `szDir` must end with '/' and `pszDirPos` must point to the last '/' */
	if( *( pszDirPos - 1 ) != '/' ) {
		pszDirPos[ 0 ] = '/';
		pszDirPos[ 1 ] = '\0';
	} else {
		--pszDirPos;
	}

	mk_dbg_outf( "Searching for .git directory, starting from: <%s>\n", szDir );

	/* find the .git directory (if successful, this part returns) */
	do {
		/* construct the .git path name */
		mk_com_strcpy( szGitDir, sizeof( szGitDir ), szDir );
		mk_com_strcat( szGitDir, sizeof( szGitDir ), ".git" );
		mk_dbg_outf( "\tTrying: <%s>\n", szGitDir );

		/* if this is a valid .git directory, then done */
		if( mk_fs_isDir( szGitDir ) ) {
			mk_dbg_outf( "\t\tFound!\n" );
			return mk_com_dup( ( char * )0, szGitDir );
		}

		/* find the beginning of the next directory up */
		while( --pszDirPos > &szDir[ 0 ] ) {
			if( *pszDirPos == '/' ) {
				pszDirPos[ 1 ] = '\0';
				break;
			}
		}
	} while( pszDirPos != &szDir[ 0 ] );

	/* no .git directory found */
	mk_dbg_outf( "\tNo .git found\n" );
	return ( char * )0;
}
/* find the file belonging to the current branch of the given .git directory */
char *mk_git_findBranchPath( const char *pszGitDir )
{
	static const char szRefPrefix[] = "ref: ";
	static const size_t cRefPrefix = sizeof( szRefPrefix ) - 1;
	MkBuffer buf;
	char szHeadFile[ PATH_MAX ];
	char szRefLine[ 512 ];

	MK_ASSERT( pszGitDir != ( const char * )0 && *pszGitDir != '\0' );

	mk_com_strcpy( szHeadFile, sizeof( szHeadFile ), pszGitDir );
	mk_com_strcat( szHeadFile, sizeof( szHeadFile ), "/HEAD" );

	mk_dbg_outf( "mk_git_findBranchPath: %s: <%s>\n", pszGitDir, szHeadFile );
	if( !( buf = mk_buf_loadFile( szHeadFile ) ) ) {
		return ( char * )0;
	}

	mk_buf_readLine( buf, szRefLine, sizeof( szRefLine ) );
	mk_dbg_outf( "mk_git_findBranchPath: %s: <%s>\n", szHeadFile, szRefLine );
	mk_buf_delete( buf );

	if( strncmp( szRefLine, szRefPrefix, ( int )( unsigned )cRefPrefix ) != 0 ) {
		mk_dbg_outf( "%s: first line does not begin with '%s'\n", szHeadFile, szRefPrefix );
		return ( char * )0;
	}

	return mk_com_dup( ( char * )0, mk_com_va( "%s/%s", pszGitDir, &szRefLine[ cRefPrefix ] ) );
}
/* get the name of the latest commit from the given branch file */
char *mk_git_getCommit( const char *pszBranchFile )
{
	MkBuffer buf;
	char szCommitLine[ 512 ];

	MK_ASSERT( pszBranchFile != ( const char * )0 && *pszBranchFile != '\0' );

	if( !( buf = mk_buf_loadFile( pszBranchFile ) ) ) {
		return ( char * )0;
	}

	mk_buf_readLine( buf, szCommitLine, sizeof( szCommitLine ) );
	mk_dbg_outf( "mk_git_getCommit(\"%s\"): <%s>\n", pszBranchFile, szCommitLine );
	mk_buf_delete( buf );

	return mk_com_dup( ( char * )0, szCommitLine );
}
/* get the timestamp of the latest commit from the given branch file */
char *mk_git_getCommitTimestamp( const char *pszBranchFile, const MkStat_t *pStat )
{
	MkStat_t s;
	struct tm *t;
	char szBuf[ 128 ];

	MK_ASSERT( pszBranchFile != ( const char * )0 && *pszBranchFile != '\0' );

	if( pStat != ( const MkStat_t * )0 ) {
		s = *pStat;
	} else if( stat( pszBranchFile, &s ) != 0 ) {
		mk_log_error( pszBranchFile, 0, ( const char * )0, "Stat failed" );
		return ( char * )0;
	}

	if( !( t = localtime( &s.st_mtime ) ) ) {
		mk_log_error( pszBranchFile, 0, ( const char * )0, "Could not determine time from stat" );
		return ( char * )0;
	}

	strftime( szBuf, sizeof( szBuf ), "%Y-%m-%d %H:%M:%S", t );
	mk_dbg_outf( "mk_git_getCommitTimestamp(\"%s\"): %s\n", pszBranchFile, szBuf );
	return mk_com_dup( ( char * )0, szBuf );
}

/* load gitinfo from the cache or generate it (NOTE: expects to be in project's source directory) */
MkGitInfo mk_git_loadInfo( void )
{
	static const char *const pszHeader = "mk-gitinfo-cache::v1";
	static const char *const pszHeaderUnavailable = "mk-gitinfo-cache::unavailable";
	static const char szPrefixGitBranchFile[] = "git-branch-file:";
	static const size_t cPrefixGitBranchFile = sizeof( szPrefixGitBranchFile ) - 1;
	static const char *const pszTailer = "mk-gitinfo-cache::end";

	MkGitInfo pGitInfo;
	MkBuffer buf;
	FILE *pCacheFile;
	char *pszGitPath;
	char szGitCacheFile[ PATH_MAX ];
	char szLine[ 512 ];

	if( !( pGitInfo = ( MkGitInfo )mk_mem_alloc( sizeof( *pGitInfo ) ) ) ) {
		return ( MkGitInfo )0;
	}

	/* construct the name of the cache file */
	mk_com_strcpy( szGitCacheFile, sizeof( szGitCacheFile ), mk_com_va( "%s/cache.gitinfo", mk_opt_getBuildGenPath() ) );
	mk_dbg_outf( "mk_git_loadInfo: cache-file: %s\n", szGitCacheFile );

	/* load the cache file */
	if( ( buf = mk_buf_loadFile( szGitCacheFile ) ) != ( MkBuffer )0 ) {
		/* using `do/while(0)` for ability to `break` on error */
		do {
			char *pszGitBranchFile;

			/* check the header for the proper version (if not then we need to regenerate) */
			mk_buf_readLine( buf, szLine, sizeof( szLine ) );
			mk_dbg_outf( "\t%s\n", szLine );
			if( strcmp( szLine, pszHeader ) != 0 ) {
				if( strcmp( szLine, pszHeaderUnavailable ) == 0 ) {
					mk_mem_dealloc( ( void * )pGitInfo );
					return ( MkGitInfo )0;
				}
				mk_dbg_outf( "%s: expected \"%s\" on line 1; regenerating\n", szGitCacheFile, pszHeader );
				break;
			}

			/* grab the branch file name */
			mk_buf_readLine( buf, szLine, sizeof( szLine ) );
			mk_dbg_outf( "\t%s\n", szLine );
			if( strncmp( szLine, szPrefixGitBranchFile, cPrefixGitBranchFile ) != 0 ) {
				mk_dbg_outf( "%s: expected \"%s\" on line 2; regenerating\n", szGitCacheFile, szPrefixGitBranchFile );
				break;
			}

			/* pszGitBranchFile guaranteed to be not null from `mk_com_dup` */
			pszGitBranchFile = mk_com_dup( ( char * )0, &szLine[ cPrefixGitBranchFile ] );
			mk_dbg_outf( "\t\t<%s>\n", pszGitBranchFile );

			/* find the commit name */
			if( !( pGitInfo->pszCommit = mk_git_getCommit( pszGitBranchFile ) ) ) {
				pszGitBranchFile = ( char * )mk_mem_dealloc( ( void * )pszGitBranchFile );
				break;
			}

			/* grab the timestamp for the commit */
			if( !( pGitInfo->pszTimestamp = mk_git_getCommitTimestamp( pszGitBranchFile, ( const MkStat_t * )0 ) ) ) {
				pGitInfo->pszCommit = ( char * )mk_mem_dealloc( ( void * )pGitInfo->pszCommit );
				pszGitBranchFile = ( char * )mk_mem_dealloc( ( void * )pszGitBranchFile );
				break;
			}

			/* store the branch file to indicate success */
			pGitInfo->pszBranchFile = pszGitBranchFile;
		} while( 0 );
		mk_buf_delete( buf );

		/* return if we successfully filled the structure (if pszBranchFile is set, assume filled) */
		if( pGitInfo->pszBranchFile != ( char * )0 ) {
			return pGitInfo;
		}
	} else {
		mk_dbg_outf( "\tCould not load cache file\n" );
	}

	if( !( pCacheFile = fopen( szGitCacheFile, "wb" ) ) ) {
		mk_log_error( szGitCacheFile, 0, ( const char * )0, "Failed to open gitinfo cache file for writing" );
	}

	/* using `do/while(0)` for easy `break` (to avoid code duplication) */
	do {
		/* find the git path */
		if( !( pszGitPath = mk_git_findRoot() ) ) {
			break;
		}

		/* find the git branch file */
		if( !( pGitInfo->pszBranchFile = mk_git_findBranchPath( pszGitPath ) ) ) {
			pszGitPath = ( char * )mk_mem_dealloc( ( void * )pszGitPath );
			break;
		}

		/* get the commit for the branch */
		if( !( pGitInfo->pszCommit = mk_git_getCommit( pGitInfo->pszBranchFile ) ) ) {
			pGitInfo->pszBranchFile = ( char * )mk_mem_dealloc( ( void * )pGitInfo->pszBranchFile );
			pszGitPath = ( char * )mk_mem_dealloc( ( void * )pszGitPath );
			break;
		}

		/* get the timestamp for the branch */
		if( !( pGitInfo->pszTimestamp = mk_git_getCommitTimestamp( pGitInfo->pszBranchFile, ( const MkStat_t * )0 ) ) ) {
			pGitInfo->pszCommit = ( char * )mk_mem_dealloc( ( void * )pGitInfo->pszCommit );
			pGitInfo->pszBranchFile = ( char * )mk_mem_dealloc( ( void * )pGitInfo->pszBranchFile );
			pszGitPath = ( char * )mk_mem_dealloc( ( void * )pszGitPath );
			break;
		}

		/* write the results to the debug log */
		mk_dbg_outf( "Caching new gitinfo ***\n" );
		mk_dbg_outf( "\tbranch: %s\n", pGitInfo->pszBranchFile );
		mk_dbg_outf( "\tcommit: %s\n", pGitInfo->pszCommit );
		mk_dbg_outf( "\ttstamp: %s\n", pGitInfo->pszTimestamp );
		mk_dbg_outf( "\n" );

		/* cache the branch file */
		if( pCacheFile != ( FILE * )0 ) {
			fprintf( pCacheFile, "%s\r\n", pszHeader );
			fprintf( pCacheFile, "%s%s\r\n", szPrefixGitBranchFile, pGitInfo->pszBranchFile );
			fprintf( pCacheFile, "%s\r\n", pszTailer );

			pCacheFile = ( fclose( pCacheFile ), ( FILE * )0 );
		}

		/* done */
		return pGitInfo;
	} while( 0 );

	/* if this point is reached, we failed; free the gitinfo */
	mk_mem_dealloc( ( void * )pGitInfo );

	/* write the "unavailable" cache file */
	if( pCacheFile != ( FILE * )0 ) {
		fprintf( pCacheFile, "%s\r\n", pszHeaderUnavailable );
		pCacheFile = ( fclose( pCacheFile ), ( FILE * )0 );
	}

	/* done */
	return ( MkGitInfo )0;
}
/* write out the header containing git info of the given commit and timestamp */
int mk_git_writeHeader( const char *pszHFilename, MkGitInfo pGitInfo )
{
	MkStat_t branchStat, headerStat;
	const char *pszBranchFile;
	const char *pszCommit;
	const char *pszTimestamp;

	const char *pszBranchName;
	FILE *fp;

	MK_ASSERT( pszHFilename != ( const char * )0 && *pszHFilename != '\0' );

	if( pGitInfo != ( MkGitInfo )0 ) {
		pszBranchFile = pGitInfo->pszBranchFile;
		MK_ASSERT( pszBranchFile != ( const char * )0 && *pszBranchFile != '\0' );

		pszCommit = pGitInfo->pszCommit;
		MK_ASSERT( pszCommit != ( const char * )0 && *pszCommit != '\0' );

		pszTimestamp = pGitInfo->pszTimestamp;
		MK_ASSERT( pszTimestamp != ( const char * )0 && *pszTimestamp != '\0' );

		if( !( pszBranchName = strrchr( pszBranchFile, '/' ) ) ) {
			mk_dbg_outf( "mk_git_writeHeader: Invalid branch filename \"%s\"", pszBranchFile );
			return 0;
		}

		/* eat the leading '/' (instead of "/master" we want "master") */
		++pszBranchName;

		/* early exit if we don't need a new file */
		if( stat( pszBranchFile, &branchStat ) == 0 && stat( pszHFilename, &headerStat ) == 0 && branchStat.st_mtime < headerStat.st_mtime ) {
			/* file already made; don't modify */
			return 1;
		}

		if( !( fp = fopen( pszHFilename, "wb" ) ) ) {
			mk_log_error( pszHFilename, 0, ( const char * )0, "Failed to open file for writing" );
			return 0;
		}

		fprintf( fp, "/* This file is generated automatically; edit at your own risk! */\r\n" );
		fprintf( fp, "#ifndef BUILDGEN_GITINFO_H__%s\r\n", pszCommit );
		fprintf( fp, "#define BUILDGEN_GITINFO_H__%s\r\n", pszCommit );
		fprintf( fp, "\r\n" );
		fprintf( fp, "#if defined( _MSC_VER ) || defined( __GNUC__ ) || defined( __clang__ )\r\n" );
		fprintf( fp, "# pragma once\r\n" );
		fprintf( fp, "#endif\r\n" );
		fprintf( fp, "\r\n" );
		fprintf( fp, "#define BUILDGEN_GITINFO_AVAILABLE 1\r\n" );
		fprintf( fp, "\r\n" );
		fprintf( fp, "#define BUILDGEN_GITINFO_BRANCH \"%s\"\r\n", pszBranchName );
		fprintf( fp, "#define BUILDGEN_GITINFO_COMMIT \"%s\"\r\n", pszCommit );
		fprintf( fp, "#define BUILDGEN_GITINFO_TSTAMP \"%s\"\r\n", pszTimestamp );
		fprintf( fp, "\r\n" );
		fprintf( fp, "#endif /* BUILDGEN_GITINFO_H */\r\n" );

		fclose( fp );
		return 1;
	}

	/* if the file exists then just return now, assuming it's already set to "unavailable" */
	if( mk_fs_isFile( pszHFilename ) ) {
		/* FIXME: Would be better to compare against the gitinfo cache file */
		return 1;
	}

	if( !( fp = fopen( pszHFilename, "wb" ) ) ) {
		mk_log_error( pszHFilename, 0, ( const char * )0, "Failed to open file for writing" );
		return 0;
	}

	fprintf( fp, "/* This file is generated automatically; edit at your own risk! */\r\n" );
	fprintf( fp, "#ifndef BUILDGEN_GITINFO_H__\r\n" );
	fprintf( fp, "#define BUILDGEN_GITINFO_H__\r\n" );
	fprintf( fp, "\r\n" );
	fprintf( fp, "#if defined( _MSC_VER ) || defined( __GNUC__ ) || defined( __clang__ )\r\n" );
	fprintf( fp, "# pragma once\r\n" );
	fprintf( fp, "#endif\r\n" );
	fprintf( fp, "\r\n" );
	fprintf( fp, "#define BUILDGEN_GITINFO_AVAILABLE 0\r\n" );
	fprintf( fp, "\r\n" );
	fprintf( fp, "#endif\r\n" );

	fclose( fp );
	return 1;
}

/* generate the gitinfo header if necessary */
int mk_git_generateInfo( void )
{
	MkGitInfo pGitInfo;
	char szHeaderDir[ PATH_MAX ];
	char szHeaderFile[ PATH_MAX ];
	int r;

	mk_com_strcpy( szHeaderDir, sizeof( szHeaderDir ), mk_opt_getBuildGenIncDir() );
	mk_com_strcpy( szHeaderFile, sizeof( szHeaderFile ), mk_com_va( "%s/gitinfo.h", szHeaderDir ) );

	mk_fs_makeDirs( szHeaderDir );

	pGitInfo = mk_git_loadInfo();
	r = mk_git_writeHeader( szHeaderFile, pGitInfo );
	pGitInfo = ( MkGitInfo )mk_mem_dealloc( ( void * )pGitInfo );

	return r;
}

/*
 *	========================================================================
 *	DEPENDENCY TRACKER
 *	========================================================================
 *	Track dependencies and manage the general structure.
 */

struct MkDep_s {
	char *name;
	MkStrList deps;

	struct MkDep_s *prev, *next;
};
MkDep mk__g_dep_head = (MkDep)0;
MkDep mk__g_dep_tail = (MkDep)0;

/* create a new dependency list */
MkDep mk_dep_new(const char *name) {
	MkDep dep;

	MK_ASSERT( name != (const char *)0 );

	dep = (MkDep)mk_com_memory((void *)0, sizeof(*dep));

	dep->name = mk_com_strdup(name);
	dep->deps = mk_sl_new();

	dep->next = (MkDep)0;
	if( (dep->prev = mk__g_dep_tail) != (MkDep)0 ) {
		mk__g_dep_tail->next = dep;
	} else {
		mk__g_dep_head = dep;
	}
	mk__g_dep_tail = dep;

	return dep;
}

/* delete a dependency list */
void mk_dep_delete(MkDep dep) {
	if( !dep ) {
		return;
	}

	dep->name = (char *)mk_com_memory((void *)dep->name, 0);
	mk_sl_delete(dep->deps);
	dep->deps = (MkStrList)0;

	if( dep->prev ) {
		dep->prev->next = dep->next;
	}
	if( dep->next ) {
		dep->next->prev = dep->prev;
	}

	if( mk__g_dep_head==dep ) {
		mk__g_dep_head = dep->next;
	}
	if( mk__g_dep_tail==dep ) {
		mk__g_dep_tail = dep->prev;
	}

	mk_com_memory((void *)dep, 0);
}

/* delete all dependency lists */
void mk_dep_deleteAll(void) {
	while( mk__g_dep_head ) {
		mk_dep_delete(mk__g_dep_head);
	}
}

/* retrieve the name of the file a dependency list is tracking */
const char *mk_dep_getFile(MkDep dep) {
	MK_ASSERT( dep != (MkDep)0 );

	return dep->name;
}

/* add a dependency to a list */
void mk_dep_push(MkDep dep, const char *name) {
	MK_ASSERT( dep != (MkDep)0 );
	MK_ASSERT( name != (const char *)0 );

	mk_sl_pushBack(dep->deps, name);
#if MK_DEBUG_DEPENDENCY_TRACKER_ENABLED
	mk_dbg_outf("~ mk_dep_push \"%s\": \"%s\";\n", dep->name, name);
#endif
}

/* retrieve the number of dependencies in a list */
size_t mk_dep_getSize(MkDep dep) {
	MK_ASSERT( dep != (MkDep)0 );

	return mk_sl_getSize(dep->deps);
}

/* retrieve a dependency from a list */
const char *mk_dep_at(MkDep dep, size_t i) {
	MK_ASSERT( dep != (MkDep)0 );
	MK_ASSERT( i < mk_dep_getSize(dep) );

	return mk_sl_at(dep->deps, i);
}

/* find a dependency */
MkDep mk_dep_find(const char *name) {
	MkDep dep;

	for( dep=mk__g_dep_head; dep; dep=dep->next ) {
		if( !strcmp(dep->name, name) ) {
			return dep;
		}
	}

	return (MkDep)0;
}

/* print all known dependencies */
void mk_dep_debugPrintAll(void) {
#if MK_DEBUG_DEPENDENCY_TRACKER_ENABLED
	MkDep dep;

	for( dep=mk__g_dep_head; dep; dep=dep->next ) {
		mk_dbg_outf( " ** dep: \"%s\"\n", dep->name );
	}
#endif
}

/*
 *	========================================================================
 *	MAKEFILE DEPENDENCY READER
 *	========================================================================
 *	Read dependencies, as produced by GCC/Clang, and put them into an array.
 */

enum {
	kMkMfDepTok_EOF = 0,
	kMkMfDepTok_Colon = ':',

	kMkMfDepTok__EnumBase = 256,
	kMkMfDepTok_Ident
};

/* internal function ported from previous version of 'mk_mfdep_load' */
static char mk_mfdep__read(MkBuffer buf, char *cur, char *look) {
	/*
	 *	! This is based on old code.
	 */

	MK_ASSERT( buf != (MkBuffer)0 );
	MK_ASSERT( cur != (char *)0 );
	MK_ASSERT( look != (char *)0 );

	*cur = mk_buf_read(buf);
	*look = mk_buf_peek(buf);

	return *cur;
}

/* read a token from the buffer */
static int mk_mfdep__lex(MkBuffer buf, char *dst, size_t dstn, char *cur, char *look) {
	size_t i;

	MK_ASSERT( buf != (MkBuffer)0 );
	MK_ASSERT( dst != (char *)0 );
	MK_ASSERT( dstn > 1 );

	i = 0;
	dst[0] = 0;

	if( !*cur ) {
		do {
			if( !mk_mfdep__read(buf, cur, look) ) {
				return kMkMfDepTok_EOF;
			}
		} while( *cur <= ' ' );
	}

	if( *cur==':' ) {
		dst[0] = ':';
		dst[1] = 0;

		*cur = 0; /* force 'cur' to be retrieved again, else infinite loop */

		return kMkMfDepTok_Colon;
	}

	while( 1 ) {
		if( *cur=='\\' ) {
			/*
			 *	! Original code expected that '\r' was possible, hence this
			 *	  setup was needed to make this portion easier. Needs to be
			 *	  fixed later.
			 */
			if( *look=='\n' ) {
				do {
					if( !mk_mfdep__read(buf, cur, look) ) {
						break;
					}
				} while( *cur <= ' ' );

				continue;
			}

			if( *look==' ' ) {
				dst[i++] = mk_mfdep__read(buf, cur, look);
				if( i==dstn ) {
					mk_buf_errorf(buf, "overflow detected");
					exit(EXIT_FAILURE);
				}
				continue;
			}

			*cur = '/'; /* correct path reads */
		} else if(*cur==':') {
			if( *look<=' ' ) {
				dst[i] = 0;
				return kMkMfDepTok_Ident; /* leave 'cur' for the next call */
			}
		} else if(*cur<=' ') {
			dst[i] = 0;
			*cur = 0; /* force read on next call */

			return kMkMfDepTok_Ident;
		}

		dst[i++] = *cur;
		if( i==dstn ) {
			mk_buf_errorf(buf, "overflow detected");
			exit(EXIT_FAILURE);
		}

		mk_mfdep__read(buf, cur, look);
	}
}

/* read dependencies from a file */
int mk_mfdep_load(const char *filename) {
	MkBuffer buf;
	MkDep dep;
	char lexan[PATH_MAX], ident[PATH_MAX], cur, look;
	int tok;

	ident[0] = '\0';
	lexan[0] = '\0';
	cur = 0;
	look = 0;

	buf = mk_buf_loadFile(filename);
	if( !buf ) {
		return 0;
	}

	mk_mfdep__read(buf, &cur, &look);

	dep = (MkDep)0;

	do {
		tok = mk_mfdep__lex(buf, lexan, sizeof(lexan), &cur, &look);
#if 0
		mk_dbg_outf( "%s(%i): %i: \"%s\"\n", mk_buf_getFilename(buf),
			(int)mk_buf_calculateLine(buf), tok, lexan );
#endif

		if( tok==kMkMfDepTok_Colon ) {
			if( ident[0] ) {
				dep = mk_dep_new(ident);
				ident[0] = 0;
			}

			continue;
		}

		if( dep && ident[0] ) {
			mk_dep_push(dep, ident);
			ident[0] = 0;
		}

		if( tok==kMkMfDepTok_Ident ) {
			mk_com_strcpy(ident, sizeof(ident), lexan);
		}
	} while( tok != kMkMfDepTok_EOF );

	mk_buf_delete(buf);
	return 1;
}

/*
 *	========================================================================
 *	LIBRARIAN
 *	========================================================================
 *	Manages individual libraries. Each library is abstracted with a platform
 *	independent name (e.g., "opengl") which is then mapped to platform
 *	dependent flags. (e.g., on Windows the above would be "-lopengl32," but on
 *	GNU/Linux it would be "-lGL.")
 */

enum {
	kMkLib_Processed_Bit = 0x01 /* indicates a library has been "processed" */
};

struct MkLib_s {
	size_t name_n;
	char *name;
	char *flags[kMkNumOS];

	MkProject proj;

	bitfield_t config;

	struct MkLib_s *prev, *next;
};

struct MkLib_s *mk__g_lib_head = (struct MkLib_s *)0;
struct MkLib_s *mk__g_lib_tail = (struct MkLib_s *)0;

/* create a new library */
MkLib mk_lib_new(void) {
	size_t i;
	MkLib lib;

	lib = (MkLib)mk_com_memory((void *)0, sizeof(*lib));

	lib->name = (char *)0;
	for( i=0; i<sizeof(lib->flags)/sizeof(lib->flags[0]); i++ ) {
		lib->flags[i] = (char *)0;
	}

	lib->proj = (MkProject)0;

	lib->config = 0;

	lib->next = (struct MkLib_s *)0;
	if( (lib->prev = mk__g_lib_tail) != (struct MkLib_s *)0 ) {
		mk__g_lib_tail->next = lib;
	} else {
		mk__g_lib_head = lib;
	}
	mk__g_lib_tail = lib;

	return lib;
}

/* delete an existing library */
void mk_lib_delete(MkLib lib) {
	size_t i;

	if( !lib ) {
		return;
	}

	lib->name_n = 0;
	lib->name = (char *)mk_com_memory((void *)lib->name, 0);
	for( i=0; i<sizeof(lib->flags)/sizeof(lib->flags[0]); i++ ) {
		lib->flags[i] = (char *)mk_com_memory((void *)lib->flags[i], 0);
	}

	if( lib->prev ) {
		lib->prev->next = lib->next;
	}
	if( lib->next ) {
		lib->next->prev = lib->prev;
	}

	if( mk__g_lib_head==lib ) {
		mk__g_lib_head = lib->next;
	}
	if( mk__g_lib_tail==lib ) {
		mk__g_lib_tail = lib->prev;
	}

	mk_com_memory((void *)lib, 0);
}

/* delete all existing libraries */
void mk_lib_deleteAll(void) {
	while( mk__g_lib_head )
		mk_lib_delete(mk__g_lib_head);
}

/* set the name of a library */
void mk_lib_setName(MkLib lib, const char *name) {
	MK_ASSERT( lib != (MkLib)0 );

	lib->name_n = name!=(const char *)0 ? strlen(name) : 0;
	lib->name = mk_com_dup(lib->name, name);
}

/* set the flags of a library */
void mk_lib_setFlags(MkLib lib, int sys, const char *flags) {
	MK_ASSERT( lib != (MkLib)0 );
	MK_ASSERT( sys>=0 && sys<kMkNumOS );

	lib->flags[sys] = mk_com_dup(lib->flags[sys], flags);
}

/* retrieve the name of a library */
const char *mk_lib_getName(MkLib lib) {
	MK_ASSERT( lib != (MkLib)0 );

	return lib->name;
}

/* retrieve the flags of a library */
const char *mk_lib_getFlags(MkLib lib, int sys) {
	MK_ASSERT( lib != (MkLib)0 );
	MK_ASSERT( sys>=0 && sys<kMkNumOS );

	return lib->flags[sys];
}

/* retrieve the library before another */
MkLib mk_lib_prev(MkLib lib) {
	MK_ASSERT( lib != (MkLib)0 );

	return lib->prev;
}

/* retrieve the library after another */
MkLib mk_lib_next(MkLib lib) {
	MK_ASSERT( lib != (MkLib)0 );

	return lib->next;
}

/* retrieve the first library */
MkLib mk_lib_head(void) {
	return mk__g_lib_head;
}

/* retrieve the last library */
MkLib mk_lib_tail(void) {
	return mk__g_lib_tail;
}

/* find a library by its name */
MkLib mk_lib_find(const char *name) {
	size_t name_n;
	MkLib lib;

	MK_ASSERT( name != (const char *)0 );

	name_n = strlen( name );

	for( lib=mk__g_lib_head; lib; lib=lib->next ) {
		if( !lib->name || name_n != lib->name_n ) {
			continue;
		}

		if( strcmp(lib->name, name)==0 ) {
			return lib;
		}
	}

	return (MkLib)0;
}

/* find or create a library by name */
MkLib mk_lib_lookup(const char *name)
{
	MkLib lib;

	MK_ASSERT( name != ( const char * )0 );

	if( ( lib = mk_lib_find( name ) ) != ( MkLib )0 ) {
		return lib;
	}

	lib = mk_lib_new();
	mk_lib_setName( lib, name );

	return lib;
}

/* mark all libraries as "not processed" */
void mk_lib_clearAllProcessed(void) {
	MkLib lib;

	for( lib=mk__g_lib_head; lib; lib=lib->next ) {
		lib->config &= ~kMkLib_Processed_Bit;
	}
}

/* mark a library as "not processed" */
void mk_lib_clearProcessed(MkLib lib) {
	MK_ASSERT( lib != (MkLib)0 );

	lib->config &= ~kMkLib_Processed_Bit;
}

/* mark a library as "processed" */
void mk_lib_setProcessed(MkLib lib) {
	MK_ASSERT( lib != (MkLib)0 );

	lib->config |= kMkLib_Processed_Bit;
}

/* determine whether a library was marked as "processed" */
int mk_lib_isProcessed(MkLib lib) {
	MK_ASSERT( lib != (MkLib)0 );

	return lib->config & kMkLib_Processed_Bit ? 1 : 0;
}

/*
 *	========================================================================
 *	AUTOLINK SYSTEM
 *	========================================================================
 *	This system maps which header files are used to access which libraries. This
 *	allows the dependency system to be exploited to reveal what flags need to be
 *	passed to the linker for the system to "just work."
 */

struct MkAutolink_s {
	char *header[kMkNumOS];
	char *lib;

	struct MkAutolink_s *prev, *next;
};
struct MkAutolink_s *mk__g_al_head = (struct MkAutolink_s *)0;
struct MkAutolink_s *mk__g_al_tail = (struct MkAutolink_s *)0;

/* allocate a new auto-link entry */
MkAutolink mk_al_new(void) {
	MkAutolink al;
	size_t i;

	al = (MkAutolink)mk_com_memory((void *)0, sizeof(*al));

	for( i=0; i<kMkNumOS; i++ ) {
		al->header[i] = (char *)0;
	}
	al->lib = (char *)0;

	al->next = (struct MkAutolink_s *)0;
	if( (al->prev = mk__g_al_tail) != (struct MkAutolink_s *)0 ) {
		mk__g_al_tail->next = al;
	} else {
		mk__g_al_head = al;
	}
	mk__g_al_tail = al;

	return al;
}

/* deallocate an existing auto-link entry */
void mk_al_delete(MkAutolink al) {
	size_t i;

	if( !al ) {
		return;
	}

	for( i=0; i<kMkNumOS; i++ ) {
		al->header[i] = (char *)mk_com_memory((void *)al->header[i], 0);
	}
	al->lib = (char *)mk_com_memory((void *)al->lib, 0);

	if( al->prev ) {
		al->prev->next = al->next;
	}
	if( al->next ) {
		al->next->prev = al->prev;
	}

	if( mk__g_al_head==al ) {
		mk__g_al_head = al->next;
	}
	if( mk__g_al_tail==al ) {
		mk__g_al_tail = al->prev;
	}

	mk_com_memory((void *)al, 0);
}

/* deallocate all existing auto-link entries */
void mk_al_deleteAll(void) {
	while( mk__g_al_head ) {
		mk_al_delete(mk__g_al_head);
	}
}

/* set a header for an auto-link entry (determines whether to auto-link) */
void mk_al_setHeader(MkAutolink al, int sys, const char *header) {
	static char rel[PATH_MAX];
	const char *p;

	MK_ASSERT( al != (MkAutolink)0 );
	MK_ASSERT( sys>=0 && sys<kMkNumOS );

	if( header != (const char *)0 ) {
		mk_com_relPathCWD(rel, sizeof(rel), header);
		p = rel;
	} else {
		p = (const char *)0;
	}

	al->header[sys] = mk_com_dup(al->header[sys], p);
}

/* set the library an auto-link entry refers to */
void mk_al_setLib(MkAutolink al, const char *libname) {
	MK_ASSERT( al != (MkAutolink)0 );

	al->lib = mk_com_dup(al->lib, libname);
}

/* retrieve a header of an auto-link entry */
const char *mk_al_getHeader(MkAutolink al, int sys) {
	MK_ASSERT( al != (MkAutolink)0 );
	MK_ASSERT( sys>=0 && sys<kMkNumOS );

	return al->header[sys];
}

/* retrieve the library an auto-link entry refers to */
const char *mk_al_getLib(MkAutolink al) {
	MK_ASSERT( al != (MkAutolink)0 );

	return al->lib;
}

/* find an auto-link entry by header and system */
MkAutolink mk_al_find(int sys, const char *header) {
	MkAutolink al;

	MK_ASSERT( sys>=0 && sys<kMkNumOS );
	MK_ASSERT( header != (const char *)0 );

	for( al=mk__g_al_head; al; al=al->next ) {
		if( !al->header[sys] ) {
			continue;
		}

		if( mk_com_matchPath(al->header[sys], header) ) {
			return al;
		}
	}

	return (MkAutolink)0;
}

/* find or create an auto-link entry by header and system */
MkAutolink mk_al_lookup(int sys, const char *header) {
	MkAutolink al;

	MK_ASSERT( sys>=0 && sys<kMkNumOS );
	MK_ASSERT( header != ( const char * )0 );

	if( !( al = mk_al_find( sys, header ) ) ) {
		al = mk_al_new();
		mk_al_setHeader( al, sys, header );
	}

	return al;
}

/* retrieve the flags needed for linking to a library based on its header */
const char *mk_al_autolink(int sys, const char *header) {
	MkAutolink al;
	MkLib lib;

	MK_ASSERT( sys>=0 && sys<kMkNumOS );
	MK_ASSERT( header != (const char *)0 );

	if( !(al = mk_al_find(sys, header)) ) {
		return (const char *)0;
	}

	if( !(lib = mk_lib_find(mk_al_getLib(al))) ) {
		return (const char *)0;
	}

	return mk_lib_getFlags(lib, sys);
}

/* recursively add autolinks for a lib from an include directory */
void mk_al_managePackage_r(const char *libname, int sys, const char *incdir) {
	struct dirent *dp;
	MkAutolink al;
	DIR *d;

	mk_dbg_outf("*** PKGAUTOLINK dir:\"%s\" lib:\"%s\" ***\n",
		incdir, libname);

	mk_fs_enter(incdir);
	d = mk_fs_openDir("./");
	if( !d ) {
		mk_fs_leave();
		mk_dbg_outf("    mk_al_managePackage_r failed to enter directory\n");
		return;
	}
	while( (dp = readdir(d)) != (struct dirent *)0 ) {
		if( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ) {
			continue;
		}

		if( mk_fs_isDir(dp->d_name) ) {
			mk_al_managePackage_r(libname, sys, dp->d_name);
			continue;
		}

		{
			char realpath[PATH_MAX];

			if( !mk_fs_realPath(dp->d_name, realpath, sizeof(realpath)) ) {
				continue;
			}

			al = mk_al_new();
			mk_al_setLib(al, libname);
			mk_al_setHeader(al, sys, realpath);

			mk_dbg_outf("    autolinking path: \"%s\"\n", realpath);
		}
	}
	mk_fs_closeDir(d);
	mk_fs_leave();
}

/*
===============================================================================

	AUTOLINK CONFIG FILE FORMAT
	===========================

	This allows you to configure a set of libraries/autolink headers. It's
	pretty simple:

		libname "linkflags" plat:"linkflags" plat:"linkflags" {
			"header/file/name.h"
			plat:"header/file/name.h"
			plat:"header/file/name.h"
		}

	e.g., for OpenGL:

		opengl "-lGL" mswin:"-lopengl32" apple:"-framework OpenGL" {
			"GL/gl.h"
			apple:"OpenGL/OpenGL.h"
		}

	New entries override old entries.

===============================================================================
*/

/* skip past whitespace, including comments */
static void mk_al__lexWhite( MkBuffer buf ) {
	int r;
	do {
		r = 0;

		mk_buf_skipWhite( buf );

		r |= mk_buf_skipLineIfStartsWith( buf, "#" );
		r |= mk_buf_skipLineIfStartsWith( buf, "//" );
	} while( r );
}
/* check whether a given character is part of a "name" token */
static int mk_al__isLexName( char c ) {
	if( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ) {
		return 1;
	}

	if( c >= '0' && c <= '9' ) {
		return 1;
	}

	if( c == '-' || c == '_' || c == '.' ) {
		return 1;
	}

	return 0;
}
/* check whether a given character is considered a valid delimiter (can break a name token) */
static int mk_al__isLexDelim( char c ) {
	if( ( unsigned char )( c <= ' ' ) ) {
		return 1;
	}

	return c == '{' || c == '}' || c == ':' || c == ',';
}
/* lex in a name token; returns 1 on success or if not a name token; 0 on error */
static int mk_al__lexName( char *pszDst, size_t cDstMax, MkBuffer buf ) {
	char *p, *e;

	MK_ASSERT( pszDst != ( char * )0 );
	MK_ASSERT( cDstMax > 1 );

	p = pszDst;
	e = pszDst + cDstMax - 1;

	for(;;) {
		char c;

		c = mk_buf_peek( buf );

		if( mk_al__isLexName( c ) ) {
			if( p == e ) {
				mk_buf_errorf( buf, "Length of libary name is too long; must be under %u bytes", ( unsigned )cDstMax );
				pszDst[ 0 ] = '\0';
				return 0;
			}

			*p++ = c;
			( void )mk_buf_read( buf );
			continue;
		} else if( p == pszDst ) {
			break;
		}

		if( mk_al__isLexDelim( c ) ) {
			*p = '\0';
			return 1;
		}

		mk_buf_errorf( buf, "Unexpected character in library name: 0x%.2X (%c)", ( unsigned )c, c );
		break;
	}

	pszDst[ 0 ] = '\0';
	return 0;
}
/* lex in a quote (string); returns 0 on error or if not a string (CONSISTENCY) */
static int mk_al__lexQuote( char *pszDst, size_t cDstMax, MkBuffer buf ) {
	char *p, *e;

	MK_ASSERT( pszDst != ( char * )0 );
	MK_ASSERT( cDstMax > 3 );

	p = pszDst;
	e = pszDst + cDstMax - 1;

	if( !mk_buf_advanceIfCharEq( buf, '\"' ) ) {
		return 0;
	}

	for(;;) {
		char c;

		if( buf->ptr == buf->endPtr ) {
			mk_buf_errorf( buf, "Unexpected end of file while reading string" );
			pszDst[ 0 ] = '\0';
			return 0;
		}

		if( ( c = mk_buf_read( buf ) ) == '\"' ) {
			break;
		}

		if( c == '\\' ) {
			c = mk_buf_read( buf );

			switch( c ) {
			case 'a': c = '\a'; break;
			case 'b': c = '\b'; break;
			case 'f': c = '\f'; break;
			case 'n': c = '\n'; break;
			case 'r': c = '\r'; break;
			case 't': c = '\t'; break;
			case 'v': c = '\v'; break;
			case '\'': c = '\''; break;
			case '\"': c = '\"'; break;
			case '\?': c = '\?'; break;
			default:
				mk_buf_errorf( buf, "Unknown escape sequence char 0x%.2X ('%c'); ignoring", ( unsigned )c, c );
				break;
			}
		}

		if( p == e ) {
			mk_buf_errorf( buf, "String is too long; keep under %u bytes", ( unsigned )cDstMax );
			*p = '\0';
			return 0;
		}

		*p++ = c;
	}

	*p = '\0';
	return 1;
}

typedef enum {
	kMkALTok_Error = -1,
	kMkALTok_EOF = 0,

	kMkALTok_LBrace = '{',
	kMkALTok_RBrace = '}',
	kMkALTok_Colon = ':',
	kMkALTok_Comma = ',',

	kMkALTok_Name = 128,
	kMkALTok_Quote,

	kMkALTok_Null
} MkALToken_t;

/* lex a token from the buffer */
MkALToken_t mk_al__lex( char *pszDst, size_t cDstMax, MkBuffer buf ) {
	char *bufPos;

	MK_ASSERT( pszDst != ( char * )0 );
	MK_ASSERT( cDstMax > 7 ); /* "(error)" == 7 bytes */
	MK_ASSERT( buf != ( MkBuffer )0 );

	mk_al__lexWhite( buf );

	if( ( bufPos = buf->ptr ) == buf->endPtr ) {
		*pszDst = '\0';
		return kMkALTok_EOF;
	}

	if( mk_al__lexName( pszDst, cDstMax, buf ) && pszDst[ 0 ] != '\0' ) {
		if( *pszDst == 'n' && strcmp( pszDst, "null" ) == 0 ) {
			return kMkALTok_Null;
		}

		return kMkALTok_Name;
	}

	if( mk_buf_peek( buf ) == '\"' ) {
		if( !mk_al__lexQuote( pszDst, cDstMax, buf ) ) {
			buf->ptr = bufPos;
			mk_buf_errorf( buf, "Incomplete quote starts here" );
			buf->ptr = buf->endPtr;
			return kMkALTok_Error;
		}

		return kMkALTok_Quote;
	}

	if( mk_buf_advanceIfCharEq( buf, '{' ) ) {
		mk_com_strcpy( pszDst, cDstMax, "{" );
		return kMkALTok_LBrace;
	}

	if( mk_buf_advanceIfCharEq( buf, '}' ) ) {
		mk_com_strcpy( pszDst, cDstMax, "}" );
		return kMkALTok_RBrace;
	}

	if( mk_buf_advanceIfCharEq( buf, ':' ) ) {
		mk_com_strcpy( pszDst, cDstMax, ":" );
		return kMkALTok_Colon;
	}

	if( mk_buf_advanceIfCharEq( buf, ',' ) ) {
		mk_com_strcpy( pszDst, cDstMax, "," );
		return kMkALTok_Comma;
	}

	mk_com_strcpy( pszDst, cDstMax, "(error)" );

	mk_buf_errorf( buf, "Unrecognized character 0x%.2X ('%c')", ( unsigned )mk_buf_peek( buf ), mk_buf_peek( buf ) );
	buf->ptr = buf->endPtr;

	return kMkALTok_Error;
}

/* retrieve the name of a token */
const char *mk_al__getTokenName( MkALToken_t tok ) {
	switch( tok ) {
	case kMkALTok_Error:  return "(error)";
	case kMkALTok_EOF:    return "EOF";

	case kMkALTok_LBrace: return "'{'";
	case kMkALTok_RBrace: return "'}'";
	case kMkALTok_Colon:  return "':'";
	case kMkALTok_Comma:  return "','";

	case kMkALTok_Name:   return "name";
	case kMkALTok_Quote:  return "quote";

	case kMkALTok_Null:   return "'null'";
	}

	return "(invalid)";
}

/* expect any of the given token types (returns index of the token in the array, or -1 on error/unmatched) */
int mk_al__expectAny( char *pszDst, size_t cDstMax, MkBuffer buf, const MkALToken_t *pToks, size_t cToks ) {
	char szErrorBuf[ 256 ];
	MkALToken_t tok;
	size_t i;

	if( ( tok = mk_al__lex( pszDst, cDstMax, buf ) ) == kMkALTok_Error ) {
		return -1;
	}

	for( i = 0; i < cToks; ++i ) {
		if( pToks[ i ] == tok ) {
			return ( int )( unsigned )i;
		}
	}

	mk_com_strcpy( szErrorBuf, sizeof( szErrorBuf ), "Expected " );
	for( i = 0; i < cToks; ++i ) {
		mk_com_strcat( szErrorBuf, sizeof( szErrorBuf ), mk_al__getTokenName( pToks[ i ] ) );
		if( i + 1 < cToks ) {
			mk_com_strcat( szErrorBuf, sizeof( szErrorBuf ), ( i + 2 == cToks ) ? ", or " : ", " );
		}
	}
	mk_com_strcat( szErrorBuf, sizeof( szErrorBuf ), ", but got " );
	mk_com_strcat( szErrorBuf, sizeof( szErrorBuf ), mk_al__getTokenName( tok ) );

	mk_buf_errorf( buf, "%s", szErrorBuf );
	return -1;
}
/* expect any of the given three token types (returns -1 on failure, or 0, 1, or 2 on success) */
int mk_al__expect3( char *pszDst, size_t cDstMax, MkBuffer buf, MkALToken_t tok1, MkALToken_t tok2, MkALToken_t tok3 ) {
	MkALToken_t toks[ 3 ];

	toks[ 0 ] = tok1;
	toks[ 1 ] = tok2;
	toks[ 2 ] = tok3;

	return mk_al__expectAny( pszDst, cDstMax, buf, toks, sizeof( toks )/sizeof( toks[0] ) );
}
/* expect any of the given two token types (returns -1 on failure, or 0 or 1 on success) */
int mk_al__expect2( char *pszDst, size_t cDstMax, MkBuffer buf, MkALToken_t tok1, MkALToken_t tok2 ) {
	MkALToken_t toks[ 2 ];

	toks[ 0 ] = tok1;
	toks[ 1 ] = tok2;

	return mk_al__expectAny( pszDst, cDstMax, buf, toks, sizeof( toks )/sizeof( toks[0] ) );
}
/* expect the given token type (returns 0 on failure, 1 on success) */
int mk_al__expect( char *pszDst, size_t cDstMax, MkBuffer buf, MkALToken_t tok ) {
	return mk_al__expectAny( pszDst, cDstMax, buf, &tok, 1 ) + 1;
}

/* retrieve the platform (kMkOS_*) from the string, or fail with -1 */
int mk_al__stringToOS( const char *pszPlat ) {
	if( strcmp( pszPlat, MK_PLATFORM_OS_NAME_MSWIN ) == 0 ) {
		return kMkOS_MSWin;
	}

	if( strcmp( pszPlat, MK_PLATFORM_OS_NAME_UWP ) == 0 ) {
		return kMkOS_UWP;
	}

	if( strcmp( pszPlat, MK_PLATFORM_OS_NAME_CYGWIN ) == 0 ) {
		return kMkOS_Cygwin;
	}

	if( strcmp( pszPlat, MK_PLATFORM_OS_NAME_LINUX ) == 0 ) {
		return kMkOS_Linux;
	}

	if( strcmp( pszPlat, MK_PLATFORM_OS_NAME_MACOSX ) == 0 ) {
		return kMkOS_MacOSX;
	}

	if( strcmp( pszPlat, MK_PLATFORM_OS_NAME_UNIX ) == 0 ) {
		return kMkOS_Unix;
	}

	return -1;
}

/* parse a series of comma-delimited platform names until a ':' is reached; expects pszDst to already have a name */
int mk_al__readOSNames( int *pDstPlats, size_t cDstPlatsMax, size_t *pcDstPlats, char *pszDst, size_t cDstMax, MkBuffer buf ) {
	size_t cPlats = 0;
	int r;

	MK_ASSERT( pDstPlats != ( int * )0 );
	MK_ASSERT( cDstPlatsMax >= kMkNumOS );
	MK_ASSERT( pcDstPlats != ( size_t * )0 );
	MK_ASSERT( pszDst != ( char * )0 );
	MK_ASSERT( cDstMax > 0 );
	MK_ASSERT( buf != ( MkBuffer )0 );

	*pcDstPlats = 0;

	for(;;) {
#if MK_DEBUG_AUTOLIBCONF_ENABLED
		mk_dbg_outf( "Got platform: \"%s\"\n", pszDst );
#endif
		if( ( pDstPlats[ cPlats ] = mk_al__stringToOS( pszDst ) ) == -1 ) {
			mk_buf_errorf( buf, "Unknown platform \"%s\"", pszDst );
			return 0;
		}

		MK_ASSERT( pDstPlats[ cPlats ] >= 0 && pDstPlats[ cPlats ] < kMkNumOS );

		++cPlats;

		if( ( r = mk_al__expect2( pszDst, cDstMax, buf, kMkALTok_Comma, kMkALTok_Colon ) ) == -1 ) {
			return 0;
		}

		if( r == 0 /* comma */ ) {
			if( cPlats == cDstPlatsMax ) {
				mk_buf_errorf( buf, "Too many platforms specified; keep under %u", ( unsigned )cDstPlatsMax );
				return 0;
			}

			continue;
		}

		MK_ASSERT( r == 1 /* colon */ );
		break;
	}

	*pcDstPlats = cPlats;
	return 1;
}

/* parse the library's flags and the left-brace; returns 0 on fail, or 1 on success */
int mk_al__readLibFlagsAndLBrace( char *pszDst, size_t cDstMax, MkBuffer buf, MkLib lib ) {
	size_t cPlats, i;
	int plat[ kMkNumOS ];
	int r;

	MK_ASSERT( pszDst != ( char * )0 );
	MK_ASSERT( cDstMax > 0 );
	MK_ASSERT( buf != ( MkBuffer )0 );
	MK_ASSERT( lib != ( MkLib )0 );

#if MK_DEBUG_AUTOLIBCONF_ENABLED
	mk_dbg_outf( "mk_al__readLibFlagsAndLBrace...\n" );
#endif

	/* keep going until we get an error or a left-brace */
	for(;;) {
		if( ( r = mk_al__expect3( pszDst, cDstMax, buf, kMkALTok_Name, kMkALTok_Quote, kMkALTok_LBrace ) ) == -1 ) {
			break;
		}

		cPlats = 0;
		switch( r ) {
		case 0: /* name */
			/* read the sequence of platform names (e.g., mswin,uwp,cygwin: etc...) */
			if( !mk_al__readOSNames( plat, sizeof( plat )/sizeof( plat[ 0 ] ), &cPlats, pszDst, cDstMax, buf ) ) {
				return 0;
			}

			/* immediately following the platform names must be a quote or "null" */
			if( ( r = mk_al__expect2( pszDst, cDstMax, buf, kMkALTok_Quote, kMkALTok_Null ) ) == -1 ) {
				return 0;
			}

			/* if we got null, then we can't share the code */
			if( r == 1 /* null */ ) {
#if MK_DEBUG_AUTOLIBCONF_ENABLED
				mk_dbg_outf( "Setting platform linker flags to null.\n" );
#endif
				for( i = 0; i < cPlats; ++i ) {
					mk_lib_setFlags( lib, plat[ i ], ( const char * )0 );
				}

				break;
			}

			/* fallthrough */
		case 1: /* quote */
			/* if we got a string and no platforms were specified then assume all */
			if( !cPlats ) {
#if MK_DEBUG_AUTOLIBCONF_ENABLED
				mk_dbg_outf( "No platforms given; assuming all for this flag.\n" );
#endif

				while( cPlats < ( size_t )kMkNumOS ) {
					plat[ cPlats ] = ( int )( unsigned )cPlats;
					++cPlats;
				}
			}

#if MK_DEBUG_AUTOLIBCONF_ENABLED
			mk_dbg_outf( "Setting platforms to linker flag: \"%s\"\n", pszDst );
#endif

			/* set each platform's library flags */
			for( i = 0; i < cPlats; ++i ) {
				mk_lib_setFlags( lib, plat[ i ], pszDst );
			}

			break;

		case 2: /* left-brace */
#if MK_DEBUG_AUTOLIBCONF_ENABLED
			mk_dbg_outf( "Got left-brace.\n\n" );
#endif
			return 1;
		}
	}

	/* we got an error */
	return 0;
}
/* parse the library's headers and the right-brace; returns 0 on fail, or 1 on success */
int mk_al__readLibHeadersAndRBrace( char *pszDst, size_t cDstMax, MkBuffer buf, MkLib lib ) {
	size_t cPlats, i;
	int plat[ kMkNumOS ];
	int r;

	MK_ASSERT( pszDst != ( char * )0 );
	MK_ASSERT( cDstMax > 0 );
	MK_ASSERT( buf != ( MkBuffer )0 );
	MK_ASSERT( lib != ( MkLib )0 );

#if MK_DEBUG_AUTOLIBCONF_ENABLED
	mk_dbg_outf( "mk_al__readLibHeadersAndRBrace...\n" );
#endif

	/* keep going until we get an error or a right-brace */
	for(;;) {
		if( ( r = mk_al__expect3( pszDst, cDstMax, buf, kMkALTok_Name, kMkALTok_Quote, kMkALTok_RBrace ) ) == -1 ) {
			break;
		}

		cPlats = 0;
		switch( r ) {
		case 0: /* name */
			/* read the sequence of platform names (e.g., mswin,uwp,cygwin: etc...) */
			if( !mk_al__readOSNames( plat, sizeof( plat )/sizeof( plat[ 0 ] ), &cPlats, pszDst, cDstMax, buf ) ) {
				return 0;
			}

			/* immediately following the platform names must be a quote */
			if( !mk_al__expect( pszDst, cDstMax, buf, kMkALTok_Quote ) ) {
				return 0;
			}

			/* fallthrough */
		case 1: /* quote */
			/* cannot have an empty string for autolinks */
			if( *pszDst == '\0' ) {
				mk_buf_errorf( buf, "Empty strings cannot be used for autolinks" );
				return 0;
			}

			/* if we got a string and no platforms were specified then assume all */
			if( !cPlats ) {
#if MK_DEBUG_AUTOLIBCONF_ENABLED
				mk_dbg_outf( "No platforms given; assuming all for this header.\n" );
#endif
				while( cPlats < ( size_t )kMkNumOS ) {
					plat[ cPlats ] = ( int )( unsigned )cPlats;
					++cPlats;
				}
			}

#if MK_DEBUG_AUTOLIBCONF_ENABLED
			mk_dbg_outf( "Adding autolink entries to %s for header: \"%s\"\n",
				cPlats == 1 ? "this platform" : "these platforms",
				pszDst );
#endif

			/* create the autolinks (pszDst has the header name) */
			for( i = 0; i < cPlats; ++i ) {
				MkAutolink al;

				al = mk_al_lookup( plat[ i ], pszDst );
				MK_ASSERT( al != ( MkAutolink )0 && "mk_al_lookup() should never return NULL" );

				/* FIXME: This could be a bit higher performance */
				mk_al_setLib( al, mk_lib_getName( lib ) );
			}

			break;

		case 2: /* right-brace */
#if MK_DEBUG_AUTOLIBCONF_ENABLED
			mk_dbg_outf( "Got right-brace.\n" );
#endif
			return 1;
		}
	}

	/* we got an error */
	return 0;
}

/* parse a top-level library, returning 0 on failure or 1 on success */
int mk_al__readLib( char *pszDst, size_t cDstMax, MkBuffer buf ) {
	MkLib lib;
	int r;

	r = mk_al__expect2( pszDst, cDstMax, buf, kMkALTok_Name, kMkALTok_EOF );
	if( r != 0 /* not kMkALTok_Name */ ) {
		return r == 1; /* EOF is success */
	}

#if MK_DEBUG_AUTOLIBCONF_ENABLED
	mk_dbg_enter( "mk_al__readLib(\"%s\")", pszDst );
#endif

	/* find or create the library; if found we overwrite some stuff */
	lib = mk_lib_lookup( pszDst );
	MK_ASSERT( lib != ( MkLib )0 );

	/* read the lib flags and the left-brace */
	if( !mk_al__readLibFlagsAndLBrace( pszDst, cDstMax, buf, lib ) ) {
#if MK_DEBUG_AUTOLIBCONF_ENABLED
		mk_dbg_leave();
#endif
		return 0;
	}

	/* read headers */
	if( !mk_al__readLibHeadersAndRBrace( pszDst, cDstMax, buf, lib ) ) {
#if MK_DEBUG_AUTOLIBCONF_ENABLED
		mk_dbg_leave();
#endif
		return 0;
	}

#if MK_DEBUG_AUTOLIBCONF_ENABLED
	mk_dbg_leave();
#endif

	/* done */
	return 1;
}

/* parse a file for autolinks and libraries */
int mk_al_loadConfig( const char *filename ) {
	MkBuffer buf;
	char szLexan[ 512 ];
	int r;

	MK_ASSERT( filename != ( const char * )0 );

	if( !( buf = mk_buf_loadFile( filename ) ) ) {
		mk_dbg_outf( "%s: Failed to load autolink config\n", filename );
		return 0;
	}

	mk_dbg_enter( "mk_al_loadConfig(\"%s\")", filename );

	r = 1;
	do {
		if( !mk_al__readLib( szLexan, sizeof( szLexan ), buf ) ) {
			r = 0;
			break;
		}
	} while( buf->ptr != buf->endPtr );

	mk_dbg_leave();

	mk_buf_delete( buf );
	return r;
}

/*
 *	========================================================================
 *	PROJECT MANAGEMENT
 *	========================================================================
 */

#if !MK_DEBUG_ENABLED
# undef  MK_DEBUG_LIBDEPS_ENABLED
# define MK_DEBUG_LIBDEPS_ENABLED 0
#endif

const int mk__g_hostOS =
#if MK_HOST_OS_MSWIN
	kMkOS_MSWin;
#elif MK_HOST_OS_CYGWIN
	kMkOS_Cygwin;
#elif MK_HOST_OS_LINUX
	kMkOS_Linux;
#elif MK_HOST_OS_MACOSX
	kMkOS_MacOSX;
#elif MK_HOST_OS_UNIX
	kMkOS_Unix;
#else
# error "OS not recognized."
#endif

const int mk__g_hostCPU =
#if MK_HOST_CPU_X86
	kMkCPU_X86;
#elif MK_HOST_CPU_X86_64
	kMkCPU_X86_64;
#elif MK_HOST_CPU_ARM
	kMkCPU_ARM;
#elif MK_HOST_CPU_AARCH64
	kMkCPU_AArch64;
#elif MK_HOST_CPU_PPC
	kMkCPU_PowerPC;
#elif MK_HOST_CPU_MIPS
	kMkCPU_MIPS;
#else
/* TODO: kMkCPU_AArch64 */
# error "Architecture not recognized."
#endif

enum {
	kMkProjCfg_UsesCxx_Bit = 0x01,
	kMkProjCfg_NeedRelink_Bit = 0x02,
	kMkProjCfg_Linking_Bit = 0x04,
	kMkProjCfg_Package_Bit = 0x08
};
enum {
	kMkProjStat_LibFlags_Bit = 0x01,
	kMkProjStat_CalcDeps_Bit = 0x02
};

struct MkProject_s {
	char *name;
	char *path;
	char *outpath;
	char *binname;
	int type;
	int sys;
	int arch;

	MkStrList defs;

	MkStrList sources;
	MkStrList specialdirs;
	MkStrList libs;

	MkStrList testsources;

	MkStrList srcdirs; /* needed for determining object paths */

	char *linkerflags;
	char *extralibs;

	char *deplibsfilename;

	bitfield_t config;
	bitfield_t status;

	MkLib lib;

	struct MkProject_s *prnt, **p_head, **p_tail;
	struct MkProject_s *head, *tail;
	struct MkProject_s *prev, *next;

	struct MkProject_s *lib_prev, *lib_next;
};
struct MkProject_s *mk__g_proj_head = (struct MkProject_s *)0;
struct MkProject_s *mk__g_proj_tail = (struct MkProject_s *)0;
struct MkProject_s *mk__g_lib_proj_head = (struct MkProject_s *)0;
struct MkProject_s *mk__g_lib_proj_tail = (struct MkProject_s *)0;

static const struct { const char *const name; const int type; } mk__g_ifiles[] = {
	{ ".library"          , kMkProjTy_StaticLib        },
	{ ".mk-library"       , kMkProjTy_StaticLib        },
	{ ".dynamiclibrary"   , kMkProjTy_DynamicLib },
	{ ".mk-dynamiclibrary", kMkProjTy_DynamicLib },
	{ ".executable"       , kMkProjTy_Program     },
	{ ".mk-executable"    , kMkProjTy_Program     },
	{ ".application"      , kMkProjTy_Application    },
	{ ".mk-application"   , kMkProjTy_Application    },

	/* ".txt" indicator files for easier creation on Windows - notkyon:160204 */
	{ "mk-staticlib.txt"  , kMkProjTy_StaticLib        },
	{ "mk-dynamiclib.txt" , kMkProjTy_DynamicLib },
	{ "mk-executable.txt" , kMkProjTy_Program     },
	{ "mk-application.txt", kMkProjTy_Application    }
};

/* create a new (empty) project */
MkProject mk_prj_new(MkProject prnt) {
	MkProject proj;

	proj = (MkProject)mk_com_memory((void *)0, sizeof(*proj));

	proj->name = (char *)0;
	proj->path = (char *)0;
	proj->outpath = (char *)0;
	proj->binname = (char *)0;

	proj->type = kMkProjTy_Program;
	proj->sys = mk__g_hostOS;
	proj->arch = mk__g_hostCPU;

	proj->defs = mk_sl_new();

	proj->sources = mk_sl_new();
	proj->specialdirs = mk_sl_new();
	proj->libs = mk_sl_new();

	proj->testsources = mk_sl_new();

	proj->srcdirs = mk_sl_new();

	proj->linkerflags = (char *)0;
	proj->extralibs = (char *)0;

	proj->deplibsfilename = (char *)0;

	proj->config = 0;
	proj->status = 0;

	proj->lib = (MkLib)0;

	proj->prnt = prnt;
	proj->p_head = prnt ? &prnt->head : &mk__g_proj_head;
	proj->p_tail = prnt ? &prnt->tail : &mk__g_proj_tail;
	proj->head = (struct MkProject_s *)0;
	proj->tail = (struct MkProject_s *)0;
	proj->next = (struct MkProject_s *)0;
	if( (proj->prev = *proj->p_tail) != (struct MkProject_s *)0 ) {
		(*proj->p_tail)->next = proj;
	} else {
		*proj->p_head = proj;
	}
	*proj->p_tail = proj;

	proj->lib_prev = (struct MkProject_s *)0;
	proj->lib_next = (struct MkProject_s *)0;

	return proj;
}

/* delete an existing project */
void mk_prj_delete(MkProject proj) {
	if( !proj )
		return;

	while( proj->head ) {
		mk_prj_delete(proj->head);
	}

	proj->name = (char *)mk_com_memory((void *)proj->name, 0);
	proj->path = (char *)mk_com_memory((void *)proj->path, 0);
	proj->outpath = (char *)mk_com_memory((void *)proj->outpath, 0);
	proj->binname = (char *)mk_com_memory((void *)proj->binname, 0);

	mk_sl_delete(proj->defs);

	mk_sl_delete(proj->sources);
	mk_sl_delete(proj->specialdirs);
	mk_sl_delete(proj->libs);

	mk_sl_delete(proj->testsources);

	mk_sl_delete(proj->srcdirs);

	proj->linkerflags = (char *)mk_com_memory((void *)proj->linkerflags, 0);
	proj->extralibs = (char *)mk_com_memory((void *)proj->extralibs, 0);

	proj->deplibsfilename = (char *)mk_com_memory((void*)proj->deplibsfilename, 0);

	if( proj->lib ) {
		mk_lib_delete(proj->lib);
	}

	if( proj->prev ) {
		proj->prev->next = proj->next;
	}
	if( proj->next ) {
		proj->next->prev = proj->prev;
	}

	MK_ASSERT( proj->p_head != (struct MkProject_s **)0 );
	MK_ASSERT( proj->p_tail != (struct MkProject_s **)0 );

	if( *proj->p_head==proj ) {
		*proj->p_head = proj->next;
	}
	if( *proj->p_tail==proj ) {
		*proj->p_tail = proj->prev;
	}

	if( proj->type == kMkProjTy_StaticLib ) {
		if( proj->lib_prev ) {
			proj->lib_prev->lib_next = proj->lib_next;
		} else {
			mk__g_lib_proj_head = proj->lib_next;
		}

		if( proj->lib_next ) {
			proj->lib_next->lib_prev = proj->lib_prev;
		} else {
			mk__g_lib_proj_tail = proj->lib_prev;
		}
	}

	mk_com_memory((void *)proj, 0);
}

/* delete all projects */
void mk_prj_deleteAll(void) {
	while( mk__g_proj_head )
		mk_prj_delete(mk__g_proj_head);
}

/* retrieve the first root project */
MkProject mk_prj_rootHead(void) {
	return mk__g_proj_head;
}

/* retrieve the last root project */
MkProject mk_prj_rootTail(void) {
	return mk__g_proj_tail;
}

/* retrieve the parent of a project */
MkProject mk_prj_getParent(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->prnt;
}

/* retrieve the first child project of the given project */
MkProject mk_prj_head(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->head;
}

/* retrieve the last child project of the given project */
MkProject mk_prj_tail(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->tail;
}

/* retrieve the sibling project before the given project */
MkProject mk_prj_prev(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->prev;
}

/* retrieve the sibling project after the given project */
MkProject mk_prj_next(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->next;
}

/* set the name of a project */
void mk_prj_setName(MkProject proj, const char *name) {
	const char *p;
	char tmp[PATH_MAX];

	MK_ASSERT( proj != (MkProject)0 );

	tmp[0] = 0;
	p = (const char *)0;

	if( name ) {
		if( !(p = strrchr(name, '/')) )
			p = strrchr(name, '\\');

		if( p ) {
			/* can't end in "/" */
			if( *(p+1)==0 ) {
				mk_log_fatalError(mk_com_va("invalid project name \"%s\"", name));
			}

			mk_com_strncpy(tmp, sizeof(tmp), name, (p-name) + 1);
			p++;
		} else {
			p = name;
		}
	}

	MK_ASSERT( p != (const char *)0 );

	proj->outpath = mk_com_dup(proj->outpath, tmp[0] ? tmp : (const char *)0);
	proj->name = mk_com_dup(proj->name, p);

	proj->deplibsfilename = mk_com_dup(proj->deplibsfilename,
		mk_com_va("%s/%s/%s.deplibs",
			mk_opt_getObjdirBase(),
			mk_opt_getConfigName(),
			proj->name));
}

/* set the output path of a project */
void mk_prj_setOutPath(MkProject proj, const char *path) {
	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( path != (const char *)0 );

	proj->outpath = mk_com_dup(proj->outpath, path);
}

/* set the path of a project */
void mk_prj_setPath(MkProject proj, const char *path) {
	MK_ASSERT( proj != (MkProject)0 );

	proj->path = mk_com_dup(proj->path, path);
}

/* set the type of a project (e.g., kMkProjTy_StaticLib) */
void mk_prj_setType(MkProject proj, int type) {
	MK_ASSERT( proj != (MkProject)0 );

	if( !proj->outpath ) {
		static const char *const pszlibdir = "lib/" MK_PLATFORM_DIR;
		static const char *const pszbindir = "bin/" MK_PLATFORM_DIR;

		proj->outpath = mk_com_dup(proj->outpath, mk_com_va( "%s/",
			type==kMkProjTy_StaticLib ? pszlibdir : pszbindir) );
		if( type==kMkProjTy_StaticLib && !mk_sl_getSize(mk__g_libdirs) ) {
			mk_front_pushLibDir(pszlibdir);
		}
	}

	if( proj->type == kMkProjTy_StaticLib ) {
		if( proj->lib_prev ) {
			proj->lib_prev->lib_next = proj->lib_next;
		} else {
			mk__g_lib_proj_head = proj->lib_next;
		}

		if( proj->lib_next ) {
			proj->lib_next->lib_prev = proj->lib_prev;
		} else {
			mk__g_lib_proj_tail = proj->lib_prev;
		}
	}

	proj->type = type;

	if( proj->type == kMkProjTy_StaticLib ) {
		proj->lib_next = (struct MkProject_s *)0;
		if( (proj->lib_prev = mk__g_lib_proj_tail) != (struct MkProject_s *)0 ) {
			mk__g_lib_proj_tail->lib_next = proj;
		} else {
			mk__g_lib_proj_head = proj;
		}
		mk__g_lib_proj_tail = proj;
	}
}

/* retrieve the (non-modifyable) name of a project */
const char *mk_prj_getName(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->name;
}

/* retrieve the (non-modifyable) path of a project */
const char *mk_prj_getPath(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->path;
}

/* retrieve the output path of a project */
const char *mk_prj_getOutPath(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->outpath;
}

/* retrieve the type of a project (e.g., kMkProjTy_Program) */
int mk_prj_getType(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->type;
}

/* add a library to the project */
void mk_prj_addLib(MkProject proj, const char *libname) {
	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( libname != (const char *)0 );

	mk_sl_pushBack(proj->libs, libname);
}

/* retrieve the number of libraries in a project */
size_t mk_prj_numLibs(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return mk_sl_getSize(proj->libs);
}

/* retrieve a library of a project */
const char *mk_prj_libAt(MkProject proj, size_t i) {
	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( i < mk_sl_getSize(proj->libs) );

	return mk_sl_at(proj->libs, i);
}

/* append a linker flag to the project */
void mk_prj_addLinkFlags(MkProject proj, const char *flags) {
	size_t l;

	MK_ASSERT( proj != (MkProject)0 );
	/* MK_ASSERT( flags != (const char *)0 ); :: this happens due to built-in libs */
	if( !flags ) {
		return;
	}

	l = proj->linkerflags ? proj->linkerflags[0]!=0 : 0;

	if( l ) {
		proj->linkerflags = mk_com_append(proj->linkerflags, " ");
	}

	proj->linkerflags = mk_com_append(proj->linkerflags, flags);
}

/* retrieve the current linker flags of a project */
const char *mk_prj_getLinkFlags(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	return proj->linkerflags ? proj->linkerflags : "";
}

/* append "extra libs" to the project (found in ".libs" files) */
void mk_prj_appendExtraLibs(MkProject proj, const char *extras)
{
	size_t l;

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( extras != (const char *)0 );

	l = proj->extralibs ? proj->extralibs[0]!=0 : 0;

	if( l ) {
		proj->extralibs = mk_com_append(proj->extralibs, " ");
	}

	proj->extralibs = mk_com_append(proj->extralibs, extras);
}

/* retrieve the "extra libs" of a project */
const char *mk_prj_getExtraLibs(MkProject proj)
{
	MK_ASSERT( proj != (MkProject)0 );

	return proj->extralibs ? proj->extralibs : "";
}

/* retrieve all "extra libs" of a project (includes project children) */
void mk_prj_completeExtraLibs(MkProject proj, char *extras, size_t n) {
	MkProject x;

	for( x=mk_prj_head(proj); x; x=mk_prj_next(x) ) {
		mk_prj_completeExtraLibs(x, extras, n);
	}

	mk_com_strcat(extras, n, mk_prj_getExtraLibs(proj));
	mk_com_strcat(extras, n, " ");
}

/* add a source file to a project */
void mk_prj_addSourceFile(MkProject proj, const char *src) {
	const char *p;

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( src != (const char *)0 );

	MK_ASSERT( proj->sources != (MkStrList)0 );

	if( (p = strrchr(src, '.')) != (const char *)0 ) {
		if( !strcmp(p, ".cc") || !strcmp(p, ".cxx") || !strcmp(p, ".cpp" )
		 || !strcmp(p, ".c++")) {
			proj->config |= kMkProjCfg_UsesCxx_Bit;
		} else if(!strcmp(p, ".mm")) {
			proj->config |= kMkProjCfg_UsesCxx_Bit;
		}
	}

	mk_sl_pushBack(proj->sources, src);

	p = strrchr(src, '/');
	if( !p ) {
		p = strrchr(src, '\\');
	}

	if( p ) {
		char temp[PATH_MAX], rel[PATH_MAX];

		mk_com_strncpy(temp, sizeof(temp), src, p - src);
		temp[p - src] = '\0';

		mk_com_relPath(rel, sizeof(rel), proj->path, temp);

		mk_sl_pushBack(proj->srcdirs, rel);
	}
}

/* retrieve the number of source files within a project */
size_t mk_prj_numSourceFiles(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	MK_ASSERT( proj->sources != (MkStrList)0 );

	return mk_sl_getSize(proj->sources);
}

/* retrieve a source file of a project */
const char *mk_prj_sourceFileAt(MkProject proj, size_t i) {
	MK_ASSERT( proj != (MkProject)0 );

	MK_ASSERT( proj->sources != (MkStrList)0 );

	return mk_sl_at(proj->sources, i);
}

/* add a test source file to a project */
void mk_prj_addTestSourceFile(MkProject proj, const char *src) {
	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( src != (const char *)0 );

	MK_ASSERT( proj->testsources != (MkStrList)0 );

	/*
	if( (p = strrchr(src, '.')) != (const char *)0 ) {
		if( !strcmp(p, ".cc") || !strcmp(p, ".cxx") || !strcmp(p, ".cpp" )
		 || !strcmp(p, ".c++"))
			proj->config |= kMkProjCfg_UsesCxx_Bit;
		else if(!strcmp(p, ".mm"))
			proj->config |= kMkProjCfg_UsesCxx_Bit;
	}
	*/

	mk_sl_pushBack(proj->testsources, src);
}

/* retrieve the number of test source files within a project */
size_t mk_prj_numTestSourceFiles(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	MK_ASSERT( proj->sources != (MkStrList)0 );

	return mk_sl_getSize(proj->testsources);
}

/* retrieve a test source file of a project */
const char *mk_prj_testSourceFileAt(MkProject proj, size_t i) {
	MK_ASSERT( proj != (MkProject)0 );

	MK_ASSERT( proj->sources != (MkStrList)0 );

	return mk_sl_at(proj->testsources, i);
}

/* add a "special directory" to a project */
void mk_prj_addSpecialDir(MkProject proj, const char *dir) {
	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( dir != (const char *)0 );

	MK_ASSERT( proj->specialdirs != (MkStrList)0 );

	mk_sl_pushBack(proj->specialdirs, dir);
}

/* retrieve the number of "special directories" within a project */
size_t mk_prj_numSpecialDirs(MkProject proj) {
	MK_ASSERT( proj != (MkProject)0 );

	MK_ASSERT( proj->specialdirs != (MkStrList)0 );

	return mk_sl_getSize(proj->specialdirs);
}

/* retrieve a "special directory" of a project */
const char *mk_prj_specialDirAt(MkProject proj, size_t i) {
	MK_ASSERT( proj != (MkProject)0 );

	MK_ASSERT( proj->specialdirs != (MkStrList)0 );

	return mk_sl_at(proj->specialdirs, i);
}

/* determine if a given project is targetted */
int mk_prj_isTarget(MkProject proj) {
	size_t i, n;

	if( !(n = mk_sl_getSize(mk__g_targets)) ) {
		return 1;
	}

	for( i=0; i<n; i++ ) {
		if( !strcmp(mk_prj_getName(proj), mk_sl_at(mk__g_targets, i)) ) {
			return 1;
		}
	}

	return 0;
}

/* display a tree of projects to stdout */
void mk_prj_printAll(MkProject proj, const char *margin) {
	size_t i, n;
	char marginbuf[256];
	char bin[256];

	MK_ASSERT( margin != (const char *)0 );

	if( !proj ) {
		for( proj=mk_prj_rootHead(); proj; proj=mk_prj_next(proj) ) {
			mk_prj_printAll(proj, margin);
		}

		return;
	}

	if( mk_prj_isTarget(proj) ) {
		mk_bld_getBinName(proj, bin, sizeof(bin));

		printf("%s%s; \"%s\"\n", margin, mk_prj_getName(proj), bin);

		n = mk_prj_numSourceFiles(proj);
		for( i=0; i<n; i++ ) {
			printf("%s * %s\n", margin, mk_prj_sourceFileAt(proj, i));
		}

		n = mk_prj_numTestSourceFiles(proj);
		for( i=0; i<n; i++ ) {
			printf("%s # %s\n", margin, mk_prj_testSourceFileAt(proj, i));
		}
	}

	mk_com_strcpy(marginbuf, sizeof(marginbuf), margin);
	mk_com_strcat(marginbuf, sizeof(marginbuf), "  ");
	for( proj=mk_prj_head(proj); proj; proj=mk_prj_next(proj) ) {
		mk_prj_printAll(proj, marginbuf);
	}
}

/* expand library dependencies */
static void mk_prj__expandLibDeps_r( MkProject proj )
{
	const char *libname;
	MkProject libproj;
	size_t i, j, n, m;
	MkLib lib;

	do {
		n = mk_sl_getSize(proj->libs);
		for( i=0; i<n; ++i ) {
			if( !( libname = mk_sl_at( proj->libs, i ) ) ) {
				continue;
			}

			if( !( lib = mk_lib_find( libname ) ) ) {
				mk_log_errorMsg( mk_com_va( "couldn't find library ^E\"%s\"^&",
					libname ) );
				continue;
			}

			if( !lib->proj ) {
				continue;
			}

			libproj = lib->proj;
			lib->proj = ( MkProject )0; /*prevent infinite recursion*/
			mk_prj__expandLibDeps_r( libproj );
			lib->proj = libproj; /*restore*/

			m = mk_sl_getSize( libproj->libs );
			for( j=0; j<m; ++j ) {
				mk_sl_pushBack( proj->libs, mk_sl_at( libproj->libs, j ) );
			}
		}

		/* optimization -- if there were no changes to the working set then
		 * don't spend time looking for unique entries */
		if( mk_sl_getSize( proj->libs ) == n ) {
			break;
		}

		mk_sl_makeUnique( proj->libs );
	} while( mk_sl_getSize( proj->libs ) != n ); /* mk_sl_makeUnique can alter count */
}
static void mk_prj__saveLibDeps( MkProject proj ) {
	const char *p;
	size_t i, n, len;
	FILE *fp;

	MK_ASSERT( proj->deplibsfilename != ( char * )0 );

	fp = fopen( proj->deplibsfilename, "wb" );
	if( !fp ) {
		mk_log_errorMsg( mk_com_va( "failed to write ^E\"%s\"^&",
			proj->deplibsfilename ) );
	}

	fprintf( fp, "DEPS1.00" );

	n = mk_sl_getSize( proj->libs );
	fwrite( &n, sizeof( n ), 1, fp );
	for( i=0; i<n; ++i ) {
		p = mk_sl_at( proj->libs, i );
		if( !p ) {
			p = "";
		}

		len = strlen( p );

		fwrite( &len, sizeof( len ), 1, fp );
		fprintf( fp, "%s", p );
	}

	fclose( fp );

#if MK_DEBUG_LIBDEPS_ENABLED
	mk_dbg_outf( "libdeps: saved \"%s\"\n", proj->deplibsfilename );
#endif
}
static void mk_prj__loadLibDeps( MkProject proj ) {
	size_t i, n, temp;
	FILE *fp;
	char buf[ 512 ];

	MK_ASSERT( proj->deplibsfilename != ( char * )0 );

#if MK_DEBUG_LIBDEPS_ENABLED
	mk_dbg_outf( "libdeps: opening \"%s\"...\n", proj->deplibsfilename );
#endif

	fp = fopen( proj->deplibsfilename, "rb" );
	if( !fp ) {
#if MK_DEBUG_LIBDEPS_ENABLED
		mk_dbg_outf( "libdeps: failed to open\n" );
#endif
		return;
	}

	if( !fread( &buf[0], 8, 1, fp ) ) {
		fclose( fp );
#if MK_DEBUG_LIBDEPS_ENABLED
		mk_dbg_outf( "libdeps: failed; no header\n" );
#endif
		return;
	}
	buf[8] = '\0';
	if( strcmp( buf, "DEPS1.00" ) != 0 ) {
		fclose( fp );
#if MK_DEBUG_LIBDEPS_ENABLED
		mk_dbg_outf( "libdeps: failed; invalid header "
			"[%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X]{\"%.8s\"}\n",
			buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],
			buf );
#endif
		return;
	}

	if( !fread( &n, sizeof( n ), 1, fp ) ) {
		fclose( fp );
#if MK_DEBUG_LIBDEPS_ENABLED
		mk_dbg_outf( "libdeps: failed; no item count\n" );
#endif
		return;
	}

	for( i=0; i<n; ++i ) {
		if( !fread( &temp, sizeof( temp ), 1, fp ) ) {
			fclose( fp );
#if MK_DEBUG_LIBDEPS_ENABLED
			mk_dbg_outf( "libdeps: failed; item %u -- missing length\n",
				( unsigned int )i );
#endif
			return;
		}

		if( temp >= sizeof( buf ) ) {
			fclose( fp );
#if MK_DEBUG_LIBDEPS_ENABLED
			mk_dbg_outf( "libdeps: failed; item %u is too long (%u >= %u)\n",
				( unsigned int )i, ( unsigned int )temp,
				( unsigned int )sizeof( buf ) );
#endif
			while( i > 0 ) {
				mk_sl_popBack( proj->libs );
				--i;
			}
			return;
		}

		if( !fread( &buf[0], temp, 1, fp ) ) {
			fclose( fp );
#if MK_DEBUG_LIBDEPS_ENABLED
			mk_dbg_outf( "libdeps: failed; item %u -- missing data\n",
				( unsigned int )i );
#endif
			while( i > 0 ) {
				mk_sl_popBack( proj->libs );
				--i;
			}
			return;
		}
		buf[ temp ] = '\0';

		mk_sl_pushBack( proj->libs, buf );
	}

	fclose( fp );

#if MK_DEBUG_LIBDEPS_ENABLED
	mk_dbg_outf( "libdeps: succeeded\n" );
#endif

	mk_sl_makeUnique( proj->libs );

#if MK_DEBUG_LIBDEPS_ENABLED
	mk_dbg_enter( "libdeps-project(\"%s\")", proj->name );
	n = mk_sl_getSize( proj->libs );
	for( i = 0; i < n; ++i ) {
		mk_dbg_outf( "\"%s\"\n", mk_sl_at( proj->libs, i ) );
	}
	mk_dbg_leave();
#endif
}

void mk_prj_calcDeps(MkProject proj) {
	if( proj->status & kMkProjStat_CalcDeps_Bit ) {
		return;
	}
	proj->status |= kMkProjStat_CalcDeps_Bit;

	/* <BUG> requires rebuild if a library is removed */
	mk_prj__loadLibDeps( proj );
	/* </BUG> */

	mk_prj__expandLibDeps_r( proj );
	mk_prj__saveLibDeps( proj );
}

/* given an array of library names, return a string of flags */
void mk_prj_calcLibFlags(MkProject proj) {
	static char flags[32768];
	const char *libname;
	size_t i, n;
	MkLib lib;

	if( proj->status & kMkProjStat_LibFlags_Bit ) {
		return;
	}

	proj->status |= kMkProjStat_LibFlags_Bit;

	/*
	 * TODO: This would be a good place to parse the extra libs of the project
	 *       to remove any "!blah" linker flags. Also, it would be good to check
	 *       if the extra libs is referring to a Mk-known library (e.g.,
	 *       "opengl"), or a command-line setting as well (e.g., "-lGL") as the
	 *       latter is not very platform friendly. Elsewhere, conditionals could
	 *       be checked as well. (e.g., "mswin:-lopengl32 linux:-lGL" etc...)
	 *       For now, the current system (just extra linker flags) will work.
	 */

	mk_prj_calcDeps(proj);

	/* find libraries */
	n = mk_sl_getSize(proj->libs);
	for( i=0; i<n; i++ ) {
		if( !(libname = mk_sl_at(proj->libs, i)) ) {
			continue;
		}

		if( !(lib = mk_lib_find(libname)) ) {
			/*
			mk_log_errorMsg(mk_com_va("couldn't find library ^E\"%s\"^&", libname));
			*/
			continue;
		}

#if MK_DEBUG_LIBDEPS_ENABLED
		mk_dbg_outf( "libdeps: \"%s\" <- \"%s\"\n", proj->name, libname );
#endif

		/*
		 * NOTE: Projects are managed as dependencies; not as linker flags
		 */
		if( lib->proj ) {
			continue;
		}

		mk_prj_addLinkFlags(proj, mk_lib_getFlags(lib, proj->sys));
	}

	/* add these flags */
	mk_prj_completeExtraLibs(proj, flags, sizeof(flags));
	mk_prj_addLinkFlags(proj, flags);
}

/* find a project by name */
MkProject mk_prj_find_r(MkProject from, const char *name) {
	MkProject p, r;

	MK_ASSERT( from != (MkProject)0 );
	MK_ASSERT( name != (const char *)0 );

	if( !strcmp(mk_prj_getName(from), name) ) {
		return from;
	}

	for( p=mk_prj_head(from); p; p=mk_prj_next(p) ) {
		r = mk_prj_find_r(p, name);
		if( r != (MkProject)0 ) {
			return r;
		}
	}

	return (MkProject)0;
}
MkProject mk_prj_find(const char *name) {
	MkProject p, r;

	MK_ASSERT( name != (const char *)0 );

	for( p=mk_prj_rootHead(); p; p=mk_prj_next(p) ) {
		r = mk_prj_find_r(p, name);
		if( r != (MkProject)0 ) {
			return r;
		}
	}

	return (MkProject)0;
}

/*
 *	========================================================================
 *	DIRECTORY MANAGEMENT
 *	========================================================================
 */

/* determine if the directory name specified is special */
int mk_prjfs_isSpecialDir(MkProject proj, const char *name) {
	static const char *const specialdirs[] = {
		MK_PLATFORM_OS_NAME_MSWIN,
		MK_PLATFORM_OS_NAME_UWP,
		MK_PLATFORM_OS_NAME_CYGWIN,
		MK_PLATFORM_OS_NAME_LINUX,
		MK_PLATFORM_OS_NAME_MACOSX,
		MK_PLATFORM_OS_NAME_UNIX,

		MK_PLATFORM_CPU_NAME_X86,
		MK_PLATFORM_CPU_NAME_X64,
		MK_PLATFORM_CPU_NAME_ARM,
		MK_PLATFORM_CPU_NAME_AARCH64,
		MK_PLATFORM_CPU_NAME_MIPS,
		MK_PLATFORM_CPU_NAME_PPC,
		MK_PLATFORM_CPU_NAME_WASM,

		"appexec", "exec", "lib", "dylib"
	};
	size_t i;

	/* check the target system */
	switch(proj->sys) {
#define MK__TRY_PLAT(ProjSys_,Plat_) \
	case ProjSys_: \
		if( !strcmp(name, Plat_) ) { \
			return 1; \
		} \
		break

	MK__TRY_PLAT(kMkOS_MSWin , MK_PLATFORM_OS_NAME_MSWIN );
	MK__TRY_PLAT(kMkOS_UWP   , MK_PLATFORM_OS_NAME_UWP   );
	MK__TRY_PLAT(kMkOS_Cygwin, MK_PLATFORM_OS_NAME_CYGWIN);
	MK__TRY_PLAT(kMkOS_Linux , MK_PLATFORM_OS_NAME_LINUX );
	MK__TRY_PLAT(kMkOS_MacOSX, MK_PLATFORM_OS_NAME_MACOSX);
	MK__TRY_PLAT(kMkOS_Unix  , MK_PLATFORM_OS_NAME_UNIX  );

#undef MK__TRY_PLAT

	default:
		/*
		 *	NOTE: If you get this message, it's likely because you failed to add
		 *	      the appropriate case statement to this switch. Make sure you
		 *	      port this function too, when adding a new target system.
		 */
		mk_log_errorMsg(mk_com_va("unknown project system ^E'%d'^&", proj->sys));
		break;
	}

	/* check the target architecture */
	switch(proj->arch) {
#define MK__TRY_PLAT(ProjArch_,Plat_) \
	case ProjArch_: \
		if( !strcmp(name, Plat_) ) { \
			return 1; \
		} \
		break

	MK__TRY_PLAT(kMkCPU_X86        , MK_PLATFORM_CPU_NAME_X86    );
	MK__TRY_PLAT(kMkCPU_X86_64     , MK_PLATFORM_CPU_NAME_X64    );
	MK__TRY_PLAT(kMkCPU_ARM        , MK_PLATFORM_CPU_NAME_ARM    );
	MK__TRY_PLAT(kMkCPU_AArch64    , MK_PLATFORM_CPU_NAME_AARCH64);
	MK__TRY_PLAT(kMkCPU_PowerPC    , MK_PLATFORM_CPU_NAME_PPC    );
	MK__TRY_PLAT(kMkCPU_MIPS       , MK_PLATFORM_CPU_NAME_MIPS   );
	MK__TRY_PLAT(kMkCPU_WebAssembly, MK_PLATFORM_CPU_NAME_WASM   );

#undef MK__TRY_PLAT

	default:
		/*
		 *	NOTE: If you get this message, it's likely because you failed to add
		 *	      the appropriate case statement to this switch. Make sure you
		 *	      port this function too, when adding a new target architecture.
		 */
		mk_log_errorMsg(mk_com_va("unknown project architecture ^E'%d'^&", proj->arch));
		break;
	}

	/* check the project type */
	switch(mk_prj_getType(proj)) {
	case kMkProjTy_Application:
		if( !strcmp(name, "appexec") ) {
			return 1;
		}
		break;
	case kMkProjTy_Program:
		if( !strcmp(name, "exec") ) {
			return 1;
		}
		break;
	case kMkProjTy_StaticLib:
		if( !strcmp(name, "lib") ) {
			return 1;
		}
		break;
	case kMkProjTy_DynamicLib:
		if( !strcmp(name, "dylib") ) {
			return 1;
		}
		break;
	default:
		/*
		 *	NOTE: If you get this message, it's likely because you failed to add
		 *	      the appropriate case statement to this switch. Make sure you
		 *	      port this function too, when adding a new binary target.
		 */
		mk_log_errorMsg(mk_com_va("unknown project type ^E'%d'^&", proj->type));
		break;
	}

	/*
	 * XXX: STUPID HACK! Special directories cannot be included if their
	 *      condition does not match. We're testing for all of the special
	 *      directories again here to ensure this doesn't happen now that any
	 *      non-project-owning directory gets scavanged for source files.
	 */
	for( i=0; i<sizeof(specialdirs)/sizeof(specialdirs[0]); i++ ) {
		if( !strcmp(name, specialdirs[i]) ) {
			return -1;
		}
	}

	/*
	 *	PLAN: Add a ".freestanding" (or perhaps ".native?") indicator file to
	 *        support OS/kernel development. Would be cool.
	 */

	return 0;
}

/* determine whether the directory is an include directory */
int mk_prjfs_isIncDir(MkProject proj, const char *name) {
	static const char *const names[] = {
		"inc",
		"incs",
		"include",
		"includes",
		"headers"
	};
	size_t i;

	(void)proj;

	for( i=0; i<sizeof(names)/sizeof(names[0]); i++ ) {
		if( !strcmp(name, names[i]) ) {
			return 1;
		}
	}

	return 0;
}

/* determine whether the directory is a library directory */
int mk_prjfs_isLibDir(MkProject proj, const char *name) {
	static const char *const names[] = {
		"lib",
		"libs",
		"library",
		"libraries"
	};
	size_t i;

	(void)proj;

	for( i=0; i<sizeof(names)/sizeof(names[0]); i++ ) {
		if( !strcmp(name, names[i]) ) {
			return 1;
		}
	}

	return 0;
}

/* determine if the directory is a unit testing directory */
int mk_prjfs_isTestDir(MkProject proj, const char *name) {
	if( proj ) {}

	if( !strcmp(name, "uts" )
	 || !strcmp(name, "tests")
	 || !strcmp(name, "units")) {
		return 1;
	}

	return 0;
}

/* determine whether a directory owns a project indicator file or not */
int mk_prjfs_isDirOwner(const char *path) {
	MkStat_t s;
	size_t i;

	for( i=0; i<sizeof(mk__g_ifiles)/sizeof(mk__g_ifiles[0]); i++ ) {
		if( stat(mk_com_va("%s%s", path, mk__g_ifiles[i]), &s) != -1 ) {
			return 1;
		}
	}

	return 0;
}

/* enumerate all source files in a directory */
static void mk_prj__enumSourceFilesImpl(MkProject proj, const char *srcdir) {
	struct dirent *dp;
	char path[PATH_MAX], *p;
	DIR *d;
	int r;

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( srcdir != (const char *)0 );

	if( !(d = mk_fs_openDir(srcdir)) ) {
		mk_log_error(srcdir, 0, 0, "mk_fs_openDir() call in mk_prjfs_enumSourceFiles() failed");
		return;
	}

	while( (dp = mk_fs_readDir(d)) != (struct dirent *)0 ) {
		mk_com_strcpy(path, sizeof(path), srcdir);
		mk_com_strcat(path, sizeof(path), dp->d_name);

		p = strrchr(dp->d_name, '.');
		if( p != ( char * )0 ) {
			if( *(p+1)==0 ) {
				continue;
			}

			if( !strcmp(p, ".c" )
			 || !strcmp(p, ".cc")
			 || !strcmp(p, ".cpp")
			 || !strcmp(p, ".cxx")
			 || !strcmp(p, ".c++")
			 || !strcmp(p, ".m")
			 || !strcmp(p, ".mm")) {
				if( !mk_fs_isFile(path) ) {
					continue;
				}

				mk_prj_addSourceFile(proj, path);
				continue;
			}
		}

		if( !mk_fs_isDir(path) ) {
			errno = 0;
			continue;
		}

		/* the following tests all require an ending '/' */
		mk_com_strcat(path, sizeof(path), "/");

		r = mk_prjfs_isSpecialDir(proj, dp->d_name); /*can now return -1*/
		if( r!=0 ) {
			if( r==1 ) {
				mk_prj_addSpecialDir(proj, dp->d_name);
				mk_prjfs_enumSourceFiles(proj, path);
			}
			continue;
		}

		if( mk_prjfs_isIncDir(proj, dp->d_name) ) {
			mk_front_pushIncDir(path);
			mk_al_managePackage_r(proj->name, proj->sys, path);
			continue;
		}

		if( mk_prjfs_isLibDir(proj, dp->d_name) ) {
			if( proj->config & kMkProjCfg_Package_Bit ) {
				mk_prj_setOutPath(proj, path);
			}
			mk_front_pushLibDir(path);
			continue;
		}

		if( mk_prjfs_isTestDir(proj, dp->d_name) ) {
			mk_prjfs_enumTestSourceFiles(proj, path);
			continue;
		}

		if( !mk_prjfs_isDirOwner(path) ) {
			mk_prjfs_enumSourceFiles(proj, path);
			continue;
		}
	}

	mk_fs_closeDir(d);

	if( errno ) {
		mk_log_error(srcdir, 0, 0, "mk_prjfs_enumSourceFiles() failed");
	}
}
void mk_prjfs_enumSourceFiles( MkProject proj, const char *srcdir ) {
	mk_dbg_enter( "mk_prjfs_enumSourceFiles(project:\"%s\", srcdir:\"%s\")", proj->name, srcdir );
	mk_prj__enumSourceFilesImpl( proj, srcdir );
	mk_dbg_leave();
}

/* enumerate all unit tests in a directory */
static void mk_prj__enumTestSourceFilesImpl(MkProject proj, const char *srcdir) {
	struct dirent *dp;
	char path[PATH_MAX], *p;
	DIR *d;

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( srcdir != (const char *)0 );

	if( !(d = mk_fs_openDir(srcdir)) ) {
		mk_log_error(srcdir, 0, 0, "mk_fs_openDir() call in mk_prjfs_enumTestSourceFiles() failed");
		return;
	}

	while( (dp = mk_fs_readDir(d)) != (struct dirent *)0 ) {
		mk_com_strcpy(path, sizeof(path), srcdir);
		mk_com_strcat(path, sizeof(path), dp->d_name);

		p = strrchr(dp->d_name, '.');
		if( p ) {
			if( *(p+1)==0 ) {
				continue;
			}

			if( !strcmp(p, ".c" )
			 || !strcmp(p, ".cc")
			 || !strcmp(p, ".cpp")
			 || !strcmp(p, ".cxx")
			 || !strcmp(p, ".c++")
			 || !strcmp(p, ".m")
			 || !strcmp(p, ".mm")) {
				if( !mk_fs_isFile(path) ) {
					continue;
				}

				mk_prj_addTestSourceFile(proj, path);
				continue;
			}
		}

		if( !mk_fs_isDir(path) ) {
			errno = 0;
			continue;
		}

		if( mk_prjfs_isSpecialDir(proj, dp->d_name) ) {
			mk_prj_addSpecialDir(proj, dp->d_name);
			mk_com_strcat(path, sizeof(path), "/"); /* ending '/' is necessary */
			mk_prjfs_enumTestSourceFiles(proj, path);
		}
	}

	mk_fs_closeDir(d);

	if( errno ) {
		mk_log_error(srcdir, 0, 0, "mk_prjfs_enumTestSourceFiles() failed");
	}
}
void mk_prjfs_enumTestSourceFiles( MkProject proj, const char *srcdir ) {
	MK_ASSERT( proj != ( MkProject )0 );
	MK_ASSERT( proj->name != ( const char * )0 );

	mk_dbg_enter( "mk_prjfs_enumTestSourceFiles(project:\"%s\", srcdir:\"%s\")", proj->name, srcdir );
	mk_prj__enumTestSourceFilesImpl( proj, srcdir );
	mk_dbg_leave();
}

/* calculate the name of a project based on a directory or file */
int mk_prjfs_calcName(MkProject proj, const char *path, const char *file) {
	size_t i;
	FILE *f;
	char buf[256], cwd[256], *p;

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( path != (const char *)0 );

	/* read a single line from the file */
	if( file != (const char *)0 ) {
		if( !(f = fopen(file, "r")) ) {
			mk_log_error(file, 0, 0, "fopen() call in mk_prjfs_calcName() failed");
			return 0;
		}

		buf[0] = 0;
		fgets(buf, sizeof(buf), f);

		fclose(f);

		/* strip the line of whitespace on both ends */
		i = 0;
		while( buf[i]<=' ' && buf[i]!=0 ) {
			i++;
		}

		for( p=&buf[i]; *p>' '; p++ );
		if( *p!=0 ) {
			*p = 0;
		}
	} else {
#if 0
		/*
		 * XXX: Why did I use 'cwd' instead of path? ...
		 */
		mk_fs_getCWD(cwd, sizeof(cwd));
#else
		mk_com_strcpy(cwd, sizeof(cwd), path);
#endif

		if( !(p = strrchr(cwd, '/')) ) {
			p = cwd;
		} else if(*(p+1)==0) {
			if( strcmp(cwd, "/") != 0 ) {
				*p = 0;
				p = strrchr(cwd, '/');
				if( !p ) {
					p = cwd;
				}
			}
		}

		MK_ASSERT( p != (char *)0 );

		if( p!=cwd && *p=='/' ) {
			p++;
		}

		mk_com_strcpy(buf, sizeof(buf), p);
		i = 0;
	}

	/* if no name was specified in the file, use the directory name */
	if( !buf[i] ) {
		mk_com_strcpy(buf, sizeof(buf), path);

		/* mk_fs_realPath() always adds a '/' at the end of a directory */
		p = strrchr(buf, '/');

		/* ensure this is the case (debug-mode only) */
		MK_ASSERT( p != (char *)0 );
		MK_ASSERT( *(p+1)==0 );
		*p = 0; /* remove this ending '/' to not conflict with the following */

		/* if there's an ending '/' (always should be), mark the character
		   past that as the start of the directory name */
		i = ((p = strrchr(buf, '/')) != (char *)0) ? (p - buf) + 1 : 0;
	}

	/* set the project's name */
	mk_prj_setName(proj, &buf[i]);

	/* done */
	return 1;
}

/* add a single project using 'file' for information */
MkProject mk_prjfs_add(MkProject prnt, const char *path, const char *file, int type) {
	MkProject proj;

	MK_ASSERT( path != (const char *)0 );

	/* create the project */
	proj = mk_prj_new(prnt);

	/* calculate the appropriate name for the project */
	if( !mk_prjfs_calcName(proj, path, file) ) {
		mk_prj_delete(proj);
		return (MkProject)0;
	}

	/* prepare the passed settings of the project */
	mk_prj_setPath(proj, path);
	mk_prj_setType(proj, type);
	mk_bld_getBinName(proj, (char *)0, 0);

	if( proj->type==kMkProjTy_StaticLib || proj->type==kMkProjTy_DynamicLib ) {
		proj->lib = mk_lib_new();
		proj->lib->proj = proj;
		mk_lib_setName(proj->lib, proj->name);
		mk_lib_setFlags(proj->lib, proj->sys, mk_com_va("\"%s\"", proj->binname));
	}

	/* add the sources for the project */
	mk_prjfs_enumSourceFiles(proj, path);

	/* return the project */
	return proj;
}

/* find all projects within a specific packages directory */
void mk_prjfs_findPackages(const char *pkgdir) {
	struct dirent *dp;
	MkProject proj;
	char path[PATH_MAX];
	DIR *d;

	/* opne the directory to find packages */
	if( !(d = mk_fs_openDir(pkgdir)) ) {
		mk_log_fatalError(pkgdir);
	}

	/* run through each entry in the directory */
	while( (dp=mk_fs_readDir(d)) != (struct dirent *)0 ) {
		if( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ) {
			continue;
		}

		/* validate the directory */
		if( !mk_fs_realPath(mk_com_va("%s/%s/", pkgdir, dp->d_name ), path,
		sizeof(path))) {
			errno = 0;
			continue;
		}

		/* this is a project directory; treat as a static library */
		proj = mk_prjfs_add((MkProject)0, path, (const char *)0,
		kMkProjTy_StaticLib);
		if( !proj ) {
			continue;
		}
	}

	/* close this directory */
	mk_fs_closeDir(d);

	/* if an error occurred, display it */
	if( errno ) {
		mk_log_fatalError(mk_com_va("readdir(\"%s\") failed", pkgdir));
	}
}

/* find all dlls */
void mk_prjfs_findDynamicLibs(const char *dllsdir) {
	struct dirent *dp;
	MkProject proj;
	char path[PATH_MAX];
	DIR *d;

	/* opne the directory to find packages */
	if( !(d = mk_fs_openDir(dllsdir)) ) {
		mk_log_fatalError(dllsdir);
	}

	/* run through each entry in the directory */
	while( (dp=mk_fs_readDir(d)) != (struct dirent *)0 ) {
		if( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ) {
			continue;
		}

		/* validate the directory */
		if( !mk_fs_realPath(mk_com_va("%s/%s/", dllsdir, dp->d_name ), path,
		sizeof(path))) {
			errno = 0;
			continue;
		}

		/* this is a project directory; treat as a static library */
		proj = mk_prjfs_add((MkProject)0, path, (const char *)0,
		kMkProjTy_DynamicLib);
		if( !proj ) {
			continue;
		}
	}

	/* close this directory */
	mk_fs_closeDir(d);

	/* if an error occurred, display it */
	if( errno ) {
		mk_log_fatalError(mk_com_va("readdir(\"%s\") failed", dllsdir));
	}
}

/* find all tools */
void mk_prjfs_findTools(const char *tooldir) {
	struct dirent *dp;
	MkProject proj;
	char path[PATH_MAX];
	DIR *d;

	/* opne the directory to find packages */
	if( !(d = mk_fs_openDir(tooldir)) ) {
		mk_log_fatalError(tooldir);
	}

	/* run through each entry in the directory */
	while( (dp=mk_fs_readDir(d)) != (struct dirent *)0 ) {
		if( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ) {
			continue;
		}

		/* validate the directory */
		if( !mk_fs_realPath(mk_com_va("%s/%s/", tooldir, dp->d_name ), path,
		sizeof(path))) {
			errno = 0;
			continue;
		}

		/* this is a project directory; treat as a static library */
		proj = mk_prjfs_add((MkProject)0, path, (const char *)0,
		kMkProjTy_Program);
		if( !proj ) {
			continue;
		}
	}

	/* close this directory */
	mk_fs_closeDir(d);

	/* if an error occurred, display it */
	if( errno ) {
		mk_log_fatalError(mk_com_va("readdir(\"%s\") failed", tooldir));
	}
}

/* find all projects within a specific source directory, adding them as children
   to the 'prnt' project. */
void mk_prjfs_findProjects(MkProject prnt, const char *srcdir) {
	static const char *libs[] = { ".libs", ".user.libs", ".mk-libs",
		".user.mk-libs" };
	static char buf[32768];
	struct dirent *dp;
	MkStat_t s;
	MkProject proj;
	size_t i, j;
	FILE *f;
	char path[PATH_MAX], file[PATH_MAX], *p;
	DIR *d;

	proj = (MkProject)0;

	/* open the source directory to add projects */
	if( !(d = mk_fs_openDir(srcdir)) ) {
		mk_log_fatalError(srcdir);
	}

	/* run through each entry in the directory */
	while( (dp=mk_fs_readDir(d)) != (struct dirent *)0 ) {
		if( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ) {
			continue;
		}

		/* make sure this is a directory; get the real (absolute) path */
		if( !mk_fs_realPath(mk_com_va("%s/%s/", srcdir, dp->d_name ), path,
		sizeof(path))) {
			errno = 0;
			continue;
		}

		/* run through each indicator file test (.executable, ...) */
		for( i=0; i<sizeof(mk__g_ifiles)/sizeof(mk__g_ifiles[0]); i++ ) {
			mk_com_strcpy(file, sizeof(file), mk_com_va("%s%s", path, mk__g_ifiles[i].name));

			/* make sure this file exists */
			if( !mk_fs_isFile(file) ) {
				errno = 0;
				continue;
			}

			/* add the project file */
			proj = mk_prjfs_add(prnt, path, file, mk__g_ifiles[i].type);
			if( !proj ) {
				continue;
			}

			/* if this entry contains a '.libs' file, then add to the project */
			for( j=0; j<sizeof(libs)/sizeof(libs[0]); j++ ) {
				mk_com_strcpy(file, sizeof(file), mk_com_va("%s/%s", srcdir, libs[j]));

				if( stat(file, &s)!=0 ) {
					continue;
				}

				f = fopen(file, "r");
				if( f ) {
					if( fgets(buf, sizeof(buf), f)!=(char *)0 ) {
						p = strchr(buf, '\r');
						if( p ) {
							*p = '\0';
						}

						p = strchr(buf, '\n');
						if( p ) {
							*p = '\0';
						}

						mk_prj_appendExtraLibs(proj, buf);
					} else {
						mk_log_error(file, 1, 0, "failed to read line");
					}

					fclose(f);
					f = (FILE *)0;
				} else {
					mk_log_error(file, 0, 0, "failed to open for reading");
				}
			}

			/* find sub-projects */
			mk_prjfs_findProjects(proj, path);
		}
	}

	/* close this directory */
	mk_fs_closeDir(d);

	/* if an error occurred, display it */
	if( errno ) {
		mk_log_fatalError(mk_com_va("readdir(\"%s\") failed", srcdir));
	}
}

/* find the root directories, adding projects for any of the 'srcdirs' */
void mk_prjfs_findRootDirs(MkStrList srcdirs, MkStrList incdirs, MkStrList libdirs, MkStrList pkgdirs, MkStrList tooldirs, MkStrList dllsdirs) {
	struct { const char *name; MkStrList dst; } tests[] = {
		{ "src", (MkStrList)0 }, { "source"    , (MkStrList)0 },
		                       { "code"      , (MkStrList)0 },
		                       { "projects"  , (MkStrList)0 },
		{ "inc", (MkStrList)1 }, { "include"   , (MkStrList)1 },
		                       { "headers"   , (MkStrList)1 },
		                       { "includes"  , (MkStrList)1 },
		{ "lib", (MkStrList)2 }, { "library"   , (MkStrList)2 },
		                       { "libraries" , (MkStrList)2 },
		                       { "libs"      , (MkStrList)2 },
		{ "pkg", (MkStrList)3 }, { "packages"  , (MkStrList)3 },
		                       { "repository", (MkStrList)3 },
		                       { "repo"      , (MkStrList)3 },
		{ "tools", (MkStrList)4 }, { "games", (MkStrList)4 },
		{ "apps", (MkStrList)4 }, { "dlls", (MkStrList)5 },
		{ "sdk/include" , (MkStrList)1 },
		{ "sdk/lib"     , (MkStrList)2 },
		{ "sdk/packages", (MkStrList)3 },
		{ "sdk/tools"   , (MkStrList)4 },
		{ "sdk/dlls"    , (MkStrList)5 }
	};
	static const char *workspaces[] = { ".workspace", ".user.workspace",
		".mk-workspace", ".user.mk-workspace" };
	static char buf[32768];
	MkStat_t s;
	MkProject proj;
	size_t i, n;
	FILE *f;
	char *p;
	char path[256];

	MK_ASSERT( srcdirs  != (MkStrList)0 );
	MK_ASSERT( incdirs  != (MkStrList)0 );
	MK_ASSERT( libdirs  != (MkStrList)0 );
	MK_ASSERT( pkgdirs  != (MkStrList)0 );
	MK_ASSERT( tooldirs != (MkStrList)0 );
	MK_ASSERT( dllsdirs != (MkStrList)0 );

	/* "initializer element is not computable at load time" warning -- fix */
	for( i=0; i<sizeof(tests)/sizeof(tests[0]); i++ ) {
		if( tests[i].dst==(MkStrList)0 ) {
			tests[i].dst = srcdirs;
		} else if(tests[i].dst==(MkStrList)1) {
			tests[i].dst = incdirs;
		} else if(tests[i].dst==(MkStrList)2) {
			tests[i].dst = libdirs;
		} else if(tests[i].dst==(MkStrList)4) {
			tests[i].dst = tooldirs;
		} else if(tests[i].dst==(MkStrList)5) {
			tests[i].dst = dllsdirs;
		} else {
			MK_ASSERT( tests[i].dst==(MkStrList)3 );
			tests[i].dst = pkgdirs;
		}
	}

	/* run through each of the tests in the 'tests' array to see which
	   directories will be used in the build... */
	for( i=0; i<sizeof(tests)/sizeof(tests[0]); i++ ) {
		MK_ASSERT( tests[i].name != (const char *)0 );
		MK_ASSERT( tests[i].dst != (MkStrList)0 );

		if( mk_sl_getSize(tests[i].dst) > 0 ) {
			continue;
		}

		if( stat(tests[i].name, &s)==-1 ) {
			continue;
		}
		if( ~s.st_mode & S_IFDIR ) {
			continue;
		}

		mk_sl_pushBack(tests[i].dst, tests[i].name);
	}

	/* use the .workspace file to add extra directories */
	for( i=0; i<sizeof(workspaces)/sizeof(workspaces[0]); i++ ) {
		f = fopen(workspaces[i], "r");
		if( f ) {
			/* every line of the file specifies another directory */
			while( fgets(buf, sizeof(buf), f) != (char *)0 ) {
				p = strchr(buf, '\r');
				if( p ) {
					*p = '\0';
				}

				p = strchr(buf, '\n');
				if( p ) {
					*p = '\0';
				}

				if( !strncmp(buf, "src:", 4) ) {
					mk_front_pushSrcDir(&buf[4]);
					continue;
				}
				if( !strncmp(buf, "inc:", 4) ) {
					mk_front_pushIncDir(&buf[4]);
					continue;
				}
				if( !strncmp(buf, "lib:", 4) ) {
					mk_front_pushLibDir(&buf[4]);
					continue;
				}
				if( !strncmp(buf, "pkg:", 4) ) {
					mk_front_pushPkgDir(&buf[4]);
					continue;
				}
				if( !strncmp(buf, "tool:", 5) ) {
					mk_front_pushToolDir(&buf[5]);
					continue;
				}
				if( !strncmp(buf, "dlls:", 5) ) {
					mk_front_pushDynamicLibsDir(&buf[5]);
					continue;
				}
			}

			fclose(f);
		}
	}

	errno = 0;

	/* find packages first */
	n = mk_sl_getSize(pkgdirs);
	for( i=0; i<n; i++ ) {
		mk_prjfs_findPackages(mk_sl_at(pkgdirs, i));
	}

	/* find dlls */
	n = mk_sl_getSize(dllsdirs);
	for( i=0; i<n; i++ ) {
		mk_prjfs_findDynamicLibs(mk_sl_at(dllsdirs, i));
	}

	/* find tools next */
	n = mk_sl_getSize(tooldirs);
	for( i=0; i<n; i++ ) {
		mk_prjfs_findTools(mk_sl_at(tooldirs, i));
	}

	/* we'll try to find projects in the source directories */
	n = mk_sl_getSize(srcdirs);
	for( i=0; i<n; i++ ) {
		mk_prjfs_findProjects((MkProject)0, mk_sl_at(srcdirs, i));
	}

	/* if we found no projects in srcdirs then treat the current directory as a
	   project */
	if( !mk_prj_rootHead() && n>0 ) {
		if( !mk_fs_realPath("./", path, sizeof(path)) ) {
			return;
		}

		proj = mk_prj_new((MkProject)0);

		if( !mk_prjfs_calcName(proj, path, (const char *)0) ) {
			mk_prj_delete(proj);
			return;
		}

		mk_com_strcat(path, sizeof(path), mk_com_va("%s/", mk_sl_at(srcdirs, 0)));

		mk_prj_setPath(proj, path);
		mk_prj_setType(proj, kMkProjTy_Program);

		mk_prjfs_enumSourceFiles(proj, path);
	}
}

/* create the object directories for a particular project */
int mk_prjfs_makeObjDirs(MkProject proj) {
	const char *typename = "";
	size_t i, n;
	char objdir[PATH_MAX], cwd[PATH_MAX];

	MK_ASSERT( proj != (MkProject)0 );

	if( getcwd(cwd, sizeof(cwd))==(char *)0 ) {
		mk_log_errorMsg("getcwd() failed");
		return 0;
	}

	n = mk_com_strlen(cwd);

	switch(mk_prj_getType(proj)) {
	case kMkProjTy_Application:
		typename = "appexec";
		break;
	case kMkProjTy_Program:
		typename = "exec";
		break;
	case kMkProjTy_StaticLib:
		typename = "lib";
		break;
	case kMkProjTy_DynamicLib:
		typename = "dylib";
		break;
	default:
		MK_ASSERT_MSG(0, "project type is invalid");
		break;
	}

#if MK_SECLIB
# define snprintf sprintf_s
#endif
	snprintf(objdir, sizeof(objdir)-1, "%s/%s/%s/%s",
		mk_opt_getObjdirBase(),
		mk_opt_getConfigName(), typename,
		&mk_prj_getPath(proj)[n]);
	objdir[sizeof(objdir)-1] = 0;
#if MK_SECLIB
# undef snprintf
#endif

	if( ~mk__g_flags & kMkFlag_NoCompile_Bit ) {
		mk_fs_makeDirs(objdir);
	}

#if 0
	n = mk_prj_numSpecialDirs(proj);
	for( i=0; i<n; i++ )
		mk_fs_makeDirs(mk_com_va("%s%s/", objdir, mk_prj_specialDirAt(proj, i)));
#else
	n = mk_sl_getSize(proj->sources);
	for( i=0; i<n; i++ ) {
		char rel[PATH_MAX];
		char *p;
# ifdef _WIN32
		char *q;
# endif

		mk_com_relPath(rel, sizeof(rel), proj->path, mk_sl_at(proj->sources, i));
		p = strrchr( rel, '/' );
# ifdef _WIN32
		q = strrchr( rel, '\\' );
		if( q != NULL && q > p ) {
			p = q;
		}
# endif
		if( p != NULL ) {
			*p = '\0';
		}
		mk_sl_pushBack(proj->srcdirs, rel);
	}
#endif

	mk_sl_makeUnique(proj->srcdirs);
	if( ~mk__g_flags & kMkFlag_NoCompile_Bit ) {
		n = mk_sl_getSize(proj->srcdirs);
		for( i=0; i<n; i++ ) {
			mk_fs_makeDirs(mk_com_va("%s%s/", objdir, mk_sl_at(proj->srcdirs, i)));
		}
	}

	return 1;
}

/*
 *	========================================================================
 *	COMPILATION MANAGEMENT
 *	========================================================================
 */

MkStrList mk__g_unitTestCompiles = (MkStrList)0;
MkStrList mk__g_unitTestRuns = (MkStrList)0;
MkStrList mk__g_unitTestNames = (MkStrList)0;

/* initialize unit test arrays */
void mk_bld_initUnitTestArrays(void) {
	mk__g_unitTestCompiles = mk_sl_new();
	mk__g_unitTestRuns = mk_sl_new();
	mk__g_unitTestNames = mk_sl_new();
}

/* find which libraries are to be autolinked from a source file */
int mk_bld_findSourceLibs(MkStrList dst, int sys, const char *obj, const char *dep) {
	MkAutolink al;
	size_t i, n;
	MkDep d;
	MkLib l;

#if MK_DEBUG_FIND_SOURCE_LIBS_ENABLED
	mk_dbg_outf("mk_bld_findSourceLibs: \"%s\", \"%s\"\n", obj, dep);
#endif

	d = mk_dep_find(obj);
	if( !d ) {
		if( !mk_mfdep_load(dep) ) {
			mk_log_errorMsg(mk_com_va("failed to read dependency ^F\"%s\"^&", dep));
			return 0;
		}

		d = mk_dep_find(obj);
		if( !d ) {
			mk_log_errorMsg(mk_com_va("mk_dep_find(^F\"%s\"^&) failed", obj));
			mk_dep_debugPrintAll();
			return 0;
		}
	}

	n = mk_dep_getSize(d);
	for( i=0; i<n; i++ ) {
		if( !(al = mk_al_find(sys, mk_dep_at(d, i))) ) {
			continue;
		}

#if MK_DEBUG_FIND_SOURCE_LIBS_ENABLED
		mk_dbg_outf("  found dependency on \"%s\"; investigating\n", al->lib);
#endif

		l = mk_lib_find(al->lib);
		if( !l ) {
#if MK_DEBUG_FIND_SOURCE_LIBS_ENABLED
			mk_dbg_outf("   -ignoring because did not find associated lib\n");
#endif
			continue;
		}

		/* a project does not have a dependency on itself (for linking
		 * purposes); ignore */
		if( l->proj && l->proj->libs==dst ) {
#if MK_DEBUG_FIND_SOURCE_LIBS_ENABLED
			mk_dbg_outf("   -ignoring because is current project\n");
#endif
			continue;
		}

		mk_sl_pushBack(dst, mk_al_getLib(al));
#if MK_DEBUG_FIND_SOURCE_LIBS_ENABLED
		mk_dbg_outf("   +keeping\n", mk_al_getLib(al));
#endif
	}

	mk_sl_makeUnique(dst);
	return 1;
}

/* determine whether a source file should be built */
int mk_bld_shouldCompile(const char *obj) {
	MkStat_t s, obj_s;
	size_t i, n;
	MkDep d;
	char dep[PATH_MAX];

	MK_ASSERT( obj != (const char *)0 );

	if( mk__g_flags & kMkFlag_Rebuild_Bit ) {
		return 1;
	}
	if( mk__g_flags & kMkFlag_NoCompile_Bit ) {
		return 0;
	}

	if( stat(obj, &obj_s)==-1 ) {
		return 1;
	}

	mk_com_substExt(dep, sizeof(dep), obj, ".d");

	if( stat(dep, &s)==-1 ) {
		return 1;
	}

	d = mk_dep_find(obj);
	if( !d ) {
		if( !mk_mfdep_load(dep) ) {
			return 1;
		}

		d = mk_dep_find(obj);
		if( !d ) {
			return 1;
		}
	}

	n = mk_dep_getSize(d);
	for( i=0; i<n; i++ ) {
		if( stat(mk_dep_at(d, i), &s)==-1 ) {
			return 1; /* need recompile for new dependency list; this file is
			             (potentially) missing */
		}

		if( obj_s.st_mtime <= s.st_mtime ) {
			return 1;
		}
	}

	return 0; /* no reason to rebuild */
}

/* determine whether a project should be linked */
int mk_bld_shouldLink(const char *bin, int numbuilds) {
	MkStat_t bin_s;

	MK_ASSERT( bin != (const char *)0 );

	if( mk__g_flags & kMkFlag_Rebuild_Bit ) {
		return 1;
	}
	if( mk__g_flags & kMkFlag_NoLink_Bit ) {
		return 0;
	}

	if( stat(bin, &bin_s)==-1 ) {
		return 1;
	}

	if( numbuilds != 0 ) {
		return 1;
	}

	return 0;
}

/* retrieve the compiler to use */
const char *mk_bld_getCompiler(int iscxx) {
	static char cc[128], cxx[128];
	static int didinit = 0;

	/*
	 *	TODO: Check command-line option
	 */

	if( !didinit ) {
		const char *p;
		didinit = 1;

		p = getenv("CC");
		if( p != ( const char * )0 ) {
			mk_com_strcpy(cc,sizeof(cc),p);
		} else {
#ifdef __clang__
			mk_com_strcpy(cc,sizeof(cc),"clang");
#else
			mk_com_strcpy(cc,sizeof(cc),"gcc");
#endif
		}

		p = getenv("CXX");
		if( p != ( const char * )0 ) {
			mk_com_strcpy(cxx,sizeof(cxx),p);
		} else {
			if( strcmp(cc,"clang")==0) {
				mk_com_strcpy(cxx,sizeof(cxx),"clang++");
			} else {
				char *q;
				mk_com_strcpy(cxx,sizeof(cxx),cc);
				q = strstr(cxx,"gcc");
				if( q != ( char * )0 ) {
					q[1]='+';
					q[2]='+';
				}
			}
		}
	}

	return iscxx ? cxx : cc;
}

/* retrieve the warning flags for compilation */
void mk_bld_getCFlags_warnings(char *flags, size_t nflags) {
	/*
	 *	TODO: Allow the front-end to override the warning level
	 */

	/* cl: /Wall */
	mk_com_strcat(flags, nflags, "-W -Wall -Wextra -Warray-bounds -pedantic ");
}
/* figure out the standard :: returns 1 if c++ file */
int mk_bld_getCFlags_standard(char *flags, size_t nflags, const char *filename) {
	const char *p;
	int iscplusplus;

	iscplusplus = 0;

	p = strrchr(filename, '.');
	if( !p ) {
		p = filename;
	}

	if( !strcmp(p, ".cc") || !strcmp(p, ".cxx") || !strcmp(p, ".cpp" )
	|| !strcmp(p, ".c++")) {
		if( mk__g_flags & kMkFlag_Pedantic_Bit ) {
			mk_com_strcat(flags, nflags, "-Weffc++ ");
		}

		mk_com_strcat(flags, nflags, "-std=gnu++11 ");
		iscplusplus = 1;
	} else {
#if 0
# ifdef __clang__
		mk_com_strcat(flags, nflags, "-std=gnu99 ");
# else
		mk_com_strcat(flags, nflags, "-std=gnu11 ");
# endif
#else
		mk_com_strcat(flags, nflags, "-std=gnu11 ");
#endif
	}

	return iscplusplus;
}
/* get configuration specific flags */
void mk_bld_getCFlags_config(char *flags, size_t nflags, int projarch) {
	/* optimization/debugging */
	if( mk__g_flags & kMkFlag_Release_Bit ) {
		/* cl: /DNDEBUG /Og /Ox /Oy /GL /QIfist; /Qpar? (parallel code gen.) */
		mk_com_strcat(flags, nflags, "-DNDEBUG -s -O3 -fno-strict-aliasing ");
		switch(projarch) {
		case kMkCPU_X86:
			/* cl: /arch:SSE */
			mk_com_strcat(flags, nflags, "-fomit-frame-pointer ");
			break;
		case kMkCPU_X86_64:
			/* cl: /arch:SSE2 */
			mk_com_strcat(flags, nflags, "-fomit-frame-pointer ");
			break;
		default:
			/*
			 *	NOTE: If you have architecture specific optimizations to apply
			 *        to release code, add them here.
			 */
			break;
		}
	} else {
		/* cl: /Zi /D_DEBUG /DDEBUG /D__debug__ */
		mk_com_strcat(flags, nflags, "-g3 -D_DEBUG ");
	}
}
/* get platform specific flags */
void mk_bld_getCFlags_platform(char *flags, size_t nflags, int projarch, int projsys, int usenative) {
	switch(projarch) {
	case kMkCPU_X86:
		if( usenative ) {
			mk_com_strcat(flags, nflags, "-m32 ");
		} else {
			mk_com_strcat(flags, nflags, "-m32 -march=pentium -mtune=core2 ");
		}
		break;
	case kMkCPU_X86_64:
		if( usenative ) {
			mk_com_strcat(flags, nflags, "-m64 ");
		} else {
			mk_com_strcat(flags, nflags, "-m64 -march=core2 -mtune=core2 ");
		}
		break;
	default:
		break;
	}

	if( usenative ) {
#if 0
		mk_com_strcat(flags, nflags, "-march=native -mtune=native ");
#endif
	}

	/* add a macro for the target system (some systems don't provide their own,
	   or aren't consistent/useful */
	switch(projsys) {
	case kMkOS_MSWin:
		/* cl: /DMK_MSWIN */
		mk_com_strcat(flags, nflags, "-DMK_MSWIN ");
		break;
	case kMkOS_UWP:
		/* cl: /DMK_UWP */
		mk_com_strcat(flags, nflags, "-DMK_UWP ");
		break;
	case kMkOS_Cygwin:
		/* cl: /DMK_CYGWIN */
		mk_com_strcat(flags, nflags, "-DMK_CYGWIN ");
		break;
	case kMkOS_Linux:
		/* cl: /DMK_LINUX */
		mk_com_strcat(flags, nflags, "-DMK_LINUX ");
		break;
	case kMkOS_MacOSX:
		/* cl: /DMK_MACOS */
		mk_com_strcat(flags, nflags, "-DMK_MACOSX ");
		break;
	case kMkOS_Unix:
		/* cl: /DMK_UNIX */
		mk_com_strcat(flags, nflags, "-DMK_UNIX ");
		break;
	default:
		/*
		 *	PLAN: Add Android/iOS support here too.
		 */
		MK_ASSERT_MSG(0, "project system not handled");
		break;
	}
}
/* get project type (executable, dll, ...) specific flags */
void mk_bld_getCFlags_projectType(char *flags, size_t nflags, int projtype) {
	/* add a macro for the target build type */
	switch(projtype) {
	case kMkProjTy_Application:
		/* cl: /DAPPLICATION */
		mk_com_strcat(flags, nflags, "-DAPPLICATION -DMK_APPLICATION ");
		break;
	case kMkProjTy_Program:
		/* cl: /DEXECUTABLE */
		mk_com_strcat(flags, nflags, "-DEXECUTABLE -DMK_EXECUTABLE ");
		break;
	case kMkProjTy_StaticLib:
		/* cl: /DLIBRARY */
		mk_com_strcat(flags, nflags, "-DLIBRARY -DMK_LIBRARY "
			"-DLIB -DMK_LIB ");
		break;
	case kMkProjTy_DynamicLib:
		/* cl: /DDYNAMICLIBRARY */
		mk_com_strcat(flags, nflags, "-DDYNAMICLIBRARY -DMK_DYNAMICLIBRARY "
			"-DDLL -DMK_DLL ");
			/* " -fPIC" */
		break;
	default:
		MK_ASSERT_MSG(0, "project type not handled");
		break;
	}
}
/* add all include paths */
void mk_bld_getCFlags_incDirs(char *flags, size_t nflags) {
	size_t i, n;

	mk_com_strcat(flags, nflags, mk_com_va("-I \"%s/..\" ", mk_opt_getBuildGenIncDir()));

	/* add the include search paths */
	n = mk_sl_getSize(mk__g_incdirs);
	for( i=0; i<n; i++ ) {
		/* cl: "/I \"%s\" " */
		mk_com_strcat(flags, nflags, mk_com_va("-I \"%s\" ", mk_sl_at(mk__g_incdirs, i)));
	}
}
/* add all preprocessor definitions */
void mk_bld_getCFlags_defines(char *flags, size_t nflags, MkStrList defs) {
	size_t i, n;

	/* add project definitions */
	n = mk_sl_getSize(defs);
	for( i=0; i<n; i++ ) {
		/* cl: "\"/D%s\" " */
		mk_com_strcat(flags, nflags, mk_com_va("\"-D%s\" ", mk_sl_at(defs, i)));
	}
}
/* add the input/output flags */
void mk_bld_getCFlags_unitIO(char *flags, size_t nflags, const char *obj, const char *src) {
	/*
	 *	TODO: Visual C++ and dependencies. How?
	 *	-     We can use /allincludes (or whatevertf it's called) to get them...
	 */

	/* add the remaining flags (e.g., dependencies, compile-only, etc) */
	mk_com_strcat(flags, nflags, "-MD -MP -c ");

	/* add the appropriate compilation flags */
	mk_com_strcat(flags, nflags, mk_com_va("-o \"%s\" \"%s\"", obj, src));
}

/* retrieve the flags for compiling a particular source file */
const char *mk_bld_getCFlags(MkProject proj, const char *obj, const char *src) {
	static char flags[16384];

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( obj != (const char *)0 );
	MK_ASSERT( src != (const char *)0 );

	flags[0] = '\0';

	mk_bld_getCFlags_warnings(flags, sizeof(flags));
	mk_bld_getCFlags_standard(flags, sizeof(flags), src);
	mk_bld_getCFlags_config(flags, sizeof(flags), proj->arch);
	mk_bld_getCFlags_platform(flags, sizeof(flags), proj->arch, proj->sys, 0);
	mk_bld_getCFlags_projectType(flags, sizeof(flags), proj->type);
	mk_bld_getCFlags_incDirs(flags, sizeof(flags));
	mk_bld_getCFlags_defines(flags, sizeof(flags), proj->defs);
	mk_bld_getCFlags_unitIO(flags, sizeof(flags), obj, src);

	return flags;
}

/* retrieve the dependencies of a project and its subprojects */
void mk_bld_getDeps_r(MkProject proj, MkStrList deparray) {
	static size_t nest = 0;
	static char spaces[512] = { '\0', };
	size_t i, n, j, m;
	const char *libname;
	MkLib lib;
	char src[PATH_MAX], obj[PATH_MAX], dep[PATH_MAX];

	if( !spaces[0] ) {
		for( i=0; i<sizeof(spaces) - 1; i++ ) {
			spaces[i] = ' ';
		}

		spaces[i] = '\0';
	}

	nest++;

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( deparray != (MkStrList)0 );

	/* ensure the project is up to date */
	mk_dbg_outf(" **** mk_bld_getDeps_r \"%s\" ****\n", proj->name);
	n = mk_prj_numSourceFiles(proj);
	for( i=0; i<n; i++ ) {
		mk_com_relPathCWD(src, sizeof(src), mk_prj_sourceFileAt(proj, i));
		mk_bld_getObjName(proj, obj, sizeof(obj), src);
		mk_com_substExt(dep, sizeof(dep), obj, ".d");
		mk_dbg_outf(" **** src:\"%s\" obj:\"%s\" dep:\"%s\" ****\n",
			src, obj, dep);
		if( !mk_bld_findSourceLibs(proj->libs, proj->sys, obj, dep) ) {
			mk_dbg_outf("mk_bld_findSourceLibs(\"%s\"->libs, %i, \"%s\", \"%s\") "
				"call failed\n", proj->name, proj->sys, obj, dep);
			continue;
		}
	}

	/* check the libraries */
	n = mk_sl_getSize(proj->libs);
	for( i=0; i<n; i++ ) {
		libname = mk_sl_at(proj->libs, i);
		if( !libname ) {
			continue;
		}

		mk_dbg_outf("%.*s[dep] %s\n", (nest - 1)*2, spaces, libname);

		lib = mk_lib_find(libname);
		if( !lib || !lib->proj || lib->proj==proj ) {
			continue;
		}

		m = mk_sl_getSize(deparray);
		for( j=0; j<m; j++ ) {
			if( !strcmp(mk_sl_at(deparray, j), libname) ) {
				break;
			}
		}

		if( j < m ) {
			continue;
		}

		mk_sl_pushBack(deparray, mk_sl_at(proj->libs, i));
		mk_bld_getDeps_r(lib->proj, deparray);
	}

	nest--;
}

/* determine whether a library depends upon another library */
int mk_bld_doesLibDependOnLib(MkLib mainlib, MkLib deplib) {
	static size_t nest = 0;
	static char spaces[512] = { '\0', };
	size_t i, n;
	MkLib libs[1024];

	MK_ASSERT( mainlib != (MkLib)0 );
	MK_ASSERT( deplib != (MkLib)0 );

	if( !spaces[0] ) {
		for( i=0; i<sizeof(spaces) - 1; i++ )
			spaces[i] = ' ';

		spaces[i] = '\0';
	}

	nest++;

	mk_dbg_outf("%.*sdoeslibdependonlib_r(\"%s\",\"%s\")\n",
		(nest - 1)*2, spaces, mainlib->name, deplib->name);

	if( nest > 10 ) {
		nest--;
		return 0;
	}

#if 0
	printf("%s%p %s%p\n", _WIN32 ? "0x" : "",
		(void *)mainlib, _WIN32 ? "0x" : "", (void *)deplib);
	printf("%s %s\n", mainlib->name, deplib->name);
#endif

	if( !mainlib->proj || mainlib==deplib ) {
		nest--;
		return 0;
	}

	mk_sl_makeUnique(mainlib->proj->libs);
	n = mk_sl_getSize(mainlib->proj->libs);
	if( n > sizeof(libs)/sizeof(libs[0]) ) {
		mk_log_error(__FILE__,__LINE__,__func__, "n > 1024; too many libraries");
		exit(EXIT_FAILURE);
	}

	/* first check; any immediate dependencies? */
	for( i=0; i<n; i++ ) {
		mk_dbg_outf("%.*s%.2u of %u \"%s\"\n", (nest - 1)*2, spaces,
			i + 1, n, mk_sl_at(mainlib->proj->libs, i));
		libs[i] = mk_lib_find(mk_sl_at(mainlib->proj->libs, i));

		if( libs[i]==deplib ) {
			mk_dbg_outf("%.*s\"%s\" depends on \"%s\"\n",
				(nest - 1)*2, spaces, libs[i]->name, deplib->name);
			nest--;
			return 1;
		}
	}

	/* second check; recursive check */
	for( i=0; i<n; i++ ) {
		if( !mk_bld_doesLibDependOnLib(libs[i], deplib) ) {
			continue;
		}

		nest--;
		return 1;
	}

	/* nope; this lib does not depend on that lib */
	nest--;
	return 0;
}

/* determine which library should come first */
int mk_bld__cmpLibs(const void *a, const void *b) {
#if 0
	int r;
	printf("About to compare \"%s\" to \"%s\"\n",
		(*(MkLib *)a)->name, (*(MkLib *)b)->name);
	fflush(stdout);
	r = mk_bld_doesLibDependOnLib(*(MkLib *)a, *(MkLib *)b);
	printf("mk_bld__cmpLibs \"%s\" \"%s\" -> %i\n",
		(*(MkLib *)a)->name, (*(MkLib *)b)->name, r);
	fflush(stdout);
	return 1 - r;
#else
	return 1 - mk_bld_doesLibDependOnLib(*(MkLib *)a, *(MkLib *)b);
#endif
}
void mk_bld_sortDeps(MkStrList deparray) {
	size_t i, n;
	MkLib libs[1024];

	MK_ASSERT( deparray != (MkStrList)0 );

	/* remove duplicates from the array */
	mk_dbg_outf("Dependency Array (Before Unique):\n");
	mk_sl_makeUnique(deparray);

	mk_dbg_outf("Dependency Array:\n");
	mk_sl_debugPrint(deparray);

	/* too many libraries? */
	n = mk_sl_getSize(deparray);
	if( n > sizeof(libs)/sizeof(libs[0]) ) {
		mk_log_error(__FILE__,__LINE__,__func__, "Too many libraries");
		exit(EXIT_FAILURE);
	}

	/* fill the array, then sort it */
	for( i=0; i<n; i++ ) {
		libs[i] = mk_lib_find(mk_sl_at(deparray, i));
		if( !libs[i] ) {
			mk_log_fatalError(mk_com_va("Couldn't find library ^E\"%s\"^&",
				mk_sl_at(deparray, i)));
		}
	}

	mk_dbg_outf("About to sort...\n");
	qsort((void *)libs, n, sizeof(MkLib), mk_bld__cmpLibs);
	mk_dbg_outf("Sorted!\n");

	/* set the array elements, sorted */
	for( i=0; i<n; i++ ) {
		mk_sl_set(deparray, i, mk_lib_getName(libs[i]));
	}

	mk_dbg_outf("Sorted Dependency Array:\n");
	mk_sl_debugPrint(deparray);
}

/* get the linker flags for linking dependencies of a project */
const char *mk_bld_getProjDepLinkFlags(MkProject proj) {
	static char buf[65536];
	MkStrList deps;
	size_t i, n;
	MkLib lib;

	deps = mk_sl_new();

	mk_bld_getDeps_r(proj, deps);
	mk_dbg_outf("Project: \"%s\"\n", proj->name);
	mk_bld_sortDeps(deps);

	buf[0] = '\0';

	n = mk_sl_getSize(deps);
	for( i=0; i<n; i++ ) {
		lib = mk_lib_find(mk_sl_at(deps, i));
		if( !lib ) {
			continue;
		}

		if( !lib->flags[proj->sys] ) {
			continue;
		}

		if( !lib->flags[proj->sys][0] ) {
			continue;
		}

		mk_com_strcat(buf, sizeof(buf), mk_com_va("%s ", lib->flags[proj->sys]));
	}

	mk_sl_delete(deps);
	return buf;
}

/* construct a list of dependent libraries */
/*
 *	???: Dependent on what? Libraries the project passed in relies on? Libraries
 *	     other projects would need if they were using this?
 *
 *	NOTE: It appears this function is used by mk_bld_getLFlags() to determine how to
 *	      link with a given project.
 */
void mk_bld_getLibs(MkProject proj, char *dst, size_t n) {
	const char *name, *suffix = "";
	MkProject p;
	char bin[PATH_MAX];
	int type;

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( dst != (char *)0 );
	MK_ASSERT( n > 1 );

	/*
	 *	HACK: "cannot find -lblah" is really annoying when you don't want the
	 *	      project
	 */
	if( !mk_prj_isTarget(proj) ) {
		return;
	}

	type = mk_prj_getType(proj);
	name = mk_prj_getName(proj);

	suffix = mk__g_flags & kMkFlag_Release_Bit ? "" : mk_opt_getDebugSuffix();

	if( type==kMkProjTy_StaticLib ) {
		/*
		 *	???: Why is this here? What is it doing?
		 *	NOTE: This code did fix a bug... but which bug? And why?
		 *
		 *	???: Was this to make sure that a dependent project with no source
		 *	     files wouldn't be linked in?
		 */
		if( (~proj->config & kMkProjCfg_Linking_Bit )
		&& mk_prj_numSourceFiles(proj) > 0) {
			mk_com_strcat(dst, n, mk_com_va("-l%s%s ", name, suffix));
		}

		/* NOTE: these should be linked after the library itself is linked in */
		for( p=mk_prj_head(proj); p; p=mk_prj_next(p) )
			mk_bld_getLibs(p, dst, n);
	} else if(type==kMkProjTy_DynamicLib) {
		if( (~proj->config & kMkProjCfg_Linking_Bit )
		&& mk_prj_numSourceFiles(proj) > 0) {
			mk_bld_getBinName(proj, bin, sizeof(bin));
			mk_com_strcat(dst, n, mk_com_va("\"%s\" ", bin));
			/*
			mk_com_strcat(dst, n, mk_com_va("-L \"%s\" ", mk_prj_getOutPath(proj)));
			mk_com_strcat(dst, n, mk_com_va("-l%s%s.dll ", name, suffix));
			*/
		}
	}
}

/* retrieve the flags for linking a project */
const char *mk_bld_getLFlags(MkProject proj, const char *bin, MkStrList objs) {
	static char flags[32768];
	static char libs[16384], libs_stripped[16384];
	const char *pszStaticFlags;
	MkProject p;
	size_t i, n;

	(void)p;
	(void)libs_stripped;

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( bin != (const char *)0 );
	MK_ASSERT( objs != (MkStrList)0 );

	flags[0] = '\0';
	libs[0] = '\0';

	pszStaticFlags = "";
	if( proj->sys == kMkOS_MSWin || proj->sys == kMkOS_UWP ) {
		pszStaticFlags = "-static -static-libgcc ";
	}

	switch(mk_prj_getType(proj)) {
	case kMkProjTy_Application:
		if( mk__g_flags & kMkFlag_Release_Bit ) {
			mk_com_strcat(flags, sizeof(flags), "-s ");
		}
		if( proj->sys == kMkOS_MSWin && ( mk__g_flags & kMkFlag_Release_Bit ) ) {
			mk_com_strcat(flags, sizeof(flags),
				mk_com_va("%s-Wl,subsystem,windows -o \"%s\" ", pszStaticFlags, bin));
		} else {
			mk_com_strcat(flags, sizeof(flags),
				mk_com_va("%s-o \"%s\" ", pszStaticFlags, bin));
		}
		break;
	case kMkProjTy_Program:
		if( mk__g_flags & kMkFlag_Release_Bit ) {
			mk_com_strcat(flags, sizeof(flags), "-s ");
		}
		mk_com_strcat(flags, sizeof(flags), mk_com_va("%s-o \"%s\" ",
			pszStaticFlags, bin));
		break;
	case kMkProjTy_StaticLib:
		mk_com_strcat(flags, sizeof(flags), mk_com_va("cr \"%s\" ", bin));
		break;
	case kMkProjTy_DynamicLib:
		mk_com_strcat(flags, sizeof(flags),
			mk_com_va("%s-shared -o \"%s\" ", pszStaticFlags, bin));
		break;
	default:
		MK_ASSERT_MSG(0, "unhandled project type");
		break;
	}

	if( mk_prj_getType(proj)!=kMkProjTy_StaticLib ) {
		n = mk_sl_getSize(mk__g_libdirs);
		for( i=0; i<n; i++ ) {
			mk_com_strcat(flags, sizeof(flags), mk_com_va("-L \"%s\" ",
				mk_sl_at(mk__g_libdirs, i)));
		}
	}

	n = mk_sl_getSize(objs);
	for( i=0; i<n; i++ ) {
		mk_com_strcat(flags, sizeof(flags), mk_com_va("\"%s\" ", mk_sl_at(objs, i)));
	}

	if( mk_prj_getType(proj)!=kMkProjTy_StaticLib ) {
		proj->config |= kMkProjCfg_Linking_Bit;

#if 0
		/*
		 *	NOTE: This should no longer be necessary. The "lib" system used now
		 *	      should take care of this. (Dependencies found by the header
		 *	      files of each project.)
		 */
		for( p=mk__g_lib_proj_head; p; p=p->lib_next )
			mk_bld_getLibs(p, libs, sizeof(libs));

		p = mk_prj_getParent(proj) ? mk_prj_head(mk_prj_getParent(proj))
			: mk_prj_rootHead();
		while( p != (MkProject)0 ) {
			MK_ASSERT( p != (MkProject)0 );
			mk_bld_getLibs(p, libs, sizeof(libs));
			p = mk_prj_next(p);
		}
#else
		mk_com_strcpy(libs, sizeof(libs), mk_bld_getProjDepLinkFlags(proj));
#endif

		proj->config &= ~kMkProjCfg_Linking_Bit;

#if 0
		mk_com_stripArgs(libs_stripped, sizeof(libs_stripped), libs);
		mk_com_strcat(flags, sizeof(flags), libs_stripped);
#else
		mk_com_strcat(flags, sizeof(flags), libs);
#endif

		switch(proj->sys) {
		case kMkOS_MSWin:
			if( mk_prj_getType(proj)==kMkProjTy_DynamicLib ) {
				mk_com_strcat(flags, sizeof(flags),
					/* NOTE: we don't use the import library; it's pointless */
					mk_com_va(/*"\"-Wl,--out-implib=%s%s.a\" "*/
					   "-Wl,--export-all-symbols "
					   "-Wl,--enable-auto-import "/*, bin,
					   ~mk__g_flags & kMkFlag_Release_Bit ? mk_opt_getDebugSuffix() : ""*/));
			}

			/*mk_com_strcat(flags, sizeof(flags), "-lkernel32 -luser32 -lgdi32 "
				"-lshell32 -lole32 -lopengl32 -lmsimg32");*/
			break;
		case kMkOS_Linux:
			/*mk_com_strcat(flags, sizeof(flags), "-lGL");*/
			break;
		case kMkOS_MacOSX:
			/*mk_com_strcat(flags, sizeof(flags), "-lobjc -framework Cocoa "
				"-framework OpenGL");*/
			break;
		default:
			/*
			 *	NOTE: Add OS specific link flags here.
			 */
			break;
		}

		mk_com_strcat(flags, sizeof(flags), mk_prj_getLinkFlags(proj));
	}
#if 0
	else {
		static char extras[32768];

		proj->config |= kMkProjCfg_Linking_Bit;

		p = mk_prj_getParent(proj) ? mk_prj_head(mk_prj_getParent(proj))
			: mk_prj_rootHead();
		for( p=p; p!=(MkProject)0; p=mk_prj_next(p) ) {
			if( p==proj )
				continue;

			extras[0] = '\0';
			mk_prj_completeExtraLibs(p, extras, sizeof(extras));
			mk_prj_appendExtraLibs(proj, extras);
		}
		while( p != (MkProject)0 ) {
			MK_ASSERT( p != (MkProject)0 );
			mk_bld_getLibs(p, libs, sizeof(libs));
			p = mk_prj_next(p);
		}
	}
#endif

	return flags;
}

/* find the name of an object file for a given source file */
void mk_bld_getObjName(MkProject proj, char *obj, size_t n, const char *src) {
	const char *p = "";

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( obj != (char *)0 );
	MK_ASSERT( n > 1 );
	MK_ASSERT( src != (const char *)0 );

	switch(mk_prj_getType(proj)) {
	case kMkProjTy_Application:
		p = "appexec";
		break;
	case kMkProjTy_Program:
		p = "exec";
		break;
	case kMkProjTy_StaticLib:
		p = "lib";
		break;
	case kMkProjTy_DynamicLib:
		p = "dylib";
		break;
	default:
		MK_ASSERT_MSG(0, "project type is invalid");
		break;
	}

	mk_com_substExt(obj, n, mk_com_va("%s/%s/%s/%s",
		mk_opt_getObjdirBase(),
		mk_opt_getConfigName(), p, src), ".o");
}

/* find the binary name of a target */
void mk_bld_getBinName(MkProject proj, char *bin, size_t n) {
	const char *dir, *prefix, *name, *symbol, *suffix = "";
	int type;
	int sys;

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( bin != (char *)0 ? n > 1 : 1 );

	if( !proj->binname || bin!=(char *)0 ) {
		type = mk_prj_getType(proj);
		sys = proj->sys;

		/*dir = type==kMkProjTy_StaticLib ? "lib/" : "bin/";*/
		dir = mk_prj_getOutPath(proj);
		MK_ASSERT( dir != (const char *)0 );
		prefix = "";
		name = mk_prj_getName(proj);
		symbol = mk__g_flags & kMkFlag_Release_Bit ? "" : mk_opt_getDebugSuffix();

		switch(type) {
		case kMkProjTy_Application:
		case kMkProjTy_Program:
			if( sys==kMkOS_MSWin || sys==kMkOS_UWP || sys==kMkOS_Cygwin ) {
				suffix = ".exe";
			}
			break;
		case kMkProjTy_StaticLib:
			prefix = "lib";
			suffix = ".a";
			break;
		case kMkProjTy_DynamicLib:
			if( sys!=kMkOS_MSWin && sys!=kMkOS_UWP ) {
				prefix = "lib";
			}
			if( sys==kMkOS_MSWin || sys==kMkOS_UWP || sys==kMkOS_Cygwin ) {
				suffix = ".dll";
			} else if(sys==kMkOS_Linux || sys==kMkOS_Unix) {
				suffix = ".so";
			} else if(sys==kMkOS_MacOSX) {
				suffix = ".dylib";
			}
			break;
		default:
			MK_ASSERT_MSG(0, "unhandled project type");
			break;
		}

		proj->binname = mk_com_dup(proj->binname, mk_com_va("%s%s%s%s%s",
			dir, prefix, name, symbol, suffix));
	}

	if( bin != (char *)0 ) {
		mk_com_strcpy(bin, n, proj->binname);
	}
}

/* compile and run a unit test */
void mk_bld_unitTest(MkProject proj, const char *src)
{
	static char flags[32768];
	static char libs[16384], libs_stripped[16384];
	const char *tool, *libname, *libf;
	const char *cc, *cxx;
	MkProject chld;
	size_t i, j, n;
	MkLib lib;
	char out[PATH_MAX], dep[PATH_MAX], projbin[PATH_MAX];

	MK_ASSERT( proj != (MkProject)0 );
	MK_ASSERT( src != (const char *)0 );

	(void)chld;
	(void)libs_stripped;

	cc = mk_bld_getCompiler(0);
	cxx = mk_bld_getCompiler(1);

	/* determine the name for the unit test's executable */
	mk_com_strcpy(out, sizeof(out), mk_com_va("%s/%s/%s/test/",
		mk_opt_getObjdirBase(),
		mk_opt_getConfigName(),
		mk_prj_getName(proj)));
	mk_fs_makeDirs(out);
	mk_com_substExt(out, sizeof(out), mk_com_va("%s%s", out, strrchr(src, '/') + 1),
		".test");

#if MK_WINDOWS_ENABLED
	mk_com_strcat(out, sizeof(out), ".exe");
	mk_com_substExt(dep, sizeof(dep), out, ".d");
#else
	mk_com_strcpy(dep, sizeof(dep), out);
	mk_com_strcat(dep, sizeof(dep), ".d");
#endif

	flags[0] = '\0';

	mk_bld_getCFlags_warnings(flags, sizeof(flags));
	if( mk_bld_getCFlags_standard(flags, sizeof(flags), src) ) {
		tool = cxx;
	} else {
		tool = (proj->config & kMkProjCfg_UsesCxx_Bit) ? cxx : cc;
	}
	mk_bld_getCFlags_config(flags, sizeof(flags), proj->arch);
	mk_bld_getCFlags_platform(flags, sizeof(flags), proj->arch, proj->sys, 1);
	mk_com_strcat(flags, sizeof(flags), "-DTEST -DMK_TEST ");
	mk_com_strcat(flags, sizeof(flags), "-DEXECUTABLE -DMK_EXECUTABLE ");
	mk_bld_getCFlags_incDirs(flags, sizeof(flags));
	mk_bld_getCFlags_defines(flags, sizeof(flags), proj->defs);

	/* retrieve all of the library directories */
	n = mk_sl_getSize(mk__g_libdirs);
	for( i=0; i<n; i++ ) {
		mk_com_strcat(flags, sizeof(flags), mk_com_va("-L \"%s\" ",
			mk_sl_at(mk__g_libdirs, i)));
	}

	/* determine compilation flags: output and source */
	mk_com_strcat(flags, sizeof(flags), mk_com_va("-o \"%s\" \"%s\" ", out, src));

	/* link to the project directly if it's a library (static or dynamic) */
	switch(mk_prj_getType(proj))
	{
	case kMkProjTy_StaticLib:
	case kMkProjTy_DynamicLib:
		mk_bld_getBinName(proj, projbin, sizeof(projbin));
		mk_com_strcat(flags, sizeof(flags), mk_com_va("\"%s\" ", projbin));
		break;
	}

	/* now include all of the necessary libs we depend on (we're assuming these
	   are the same libs the project itself depends on)

	   FIXME: this won't work if we try using a lib that the project doesn't
	          depend on here... perhaps the solution is to make unit tests their
			  own projects? */
	libs[0] = '\0';

#if 0
	/* first pass: grab all sibling projects */
	chld = mk_prj_getParent(proj) ? mk_prj_head(mk_prj_getParent(proj))
	       : mk_prj_rootHead();

	for( chld=chld; chld != (MkProject)0; chld=mk_prj_next(chld) ) {
		MK_ASSERT( chld != (MkProject)0 );

		if( chld==proj )
			continue;

		mk_bld_getLibs(chld, libs, sizeof(libs));
		chld = mk_prj_next(chld);
	}

	/* second pass: grab all of our direct dependencies */
	for( chld=mk_prj_head(proj); chld != (MkProject)0; chld=mk_prj_next(chld) )
	{
		MK_ASSERT( chld != (MkProject)0 );
		mk_bld_getLibs(chld, libs, sizeof(libs));
		chld = mk_prj_next(chld);
	}
#endif

	/*
	 *	TODO: Use the proper dependency checking method.
	 */

	/* grab all of the directly dependent libraries */
	n = mk_sl_getSize(proj->libs);
	for( i=0; i<n; i++ ) {
		/* if we have a null pointer in the array, skip it */
		if( !(libname = mk_sl_at(proj->libs, i)) ) {
			continue;
		}

		/* check for a mk_com_dup library */
		for( j=0; j<i; j++ ) {
			/* skip null pointers... */
			if( !mk_sl_at(proj->libs, j) ) {
				continue;
			}

			/* is there a match? (did we already include it, that is) */
			if( !strcmp(mk_sl_at(proj->libs, j), libname) ) {
				break;
			}
		}

		/* found a duplicate, don't include again */
		if( j != i ) {
			continue;
		}

		/* try to find the library handle from the name in the array */
		if( !(lib = mk_lib_find(libname)) ) {
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_YELLOW, "WARN");
			mk_sys_uncoloredPuts(kMkSIO_Err, ": ", 2);
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_PURPLE, src);
			mk_sys_uncoloredPuts(kMkSIO_Err, ": couldn't find library \"", 0);
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_BROWN, libname);
			mk_sys_uncoloredPuts(kMkSIO_Err, "\"\n", 2);
			continue;
		}

		/* retrieve the flags... if we have a non-empty string, add them to the
		   linker flags */
		if( (libf=mk_lib_getFlags(lib, proj->sys))!=(const char *)0 && *libf!='\0' ) {
			mk_com_strcat(libs, sizeof(libs), mk_com_va("%s ", libf));
		}
	}

	/* link in all libraries */
#if 0
	/*
	 *	NOTE: This should no longer be necessary. Using the mk_al_autolink system.
	 */
	for( chld=mk__g_lib_proj_head; chld; chld=chld->lib_next )
		mk_bld_getLibs(chld, libs, sizeof(libs));

# if 0
	mk_com_stripArgs(libs_stripped, sizeof(libs_stripped), libs);
	mk_com_strcat(flags, sizeof(flags), libs_stripped);
# else
	mk_com_strcat(flags, sizeof(flags), libs);
# endif
#endif

	/* queue unit tests */
	mk_sl_pushBack(mk__g_unitTestCompiles, mk_com_va("%s %s", tool, flags));
	mk_sl_pushBack(mk__g_unitTestRuns, out);

	mk_com_relPathCWD(out, sizeof(out), src);
	mk_sl_pushBack(mk__g_unitTestNames, out);
}

/* perform each unit test */
void mk_bld_runTests(void) {
	static size_t buffer[65536];
	MkStrList failedtests;
	size_t i, n;
	int e;

	MK_ASSERT( mk_sl_getSize(mk__g_unitTestCompiles)==mk_sl_getSize(mk__g_unitTestRuns) );
	MK_ASSERT( mk_sl_getSize(mk__g_unitTestRuns)==mk_sl_getSize(mk__g_unitTestNames) );

	n = mk_sl_getSize(mk__g_unitTestCompiles);
	if( !n ) {
		return;
	}
	MK_ASSERT( n <= sizeof(buffer)/sizeof(buffer[0]) );
	mk_sl_orderedSort(mk__g_unitTestNames, buffer, sizeof(buffer)/sizeof(buffer[0]));
	mk_sl_indexedSort(mk__g_unitTestCompiles, buffer, n);
	mk_sl_indexedSort(mk__g_unitTestRuns, buffer, n);

	failedtests = mk_sl_new();

	for( i=0; i<n; i++ ) {
		/* compile the unit test */
		if( mk_com_shellf("%s", mk_sl_at(mk__g_unitTestCompiles, i)) != 0 )
		{
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_LIGHT_RED, "KO");
			mk_sys_uncoloredPuts(kMkSIO_Err, ": ", 2);
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_PURPLE, mk_sl_at(mk__g_unitTestNames, i));
			mk_sys_uncoloredPuts(kMkSIO_Err, " (did not build)\n", 0);
			mk_sl_pushBack(failedtests, mk_sl_at(mk__g_unitTestNames, i));
			continue;
		}

		/* run the unit test */
		e = mk_com_shellf("%s", mk_sl_at(mk__g_unitTestRuns, i));
		if( e != 0 ) {
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_LIGHT_RED, "KO");
			mk_sys_uncoloredPuts(kMkSIO_Err, ": ", 2);
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_RED, mk_sl_at(mk__g_unitTestNames, i));
			mk_sys_printf(kMkSIO_Err, " (returned " MK_S_COLOR_WHITE "%i"
				MK_S_COLOR_RESTORE ")\n", e);
			mk_sl_pushBack(failedtests, mk_sl_at(mk__g_unitTestRuns, i));
		} else {
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_LIGHT_GREEN, "OK");
			mk_sys_uncoloredPuts(kMkSIO_Err, ": ", 2);
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_WHITE, mk_sl_at(mk__g_unitTestNames, i));
			mk_sys_uncoloredPuts(kMkSIO_Err, "\n", 1);
		}
	}

	n = mk_sl_getSize(failedtests);
	if( n > 0 ) {
		mk_sys_printStr(kMkSIO_Err, MK_COLOR_RED, "\n  *** ");
		mk_sys_printStr(kMkSIO_Err, MK_COLOR_WHITE, mk_com_va("%u", (unsigned int)n));
		mk_sys_printStr(kMkSIO_Err, MK_COLOR_LIGHT_RED, n==1 ? " FAILURE" : " FAILURES");
		mk_sys_printStr(kMkSIO_Err, MK_COLOR_RED, " ***\n  ");
		for( i=0; i<n; i++ ) {
			mk_sys_printStr(kMkSIO_Err, MK_COLOR_YELLOW, mk_sl_at(failedtests, i));
			mk_sys_uncoloredPuts(kMkSIO_Err, "\n  ", 3);
		}
		mk_sys_printStr(kMkSIO_Err, MK_COLOR_RED, "\n");
	}

	mk_sl_delete(failedtests);
}

/* sort the projects in a list */
void mk_bld_sortProjects(struct MkProject_s *proj) {
	struct MkProject_s **head, **tail;
	struct MkProject_s *p, *next;
	int numsorts;

	head = proj ? &proj->head : &mk__g_proj_head;
	tail = proj ? &proj->tail : &mk__g_proj_tail;

	do {
		numsorts = 0;

		for( p=*head; p; p=next ) {
			if( !(next = p->next) ) {
				break;
			}

			if( p->type > next->type ) {
				numsorts++;

				if( p==*head ) {
					*head = next;
				}
				if( next==*tail ) {
					*tail = p;
				}

				if( p->prev ) {
					p->prev->next = next;
				}
				if( next->next ) {
					next->next->prev = p;
				}

				next->prev = p->prev;
				p->prev = next;
				p->next = next->next;
				next->next = p;

				next = p;
			}
		}
	} while( numsorts > 0 );
}

/* set a projects dependents to be relinked */
void mk_bld_relinkDeps(MkProject proj) {
	MkProject prnt, next;

	MK_ASSERT( proj != (MkProject)0 );

	for( prnt=proj->prnt; prnt; prnt=prnt->prnt ) {
		prnt->config |= kMkProjCfg_NeedRelink_Bit;
	}

	if( proj->type==kMkProjTy_Program
	|| proj->type==kMkProjTy_Application ) {
		return;
	}

	for( next=proj->next; next; next=next->next ) {
		if( proj->type > next->type ) {
			break;
		}

		next->config |= kMkProjCfg_NeedRelink_Bit;
		mk_bld_relinkDeps(next);
	}
}

/* build a project */
int mk_bld_makeProject(MkProject proj) {
	const char *src, *lnk, *tool, *cxx, *cc;
	MkProject chld;
	MkStrList objs;
	size_t cwd_l;
	size_t i, n;
	char cwd[PATH_MAX], obj[PATH_MAX], bin[PATH_MAX];
	int numbuilds;

	/* build the child projects */
	mk_bld_sortProjects(proj);
	for( chld=mk_prj_head(proj); chld; chld=mk_prj_next(chld) ) {
		if( !mk_bld_makeProject(chld) ) {
			return 0;
		}
	}

	/* if this project isn't targetted, just return now */
	if( !mk_prj_isTarget(proj) )
		return 1;

	/* retrieve the current working directory */
	if( getcwd(cwd, sizeof(cwd))==(char *)0 ) {
		mk_log_fatalError("getcwd() failed");
	}

	cwd_l = mk_com_strlen(cwd);

	/*
	 *	NOTE: This appears to be a hack.
	 */
	if( !mk_prj_numSourceFiles(proj )
	 && mk_prj_getType(proj)!=kMkProjTy_DynamicLib) {
		mk_log_errorMsg(mk_com_va("project ^E'%s'^& has no source files!",
			mk_prj_getName(proj)));
		return 1;
	}

	cc = mk_bld_getCompiler(0);
	cxx = mk_bld_getCompiler(1);
	tool = proj->config & kMkProjCfg_UsesCxx_Bit ? cxx : cc;

	/* make the object directories */
	mk_prjfs_makeObjDirs(proj);

	/* store each object file */
	objs = mk_sl_new();

	/* run through each source file */
	numbuilds = 0;
	n = mk_prj_numSourceFiles(proj);
	for( i=0; i<n; i++ ) {
		src = mk_prj_sourceFileAt(proj, i);
		mk_bld_getObjName(proj, obj, sizeof(obj), &src[cwd_l+1]);
		mk_sl_pushBack(objs, obj);

		if( mk_bld_shouldCompile(obj) ) {
			if (mk_com_shellf("%s %s", tool,
			mk_bld_getCFlags(proj, obj, &src[cwd_l+1]))) {
				mk_sl_delete(objs);
				return 0;
			}
			numbuilds++;
		}

		mk_com_substExt(bin, sizeof(bin), obj, ".d");
		if( (~mk__g_flags & kMkFlag_NoLink_Bit)
		 && !mk_bld_findSourceLibs(proj->libs, proj->sys, obj, bin ) ) {
		 	mk_log_errorMsg("call to mk_bld_findSourceLibs() failed");
			return 0;
		}
	}

	/* link the project's object files together */
	mk_bld_getBinName(proj, bin, sizeof(bin));
	/*printf("bin: %s\n", bin);*/
	if( ( proj->config & kMkProjCfg_NeedRelink_Bit )
	 || mk_bld_shouldLink(bin, numbuilds) ) {
		mk_fs_makeDirs(mk_prj_getOutPath(proj));
		mk_sl_makeUnique(proj->libs);
		mk_prj_calcLibFlags(proj);

		/* find the libraries this project needs */
		lnk = mk_prj_getType(proj)==kMkProjTy_StaticLib ? "ar" : tool;
		if( mk_com_shellf("%s %s", lnk, mk_bld_getLFlags(proj, bin, objs)) ) {
			mk_sl_delete(objs);
			return 0;
		}

		/* dependent projects need to be rebuilt */
		mk_bld_relinkDeps(proj);
	} else if( ~mk__g_flags & kMkFlag_FullClean_Bit ) {
		mk_prj_calcDeps(proj);
	}

	/* unit testing */
	if( mk__g_flags & kMkFlag_Test_Bit )
	{
		n = mk_prj_numTestSourceFiles(proj);
		for( i=0; i<n; i++ ) {
			mk_bld_unitTest(proj, mk_prj_testSourceFileAt(proj, i));
		}
	}

	/* clean (removes temporaries) -- only if not rebuilding */
	if( mk__g_flags & kMkFlag_LightClean_Bit ) {
		n = mk_sl_getSize(objs);
		for( i=0; i<n; i++ ) {
			mk_com_substExt(obj, sizeof(obj), mk_sl_at(objs, i), ".d");
			mk_fs_remove(mk_sl_at(objs, i)); /* object (.o) */
			mk_fs_remove(obj); /* dependency (.d) */
		}

		if( mk__g_flags & kMkFlag_NoLink_Bit ) {
			mk_fs_remove(bin);
		}
	}

	mk_sl_delete(objs);

	return 1;
}

/* build all the projects */
int mk_bld_makeAllProjects(void) {
	MkProject proj;

	if( mk__g_flags & kMkFlag_FullClean_Bit ) {
		mk_fs_remove( mk_opt_getObjdirBase() );
	}

	mk_bld_sortProjects((struct MkProject_s *)0);

	mk_git_generateInfo();

	for( proj=mk_prj_rootHead(); proj; proj=mk_prj_next(proj) ) {
		if( !mk_bld_makeProject(proj) ) {
			return 0;
		}
	}

	mk_bld_runTests();

	return 1;
}

/*
 *	========================================================================
 *	MAIN
 *	========================================================================
 */

void mk_front_pushSrcDir(const char *srcdir) {
	if( !mk_fs_isDir(srcdir) ) {
		mk_log_errorMsg(mk_com_va("^F%s^&", srcdir));
		return;
	}

	mk_sl_pushBack(mk__g_srcdirs, srcdir);
}
void mk_front_pushIncDir(const char *incdir) {
	char inc[PATH_MAX];

	if( !mk_fs_isDir(incdir) ) {
		mk_log_errorMsg(mk_com_va("^F%s^&", incdir));
		return;
	}

	mk_com_relPathCWD(inc, sizeof(inc), incdir);
	mk_sl_pushBack(mk__g_incdirs, inc);
}
void mk_front_pushLibDir(const char *libdir) {
#if 0
	/*
	 *	XXX: If project has not yet been built, the lib dir might not exist.
	 */
	if( !mk_fs_isDir(libdir) ) {
		mk_log_errorMsg(mk_com_va("^F%s^&", libdir));
		return;
	}
#endif

	mk_sl_pushBack(mk__g_libdirs, libdir);
}
void mk_front_pushPkgDir(const char *pkgdir) {
	if( !mk_fs_isDir(pkgdir) ) {
		mk_log_errorMsg(mk_com_va("^F%s^&", pkgdir));
		return;
	}

	mk_sl_pushBack(mk__g_pkgdirs, pkgdir);
}
void mk_front_pushToolDir(const char *tooldir) {
	if( !mk_fs_isDir(tooldir) ) {
		mk_log_errorMsg(mk_com_va("^F%s^&", tooldir));
		return;
	}

	mk_sl_pushBack(mk__g_tooldirs, tooldir);
}
void mk_front_pushDynamicLibsDir(const char *dllsdir) {
	if( !mk_fs_isDir(dllsdir) ) {
		mk_log_errorMsg(mk_com_va("^F%s^&", dllsdir));
		return;
	}

	mk_sl_pushBack(mk__g_dllsdirs, dllsdir);
}

void mk_fs_unwindDirs(void) {
	while( mk_sl_getSize(mk__g_fs_dirstack) > 0 ) {
		mk_fs_leave();
	}
}

void mk_main_init(int argc, char **argv) {
	static const struct { const char *const header[3], *const lib; } autolinks[] = {
		/* OpenGL */
		{ { "GL/gl.h","GL/gl.h","OpenGL/OpenGL.h" },
		  "opengl" },
		{ { (const char *)0,(const char *)0,"GL/gl.h" },
		  "opengl" },
		{ { (const char *)0,(const char *)0,"OpenGL/gl.h" },
		  "opengl" },

		/* OpenGL - GLU */
		{ { "GL/glu.h","GL/glu.h","OpenGL/glu.h" },
		  "glu" },

		/* OpenGL - GLFW/GLEW */
		{ { "GL/glfw.h","GL/glfw.h","GL/glfw.h" },
		  "glfw" },
		{ { "GLFW/glfw3.h","GLFW/glfw3.h","GLFW/glfw3.h" },
		  "glfw3" },
		{ { "GL/glew.h","GL/glew.h","GL/glew.h" },
		  "glew" },

		/* SDL */
		{ { "SDL/sdl.h","SDL/sdl.h","SDL/sdl.h" },
		  "sdl" },
		{ { "SDL/sdl_mixer.h","SDL/sdl_mixer.h","SDL/sdl_mixer.h" },
		  "sdl_mixer" },
		{ { "SDL/sdl_main.h","SDL/sdl_main.h","SDL/sdl_main.h" },
		  "sdl_main" },

		/* SDL2 - EXPERIMENTAL */
		{ { "SDL2/SDL.h","SDL2/SDL.h","SDL2/SDL.h" },
		  "sdl2" },

		 /* SFML */
		{ { "SFML/Config.hpp","SFML/Config.hpp","SFML/Config.hpp" },
		  "sfml" },

		/* Ogg/Vorbis */
		{ { "ogg/ogg.h","ogg/ogg.h","ogg/ogg.h" },
		  "ogg" },
		{ { "vorbis/codec.h","vorbis/codec.h","vorbis/codec.h" },
		  "vorbis" },
		{ { "vorbis/vorbisenc.h","vorbis/vorbisenc.h","vorbis/vorbisenc.h" },
		  "vorbisenc" },
		{ { "vorbis/vorbisfile.h","vorbis/vorbisfile.h","vorbis/vorbisfile.h" },
		  "vorbisfile" },

		/* Windows */
		{ { "winuser.h",(const char *)0,(const char *)0 },
		  "user32" },
		{ { "winbase.h",(const char *)0,(const char *)0 },
		  "kernel32" },
		{ { "shellapi.h",(const char *)0,(const char *)0 },
		  "shell32" },
		{ { "ole.h",(const char *)0,(const char *)0 },
		  "ole32" },
		{ { "commctrl.h",(const char *)0,(const char *)0 },
		  "comctl32" },
		{ { "commdlg.h",(const char *)0,(const char *)0 },
		  "comdlg32" },
		{ { "wininet.h",(const char *)0,(const char *)0 },
		  "wininet" },
		{ { "mmsystem.h",(const char *)0,(const char *)0 },
		  "winmm" },
		{ { "uxtheme.h",(const char *)0,(const char *)0 },
		  "uxtheme" },
		{ { "wingdi.h",(const char *)0,(const char *)0 },
		  "gdi32" },
		{ { "winsock2.h",(const char *)0,(const char *)0 },
		  "winsock2" },

		/* POSIX */
		{ { (const char *)0,"time.h","time.h" },
		  "realtime" },
		{ { (const char *)0,"math.h","math.h" },
		  "math" },

		/* PNG */
		{ { "png.h","png.h","png.h" },
		  "png" },

		/* BZip2 */
		{ { "bzlib.h","bzlib.h","bzlib.h" },
		  "bzip2" },

		/* ZLib */
		{ { "zlib.h","zlib.h","zlib.h" },
		  "z" },

		/* PThread */
		{ { "pthread.h","pthread.h","pthread.h" },
		  "pthread" },

		/* Curses */
		{ { "curses.h","curses.h","curses.h" },
		  "curses" },
	};
	static const struct { const char *const lib, *const flags[3]; } libs[] = {
		/* OpenGL (and friends) */
		{ "opengl",
		  { "-lopengl32","-lGL","-framework OpenGL" } },
		{ "glu",
		  { "-lglu32","-lGLU","-lGLU" } },
		{ "glfw",
		  { "-lglfw","-lglfw","-lglfw -framework OpenGL" } },
		{ "glfw3",
		  { "-lglfw3","-lglfw.so.3","-lglfw -framework OpenGL" } },
		{ "glew",
		  { "-lglew32","-lGLEW","-lGLEW" } },

		/* SDL */
		{ "sdl",
		  { "-lSDL","-lSDL","-lSDL" } },
		{ "sdl_mixer",
		  { "-lSDL_mixer","-lSDL_mixer","-lSDL_mixer" } },
		{ "sdl_main",
		  { "-lSDLmain","-lSDLmain","-lSDLmain" } },

		/* SDL2 - EXPERIMENTAL */
		{ "sdl2",
		  {  "-lSDL2 -luser32 -lkernel32 -lshell32 -lole32 -lwininet -lwinmm"
			 " -limm32 -lgdi32 -loleaut32 -lversion -luuid",
		     "-lSDL2", "-lSDL2" } },

		/* SFML */
		{ "sfml",
		  { "-lsfml-window-s -lsfml-graphics-s -lsfml-audio-s"
		    " -lsfml-network-s -lsfml-main -lsfml-system-s",
			"-lsfml-window-s -lsfml-graphics-s -lsfml-audio-s"
		    " -lsfml-network-s -lsfml-main -lsfml-system-s",
			"-lsfml-window-s -lsfml-graphics-s -lsfml-audio-s"
		    " -lsfml-network-s -lsfml-main -lsfml-system-s" } },

		/* Ogg/Vorbis */
		{ "ogg",
		  { "-logg","-logg","-logg" } },
		{ "vorbis",
		  { "-lvorbis","-lvorbis","-lvorbis" } },
		{ "vorbisenc",
		  { "-lvorbisenc","-lvorbisenc","-lvorbisenc" } },
		{ "vorbisfile",
		  { "-lvorbisfile","-lvorbisfile","-lvorbisfile" } },

		/* Windows */
		{ "user32",
		  { "-luser32",(const char *)0,(const char *)0 } },
		{ "kernel32",
		  { "-lkernel32",(const char *)0,(const char *)0 } },
		{ "shell32",
		  { "-lshell32",(const char *)0,(const char *)0 } },
		{ "ole32",
		  { "-lole32",(const char *)0,(const char *)0 } },
		{ "comctl32",
		  { "-lcomctl32",(const char *)0,(const char *)0 } },
		{ "comdlg32",
		  { "-lcomdlg32",(const char *)0,(const char *)0 } },
		{ "wininet",
		  { "-lwininet",(const char *)0,(const char *)0 } },
		{ "winmm",
		  { "-lwinmm",(const char *)0,(const char *)0 } },
		{ "uxtheme",
		  { "-luxtheme",(const char *)0,(const char *)0 } },
		{ "gdi32",
		  { "-lgdi32",(const char *)0,(const char *)0 } },
		{ "winsock2",
		  { "-lws2_32",(const char *)0,(const char *)0 } },

		/* POSIX */
		{ "realtime",
		  { (const char *)0,"-lrt",(const char *)0 } },
		{ "math",
		  { (const char *)0,"-lm","-lm" } },

		/* PNG */
		{ "png",
		  { "-lpng","-lpng","-lpng" } },

		/* BZip2 */
		{ "bzip2",
		  { "-lbzip2","-lbzip2","-lbzip2" } },

		/* ZLib */
		{ "z",
		  { "-lz","-lz","-lz" } },

		/* PThread */
		{ "pthread",
		  { "-lpthread","-lpthread","-lpthread" } },

		/* Curses */
		{ "curses",
		  { "-lncurses","-lncurses","-lncurses" } }
	};
	const char *optlinks[256], *p;
	bitfield_t bit;
	MkAutolink al;
	size_t j;
	MkLib lib;
	char temp[PATH_MAX];
	int builtinautolinks = 1, userautolinks = 1;
	int i, op;

	/* core initialization */
	mk_sys_initColoredOutput();
	atexit(mk_sl_deleteAll);
	mk_fs_init();
	atexit(mk_fs_unwindDirs);
	atexit(mk_al_deleteAll);
	atexit(mk_dep_deleteAll);
	atexit(mk_prj_deleteAll);
	mk_bld_initUnitTestArrays();

	/* set single-character options here */
	memset((void *)optlinks, 0, sizeof(optlinks));
	optlinks['h'] = "help";
	optlinks['v'] = "version";
	optlinks['V'] = "verbose";
	optlinks['r'] = "release";
	optlinks['R'] = "rebuild";
	optlinks['c'] = "compile-only";
	optlinks['b'] = "brush";
	optlinks['C'] = "clean";
	optlinks['H'] = "print-hierarchy";
	optlinks['p'] = "pedantic";
	optlinks['T'] = "test";
	optlinks['D'] = "dir";

	/* arrays need to be initialized */
	mk__g_targets = mk_sl_new();
	mk__g_srcdirs = mk_sl_new();
	mk__g_incdirs = mk_sl_new();
	mk__g_libdirs = mk_sl_new();
	mk__g_pkgdirs = mk_sl_new();
	mk__g_tooldirs = mk_sl_new();
	mk__g_dllsdirs = mk_sl_new();

	/* process command line arguments */
	for( i=1; i<argc; i++ ) {
		const char *opt;

		opt = argv[i];
		if( *opt=='-' ) {
			op = 0;
			p = (const char *)0;

			if( *(opt+1)=='-' ) {
				opt = &opt[2];

				if( (op = !strncmp(opt, "no-", 3) ? 1 : 0)==1 ) {
					opt = &opt[3];
				}

				if( (p = strchr(opt, '=')) != (const char *)0 ) {
					mk_com_strncpy(temp, sizeof(temp), opt, p-opt);
					p++;
					opt = temp;
				}
			} else {
				if( *(opt+1)=='I' ) {
					if( *(opt+2)==0 ) {
						p = argv[++i];
					} else {
						p = &opt[2];
					}

					opt = "incdir";
				} else if(*(opt+1)=='L') {
					if( *(opt+2)==0 ) {
						p = argv[++i];
					} else {
						p = &opt[2];
					}

					opt = "libdir";
				} else if(*(opt+1)=='S') {
					if( *(opt+2)==0 ) {
						p = argv[++i];
					} else {
						p = &opt[2];
					}

					opt = "srcdir";
				} else if(*(opt+1)=='P') {
					if( *(opt+2)==0 ) {
						p = argv[++i];
					} else {
						p = &opt[2];
					}
				} else {
					/* not allowing repeats (e.g., -hv) yet */
					if( *(opt+2)!=0 ) {
						mk_log_errorMsg(mk_com_va("^E'%s'^& is a malformed argument",
							argv[i]));
						continue;
					}

					if( !(opt = optlinks[(unsigned char)*(opt+1)]) ) {
						mk_log_errorMsg(mk_com_va("unknown option ^E'%s'^&; ignoring",
							argv[i]));
						continue;
					}
				}
			}

			MK_ASSERT( opt != (const char *)0 );

			bit = 0;
			if( !strcmp(opt, "help") ) {
				bit = kMkFlag_ShowHelp_Bit;
			} else if(!strcmp(opt, "version")) {
				bit = kMkFlag_ShowVersion_Bit;
			} else if(!strcmp(opt, "verbose")) {
				bit = kMkFlag_Verbose_Bit;
			} else if(!strcmp(opt, "release")) {
				bit = kMkFlag_Release_Bit;
			} else if(!strcmp(opt, "rebuild")) {
				bit = kMkFlag_Rebuild_Bit;
			} else if(!strcmp(opt, "compile-only")) {
				bit = kMkFlag_NoLink_Bit;
			} else if(!strcmp(opt, "brush")) {
				bit = kMkFlag_NoCompile_Bit|kMkFlag_NoLink_Bit|kMkFlag_LightClean_Bit;
			} else if(!strcmp(opt, "clean")) {
				bit = kMkFlag_NoCompile_Bit|kMkFlag_NoLink_Bit|kMkFlag_FullClean_Bit;
			} else if(!strcmp(opt, "print-hierarchy")) {
				bit = kMkFlag_PrintHierarchy_Bit;
			} else if(!strcmp(opt, "pedantic")) {
				bit = kMkFlag_Pedantic_Bit;
			} else if(!strcmp(opt, "test")) {
				bit = kMkFlag_Test_Bit;
			} else if(!strcmp(opt, "color")) {
				if( op ) {
					mk__g_flags_color = kMkColorMode_None;
				} else {
					mk__g_flags_color = MK__DEFAULT_COLOR_MODE_IMPL;
				}
				continue;
			} else if(!strcmp(opt, "ansi-colors")) {
				if( op ) {
					mk__g_flags_color = kMkColorMode_None;
				} else {
					mk__g_flags_color = kMkColorMode_ANSI;
				}
				continue;
			} else if(!strcmp(opt, "win32-colors")) {
#if MK_WINDOWS_COLORS_ENABLED
				if( op ) {
					mk__g_flags_color = kMkColorMode_None;
				} else {
					mk__g_flags_color = kMkColorMode_Windows;
				}
#else
				mk_log_errorMsg("option \"--[no-]win32-colors\" is disabled in this build");
#endif
				continue;
			} else if(!strcmp(opt, "builtin-autolinks")) {
				builtinautolinks = (int)!op;
				continue;
			} else if(!strcmp(opt, "user-autolinks")) {
				userautolinks = (int)!op;
				continue;
			} else if(!strcmp(opt, "srcdir")) {
				if( i+1==argc&&!p ) {
					mk_log_errorMsg(mk_com_va("expected argument to ^E'%s'^&", argv[i]));
					continue;
				}

				mk_front_pushSrcDir(p ? p : argv[++i]);
				continue;
			} else if(!strcmp(opt, "incdir")) {
				if( i+1==argc&&!p ) {
					mk_log_errorMsg(mk_com_va("expected argument to ^E'%s'^&", argv[i]));
					continue;
				}

				mk_front_pushIncDir(p ? p : argv[++i]);
				continue;
			} else if(!strcmp(opt, "libdir")) {
				if( i+1==argc&&!p ) {
					mk_log_errorMsg(mk_com_va("expected argument to ^E'%s'^&", argv[i]));
					continue;
				}

				mk_front_pushLibDir(p ? p : argv[++i]);
				continue;
			} else if(!strcmp(opt, "pkgdir")) {
				if( i+1==argc&&!p ) {
					mk_log_errorMsg(mk_com_va("expected argument to ^E'%s'^&", argv[i]));
					continue;
				}

				mk_front_pushPkgDir(p ? p : argv[++i]);
				continue;
			} else if(!strcmp(opt, "toolsdir")) {
				if( i+1==argc&&!p ) {
					mk_log_errorMsg(mk_com_va("expected argument to ^E'%s'^&", argv[i]));
					continue;
				}

				mk_front_pushToolDir(p ? p : argv[++i]);
				continue;
			} else if(!strcmp(opt, "dllsdir")) {
				if( i+1==argc&&!p ) {
					mk_log_errorMsg(mk_com_va("expected argument to ^E'%s'^&", argv[i]));
					continue;
				}

				mk_front_pushDynamicLibsDir(p ? p : argv[++i]);
				continue;
			} else if(!strcmp(opt, "dir")) {
				if( i+1==argc&&!p ) {
					mk_log_errorMsg(mk_com_va("expected argument to ^E'%s'^&", argv[i]));
					continue;
				}

				mk_fs_enter(p ? p : argv[++i]);
				continue;
			} else {
				mk_log_errorMsg(mk_com_va("unknown option ^E'%s'^&; ignoring", argv[i]));
				continue;
			}

			MK_ASSERT( bit != 0 );
			MK_ASSERT( op==0 || op==1 );

			if( op ) {
				mk__g_flags &= ~bit;
			} else {
				mk__g_flags |= bit;
			}
		} else {
			mk_sl_pushBack(mk__g_targets, opt);
		}
	}

	/* support "clean rebuilds" */
	if( ( mk__g_flags & kMkFlag_Rebuild_Bit ) && ( mk__g_flags &
	( kMkFlag_LightClean_Bit | kMkFlag_FullClean_Bit ) ) ) {
		mk__g_flags &= ~( kMkFlag_NoCompile_Bit | kMkFlag_NoLink_Bit );
	}

	/* show the version */
	if( mk__g_flags & kMkFlag_ShowVersion_Bit ) {
		printf(
			"mk " MK_VERSION_STR " - compiled %s\nCopyright (c) 2012-2016 NotKyon\n\n"
			"This software contains ABSOLUTELY NO WARRANTY.\n\n", __DATE__);
	}

	/* show the help */
	if( mk__g_flags & kMkFlag_ShowHelp_Bit ) {
		printf("Usage: mk [options...] [targets...]\n");
		printf("Options:\n");
		printf("  -h,--help                Show this help message.\n");
		printf("  -v,--version             Show the version.\n");
		printf("  -V,--verbose             Show the commands invoked.\n");
		printf("  -b,--brush               Remove intermediate files after "
			"building.\n");
		printf("  -C,--clean               Remove \"%s\"\n", mk_opt_getObjdirBase());
		printf("  -r,--release             Build in release mode.\n");
		printf("  -R,--rebuild             "
			"Force a rebuild, without cleaning.\n");
		printf("  -T,--test                Run unit tests.\n");
		printf("  -c,--compile-only        Just compile; do not link.\n");
		printf("  -p,--pedantic            Enable pedantic warnings.\n");
		printf("  -H,--print-hierarchy     Display the project hierarchy.\n");
		printf("  -S,--srcdir=<dir>        Add a source directory.\n");
		printf("  -I,--incdir=<dir>        Add an include directory.\n");
		printf("  -L,--libdir=<dir>        Add a library directory.\n");
		printf("  -P,--pkgdir=<dir>        Add a package directory.\n");
		printf("  --[no-]color             Enable or disable colored output.\n");
		printf("    --ansi-colors          Enable ANSI-based coloring.\n");
#if MK_WINDOWS_COLORS_ENABLED
		printf("    --win32-colors         Enable Windows-based coloring.\n");
#endif
		printf("  --[no-]builtin-autolinks Enable built-in autolinks (default).\n");
		printf("  --[no-]user-autolinks    Enable loading of mk-autolinks.txt (default).\n");
		printf("\n");
		printf("See the documentation (or source code) for more details.\n");
	}

	/* exit if no targets were specified and a message was requested */
	if( mk__g_flags&(kMkFlag_ShowVersion_Bit|kMkFlag_ShowHelp_Bit )
	 && !mk_sl_getSize(mk__g_targets)) {
		exit(EXIT_SUCCESS);
	}
	
	/* initialize the global directories */
	mk_fs_makeDirs( mk_opt_getGlobalSharedDir() );
	mk_fs_makeDirs( mk_opt_getGlobalVersionDir() );

	/* add builtin autolinks / libraries */
	if( builtinautolinks ) {
		/* add the autolinks */
		for( j=0; j<sizeof(autolinks)/sizeof(autolinks[0]); j++ ) {
			al = mk_al_new();

			mk_al_setLib(al, autolinks[j].lib);

			mk_al_setHeader(al, kMkOS_MSWin , autolinks[j].header[0]);
			mk_al_setHeader(al, kMkOS_UWP   , autolinks[j].header[0]);
			mk_al_setHeader(al, kMkOS_Cygwin, autolinks[j].header[1]);
			mk_al_setHeader(al, kMkOS_Linux , autolinks[j].header[1]);
			mk_al_setHeader(al, kMkOS_MacOSX, autolinks[j].header[2]);
			mk_al_setHeader(al, kMkOS_Unix  , autolinks[j].header[1]);
		}

		/* add the libraries */
		for( j=0; j<sizeof(libs)/sizeof(libs[0]); j++ ) {
			lib = mk_lib_new();

			mk_lib_setName(lib, libs[j].lib);

			mk_lib_setFlags(lib, kMkOS_MSWin , libs[j].flags[0]);
			mk_lib_setFlags(lib, kMkOS_UWP   , libs[j].flags[0]);
			mk_lib_setFlags(lib, kMkOS_Cygwin, libs[j].flags[1]);
			mk_lib_setFlags(lib, kMkOS_Linux , libs[j].flags[1]);
			mk_lib_setFlags(lib, kMkOS_MacOSX, libs[j].flags[2]);
			mk_lib_setFlags(lib, kMkOS_Unix  , libs[j].flags[1]);
		}
	}

	/* load user autolinks / libraries */
	if( userautolinks ) {
		char szFile[ PATH_MAX ];
		
		/* load from .mk/share/mk-autolinks.txt first */
		mk_com_strcpy( szFile, sizeof( szFile ), mk_opt_getGlobalSharedDir() );
		MK_ASSERT( mk_com_strends( szFile, "/" ) && "mk_opt_getGlobalSharedDir() must return path with a trailing '/'" );
		mk_com_strcat( szFile, sizeof( szFile ), "mk-autolinks.txt" );
		( void )mk_al_loadConfig( szFile );

		/* load from the current directory */
		( void )mk_al_loadConfig( "mk-autolinks.txt" );
	}

	/* grab the available directories */
	mk_prjfs_findRootDirs(mk__g_srcdirs, mk__g_incdirs, mk__g_libdirs, mk__g_pkgdirs, mk__g_tooldirs,
		mk__g_dllsdirs);

	/* if there aren't any source directories, complain */
	if( !mk_sl_getSize(mk__g_srcdirs) && !mk_sl_getSize(mk__g_pkgdirs) ) {
		mk_log_fatalError("no source ('src' or 'source') or package ('pkg') directories");
	}
}

int main(int argc, char **argv) {
	mk_main_init(argc, argv);

	if( mk__g_flags & kMkFlag_PrintHierarchy_Bit ) {
		printf("Source Directories:\n");
		mk_sl_print(mk__g_srcdirs);

		printf("Include Directories:\n");
		mk_sl_print(mk__g_incdirs);

		printf("Library Directories:\n");
		mk_sl_print(mk__g_libdirs);

		printf("Targets:\n");
		mk_sl_print(mk__g_targets);

		printf("Projects:\n");
		mk_prj_printAll((MkProject)0, "  ");
		printf("\n");
		fflush(stdout);
	}

	if( !mk_bld_makeAllProjects() ) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
