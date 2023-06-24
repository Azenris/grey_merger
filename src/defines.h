
#pragma once

#define KB( x ) 				( (u64)1024 * x )
#define MB( x ) 				( (u64)1024 * KB( x ) )
#define GB( x ) 				( (u64)1024 * MB( x ) )

#define BIT( b ) 				( ( 1 << ( b ) ) )

#define array_length( arr ) 	( sizeof( arr ) / sizeof( arr[ 0 ] ) )

#define MEMORY_ALIGNMENT		sizeof( u64 )		// must be a pwoer of 2

#define INVALID_INDEX_UINT_16 	( UINT16_MAX )
#define INVALID_INDEX_UINT_32 	( UINT32_MAX )
#define INVALID_INDEX_UINT_64	( UINT64_MAX )

#define MAX_OPEN_FILES 			( 8 )

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;