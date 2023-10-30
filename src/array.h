
#pragma once

template <typename Type, u64 Capacity>
struct Array
{
	u64 count = 0;
	Type data[ Capacity ];

	inline void add( Type &t )
	{
		assert( count < Capacity );
		data[ count++ ] = t;
	}

	inline void add( const Type &t )
	{
		assert( count < Capacity );
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
		assert( count + sizeof...( Args ) < Capacity );
		data[ count++ ] = t;
		add_no_bounds_check( args... );
	}

	inline void append( const Type *t, u64 appentCount )
	{
		assert( count + appentCount < Capacity );

		Type *p = &data[ count ];

		for ( u64 i = 0; i < appentCount; ++i )
			*p++ = *t++;

		count += appentCount;
	}

	inline void set( u64 idx, Type &t )
	{
		assert( idx < count );
		data[ idx ] = t;
	}

	inline void set( u64 idx, const Type &t )
	{
		assert( idx < count );
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
		assert( size <= Capacity );
		count = size;
	}

	inline void clear()
	{
		count = 0;
	}

	inline void swap_and_remove( u64 idx )
	{
		assert( idx < count );
		data[ idx ] = data[ --count ];
	}

	[[nodiscard]] inline Type & operator[] ( u64 idx )
	{
		assert( idx < count );
		return data[ idx ];
	}

	[[nodiscard]] inline const Type & operator[] ( u64 idx ) const
	{
		assert( idx < count );
		return data[ idx ];
	}

	[[nodiscard]] inline Type &top()
	{
		assert( count > 0 );
		return data[ count - 1 ];
	}

	inline Type &push()
	{
		assert( count < Capacity );
		return data[ count++ ];
	}

	inline Type &pop()
	{
		assert( count > 0 );
		return data[ --count ];
	}

	inline void pop_back()
	{
		assert( count > 0 );
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