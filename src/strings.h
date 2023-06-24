
#pragma once

[[nodiscard]] inline char ascii_char_upper( char c )
{
	return c >= 'a' && c <= 'z' ? c - 32 : c;
}

[[nodiscard]] inline char ascii_char_lower( char c )
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

// UTF-8 ////////////////////////////////////////////////////////////////////////////////
struct utf8Character
{
	char data[ 8 ];
};

template <u64 destSize>
int string_utf8_format( char( &destination )[ destSize ], const char *format, ... )
{
	massert( format );

	va_list args;
	va_start( args, format );

	int result = vsnprintf( destination, destSize, format, args );

	va_end( args );

	if ( result > destSize )
	{
		show_debug_warning( "string_utf8_format destination buffer too small. [ %d / %d ]", result, destSize );
		if ( result == destSize )
			show_debug_warning( "Make sure there is enough room for the ending null terminator" );
		destination[ 0 ] = '\0';
		return -1;
	}

	return result;
}

int string_utf8_format( char *destination, u64 destSize, const char *format, ... )
{
	massert( destination && format );

	va_list args;
	va_start( args, format );

	int result = vsnprintf( destination, destSize, format, args );

	va_end( args );

	if ( result >= destSize )
	{
		show_debug_warning( "string_utf8_format destination buffer too small. [ %d / %d ]", result, destSize );
		if ( result == destSize )
			show_debug_warning( "Make sure there is enough room for the ending null terminator" );
		destination[ 0 ] = '\0';
		return -1;
	}

	return result;
}

/// @desc Return the bytes of the string (INCLUDING the null terminator)
/// @return Bytes
[[nodiscard]] inline u64 string_utf8_bytes( const char *str )
{
	massert( str );

	u64 bytes = 0;

	while ( *str++ != '\0' )
		++bytes;

	return bytes + 1;
}

/// @desc Return the length of the string (NOT including the NULL terminator) (Note length != bytes)
/// @return Length
[[nodiscard]] u64 string_utf8_length( const char *str )
{
	u64 length = 0;
	int i = 0;
	char c = str[ i++ ];

	while ( c )
	{
		if ( c >= 0 && c < 127 )			// 1-byte : 0___ ____
		{
		}
		else if ( ( c & 0xE0 ) == 0xC0 )	// 2-byte : 11__ ____
		{
			++i;
		}
		else if ( ( c & 0xF0 ) == 0xE0 )	// 3-byte : 111_ ____
		{
			i += 2;
		}
		else if ( ( c & 0xF8 ) == 0xF0 )	// 4-byte : 1111 ____
		{
			i += 3;
		}
		else if ( ( c & 0xFC ) == 0xF8 )	// 5-byte : 1111 1___
		{
			i += 4;
		}
		else if ( ( c & 0xFE ) == 0xFC )	// 6-byte : 1111 1___
		{
			i += 5;
		}

		++length;
		c = str[ i++ ];
	}

	return length;
}

/// @return bytes written (NOT including the NULL terminator)
u64 string_utf8_copy( char *destination, u64 destSize, const char *source )
{
	massert( destSize >= 1, "\"string_utf8_copy\" failed. Invalid destSize Size." );
	massert( source, "\"string_utf8_copy\" failed. Null pointer is passed for source" );
	massert( destSize >= string_utf8_bytes( source ), "\"string_utf8_copy\" failed. Destination array is too small[ %d ]: %s", destSize, source );

	const char *sourceStart = source;

	while ( *source != '\0' )
		*destination++ = *source++;

	*destination = '\0';

	return source - sourceStart;
}

/// @return bytes written (NOT including the null terminator)
template <u64 destSize>
inline u64 string_utf8_copy( char( &destination )[ destSize ], const char *source )
{
	massert_static( destSize >= 1, "\"string_utf8_copy\" failed. Invalid destSize Size." );
	return string_utf8_copy( destination, destSize, source );
}

