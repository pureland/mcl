#define PUT(x) std::cout << #x "=" << x << std::endl;
#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/benchmark.hpp>
cybozu::CpuClock clk;
#include <cybozu/test.hpp>
#include <mcl/bls12_381.hpp>
#include <cybozu/option.hpp>
#include <cybozu/xorshift.hpp>

#if defined(__EMSCRIPTEN__) && !defined(MCL_AVOID_EXCEPTION_TEST)
	#define MCL_AVOID_EXCEPTION_TEST
#endif

//typedef mcl::bls12::BLS12::Compress Compress;
using namespace mcl::bls12_381;

mcl::fp::Mode g_mode;

const struct TestSet {
	mcl::bls12::CurveParam cp;
	const char *name;
	const char *p;
	const char *r;
	struct G2 {
		const char *aa;
		const char *ab;
		const char *ba;
		const char *bb;
	} g2;
	struct G1 {
		const char *a;
		const char *b;
	} g1;
	const char *e;
} g_testSetTbl[] = {
	{
		mcl::bls12::CurveFp381,
		"CurveFp381",
		"0x1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab",
		"0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001",
		{
			"0x024aa2b2f08f0a91260805272dc51051c6e47ad4fa403b02b4510b647ae3d1770bac0326a805bbefd48056c8c121bdb8",
			"0x13e02b6052719f607dacd3a088274f65596bd0d09920b61ab5da61bbdc7f5049334cf11213945d57e5ac7d055d042b7e",
			"0x0ce5d527727d6e118cc9cdc6da2e351aadfd9baa8cbdd3a76d429a695160d12c923ac9cc3baca289e193548608b82801",
			"0x0606c4a02ea734cc32acd2b02bc28b99cb3e287e85a763af267492ab572e99ab3f370d275cec1da1aaa9075ff05f79be",
		},
		{
			"0x17f1d3a73197d7942695638c4fa9ac0fc3688c4f9774b905a14e3a3f171bac586c55e83ff97a1aeffb3af00adb22c6bb",
			"0x08b3f481e3aaa0f1a09e30ed741d8ae4fcf5e095d5d00af600db18cb2c04b3edd03cc744a2888ae40caa232946c5e7e1",
		},
		"0x1250EBD871FC0A92A7B2D83168D0D727272D441BEFA15C503DD8E90CE98DB3E7B6D194F60839C508A84305AACA1789B6 "
		"0x089A1C5B46E5110B86750EC6A532348868A84045483C92B7AF5AF689452EAFABF1A8943E50439F1D59882A98EAA0170F "
		"0x1368BB445C7C2D209703F239689CE34C0378A68E72A6B3B216DA0E22A5031B54DDFF57309396B38C881C4C849EC23E87 "
		"0x193502B86EDB8857C273FA075A50512937E0794E1E65A7617C90D8BD66065B1FFFE51D7A579973B1315021EC3C19934F "
		"0x01B2F522473D171391125BA84DC4007CFBF2F8DA752F7C74185203FCCA589AC719C34DFFBBAAD8431DAD1C1FB597AAA5 "
		"0x018107154F25A764BD3C79937A45B84546DA634B8F6BE14A8061E55CCEBA478B23F7DACAA35C8CA78BEAE9624045B4B6 "
		"0x19F26337D205FB469CD6BD15C3D5A04DC88784FBB3D0B2DBDEA54D43B2B73F2CBB12D58386A8703E0F948226E47EE89D "
		"0x06FBA23EB7C5AF0D9F80940CA771B6FFD5857BAAF222EB95A7D2809D61BFE02E1BFD1B68FF02F0B8102AE1C2D5D5AB1A "
		"0x11B8B424CD48BF38FCEF68083B0B0EC5C81A93B330EE1A677D0D15FF7B984E8978EF48881E32FAC91B93B47333E2BA57 "
		"0x03350F55A7AEFCD3C31B4FCB6CE5771CC6A0E9786AB5973320C806AD360829107BA810C5A09FFDD9BE2291A0C25A99A2 "
		"0x04C581234D086A9902249B64728FFD21A189E87935A954051C7CDBA7B3872629A4FAFC05066245CB9108F0242D0FE3EF "
		"0x0F41E58663BF08CF068672CBD01A7EC73BACA4D72CA93544DEFF686BFD6DF543D48EAA24AFE47E1EFDE449383B676631 "
	},
};

CYBOZU_TEST_AUTO(size)
{
	CYBOZU_TEST_EQUAL(sizeof(Fp), 48u);
	CYBOZU_TEST_EQUAL(sizeof(Fp2), sizeof(Fp) * 2);
	CYBOZU_TEST_EQUAL(sizeof(Fp6), sizeof(Fp) * 6);
	CYBOZU_TEST_EQUAL(sizeof(Fp12), sizeof(Fp) * 12);
	CYBOZU_TEST_EQUAL(sizeof(G1), sizeof(Fp) * 3);
	CYBOZU_TEST_EQUAL(sizeof(G2), sizeof(Fp2) * 3);
}

void testParam(const TestSet& ts)
{
	CYBOZU_TEST_EQUAL(BLS12::param.r, mpz_class(ts.r));
	CYBOZU_TEST_EQUAL(BLS12::param.p, mpz_class(ts.p));
}

