
#pragma once

#define STBI_MALLOC( size )			app.memory.transient.allocate<stbi_uc>( (u64)size )
#define STBI_REALLOC( p, size )		app.memory.transient.reallocate<stbi_uc>( p, (u64)size )
#define STBI_FREE( p )				app.memory.transient.free( p )
#define STBI_ASSERT( x )			assert( x && #x )

#define STBIW_MALLOC( size )		app.memory.transient.allocate<stbi_uc>( (u64)size )
#define STBIW_REALLOC( p, size )	app.memory.transient.reallocate<stbi_uc>( p, (u64)size )
#define STBIW_FREE( p )				app.memory.transient.free( p )
#define STBIW_ASSERT( x )			assert( x && #x )
#define STBIW_MEMMOVE( d, s, size )	memmove( d, s, size )