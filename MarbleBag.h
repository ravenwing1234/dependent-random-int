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
#include <bitset>
#include <functional>
#include <random>

namespace crux
{
/// Utility for dependent probability of random integers.
template< int NumMarbles >
class MarbleBag
{
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

public:

	/// If true, auto reset marble bag when empty
	bool bAutoReset = { true };

private:

	std::function< int() > m_roll;
	std::bitset< NumMarbles > m_removedMarbles;
	int m_numRemoved = { 0 };
};

//////////////////////////////////////////////////////////////////////////
/// Implementation
//////////////////////////////////////////////////////////////////////////

//
// Public 
//

template< int NumMarbles >
template< typename RandomEngineType >
void MarbleBag< NumMarbles >::SetRandomEngine( RandomEngineType&& randomEngine )
{
	m_roll = std::bind( std::uniform_int_distribution< int >( 0, NumMarbles - 1 ), std::forward< RandomEngineType >( randomEngine ) );
}

template< int NumMarbles >
void MarbleBag< NumMarbles >::Reset()
{
	m_removedMarbles.reset();
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
	if( !HasMarbles() )
	{
		if( bAutoReset )
		{
			Reset();
		}
		else
		{
			return -1;
		}
	}
	int result = m_roll();
	int numEmptyIndexesVisited = 0;

	if( m_removedMarbles[ result ] )
	{
		int numToVisit = m_roll();
		while( numEmptyIndexesVisited < numToVisit )
		{
			if( ++result >= NumMarbles )
			{
				result = 0;
			}
			if( !m_removedMarbles[ result ] )
			{
				++numEmptyIndexesVisited;
			}
		}
	}
	++m_numRemoved;
	m_removedMarbles[ result ] = true;
	return result;
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