void finalExpC(Fp12& y, const Fp12& x)
{
	const mpz_class& r = BLS12::param.r;
	const mpz_class& p = BLS12::param.p;
	mpz_class p2 = p * p;
	mpz_class p4 = p2 * p2;
#if 1
	Fp12::pow(y, x, p2 + 1);
	Fp12::pow(y, y, p4 * p2 - 1);
	Fp12::pow(y, y, (p4 - p2 + 1) / r * 3);
#else
	Fp12::pow(y, x, (p4 * p4 * p4 - 1) / r * 3);
#endif
}

void pairingC(Fp12& e, const G1& P, const G2& Q)
{
	BLS12::millerLoop(e, P, Q);
	BLS12::finalExp(e, e);
}
void testIoAll(const G1& P, const G2& Q)
{
	const int FpTbl[] = { 0, 2, 2|mcl::IoPrefix, 10, 16, 16|mcl::IoPrefix, mcl::IoArray, mcl::IoArrayRaw };
	const int EcTbl[] = { mcl::IoEcAffine, mcl::IoEcProj, mcl::IoEcCompY, mcl::IoSerialize };
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(FpTbl); i++) {
		for (size_t j = 0; j < CYBOZU_NUM_OF_ARRAY(EcTbl); j++) {
			G1 P2 = P, P3;
			G2 Q2 = Q, Q3;
			int ioMode = FpTbl[i] | EcTbl[j];
			std::string s = P2.getStr(ioMode);
			P3.setStr(s, ioMode);
			CYBOZU_TEST_EQUAL(P2, P3);
			s = Q2.getStr(ioMode);
			Q3.setStr(s, ioMode);
			CYBOZU_TEST_EQUAL(Q2, Q3);
			s = P.x.getStr(ioMode);
			Fp Px;
			Px.setStr(s, ioMode);
			CYBOZU_TEST_EQUAL(P.x, Px);
			s = Q.x.getStr(ioMode);
			Fp2 Qx;
			Qx.setStr(s, ioMode);
			CYBOZU_TEST_EQUAL(Q.x, Qx);
		}
	}
}

void testIo(const G1& P, const G2& Q)
{
	testIoAll(P, Q);
	G1 Z1;
	G2 Z2;
	Z1.clear();
	Z2.clear();
	testIoAll(Z1, Z2);
}

void testSetStr(const G2& Q0)
{
	G2::setCompressedExpression();
	G2 Q;
	Q.clear();
	for (int i = 0; i < 10; i++) {
		G2 R;
		R.setStr(Q.getStr());
		CYBOZU_TEST_EQUAL(Q, R);
		G2::add(Q, Q, Q0);
	}
}

#if 0
void testMapToG1()
{
	G1 g;
	for (int i = 1; i < 10; i++) {
		BLS12::mapToG1(g, i);
		CYBOZU_TEST_ASSERT(!g.isZero());
		G1 gr;
		G1::mul(gr, g, BLS12::param.r);
		CYBOZU_TEST_ASSERT(gr.isZero());
	}
#ifndef MCL_AVOID_EXCEPTION_TEST
	if (BLS12::param.b == 2) {
		CYBOZU_TEST_EXCEPTION(BLS12::mapToG1(g, 0), cybozu::Exception);
		CYBOZU_TEST_EXCEPTION(BLS12::mapToG1(g, BLS12::param.mapTo.c1), cybozu::Exception);
		CYBOZU_TEST_EXCEPTION(BLS12::mapToG1(g, -BLS12::param.mapTo.c1), cybozu::Exception);
	}
#endif
}

void testMapToG2()
{
	G2 g;
	for (int i = 1; i < 10; i++) {
		BLS12::mapToG2(g, i);
		CYBOZU_TEST_ASSERT(!g.isZero());
		G2 gr;
		G2::mul(gr, g, BLS12::param.r);
		CYBOZU_TEST_ASSERT(gr.isZero());
	}
#ifndef MCL_AVOID_EXCEPTION_TEST
	if (BLS12::param.b == 2) {
		CYBOZU_TEST_EXCEPTION(BLS12::mapToG2(g, 0), cybozu::Exception);
	}
#endif
	Fp x;
	x.setHashOf("abc");
	BLS12::mapToG2(g, Fp2(x, 0));
	CYBOZU_TEST_ASSERT(g.isValid());
}
#endif

void testPrecomputed(const G1& P, const G2& Q)
{
	Fp12 e1, e2;
	BLS12::pairing(e1, P, Q);
	std::vector<Fp6> Qcoeff;
	BLS12::precomputeG2(Qcoeff, Q);
	BLS12::precomputedMillerLoop(e2, P, Qcoeff);
	BLS12::finalExp(e2, e2);
	CYBOZU_TEST_EQUAL(e1, e2);
}

#if  0
void testFp12pow(const G1& P, const G2& Q)
{
	Fp12 e, e1, e2;
	BLS12::pairing(e, P, Q);
	cybozu::XorShift rg;
	for (int i = -10; i < 10; i++) {
		mpz_class xm = i;
		Fp12::pow(e1, e, xm);
		Fp12::powGeneric(e2, e, xm);
		CYBOZU_TEST_EQUAL(e1, e2);
	}
	for (int i = 0; i < 10; i++) {
		Fr x;
		x.setRand(rg);
		mpz_class xm = x.getMpz();
		Fp12::pow(e1, e, xm);
		BLS12::param.glv2.pow(e2, e, xm);
		CYBOZU_TEST_EQUAL(e1, e2);
	}
}
#endif

