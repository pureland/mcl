#pragma once
/**
	@file
	@brief finite field extension class
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <mcl/fp.hpp>

namespace mcl {

template<class Fp>
class FpDblT : public fp::Serializable<FpDblT<Fp> > {
	typedef fp::Unit Unit;
	Unit v_[Fp::maxSize * 2];
public:
	static size_t getUnitSize() { return Fp::op_.N * 2; }
	FpDblT() : v_()
	{
	}
	FpDblT(const FpDblT& rhs)
	{
		const size_t n = getUnitSize();
		for (size_t i = 0; i < n; i++) {
			v_[i] = rhs.v_[i];
		}
	}
	void dump() const
	{
		const size_t n = getUnitSize();
		for (size_t i = 0; i < n; i++) {
			mcl::fp::dumpUnit(v_[n - 1 - i]);
		}
		printf("\n");
	}
	template<class OutputStream>
	void save(bool *pb, OutputStream& os, int) const
	{
		char buf[1024];
		size_t n = mcl::fp::arrayToHex(buf, sizeof(buf), v_, getUnitSize());
		if (n == 0) {
			*pb = false;
			return;
		}
		cybozu::write(pb, os, buf + sizeof(buf) - n, sizeof(buf));
	}
	template<class InputStream>
	void load(bool *pb, InputStream& is, int)
	{
		char buf[1024];
		*pb = false;
		size_t n = fp::local::loadWord(buf, sizeof(buf), is);
		if (n == 0) return;
		n = fp::hexToArray(v_, getUnitSize(), buf, n);
		if (n == 0) return;
		for (size_t i = n; i < getUnitSize(); i++) v_[i] = 0;
		*pb = true;
	}
#ifndef CYBOZU_DONT_USE_EXCEPTION
	template<class OutputStream>
	void save(OutputStream& os, int ioMode = IoSerialize) const
	{
		bool b;
		save(&b, os, ioMode);
		if (!b) throw cybozu::Exception("FpDblT:save") << ioMode;
	}
	template<class InputStream>
	void load(InputStream& is, int ioMode = IoSerialize)
	{
		bool b;
		load(&b, is, ioMode);
		if (!b) throw cybozu::Exception("FpDblT:load") << ioMode;
	}
	void getMpz(mpz_class& x) const
	{
		bool b;
		getMpz(&b, x);
		if (!b) throw cybozu::Exception("FpDblT:getMpz");
	}
	mpz_class getMpz() const
	{
		mpz_class x;
		getMpz(x);
		return x;
	}
#endif
	void clear()
	{
		const size_t n = getUnitSize();
		for (size_t i = 0; i < n; i++) {
			v_[i] = 0;
		}
	}
	FpDblT& operator=(const FpDblT& rhs)
	{
		const size_t n = getUnitSize();
		for (size_t i = 0; i < n; i++) {
			v_[i] = rhs.v_[i];
		}
		return *this;
	}
	// QQQ : does not check range of x strictly(use for debug)
	void setMpz(const mpz_class& x)
	{
		assert(x >= 0);
		const size_t xn = gmp::getUnitSize(x);
		const size_t N2 = getUnitSize();
		if (xn > N2) {
			assert(0);
			return;
		}
		memcpy(v_, gmp::getUnit(x), xn * sizeof(Unit));
		memset(v_ + xn, 0, (N2 - xn) * sizeof(Unit));
	}
	void getMpz(bool *pb, mpz_class& x) const
	{
		gmp::setArray(pb, x, v_, Fp::op_.N * 2);
	}
#ifdef MCL_XBYAK_DIRECT_CALL
	static void (*add)(FpDblT& z, const FpDblT& x, const FpDblT& y);
	static void (*sub)(FpDblT& z, const FpDblT& x, const FpDblT& y);
	static void (*mod)(Fp& z, const FpDblT& xy);
	static void (*addPre)(FpDblT& z, const FpDblT& x, const FpDblT& y);
	static void (*subPre)(FpDblT& z, const FpDblT& x, const FpDblT& y);
	static void addC(FpDblT& z, const FpDblT& x, const FpDblT& y) { Fp::op_.fpDbl_add(z.v_, x.v_, y.v_, Fp::op_.p); }
	static void subC(FpDblT& z, const FpDblT& x, const FpDblT& y) { Fp::op_.fpDbl_sub(z.v_, x.v_, y.v_, Fp::op_.p); }
	static void modC(Fp& z, const FpDblT& xy) { Fp::op_.fpDbl_mod(z.v_, xy.v_, Fp::op_.p); }
	static void addPreC(FpDblT& z, const FpDblT& x, const FpDblT& y) { Fp::op_.fpDbl_addPre(z.v_, x.v_, y.v_); }
	static void subPreC(FpDblT& z, const FpDblT& x, const FpDblT& y) { Fp::op_.fpDbl_subPre(z.v_, x.v_, y.v_); }
#else
	static void add(FpDblT& z, const FpDblT& x, const FpDblT& y) { Fp::op_.fpDbl_add(z.v_, x.v_, y.v_, Fp::op_.p); }
	static void sub(FpDblT& z, const FpDblT& x, const FpDblT& y) { Fp::op_.fpDbl_sub(z.v_, x.v_, y.v_, Fp::op_.p); }
	static void mod(Fp& z, const FpDblT& xy) { Fp::op_.fpDbl_mod(z.v_, xy.v_, Fp::op_.p); }
	static void addPre(FpDblT& z, const FpDblT& x, const FpDblT& y) { Fp::op_.fpDbl_addPre(z.v_, x.v_, y.v_); }
	static void subPre(FpDblT& z, const FpDblT& x, const FpDblT& y) { Fp::op_.fpDbl_subPre(z.v_, x.v_, y.v_); }
#endif
	static void mulPreC(FpDblT& xy, const Fp& x, const Fp& y) { Fp::op_.fpDbl_mulPre(xy.v_, x.v_, y.v_); }
	static void sqrPreC(FpDblT& xx, const Fp& x) { Fp::op_.fpDbl_sqrPre(xx.v_, x.v_); }
	/*
		mul(z, x, y) = mulPre(xy, x, y) + mod(z, xy)
	*/
	static void (*mulPre)(FpDblT& xy, const Fp& x, const Fp& y);
	static void (*sqrPre)(FpDblT& xx, const Fp& x);
	static void mulUnit(FpDblT& z, const FpDblT& x, Unit y)
	{
		if (mulSmallUnit(z, x, y)) return;
		assert(0); // not supported y
	}
	static void init()
	{
		const mcl::fp::Op& op = Fp::getOp();
#ifdef MCL_XBYAK_DIRECT_CALL
		add = fp::func_ptr_cast<void (*)(FpDblT&, const FpDblT&, const FpDblT&)>(op.fpDbl_addA_);
		if (add == 0) add = addC;
		sub = fp::func_ptr_cast<void (*)(FpDblT&, const FpDblT&, const FpDblT&)>(op.fpDbl_subA_);
		if (sub == 0) sub = subC;
		mod = fp::func_ptr_cast<void (*)(Fp&, const FpDblT&)>(op.fpDbl_modA_);
		if (mod == 0) mod = modC;
		addPre = fp::func_ptr_cast<void (*)(FpDblT&, const FpDblT&, const FpDblT&)>(op.fpDbl_addPre);
		if (addPre == 0) addPre = addPreC;
		subPre = fp::func_ptr_cast<void (*)(FpDblT&, const FpDblT&, const FpDblT&)>(op.fpDbl_subPre);
		if (subPre == 0) subPre = subPreC;
#endif
		if (op.fpDbl_mulPreA_) {
			mulPre = fp::func_ptr_cast<void (*)(FpDblT&, const Fp&, const Fp&)>(op.fpDbl_mulPreA_);
		} else {
			mulPre = mulPreC;
		}
		if (op.fpDbl_sqrPreA_) {
			sqrPre = fp::func_ptr_cast<void (*)(FpDblT&, const Fp&)>(op.fpDbl_sqrPreA_);
		} else {
			sqrPre = sqrPreC;
		}
	}
	void operator+=(const FpDblT& x) { add(*this, *this, x); }
	void operator-=(const FpDblT& x) { sub(*this, *this, x); }
};

