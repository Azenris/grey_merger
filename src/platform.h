
#pragma once

void platform_print_error();

struct MemoryArena;

// Memory
void platform_memmove( void *dst, void *src, u64 bytes );
void platform_memcpy( void *dst, void *src, u64 bytes );
void platform_memzero( void *dst, u64 bytes );
void platform_memset( void *dst, u8 value, u64 bytes );

// Timings
[[nodiscard]] inline u64 platform_get_tick_frequency();
[[nodiscard]] inline u64 platform_get_tick_counter();
[[nodiscard]] inline u64 platform_get_cycle_counter();

// Directory
bool platform_create_directory( const char *directory );
inline void platform_set_current_directory( const char *workingDirectory );

// Files
using FileOptions = u32;
enum FILE_OPTION : FileOptions
{
	FILE_OPTION_READ	= ( 1 << 0 ),		// open the file for reading
	FILE_OPTION_WRITE	= ( 1 << 1 ),		// open the file for writing
	FILE_OPTION_CREATE	= ( 1 << 2 ),		// create the file if it doesnt exist
	FILE_OPTION_APPEND	= ( 1 << 3 ),		// start at the end of the file
	FILE_OPTION_CLEAR	= ( 1 << 4 ),		// the file is cleared
};

using FileSeek = u32;
enum FILE_SEEK : FileSeek
{
	FILE_SEEK_START,
	FILE_SEEK_CURRENT,
	FILE_SEEK_END,
};

[[nodiscard]] u8 *platform_read_file( const char *path, u64 *fileSize, bool addNullTerminator, MemoryArena *arena );
u64 platform_write_file( const char *path, u8 *buffer, u64 size, bool append );
u32 platform_open_file( const char *path, FileOptions options );
void platform_close_file( u32 fileID );
u64 platform_get_file_size( u32 fileID );
void platform_seek_in_file( u32 fileID, FileSeek seek, u64 offset );
u8 *platform_read_whole_file( u32 fileID, MemoryArena *arena, bool addNullTerminator );
u8 *platform_read_image( const char *filename, u32 *width, u32 *height, u32 *channels );
u64 platform_read_from_file( u32 fileID, void *buffer, u64 size );
u64 platform_write_to_file( u32 fileID, void *buffer, u64 size );
[[nodiscard]] inline bool platform_file_exists( const char *path );
inline bool platform_delete_file( const char *path );
[[nodiscard]] u64 platform_last_edit_timestamp( const char *path );
inline void platform_copy_file( const char *from, const char *to );