void testMillerLoop2(const G1& P1, const G2& Q1)
{
	Fp12 e1, e2;
	mpz_class c1("12342342423442");
	mpz_class c2("329428049820348209482");
	G2 Q2;
	G1 P2;
	G2::mul(Q2, Q1, c1);
	G1::mul(P2, P1, c2);
	BLS12::pairing(e1, P1, Q1);
	BLS12::pairing(e2, P2, Q2);
	e1 *= e2;

	std::vector<Fp6> Q1coeff, Q2coeff;
	BLS12::precomputeG2(Q1coeff, Q1);
	BLS12::precomputeG2(Q2coeff, Q2);
	BLS12::precomputedMillerLoop2(e2, P1, Q1coeff, P2, Q2coeff);
	BLS12::finalExp(e2, e2);
	CYBOZU_TEST_EQUAL(e1, e2);
}

void testPairing(const G1& P, const G2& Q, const char *eStr)
{
	Fp12 e1;
	BLS12::pairing(e1, P, Q);
	Fp12 e2;
	{
		std::stringstream ss(eStr);
		ss >> e2;
	}
	CYBOZU_TEST_EQUAL(e1, e2);
#ifdef ONLY_BENCH
	for (int i = 0; i < 1000; i++) BLS12::pairing(e1, P, Q);
//	CYBOZU_BENCH_C("pairing", 1000, BLS12::pairing, e1, P, Q); // 2.4Mclk
#else
	{
		Fp12 e = e1, ea;
		G1 Pa;
		G2 Qa;
#if defined(__EMSCRIPTEN__) || MCL_SIZEOF_UNIT == 4
		const int count = 100;
#else
		const int count = 1000;
#endif
		mpz_class a;
		cybozu::XorShift rg;
		for (int i = 0; i < count; i++) {
			Fr r;
			r.setRand(rg);
			a = r.getMpz();
			Fp12::pow(ea, e, a);
			G1::mul(Pa, P, a);
			G2::mul(Qa, Q, a);
			G1 T;
			G1::mulCT(T, P, a);
			CYBOZU_TEST_EQUAL(Pa, T);
			BLS12::pairing(e1, Pa, Q);
			BLS12::pairing(e2, P, Qa);
			CYBOZU_TEST_EQUAL(ea, e1);
			CYBOZU_TEST_EQUAL(ea, e2);
		}
		mpz_class z = 3;
		CYBOZU_BENCH_C("G1::mulCT  ", 500, G1::mulCT, Pa, P, a);
		CYBOZU_BENCH_C("G1::mulCT z", 500, G1::mulCT, Pa, P, z);
		CYBOZU_BENCH_C("G1::mul  ", 500, G1::mul, Pa, Pa, a);
		CYBOZU_BENCH_C("G1::mul z", 500, G1::mul, Pa, Pa, z);
		CYBOZU_BENCH_C("G1::add", 500, G1::add, Pa, Pa, P);
		CYBOZU_BENCH_C("G1::dbl", 500, G1::dbl, Pa, Pa);
		CYBOZU_BENCH_C("G2::mulCT  ", 500, G2::mulCT, Qa, Q, a);
		CYBOZU_BENCH_C("G2::mulCT z", 500, G2::mulCT, Qa, Q, z);
		CYBOZU_BENCH_C("G2::mul  ", 500, G2::mul, Qa, Qa, a);
		CYBOZU_BENCH_C("G2::mul z", 500, G2::mul, Qa, Qa, z);
		CYBOZU_BENCH_C("G2::add", 500, G2::add, Qa, Qa, Q);
		CYBOZU_BENCH_C("G2::dbl", 500, G2::dbl, Qa, Qa);
		CYBOZU_BENCH_C("GT::pow", 500, GT::pow, e1, e1, a);
//		CYBOZU_BENCH_C("GT::powGLV", 500, BLS12::param.glv2.pow, e1, e1, a);
		G1 PP;
		G2 QQ;
//		CYBOZU_BENCH_C("hashAndMapToG1", 500, BLS12::hashAndMapToG1, PP, "abc", 3);
//		CYBOZU_BENCH_C("hashAndMapToG2", 500, BLS12::hashAndMapToG2, QQ, "abc", 3);
	}
	CYBOZU_BENCH("pairing", BLS12::pairing, e1, P, Q); // 2.4Mclk
	CYBOZU_BENCH("finalExp", BLS12::finalExp, e1, e1); // 1.3Mclk
#endif
}

void testTrivial(const G1& P, const G2& Q)
{
	G1 Z1; Z1.clear();
	G2 Z2; Z2.clear();
	Fp12 e;
	BLS12::pairing(e, Z1, Q);
	CYBOZU_TEST_EQUAL(e, 1);
	BLS12::pairing(e, P, Z2);
	CYBOZU_TEST_EQUAL(e, 1);
	BLS12::pairing(e, Z1, Z2);
	CYBOZU_TEST_EQUAL(e, 1);

	std::vector<Fp6> Qcoeff;
	BLS12::precomputeG2(Qcoeff, Z2);
	BLS12::precomputedMillerLoop(e, P, Qcoeff);
	BLS12::finalExp(e, e);
	CYBOZU_TEST_EQUAL(e, 1);

	BLS12::precomputeG2(Qcoeff, Q);
	BLS12::precomputedMillerLoop(e, Z1, Qcoeff);
	BLS12::finalExp(e, e);
	CYBOZU_TEST_EQUAL(e, 1);
}