#ifdef MCL_XBYAK_DIRECT_CALL
template<class Fp> void (*FpDblT<Fp>::add)(FpDblT&, const FpDblT&, const FpDblT&);
template<class Fp> void (*FpDblT<Fp>::sub)(FpDblT&, const FpDblT&, const FpDblT&);
template<class Fp> void (*FpDblT<Fp>::mod)(Fp&, const FpDblT&);
template<class Fp> void (*FpDblT<Fp>::addPre)(FpDblT&, const FpDblT&, const FpDblT&);
template<class Fp> void (*FpDblT<Fp>::subPre)(FpDblT&, const FpDblT&, const FpDblT&);
#endif
template<class Fp> void (*FpDblT<Fp>::mulPre)(FpDblT&, const Fp&, const Fp&);
template<class Fp> void (*FpDblT<Fp>::sqrPre)(FpDblT&, const Fp&);

template<class Fp> struct Fp12T;
template<class Fp> class BNT;
template<class Fp> struct Fp2DblT;
/*
	beta = -1
	Fp2 = F[i] / (i^2 + 1)
	x = a + bi
*/
template<class _Fp>
class Fp2T : public fp::Serializable<Fp2T<_Fp>,
	fp::Operator<Fp2T<_Fp> > > {
	typedef _Fp Fp;
	typedef fp::Unit Unit;
	typedef FpDblT<Fp> FpDbl;
	typedef Fp2DblT<Fp> Fp2Dbl;
	static const size_t gN = 5;
	/*
		g = xi^((p - 1) / 6)
		g[] = { g^2, g^4, g^1, g^3, g^5 }
	*/
	static Fp2T g[gN];
	static Fp2T g2[gN];
	static Fp2T g3[gN];
public:
	static const Fp2T *get_gTbl() { return &g[0]; }
	static const Fp2T *get_g2Tbl() { return &g2[0]; }
	static const Fp2T *get_g3Tbl() { return &g3[0]; }
	typedef typename Fp::BaseFp BaseFp;
	static const size_t maxSize = Fp::maxSize * 2;
	static inline size_t getByteSize() { return Fp::getByteSize() * 2; }
	void dump() const
	{
		a.dump();
		b.dump();
	}
	Fp a, b;
	Fp2T() { }
	Fp2T(int64_t a) : a(a), b(0) { }
	Fp2T(const Fp& a, const Fp& b) : a(a), b(b) { }
	Fp2T(int64_t a, int64_t b) : a(a), b(b) { }
	Fp* getFp0() { return &a; }
	const Fp* getFp0() const { return &a; }
	const Unit* getUnit() const { return a.getUnit(); }
	void clear()
	{
		a.clear();
		b.clear();
	}
	void set(const Fp &a_, const Fp &b_)
	{
		a = a_;
		b = b_;
	}
#ifdef MCL_XBYAK_DIRECT_CALL
	static void (*add)(Fp2T& z, const Fp2T& x, const Fp2T& y);
	static void (*sub)(Fp2T& z, const Fp2T& x, const Fp2T& y);
	static void (*neg)(Fp2T& y, const Fp2T& x);
	static void (*mul)(Fp2T& z, const Fp2T& x, const Fp2T& y);
	static void (*sqr)(Fp2T& y, const Fp2T& x);
#else
	static void add(Fp2T& z, const Fp2T& x, const Fp2T& y) { addC(z, x, y); }
	static void sub(Fp2T& z, const Fp2T& x, const Fp2T& y) { subC(z, x, y); }
	static void neg(Fp2T& y, const Fp2T& x) { negC(y, x); }
	static void mul(Fp2T& z, const Fp2T& x, const Fp2T& y) { mulC(z, x, y); }
	static void sqr(Fp2T& y, const Fp2T& x) { sqrC(y, x); }
#endif
	static void (*mul_xi)(Fp2T& y, const Fp2T& x);
	static void addPre(Fp2T& z, const Fp2T& x, const Fp2T& y) { Fp::addPre(z.a, x.a, y.a); Fp::addPre(z.b, x.b, y.b); }
	static void inv(Fp2T& y, const Fp2T& x) { Fp::op_.fp2_inv(y.a.v_, x.a.v_); }
	static void divBy2(Fp2T& y, const Fp2T& x)
	{
		Fp::divBy2(y.a, x.a);
		Fp::divBy2(y.b, x.b);
	}
	static void divBy4(Fp2T& y, const Fp2T& x)
	{
		Fp::divBy4(y.a, x.a);
		Fp::divBy4(y.b, x.b);
	}
	static void mulFp(Fp2T& z, const Fp2T& x, const Fp& y)
	{
		Fp::mul(z.a, x.a, y);
		Fp::mul(z.b, x.b, y);
	}
	template<class S>
	void setArray(bool *pb, const S *buf, size_t n)
	{
		assert((n & 1) == 0);
		n /= 2;
		a.setArray(pb, buf, n);
		if (!*pb) return;
		b.setArray(pb, buf + n, n);
	}
	template<class InputStream>
	void load(bool *pb, InputStream& is, int ioMode)
	{
		a.load(pb, is, ioMode);
		if (!*pb) return;
		b.load(pb, is, ioMode);
	}
	/*
		Fp2T = <a> + ' ' + <b>
	*/
	template<class OutputStream>
	void save(bool *pb, OutputStream& os, int ioMode) const
	{
		const char sep = *fp::getIoSeparator(ioMode);
		a.save(pb, os, ioMode);
		if (!*pb) return;
		if (sep) {
			cybozu::writeChar(pb, os, sep);
			if (!*pb) return;
		}
		b.save(pb, os, ioMode);
	}
	bool isZero() const { return a.isZero() && b.isZero(); }
	bool isOne() const { return a.isOne() && b.isZero(); }
	bool operator==(const Fp2T& rhs) const { return a == rhs.a && b == rhs.b; }
	bool operator!=(const Fp2T& rhs) const { return !operator==(rhs); }
	/*
		return true is a is odd (do not consider b)
		this function is for only compressed reprezentation of EC
		isOdd() is not good naming. QQQ
	*/
	bool isOdd() const { return a.isOdd(); }
	/*
		(a + bi)^2 = (a^2 - b^2) + 2ab i = c + di
		A = a^2
		B = b^2
		A = (c +/- sqrt(c^2 + d^2))/2
		b = d / 2a
	*/
	static inline bool squareRoot(Fp2T& y, const Fp2T& x)
	{
		Fp t1, t2;
		if (x.b.isZero()) {
			if (Fp::squareRoot(t1, x.a)) {
				y.a = t1;
				y.b.clear();
			} else {
				bool b = Fp::squareRoot(t1, -x.a);
				assert(b); (void)b;
				y.a.clear();
				y.b = t1;
			}
			return true;
		}
		Fp::sqr(t1, x.a);
		Fp::sqr(t2, x.b);
		t1 += t2; // c^2 + d^2
		if (!Fp::squareRoot(t1, t1)) return false;
		Fp::add(t2, x.a, t1);
		Fp::divBy2(t2, t2);
		if (!Fp::squareRoot(t2, t2)) {
			Fp::sub(t2, x.a, t1);
			Fp::divBy2(t2, t2);
			bool b = Fp::squareRoot(t2, t2);
			assert(b); (void)b;
		}
		y.a = t2;
		t2 += t2;
		Fp::inv(t2, t2);
		Fp::mul(y.b, x.b, t2);
		return true;
	}
	static void inline norm(Fp& y, const Fp2T& x)
	{
		Fp aa, bb;
		Fp::sqr(aa, x.a);
		Fp::sqr(bb, x.b);
		Fp::add(y, aa, bb);
	}
	/*
		Frobenius
		i^2 = -1
		(a + bi)^p = a + bi^p in Fp
		= a + bi if p = 1 mod 4
		= a - bi if p = 3 mod 4
	*/
	static void Frobenius(Fp2T& y, const Fp2T& x)
	{
		if (Fp::getOp().pmod4 == 1) {
			if (&y != &x) {
				y = x;
			}
		} else {
			if (&y != &x) {
				y.a = x.a;
			}
			Fp::neg(y.b, x.b);
		}
	}

	static uint32_t get_xi_a() { return Fp::getOp().xi_a; }
	static void init()
	{
//		assert(Fp::maxSize <= 256);
		mcl::fp::Op& op = Fp::op_;
		assert(op.xi_a);
#ifdef MCL_XBYAK_DIRECT_CALL
		add = fp::func_ptr_cast<void (*)(Fp2T& z, const Fp2T& x, const Fp2T& y)>(op.fp2_addA_);
		if (add == 0) add = addC;
		sub = fp::func_ptr_cast<void (*)(Fp2T& z, const Fp2T& x, const Fp2T& y)>(op.fp2_subA_);
		if (sub == 0) sub = subC;
		neg = fp::func_ptr_cast<void (*)(Fp2T& y, const Fp2T& x)>(op.fp2_negA_);
		if (neg == 0) neg = negC;
		mul = fp::func_ptr_cast<void (*)(Fp2T& z, const Fp2T& x, const Fp2T& y)>(op.fp2_mulA_);
		if (mul == 0) mul = mulC;
		sqr = fp::func_ptr_cast<void (*)(Fp2T& y, const Fp2T& x)>(op.fp2_sqrA_);
		if (sqr == 0) sqr = sqrC;
		mul_xi = fp::func_ptr_cast<void (*)(Fp2T&, const Fp2T&)>(op.fp2_mul_xiA_);
#endif
		op.fp2_inv = fp2_invW;
		if (mul_xi == 0) {
			if (op.xi_a == 1) {
				mul_xi = fp2_mul_xi_1_1iC;
			} else {
				mul_xi = fp2_mul_xiC;
			}
		}
		FpDblT<Fp>::init();
		Fp2DblT<Fp>::init();
		// call init before Fp2::pow because FpDbl is used in Fp2T
		const Fp2T xi(op.xi_a, 1);
		const mpz_class& p = Fp::getOp().mp;
		Fp2T::pow(g[0], xi, (p - 1) / 6); // g = xi^((p-1)/6)
		for (size_t i = 1; i < gN; i++) {
			g[i] = g[i - 1] * g[0];
		}
		/*
			permutate [0, 1, 2, 3, 4] => [1, 3, 0, 2, 4]
			g[0] = g^2
			g[1] = g^4
			g[2] = g^1
			g[3] = g^3
			g[4] = g^5
		*/
		{
			Fp2T t = g[0];
			g[0] = g[1];
			g[1] = g[3];
			g[3] = g[2];
			g[2] = t;
		}
		for (size_t i = 0; i < gN; i++) {
			Fp2T t(g[i].a, g[i].b);
			if (Fp::getOp().pmod4 == 3) Fp::neg(t.b, t.b);
			Fp2T::mul(g2[i], t, g[i]);
			g3[i] = g[i] * g2[i];
		}
	}
#ifndef CYBOZU_DONT_USE_EXCEPTION
	template<class InputStream>
	void load(InputStream& is, int ioMode = IoSerialize)
	{
		bool b;
		load(&b, is, ioMode);
		if (!b) throw cybozu::Exception("Fp2T:load");
	}
	template<class OutputStream>
	void save(OutputStream& os, int ioMode = IoSerialize) const
	{
		bool b;
		save(&b, os, ioMode);
		if (!b) throw cybozu::Exception("Fp2T:save");
	}
	template<class S>
	void setArray(const S *buf, size_t n)
	{
		bool b;
		setArray(&b, buf, n);
		if (!b) throw cybozu::Exception("Fp2T:setArray");
	}
#endif
#ifndef CYBOZU_DONT_USE_STRING
	Fp2T(const std::string& a, const std::string& b, int base = 0) : a(a, base), b(b, base) {}
	friend std::istream& operator>>(std::istream& is, Fp2T& self)
	{
		self.load(is, fp::detectIoMode(Fp::BaseFp::getIoMode(), is));
		return is;
	}
	friend std::ostream& operator<<(std::ostream& os, const Fp2T& self)
	{
		self.save(os, fp::detectIoMode(Fp::BaseFp::getIoMode(), os));
		return os;
	}
#endif
private:
	/*
		default Fp2T operator
		Fp2T = Fp[i]/(i^2 + 1)
	*/
	static void addC(Fp2T& z, const Fp2T& x, const Fp2T& y)
	{
		Fp::add(z.a, x.a, y.a);
		Fp::add(z.b, x.b, y.b);
	}
	static void subC(Fp2T& z, const Fp2T& x, const Fp2T& y)
	{
		Fp::sub(z.a, x.a, y.a);
		Fp::sub(z.b, x.b, y.b);
	}
	static void negC(Fp2T& y, const Fp2T& x)
	{
		Fp::neg(y.a, x.a);
		Fp::neg(y.b, x.b);
	}
#if 0
	/*
		x = a + bi, y = c + di, i^2 = -1
		z = xy = (a + bi)(c + di) = (ac - bd) + (ad + bc)i
		ad+bc = (a + b)(c + d) - ac - bd
		# of mod = 3
	*/
	static void fp2_mulW(Unit *z, const Unit *x, const Unit *y)
	{
		const Fp *px = reinterpret_cast<const Fp*>(x);
		const Fp *py = reinterpret_cast<const Fp*>(y);
		const Fp& a = px[0];
		const Fp& b = px[1];
		const Fp& c = py[0];
		const Fp& d = py[1];
		Fp *pz = reinterpret_cast<Fp*>(z);
		Fp t1, t2, ac, bd;
		Fp::add(t1, a, b);
		Fp::add(t2, c, d);
		t1 *= t2; // (a + b)(c + d)
		Fp::mul(ac, a, c);
		Fp::mul(bd, b, d);
		Fp::sub(pz[0], ac, bd); // ac - bd
		Fp::sub(pz[1], t1, ac);
		pz[1] -= bd;
	}
	static void fp2_mulNFW(Fp2T& z, const Fp2T& x, const Fp2T& y)
	{
		const fp::Op& op = Fp::op_;
		op.fp2_mulNF((Unit*)&z, (const Unit*)&x, (const Unit*)&y, op.p);
	}
#endif
	static void mulC(Fp2T& z, const Fp2T& x, const Fp2T& y)
	{
		Fp2Dbl d;
		Fp2Dbl::mulPre(d, x, y);
		FpDbl::mod(z.a, d.a);
		FpDbl::mod(z.b, d.b);
	}
	/*
		x = a + bi, i^2 = -1
		y = x^2 = (a + bi)^2 = (a + b)(a - b) + 2abi
	*/
	static void sqrC(Fp2T& y, const Fp2T& x)
	{
		const Fp& a = x.a;
		const Fp& b = x.b;
#if 1 // faster than using FpDbl
		Fp t1, t2, t3;
		Fp::add(t1, b, b); // 2b
		t1 *= a; // 2ab
		Fp::add(t2, a, b); // a + b
		Fp::sub(t3, a, b); // a - b
		Fp::mul(y.a, t2, t3); // (a + b)(a - b)
		y.b = t1;
#else
		Fp t1, t2;
		FpDbl d1, d2;
		Fp::addPre(t1, b, b); // 2b
		FpDbl::mulPre(d2, t1, a); // 2ab
		Fp::addPre(t1, a, b); // a + b
		Fp::sub(t2, a, b); // a - b
		FpDbl::mulPre(d1, t1, t2); // (a + b)(a - b)
		FpDbl::mod(py[0], d1);
		FpDbl::mod(py[1], d2);
#endif
	}
	/*
		xi = xi_a + i
		x = a + bi
		y = (a + bi)xi = (a + bi)(xi_a + i)
		=(a * x_ia - b) + (a + b xi_a)i
	*/
	static void fp2_mul_xiC(Fp2T& y, const Fp2T& x)
	{
		const Fp& a = x.a;
		const Fp& b = x.b;
		Fp t;
		Fp::mulUnit(t, a, Fp::getOp().xi_a);
		t -= b;
		Fp::mulUnit(y.b, b, Fp::getOp().xi_a);
		y.b += a;
		y.a = t;
	}
	/*
		xi = 1 + i ; xi_a = 1
		y = (a + bi)xi = (a - b) + (a + b)i
	*/
	static void fp2_mul_xi_1_1iC(Fp2T& y, const Fp2T& x)
	{
		const Fp& a = x.a;
		const Fp& b = x.b;
		Fp t;
		Fp::add(t, a, b);
		Fp::sub(y.a, a, b);
		y.b = t;
	}
	/*
		x = a + bi
		1 / x = (a - bi) / (a^2 + b^2)
	*/
	static void fp2_invW(Unit *y, const Unit *x)
	{
		const Fp *px = reinterpret_cast<const Fp*>(x);
		Fp *py = reinterpret_cast<Fp*>(y);
		const Fp& a = px[0];
		const Fp& b = px[1];
		Fp aa, bb;
		Fp::sqr(aa, a);
		Fp::sqr(bb, b);
		aa += bb;
		Fp::inv(aa, aa); // aa = 1 / (a^2 + b^2)
		Fp::mul(py[0], a, aa);
		Fp::mul(py[1], b, aa);
		Fp::neg(py[1], py[1]);
	}
};

