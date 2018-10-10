# Marble Bag
Gameplay utility for dependent probability of random integers. Often in video games, true independent probabilities result in a frustrating experience for players. Similar to flipping a coin or rolling a dice, something with an expected 5% chance may never occur after 1000 iterations. This class treats random like marbles in a bag where each result is removed resulting in future results always being new. With our example of 5% using the marble bag, after 100 iterations you will always encounter the result 5 times. Likewise, with 1000 iterations you will encounter the result exactly 50 times ( 5 times per 100 ).

## Usage
- MarbleBag< 100 > will return values from [0, 99]
- MarbleBag< 100 > bag;														// Default constructed with chrono-based seed
- MarbleBag< 100 > bag( std::move( std::default_random_engine{ 2017 } ) );	// Constructed with specified random engine initialized to explicit seed
- int randomVal = bag.GetNext();												// Get next random marble value
- if( bag.HasMarbles() ) { bag.Reset(); }										// For bag reuse. Test if bag has values remaining, then reset bag.

## License

MarbleBag is developed by Andrew Nguyen, and has the [zlib license](http://en.wikipedia.org/wiki/Zlib_License). While the zlib license does not require acknowledgement, we encourage you to give credit in your product.