CYBOZU_TEST_AUTO(naive)
{
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(g_testSetTbl); i++) {
		const TestSet& ts = g_testSetTbl[i];
		printf("i=%d curve=%s\n", int(i), ts.name);
		initPairing(ts.cp, g_mode);
		const G1 P(Fp(ts.g1.a), Fp(ts.g1.b));
		const G2 Q(Fp2(ts.g2.aa, ts.g2.ab), Fp2(ts.g2.ba, ts.g2.bb));
#ifdef ONLY_BENCH
		testPairing(P, Q, ts.e);
		clk.put();
		return;
#endif
		testParam(ts);
		testIo(P, Q);
//		testFp12pow(P, Q);
		testTrivial(P, Q);
		testSetStr(Q);
//		testMapToG1();
//		testMapToG2();
		testPairing(P, Q, ts.e);
		testPrecomputed(P, Q);
		testMillerLoop2(P, Q);
	}
	int count = (int)clk.getCount();
	if (count) {
		printf("count=%d ", count);
		clk.put();
	}
}

CYBOZU_TEST_AUTO(finalExp)
{
	const char *e0Str =
"012974491575E232199B73B30FE53FF643FEAE11023BCA7AF961C3600B45DFECFE4B30D52A62E73DA4C0409810304997\n"
"05CE2FB890FE65E20EC36347190ECB4884E401A64B666557B53E561F6D0979B7A96AD9E647ED78BD47187195C00F563C\n"
"02E85D1E559488603A70FEE99354DA8847215EC97282CA230DE96FED6DD5D4DD4EF4D901DB7F544A1A45EBEBA1450109\n"
"048FB1E44DDABF18D55C95704158A24678AA2A6ED0844108762E88306E5880E8C67BF44E24E40AB3F93D9E3713170341\n"
"07EF7BE685DC0DBA1B3E1D2E9090CD98EAD1325B60881772F17077386A3182B117F5FD839363F5891D08E82B88EC6F12\n"
"17803435700EF7A16C06404C6D17EB4FD84079FE9872207302A36C791B6E90447B33D703BBFE04ECB641C3A573E2CD50\n"
"19A494E6A872E46FC85D09FD6D30844B6FF05729BC253A9640F7BE64AAA8C2C8E0AE014A9DD816C53A3EDEBB2FA649EB\n"
"020949ABAA14F1DCE17FA9E091DDA963E9E492BA788E12B9B610E80A4D94DB9CC50341ED107C7D50E5738052595D4A27\n"
"09E217B513B3603723DAC3188A2F7CBDD84A56E7E5004446E7D4C63D6E378DA26E411C10898E48DB4B0C065E4699A9C5\n"
"12393BD23D0EC122082A1EC892A982F3C9AFD14240CE85258D8A3EF0A13CB545D6EF7848FD40DD4AEF1554341C5C5BBF\n"
"07EA8A0D6A57C78E5663F94E2B1ABC0D760ED18DBA64305EAD5EE350FB0342A7A81C0D5C8B3AD826D009276B0F32D2C8\n"
"16804D0D4A2633ED01568B0F8F06C4497E46E88D05FD191AAE530ACA791D0E114D74874FA88E33FAF48757153B09BB0E";

const char *e1Str =
"0E05D19E90D2C501E5502C7AC80D77201C47DF147DD1076440F0DF0179DF9802CA0775E0E73DD9174F1094D2280787B3\n"
"14D2F5C84279E7177A3543FBEAE261DE8F6C97EFD5F3FF3F959EC9FC0303F620A4B3AF00DF409496CECADDD0A7F0A164\n"
"1414E9B9DF8DF1EAC2E70D5538018377788C62016A54F28B037A68740705089AE431B86756F98CBE19690A5EAC0C2466\n"
"12D8B32157836A131CCA3CA313DAAAF909BC3AD6BDD15885BB429922B9CD7D1246D1163E5E6F88D68BF1B75E451EFABB\n"
"102C9A839A924E0D603D13F2E08A919E0B9EE2A269FC75727BA13D66027C157B9BB4077977FA94557DE4427BF11B234B\n"
"19DBEB7F2E3096AFFD44837655BD8249741B484B0EB0DBEE569DEA8D9E38AE09D210C8BC16AA6DFBC923095B2C9A8B2B\n"
"19B9A6DCCD01FA0D04D5CE94D8BDCE1DF64AFEB7FD493B955180A5C6B236E469F0E07CC9BB4203FCAC46AE6F8E5419D6\n"
"02BFA87AF7A3726A7ABACDCFDD53694AF651554F3A431AB4274F67D5DAD2D6C88AF794705FF456A936C83594731AD8DC\n"
"0F21E0173E3B50DD98EFA815B410631A57399B451FD6E1056FFD09C9FE50EFAD3D026F0C46C8BB1583A50B7853D990DA\n"
"02230237AE04B61F9269F6E7CD2FCF1231CEE4690AA658B0018EFC0D0770FD0A56B3B7294086E8D306B1465CDDD858CD\n"
"087EB8F6547015661E9CD48D6525C808636FCB8420B867CB2A87E006B2A93BBD5EF675E6CDDA9E6F94519C49EA8BB689\n"
"19F5C988B2DD6E33D7D3D34EFB1991F80DC28006AC75E0AB53FD98FC6F2476D05DD4ECA582F5FF72B8DDD9DDDE80FFC9";

	Fp12 e0, e1, e2;
	e0.setStr(e0Str, 16);
	e1.setStr(e1Str, 16);
	BLS12::finalExp(e2, e0);
//	finalExpC(e2, e0);
	CYBOZU_TEST_EQUAL(e1, e2);
	CYBOZU_BENCH_C("finalExp", 100, BLS12::finalExp, e2, e0);
}