/// @desc Copies a number of bytes from source to destination and adds an ending null terminator. destSize should be at minimum bytes + 1
/// @return bytes written (NOT including the null terminator)
inline u64 string_utf8_copy( char *destination, u64 destSize, const char *source, u64 bytes )
{
	massert( destination, "\"string_utf8_copy\" failed. Null pointer is passed for destination" );
	massert( destSize >= 1, "\"string_utf8_copy\" failed. Invalid Size: %d", destSize );
	massert( source, "\"string_utf8_copy\" failed. Null pointer is passed for source" );
	massert( destSize >= bytes + 1, "\"string_utf8_copy\" failed. Destination array is too small[ %d ]: %s", destSize, source );

	memcpy( destination, source, bytes );
	destination[ bytes ] = '\0';

	return bytes;
}

/// @desc Copies a number of bytes from source to destination and adds an ending null terminator. destSize should be at minimum bytes + 1
/// @return bytes written (NOT including the null terminator)
template <u64 destSize>
inline u64 string_utf8_copy( char( &destination )[ destSize ], const char *source, u64 bytes )
{
	massert_static( destSize >= 1, "\"string_utf8_copy\" failed. Invalid destSize Size." );
	return string_utf8_copy( destination, destSize, source, bytes );
}

[[nodiscard]] inline bool string_utf8_compare( const char *lhs, const char *rhs )
{
	massert( lhs );
	massert( rhs );

	while ( *lhs != '\0' )
		if ( *lhs++ != *rhs++ )
			return false;

	return *lhs == *rhs;
}

template <u64 lhsSize, u64 rhsSize>
[[nodiscard]] constexpr inline bool string_utf8_compare( const char( &lhs )[ lhsSize ], const char( &rhs )[ rhsSize ] )
{
	return string_utf8_compare( &lhs[ 0 ], &rhs[ 0 ] );
}

[[nodiscard]] u32 string_utf8_codepoint( const char *str, u32 *pSize )
{
	massert( pSize );

	char c = str[ 0 ];

	if ( c >= 0 && c < 127 )
	{
		// single-byte
		*pSize = 1;
		return c;
	}
	else if ( ( c & 0xE0 ) == 0xC0 )
	{
		// 2-byte
		u32 codepoint = ( ( str[ 0 ] & 0x1F ) << 6 )
			| ( ( str[ 1 ] & 0x3F ) );

		*pSize = 2;

		return codepoint;
	}
	else if ( ( c & 0xF0 ) == 0xE0 )
	{
		// 3-byte
		u32 codepoint = ( ( str[ 0 ] & 0xF ) << 12 )
			| ( ( str[ 1 ] & 0x3F ) << 6 )
			| ( ( str[ 2 ] & 0x3F ) );

		*pSize = 3;

		return codepoint;
	}
	else if ( ( c & 0xF8 ) == 0xF0 )
	{
		// 4-byte
		u32 codepoint = ( ( str[ 0 ] & 0x7 ) << 18 )
			| ( ( str[ 1 ] & 0x3F ) << 12 )
			| ( ( str[ 2 ] & 0x3F ) << 6 )
			| ( ( str[ 3 ] & 0x3F ) );

		*pSize = 4;

		return codepoint;
	}

	// TODO : 5 & 6 byte characters not supported
	show_log_warning( "Function \"string_utf8_codepoint\" failed. UTF-8 5 & 6 byte characters not supported." );
	*pSize = 0;
	return 0;
}