#ifdef MCL_XBYAK_DIRECT_CALL
template<class Fp_> void (*Fp2T<Fp_>::add)(Fp2T& z, const Fp2T& x, const Fp2T& y);
template<class Fp_> void (*Fp2T<Fp_>::sub)(Fp2T& z, const Fp2T& x, const Fp2T& y);
template<class Fp_> void (*Fp2T<Fp_>::neg)(Fp2T& y, const Fp2T& x);
template<class Fp_> void (*Fp2T<Fp_>::mul)(Fp2T& z, const Fp2T& x, const Fp2T& y);
template<class Fp_> void (*Fp2T<Fp_>::sqr)(Fp2T& y, const Fp2T& x);
#endif
template<class Fp_> void (*Fp2T<Fp_>::mul_xi)(Fp2T& y, const Fp2T& x);

template<class Fp>
struct Fp2DblT {
	typedef FpDblT<Fp> FpDbl;
	typedef Fp2T<Fp> Fp2;
	typedef fp::Unit Unit;
	FpDbl a, b;
	static void add(Fp2DblT& z, const Fp2DblT& x, const Fp2DblT& y)
	{
		FpDbl::add(z.a, x.a, y.a);
		FpDbl::add(z.b, x.b, y.b);
	}
	static void addPre(Fp2DblT& z, const Fp2DblT& x, const Fp2DblT& y)
	{
		FpDbl::addPre(z.a, x.a, y.a);
		FpDbl::addPre(z.b, x.b, y.b);
	}
	static void sub(Fp2DblT& z, const Fp2DblT& x, const Fp2DblT& y)
	{
		FpDbl::sub(z.a, x.a, y.a);
		FpDbl::sub(z.b, x.b, y.b);
	}
	static void subPre(Fp2DblT& z, const Fp2DblT& x, const Fp2DblT& y)
	{
		FpDbl::subPre(z.a, x.a, y.a);
		FpDbl::subPre(z.b, x.b, y.b);
	}
	static void neg(Fp2DblT& y, const Fp2DblT& x)
	{
		FpDbl::neg(y.a, x.a);
		FpDbl::neg(y.b, x.b);
	}
	static void mul_xi(Fp2DblT& y, const Fp2DblT& x)
	{
		const uint32_t xi_a = Fp2::get_xi_a();
		if (xi_a == 1) {
			FpDbl t;
			FpDbl::add(t, x.a, x.b);
			FpDbl::sub(y.a, x.a, x.b);
			y.b = t;
		} else {
			FpDbl t;
			FpDbl::mulUnit(t, x.a, xi_a);
			FpDbl::sub(t, t, x.b);
			FpDbl::mulUnit(y.b, x.b, xi_a);
			FpDbl::add(y.b, y.b, x.a);
			y.a = t;
		}
	}
	static void (*mulPre)(Fp2DblT&, const Fp2&, const Fp2&);
	static void (*sqrPre)(Fp2DblT&, const Fp2&);
	static void mod(Fp2& y, const Fp2DblT& x)
	{
		FpDbl::mod(y.a, x.a);
		FpDbl::mod(y.b, x.b);
	}
#ifndef CYBOZU_DONT_USE_STRING
	friend std::ostream& operator<<(std::ostream& os, const Fp2DblT& x)
	{
		return os << x.a << ' ' << x.b;
	}
#endif
	void operator+=(const Fp2DblT& x) { add(*this, *this, x); }
	void operator-=(const Fp2DblT& x) { sub(*this, *this, x); }
	static void init()
 	{
		const mcl::fp::Op& op = Fp::getOp();
		if (op.fp2Dbl_mulPreA_) {
			mulPre = fp::func_ptr_cast<void (*)(Fp2DblT&, const Fp2&, const Fp2&)>(op.fp2Dbl_mulPreA_);
		} else {
			if (op.isFullBit) {
				mulPre = fp2Dbl_mulPreW<true>;
			} else {
				mulPre = fp2Dbl_mulPreW<false>;
			}
		}
		if (op.fp2Dbl_sqrPreA_) {
			sqrPre = fp::func_ptr_cast<void (*)(Fp2DblT&, const Fp2&)>(op.fp2Dbl_sqrPreA_);
		} else {
			if (op.isFullBit) {
				sqrPre = fp2Dbl_sqrPreW<true>;
			} else {
				sqrPre = fp2Dbl_sqrPreW<false>;
			}
		}
	}
	/*
		Fp2Dbl::mulPre by FpDblT
		@note mod of NIST_P192 is fast
	*/
	template<bool isFullBit>
	static void fp2Dbl_mulPreW(Fp2DblT& z, const Fp2& x, const Fp2& y)
	{
		const Fp& a = x.a;
		const Fp& b = x.b;
		const Fp& c = y.a;
		const Fp& d = y.b;
		FpDbl& d0 = z.a;
		FpDbl& d1 = z.b;
		FpDbl d2;
		Fp s, t;
		if (isFullBit) {
			Fp::add(s, a, b);
			Fp::add(t, c, d);
		} else {
			Fp::addPre(s, a, b);
			Fp::addPre(t, c, d);
		}
		FpDbl::mulPre(d1, s, t); // (a + b)(c + d)
		FpDbl::mulPre(d0, a, c);
		FpDbl::mulPre(d2, b, d);
		if (isFullBit) {
			FpDbl::sub(d1, d1, d0); // (a + b)(c + d) - ac
			FpDbl::sub(d1, d1, d2); // (a + b)(c + d) - ac - bd
		} else {
			FpDbl::subPre(d1, d1, d0);
			FpDbl::subPre(d1, d1, d2);
		}
		FpDbl::sub(d0, d0, d2); // ac - bd
	}
	template<bool isFullBit>
	static void fp2Dbl_sqrPreW(Fp2DblT& y, const Fp2& x)
	{
		Fp t1, t2;
		if (isFullBit) {
			Fp::add(t1, x.b, x.b); // 2b
			Fp::add(t2, x.a, x.b); // a + b
		} else {
			Fp::addPre(t1, x.b, x.b); // 2b
			Fp::addPre(t2, x.a, x.b); // a + b
		}
		FpDbl::mulPre(y.b, t1, x.a); // 2ab
		Fp::sub(t1, x.a, x.b); // a - b
		FpDbl::mulPre(y.a, t1, t2); // (a + b)(a - b)
	}
};

