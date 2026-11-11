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
// MARK: - TV5

template <typename A, typename B, typename C, typename D, typename E> struct TV5 {
	// Methods
	public:
					// Lifecycle methods
					TV5(const A& a, const B& b, const C& c, const D& d, const E& e) :
						mA(a), mB(b), mC(c), mD(d), mE(e)
						{}

					// Instance methods
		const	A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }
		const	C&	getC() const
						{ return mC; }
		const	D&	getD() const
						{ return mD; }
		const	E&	getE() const
						{ return mE; }

	// Properties
	private:
		A	mA;
		B	mB;
		C	mC;
		D	mD;
		E	mE;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TV6

template <typename A, typename B, typename C, typename D, typename E, typename F> struct TV6 {
	// Methods
	public:
					// Lifecycle methods
					TV6(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f) :
						mA(a), mB(b), mC(c), mD(d), mE(e), mF(f)
						{}

					// Instance methods
		const	A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }
		const	C&	getC() const
						{ return mC; }
		const	D&	getD() const
						{ return mD; }
		const	E&	getE() const
						{ return mE; }
		const	F&	getF() const
						{ return mF; }

	// Properties
	private:
		A	mA;
		B	mB;
		C	mC;
		D	mD;
		E	mE;
		F	mF;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TV7

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G> struct TV7 {
	// Methods
	public:
					// Lifecycle methods
					TV7(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g) :
						mA(a), mB(b), mC(c), mD(d), mE(e), mF(f), mG(g)
						{}

					// Instance methods
		const	A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }
		const	C&	getC() const
						{ return mC; }
		const	D&	getD() const
						{ return mD; }
		const	E&	getE() const
						{ return mE; }
		const	F&	getF() const
						{ return mF; }
		const	G&	getG() const
						{ return mG; }

	// Properties
	private:
		A	mA;
		B	mB;
		C	mC;
		D	mD;
		E	mE;
		F	mF;
		G	mG;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TV9

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
		struct TV9 {
	// Methods
	public:
					// Lifecycle methods
					TV9(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h,
							const I& i) :
						mA(a), mB(b), mC(c), mD(d), mE(e), mF(f), mG(g), mH(h), mI(i)
						{}

					// Instance methods
		const	A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }
		const	C&	getC() const
						{ return mC; }
		const	D&	getD() const
						{ return mD; }
		const	E&	getE() const
						{ return mE; }
		const	F&	getF() const
						{ return mF; }
		const	G&	getG() const
						{ return mG; }
		const	H&	getH() const
						{ return mH; }
		const	I&	getI() const
						{ return mI; }

	// Properties
	private:
		A	mA;
		B	mB;
		C	mC;
		D	mD;
		E	mE;
		F	mF;
		G	mG;
		H	mH;
		I	mI;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TV10

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I,
		typename J>
		struct TV10 {
	// Methods
	public:
					// Lifecycle methods
					TV10(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h,
							const I& i, const J& j) :
						mA(a), mB(b), mC(c), mD(d), mE(e), mF(f), mG(g), mH(h), mI(i), mJ(j)
						{}

					// Instance methods
		const	A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }
		const	C&	getC() const
						{ return mC; }
		const	D&	getD() const
						{ return mD; }
		const	E&	getE() const
						{ return mE; }
		const	F&	getF() const
						{ return mF; }
		const	G&	getG() const
						{ return mG; }
		const	H&	getH() const
						{ return mH; }
		const	I&	getI() const
						{ return mI; }
		const	J&	getJ() const
						{ return mJ; }

	// Properties
	private:
		A	mA;
		B	mB;
		C	mC;
		D	mD;
		E	mE;
		F	mF;
		G	mG;
		H	mH;
		I	mI;
		J	mJ;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TV11

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I,
		typename J, typename K>
		struct TV11 {
	// Methods
	public:
					// Lifecycle methods
					TV11(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h,
							const I& i, const J& j, const K& k) :
						mA(a), mB(b), mC(c), mD(d), mE(e), mF(f), mG(g), mH(h), mI(i), mJ(j), mK(k)
						{}

					// Instance methods
		const	A&	getA() const
						{ return mA; }
		const	B&	getB() const
						{ return mB; }
		const	C&	getC() const
						{ return mC; }
		const	D&	getD() const
						{ return mD; }
		const	E&	getE() const
						{ return mE; }
		const	F&	getF() const
						{ return mF; }
		const	G&	getG() const
						{ return mG; }
		const	H&	getH() const
						{ return mH; }
		const	I&	getI() const
						{ return mI; }
		const	J&	getJ() const
						{ return mJ; }
		const	K&	getK() const
						{ return mK; }

	// Properties
	private:
		A	mA;
		B	mB;
		C	mC;
		D	mD;
		E	mE;
		F	mF;
		G	mG;
		H	mH;
		I	mI;
		J	mJ;
		K	mK;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TRCR

template <typename A, typename B> struct TRCR {
	// Methods
	public:
					// Lifecycle methods
					TRCR(A& a, const B& b) : mA(a), mB(b) {}

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

//----------------------------------------------------------------------------------------------------------------------
// MARK: TOV2

template <typename A, typename B> struct TOV2 {
	// Methods
	public:
						// Lifecycle methods
						TOV2() {}
						TOV2(const A& a, const B& b) : mA(a), mB(b) {}

						// Instamce methods
		const	OV<A>&	getA() const
							{ return mA; }
		const	OV<B>&	getB() const
							{ return mB; }

				void	setValues(const A& a, const B& b)
							{ mA.setValue(a); mB.setValue(b); }
				void	removeValues()
							{ mA.removeValue(); mB.removeValue(); }

	// Properties
	private:
		OV<A>	mA;
		OV<B>	mB;
};
