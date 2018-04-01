/**
* Copyright (c) 2017 Andrew Nguyen, http://www.github.com/ravenwing1234
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

/**
* MarbleBag.h
* Gameplay utility for dependent probability of random integers
* Move constructor and move assignment only, no copy.
*
* Created: Andrew Nguyen (8-11-2017)
* 
* Usage:
*	MarbleBag< 100 > will return values from [0, 99]
*	MarbleBag< 100 > bag;														// Default constructed with chrono-based seed
*	MarbleBag< 100 > bag( std::move( std::default_random_engine{ 2017 } ) );	// Constructed with specified random engine initialized to explicit seed
*	int randomVal = bag.GetNext();												// Get next random marble value
*	if( bag.HasMarbles() ) { bag.Reset(); }										// For bag reuse. Test if bag has values remaining, then reset bag.
*
*/

#pragma once

#include <chrono>
#include <array>
#include <functional>
#include <random>

namespace dark
{

template< typename FloatType, typename IntType >
inline constexpr IntType Ceil( FloatType val ) noexcept
{
	return static_cast< FloatType >( static_cast< IntType >( val ) ) == val ? static_cast< IntType >( val ) : static_cast< IntType >( val + 1 );
}

inline constexpr int DoubleCeilToInt( double val ) noexcept
{
	return Ceil< double, int >( val );
}

/// Utility for dependent probability of random integers.
template< int NumMarbles >
class MarbleBag
{
	static constexpr int NUM_BITS = { sizeof( std::uint64_t ) * 8 };

public:
	
	/// Default Constructor
	MarbleBag();

	/// Constructor with move of random engine type
	template< typename RandomEngineType >
	MarbleBag( RandomEngineType&& randomEngine );

	/// Destructor
	~MarbleBag() = default;

	/// No copy operations
	MarbleBag( const MarbleBag< NumMarbles >& other ) = delete;
	MarbleBag& operator=( const MarbleBag< NumMarbles >& other ) = delete;

	/// Move operations
	MarbleBag( MarbleBag< NumMarbles >&& other );
	MarbleBag& operator=( MarbleBag< NumMarbles >&& other );

	/// Returns next marble value. Returns -1 if no marbles remain. Use Reset() to restore marbles.
	const int GetNext();

	/// Returns quantity of marble values that still exist.
	const int GetRemainingCount() const;

	/// Returns if any marble values remain.
	bool HasMarbles() const;

	/// Returns all marble values to bag.
	void Reset();

	/// Explicitly set random engine.
	template< typename RandomEngineType >
	void SetRandomEngine( RandomEngineType&& randomEngine );

	/// Get a pointer to internal data that tracks used marbles and the number of int64 elements in _outData and _outNumElements respectively
	/// Use to serialize out usage information
	const void GetUsageData( std::uint64_t* _outData, std::size_t& _outNumElements ) const;

	/// Sets the internal marble usage data from _inData with the number of _inNumElements of int64
	/// Use to deserialize usage information
	void SetUsageData( std::uint64_t* _inData, std::size_t _inNumElements );

private:
	
	std::function< int() > m_roll;
	std::array< std::uint64_t, DoubleCeilToInt( 1.0 * NumMarbles / NUM_BITS ) > m_removedMarbles = {}; 
	int m_numRemoved = { 0 };
};

//////////////////////////////////////////////////////////////////////////
/// Implementation
//////////////////////////////////////////////////////////////////////////

//
// Public 
//

template< int NumMarbles >
void MarbleBag< NumMarbles >::SetUsageData( std::uint64_t* _inData, std::size_t _inNumElements )
{
	int smallerSize = m_removedMarbles.size() > _inNumElements ? _inNumElements : m_removedMarbles.size();
	m_removedMarbles.fill( 0 );
	std::memcpy( m_removedMarbles.data(), _inData, smallerSize * sizeof( std::uint64_t ) );
}

template< int NumMarbles >
const void MarbleBag< NumMarbles >::GetUsageData( std::uint64_t* _outData, std::size_t& _outNumElements ) const
{
	_outData = m_removedMarbles.data();
	_outNumElements = m_removedMarbles.size();
}

template< int NumMarbles >
template< typename RandomEngineType >
void MarbleBag< NumMarbles >::SetRandomEngine( RandomEngineType&& randomEngine )
{
	m_roll = std::bind( std::uniform_int_distribution< int >( 0, NumMarbles - 1 ), std::forward< RandomEngineType >( randomEngine ) );
}

template< int NumMarbles >
void MarbleBag< NumMarbles >::Reset()
{
	m_removedMarbles.fill( 0 );
	m_numRemoved = 0;
}

template< int NumMarbles >
bool MarbleBag< NumMarbles >::HasMarbles() const
{
	return GetRemainingCount() > 0;
}

template< int NumMarbles >
const int MarbleBag< NumMarbles >::GetRemainingCount() const
{
	return ( NumMarbles - m_numRemoved );
}

template< int NumMarbles >
const int MarbleBag< NumMarbles >::GetNext()
{
	constexpr int arraySize = DoubleCeilToInt( 1.0 * NumMarbles / NUM_BITS );
	constexpr int bitRange = NumMarbles < NUM_BITS ? NumMarbles : NUM_BITS;
	constexpr double divisor = 1.0 / NUM_BITS;
	constexpr std::uint64_t mask = 0xffffffffffffffffU;
	const int result = m_roll();
	int idx = static_cast< int >( result * divisor );
	int remainder = result - ( NUM_BITS * idx );
	for( int arrayIdx = 0; arrayIdx < arraySize; ++arrayIdx )
	{
		std::uint64_t currVal = m_removedMarbles[ idx ];
		if( currVal ^ mask )
		{
			std::uint64_t indexer;
			for( int i = 0; i < bitRange; ++i )
			{
				indexer = 1LL << remainder;
				if( ( indexer & currVal ) == 0 )
				{
					m_removedMarbles[ idx ] |= indexer;
					++m_numRemoved;
					return idx * NUM_BITS + remainder;
				}
				if( ++remainder >= bitRange )
				{
					remainder = 0;
				}
			}
		}
		else
		{
			if( ++idx >= arraySize )
			{
				idx = 0;
			}
			remainder = 0;
		}
	}
	return -1;
}

template< int NumMarbles >
MarbleBag< NumMarbles >& MarbleBag< NumMarbles >::operator=( MarbleBag< NumMarbles >&& other )
{
	m_removedMarbles = std::move( other.m_removedMarbles );
	m_roll = std::move( other.m_roll );

	return *this;
}

template< int NumMarbles >
MarbleBag< NumMarbles >::MarbleBag( MarbleBag< NumMarbles >&& other )
{
	*this = std::forward< MarbleBag< NumMarbles > >( other );
}

template< int NumMarbles >
template< typename RandomEngineType >
MarbleBag< NumMarbles >::MarbleBag( RandomEngineType&& randomEngine )
{
	SetRandomEngine( ( std::forward< RandomEngineType >( randomEngine ) ) );
}

template< int NumMarbles >
MarbleBag< NumMarbles >::MarbleBag()
	: MarbleBag( std::move( std::default_random_engine{ static_cast< std::uint32_t >( std::chrono::system_clock::now().time_since_epoch().count() ) } ) )
{}

}