template<class Fp> void (*Fp2DblT<Fp>::mulPre)(Fp2DblT&, const Fp2T<Fp>&, const Fp2T<Fp>&);
template<class Fp> void (*Fp2DblT<Fp>::sqrPre)(Fp2DblT&, const Fp2T<Fp>&);

template<class Fp> Fp2T<Fp> Fp2T<Fp>::g[Fp2T<Fp>::gN];
template<class Fp> Fp2T<Fp> Fp2T<Fp>::g2[Fp2T<Fp>::gN];
template<class Fp> Fp2T<Fp> Fp2T<Fp>::g3[Fp2T<Fp>::gN];

template<class Fp>
struct Fp6DblT;
/*
	Fp6T = Fp2[v] / (v^3 - xi)
	x = a + b v + c v^2
*/
template<class _Fp>
struct Fp6T : public fp::Serializable<Fp6T<_Fp>,
	fp::Operator<Fp6T<_Fp> > > {
	typedef _Fp Fp;
	typedef Fp2T<Fp> Fp2;
	typedef Fp2DblT<Fp> Fp2Dbl;
	typedef Fp6DblT<Fp> Fp6Dbl;
	typedef Fp BaseFp;
	Fp2 a, b, c;
	Fp6T() { }
	Fp6T(int64_t a) : a(a) , b(0) , c(0) { }
	Fp6T(const Fp2& a, const Fp2& b, const Fp2& c) : a(a) , b(b) , c(c) { }
	void clear()
	{
		a.clear();
		b.clear();
		c.clear();
	}
	Fp* getFp0() { return a.getFp0(); }
	const Fp* getFp0() const { return a.getFp0(); }
	Fp2* getFp2() { return &a; }
	const Fp2* getFp2() const { return &a; }
	void set(const Fp2 &a_, const Fp2 &b_, const Fp2 &c_)
	{
		a = a_;
		b = b_;
		c = c_;
	}
	bool isZero() const
	{
		return a.isZero() && b.isZero() && c.isZero();
	}
	bool isOne() const
	{
		return a.isOne() && b.isZero() && c.isZero();
	}
	bool operator==(const Fp6T& rhs) const
	{
		return a == rhs.a && b == rhs.b && c == rhs.c;
	}
	bool operator!=(const Fp6T& rhs) const { return !operator==(rhs); }
	template<class InputStream>
	void load(bool *pb, InputStream& is, int ioMode)
	{
		a.load(pb, is, ioMode); if (!*pb) return;
		b.load(pb, is, ioMode); if (!*pb) return;
		c.load(pb, is, ioMode); if (!*pb) return;
	}
	template<class OutputStream>
	void save(bool *pb, OutputStream& os, int ioMode) const
	{
		const char sep = *fp::getIoSeparator(ioMode);
		a.save(pb, os, ioMode); if (!*pb) return;
		if (sep) {
			cybozu::writeChar(pb, os, sep);
			if (!*pb) return;
		}
		b.save(pb, os, ioMode); if (!*pb) return;
		if (sep) {
			cybozu::writeChar(pb, os, sep);
			if (!*pb) return;
		}
		c.save(pb, os, ioMode);
	}
#ifndef CYBOZU_DONT_USE_EXCEPTION
	template<class InputStream>
	void load(InputStream& is, int ioMode = IoSerialize)
	{
		bool b;
		load(&b, is, ioMode);
		if (!b) throw cybozu::Exception("Fp6T:load");
	}
	template<class OutputStream>
	void save(OutputStream& os, int ioMode = IoSerialize) const
	{
		bool b;
		save(&b, os, ioMode);
		if (!b) throw cybozu::Exception("Fp6T:save");
	}
#endif
#ifndef CYBOZU_DONT_USE_STRING
	friend std::istream& operator>>(std::istream& is, Fp6T& self)
	{
		self.load(is, fp::detectIoMode(Fp::BaseFp::getIoMode(), is));
		return is;
	}
	friend std::ostream& operator<<(std::ostream& os, const Fp6T& self)
	{
		self.save(os, fp::detectIoMode(Fp::BaseFp::getIoMode(), os));
		return os;
	}
#endif
	static void add(Fp6T& z, const Fp6T& x, const Fp6T& y)
	{
		Fp2::add(z.a, x.a, y.a);
		Fp2::add(z.b, x.b, y.b);
		Fp2::add(z.c, x.c, y.c);
	}
	static void sub(Fp6T& z, const Fp6T& x, const Fp6T& y)
	{
		Fp2::sub(z.a, x.a, y.a);
		Fp2::sub(z.b, x.b, y.b);
		Fp2::sub(z.c, x.c, y.c);
	}
	static void neg(Fp6T& y, const Fp6T& x)
	{
		Fp2::neg(y.a, x.a);
		Fp2::neg(y.b, x.b);
		Fp2::neg(y.c, x.c);
	}
	/*
		x = a + bv + cv^2, v^3 = xi
		x^2 = (a^2 + 2bc xi) + (c^2 xi + 2ab)v + (b^2 + 2ac)v^2

		b^2 + 2ac = (a + b + c)^2 - a^2 - 2bc - c^2 - 2ab
	*/
	static void sqr(Fp6T& y, const Fp6T& x)
	{
		Fp2 t1, t2, t3;
		Fp2::mul(t1, x.a, x.b);
		t1 += t1; // 2ab
		Fp2::mul(t2, x.b, x.c);
		t2 += t2; // 2bc
		Fp2::sqr(t3, x.c); // c^2
		Fp2::add(y.c, x.a, x.c); // a + c, destroy y.c
		y.c += x.b; // a + b + c
		Fp2::sqr(y.b, y.c); // (a + b + c)^2, destroy y.b
		y.b -= t2; // (a + b + c)^2 - 2bc
		Fp2::mul_xi(t2, t2); // 2bc xi
		Fp2::sqr(y.a, x.a); // a^2, destroy y.a
		y.b -= y.a; // (a + b + c)^2 - 2bc - a^2
		y.a += t2; // a^2 + 2bc xi
		Fp2::sub(y.c, y.b, t3); // (a + b + c)^2 - 2bc - a^2 - c^2
		Fp2::mul_xi(y.b, t3); // c^2 xi
		y.b += t1; // c^2 xi + 2ab
		y.c -= t1; // b^2 + 2ac
	}
	static inline void mul(Fp6T& z, const Fp6T& x, const Fp6T& y);
	/*
		x = a + bv + cv^2, v^3 = xi
		y = 1/x = p/q where
		p = (a^2 - bc xi) + (c^2 xi - ab)v + (b^2 - ac)v^2
		q = c^3 xi^2 + b(b^2 - 3ac)xi + a^3
		  = (a^2 - bc xi)a + ((c^2 xi - ab)c + (b^2 - ac)b) xi
	*/
	static void inv(Fp6T& y, const Fp6T& x)
	{
		const Fp2& a = x.a;
		const Fp2& b = x.b;
		const Fp2& c = x.c;
		Fp2 aa, bb, cc, ab, bc, ac;
		Fp2::sqr(aa, a);
		Fp2::sqr(bb, b);
		Fp2::sqr(cc, c);
		Fp2::mul(ab, a, b);
		Fp2::mul(bc, b, c);
		Fp2::mul(ac, c, a);

		Fp6T p;
		Fp2::mul_xi(p.a, bc);
		Fp2::sub(p.a, aa, p.a); // a^2 - bc xi
		Fp2::mul_xi(p.b, cc);
		p.b -= ab; // c^2 xi - ab
		Fp2::sub(p.c, bb, ac); // b^2 - ac
		Fp2 q, t;
		Fp2::mul(q, p.b, c);
		Fp2::mul(t, p.c, b);
		q += t;
		Fp2::mul_xi(q, q);
		Fp2::mul(t, p.a, a);
		q += t;
		Fp2::inv(q, q);

		Fp2::mul(y.a, p.a, q);
		Fp2::mul(y.b, p.b, q);
		Fp2::mul(y.c, p.c, q);
	}
};

