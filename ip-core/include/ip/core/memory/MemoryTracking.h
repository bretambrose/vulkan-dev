
#pragma once

#ifdef TRACK_MEMORY_ALLOCATIONS

#define TOSTRING2(x) #x
#define TOSTRING(x) TOSTRING2( x )
#define MEMORY_TAG __FILE__ ## ":" ## TOSTRING( __LINE__ )

#else

#define MEMORY_TAG nullptr

#endif // TRACK_MEMORY_ALLOCATIONS