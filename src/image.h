
#pragma once

#pragma warning(push, 0)

#define STBI_MALLOC( size )			(stbi_uc *)memory_arena_transient_allocate( &arena, size )
#define STBI_REALLOC( p, size )		(stbi_uc *)memory_arena_transient_reallocate( &arena, (u8 *)( p ), size )
#define STBI_FREE( p )				memory_arena_transient_free( &arena, (u8 *)( p ) )
#define STBI_ASSERT( x )			massert( x, #x )

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBIW_MALLOC( size )		(stbi_uc *)memory_arena_transient_allocate( &arena, size )
#define STBIW_REALLOC( p, size )	(stbi_uc *)memory_arena_transient_reallocate( &arena, (u8 *)( p ), size )
#define STBIW_FREE( p )				memory_arena_transient_free( &arena, (u8 *)( p ) )
#define STBIW_ASSERT( x )			massert( x, #x )
#define STBIW_MEMMOVE( d, s, size )	platform_memmove( d, s, size )

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#pragma warning(pop)