CYBOZU_TEST_AUTO(addLine)
{
const char *l0Str=
"0EF586FCDB69442CB41C0DA719AC5C92BD99A916C1F01BCFC7606AA7A23D680C04620FDFC2144E0EA6025F05241A791F\n"
"164CFDADE9B91150C6D2C7F7CDF29BC3105A7EA51217283CDF801EBEE9E86CE5078253E322C72129DAA42F6DBAD17D37";
const char *l1Str =
"07A124F536BE83CCB3AF8D3DA2AE094942755040B9DA8E0796C462ACE3805D6916ACA7E9281261D8025571E2F31AAF0D\n"
"12D05751A9B255143541D0A4E57E120F937D51F9A07D31982390CA6EB5DF8CC0640FD291521069BF9964AE33EDD1323D";
const char *l4Str =
"0D609DE41CF1260B332C1A53AA54703F62AB8224777E34FEEAB09AA06187CA71D8C7C2EB66F59D3622D431BE17D0FEE6\n"
"0A93C2984041171BE701560017D64D0640B6F61D7DCA8F527FA6B6A1A1033261C0761CAA56A00D4D16C9D3B7371E02D9";

const char *rStr =
"4 0A8DFA69FDD43EDCCC6375750373B443157EF4641E5B4CA82FBF23E3E8EA72351EA754168CEC77573D337E6227C0D0DD\n"
"12C8508CF1828C52A9A1A71779129D605327191EE459AED3C0B4B14657B08B2927173FADF8E4275188E8D49E57A75B33\n"
"12AD7EB96734F2C93B669FD54845CD2FF351AFDF0123E96772021DC3F4F3B456DB1B37CB1C380B1947616165FF0DDAEA\n"
"03D80F92C8A6005DEB291AF28406B7B4FCEDD81A244997DBB719B01D162BD7D71F0FD63BF76F8F1AC90618C3702294DF\n"
"199F7A719EA1CA2CD03CFFBB9A4BC2FE1BD8BCA7C772D223E6CB20C1313C3D3C52CFBB88445E56C833908C52A4EC68F1\n"
"0A3F6B27A6DDA00DB848574ECB06F179271D5844BDA66CD5AE1792A8FDD25E3A504A95839113BAA8B1FCB53EEE5F1FF0";

const char *qStr =
"4 0B5C339C23F8EAB3647E974BCDDF72C96F97A444346BE72CA73AB1323B83B8F6161257AB34C7E0CF34F6C45086CA5868\n"
"13C2235E9F9DFB33344BA2EE5A71435859022880732EDC9EC75AC79AE9DA972593CDC40A0AC334D6D2E8D7FAD1D98D0B\n"
"134B8EED8196A00D3B70ADBC26FF963B725A351CF0B73FE1A541788AFB0BB081AF82A438021B5E878B15D53B1D27C6A7\n"
"18CC69F847BEE826B939DCB4030D33020D03B046465C9EE103AA8009A175DB169070294E75771586687FE361DB884BCD\n"
"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001\n"
"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

const char *pStr =
"4 0FD3977C60EC322BC281C915955ED534B491E39C72E8E800271CEF3F0492D890829FA69C45FCE93D9847A0CAB325D871\n"
"17CC2C36C5D283C05BFCECCF48DBB2050332DA058DD67326A9EE520967DBCAEDFCB5F05A085D1A49DF08BB968CC782C5\n"
"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001";

const char *m0Str =
"1010A4F9944514352AAD5D290AFB95C435EB64B5E74519807C9602DABCD6B6F5494E419758AE9A43F539F812252C65E5\n"
"1622F7A52BAAC10EBFF0977F61866544BF9B91315FA66ADB6AC200AFF7A3676C1AD3480340B23C66F7C6AE50A703F245";
const char *m1Str =
"0905180B20FCE2AA545A73D6B9EA1F82479EF3FB5A3BA8DDB656D9B2A4DA7B63CCF75538D15093949B442E27804C8CE2\n"
"0FE834508BBAD2F22DCBF1C3C1BCCD69561827613EB63F4907832A7ABBBC6040CF2E517D0D5E22D3812EEE1EC55640DD";
const char *m4Str =
"1197D94A8DAFE6F72B17A31964CA5D0C3B2A12BEFF959F7DF7A37938C64C01508D12C24479E5C3D314F862A9977D6C7D\n"
"0B0254A26810307964E2A05680C19DE7C63CCBD7CC3558AD366BA48D9F7049E245BF2C54EA0339301739851E6EB2158F";

const char *r1Str =
"4 16A33C4ED01E95833D39EECE223380FE860B6DC1F71B1DDBEE2BE39B5D682B090F18758495E202010D3B9B45A46FF80E\n"
"01CECF5CC255E32B833C16EF3F983CCA9996A33504615909AD5685D9F637BF2357442BEC89DAFC747B69EFEF57D5A213\n"
"061C33850E5E4A418F3B68D6097C3911A551D6DB3C7E21196410F69D510C1A888635B6B699F98C14912A8D97742D36A9\n"
"0457FB78282C2B8F8BA803E77D058EF3BF6B06C6241D6FA6E2B1607F3E9D7597D1A10B7AA4E06B89FBA01736901AD826\n"
"0B45E9B7D311B8C37F7E9262227A4D721F2D148DE6F12EC5D599B45E4790F35B37A1D6928709F438849324A807EC1913\n"
"0E771545F892C247A5365BA1F14934D8ED37483A6B7DD3EB4C3FBA0AC884D7EE9C080C3B39ADA64AE545E7339F83AFB0";

	const int mode = mcl::IoEcProj | 16;
	Fp6 l, m;
	G2 R, Q, R1;
	G1 P;

	R.setStr(rStr, mode);
	Q.setStr(qStr, mode);
	P.setStr(pStr, mode);
	l.a.setStr(l0Str, mode);
	l.b.setStr(l4Str, mode);
	l.c.setStr(l1Str, mode);
	BLS12::addLine(l, R, Q, P);
	m.a.setStr(m0Str, mode);
	m.b.setStr(m4Str, mode);
	m.c.setStr(m1Str, mode);
	R1.setStr(r1Str, mode);
	CYBOZU_TEST_EQUAL(l, m);
	CYBOZU_TEST_EQUAL(R, R1);
}

