
#pragma once

#define KB( x ) 				( (u64)1024 * x )
#define MB( x ) 				( (u64)1024 * KB( x ) )
#define GB( x ) 				( (u64)1024 * MB( x ) )

#define BIT( b ) 				( ( 1 << ( b ) ) )

#define array_length( arr ) 	( sizeof( arr ) / sizeof( arr[ 0 ] ) )

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