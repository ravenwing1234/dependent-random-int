# Marble Bag
Gameplay utility for dependent probability of random integers

## Usage
- MarbleBag< 100 > will return values from [0, 99]
- MarbleBag< 100 > bag;														// Default constructed with chrono-based seed
- MarbleBag< 100 > bag( std::move( std::default_random_engine{ 2017 } ) );	// Constructed with specified random engine initialized to explicit seed
- int randomVal = bag.GetNext();												// Get next random marble value
- if( bag.HasMarbles() ) { bag.Reset(); }										// For bag reuse. Test if bag has values remaining, then reset bag.

## License

MarbleBag is developed by Andrew Nguyen, and has the [zlib license](http://en.wikipedia.org/wiki/Zlib_License). While the zlib license does not require acknowledgement, we encourage you to give credit in your product.