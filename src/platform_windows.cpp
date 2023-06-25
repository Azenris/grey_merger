
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#undef min
#undef max

Array<HANDLE, MAX_OPEN_FILES> allOpenFiles;
Array<u32, MAX_OPEN_FILES> freeOpenFileIDs;

void show_debug_function( WORD colour, const char *prefix, const char *message )
{
	HANDLE consoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );

	SetConsoleTextAttribute( consoleHandle, colour );

	#ifdef DEBUG
		OutputDebugStringA( prefix );
		OutputDebugStringA( message );
		OutputDebugStringA( "\n" );
	#endif

	WriteConsoleA( consoleHandle, prefix, (DWORD)strlen( prefix ), 0, 0 );
	WriteConsoleA( consoleHandle, message, (DWORD)strlen( message ), 0, 0 );
	WriteConsoleA( consoleHandle, "\n", 1, 0, 0 );
}

// Messages
void show_debug_message_function( const char *message, ... )
{
	char buffer[ 2048 ];
	va_list args;
	va_start( args, message );
	vsnprintf( buffer, array_length( buffer ), message, args );
	show_debug_function( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "[  MSG]: ", buffer );
	va_end( args );
}

void show_debug_message_function_ext( const char *message, va_list args )
{
	char buffer[ 2048 ];
	vsnprintf( buffer, array_length( buffer ), message, args );
	show_debug_function( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "[  MSG]: ", buffer );
}

void show_debug_information_function( const char *message, ... )
{
	char buffer[ 2048 ];
	va_list args;
	va_start( args, message );
	vsnprintf( buffer, array_length( buffer ), message, args );
	show_debug_function( FOREGROUND_GREEN | FOREGROUND_BLUE, "[ INFO]: ", buffer );
	va_end( args );
}

void show_debug_information_function_ext( const char *message, va_list args )
{
	char buffer[ 2048 ];
	vsnprintf( buffer, array_length( buffer ), message, args );
	show_debug_function( FOREGROUND_GREEN | FOREGROUND_BLUE, "[ INFO]: ", buffer );
}

void show_debug_warning_function( const char *message, ... )
{
	char buffer[ 2048 ];
	va_list args;
	va_start( args, message );
	vsnprintf( buffer, array_length( buffer ), message, args );
	show_debug_function( FOREGROUND_RED | FOREGROUND_GREEN, "[ WARN]: ", buffer );
	va_end( args );
}

void show_debug_warning_function_ext( const char *message, va_list args )
{
	char buffer[ 2048 ];
	vsnprintf( buffer, array_length( buffer ), message, args );
	show_debug_function( FOREGROUND_RED | FOREGROUND_GREEN, "[ WARN]: ", buffer );
}

void show_debug_error_function( const char *message, ... )
{
	char buffer[ 2048 ];
	va_list args;
	va_start( args, message );
	vsnprintf( buffer, array_length( buffer ), message, args );
	show_debug_function( FOREGROUND_RED, "[ERROR]: ", buffer );
	va_end( args );
}

void show_debug_error_function_ext( const char *message, va_list args )
{
	char buffer[ 2048 ];
	vsnprintf( buffer, array_length( buffer ), message, args );
	show_debug_function( FOREGROUND_RED, "[ERROR]: ", buffer );
}

void platform_print_error()
{
	int e = GetLastError();
	LPTSTR error_text = nullptr;

	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, 0, e, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&error_text, 0, 0 );

	if ( error_text )
	{
		show_debug_error_function( error_text );
		LocalFree( error_text );
	}
}

// Memory
inline void platform_memmove( void *dst, void *src, u64 bytes )
{
	memmove( dst, src, bytes );
}

inline void platform_memcpy( void *dst, void *src, u64 bytes )
{
	memcpy( dst, src, bytes );
}

inline void platform_memzero( void *dst, u64 bytes )
{
	memset( dst, 0, bytes );
}

inline void platform_memset( void *dst, u8 value, u64 bytes )
{
	memset( dst, value, bytes );
}

// Timing
/// @desc Returns how many tick counts happen per second. Cannot change after system boot. Only need once and cache.
[[nodiscard]] inline u64 platform_get_tick_frequency()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency( &freq );
	return freq.QuadPart;
}

[[nodiscard]] inline u64 platform_get_tick_counter()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter( &counter );
	return counter.QuadPart;
}

[[nodiscard]] inline u64 platform_get_cycle_counter()
{
	return __rdtsc();
}

