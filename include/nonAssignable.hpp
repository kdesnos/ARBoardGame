#pragma once

/**
* This Class should be inherited by classes where default copy constructors and
* copy assignment with operator= are unwanted features.
* The main purpose of this class is to force handling of derived class with 
* either pointers of references.
*/
class NonAssignable {
public:
	NonAssignable(NonAssignable const&) = delete;
	NonAssignable& operator=(NonAssignable const&) = delete;
	NonAssignable() {}
};