CYBOZU_TEST_AUTO(dblLine)
{
const char *l0Str=
"0905F47F07FA2177E4B73559D93D9B41A2FD20E0440167E63A0F62321EB73C784405DE360477245F5E4AE8FA2EAEC6FF\n"
"02DA455C573517EE8BD4E40A60851E2EC85CF514351681B89B1B72664E384A3678A25DC5C4CF84C086C13968BC701F91";
const char *l1Str =
"0F48059E31CEF9546D41DEF31FB3BFD13BBCC38248E4846AFD216350B4B661FA1382E74BCF904BE8E33C26FF0D29ABA0\n"
"0D7FAEE426E4A6F32E4DE1CCB76BBA870A183113589CF58A358DC9C284C82B54C65A754F9A71EAD87304744CFD105051";

const char *l4Str =
"162AE8B029F8420BEDE5A399BA19C09FED01DE2748F4458065DBE51044FBFE749B28EF1B7F79A5E4545EB550DD0CFE02\n"
"091042B589FD59FBF286D21475CCCF444D8DCC49389EA3B98AF864DDB9C08BDDEB320D6D88F0C07D7CD1733A41404C1F";

const char *qStr =
"4 047ACF79048EDCA97A19DB1779A44CE610CEA9FDCC62D1C4014B335834BC63414F55B670CCF15A86E58BC3737FB70919\n"
"11D8063B2FFA2E1F5224593FF74D0E9650CAB6AF702B74159F7F97E2CF0843FBCD561D41FEC779BEB48746CD2F30FF74\n"
"17684E8EA85C990DF5472B4EBAF51002CDBBECF79BA2988FC610A5CE09F4A584248DCC506E78C39C9BB4F6008115DE64\n"
"1334CA7263ED9395BBEDBB6C938C642200C239FB0CADF1F652332A037FFBC7E567CCE09540939F25316CBC4FC68CE4DB\n"
"0670DCF344F027CB92F7B1F569B281C8FF756D83CD7B65EB118FE95FBE16ED28649B8F739FE3A2BECA1979FC18484ECD\n"
"13E3E30759DCC1FA9F1A62D54BEF0EE1CC75E194A559F2D6BE3025BE4BEB9F7351A7608FE8D4CCAD30BA2A8748205487";

const char *pStr =
"4 1579B48AE944AFE8FC69B38A7CD0D2C6B93E5F506535A5410E25FB1C1707938D6932F3D620A2BBB90ED7E2601971DEA8\n"
"0234E5B373AD62D9EF1EBAE6FA6FFAD26144717F65AE9F98BD4280978ED52B3621F60FA4A8F6E5B5DAF64469733827E6\n"
"0B4D1755924541041F75FF399E3B5F535BC85D5A982AEEC5FC2404F06AC7F062C090C4545D1602C3D8B559801EE7B9A2";

const char *m0Str =
"198D1123A9817C40A914A3FF9E29BB16DD2F0DF98D0AB5C3A6014D60E31AE973051C35ADCEA0A41F32BB16E6688DC73F\n"
"10339DB2F26D1B867FD3E60A24F938210EABEFC51536845A490F28A088A4AC53575DBBAA218D70D34E28EBDE14DB3465\n";
const char *m1Str =
"066852248D915F6378B3F4A8E6173AC748FBFAE236AAEEAECC3F34D2D22706A06B925A83DD8276B2B240F61D761587B0\n"
"17CC8195E6607FF19A26AA69CA50C32487E35D2D75301AC4B6988F1B77523BF472927EE5710DF49A563534D86C684BE0\n";
const char *m4Str =
"10B67D1A0CE430B7AD74F64DD6C2E44C4788EADF8340909843C96B918BF54703CC14686B26E350EB1140ACC3337EEEB4\n"
"0F5D52E6F0B10A081EFF885CC858109241B379985ADD8982E6B8A202FD283897EFBA4CBE444C29751410A61FC8346545";

const char *q1Str =
"4 17B5E51EC931E724ABACE9C7F8AFDD51F3929478B47C222F99844D166936063D3BFCDF7AD7079EEF4BE8514E3D09EF0F\n"
"0F5794F38BAEC0FA3C30AC4C0B8E9024B2047F2F4576434F91768F2B51AD58C48E88414B1D4B7A9119A947D3CFEDEF0A\n"
"1320714A8B7E23C4C558D2B1C556CC8FB6B41F3669EFD70B6D204C2A7C6EF2E0CBCA945AA7BACB402E00ED338F7D12FC\n"
"0C2846E386161F123344704528D9944677806C3F784E3997857C91D2F3F37AB6AD92360CD97CABD5D631E9FC74708AD3\n"
"17F92FF3D71B473B802F2DE90C19A5F5DBFAA397293871AB58E5B813F7D686EA8E1814C69C50C02D062F3A13C1D045E1\n"
"05214392858DE04B3B468B2D0C703A485508C29157D81E9F799BAB2FEF0F514C99D5F8085D8062281418C6CCE5621D18\n";

	const int mode = mcl::IoEcProj | 16;
	Fp6 l, m;
	G2 Q, Q1;
	G1 P;

	Q.setStr(qStr, mode);
	P.setStr(pStr, mode);
	l.a.setStr(l0Str, mode);
	l.b.setStr(l4Str, mode);
	l.c.setStr(l1Str, mode);
	BLS12::dblLine(l, Q, P);
	m.a.setStr(m0Str, mode);
	m.b.setStr(m4Str, mode);
	m.c.setStr(m1Str, mode);
	Q1.setStr(q1Str, mode);
	CYBOZU_TEST_EQUAL(l, m);
	CYBOZU_TEST_EQUAL(Q, Q1);
}

