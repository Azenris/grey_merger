
// System Includes
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <assert.h>

// Platform Specific Includes
#ifdef PLATFORM_WINDOWS
	#include <direct.h>
	#include "dirent/dirent.h"
#else
	#include <sys/stat.h>
	#include <unistd.h>
	#include <dirent.h>
#endif

// Third Party Includes
#include "stb_image.h"
#include "stb_image_write.h"

// Includes
#include "defines.h"
#include "array.h"
#include "map.h"
#include "memory_arena.h"
#include "error_codes.h"
#include "image.h"

struct App
{
	MemoryArena memory;

} app;

struct ImageChannel
{
	u8 *image = nullptr;
	u32 w = 0;
	u32 h = 0;
	u32 channels = 0;
	u64 size = 0;
};

struct Options
{
	u64 memory = MB( 16 );
	const char *programName = "grey_merger.exe";
	const char *workingDirectory = nullptr;
	bool verbose = false;
	char inputFileR[ 4096 ];
	char inputFileG[ 4096 ];
	char inputFileB[ 4096 ];
	bool redChannel = false;
	bool greenChannel = false;
	bool blueChannel = false;
	char outputFile[ 4096 ];

} options;

static void log( const char *message, ... )
{
	va_list args;
	va_start( args, message );
	vfprintf( stdout, message, args );
	va_end( args );
	fprintf( stderr, "\n" );
}

static void log_warning( const char *message, ... )
{
	va_list args;
	va_start( args, message );
	vfprintf( stderr, message, args );
	va_end( args );
	fprintf( stderr, "\n" );
}

static void log_error( const char *message, ... )
{
	va_list args;
	va_start( args, message );
	vfprintf( stderr, message, args );
	va_end( args );
	fprintf( stderr, "\n" );
}

static int usage_message( RESULT_CODE code )
{
	log( "\nERROR_CODE: %s\n", error_code_string( code ) );
	log( ":: USAGE ::" );
	log( "Expects %s <commands>", options.programName );
	log( "Eg. %s %s\n", options.programName, "-channel-r test\\image_0.png -channel-b test\\image_1.png -o test\\merged.png" );
	log( "COMMANDS" );
	log( "[-v]                         EG. -v                                             (enable verbose outputs)" );
	log( "[-ra]                        EG. -ra                                            (outputs received arguments)" );
	log( "[-wd] <path>                 EG. -wd TEMP\\                                      (override the default working directory)" );
	log( "[-channel-r] <file>          EG. -channel-r assets\\image\\image_r.png        (input file for red channel)" );
	log( "[-channel-g] <file>          EG. -channel-g assets\\image\\image_g.png        (input file for green channel)" );
	log( "[-channel-b] <file>          EG. -channel-b assets\\image\\image_b.png        (input file for blue channel)" );
	log( "[-o] <file>                  EG. -o assets\\image\\mergedimg.png                  (override the default output file)" );
	log( "[-memory] <bytes>            EG. -memory 1024                                   (specify memory allocation))" );

	return code;
}

static u64 string_copy( char *destination, u64 destSize, const char *source )
{
	assert( destSize >= 1 );
	assert( source );

	const char *sourceStart = source;

	while ( *source != '\0' && --destSize > 0 )
	{
		*destination++ = *source++;
	}

	*destination = '\0';

	return source - sourceStart;
}

static u64 string_append( char *destination, u64 destSize, const char *append )
{
	assert( destination );
	assert( append );

	u64 bytes = 0;
	{
		const char *str = destination;
		while ( *str++ != '\0' )
		{
			bytes += 1;
		}
	}

	u64 appendBytes = 0;
	{
		const char *str = append;
		while ( *str++ != '\0' )
		{
			appendBytes += 1;
		}
		appendBytes += 1;
	}

	// Check there is enough room to append
	if ( ( destSize - bytes ) < appendBytes )
		return 0;

	strcpy( destination + bytes, append );

	// Doesn't include null terminator
	return appendBytes - 1;
}

[[nodiscard]] static u64 string_span( const char *tok, const char *delim )
{
	assert( tok && delim );

	u64 i, j;

	for ( i = 0; tok[ i ] != '\0'; ++i )
		for ( j = 0; delim[ j ] != '\0'; ++j )
			if ( tok[ i ] == delim[ j ] )
				return i;

	return i;
}

