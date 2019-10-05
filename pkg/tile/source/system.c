#include <tile/system.h>

#define TL_TIME_NANOSECS  1000000000
#define TL_TIME_MICROSECS 1000000
#define TL_TIME_MILLISECS 1000

#define TL_TIME_ConvRes(T_,F_,R_) \
	(((TlU64)(T_))*((TlU64)(R_))/((TlU64)(F_)))

/* Convert a time (t) from the system frequency to the target frequency (tf) */
static TlU64 tlConvFreq( TlU64 t, TlU64 tf )
{
#if defined(_WIN32)
	static LARGE_INTEGER f;
	static int didinit = 0;

	if( !didinit ) {
		if( !QueryPerformanceFrequency( &f ) ) {
			return 0;
		}
		didinit = 1;
	}

	return TL_TIME_ConvRes( t, f.QuadPart, tf );
#elif defined(__APPLE__)
	static TlU64 rate = 0;

	if( __builtin_expect( rate == 0, 0 ) ) {
		static mach_timebase_info_data_t info;
		if( mach_timebase_info( &info ) != KERN_SUCCESS ) {
			return 0;
		}
		rate = ((TlU64)1000000000)*info.numer/info.denom;
	}

	return TL_TIME_ConvRes( t, rate, tf );
# else
	return TL_TIME_ConvRes( t, TL_TIME_NANOSECS, tf );
# endif
}

#ifdef CLOCK_MONOTONIC_PRECISE
# define TILE_POSIX_CLOCK CLOCK_MONOTONIC_PRECISE
#elif defined( CLOCK_MONOTONIC_RAW )
# define TILE_POSIX_CLOCK CLOCK_MONOTONIC_RAW
#elif defined( CLOCK_HIGHRES )
# define TILE_POSIX_CLOCK CLOCK_HIGHRES
#elif defined( CLOCK_MONOTONIC )
# define TILE_POSIX_CLOCK CLOCK_MONOTONIC
#elif defined( CLOCK_REALTIME )
# define TILE_POSIX_CLOCK CLOCK_REALTIME
#endif

static TlU64 tlQueryTime( void )
{
# if defined(_WIN32)
	LARGE_INTEGER t;

	if( !QueryPerformanceCounter( &t ) ) {
		return 0;
	}

	return t.QuadPart;
# elif defined(__APPLE__)
	static TlU64 base = 0;
	if( __builtin_expect( base == 0, 0 ) ) {
		base = mach_absolute_time();
	}
	return mach_absolute_time() - base;
# else
	struct timespec t;

#  if TILE_POSIX_CLOCK != -1
	if( clock_gettime( TILE_POSIX_CLOCK, &t ) == 0 ) {
		return (((TlU64)t.tv_sec)*AXTIME_NANOSECS) + (TlU64)t.tv_nsec;
	}
	else
#  endif /*TILE_POSIX_CLOCK != -1*/
	{
		struct timeval tv;

		if( gettimeofday( &tv, NULL ) == -1 ) {
			return 0;
		}

		return (((TlU64)tv.tv_sec)*AXTIME_NANOSECS) +
			(TlU64)tv.tv_usec*1000;
	}
# endif
}
TlU64 tlSys_Microtime( void )
{
	return tlConvFreq( tlQueryTime(), TL_TIME_MICROSECS );
}