[[nodiscard]] utf8Character string_utf8_encode( u32 codepoint )
{
	utf8Character character;

	if ( codepoint <= 0x7F )
	{
		// single-byte
		character.data[ 0 ] = static_cast<char>( codepoint );
		character.data[ 1 ] = '\0';
		return character;
	}
	else if ( codepoint <= 0x07FF )
	{
		// 2-byte
		character.data[ 0 ] = static_cast<char>( ( ( codepoint >> 6 ) & 0x1F ) | 0xC0 );
		character.data[ 1 ] = static_cast<char>( ( ( codepoint >> 0 ) & 0x3F ) | 0x80 );
		character.data[ 2 ] = '\0';
		return character;
	}
	else if ( codepoint <= 0xFFFF )
	{
		// 3-byte
		character.data[ 0 ] = static_cast<char>( ( ( codepoint >> 12 ) & 0x0F ) | 0xE0 );
		character.data[ 1 ] = static_cast<char>( ( ( codepoint >> 6 ) & 0x3F ) | 0x80 );
		character.data[ 2 ] = static_cast<char>( ( ( codepoint >> 0 ) & 0x3F ) | 0x80 );
		character.data[ 3 ] = '\0';
		return character;
	}
	else if ( codepoint <= 0x10FFFF )
	{
		// 4-byte
		character.data[ 0 ] = static_cast<char>( ( ( codepoint >> 18 ) & 0x07) | 0xF0 );
		character.data[ 1 ] = static_cast<char>( ( ( codepoint >> 12 ) & 0x3F) | 0x80 );
		character.data[ 2 ] = static_cast<char>( ( ( codepoint >> 6 ) & 0x3F) | 0x80 );
		character.data[ 3 ] = static_cast<char>( ( ( codepoint >> 0 ) & 0x3F) | 0x80 );
		character.data[ 4 ] = '\0';
		return character;
	}

	// TODO : 5 & 6 byte characters not supported
	show_log_warning( "Function \"string_utf8_from_codepoint\" failed. UTF-8 5 & 6 byte characters not supported." );
	character.data[ 0 ] = '\0';
	return character;
}

[[nodiscard]] inline bool string_utf8_is_ascii( const char *str )
{
	return ( *str & 0b10000000 ) == 0;
}

[[nodiscard]] bool string_utf8_is_number( const char *str, bool *integer )
{
	if ( !str || *str == '\0' )
	{
		*integer = false;
		return false;
	}

	char c = *str++;
	int digits = 0;
	bool frac = false;

	// While its ascii keep checking
	while ( ( c & 0b10000000 ) == 0 )
	{
		if ( c >= '0' && c <= '9' )
		{
			digits += !frac;
		}
		else if ( c == '.' )
		{
			frac = true;
		}
		else if ( c == '\0' )
		{
			*integer = ( digits > 0 && !frac );
			return digits > 0;
		}
		else
		{
			*integer = false;
			return false;
		}

		c = *str++;
	}

	*integer = false;
	return false;
}

// utf8 first byte, if MSB is 0, its ASCII
// if the MSB is 1, then, code the 1's to determine the byte size
// eg. 110_ ____ , 10__ ____ the first byte shows there is 2 bytes ( 2 1's )
// the first byte 5 bits are used for the codepoint
// the seconds byte starts with a 10, with the remaining 6 bits for the codepoint
// 4 byte example : 1111 0___ , 10__ ____ , 10__ ____ , 10__ ____

[[nodiscard]] inline bool string_utf8_is_leading_byte( char c )
{
	bool isASCII = ( c & 0b10000000 ) == 0;				// if first bit is not set, then its an ASCII character ( always leading )
	bool isLeadingMultibyte = ( c & 0b01000000 ) != 0;	// non-leading multi-byte characters start with 10__ ____ ( so if 1 is set, it can't be leading )
	return isASCII || isLeadingMultibyte;
}

char *string_utf8_skip_codepoint( char *str, u32 *pSize, int num )
{
	if ( num <= 0 )
	{
		*pSize = 0;
		return str;
	}

	int length = 0;
	int i = 0;
	char c = str[ i++ ];

	while ( c )
	{
		if ( c >= 0 && c < 127 )			// 1-byte : 0___ ____
		{
		}
		else if ( ( c & 0xE0 ) == 0xC0 )	// 2-byte : 11__ ____
		{
			++i;
		}
		else if ( ( c & 0xF0 ) == 0xE0 )	// 3-byte : 111_ ____
		{
			i += 2;
		}
		else if ( ( c & 0xF8 ) == 0xF0 )	// 4-byte : 1111 ____
		{
			i += 3;
		}
		else if ( ( c & 0xFC ) == 0xF8 )	// 5-byte : 1111 1___
		{
			i += 4;
		}
		else if ( ( c & 0xFE ) == 0xFC )	// 6-byte : 1111 1___
		{
			i += 5;
		}

		if ( ++length >= num )
		{
			*pSize = i;
			return &str[ i ];
		}

		c = str[ i++ ];
	}

	*pSize = i;

	return &str[ i - 1 ];
}