[[nodiscard]] static u64 string_nspan( const char *tok, const char *delim )
{
	assert( tok && delim );

	u64 i, j;

	for ( i = 0; tok[ i ] != '\0'; ++i )
	{
		bool delimFound = false;

		for ( j = 0; delim[ j ] != '\0'; ++j )
		{
			if ( tok[ i ] == delim[ j ] )
			{
				delimFound = true;
				break;
			}
		}

		if ( !delimFound )
			return i;
	}

	return i;
}

[[nodiscard]] static char *string_tokenise( char *str, const char *delim, const char **token, char *found )
{
	// Invalid input
	if ( !str )
	{
		*token = nullptr;
		return nullptr;
	}

	// Add a count of characters until a NON delimiter is found
	str += string_nspan( str, delim );

	// No NON delimiter found, reached end of string
	if ( *str == '\0' )
	{
		*token = nullptr;
		return nullptr;
	}

	*token = str;

	// Set e to str + a count of characters until a delimiter IS found
	char *e = str + string_span( str, delim );

	// If there was a break on \r && the next char is \n && you was looking for \r\n
	// Then return the delim found as \n
	u64 crlfSkip = 0;
	if ( *e == '\r' && *( e + 1 ) == '\n' )
	{
		crlfSkip = 1;
		if ( found )
			*found = '\n';
	}
	// Which delimiter was found
	else if ( found )
		*found = *e;

	// If it didn't reached the end of the string
	// it will need a null terminator inserting and
	// the new position of the string returned
	if ( *e != '\0' )
	{
		*e = '\0';
		return e + 1 + crlfSkip;
	}

	return nullptr;
}

static bool make_directory( const char *directory )
{
	char pathMem[ 4096 ];
	if ( string_copy( pathMem, sizeof( pathMem ), directory ) == 0 )
		return false;

	char *path = pathMem;
	const char *token;
	const char *delimiters = "./\\";
	char delim;
	char dir[ 4096 ] = "\0";

	if ( *path == '.' )
	{
		path += 1;

		if ( *path == '/' || *path == '\\' )
		{
			// ./ (current directory)
			string_append( dir, sizeof( dir ), "./" );
		}
		else if ( *path == '.' )
		{
			path += 1;

			// ../ (moving up from current directory)
			if ( *path == '/' || *path == '\\' )
			{
				// ./ (current directory)
				string_append( dir, sizeof( dir ), "../" );
			}
		}
	}

	path = string_tokenise( path, delimiters, &token, &delim );

	while ( token )
	{
		if ( delim == '.' )
		{
			// file ext
			return true;
		}

		string_append( dir, sizeof( dir ), token );
		string_append( dir, sizeof( dir ), "/" );

		#ifdef PLATFORM_WINDOWS
			_mkdir( dir );
		#else
			mkdir( dir, 0777 );
		#endif

		path = string_tokenise( path, delimiters, &token, &delim );
	}

	return true;
}