// Directory
bool platform_create_directory( const char *directory )
{
	char pathMem[ 4096 ];
	if ( string_utf8_copy( pathMem, directory ) == 0 )
		return false;

	char *path = pathMem;
	const char *token;
	const char *delimiters = "./\\";
	char delim;
	char dir[ 4096 ] = "\0";

	if ( *path == '.' )
	{
		++path;

		if ( *path == '/' || *path == '\\' )
		{
			// ./ (current directory)
			string_utf8_append( dir, "./" );
		}
		else if ( *path == '.' )
		{
			++path;

			// ../ (moving up from current directory)
			if ( *path == '/' || *path == '\\' )
			{
				// ./ (current directory)
				string_utf8_append( dir, "../" );
			}
		}
	}

	path = string_utf8_tokenise( path, delimiters, &token, &delim );

	while ( token )
	{
		if ( delim == '.' )
		{
			// file ext
			return true;
		}

		string_utf8_append( dir, token );
		string_utf8_append( dir, "/" );

		if ( !CreateDirectory( dir, nullptr ) )
			return false;

		path = string_utf8_tokenise( path, delimiters, &token, &delim );
	}

	return true;
}

inline void platform_set_current_directory( const char *workingDirectory )
{
	SetCurrentDirectory( workingDirectory );
}