template<class Fp>
struct Fp6DblT {
	typedef Fp2T<Fp> Fp2;
	typedef Fp6T<Fp> Fp6;
	typedef Fp2DblT<Fp> Fp2Dbl;
	typedef Fp6DblT<Fp> Fp6Dbl;
	typedef fp::Unit Unit;
	Fp2Dbl a, b, c;
	static void add(Fp6Dbl& z, const Fp6Dbl& x, const Fp6Dbl& y)
	{
		Fp2Dbl::add(z.a, x.a, y.a);
		Fp2Dbl::add(z.b, x.b, y.b);
		Fp2Dbl::add(z.c, x.c, y.c);
	}
	static void sub(Fp6Dbl& z, const Fp6Dbl& x, const Fp6Dbl& y)
	{
		Fp2Dbl::sub(z.a, x.a, y.a);
		Fp2Dbl::sub(z.b, x.b, y.b);
		Fp2Dbl::sub(z.c, x.c, y.c);
	}
	/*
		x = a + bv + cv^2, y = d + ev + fv^2, v^3 = xi
		xy = (ad + (bf + ce)xi) + ((ae + bd) + cf xi)v + ((af + cd) + be)v^2
		bf + ce = (b + c)(e + f) - be - cf
		ae + bd = (a + b)(e + d) - ad - be
		af + cd = (a + c)(d + f) - ad - cf
	*/
	static void mulPre(Fp6DblT& z, const Fp6& x, const Fp6& y)
	{
//clk.begin();
		const Fp2& a = x.a;
		const Fp2& b = x.b;
		const Fp2& c = x.c;
		const Fp2& d = y.a;
		const Fp2& e = y.b;
		const Fp2& f = y.c;
		Fp2Dbl& za = z.a;
		Fp2Dbl& zb = z.b;
		Fp2Dbl& zc = z.c;
		Fp2Dbl BE;
		Fp2Dbl::mulPre(za, a, d);
		Fp2Dbl::mulPre(BE, b, e);
		Fp2Dbl::mulPre(zb, c, f);

		Fp2 t1, t2, t3, t4;
		Fp2::add(t1, b, c);
		Fp2::add(t2, e, f);
		Fp2Dbl T1;
		Fp2Dbl::mulPre(T1, t1, t2);
		Fp2Dbl::sub(T1, T1, BE);
		Fp2Dbl::sub(T1, T1, zb);
		Fp2Dbl::mul_xi(T1, T1);

		Fp2::add(t2, a, b);
		Fp2::add(t3, e, d);
		Fp2Dbl T2;
		Fp2Dbl::mulPre(T2, t2, t3);
		Fp2Dbl::sub(T2, T2, za);
		Fp2Dbl::sub(T2, T2, BE);

		Fp2::add(t3, a, c);
		Fp2::add(t4, d, f);
		Fp2Dbl::mulPre(zc, t3, t4);
		Fp2Dbl::sub(zc, zc, za);
		Fp2Dbl::sub(zc, zc, zb);

		Fp2Dbl::add(za, za, T1);
		Fp2Dbl::mul_xi(zb, zb);
		Fp2Dbl::add(zb, zb, T2);
		Fp2Dbl::add(zc, zc, BE);
//clk.end();
	}
	static void mod(Fp6& y, const Fp6Dbl& x)
	{
		Fp2Dbl::mod(y.a, x.a);
		Fp2Dbl::mod(y.b, x.b);
		Fp2Dbl::mod(y.c, x.c);
	}
};

