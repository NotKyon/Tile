#include <tile/system.h>

#if defined( _WIN32 )
static TlU64 tlSys_TimerFreq( void )
{
	LARGE_INTEGER f;

	if( !QueryPerformanceFrequency( &f ) ) {
		f.QuadPart = 1;
	}

	return ( TlU64 )f.QuadPart;
}
#endif

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

TlU64 tlSys_Microtime( void )
{
#if defined( _WIN32 )
	static TlU64 base = 0;
	LARGE_INTEGER t;
	TlU64 r;

	t.QuadPart = 0;
	QueryPerformanceCounter( ( LARGE_INTEGER * )&t );

	r = ( t.QuadPart - base )*1000000/tlSys_TimerFreq();
	if( TL_UNLIKELY( !base ) ) {
		base = t.QuadPart;
		r = 0;
	}

	return r;
#elif defined( __linux__ ) || defined( __linux ) || defined( linux )
# ifdef TILE_POSIX_CLOCK
	struct timespec t;

	if( clock_gettime( TILE_POSIX_CLOCK, &t ) == 0 ) {
		return ((TlU64)t.tv_sec)*1000000 + (TlU64)(t.tv_nsec/1000);
	}
	else
# endif /*TILE_POSIX_CLOCK*/
	{
		struct timeval tv;

		if( gettimeofday( &tv, NULL ) == -1 ) {
			return 0;
		}

		return ((TlU64)tv.tv_sec)*1000000 + (TlU64)tv.tv_usec;
	}
#elif defined( __APPLE__ )
	/* THIS IS NOT TESTED */
	static mach_timebase_info_data_t info;
	static int didinit = 0;

	TlU64 v;

	if( !didinit ) {
		if( mach_timebase_info( &info ) != KERN_SUCCESS ) {
			return 0;
		}
		didinit = 1;
	}

	v = mach_absolute_time();
	return v*info.numer/(((TlU64)info.denom)*1000000);
#else
# error Unknown or unsupported platform.
#endif
}