// Files
[[nodiscard]] u8 *platform_read_file( const char *path, u64 *fileSize, bool addNullTerminator, MemoryArena *arena )
{
	HANDLE file = CreateFile( path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

	if ( file == INVALID_HANDLE_VALUE )
	{
		show_log_warning( "\"platform_read_file\": Failed to open file: %s", path );
		return nullptr;
	}

	LARGE_INTEGER size;

	if ( !GetFileSizeEx( file, &size ) )
	{
		CloseHandle( file );
		show_log_warning( "\"platform_read_file\": Failed to get size for file: %s", path );
		return nullptr;
	}

	DWORD fileSizeInBytes = (DWORD)size.QuadPart;
	DWORD bytesRead;

	u8 *buffer = memory_arena_transient_allocate( arena, fileSizeInBytes + ( addNullTerminator ? 1 : 0 ) );

	if ( !buffer )
	{
		CloseHandle( file );
		show_log_warning( "\"platform_read_file\": Failed to allocate memory ( %d bytes )", fileSizeInBytes );
		return nullptr;
	}

	if ( !ReadFile( file, buffer, fileSizeInBytes, &bytesRead, 0 ) || fileSizeInBytes != bytesRead )
	{
		CloseHandle( file );
		memory_arena_transient_free( arena, buffer );
		show_log_warning( "\"platform_read_file\": Failed to read file: %s", path );
		return nullptr;
	}

	if ( addNullTerminator )
	{
		buffer[ bytesRead ] = '\0';
		bytesRead += 1;
	}

	if ( fileSize )
		*fileSize = bytesRead;

	CloseHandle( file );

	return buffer;
}

u64 platform_write_file( const char *path, u8 *buffer, u64 size, bool append )
{
	HANDLE file = CreateFile( path, append ? FILE_APPEND_DATA : GENERIC_WRITE, FILE_SHARE_WRITE, 0, append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

	if ( file == INVALID_HANDLE_VALUE )
	{
		show_log_warning( "\"platform_write_file\": Failed to open file: %s", path );
		return 0;
	}

	if ( append )
	{
		DWORD result = SetFilePointer( file, 0, 0, FILE_END );

		if ( result == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		{
			CloseHandle( file );
			show_log_warning( "\"platform_write_file\": Failed to set pointer to end of file: %s", path );
			return 0;
		}
	}

	massert( size < UINT32_MAX );

	DWORD bytesWritten;

	BOOL result = WriteFile( file, buffer, static_cast<u32>( size ), &bytesWritten, 0 );

	CloseHandle( file );

	if ( !result || size != bytesWritten )
	{
		show_log_warning( "\"platform_write_file\": Failed to write file: %s", path );
		return 0;
	}

	return bytesWritten;
}

u32 platform_open_file( const char *path, FileOptions options )
{
	DWORD access = 0;
	DWORD flags = OPEN_EXISTING;
	DWORD shareMode = 0;

	if ( options & FILE_OPTION_READ )	{ access |= GENERIC_READ; shareMode |= FILE_SHARE_READ; }
	if ( options & FILE_OPTION_WRITE )	{ access |= GENERIC_WRITE; shareMode |= FILE_SHARE_WRITE; }
	if ( options & FILE_OPTION_CREATE )	{ flags = OPEN_ALWAYS; }
	if ( options & FILE_OPTION_CLEAR )	{ flags = CREATE_ALWAYS; }

	HANDLE file = CreateFile( path, access, shareMode, 0, flags, FILE_ATTRIBUTE_NORMAL, 0 );

	if ( file == INVALID_HANDLE_VALUE )
	{
		show_log_warning( "\"platform_open_file\": Failed to open file: %s", path );
		return INVALID_INDEX_UINT_32;
	}

	if ( options & FILE_OPTION_APPEND )
	{
		DWORD result = SetFilePointer( file, 0, 0, FILE_END );

		if ( result == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		{
			CloseHandle( file );
			show_log_warning( "\"platform_open_file\": Failed to set pointer to end of file: %s", path );
			return INVALID_INDEX_UINT_32;
		}
	}

	u32 fileID = freeOpenFileIDs.pop();
	allOpenFiles[ fileID ] = file;
	return fileID;
}

void platform_close_file( u32 fileID )
{
	CloseHandle( allOpenFiles[ fileID ] );
	freeOpenFileIDs.add( fileID );
}

u64 platform_get_file_size( u32 fileID )
{
	LARGE_INTEGER size;

	if ( !GetFileSizeEx( allOpenFiles[ fileID ], &size ) )
	{
		show_log_warning( "Failed to get size for fileID: %d", fileID );
		return 0;
	}

	return size.QuadPart;
}

void platform_seek_in_file( u32 fileID, FileSeek seek, u64 offset )
{
	DWORD move;

	switch ( seek )
	{
	case FILE_SEEK_START:	move = FILE_BEGIN;		break;
	case FILE_SEEK_CURRENT:	move = FILE_CURRENT;	break;
	case FILE_SEEK_END:		move = FILE_END;		break;
	default:
		show_log_warning( "Function received unknown seek: %d", seek );
		return;
	}

	LARGE_INTEGER li;

	li.QuadPart = offset;

	DWORD result = SetFilePointer( allOpenFiles[ fileID ], li.LowPart, &li.HighPart, move );

	if ( result == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		show_log_warning( "Failed to seek pointer in fileID: %d", fileID );
}

u8 *platform_read_whole_file( u32 fileID, MemoryArena *arena, bool addNullTerminator )
{
	u64 fileSize = platform_get_file_size( fileID );

	if ( fileSize == 0 )
		return nullptr;

	u8 *buffer = memory_arena_transient_allocate( arena, fileSize + addNullTerminator );

	platform_seek_in_file( fileID, FILE_SEEK_START, 0 );

	u64 bytesRead = platform_read_from_file( fileID, buffer, fileSize );

	if ( bytesRead != fileSize )
	{
		memory_arena_transient_free( arena, buffer );
		return nullptr;
	}

	if ( addNullTerminator )
		buffer[ fileSize ] = '\0';

	return buffer;
}

u8 *platform_read_image( const char *filename, u32 *width, u32 *height, u32 *channels )
{
	massert( *channels <= 4 );

	int w, h, c;

	u8 *data = (u8 *)stbi_load( filename, &w, &h, &c, *channels );

	if ( !data )
	{
		show_log_warning( "\"platform_read_image\": Failed to open file: \"%s\"", filename );
		return nullptr;
	}

	*width = static_cast<u32>( w );
	*height = static_cast<u32>( h );

	// stbi_load channel will always return as what it would have been if the final param was 0
	// so only set the channel if it was 0, else it will actually be whatever was requested
	if ( *channels == 0 )
		*channels = static_cast<u32>( c );

	return data;
}

u64 platform_read_from_file( u32 fileID, void *buffer, u64 size )
{
	HANDLE file = allOpenFiles[ fileID ];

	massert( size < UINT32_MAX );

	DWORD bytesRead;

	if ( !ReadFile( file, buffer, static_cast<u32>( size ), &bytesRead, 0 ) )
	{
		show_log_warning( "Failed to read from fileID: %d", fileID );
		return 0;
	}

	return bytesRead;
}

u64 platform_write_to_file( u32 fileID, void *buffer, u64 size )
{
	HANDLE file = allOpenFiles[ fileID ];

	massert( size < UINT32_MAX );

	DWORD bytesWritten;

	BOOL result = WriteFile( file, buffer, static_cast<u32>( size ), &bytesWritten, 0 );

	if ( !result || size != bytesWritten )
	{
		show_log_warning( "Failed to write to fileID: %d", fileID );
		return 0;
	}

	return bytesWritten;
}

[[nodiscard]] inline bool platform_file_exists( const char *path )
{
	DWORD attributes = GetFileAttributes( path );
	return ( attributes != INVALID_FILE_ATTRIBUTES && !( attributes & FILE_ATTRIBUTE_DIRECTORY ) );
}

inline bool platform_delete_file( const char *path )
{
	return DeleteFileA( path ) != 0;
}

[[nodiscard]] u64 platform_last_edit_timestamp( const char *path )
{
	HANDLE file = CreateFile( path, GENERIC_READ, FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

	if ( file == INVALID_HANDLE_VALUE )
	{
		return 0;
	}

	FILETIME writeTime;
	if ( !GetFileTime( file, 0, 0, &writeTime ) )
	{
		CloseHandle( file );
		return 0;
	}

	CloseHandle( file );

	return ULARGE_INTEGER{ writeTime.dwLowDateTime, writeTime.dwHighDateTime }.QuadPart;
}

inline void platform_copy_file( const char *from, const char *to )
{
	while ( !CopyFile( from, to, false ) )
	{
		Sleep( 10 );
	}
}

inline void platform_initialise()
{
	allOpenFiles.set_full();
	freeOpenFileIDs.clear();
	for ( int i = (int)freeOpenFileIDs.capacity() - 1; i >= 0; --i )
		freeOpenFileIDs.add( i );
}