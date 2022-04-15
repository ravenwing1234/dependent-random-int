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
template< int NumMarbles, typename RandomEngineType = std::default_random_engine >
class MarbleBag
{
public:

	/// Default Constructor
	MarbleBag();

	/// Constructor with move of random engine type
	MarbleBag( RandomEngineType&& randomEngine );

	/// Destructor
	~MarbleBag() = default;

	/// No copy operations
	MarbleBag( const MarbleBag< NumMarbles, RandomEngineType >& other ) = delete;
	MarbleBag& operator=( const MarbleBag< NumMarbles, RandomEngineType >& other ) = delete;

	/// Move operations
	MarbleBag( MarbleBag< NumMarbles, RandomEngineType >&& other );
	MarbleBag& operator=( MarbleBag< NumMarbles, RandomEngineType >&& other );

	/// Returns next marble value. Returns -1 if no marbles remain. Use Reset() to restore marbles.
	const int GetNext();

	/// Returns quantity of marble values that still exist.
	const int GetRemainingCount() const;

	/// Returns if any marble values remain.
	bool HasMarbles() const;

	/// Returns all marble values to bag.
	void Reset();

	/// Explicitly set random engine.
	void SetRandomEngine( RandomEngineType&& randomEngine );

private:

	int Roll();

private:

	RandomEngineType m_randomEngine;
	std::bitset< NumMarbles > m_removedMarbles;
	int m_numRemoved = { 0 };

public:

	/// If true, auto reset marble bag when empty
	bool bAutoReset = { true };
};

//////////////////////////////////////////////////////////////////////////
/// Implementation
//////////////////////////////////////////////////////////////////////////

//
// Public 
//

template< int NumMarbles, typename RandomEngineType >
void MarbleBag< NumMarbles, RandomEngineType >::SetRandomEngine( RandomEngineType&& randomEngine )
{
	m_randomEngine = std::forward< RandomEngineType >( randomEngine );
}

template< int NumMarbles, typename RandomEngineType >
void MarbleBag< NumMarbles, RandomEngineType >::Reset()
{
	m_removedMarbles.reset();
	m_numRemoved = 0;
}

template< int NumMarbles, typename RandomEngineType >
bool MarbleBag< NumMarbles, RandomEngineType >::HasMarbles() const
{
	return GetRemainingCount() > 0;
}

template< int NumMarbles, typename RandomEngineType >
const int MarbleBag< NumMarbles, RandomEngineType >::GetRemainingCount() const
{
	return ( NumMarbles - m_numRemoved );
}

template< int NumMarbles, typename RandomEngineType >
const int MarbleBag< NumMarbles, RandomEngineType >::GetNext()
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
	int numToVisit = Roll();
	int resultIdx = 0;
	int numEmptyIndexesVisited = 0;
	while( numEmptyIndexesVisited < numToVisit )
	{
		if( ++resultIdx >= NumMarbles )
		{
			resultIdx = 0;
		}
		if( !m_removedMarbles[ resultIdx ] )
		{
			++numEmptyIndexesVisited;
		}
	}
	++m_numRemoved;
	m_removedMarbles[ resultIdx ] = true;
	return resultIdx;
}

template< int NumMarbles, typename RandomEngineType >
MarbleBag< NumMarbles, RandomEngineType >& MarbleBag< NumMarbles, RandomEngineType >::operator=( MarbleBag< NumMarbles, RandomEngineType >&& other )
{
	m_removedMarbles = std::move( other.m_removedMarbles );
	m_randomEngine = std::move( other.m_randomEngine );

	return *this;
}

template< int NumMarbles, typename RandomEngineType >
MarbleBag< NumMarbles, RandomEngineType >::MarbleBag( MarbleBag< NumMarbles, RandomEngineType >&& other )
{
	*this = std::forward< MarbleBag< NumMarbles, RandomEngineType > >( other );
}

template< int NumMarbles, typename RandomEngineType >
MarbleBag< NumMarbles, RandomEngineType >::MarbleBag( RandomEngineType&& randomEngine )
{
	SetRandomEngine( ( std::forward< RandomEngineType >( randomEngine ) ) );
}

template< int NumMarbles, typename RandomEngineType >
MarbleBag< NumMarbles, RandomEngineType >::MarbleBag()
	: MarbleBag( std::move( std::default_random_engine{ static_cast< std::uint32_t >( std::chrono::system_clock::now().time_since_epoch().count() ) } ) )
{}

//
// Private
//

template< int NumMarbles, typename RandomEngineType >
int crux::MarbleBag< NumMarbles, RandomEngineType >::Roll()
{
	std::uniform_int_distribution< int > distribution( 1, NumMarbles - m_numRemoved );
	return distribution( m_randomEngine );
}

}
