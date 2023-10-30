
#pragma once

#pragma warning(push, 0)

#define STBI_MALLOC( size )			app.memory.transient.allocate<stbi_uc>( (u64)size )
#define STBI_REALLOC( p, size )		app.memory.transient.reallocate<stbi_uc>( (u8 *)( p ), (u64)size )
#define STBI_FREE( p )				app.memory.transient.free( (u8 *)( p ) )
#define STBI_ASSERT( x )			assert( x && #x )

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBIW_MALLOC( size )		app.memory.transient.allocate<stbi_uc>( (u64)size )
#define STBIW_REALLOC( p, size )	app.memory.transient.reallocate<stbi_uc>( (u8 *)( p ), (u64)size )
#define STBIW_FREE( p )				app.memory.transient.free( (u8 *)( p ) )
#define STBIW_ASSERT( x )			assert( x && #x )
#define STBIW_MEMMOVE( d, s, size )	memmove( d, s, size )

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#pragma warning(pop)
