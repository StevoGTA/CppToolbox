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
		const	A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }

	// Properties
	private:
		A	mA;
		B	mB;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TV3

template <typename A, typename B, typename C> struct TV3 {
	// Methods
	public:
					// Lifecycle methods
					TV3(const A& a, const B& b, const C& c) : mA(a), mB(b), mC(c) {}
					TV3(const TV3& other) : mA(other.mA), mB(other.mB), mC(other.mC) {}

					// Instamce methods
		const	A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }
		const	C&	getC() const
						{ return mC; }

	// Properties
	private:
		A	mA;
		B	mB;
		C	mC;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TV4

template <typename A, typename B, typename C, typename D> struct TV4 {
	// Methods
	public:
					// Lifecycle methods
					TV4(const A& a, const B& b, const C& c, const D& d) : mA(a), mB(b), mC(c), mD(d) {}
					TV4(const TV4& other) : mA(other.mA), mB(other.mB), mC(other.mC), mD(other.mD) {}

					// Instamce methods
		const	A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }
		const	C&	getC() const
						{ return mC; }
		const	D&	getD() const
						{ return mD; }

	// Properties
	private:
		A	mA;
		B	mB;
		C	mC;
		D	mD;
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
