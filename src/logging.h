
#pragma once

#ifdef DEBUG

#define massert( condition, ... ) do { if ( !( condition ) ) { show_debug_error_function( #condition __VA_OPT__(" : ") __VA_ARGS__ ); __debugbreak(); } } while (0)
#define massert_static( condition, ... ) do { static_assert( condition __VA_OPT__(,) __VA_ARGS__ ); } while (0)
#define show_debug_message( message, ... ) show_debug_message_function( message __VA_OPT__(,) __VA_ARGS__ )
#define show_debug_info( message, ... ) show_debug_information_function( message __VA_OPT__(,) __VA_ARGS__ )
#define show_debug_warning( message, ... ) show_debug_warning_function( message __VA_OPT__(,) __VA_ARGS__ )
#define show_debug_error( message, ... ) do { show_debug_error_function( message __VA_OPT__(,) __VA_ARGS__ ); __debugbreak(); } while (0)
#define show_log_message( message, ... ) show_debug_message_function( message __VA_OPT__(,) __VA_ARGS__ )
#define show_log_info( message, ... ) show_debug_information_function( message __VA_OPT__(,) __VA_ARGS__ )
#define show_log_warning( message, ... ) show_debug_warning_function( message __VA_OPT__(,) __VA_ARGS__ )
#define show_log_error( message, ... ) do { show_debug_error_function( message __VA_OPT__(,) __VA_ARGS__ ); __debugbreak(); } while (0)

#else

#define massert( condition, ... ) do {} while (0)
#define massert_static( condition, ... ) do {} while (0)
#define show_debug_message( message, ... ) do {} while (0)
#define show_debug_info( message, ... ) do {} while (0)
#define show_debug_warning( message, ... ) do {} while (0)
#define show_debug_error( message, ... ) do {} while (0)
#define show_log_message( message, ... ) show_debug_message_function( message __VA_OPT__(,) __VA_ARGS__ )
#define show_log_info( message, ... ) show_debug_information_function( message __VA_OPT__(,) __VA_ARGS__ )
#define show_log_warning( message, ... ) show_debug_warning_function( message __VA_OPT__(,) __VA_ARGS__ )
#define show_log_error( message, ... ) show_debug_error_function( message __VA_OPT__(,) __VA_ARGS__ )

#endif

void show_debug_message_function( const char *message, ... );
void show_debug_message_function_ext( const char *message, va_list args );
void show_debug_information_function( const char *message, ... );
void show_debug_information_function_ext( const char *message, va_list args );
void show_debug_warning_function( const char *message, ... );
void show_debug_warning_function_ext( const char *message, va_list args );
void show_debug_error_function( const char *message, ... );
void show_debug_error_function_ext( const char *message, va_list args );