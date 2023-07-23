
#pragma once

// MEMORY ARENA /////////////////////////////////////////////////////////////////
using MemoryFlags = u32;
enum MEMORY_FLAGS : MemoryFlags
{
	MEMORY_FLAGS_INITIALISED			= BIT( 0 ),
	MEMORY_FLAGS_SEPERATE_ALLOCATIONS	= BIT( 1 ),
};

struct MemoryHeader
{
	u8 *prev;				// last block before this one
	u64 reqSize;			// amount allocated
	u64 size;				// size requested
	u16 alignment;			// alignment of allocation
	u16 padding;			// padding used to gain the alignment
};

static_assert( sizeof( MemoryHeader ) % MEMORY_ALIGNMENT == 0 );

struct MemoryBlockPermanent
{
	u64 capacity;
	u64 available;
	u8 *memory;
	u8 *lastAlloc;
};

struct MemoryBlockTransient
{
	u64 capacity;
	u64 available;
	u8 *memory;
	u8 *lastAlloc;
};

struct MemoryArena
{
	MemoryFlags flags = 0;
	u8 *memory = nullptr;
	MemoryBlockPermanent permanent;
	MemoryBlockTransient transient;
};

// Check it's a power of 2
[[nodiscard ]] inline bool valid_memory_alignment( u64 number )
{
	return ( number & ( number - 1 ) ) == 0;
}

// FUNCTIONS ////////////////////////////////////////////////////////////////////
bool memory_arena_initialise( MemoryArena *arena, u64 permanentSize, u64 transientSize, bool clearZero = false, u16 alignment = MEMORY_ALIGNMENT );
void memory_arena_free( MemoryArena *arena );
inline void memory_arena_update( MemoryArena *arena );

// Permanent Memory
template <typename T> [[nodiscard]] T *memory_arena_permanent_allocate( MemoryArena *arena, bool clearZero = false )
{
	return reinterpret_cast<T*>( memory_arena_permanent_allocate( arena, sizeof( T ), clearZero, alignof( T ) ) );
}

[[nodiscard]] u8 *memory_arena_permanent_allocate( MemoryArena *arena, u64 size, bool clearZero = false, u16 alignment = MEMORY_ALIGNMENT );
[[nodiscard]] u8 *memory_arena_permanent_reallocate( MemoryArena *arena, void *p, u64 size );
void memory_arena_permanent_free( MemoryArena *arena, void *p );

// Transient Memory
template <typename T> [[nodiscard]] T *memory_arena_transient_allocate( MemoryArena *arena, bool clearZero = false )
{
	return reinterpret_cast<T*>( memory_arena_transient_allocate( arena, sizeof( T ), clearZero, alignof( T ) ) );
}

[[nodiscard]] u8 *memory_arena_transient_allocate( MemoryArena *arena, u64 size, bool clearZero = false, u16 alignment = MEMORY_ALIGNMENT );
[[nodiscard]] u8 *memory_arena_transient_reallocate( MemoryArena *arena, void *p, u64 size );
void memory_arena_transient_free( MemoryArena *arena, void *p );