[[nodiscard]] static u8 *read_image( const char *filename, u32 *width, u32 *height, u32 *channels )
{
	assert( *channels <= 4 );

	int w, h, c;

	u8 *data = (u8 *)stbi_load( filename, &w, &h, &c, *channels );

	if ( !data )
	{
		log_warning( "\"read_image\": Failed to open file: \"%s\"", filename );
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

static RESULT_CODE read_channel_image( ImageChannel *imgChannel, const char *path, u32 *w, u32 *h )
{
	imgChannel->image = read_image( path, &imgChannel->w, &imgChannel->h, &imgChannel->channels );

	imgChannel->size = imgChannel->w * imgChannel->h * imgChannel->channels;

	if ( !imgChannel->image )
	{
		log_warning( "Failed to open file: %s", path );
		return RESULT_CODE_FAILED_TO_OPEN_INPUT_FILE;
	}
	if ( imgChannel->size <= 0 )
	{
		log_warning( "Failed to open file: %s ( 0 bytes )", path );
		return RESULT_CODE_FAILED_TO_OPEN_INPUT_FILE;
	}
	else if ( options.verbose )
	{
		log( "Read %d bytes for file: %s", imgChannel->size, path );
	}

	if ( *w == 0 )
		*w = imgChannel->w;

	if ( *h == 0 )
		*h = imgChannel->h;

	return RESULT_CODE_SUCCESS;
}

static bool change_directory( const char *directory )
{
	#ifdef PLATFORM_WINDOWS
		return _chdir( directory ) == 0;
	#else
		return chdir( directory ) == 0;
	#endif
}

// -------------------------------------------------------------------------
// ENTRY
// -------------------------------------------------------------------------
int main( int argc, const char *argv[] )
{
	options.inputFileR[ 0 ] = '\0';
	options.inputFileG[ 0 ] = '\0';
	options.inputFileB[ 0 ] = '\0';
	options.outputFile[ 0 ] = '\0';

	Map<const char *, RESULT_CODE(*)( int &, int, const char ** ), 256> commands;

	commands.insert( "-v", [] ( int &index, int argc, const char *argv[] )
		{
			options.verbose = true;

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-ra", [] ( int &index, int argc, const char *argv[] )
		{
			log( "Arguments received [#%d]", argc );
			for ( int i = 0; i < argc; ++i )
				log( " [%d] = %s", i, argv[ i ] );

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-wd", [] ( int &index, int argc, const char *argv[] )
		{
			options.workingDirectory = argv[ ++index ];

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-channel-r", [] ( int &index, int argc, const char *argv[] )
		{
			options.redChannel = true;

			string_copy( options.inputFileR, sizeof( options.inputFileR ), argv[ ++index ] );

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-channel-g", [] ( int &index, int argc, const char *argv[] )
		{
			options.greenChannel = true;

			string_copy( options.inputFileG, sizeof( options.inputFileG ), argv[ ++index ] );

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-channel-b", [] ( int &index, int argc, const char *argv[] )
		{
			options.blueChannel = true;

			string_copy( options.inputFileB, sizeof( options.inputFileB ), argv[ ++index ] );

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-o", [] ( int &index, int argc, const char *argv[] )
		{
			string_copy( options.outputFile, sizeof( options.outputFile ), argv[ ++index ] );

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-memory", [] ( int &index, int argc, const char *argv[] )
		{
			options.memory = atoi( argv[ ++index ] );

			return RESULT_CODE_SUCCESS;
		} );

	// Process the option commands
	for ( int i = 1; i < argc; ++i )
	{
		auto f = commands.find( argv[ i ] );

		if ( f )
		{
			RESULT_CODE code = f->value( i, argc, &argv[ 0 ] );
			if ( code != RESULT_CODE_SUCCESS )
				return usage_message( code );
		}
		else
		{
			log_warning( "Unknown command: %s", argv[ i ] );
			return usage_message( RESULT_CODE_UNKNOWN_OPTIONAL_COMMAND );
		}
	}

	app.memory =
	{
		.flags = 0,
		.memory = nullptr,
		.permanent =
		{
			.capacity = 0,
			.available = 0,
			.memory = nullptr,
			.lastAlloc = nullptr,
			.allocate_func = memory_bump_allocate,
			.reallocate_func = memory_bump_reallocate,
			.shrink_func = memory_bump_shrink,
			.free_func = memory_bump_free,
			.attach_func = memory_bump_attach,
		},
		.transient =
		{
			.capacity = 0,
			.available = 0,
			.memory = nullptr,
			.lastAlloc = nullptr,
			.allocate_func = memory_bump_allocate,
			.reallocate_func = memory_bump_reallocate,
			.shrink_func = memory_bump_shrink,
			.free_func = memory_bump_free,
			.attach_func = memory_bump_attach,
		},
		.fastBump =
		{
			.capacity = 0,
			.available = 0,
			.memory = nullptr,
			.lastAlloc = nullptr,
			.allocate_func = memory_fast_bump_allocate,
			.attach_func = nullptr,
		},
	};

	if ( !app.memory.init( 0, options.memory, 0, true ) )
	{
		log_error( "Failed to initialise memory app.memory" );
		return usage_message( RESULT_CODE_FAILED_MEMORY_ARENA_INITIALISATION );
	}

	// Set working directory
	if ( options.workingDirectory )
	{
		if ( change_directory( options.workingDirectory ) && options.verbose )
			log( "Working directory changed to: %s", options.workingDirectory );
	}

	if ( !options.redChannel && !options.greenChannel && !options.blueChannel )
	{
		return usage_message( RESULT_CODE_NO_INPUT_FILES );
	}

	if ( options.verbose )
	{
		if ( options.redChannel )
			log( "Channel Red Input file: %s", options.inputFileR );

		if ( options.greenChannel )
			log( "Channel Green Input file: %s", options.inputFileG );

		if ( options.blueChannel )
			log( "Channel Blue Input file: %s", options.inputFileB );
	}

	// Create an output filename if one was not provided
	if ( options.outputFile[ 0 ] == '\0' )
	{
		string_append( options.outputFile, sizeof( options.outputFile ), "output.png" );

		if ( options.verbose )
			log( "Output file automatically assigned filename: %s", options.outputFile );
	}

	// -----------------------------------------------------------------------------
	// Open the input files

	ImageChannel red;
	ImageChannel green;
	ImageChannel blue;

	u32 w = 0;
	u32 h = 0;

	if ( options.redChannel )
	{
		RESULT_CODE code = read_channel_image( &red, options.inputFileR, &w, &h );
		if ( code != RESULT_CODE_SUCCESS )
			return usage_message( code );
	}

	if ( options.greenChannel )
	{
		RESULT_CODE code = read_channel_image( &green, options.inputFileG, &w, &h );
		if ( code != RESULT_CODE_SUCCESS )
			return usage_message( code );
	}

	if ( options.blueChannel )
	{
		RESULT_CODE code = read_channel_image( &blue, options.inputFileB, &w, &h );
		if ( code != RESULT_CODE_SUCCESS )
			return usage_message( code );
	}

	bool rInvalid = options.redChannel && ( red.w != w || red.h != h );
	bool gInvalid = options.greenChannel && ( green.w != w || green.h != h );
	bool bInvalid = options.blueChannel && ( blue.w != w || blue.h != h );

	// Make sure they are all the same size
	if ( rInvalid || gInvalid || bInvalid )
	{
		return usage_message( RESULT_CODE_INPUT_FILE_SIZES_DONT_MATCH );
	}

	// Create the output data
	u32 outWidth = w;
	u32 outHeight = h;
	u32 outChannels = 4;
	u64 outSize = w * h * outChannels;
	u8 *outImage = app.memory.transient.allocate<u8>( outSize, true );

	if ( !outImage )
	{
		log_warning( "Failed to allocate &d bytes.", outSize );
		return usage_message( RESULT_CODE_FAILED_TO_ALLOCATE_MEMORY_FOR_OUTPUT_IMAGE );
	}

	u8 *image = outImage;
	u8 *rImage = red.image;
	u8 *gImage = green.image;
	u8 *bImage = blue.image;

	for ( u64 y = 0; y < outHeight; ++y )
	{
		for ( u64 x = 0; x < outWidth; ++x )
		{
			if ( options.redChannel )
			{
				*image++ = *rImage;
				rImage += red.channels;
			}
			else
			{
				*image++ = 0;
			}

			if ( options.greenChannel )
			{
				*image++ = *gImage;
				gImage += green.channels;
			}
			else
			{
				*image++ = 0;
			}

			if ( options.blueChannel )
			{
				*image++ = *bImage;
				bImage += blue.channels;
			}
			else
			{
				*image++ = 0;
			}

			*image++ = 255;
		}
	}

	if ( options.verbose )
		log( "Finished creating image. Preparing to save to disk." );

	make_directory( options.outputFile );

	if ( !stbi_write_png( options.outputFile, outWidth, outHeight, outChannels, outImage, outWidth * outChannels ) )
	{
		log_warning( "Failed to create output image: %s", options.outputFile );
		return usage_message( RESULT_CODE_FAILED_TO_CREATE_OUTPUT_FILE );
	}
	else if ( options.verbose )
		log( "Successfully created output image[ %d x %d ]: %s", outWidth, outHeight, options.outputFile );

	return RESULT_CODE_SUCCESS;
}

// -------------------------------------------------------------------------
// Unity Build

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"