CYBOZU_TEST_AUTO(mul_012)
{
	const char *fStr =
"087590AFBFEB8F85DD912EC297C2B5DD7BC0A9B0914348C5D99F0089C09CDBA0DCDAF1C704A7B092D8FB9A75B7C06D88\n"
"119DD8B08C40D4EB3D7AF19221E41639A98A10EF1A22F9AD8CB1350526B9335F47E76B2FFD6652E693A67440574D5A0C\n"
"134ADA7C4ABFBA4324900D25E5077A119F9E55A7F337C03FD539D8DAC392B458F11261BEA007393D43657E9656B984D6\n"
"01032DDB3CAEC38B7DA916CA111C46A013F1DC83AF13DFF5B71CC3789974F946CFC43FE7B8EE519E524627248369FCE7\n"
"19E9455C14A9640139224BB1337E4EC5EE92BFF757DB179CC98CF0F09682E44ED4B6004F31D4788DE28BB2D8F41DDAE4\n"
"0B9877DF6AC1015375AB421363A5B06D2DC1763B923FF674A06AE101306A4A39967A3F9EF12E870C124A26CE68E2D003\n"
"02AA5AC5901C9C91CD0B43CA62F21FA541896802A8AAF0FD5EDF8DAF4A98CEC19F457A67369E795594543677B4A16EA4\n"
"0604DB7CE2A0ABD8ADB5F4F06F20B01510BF9787C912B1036F570E7368D341D9B794F078DFD3265306841180865500D0\n"
"08145045CF5751502778739EFE6FEA6036C8F14800F4818C2FD8BA5AF98E89B0BBE6510D511C4E5A83A2813A92B655F0\n"
"0FDE93D3326321ECF6171FBC4665F1C171F19A6F1D521BFA1A1B80E0B08CEBB78B255AF0B5F7E45AA6C1D01005200FB1\n"
"0F2A9EA2891A683AE15A79EDB0C6DF45FFAD4D22F3293AE59D3CE8F6E0E59A097673D05D81ACAD8C59817FFDD3E89CF1\n"
"0724BD07BDDCA23775C1DECD80CE7722F98C10E75A0CD9A1FA81921A04EEFAC55BE0740C5F01ED83FDFC66380339D417\n";

const char *l0Str =
"198D1123A9817C40A914A3FF9E29BB16DD2F0DF98D0AB5C3A6014D60E31AE973051C35ADCEA0A41F32BB16E6688DC73F\n"
"10339DB2F26D1B867FD3E60A24F938210EABEFC51536845A490F28A088A4AC53575DBBAA218D70D34E28EBDE14DB3465";
const char *l1Str =
"066852248D915F6378B3F4A8E6173AC748FBFAE236AAEEAECC3F34D2D22706A06B925A83DD8276B2B240F61D761587B0\n"
"17CC8195E6607FF19A26AA69CA50C32487E35D2D75301AC4B6988F1B77523BF472927EE5710DF49A563534D86C684BE0\n";
const char *l4Str =
"10B67D1A0CE430B7AD74F64DD6C2E44C4788EADF8340909843C96B918BF54703CC14686B26E350EB1140ACC3337EEEB4\n"
"0F5D52E6F0B10A081EFF885CC858109241B379985ADD8982E6B8A202FD283897EFBA4CBE444C29751410A61FC8346545\n";

const char *f2Str =
"10128E1A9BD00FC81F6550921D0FED3944F63F980ABF91FDB73B1ED162337ED16075730ACD60A0FA7DFABAD9FC9657C5\n"
"055BE26091D8CDA32241F4991A1F184E403C3FFDD54858B23D5CE4B44402B65B26BCA6855DA7AC1C60F1D6651632DCD8\n"
"0D70827981F0D33185DE8767FDDFEC26CEB6A28F82C83BBABB0057E432FCF9072B666974123274751E35F371E931D6CC\n"
"02382B1A80E5BC95C75AE71BE2E097FD59365279CDD7EA358D87DEF132430744DABBF1B685D110CC731A9FDA40EEFC1B\n"
"0AAB560FB99D57A9B1B6C753DAF6B0619ED598C9B5FB0908F2DAE83C530E6365DBEDE29B9357D63803F46247A1F41C73\n"
"13C048F553BFC3C56516786DD26FF9D59ECFB9BE6B165F90E77CCED623BC66C6E93EFBF14576DB7E33C8C4F4E21F64DC\n"
"0987D7DEBB96A10D977F256432871BEBB4B3A620E4AE822E089E9DAA192CD278E9FA0CF598444F6758628BC38C33A5AD\n"
"0A4F1B75845B6C976BF49C35134AE73CA7A3C16D2E0BDA39C70367E3829E94EB7CAFBB0F8B57F4734B696D9CEF84FE73\n"
"0DFAB9C035F3DA51226F27998A494A32245800F0313446D6437D2F5B3F34A9E91428818B0C9AF63EB3AA618E80055FD5\n"
"06A58B9640FF8931616F6D08BA16ECE71F341C61F22E5EC5B556DF217179C3ECEC20E4BE425A3471F1D6648D14F89FBF\n"
"1614391845CDC212937BC1070010603FB4DF99A6B3FA7E7CD3316C56BA8B633B3DC7D864B36DA2F9A1E6B977DB150100\n"
"144A44415BCCB077EAA64C8DAC50631AF432C1420EBD8538818D65D6176BC1EB699579CED8340493306AF842B4B6822E";

	Fp6 l;
	Fp12 f, f2;
	l.a.setStr(l0Str, 16);
	l.b.setStr(l4Str, 16);
	l.c.setStr(l1Str, 16);
	f.setStr(fStr, 16);
	f2.setStr(f2Str, 16);
	BLS12::mul_024(f, l);
	CYBOZU_TEST_EQUAL(f, f2);
}