template<class Fp>
inline void Fp6T<Fp>::mul(Fp6T<Fp>& z, const Fp6T<Fp>& x, const Fp6T<Fp>& y)
{
	Fp6DblT<Fp> Z;
	Fp6DblT<Fp>::mulPre(Z, x, y);
	Fp6DblT<Fp>::mod(z, Z);
}

/*
	Fp12T = Fp6[w] / (w^2 - v)
	x = a + b w
*/
template<class Fp>
struct Fp12T : public fp::Serializable<Fp12T<Fp>,
	fp::Operator<Fp12T<Fp> > > {
	typedef Fp2T<Fp> Fp2;
	typedef Fp6T<Fp> Fp6;
	typedef Fp2DblT<Fp> Fp2Dbl;
	typedef Fp6DblT<Fp> Fp6Dbl;
	typedef Fp BaseFp;
	Fp6 a, b;
	Fp12T() {}
	Fp12T(int64_t a) : a(a), b(0) {}
	Fp12T(const Fp6& a, const Fp6& b) : a(a), b(b) {}
	void clear()
	{
		a.clear();
		b.clear();
	}
	void setOne()
	{
		clear();
		a.a.a = 1;
	}

	Fp* getFp0() { return a.getFp0(); }
	const Fp* getFp0() const { return a.getFp0(); }
	Fp2* getFp2() { return a.getFp2(); }
	const Fp2* getFp2() const { return a.getFp2(); }
	void set(const Fp2& v0, const Fp2& v1, const Fp2& v2, const Fp2& v3, const Fp2& v4, const Fp2& v5)
	{
		a.set(v0, v1, v2);
		b.set(v3, v4, v5);
	}

	bool isZero() const
	{
		return a.isZero() && b.isZero();
	}
	bool isOne() const
	{
		return a.isOne() && b.isZero();
	}
	bool operator==(const Fp12T& rhs) const
	{
		return a == rhs.a && b == rhs.b;
	}
	bool operator!=(const Fp12T& rhs) const { return !operator==(rhs); }
	static void add(Fp12T& z, const Fp12T& x, const Fp12T& y)
	{
		Fp6::add(z.a, x.a, y.a);
		Fp6::add(z.b, x.b, y.b);
	}
	static void sub(Fp12T& z, const Fp12T& x, const Fp12T& y)
	{
		Fp6::sub(z.a, x.a, y.a);
		Fp6::sub(z.b, x.b, y.b);
	}
	static void neg(Fp12T& z, const Fp12T& x)
	{
		Fp6::neg(z.a, x.a);
		Fp6::neg(z.b, x.b);
	}
	/*
		z = x v + y
		in Fp6 : (a + bv + cv^2)v = cv^3 + av + bv^2 = cxi + av + bv^2
	*/
	static void mulVadd(Fp6& z, const Fp6& x, const Fp6& y)
	{
		Fp2 t;
		Fp2::mul_xi(t, x.c);
		Fp2::add(z.c, x.b, y.c);
		Fp2::add(z.b, x.a, y.b);
		Fp2::add(z.a, t, y.a);
	}
	static void mulVadd(Fp6Dbl& z, const Fp6Dbl& x, const Fp6Dbl& y)
	{
		Fp2Dbl t;
		Fp2Dbl::mul_xi(t, x.c);
		Fp2Dbl::add(z.c, x.b, y.c);
		Fp2Dbl::add(z.b, x.a, y.b);
		Fp2Dbl::add(z.a, t, y.a);
	}
	/*
		x = a + bw, y = c + dw, w^2 = v
		z = xy = (a + bw)(c + dw) = (ac + bdv) + (ad + bc)w
		ad+bc = (a + b)(c + d) - ac - bd

		in Fp6 : (a + bv + cv^2)v = cv^3 + av + bv^2 = cxi + av + bv^2
	*/
	static void mul(Fp12T& z, const Fp12T& x, const Fp12T& y)
	{
		// 4.7Kclk -> 4.55Kclk
		const Fp6& a = x.a;
		const Fp6& b = x.b;
		const Fp6& c = y.a;
		const Fp6& d = y.b;
		Fp6 t1, t2;
		Fp6::add(t1, a, b);
		Fp6::add(t2, c, d);
#if 1
		Fp6Dbl T, AC, BD;
		Fp6Dbl::mulPre(AC, a, c);
		Fp6Dbl::mulPre(BD, b, d);
		mulVadd(T, BD, AC);
		Fp6Dbl::mod(z.a, T);
		Fp6Dbl::mulPre(T, t1, t2); // (a + b)(c + d)
		Fp6Dbl::sub(T, T, AC);
		Fp6Dbl::sub(T, T, BD);
		Fp6Dbl::mod(z.b, T);
#else
		Fp6 ac, bd;
		t1 *= t2; // (a + b)(c + d)
		Fp6::mul(ac, a, c);
		Fp6::mul(bd, b, d);
		mulVadd(z.a, bd, ac);
		t1 -= ac;
		Fp6::sub(z.b, t1, bd);
#endif
	}
	/*
		x = a + bw, w^2 = v
		y = x^2 = (a + bw)^2 = (a^2 + b^2v) + 2abw
		a^2 + b^2v = (a + b)(bv + a) - (abv + ab)
	*/
	static void sqr(Fp12T& y, const Fp12T& x)
	{
		const Fp6& a = x.a;
		const Fp6& b = x.b;
		Fp6 t0, t1;
		Fp6::add(t0, a, b); // a + b
		mulVadd(t1, b, a); // bv + a
		t0 *= t1; // (a + b)(bv + a)
		Fp6::mul(t1, a, b); // ab
		Fp6::add(y.b, t1, t1); // 2ab
		mulVadd(y.a, t1, t1); // abv + ab
		Fp6::sub(y.a, t0, y.a);
	}
	/*
		x = a + bw, w^2 = v
		y = 1/x = (a - bw) / (a^2 - b^2v)
	*/
	static void inv(Fp12T& y, const Fp12T& x)
	{
		const Fp6& a = x.a;
		const Fp6& b = x.b;
		Fp6 t0, t1;
		Fp6::sqr(t0, a);
		Fp6::sqr(t1, b);
		Fp2::mul_xi(t1.c, t1.c);
		t0.a -= t1.c;
		t0.b -= t1.a;
		t0.c -= t1.b; // t0 = a^2 - b^2v
		Fp6::inv(t0, t0);
		Fp6::mul(y.a, x.a, t0);
		Fp6::mul(y.b, x.b, t0);
		Fp6::neg(y.b, y.b);
	}
	/*
		y = 1 / x = conjugate of x if |x| = 1
	*/
	static void unitaryInv(Fp12T& y, const Fp12T& x)
	{
		if (&y != &x) y.a = x.a;
		Fp6::neg(y.b, x.b);
	}
	/*
		Frobenius
		i^2 = -1
		(a + bi)^p = a + bi^p in Fp
		= a + bi if p = 1 mod 4
		= a - bi if p = 3 mod 4

		g = xi^(p - 1) / 6
		v^3 = xi in Fp2
		v^p = ((v^6) ^ (p-1)/6) v = g^2 v
		v^2p = g^4 v^2
		(a + bv + cv^2)^p in Fp6
		= F(a) + F(b)g^2 v + F(c) g^4 v^2

		w^p = ((w^6) ^ (p-1)/6) w = g w
		((a + bv + cv^2)w)^p in Fp12T
		= (F(a) g + F(b) g^3 v + F(c) g^5 v^2)w
	*/
	static void Frobenius(Fp12T& y, const Fp12T& x)
	{
		for (int i = 0; i < 6; i++) {
			Fp2::Frobenius(y.getFp2()[i], x.getFp2()[i]);
		}
		for (int i = 1; i < 6; i++) {
			y.getFp2()[i] *= Fp2::get_gTbl()[i - 1];
		}
	}
	static void Frobenius2(Fp12T& y, const Fp12T& x)
	{
#if 0
		Frobenius(y, x);
		Frobenius(y, y);
#else
		y.getFp2()[0] = x.getFp2()[0];
		if (Fp::getOp().pmod4 == 1) {
			for (int i = 1; i < 6; i++) {
				Fp2::mul(y.getFp2()[i], x.getFp2()[i], Fp2::get_g2Tbl()[i]);
			}
		} else {
			for (int i = 1; i < 6; i++) {
				Fp2::mulFp(y.getFp2()[i], x.getFp2()[i], Fp2::get_g2Tbl()[i - 1].a);
			}
		}
#endif
	}
	static void Frobenius3(Fp12T& y, const Fp12T& x)
	{
#if 0
		Frobenius(y, x);
		Frobenius(y, y);
		Frobenius(y, y);
#else
		Fp2::Frobenius(y.getFp2()[0], x.getFp2()[0]);
		for (int i = 1; i < 6; i++) {
			Fp2::Frobenius(y.getFp2()[i], x.getFp2()[i]);
			y.getFp2()[i] *= Fp2::get_g3Tbl()[i - 1];
		}
#endif
	}
	template<class InputStream>
	void load(bool *pb, InputStream& is, int ioMode)
	{
		a.load(pb, is, ioMode); if (!*pb) return;
		b.load(pb, is, ioMode);
	}
	template<class OutputStream>
	void save(bool *pb, OutputStream& os, int ioMode) const
	{
		const char sep = *fp::getIoSeparator(ioMode);
		a.save(pb, os, ioMode); if (!*pb) return;
		if (sep) {
			cybozu::writeChar(pb, os, sep);
			if (!*pb) return;
		}
		b.save(pb, os, ioMode);
	}
#ifndef CYBOZU_DONT_USE_EXCEPTION
	template<class InputStream>
	void load(InputStream& is, int ioMode = IoSerialize)
	{
		bool b;
		load(&b, is, ioMode);
		if (!b) throw cybozu::Exception("Fp12T:load");
	}
	template<class OutputStream>
	void save(OutputStream& os, int ioMode = IoSerialize) const
	{
		bool b;
		save(&b, os, ioMode);
		if (!b) throw cybozu::Exception("Fp12T:save");
	}
#endif
#ifndef CYBOZU_DONT_USE_STRING
	friend std::istream& operator>>(std::istream& is, Fp12T& self)
	{
		self.load(is, fp::detectIoMode(Fp::BaseFp::getIoMode(), is));
		return is;
	}
	friend std::ostream& operator<<(std::ostream& os, const Fp12T& self)
	{
		self.save(os, fp::detectIoMode(Fp::BaseFp::getIoMode(), os));
		return os;
	}
#endif
};

