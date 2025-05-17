//----------------------------------------------------------------------------------------------------------------------
//	Tuple.h			©2025 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: TV2

template <typename A, typename B> struct TV2 {
	// Methods
	public:
			// Lifecycle methods
			TV2(const A& a, const B& b) : mA(a), mB(b) {}
			TV2(const TV2& other) : mA(other.mA), mB(other.mB) {}

			// Instamce methods
		A&	getA() const
				{ return mA; }
		B&	getB() const
				{ return mB; }

	// Properties
	private:
		A	mA;
		B	mB;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TRCR

template <typename A, typename B> struct TRCR {
	// Methods
	public:
					// Lifecycle methods
					TRCR(A& a, const B& b) : mA(a), mB(b) {}
					TRCR(const TRCR& other) : mA(other.mA), mB(other.mB) {}

					// Instamce methods
				A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }

	// Properties
	private:
				A&	mA;
		const	B&	mB;
};