// FUNCTION IMPLEMENTATIONS /////////////////////////////////////////////////////
bool memory_arena_initialise( MemoryArena *arena, u64 permanentSize, u64 transientSize, bool clearZero, u16 alignment )
{
	massert( valid_memory_alignment( alignment ) && alignment >= MEMORY_ALIGNMENT );

	constexpr const u64 permanentMinSize = sizeof( MemoryBlockPermanent ) + sizeof( MemoryHeader );
	constexpr const u64 transientMinSize = sizeof( MemoryBlockTransient ) + sizeof( MemoryHeader );
	if ( permanentSize < permanentMinSize ) permanentSize = permanentMinSize;
	if ( transientSize < transientMinSize ) transientSize = transientMinSize;

	if ( arena->flags & MEMORY_FLAGS_INITIALISED )
		memory_arena_free( arena );

	u64 permanentReqSize = permanentSize + ( alignment - ( permanentSize & ( alignment - 1 ) ) );
	u64 transientReqSize = transientSize + ( alignment - ( transientSize & ( alignment - 1 ) ) );
	u64 reqSize = permanentReqSize + transientReqSize;
	u8 *permanentMemory = nullptr;
	u8 *transientMemory = nullptr;

	arena->memory = (u8 *)malloc( reqSize );

	// If the memory allocation fails, attempt to allocate
	// seperately for the memory blocks
	if ( !arena->memory )
	{
		permanentMemory = (u8 *)malloc( permanentReqSize );
		transientMemory = (u8 *)malloc( transientReqSize );
		arena->memory = permanentMemory;
		arena->flags |= MEMORY_FLAGS_SEPERATE_ALLOCATIONS;
	}
	else
	{
		permanentMemory = arena->memory;
		transientMemory = permanentMemory + permanentReqSize;
		arena->flags &= ~MEMORY_FLAGS_SEPERATE_ALLOCATIONS;
	}

	if ( !permanentMemory || !transientMemory )
	{
		show_log_error( "memory_arena_initialise failed to allocate %d bytes.", reqSize );
		return false;
	}

	if ( clearZero )
	{
		memset( permanentMemory, 0, permanentReqSize );
		memset( transientMemory, 0, transientReqSize );
	}

	MemoryBlockPermanent permanentMemoryBlock =
	{
		.capacity = permanentSize,
		.available = permanentSize,
		.memory = permanentMemory,
		.lastAlloc = nullptr,
	};

	MemoryBlockTransient transientMemoryBlock =
	{
		.capacity = transientSize,
		.available = transientSize,
		.memory = transientMemory,
		.lastAlloc = nullptr,
	};

	arena->permanent = permanentMemoryBlock;
	arena->transient = transientMemoryBlock;

	arena->flags |= MEMORY_FLAGS_INITIALISED;

	return true;
}

void memory_arena_free( MemoryArena *arena )
{
	massert( arena );

	if ( arena->flags & MEMORY_FLAGS_INITIALISED )
	{
		// Check if it was a single allocation or 2 seperate ones
		if ( arena->flags & MEMORY_FLAGS_SEPERATE_ALLOCATIONS )
		{
			free( arena->permanent.memory );
			free( arena->transient.memory );
		}
		else
		{
			free( arena->memory );
		}

		memset( arena, 0, sizeof( *arena ) );
	}
}

inline void memory_arena_update( MemoryArena *arena )
{
	MemoryBlockTransient &transientMemory = arena->transient;
	transientMemory.available = transientMemory.capacity;
	transientMemory.lastAlloc = nullptr;
}

// Permanent Memory
[[nodiscard]] u8 *memory_arena_permanent_allocate( MemoryArena *arena, u64 size, bool clearZero, u16 alignment )
{
	MemoryBlockPermanent &memoryBlock = arena->permanent;

	massert( size );
	massert( valid_memory_alignment( alignment ) && alignment >= MEMORY_ALIGNMENT );

	u8 *p = memoryBlock.memory + ( memoryBlock.capacity - memoryBlock.available ) + sizeof( MemoryHeader );
	u64 padding = alignment - ( reinterpret_cast<u64>( p ) & ( alignment - 1 ) );

	// P now points to the data
	p += padding;

	// Total size that needs allocating
	u64 reqSize = padding + sizeof( MemoryHeader ) + size;

	if ( reqSize > memoryBlock.available )
	{
		show_log_error( "Failed to allocate %d bytes memory.", reqSize );
		return nullptr;
	}

	MemoryHeader *header = reinterpret_cast<MemoryHeader *>( p - sizeof( MemoryHeader ) );
	header->prev = memoryBlock.lastAlloc;
	header->reqSize = reqSize;
	header->size = size;
	header->alignment = alignment;
	header->padding = static_cast<u16>( padding );

	memoryBlock.available -= reqSize;
	memoryBlock.lastAlloc = p;

	if ( clearZero )
		memset( memoryBlock.lastAlloc, 0, size );

	return memoryBlock.lastAlloc;
}

