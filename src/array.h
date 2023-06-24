
#pragma once

template <typename Type, u64 Capacity>
struct Array
{
	u64 count = 0;
	Type data[ Capacity ];

	inline void add( Type &t )
	{
		massert( count < Capacity );
		data[ count++ ] = t;
	}

	inline void add( const Type &t )
	{
		massert( count < Capacity );
		data[ count++ ] = t;
	}

	inline void add_no_bounds_check( const Type &t )
	{
		data[ count++ ] = t;
	}

	template <typename ...Args>
	inline void add_no_bounds_check( const Type &t, Args&&... args )
	{
		data[ count++ ] = t;
		add_no_bounds_check( args... );
	}

	template <typename ...Args>
	void add( const Type &t, Args&&... args )
	{
		massert( count + sizeof...( Args ) < Capacity );
		data[ count++ ] = t;
		add_no_bounds_check( args... );
	}

	inline void append( const Type *t, u64 appentCount )
	{
		massert( count + appentCount < Capacity );

		Type *p = &data[ count ];

		for ( u64 i = 0; i < appentCount; ++i )
			*p++ = *t++;

		count += appentCount;
	}

	inline void set( u64 idx, Type &t )
	{
		massert( idx < count );
		data[ idx ] = t;
	}

	inline void set( u64 idx, const Type &t )
	{
		massert( idx < count );
		data[ idx ] = t;
	}

	void set_all( Type &value, bool totalCapacity )
	{
		if ( totalCapacity )
			count = Capacity;

		for ( u64 i = 0; i < count; ++i )
			data[ i ] = value;
	}

	void set_all( const Type &value, bool totalCapacity )
	{
		if ( totalCapacity )
			count = Capacity;

		for ( u64 i = 0; i < count; ++i )
			data[ i ] = value;
	}

	inline void set_full()
	{
		count = Capacity;
	}

	inline void resize( u64 size )
	{
		massert( size <= Capacity );
		count = size;
	}

	inline void clear()
	{
		count = 0;
	}

	inline void swap_and_remove( u64 idx )
	{
		massert( idx < count );
		data[ idx ] = data[ --count ];
	}

	[[nodiscard]] inline Type & operator[] ( u64 idx )
	{
		massert( idx < count );
		return data[ idx ];
	}

	[[nodiscard]] inline const Type & operator[] ( u64 idx ) const
	{
		massert( idx < count );
		return data[ idx ];
	}

	[[nodiscard]] inline Type &top()
	{
		massert( count > 0 );
		return data[ count - 1 ];
	}

	inline Type &push()
	{
		massert( count < Capacity );
		return data[ count++ ];
	}

	inline Type &pop()
	{
		massert( count > 0 );
		return data[ --count ];
	}

	inline void pop_back()
	{
		massert( count > 0 );
		--count;
	}

	[[nodiscard]] inline bool has_value( const Type &t ) const
	{
		const Type *p = data;
		for ( u64 i = 0; i < count; ++i )
			if ( *p++ == t )
				return true;
		return false;
	}

	[[nodiscard]] inline bool empty() const
	{
		return count == 0;
	}

	[[nodiscard]] inline bool full() const
	{
		return count == Capacity;
	}

	[[nodiscard]] constexpr inline u64 capacity() const
	{
		return Capacity;
	}

	[[nodiscard]] inline u64 bytes() const
	{
		return sizeof( Type ) * count;
	}
};