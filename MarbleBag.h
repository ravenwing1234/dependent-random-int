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
*	MarbleBag bag ( 100 );														// Will return values from [0, 99]. Constructed with chrono-based seed														
*	int randomVal = bag.GetNext();												// Get next random marble value
*	if( bag.HasMarbles() ) { bag.Reset(); }										// For bag reuse. Test if bag has values remaining, then reset bag.
*
*/

#pragma once

#include <chrono>
#include <random>
#include <vector>

struct FMarbleSaveData
{
	std::vector< bool > removedMarbles;

	int numRemoved = { 0 };
};

namespace crux
{
/// Utility for dependent probability of random integers.
class MarbleBag
{
public:

	/// Default Constructor
	MarbleBag();

	/// Constructor that sets num marbles
	MarbleBag( int numMarbles );

	/// Destructor
	~MarbleBag();

	/// No copy operations
	MarbleBag( const MarbleBag& other ) = delete;
	MarbleBag& operator=( const MarbleBag& other ) = delete;

	/// Move operations
	MarbleBag( MarbleBag&& other ) noexcept;
	MarbleBag& operator=( MarbleBag&& other ) noexcept;

	/// Get total marble count
	int GetNumMarbles() const;

	/// Returns next marble value. Returns -1 if no marbles remain. Use Reset() to restore marbles.
	int GetNext();

	/// Returns quantity of marble values that still exist.
	int GetRemainingCount() const;

	/// Returns if any marble values remain.
	bool HasMarbles() const;

	/// Returns all marble values to bag.
	void Reset();

	/// Remove specific marble
	void RemoveMarble( unsigned index );

	/// Change the number of marbles. Resets the bag.
	void SetMarbleQuantity( int numMarbles );

	FMarbleSaveData GetSaveData() const;
	void LoadFromSaveData( const FMarbleSaveData& saveData );

private:
	
	int Roll();

public:

	/// If true, auto reset marble bag when empty
	bool bAutoReset = { true };

private:

	std::default_random_engine m_generator;
	std::uniform_int_distribution< int > m_distribution;
	std::vector< bool > m_removedMarbles;
	int m_numRemoved = { 0 };
};

//////////////////////////////////////////////////////////////////////////
/// Implementation
//////////////////////////////////////////////////////////////////////////

//
// Public 
//

inline int MarbleBag::Roll()
{
	return m_distribution( m_generator );
}

inline FMarbleSaveData MarbleBag::GetSaveData() const
{
	FMarbleSaveData saveData;
	saveData.numRemoved = m_numRemoved;
	for( int i = 0; i < m_removedMarbles.size(); ++i )
	{
		saveData.removedMarbles.push_back( m_removedMarbles[ i ] );
	}
	return saveData;
}

inline void MarbleBag::LoadFromSaveData( const FMarbleSaveData& saveData )
{
	m_numRemoved = saveData.numRemoved;
	const size_t numRemovedMarbles = saveData.removedMarbles.size();
	m_removedMarbles.resize( numRemovedMarbles );
	for( int i = 0; i < numRemovedMarbles; ++i )
	{
		m_removedMarbles[ i ] = saveData.removedMarbles[ i ];
	}
	m_distribution = std::uniform_int_distribution<int>( 1, GetRemainingCount() );
}

inline void MarbleBag::SetMarbleQuantity( int numMarbles )
{
	m_removedMarbles.clear();
	m_removedMarbles.resize( numMarbles, false );
	m_generator = std::default_random_engine{ static_cast< std::uint32_t >( std::chrono::system_clock::now().time_since_epoch().count() ) };
	m_distribution = std::uniform_int_distribution< int >( 1, GetNumMarbles() );
}

inline void MarbleBag::RemoveMarble( unsigned index )
{
	if( index < m_removedMarbles.size() && !m_removedMarbles[ index ] )
	{
		++m_numRemoved;
		m_removedMarbles[ index ] = true;
	}
}

inline void MarbleBag::Reset()
{
	for( int i = 0; i < m_removedMarbles.size(); ++i )
	{
		m_removedMarbles[ i ] = false;
	}
	m_numRemoved = 0;
	m_distribution = std::uniform_int_distribution< int >( 1, GetNumMarbles() );
}

inline bool MarbleBag::HasMarbles() const
{
	return GetRemainingCount() > 0;
}

inline int MarbleBag::GetRemainingCount() const
{
	return ( GetNumMarbles() - m_numRemoved );
}

inline int MarbleBag::GetNumMarbles() const
{
	return static_cast< int >( m_removedMarbles.size() );
}

inline int MarbleBag::GetNext()
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
	else
	{
		m_distribution = std::uniform_int_distribution< int >( 1, GetRemainingCount() );
	}
	int result = 0;
	int numToVisit = Roll();
	int numEmptyIndexesVisited = 0;

	while( numEmptyIndexesVisited < numToVisit )
	{
		if( !m_removedMarbles[ result ] )
		{
			if( ++numEmptyIndexesVisited == numToVisit )
			{
				break;
			}
		}
		if( ++result >= GetNumMarbles() )
		{
			result = 0;
		}
	}
	++m_numRemoved;
	m_removedMarbles[ result ] = true;
	return result;
}

inline MarbleBag& MarbleBag::operator=( MarbleBag&& other ) noexcept
{
	m_removedMarbles = std::move( other.m_removedMarbles );
	m_generator = other.m_generator ;
	m_distribution = other.m_distribution ;

	return *this;
}

inline MarbleBag::MarbleBag( MarbleBag&& other ) noexcept
{
	*this = std::forward< MarbleBag >( other );
}

inline MarbleBag::~MarbleBag()
{
	m_removedMarbles.clear();
}

inline MarbleBag::MarbleBag( int numMarbles )
{
	SetMarbleQuantity( numMarbles );
}

inline MarbleBag::MarbleBag()
{
	SetMarbleQuantity( 1 );
}

}