[[nodiscard]] u8 *memory_arena_permanent_reallocate( MemoryArena *arena, void *p, u64 size )
{
	show_log_error( "NYI memory_arena_permanent_reallocate" );
	return nullptr;
}

void memory_arena_permanent_free( MemoryArena *arena, void *p )
{
	show_log_error( "NYI memory_arena_permanent_free" );
}

// Transient Memory
[[nodiscard]] u8 *memory_arena_transient_allocate( MemoryArena *arena, u64 size, bool clearZero, u16 alignment )
{
	MemoryBlockTransient &memoryBlock = arena->transient;

	massert( size );
	massert( valid_memory_alignment( alignment ) && alignment >= MEMORY_ALIGNMENT );

	u8 *p = memoryBlock.memory + ( memoryBlock.capacity - memoryBlock.available ) + sizeof( MemoryHeader );
	u64 padding = alignment - ( reinterpret_cast<u64>( p ) & ( alignment - 1 ) );

	// P now points to the data
	p += padding;

	// Total size that needs allocating
	u64 reqSize = padding + sizeof( MemoryHeader ) + size;

	if ( reqSize > memoryBlock.available )
	{
		show_log_error( "Failed to allocate %d bytes memory.", reqSize );
		return nullptr;
	}

	MemoryHeader *header = reinterpret_cast<MemoryHeader *>( p - sizeof( MemoryHeader ) );
	header->prev = memoryBlock.lastAlloc;
	header->reqSize = reqSize;
	header->size = size;
	header->alignment = alignment;
	header->padding = static_cast<u16>( padding );

	memoryBlock.available -= reqSize;
	memoryBlock.lastAlloc = p;

	if ( clearZero )
		memset( memoryBlock.lastAlloc, 0, size );

	return memoryBlock.lastAlloc;
}

[[nodiscard]] u8 *memory_arena_transient_reallocate( MemoryArena *arena, void *p, u64 size )
{
	if ( !p )
		return memory_arena_transient_allocate( arena, size );

	massert( size );

	MemoryBlockTransient &memoryBlock = arena->transient;

	MemoryHeader *header = reinterpret_cast<MemoryHeader*>( static_cast<u8 *>( p ) - sizeof( MemoryHeader ) );

	// Same size
	if ( size == header->size )
		return static_cast<u8 *>( p );

	u64 oldSize = header->size;
	u64 oldReqSize = header->reqSize;

	// See if it was the last used allocation
	if ( memoryBlock.lastAlloc == p )
	{
		u64 reqSize = header->padding + sizeof( MemoryHeader ) + size;

		// Either shrinking or same required memory
		if ( size < oldSize || reqSize == oldReqSize )
		{
			header->reqSize = reqSize;
			header->size = size;
			memoryBlock.available += ( oldReqSize - reqSize );
			return static_cast<u8 *>( p );
		}

		u64 extraReqSizeNeeded = ( reqSize - oldReqSize );

		if ( extraReqSizeNeeded > memoryBlock.available )
		{
			show_log_error( "Failed to grow memory by %d bytes.", extraReqSizeNeeded );
			return nullptr;
		}

		header->reqSize = reqSize;
		header->size = size;

		// Remove the extra space required for this reallocation
		memoryBlock.available -= extraReqSizeNeeded;

		return static_cast<u8 *>( p );
	}

	// Since it wasn't the last allocation, allocate a new block and copy the data over
	u8 *newMemory = memory_arena_transient_allocate( arena, size, false, header->alignment );

	if ( newMemory )
		memcpy( newMemory, p, size < oldSize ? size : oldSize );

	memory_arena_transient_free( arena, p );

	return newMemory;
}

void memory_arena_transient_free( MemoryArena *arena, void *p )
{
	MemoryBlockTransient &memoryBlock = arena->transient;

	if ( p && memoryBlock.lastAlloc == p )
	{
		MemoryHeader *header = reinterpret_cast<MemoryHeader*>( static_cast<u8 *>( p ) - sizeof( MemoryHeader ) );
		u64 reqSize = header->reqSize;
		memoryBlock.available += reqSize;
		memoryBlock.lastAlloc = header->prev;
	}
}