void string_utf8_delete( char *str, int position )
{
	u32 size;
	str = string_utf8_skip_codepoint( str, &size, position );
	string_utf8_copy( str, string_utf8_bytes( str ), string_utf8_skip_codepoint( str, &size, 1 ) );
}

void string_utf8_pop( char *str )
{
	u64 len = string_utf8_bytes( str ) - 1;
	char *txt = str + len;

	while ( !string_utf8_is_leading_byte( *txt-- ) )
		--len;
	--len;

	if ( len >= 0 )
		str[ len ] = '\0';
}

void string_utf8_pop( char *str, int num )
{
	u64 len = string_utf8_bytes( str ) - 1;
	char *txt = str + len;

	for ( int i = 0; i < num; ++i )
	{
		while ( !string_utf8_is_leading_byte( *txt-- ) )
			--len;
		--len;
		--txt;
	}

	if ( len >= 0 )
		str[ len ] = '\0';
}

void string_utf8_trim_ext( char *str )
{
	u64 len = string_utf8_bytes( str ) - 1;
	char *txt = str + len;

	while ( len >= 0 )
	{
		if ( *txt == '.' )
		{
			*txt = '\0';
			return;
		}

		--len;
		--txt;
	}
}

[[nodiscard]] bool string_utf8_has_ext( const char *str, const char *ext )
{
	u64 len = string_utf8_bytes( str ) - 1;
	const char *txt = str + len;

	while ( len >= 0 )
	{
		if ( *txt == '.' )
			return ext != nullptr && string_utf8_compare( txt += ( ext[ 0 ] != '.' ), ext );

		--len;
		--txt;
	}

	return ext == nullptr;
}

[[nodiscard]] const char *string_utf8_get_filename( const char *str )
{
	massert( str );

	const char *p = str++;
	char c = *p;

	while ( c != '\0' )
	{
		if ( c == '/' || c == '\\' )
			p = str;

		c = *str++;
	}

	return p;
}

[[nodiscard]] char *string_utf8_filename( char *str )
{
	massert( str );

	char *p = str++;
	char c = *p;

	while ( c != '\0' )
	{
		if ( c == '/' || c == '\\' )
			p = str;

		c = *str++;
	}

	return p;
}

[[nodiscard]] inline char *string_utf8_base_filename( char *str )
{
	str = string_utf8_filename( str );
	string_utf8_trim_ext( str );
	return str;
}

inline void string_utf8_trim_path( char *str )
{
	massert( str );
	if ( str[ 0 ] != '\0' )
		string_utf8_copy( str, string_utf8_bytes( str ) - 1, string_utf8_get_filename( str ) );
}

[[nodiscard]] const char *string_utf8_past_start( const char *str, const char *start )
{
	massert( str && start );

	while ( *str == *start++ )
	{
		if ( *str == '\0' )
			return str;
		++str;
	}

	return str;
}

[[nodiscard]] char *string_utf8_past_start_case_insensitive( char *str, char *start )
{
	massert( str && start );

	while ( ascii_char_lower( *str ) == ascii_char_lower( *start++ ) )
	{
		if ( *str == '\0' )
			return str;
		++str;
	}

	return str;
}

[[nodiscard]] const char *string_utf8_past_start_case_insensitive( const char *str, const char *start )
{
	massert( str && start );

	while ( ascii_char_lower( *str ) == ascii_char_lower( *start++ ) )
	{
		if ( *str == '\0' )
			return str;
		++str;
	}

	return str;
}

