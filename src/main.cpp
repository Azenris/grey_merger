
// System includes
#include <stdarg.h>
#include <cmath>

// Includes
#include "defines.h"
#include "logging.h"
#include "platform.h"
#include "zlib\zlib.h"
#include "array.h"
#include "strings.h"
#include "map.h"
#include "memory_arena.h"
#include "utility.h"
#include "error_codes.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "platform_windows.cpp"

MemoryArena arena;

#include "image.h"

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

static int usage_message( RESULT_CODE code )
{
	show_log_warning( "\nERROR_CODE: %s\n", error_code_string( code ) );
	show_log_info( ":: USAGE ::" );
	show_log_message( "Expects %s <commands>", options.programName );
	show_log_message( "Eg. %s %s\n", options.programName, "-channel-r test\\image_0.png -channel-b test\\image_1.png -o test\\merged.png" );
	show_log_info( "COMMANDS" );
	show_log_message( "[-v]                         EG. -v                                             (enable verbose outputs)" );
	show_log_message( "[-ra]                        EG. -ra                                            (outputs received arguments)" );
	show_log_message( "[-wd] <path>                 EG. -wd TEMP\\                                      (override the default working directory)" );
	show_log_message( "[-channel-r] <file>          EG. -channel-r assets\\image\\image_r.png        (input file for red channel)" );
	show_log_message( "[-channel-g] <file>          EG. -channel-g assets\\image\\image_g.png        (input file for green channel)" );
	show_log_message( "[-channel-b] <file>          EG. -channel-b assets\\image\\image_b.png        (input file for blue channel)" );
	show_log_message( "[-o] <file>                  EG. -o assets\\image\\mergedimg.png                  (override the default output file)" );
	show_log_message( "[-memory] <bytes>            EG. -memory 1024                                   (specify memory allocation))" );

#ifdef DEBUG
	system( "pause" );
#endif

	return code;
}

RESULT_CODE read_channel_image( ImageChannel *imgChannel, const char *path, u32 *w, u32 *h )
{
	imgChannel->image = platform_read_image( path, &imgChannel->w, &imgChannel->h, &imgChannel->channels );

	imgChannel->size = imgChannel->w * imgChannel->h * imgChannel->channels;

	if ( !imgChannel->image )
	{
		show_log_warning( "Failed to open file: %s", path );
		return RESULT_CODE_FAILED_TO_OPEN_INPUT_FILE;
	}
	if ( imgChannel->size <= 0 )
	{
		show_log_warning( "Failed to open file: %s ( 0 bytes )", path );
		return RESULT_CODE_FAILED_TO_OPEN_INPUT_FILE;
	}
	else if ( options.verbose )
	{
		show_log_message( "Read %d bytes for file: %s", imgChannel->size, path );
	}

	if ( *w == 0 )
		*w = imgChannel->w;

	if ( *h == 0 )
		*h = imgChannel->h;

	return RESULT_CODE_SUCCESS;
}

// -------------------------------------------------------------------------
// ENTRY
// -------------------------------------------------------------------------
#ifdef DEBUG
int main( int _argc, const char *_argv[] )
{
	const char *argv[] = { "grey_merger.exe",
		"-channel-r", "debug\\test\\image_0.png",
		"-channel-g", "debug\\test\\image_1.png",
		"-channel-b", "debug\\test\\image_2.png",
		"-v", "-ra"
		, "-wd", "C:\\ProgrammingCDrive\\grey_merger"
		, "-o", "debug\\test\\output.png" };
	int argc = array_length( argv );
#else
int main( int argc, const char *argv[] )
{
#endif

	platform_initialise();

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
			show_log_message( "Arguments received [#%d]", argc );
			for ( int i = 0; i < argc; ++i )
				show_log_info( " [%d] = %s", i, argv[ i ] );

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

			string_utf8_copy( options.inputFileR, argv[ ++index ] );

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-channel-g", [] ( int &index, int argc, const char *argv[] )
		{
			options.greenChannel = true;

			string_utf8_copy( options.inputFileG, argv[ ++index ] );

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-channel-b", [] ( int &index, int argc, const char *argv[] )
		{
			options.blueChannel = true;

			string_utf8_copy( options.inputFileB, argv[ ++index ] );

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-o", [] ( int &index, int argc, const char *argv[] )
		{
			string_utf8_copy( options.outputFile, argv[ ++index ] );

			return RESULT_CODE_SUCCESS;
		} );

	commands.insert( "-memory", [] ( int &index, int argc, const char *argv[] )
		{
			options.memory = convert_to_int( argv[ ++index ] );

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
			show_log_warning( "Unknown command: %s", argv[ i ] );
			return usage_message( RESULT_CODE_UNKNOWN_OPTIONAL_COMMAND );
		}
	}

	if ( !memory_arena_initialise( &arena, 0, options.memory ) )
	{
		show_log_error( "Failed to initialise memory arena" );
		return usage_message( RESULT_CODE_FAILED_MEMORY_ARENA_INITIALISATION );
	}

	// Set working directory
	if ( options.workingDirectory )
	{
		platform_set_current_directory( options.workingDirectory );

		if ( options.verbose )
			show_log_info( "Working directory changed to: %s", options.workingDirectory );
	}

	if ( !options.redChannel && !options.greenChannel && !options.blueChannel )
	{
		return usage_message( RESULT_CODE_NO_INPUT_FILES );
	}

	if ( options.verbose )
	{
		if ( options.redChannel )
			show_log_info( "Channel Red Input file: %s", options.inputFileR );

		if ( options.greenChannel )
			show_log_info( "Channel Green Input file: %s", options.inputFileG );

		if ( options.blueChannel )
			show_log_info( "Channel Blue Input file: %s", options.inputFileB );
	}

	// Create an output filename if one was not provided
	if ( options.outputFile[ 0 ] == '\0' )
	{
		string_utf8_append( options.outputFile, "output.png" );

		if ( options.verbose )
			show_log_info( "Output file automatically assigned filename: %s", options.outputFile );
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
	u8 *outImage = memory_arena_transient_allocate( &arena, outSize, true );

	if ( !outImage )
	{
		show_log_warning( "Failed to allocate &d bytes.", outSize );
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
		show_log_message( "Finished creating image. Preparing to save to disk." );

	platform_create_directory( options.outputFile );

	if ( !stbi_write_png( options.outputFile, outWidth, outHeight, outChannels, outImage, outWidth * outChannels ) )
	{
		show_log_warning( "Failed to create output image: %s", options.outputFile );
		return usage_message( RESULT_CODE_FAILED_TO_CREATE_OUTPUT_FILE );
	}
	else if ( options.verbose )
		show_log_message( "Successfully created output image[ %d x %d ]: %s", outWidth, outHeight, options.outputFile );

#ifdef DEBUG
	system( "pause");
#endif

	return RESULT_CODE_SUCCESS;
}