CYBOZU_TEST_AUTO(pairing)
{
	const int mode = mcl::IoEcProj | 16;

const char *pStr =
"4 0FD3977C60EC322BC281C915955ED534B491E39C72E8E800271CEF3F0492D890829FA69C45FCE93D9847A0CAB325D871\n"
"17CC2C36C5D283C05BFCECCF48DBB2050332DA058DD67326A9EE520967DBCAEDFCB5F05A085D1A49DF08BB968CC782C5\n"
"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001";
const char *qStr =
"4 0B5C339C23F8EAB3647E974BCDDF72C96F97A444346BE72CA73AB1323B83B8F6161257AB34C7E0CF34F6C45086CA5868\n"
"13C2235E9F9DFB33344BA2EE5A71435859022880732EDC9EC75AC79AE9DA972593CDC40A0AC334D6D2E8D7FAD1D98D0B\n"
"134B8EED8196A00D3B70ADBC26FF963B725A351CF0B73FE1A541788AFB0BB081AF82A438021B5E878B15D53B1D27C6A7\n"
"18CC69F847BEE826B939DCB4030D33020D03B046465C9EE103AA8009A175DB169070294E75771586687FE361DB884BCD\n"
"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001\n"
"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
const char *eStr =
"0E05D19E90D2C501E5502C7AC80D77201C47DF147DD1076440F0DF0179DF9802CA0775E0E73DD9174F1094D2280787B3\n"
"14D2F5C84279E7177A3543FBEAE261DE8F6C97EFD5F3FF3F959EC9FC0303F620A4B3AF00DF409496CECADDD0A7F0A164\n"
"1414E9B9DF8DF1EAC2E70D5538018377788C62016A54F28B037A68740705089AE431B86756F98CBE19690A5EAC0C2466\n"
"12D8B32157836A131CCA3CA313DAAAF909BC3AD6BDD15885BB429922B9CD7D1246D1163E5E6F88D68BF1B75E451EFABB\n"
"102C9A839A924E0D603D13F2E08A919E0B9EE2A269FC75727BA13D66027C157B9BB4077977FA94557DE4427BF11B234B\n"
"19DBEB7F2E3096AFFD44837655BD8249741B484B0EB0DBEE569DEA8D9E38AE09D210C8BC16AA6DFBC923095B2C9A8B2B\n"
"19B9A6DCCD01FA0D04D5CE94D8BDCE1DF64AFEB7FD493B955180A5C6B236E469F0E07CC9BB4203FCAC46AE6F8E5419D6\n"
"02BFA87AF7A3726A7ABACDCFDD53694AF651554F3A431AB4274F67D5DAD2D6C88AF794705FF456A936C83594731AD8DC\n"
"0F21E0173E3B50DD98EFA815B410631A57399B451FD6E1056FFD09C9FE50EFAD3D026F0C46C8BB1583A50B7853D990DA\n"
"02230237AE04B61F9269F6E7CD2FCF1231CEE4690AA658B0018EFC0D0770FD0A56B3B7294086E8D306B1465CDDD858CD\n"
"087EB8F6547015661E9CD48D6525C808636FCB8420B867CB2A87E006B2A93BBD5EF675E6CDDA9E6F94519C49EA8BB689\n"
"19F5C988B2DD6E33D7D3D34EFB1991F80DC28006AC75E0AB53FD98FC6F2476D05DD4ECA582F5FF72B8DDD9DDDE80FFC9";
	G1 P;
	G2 Q;
	P.setStr(pStr, mode);
	Q.setStr(qStr, mode);
	Fp12 e1, e2;
	e1.setStr(eStr, 16);
	BLS12::pairing(e2, P, Q);
	CYBOZU_TEST_EQUAL(e1, e2);
}

int main(int argc, char *argv[])
	try
{
	cybozu::Option opt;
	std::string mode;
	opt.appendOpt(&mode, "auto", "m", ": mode(gmp/gmp_mont/llvm/llvm_mont/xbyak)");
	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 1;
	}
	g_mode = mcl::fp::StrToMode(mode);
	printf("JIT %d\n", mcl::fp::isEnableJIT());
	return cybozu::test::autoRun.run(argc, argv);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