/*
	convert multiplicative group to additive group
*/
template<class T>
struct GroupMtoA : public T {
	static T& castT(GroupMtoA& x) { return static_cast<T&>(x); }
	static const T& castT(const GroupMtoA& x) { return static_cast<const T&>(x); }
	void clear()
	{
		castT(*this) = 1;
	}
	bool isZero() const { return castT(*this).isOne(); }
	static void add(GroupMtoA& z, const GroupMtoA& x, const GroupMtoA& y)
	{
		T::mul(castT(z), castT(x), castT(y));
	}
	static void dbl(GroupMtoA& y, const GroupMtoA& x)
	{
		T::sqr(castT(y), castT(x));
	}
	static void neg(GroupMtoA& y, const GroupMtoA& x)
	{
		// assume Fp12
		T::unitaryInv(castT(y), castT(x));
	}
	static void Frobenus(GroupMtoA& y, const GroupMtoA& x)
	{
		T::Frobenius(castT(y), castT(x));
	}
	template<class INT>
	static void mul(GroupMtoA& z, const GroupMtoA& x, const INT& y)
	{
		T::pow(castT(z), castT(x), y);
	}
	template<class INT>
	static void mulGeneric(GroupMtoA& z, const GroupMtoA& x, const INT& y)
	{
		T::powGeneric(castT(z), castT(x), y);
	}
	void operator+=(const GroupMtoA& rhs)
	{
		add(*this, *this, rhs);
	}
	void normalize() {}
private:
	bool isOne() const;
};

} // mcl