[[nodiscard]] bool string_utf8_has_character( const char *str, const char *character )
{
	massert( str && character );

	u32 size, charSize;
	u32 charCodepoint = string_utf8_codepoint( character, &charSize );

	while ( *str )
	{
		if ( string_utf8_is_leading_byte( *str ) )
		{
			if ( charCodepoint == string_utf8_codepoint( str, &size ) )
				return true;

			str += size;
		}
		else
		{
			++str;
		}
	}

	return false;
}

template <u64 destSize>
inline u64 string_utf8_append( char( &destination )[ destSize ], const char *append )
{
	massert_static( destSize >= 1, "\"string_utf8_append\" failed. Invalid Size." );

	u64 p = string_utf8_bytes( destination ) - 1; // -1 is OK because only 1 requires a null terminator to count
	u64 appendBytes = string_utf8_bytes( append );

	// Check there is enough room to append
	if ( ( destSize - p ) < appendBytes )
		return 0;

	strcpy( destination + p, append );

	// Doesn't include null terminator
	return appendBytes - 1;
}

inline u64 string_utf8_append( char *destination, u64 destSize, const char *append )
{
	u64 p = string_utf8_bytes( destination ) - 1; // -1 is OK because only 1 requires a null terminator to count
	u64 appendBytes = string_utf8_bytes( append );

	// Check there is enough room to append
	if ( ( destSize - p ) < appendBytes )
		return 0;

	strcpy( destination + p, append );

	// Doesn't include null terminator
	return appendBytes - 1;
}

u64 string_utf8_insert( char *destination, u64 destSize, const char *insert, int index )
{
	u64 p = string_utf8_bytes( destination ) - 1; // -1 is OK because only 1 requires a null terminator to count
	u64 insertBytes = string_utf8_bytes( insert );

	// Check there is enough room to insert
	if ( ( destSize - p ) < insertBytes )
		return 0;

	u32 insertIndexSize;
	char *newDest = string_utf8_skip_codepoint( destination, &insertIndexSize, index );
	massert( newDest );

	--insertBytes; // Don't include the null terminator

	// Shuffle the destination up from where the insert will go (start at the end and work back)
	char *src = destination + p;
	char *dst = src + insertBytes;
	while ( src >= newDest )
		*dst-- = *src--;

	// Insert the new data
	u64 bytesToInsert = insertBytes;
	while ( bytesToInsert-- )
		*newDest++ = *insert++;

	// Doesn't include null terminator
	return insertBytes;
}

/// @desc Returns a count of characters until a delimiter is found
///       While marked as utf8 the delimiter should be ASCII only
[[nodiscard]] u64 string_utf8_string_span( const char *tok, const char *delim )
{
	massert( tok && delim );

	u64 i, j;

	for ( i = 0; tok[ i ] != '\0'; ++i )
		for ( j = 0; delim[ j ] != '\0'; ++j )
			if ( tok[ i ] == delim[ j ] )
				return i;

	return i;
}

/// @desc Returns a count of characters until a non delimiter is found
///       While marked as utf8 the delimiter should be ASCII only
[[nodiscard]] u64 string_utf8_string_nspan( const char *tok, const char *delim )
{
	massert( tok && delim );

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

/// @desc Output a string into token, a string split by the delimiters
///       While marked as utf8 the delimiter should be ASCII only
///       Also note it will cannibalise the input string ( do not use on string literals )
[[nodiscard]] char *string_utf8_tokenise( char *str, const char *delim, const char **token, char *found = nullptr )
{
	// Invalid input
	if ( !str )
	{
		*token = nullptr;
		return nullptr;
	}

	// Add a count of characters until a NON delimiter is found
	str += string_utf8_string_nspan( str, delim );

	// No NON delimiter found, reached end of string
	if ( *str == '\0' )
	{
		*token = nullptr;
		return nullptr;
	}

	*token = str;

	// Set e to str + a count of characters until a delimiter IS found
	char *e = str + string_utf8_string_span( str, delim );

	// Which delimiter was found
	if ( found )
		*found = *e;

	// If it didn't reached the end of the string
	// it will need a null terminator inserting and
	// the new position of the string returned
	if ( *e != '\0' )
	{
		*e = '\0';
		return e + 1;
	}

	return nullptr;
}

/// @desc While marked as utf8 the delimiter should be ASCII only
///       The delimiter should include a newline to work as expected
///       Expect a unix style line ending of just \n, not \r\n, or \r
template <u64 tokenLength, u64 maxTokens>
[[nodiscard]] char *string_utf8_tokenise_line( char *str, const char *delim, Array<Array<char, tokenLength>, maxTokens> &tokens )
{
	massert( string_utf8_has_character( delim, "\n" ) );

	tokens.clear();

	char delimFound;
	const char *token;

	str = string_utf8_tokenise( str, delim, &token, &delimFound );

	while ( token )
	{
		Array<char, tokenLength> *entry = &tokens.push();

		entry->resize( string_utf8_length( token ) + 1 );

		string_utf8_copy( entry->data, token );

		// if the newline was hit, process no more tokens
		if ( delimFound == '\n' )
			return str;

		str = string_utf8_tokenise( str, delim, &token, &delimFound );
	}

	return str;
}

char *string_utf8_replace_ascii_char( char *str, char find, char replace )
{
	massert( str );

	char *start = str;
	u32 size;
	u32 findCodepoint = static_cast<u32>( find );

	while ( *str )
	{
		if ( string_utf8_is_leading_byte( *str ) )
		{
			if ( findCodepoint == string_utf8_codepoint( str, &size ) )
			{
				*str = replace;
			}

			str += size;
		}
		else
		{
			++str;
		}
	}

	return start;
}

// UTF-16 ///////////////////////////////////////////////////////////////////////////////
#define SURROGATE_CODEPOINT_HIGH_START 		( 0xD800 )
#define SURROGATE_CODEPOINT_LOW_START 		( 0xDC00 )
#define SURROGATE_CODEPOINT_OFFSET 			( 0x010000 )
#define SURROGATE_CODEPOINT_MASK 			( 0x03FF )
#define SURROGATE_CODEPOINT_BITS 			( 10 )

struct utf16Character
{
	// high, low
	u16 data[ 3 ];
	u16 reserved;
};

[[nodiscard]] inline bool string_utf16_surrogate_pair_high( u16 c )
{
	return c >= SURROGATE_CODEPOINT_HIGH_START && c <= 0xDBFF;
}

[[nodiscard]] inline bool string_utf16_surrogate_pair_low( u16 c )
{
	return c >= SURROGATE_CODEPOINT_LOW_START && c <= 0xDFFF;
}

[[nodiscard]] u32 string_utf16_codepoint( const u16 *str, u32 *pSize )
{
	if ( !string_utf16_surrogate_pair_high( *str ) )
	{
		*pSize = 1;
		return *str;
	}

	u16 high = *str++;
	u16 low = *str++; 

	massert( string_utf16_surrogate_pair_low( low ) );

	*pSize = 2;

	u32 codepoint = static_cast<u32>( high & SURROGATE_CODEPOINT_MASK ) << SURROGATE_CODEPOINT_BITS;
	codepoint |= low & SURROGATE_CODEPOINT_MASK;
	return codepoint + SURROGATE_CODEPOINT_OFFSET;
}

[[nodiscard]] utf16Character string_utf16_encode( u32 codepoint )
{
	utf16Character character;

	// BMP
	if ( codepoint <= 0xFFFF )
	{
		character.data[ 0 ] = static_cast<u16>( codepoint );
		character.data[ 1 ] = '\0';
		return character;
	}

	// SURROGATE
	codepoint -= SURROGATE_CODEPOINT_OFFSET;
	character.data[ 0 ] = SURROGATE_CODEPOINT_HIGH_START + static_cast<u16>( ( codepoint >> SURROGATE_CODEPOINT_BITS ) & SURROGATE_CODEPOINT_MASK );
	character.data[ 1 ] = SURROGATE_CODEPOINT_LOW_START + static_cast<u16>( codepoint & SURROGATE_CODEPOINT_MASK );
	character.data[ 2 ] = '\0';

	return character;
}