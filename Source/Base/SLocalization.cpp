//----------------------------------------------------------------------------------------------------------------------
//	SLocalization.cpp			©2024 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SLocalization.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local procs

//----------------------------------------------------------------------------------------------------------------------
static CString sGetDisplayNameForLocalizationLanguage(SLocalization::Language* localizationLanguage, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	return localizationLanguage->getDisplayName();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SLocalization::Currency

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SLocalization::Currency>& SLocalization::Currency::getAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	Currency			sCurrencyAED(CString(OSSTR("AED")), false);
	static	Currency			sCurrencyAFN(CString(OSSTR("AFN")), false);
	static	Currency			sCurrencyALL(CString(OSSTR("ALL")), false);
	static	Currency			sCurrencyAMD(CString(OSSTR("AMD")), false);
	static	Currency			sCurrencyANG(CString(OSSTR("ANG")), false);
	static	Currency			sCurrencyAOA(CString(OSSTR("AOA")), false);
	static	Currency			sCurrencyARS(CString(OSSTR("ARS")), false);
	static	Currency			sCurrencyAUD(CString(OSSTR("AUD")), true);
	static	Currency			sCurrencyAWG(CString(OSSTR("AWG")), false);
	static	Currency			sCurrencyAZN(CString(OSSTR("AZN")), false);
	static	Currency			sCurrencyBAM(CString(OSSTR("BAM")), false);
	static	Currency			sCurrencyBBD(CString(OSSTR("BBD")), false);
	static	Currency			sCurrencyBDT(CString(OSSTR("BDT")), false);
	static	Currency			sCurrencyBGN(CString(OSSTR("BGN")), false);
	static	Currency			sCurrencyBHD(CString(OSSTR("BHD")), false);
	static	Currency			sCurrencyBIF(CString(OSSTR("BIF")), false);
	static	Currency			sCurrencyBMD(CString(OSSTR("BMD")), false);
	static	Currency			sCurrencyBND(CString(OSSTR("BND")), false);
	static	Currency			sCurrencyBOB(CString(OSSTR("BOB")), false);
	static	Currency			sCurrencyBOV(CString(OSSTR("BOV")), false);
	static	Currency			sCurrencyBRL(CString(OSSTR("BRL")), false);
	static	Currency			sCurrencyBSD(CString(OSSTR("BSD")), false);
	static	Currency			sCurrencyBTN(CString(OSSTR("BTN")), false);
	static	Currency			sCurrencyBWP(CString(OSSTR("BWP")), false);
	static	Currency			sCurrencyBYN(CString(OSSTR("BYN")), false);
	static	Currency			sCurrencyBZD(CString(OSSTR("BZD")), false);
	static	Currency			sCurrencyCAD(CString(OSSTR("CAD")), true);
	static	Currency			sCurrencyCDF(CString(OSSTR("CDF")), false);
	static	Currency			sCurrencyCHE(CString(OSSTR("CHE")), false);
	static	Currency			sCurrencyCHF(CString(OSSTR("CHF")), false);
	static	Currency			sCurrencyCHW(CString(OSSTR("CHW")), false);
	static	Currency			sCurrencyCLF(CString(OSSTR("CLF")), false);
	static	Currency			sCurrencyCLP(CString(OSSTR("CLP")), false);
	static	Currency			sCurrencyCNY(CString(OSSTR("CNY")), true);
	static	Currency			sCurrencyCOP(CString(OSSTR("COP")), false);
	static	Currency			sCurrencyCOU(CString(OSSTR("COU")), false);
	static	Currency			sCurrencyCRC(CString(OSSTR("CRC")), false);
	static	Currency			sCurrencyCUP(CString(OSSTR("CUP")), false);
	static	Currency			sCurrencyCVE(CString(OSSTR("CVE")), false);
	static	Currency			sCurrencyCZK(CString(OSSTR("CZK")), false);
	static	Currency			sCurrencyDJF(CString(OSSTR("DJF")), false);
	static	Currency			sCurrencyDKK(CString(OSSTR("DKK")), false);
	static	Currency			sCurrencyDOP(CString(OSSTR("DOP")), false);
	static	Currency			sCurrencyDZD(CString(OSSTR("DZD")), false);
	static	Currency			sCurrencyEGP(CString(OSSTR("EGP")), false);
	static	Currency			sCurrencyERN(CString(OSSTR("ERN")), false);
	static	Currency			sCurrencyETB(CString(OSSTR("ETB")), false);
	static	Currency			sCurrencyEUR(CString(OSSTR("EUR")), true);
	static	Currency			sCurrencyFJD(CString(OSSTR("FJD")), false);
	static	Currency			sCurrencyFKP(CString(OSSTR("FKP")), false);
	static	Currency			sCurrencyGBP(CString(OSSTR("GBP")), false);
	static	Currency			sCurrencyGEL(CString(OSSTR("GEL")), false);
	static	Currency			sCurrencyGHS(CString(OSSTR("GHS")), false);
	static	Currency			sCurrencyGIP(CString(OSSTR("GIP")), false);
	static	Currency			sCurrencyGMD(CString(OSSTR("GMD")), false);
	static	Currency			sCurrencyGNF(CString(OSSTR("GNF")), false);
	static	Currency			sCurrencyGTQ(CString(OSSTR("GTQ")), false);
	static	Currency			sCurrencyGYD(CString(OSSTR("GYD")), false);
	static	Currency			sCurrencyHKD(CString(OSSTR("HKD")), true);
	static	Currency			sCurrencyHNL(CString(OSSTR("HNL")), false);
	static	Currency			sCurrencyHTG(CString(OSSTR("HTG")), false);
	static	Currency			sCurrencyHUF(CString(OSSTR("HUF")), false);
	static	Currency			sCurrencyIDR(CString(OSSTR("IDR")), false);
	static	Currency			sCurrencyILS(CString(OSSTR("ILS")), false);
	static	Currency			sCurrencyINR(CString(OSSTR("INR")), true);
	static	Currency			sCurrencyIQD(CString(OSSTR("IQD")), false);
	static	Currency			sCurrencyIRR(CString(OSSTR("IRR")), false);
	static	Currency			sCurrencyISK(CString(OSSTR("ISK")), false);
	static	Currency			sCurrencyJMD(CString(OSSTR("JMD")), false);
	static	Currency			sCurrencyJOD(CString(OSSTR("JOD")), false);
	static	Currency			sCurrencyJPY(CString(OSSTR("JPY")), true);
	static	Currency			sCurrencyKES(CString(OSSTR("KES")), false);
	static	Currency			sCurrencyKGS(CString(OSSTR("KGS")), false);
	static	Currency			sCurrencyKHR(CString(OSSTR("KHR")), false);
	static	Currency			sCurrencyKMF(CString(OSSTR("KMF")), false);
	static	Currency			sCurrencyKPW(CString(OSSTR("KPW")), false);
	static	Currency			sCurrencyKRW(CString(OSSTR("KRW")), true);
	static	Currency			sCurrencyKWD(CString(OSSTR("KWD")), false);
	static	Currency			sCurrencyKYD(CString(OSSTR("KYD")), false);
	static	Currency			sCurrencyKZT(CString(OSSTR("KZT")), false);
	static	Currency			sCurrencyLAK(CString(OSSTR("LAK")), false);
	static	Currency			sCurrencyLBP(CString(OSSTR("LBP")), false);
	static	Currency			sCurrencyLKR(CString(OSSTR("LKR")), false);
	static	Currency			sCurrencyLRD(CString(OSSTR("LRD")), false);
	static	Currency			sCurrencyLSL(CString(OSSTR("LSL")), false);
	static	Currency			sCurrencyLYD(CString(OSSTR("LYD")), false);
	static	Currency			sCurrencyMAD(CString(OSSTR("MAD")), false);
	static	Currency			sCurrencyMDL(CString(OSSTR("MDL")), false);
	static	Currency			sCurrencyMGA(CString(OSSTR("MGA")), false);
	static	Currency			sCurrencyMKD(CString(OSSTR("MKD")), false);
	static	Currency			sCurrencyMMK(CString(OSSTR("MMK")), false);
	static	Currency			sCurrencyMNT(CString(OSSTR("MNT")), false);
	static	Currency			sCurrencyMOP(CString(OSSTR("MOP")), false);
	static	Currency			sCurrencyMRU(CString(OSSTR("MRU")), false);
	static	Currency			sCurrencyMUR(CString(OSSTR("MUR")), false);
	static	Currency			sCurrencyMVR(CString(OSSTR("MVR")), false);
	static	Currency			sCurrencyMWK(CString(OSSTR("MWK")), false);
	static	Currency			sCurrencyMXN(CString(OSSTR("MXN")), false);
	static	Currency			sCurrencyMXV(CString(OSSTR("MXV")), false);
	static	Currency			sCurrencyMYR(CString(OSSTR("MYR")), false);
	static	Currency			sCurrencyMZN(CString(OSSTR("MZN")), false);
	static	Currency			sCurrencyNAD(CString(OSSTR("NAD")), false);
	static	Currency			sCurrencyNGN(CString(OSSTR("NGN")), false);
	static	Currency			sCurrencyNIO(CString(OSSTR("NIO")), false);
	static	Currency			sCurrencyNOK(CString(OSSTR("NOK")), false);
	static	Currency			sCurrencyNPR(CString(OSSTR("NPR")), false);
	static	Currency			sCurrencyNZD(CString(OSSTR("NZD")), false);
	static	Currency			sCurrencyOMR(CString(OSSTR("OMR")), false);
	static	Currency			sCurrencyPAB(CString(OSSTR("PAB")), false);
	static	Currency			sCurrencyPEN(CString(OSSTR("PEN")), false);
	static	Currency			sCurrencyPGK(CString(OSSTR("PGK")), false);
	static	Currency			sCurrencyPHP(CString(OSSTR("PHP")), false);
	static	Currency			sCurrencyPKR(CString(OSSTR("PKR")), false);
	static	Currency			sCurrencyPLN(CString(OSSTR("PLN")), false);
	static	Currency			sCurrencyPYG(CString(OSSTR("PYG")), false);
	static	Currency			sCurrencyQAR(CString(OSSTR("QAR")), false);
	static	Currency			sCurrencyRON(CString(OSSTR("RON")), false);
	static	Currency			sCurrencyRSD(CString(OSSTR("RSD")), false);
	static	Currency			sCurrencyRUB(CString(OSSTR("RUB")), false);
	static	Currency			sCurrencyRWF(CString(OSSTR("RWF")), false);
	static	Currency			sCurrencySAR(CString(OSSTR("SAR")), false);
	static	Currency			sCurrencySBD(CString(OSSTR("SBD")), false);
	static	Currency			sCurrencySCR(CString(OSSTR("SCR")), false);
	static	Currency			sCurrencySDG(CString(OSSTR("SDG")), false);
	static	Currency			sCurrencySEK(CString(OSSTR("SEK")), false);
	static	Currency			sCurrencySGD(CString(OSSTR("SGD")), false);
	static	Currency			sCurrencySHP(CString(OSSTR("SHP")), false);
	static	Currency			sCurrencySLE(CString(OSSTR("SLE")), false);
	static	Currency			sCurrencySOS(CString(OSSTR("SOS")), false);
	static	Currency			sCurrencySRD(CString(OSSTR("SRD")), false);
	static	Currency			sCurrencySSP(CString(OSSTR("SSP")), false);
	static	Currency			sCurrencySTN(CString(OSSTR("STN")), false);
	static	Currency			sCurrencySVC(CString(OSSTR("SVC")), false);
	static	Currency			sCurrencySYP(CString(OSSTR("SYP")), false);
	static	Currency			sCurrencySZL(CString(OSSTR("SZL")), false);
	static	Currency			sCurrencyTHB(CString(OSSTR("THB")), false);
	static	Currency			sCurrencyTJS(CString(OSSTR("TJS")), false);
	static	Currency			sCurrencyTMT(CString(OSSTR("TMT")), false);
	static	Currency			sCurrencyTND(CString(OSSTR("TND")), false);
	static	Currency			sCurrencyTOP(CString(OSSTR("TOP")), false);
	static	Currency			sCurrencyTRY(CString(OSSTR("TRY")), false);
	static	Currency			sCurrencyTTD(CString(OSSTR("TTD")), false);
	static	Currency			sCurrencyTWD(CString(OSSTR("TWD")), true);
	static	Currency			sCurrencyTZS(CString(OSSTR("TZS")), false);
	static	Currency			sCurrencyUAH(CString(OSSTR("UAH")), false);
	static	Currency			sCurrencyUGX(CString(OSSTR("UGX")), false);
	static	Currency			sCurrencyUSD(CString(OSSTR("USD")), true);
	static	Currency			sCurrencyUYI(CString(OSSTR("UYI")), false);
	static	Currency			sCurrencyUYU(CString(OSSTR("UYU")), false);
	static	Currency			sCurrencyUYW(CString(OSSTR("UYW")), false);
	static	Currency			sCurrencyUZS(CString(OSSTR("UZS")), false);
	static	Currency			sCurrencyVED(CString(OSSTR("VED")), false);
	static	Currency			sCurrencyVES(CString(OSSTR("VES")), false);
	static	Currency			sCurrencyVND(CString(OSSTR("VND")), false);
	static	Currency			sCurrencyVUV(CString(OSSTR("VUV")), false);
	static	Currency			sCurrencyWST(CString(OSSTR("WST")), false);
	static	Currency			sCurrencyXAF(CString(OSSTR("XAF")), false);
	static	Currency			sCurrencyXAG(CString(OSSTR("XAG")), false);
	static	Currency			sCurrencyXAU(CString(OSSTR("XAU")), false);
	static	Currency			sCurrencyXBA(CString(OSSTR("XBA")), false);
	static	Currency			sCurrencyXBB(CString(OSSTR("XBB")), false);
	static	Currency			sCurrencyXBC(CString(OSSTR("XBC")), false);
	static	Currency			sCurrencyXBD(CString(OSSTR("XBD")), false);
	static	Currency			sCurrencyXCD(CString(OSSTR("XCD")), false);
	static	Currency			sCurrencyXDR(CString(OSSTR("XDR")), false);
	static	Currency			sCurrencyXOF(CString(OSSTR("XOF")), false);
	static	Currency			sCurrencyXPD(CString(OSSTR("XPD")), false);
	static	Currency			sCurrencyXPF(CString(OSSTR("XPF")), false);
	static	Currency			sCurrencyXPT(CString(OSSTR("XPT")), false);
	static	Currency			sCurrencyXSU(CString(OSSTR("XSU")), false);
	static	Currency			sCurrencyXTS(CString(OSSTR("XTS")), false);
	static	Currency			sCurrencyXUA(CString(OSSTR("XUA")), false);
	static	Currency			sCurrencyYER(CString(OSSTR("YER")), false);
	static	Currency			sCurrencyZAR(CString(OSSTR("ZAR")), false);
	static	Currency			sCurrencyZMW(CString(OSSTR("ZMW")), false);
	static	Currency			sCurrencyZWG(CString(OSSTR("ZWG")), false);
	static	Currency			sAll[] =
									{
										// Common
										sCurrencyAUD,
										sCurrencyCAD,
										sCurrencyCNY,
										sCurrencyEUR,
										sCurrencyHKD,
										sCurrencyINR,
										sCurrencyJPY,
										sCurrencyKRW,
										sCurrencyTWD,
										sCurrencyUSD,

										// Not common
										sCurrencyAED,
										sCurrencyAFN,
										sCurrencyALL,
										sCurrencyAMD,
										sCurrencyANG,
										sCurrencyAOA,
										sCurrencyARS,
										sCurrencyAWG,
										sCurrencyAZN,
										sCurrencyBAM,
										sCurrencyBBD,
										sCurrencyBDT,
										sCurrencyBGN,
										sCurrencyBHD,
										sCurrencyBIF,
										sCurrencyBMD,
										sCurrencyBND,
										sCurrencyBOB,
										sCurrencyBOV,
										sCurrencyBRL,
										sCurrencyBSD,
										sCurrencyBTN,
										sCurrencyBWP,
										sCurrencyBYN,
										sCurrencyBZD,
										sCurrencyCDF,
										sCurrencyCHE,
										sCurrencyCHF,
										sCurrencyCHW,
										sCurrencyCLF,
										sCurrencyCLP,
										sCurrencyCOP,
										sCurrencyCOU,
										sCurrencyCRC,
										sCurrencyCUP,
										sCurrencyCVE,
										sCurrencyCZK,
										sCurrencyDJF,
										sCurrencyDKK,
										sCurrencyDOP,
										sCurrencyDZD,
										sCurrencyEGP,
										sCurrencyERN,
										sCurrencyETB,
										sCurrencyFJD,
										sCurrencyFKP,
										sCurrencyGBP,
										sCurrencyGEL,
										sCurrencyGHS,
										sCurrencyGIP,
										sCurrencyGMD,
										sCurrencyGNF,
										sCurrencyGTQ,
										sCurrencyGYD,
										sCurrencyHNL,
										sCurrencyHTG,
										sCurrencyHUF,
										sCurrencyIDR,
										sCurrencyILS,
										sCurrencyIQD,
										sCurrencyIRR,
										sCurrencyISK,
										sCurrencyJMD,
										sCurrencyJOD,
										sCurrencyKES,
										sCurrencyKGS,
										sCurrencyKHR,
										sCurrencyKMF,
										sCurrencyKPW,
										sCurrencyKWD,
										sCurrencyKYD,
										sCurrencyKZT,
										sCurrencyLAK,
										sCurrencyLBP,
										sCurrencyLKR,
										sCurrencyLRD,
										sCurrencyLSL,
										sCurrencyLYD,
										sCurrencyMAD,
										sCurrencyMDL,
										sCurrencyMGA,
										sCurrencyMKD,
										sCurrencyMMK,
										sCurrencyMNT,
										sCurrencyMOP,
										sCurrencyMRU,
										sCurrencyMUR,
										sCurrencyMVR,
										sCurrencyMWK,
										sCurrencyMXN,
										sCurrencyMXV,
										sCurrencyMYR,
										sCurrencyMZN,
										sCurrencyNAD,
										sCurrencyNGN,
										sCurrencyNIO,
										sCurrencyNOK,
										sCurrencyNPR,
										sCurrencyNZD,
										sCurrencyOMR,
										sCurrencyPAB,
										sCurrencyPEN,
										sCurrencyPGK,
										sCurrencyPHP,
										sCurrencyPKR,
										sCurrencyPLN,
										sCurrencyPYG,
										sCurrencyQAR,
										sCurrencyRON,
										sCurrencyRSD,
										sCurrencyRUB,
										sCurrencyRWF,
										sCurrencySAR,
										sCurrencySBD,
										sCurrencySCR,
										sCurrencySDG,
										sCurrencySEK,
										sCurrencySGD,
										sCurrencySHP,
										sCurrencySLE,
										sCurrencySOS,
										sCurrencySRD,
										sCurrencySSP,
										sCurrencySTN,
										sCurrencySVC,
										sCurrencySYP,
										sCurrencySZL,
										sCurrencyTHB,
										sCurrencyTJS,
										sCurrencyTMT,
										sCurrencyTND,
										sCurrencyTOP,
										sCurrencyTRY,
										sCurrencyTTD,
										sCurrencyTZS,
										sCurrencyUAH,
										sCurrencyUGX,
										sCurrencyUYI,
										sCurrencyUYU,
										sCurrencyUYW,
										sCurrencyUZS,
										sCurrencyVED,
										sCurrencyVES,
										sCurrencyVND,
										sCurrencyVUV,
										sCurrencyWST,
										sCurrencyXAF,
										sCurrencyXAG,
										sCurrencyXAU,
										sCurrencyXBA,
										sCurrencyXBB,
										sCurrencyXBC,
										sCurrencyXBD,
										sCurrencyXCD,
										sCurrencyXDR,
										sCurrencyXOF,
										sCurrencyXPD,
										sCurrencyXPF,
										sCurrencyXPT,
										sCurrencyXSU,
										sCurrencyXTS,
										sCurrencyXUA,
										sCurrencyYER,
										sCurrencyZAR,
										sCurrencyZMW,
										sCurrencyZWG,
									};
	static	TArray<Currency>	sAllArray = TSARRAY_FROM_C_ARRAY(Currency, sAll);

	return sAllArray;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SLocalization::Currency> SLocalization::Currency::getFor(const CString& iso4217Code)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all
	for (TIteratorD<Currency> iterator = getAll().getIterator(); iterator.hasValue(); iterator.advance()) {
		// Check
		if (iterator->getISO4217Code() == iso4217Code)
			// Found
			return OV<Currency>(*iterator);
	}

	return OV<Currency>();
}

//----------------------------------------------------------------------------------------------------------------------
const SLocalization::Currency& SLocalization::Currency::getDefault()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	Currency	sCurrencyUSD(CString(OSSTR("USD")), true);

	return sCurrencyUSD;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SLocalization::Language

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SLocalization::Language>& SLocalization::Language::getAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	Language			sLanguageABK(MAKE_OSTYPE('a', 'b', 'k', 0), false);
	static	Language			sLanguageACE(MAKE_OSTYPE('a', 'c', 'e', 0), false);
	static	Language			sLanguageACH(MAKE_OSTYPE('a', 'c', 'h', 0), false);
	static	Language			sLanguageADA(MAKE_OSTYPE('a', 'd', 'a', 0), false);
	static	Language			sLanguageADY(MAKE_OSTYPE('a', 'd', 'y', 0), false);
	static	Language			sLanguageAAR(MAKE_OSTYPE('a', 'a', 'r', 0), false);
	static	Language			sLanguageAFH(MAKE_OSTYPE('a', 'f', 'h', 0), false);
	static	Language			sLanguageAFR(MAKE_OSTYPE('a', 'f', 'r', 0), false);
	static	Language			sLanguageAIN(MAKE_OSTYPE('a', 'i', 'n', 0), false);
	static	Language			sLanguageAKA(MAKE_OSTYPE('a', 'k', 'a', 0), false);
	static	Language			sLanguageAKK(MAKE_OSTYPE('a', 'k', 'k', 0), false);
	static	Language			sLanguageALB(MAKE_OSTYPE('a', 'l', 'b', 0), false);
	static	Language			sLanguageALE(MAKE_OSTYPE('a', 'l', 'e', 0), false);
	static	Language			sLanguageAMH(MAKE_OSTYPE('a', 'm', 'h', 0), false);
	static	Language			sLanguageGRC(MAKE_OSTYPE('g', 'r', 'c', 0), false);
	static	Language			sLanguageANP(MAKE_OSTYPE('a', 'n', 'p', 0), false);
	static	Language			sLanguageARA(MAKE_OSTYPE('a', 'r', 'a', 0), false);
	static	Language			sLanguageARG(MAKE_OSTYPE('a', 'r', 'g', 0), false);
	static	Language			sLanguageARP(MAKE_OSTYPE('a', 'r', 'p', 0), false);
	static	Language			sLanguageARW(MAKE_OSTYPE('a', 'r', 'w', 0), false);
	static	Language			sLanguageARM(MAKE_OSTYPE('a', 'r', 'm', 0), false);
	static	Language			sLanguageASM(MAKE_OSTYPE('a', 's', 'm', 0), false);
	static	Language			sLanguageAST(MAKE_OSTYPE('a', 's', 't', 0), false);
	static	Language			sLanguageAVA(MAKE_OSTYPE('a', 'v', 'a', 0), false);
	static	Language			sLanguageAVE(MAKE_OSTYPE('a', 'v', 'e', 0), false);
	static	Language			sLanguageAWA(MAKE_OSTYPE('a', 'w', 'a', 0), false);
	static	Language			sLanguageAYM(MAKE_OSTYPE('a', 'y', 'm', 0), false);
	static	Language			sLanguageAZE(MAKE_OSTYPE('a', 'z', 'e', 0), false);
	static	Language			sLanguageBAN(MAKE_OSTYPE('b', 'a', 'n', 0), false);
	static	Language			sLanguageBAL(MAKE_OSTYPE('b', 'a', 'l', 0), false);
	static	Language			sLanguageBAM(MAKE_OSTYPE('b', 'a', 'm', 0), false);
	static	Language			sLanguageBAS(MAKE_OSTYPE('b', 'a', 's', 0), false);
	static	Language			sLanguageBAK(MAKE_OSTYPE('b', 'a', 'k', 0), false);
	static	Language			sLanguageBAQ(MAKE_OSTYPE('b', 'a', 'q', 0), false);
	static	Language			sLanguageBEJ(MAKE_OSTYPE('b', 'e', 'j', 0), false);
	static	Language			sLanguageBEL(MAKE_OSTYPE('b', 'e', 'l', 0), false);
	static	Language			sLanguageBEM(MAKE_OSTYPE('b', 'e', 'm', 0), false);
	static	Language			sLanguageBEN(MAKE_OSTYPE('b', 'e', 'n', 0), false);
	static	Language			sLanguageBHO(MAKE_OSTYPE('b', 'h', 'o', 0), false);
	static	Language			sLanguageBIK(MAKE_OSTYPE('b', 'i', 'k', 0), false);
	static	Language			sLanguageBYN(MAKE_OSTYPE('b', 'y', 'n', 0), false);
	static	Language			sLanguageBIN(MAKE_OSTYPE('b', 'i', 'n', 0), false);
	static	Language			sLanguageBIS(MAKE_OSTYPE('b', 'i', 's', 0), false);
	static	Language			sLanguageZBL(MAKE_OSTYPE('z', 'b', 'l', 0), false);
	static	Language			sLanguageBOS(MAKE_OSTYPE('b', 'o', 's', 0), false);
	static	Language			sLanguageBRA(MAKE_OSTYPE('b', 'r', 'a', 0), false);
	static	Language			sLanguageBRE(MAKE_OSTYPE('b', 'r', 'e', 0), false);
	static	Language			sLanguageBUG(MAKE_OSTYPE('b', 'u', 'g', 0), false);
	static	Language			sLanguageBUL(MAKE_OSTYPE('b', 'u', 'l', 0), false);
	static	Language			sLanguageBUA(MAKE_OSTYPE('b', 'u', 'a', 0), false);
	static	Language			sLanguageBUR(MAKE_OSTYPE('b', 'u', 'r', 0), false);
	static	Language			sLanguageCAD(MAKE_OSTYPE('c', 'a', 'd', 0), false);
	static	Language			sLanguageCAT(MAKE_OSTYPE('c', 'a', 't', 0), false);
	static	Language			sLanguageCEB(MAKE_OSTYPE('c', 'e', 'b', 0), false);
	static	Language			sLanguageCHG(MAKE_OSTYPE('c', 'h', 'g', 0), false);
	static	Language			sLanguageCHA(MAKE_OSTYPE('c', 'h', 'a', 0), false);
	static	Language			sLanguageCHE(MAKE_OSTYPE('c', 'h', 'e', 0), false);
	static	Language			sLanguageCHR(MAKE_OSTYPE('c', 'h', 'r', 0), false);
	static	Language			sLanguageCHY(MAKE_OSTYPE('c', 'h', 'y', 0), false);
	static	Language			sLanguageCHB(MAKE_OSTYPE('c', 'h', 'b', 0), false);
	static	Language			sLanguageCHI(MAKE_OSTYPE('c', 'h', 'i', 0), true);
	static	Language			sLanguageCHN(MAKE_OSTYPE('c', 'h', 'n', 0), false);
	static	Language			sLanguageCHP(MAKE_OSTYPE('c', 'h', 'p', 0), false);
	static	Language			sLanguageCHO(MAKE_OSTYPE('c', 'h', 'o', 0), false);
	static	Language			sLanguageCHU(MAKE_OSTYPE('c', 'h', 'u', 0), false);
	static	Language			sLanguageCHK(MAKE_OSTYPE('c', 'h', 'k', 0), false);
	static	Language			sLanguageCHV(MAKE_OSTYPE('c', 'h', 'v', 0), false);
	static	Language			sLanguageNWC(MAKE_OSTYPE('n', 'w', 'c', 0), false);
	static	Language			sLanguageSYC(MAKE_OSTYPE('s', 'y', 'c', 0), false);
	static	Language			sLanguageCOP(MAKE_OSTYPE('c', 'o', 'p', 0), false);
	static	Language			sLanguageCOR(MAKE_OSTYPE('c', 'o', 'r', 0), false);
	static	Language			sLanguageCOS(MAKE_OSTYPE('c', 'o', 's', 0), false);
	static	Language			sLanguageCRE(MAKE_OSTYPE('c', 'r', 'e', 0), false);
	static	Language			sLanguageMUS(MAKE_OSTYPE('m', 'u', 's', 0), false);
	static	Language			sLanguageCRH(MAKE_OSTYPE('c', 'r', 'h', 0), false);
	static	Language			sLanguageHRV(MAKE_OSTYPE('h', 'r', 'v', 0), false);
	static	Language			sLanguageCZE(MAKE_OSTYPE('c', 'z', 'e', 0), false);
	static	Language			sLanguageDAK(MAKE_OSTYPE('d', 'a', 'k', 0), false);
	static	Language			sLanguageDAN(MAKE_OSTYPE('d', 'a', 'n', 0), false);
	static	Language			sLanguageDAR(MAKE_OSTYPE('d', 'a', 'r', 0), false);
	static	Language			sLanguageDEL(MAKE_OSTYPE('d', 'e', 'l', 0), false);
	static	Language			sLanguageDIV(MAKE_OSTYPE('d', 'i', 'v', 0), false);
	static	Language			sLanguageDIN(MAKE_OSTYPE('d', 'i', 'n', 0), false);
	static	Language			sLanguageDOI(MAKE_OSTYPE('d', 'o', 'i', 0), false);
	static	Language			sLanguageDUA(MAKE_OSTYPE('d', 'u', 'a', 0), false);
	static	Language			sLanguageDUT(MAKE_OSTYPE('d', 'u', 't', 0), false);
	static	Language			sLanguageDYU(MAKE_OSTYPE('d', 'y', 'u', 0), false);
	static	Language			sLanguageDZO(MAKE_OSTYPE('d', 'z', 'o', 0), false);
	static	Language			sLanguageFRS(MAKE_OSTYPE('f', 'r', 's', 0), false);
	static	Language			sLanguageEFI(MAKE_OSTYPE('e', 'f', 'i', 0), false);
	static	Language			sLanguageEGY(MAKE_OSTYPE('e', 'g', 'y', 0), false);
	static	Language			sLanguageEKA(MAKE_OSTYPE('e', 'k', 'a', 0), false);
	static	Language			sLanguageELX(MAKE_OSTYPE('e', 'l', 'x', 0), false);
	static	Language			sLanguageENG(MAKE_OSTYPE('e', 'n', 'g', 0), true);
	static	Language			sLanguageMYV(MAKE_OSTYPE('m', 'y', 'v', 0), false);
	static	Language			sLanguageEPO(MAKE_OSTYPE('e', 'p', 'o', 0), false);
	static	Language			sLanguageEST(MAKE_OSTYPE('e', 's', 't', 0), false);
	static	Language			sLanguageEWE(MAKE_OSTYPE('e', 'w', 'e', 0), false);
	static	Language			sLanguageEWO(MAKE_OSTYPE('e', 'w', 'o', 0), false);
	static	Language			sLanguageFAN(MAKE_OSTYPE('f', 'a', 'n', 0), false);
	static	Language			sLanguageFAT(MAKE_OSTYPE('f', 'a', 't', 0), false);
	static	Language			sLanguageFAO(MAKE_OSTYPE('f', 'a', 'o', 0), false);
	static	Language			sLanguageFIJ(MAKE_OSTYPE('f', 'i', 'j', 0), false);
	static	Language			sLanguageFIL(MAKE_OSTYPE('f', 'i', 'l', 0), false);
	static	Language			sLanguageFIN(MAKE_OSTYPE('f', 'i', 'n', 0), false);
	static	Language			sLanguageFON(MAKE_OSTYPE('f', 'o', 'n', 0), false);
	static	Language			sLanguageFRE(MAKE_OSTYPE('f', 'r', 'e', 0), true);
	static	Language			sLanguageFUR(MAKE_OSTYPE('f', 'u', 'r', 0), false);
	static	Language			sLanguageFUL(MAKE_OSTYPE('f', 'u', 'l', 0), false);
	static	Language			sLanguageGAA(MAKE_OSTYPE('g', 'a', 'a', 0), false);
	static	Language			sLanguageCAR(MAKE_OSTYPE('c', 'a', 'r', 0), false);
	static	Language			sLanguageGLG(MAKE_OSTYPE('g', 'l', 'g', 0), false);
	static	Language			sLanguageLUG(MAKE_OSTYPE('l', 'u', 'g', 0), false);
	static	Language			sLanguageGAY(MAKE_OSTYPE('g', 'a', 'y', 0), false);
	static	Language			sLanguageGBA(MAKE_OSTYPE('g', 'b', 'a', 0), false);
	static	Language			sLanguageGEZ(MAKE_OSTYPE('g', 'e', 'z', 0), false);
	static	Language			sLanguageGEO(MAKE_OSTYPE('g', 'e', 'o', 0), false);
	static	Language			sLanguageGER(MAKE_OSTYPE('g', 'e', 'r', 0), true);
	static	Language			sLanguageGIL(MAKE_OSTYPE('g', 'i', 'l', 0), false);
	static	Language			sLanguageGON(MAKE_OSTYPE('g', 'o', 'n', 0), false);
	static	Language			sLanguageGOR(MAKE_OSTYPE('g', 'o', 'r', 0), false);
	static	Language			sLanguageGOT(MAKE_OSTYPE('g', 'o', 't', 0), false);
	static	Language			sLanguageGRB(MAKE_OSTYPE('g', 'r', 'b', 0), false);
	static	Language			sLanguageGRN(MAKE_OSTYPE('g', 'r', 'n', 0), false);
	static	Language			sLanguageGUJ(MAKE_OSTYPE('g', 'u', 'j', 0), false);
	static	Language			sLanguageGWI(MAKE_OSTYPE('g', 'w', 'i', 0), false);
	static	Language			sLanguageHAI(MAKE_OSTYPE('h', 'a', 'i', 0), false);
	static	Language			sLanguageHAT(MAKE_OSTYPE('h', 'a', 't', 0), false);
	static	Language			sLanguageHAU(MAKE_OSTYPE('h', 'a', 'u', 0), false);
	static	Language			sLanguageHAW(MAKE_OSTYPE('h', 'a', 'w', 0), false);
	static	Language			sLanguageHEB(MAKE_OSTYPE('h', 'e', 'b', 0), false);
	static	Language			sLanguageHER(MAKE_OSTYPE('h', 'e', 'r', 0), false);
	static	Language			sLanguageHIL(MAKE_OSTYPE('h', 'i', 'l', 0), false);
	static	Language			sLanguageHIN(MAKE_OSTYPE('h', 'i', 'n', 0), false);
	static	Language			sLanguageHMO(MAKE_OSTYPE('h', 'm', 'o', 0), false);
	static	Language			sLanguageHIT(MAKE_OSTYPE('h', 'i', 't', 0), false);
	static	Language			sLanguageHMN(MAKE_OSTYPE('h', 'm', 'n', 0), false);
	static	Language			sLanguageHUN(MAKE_OSTYPE('h', 'u', 'n', 0), false);
	static	Language			sLanguageHUP(MAKE_OSTYPE('h', 'u', 'p', 0), false);
	static	Language			sLanguageIBA(MAKE_OSTYPE('i', 'b', 'a', 0), false);
	static	Language			sLanguageICE(MAKE_OSTYPE('i', 'c', 'e', 0), false);
	static	Language			sLanguageIDO(MAKE_OSTYPE('i', 'd', 'o', 0), false);
	static	Language			sLanguageIBO(MAKE_OSTYPE('i', 'b', 'o', 0), false);
	static	Language			sLanguageILO(MAKE_OSTYPE('i', 'l', 'o', 0), false);
	static	Language			sLanguageSMN(MAKE_OSTYPE('s', 'm', 'n', 0), false);
	static	Language			sLanguageIND(MAKE_OSTYPE('i', 'n', 'd', 0), false);
	static	Language			sLanguageINH(MAKE_OSTYPE('i', 'n', 'h', 0), false);
	static	Language			sLanguageINA(MAKE_OSTYPE('i', 'n', 'a', 0), false);
	static	Language			sLanguageILE(MAKE_OSTYPE('i', 'l', 'e', 0), false);
	static	Language			sLanguageIKU(MAKE_OSTYPE('i', 'k', 'u', 0), false);
	static	Language			sLanguageIPK(MAKE_OSTYPE('i', 'p', 'k', 0), false);
	static	Language			sLanguageGLE(MAKE_OSTYPE('g', 'l', 'e', 0), false);
	static	Language			sLanguageITA(MAKE_OSTYPE('i', 't', 'a', 0), true);
	static	Language			sLanguageJPN(MAKE_OSTYPE('j', 'p', 'n', 0), true);
	static	Language			sLanguageJAV(MAKE_OSTYPE('j', 'a', 'v', 0), false);
	static	Language			sLanguageJRB(MAKE_OSTYPE('j', 'r', 'b', 0), false);
	static	Language			sLanguageJPR(MAKE_OSTYPE('j', 'p', 'r', 0), false);
	static	Language			sLanguageKBD(MAKE_OSTYPE('k', 'b', 'd', 0), false);
	static	Language			sLanguageKAB(MAKE_OSTYPE('k', 'a', 'b', 0), false);
	static	Language			sLanguageKAC(MAKE_OSTYPE('k', 'a', 'c', 0), false);
	static	Language			sLanguageKAL(MAKE_OSTYPE('k', 'a', 'l', 0), false);
	static	Language			sLanguageXAL(MAKE_OSTYPE('x', 'a', 'l', 0), false);
	static	Language			sLanguageKAM(MAKE_OSTYPE('k', 'a', 'm', 0), false);
	static	Language			sLanguageKAN(MAKE_OSTYPE('k', 'a', 'n', 0), false);
	static	Language			sLanguageKAU(MAKE_OSTYPE('k', 'a', 'u', 0), false);
	static	Language			sLanguageKAA(MAKE_OSTYPE('k', 'a', 'a', 0), false);
	static	Language			sLanguageKRC(MAKE_OSTYPE('k', 'r', 'c', 0), false);
	static	Language			sLanguageKRL(MAKE_OSTYPE('k', 'r', 'l', 0), false);
	static	Language			sLanguageKAS(MAKE_OSTYPE('k', 'a', 's', 0), false);
	static	Language			sLanguageCSB(MAKE_OSTYPE('c', 's', 'b', 0), false);
	static	Language			sLanguageKAW(MAKE_OSTYPE('k', 'a', 'w', 0), false);
	static	Language			sLanguageKAZ(MAKE_OSTYPE('k', 'a', 'z', 0), false);
	static	Language			sLanguageKHA(MAKE_OSTYPE('k', 'h', 'a', 0), false);
	static	Language			sLanguageKHM(MAKE_OSTYPE('k', 'h', 'm', 0), false);
	static	Language			sLanguageKHO(MAKE_OSTYPE('k', 'h', 'o', 0), false);
	static	Language			sLanguageKIK(MAKE_OSTYPE('k', 'i', 'k', 0), false);
	static	Language			sLanguageKMB(MAKE_OSTYPE('k', 'm', 'b', 0), false);
	static	Language			sLanguageKIN(MAKE_OSTYPE('k', 'i', 'n', 0), false);
	static	Language			sLanguageKIR(MAKE_OSTYPE('k', 'i', 'r', 0), false);
	static	Language			sLanguageTLH(MAKE_OSTYPE('t', 'l', 'h', 0), false);
	static	Language			sLanguageKOM(MAKE_OSTYPE('k', 'o', 'm', 0), false);
	static	Language			sLanguageKON(MAKE_OSTYPE('k', 'o', 'n', 0), false);
	static	Language			sLanguageKOK(MAKE_OSTYPE('k', 'o', 'k', 0), false);
	static	Language			sLanguageKOR(MAKE_OSTYPE('k', 'o', 'r', 0), true);
	static	Language			sLanguageKOS(MAKE_OSTYPE('k', 'o', 's', 0), false);
	static	Language			sLanguageKPE(MAKE_OSTYPE('k', 'p', 'e', 0), false);
	static	Language			sLanguageKUA(MAKE_OSTYPE('k', 'u', 'a', 0), false);
	static	Language			sLanguageKUM(MAKE_OSTYPE('k', 'u', 'm', 0), false);
	static	Language			sLanguageKUR(MAKE_OSTYPE('k', 'u', 'r', 0), false);
	static	Language			sLanguageKRU(MAKE_OSTYPE('k', 'r', 'u', 0), false);
	static	Language			sLanguageKUT(MAKE_OSTYPE('k', 'u', 't', 0), false);
	static	Language			sLanguageLAD(MAKE_OSTYPE('l', 'a', 'd', 0), false);
	static	Language			sLanguageLAH(MAKE_OSTYPE('l', 'a', 'h', 0), false);
	static	Language			sLanguageLAM(MAKE_OSTYPE('l', 'a', 'm', 0), false);
	static	Language			sLanguageLAO(MAKE_OSTYPE('l', 'a', 'o', 0), false);
	static	Language			sLanguageLAT(MAKE_OSTYPE('l', 'a', 't', 0), false);
	static	Language			sLanguageLAV(MAKE_OSTYPE('l', 'a', 'v', 0), false);
	static	Language			sLanguageLEZ(MAKE_OSTYPE('l', 'e', 'z', 0), false);
	static	Language			sLanguageLIM(MAKE_OSTYPE('l', 'i', 'm', 0), false);
	static	Language			sLanguageLIN(MAKE_OSTYPE('l', 'i', 'n', 0), false);
	static	Language			sLanguageLIT(MAKE_OSTYPE('l', 'i', 't', 0), false);
	static	Language			sLanguageJBO(MAKE_OSTYPE('j', 'b', 'o', 0), false);
	static	Language			sLanguageNDS(MAKE_OSTYPE('n', 'd', 's', 0), false);
	static	Language			sLanguageDSB(MAKE_OSTYPE('d', 's', 'b', 0), false);
	static	Language			sLanguageLOZ(MAKE_OSTYPE('l', 'o', 'z', 0), false);
	static	Language			sLanguageLUB(MAKE_OSTYPE('l', 'u', 'b', 0), false);
	static	Language			sLanguageLUA(MAKE_OSTYPE('l', 'u', 'a', 0), false);
	static	Language			sLanguageLUI(MAKE_OSTYPE('l', 'u', 'i', 0), false);
	static	Language			sLanguageSMJ(MAKE_OSTYPE('s', 'm', 'j', 0), false);
	static	Language			sLanguageLUN(MAKE_OSTYPE('l', 'u', 'n', 0), false);
	static	Language			sLanguageLUO(MAKE_OSTYPE('l', 'u', 'o', 0), false);
	static	Language			sLanguageLUS(MAKE_OSTYPE('l', 'u', 's', 0), false);
	static	Language			sLanguageLTZ(MAKE_OSTYPE('l', 't', 'z', 0), false);
	static	Language			sLanguageRUP(MAKE_OSTYPE('r', 'u', 'p', 0), false);
	static	Language			sLanguageMAC(MAKE_OSTYPE('m', 'a', 'c', 0), false);
	static	Language			sLanguageMAD(MAKE_OSTYPE('m', 'a', 'd', 0), false);
	static	Language			sLanguageMAG(MAKE_OSTYPE('m', 'a', 'g', 0), false);
	static	Language			sLanguageMAI(MAKE_OSTYPE('m', 'a', 'i', 0), false);
	static	Language			sLanguageMAK(MAKE_OSTYPE('m', 'a', 'k', 0), false);
	static	Language			sLanguageMLG(MAKE_OSTYPE('m', 'l', 'g', 0), false);
	static	Language			sLanguageMAY(MAKE_OSTYPE('m', 'a', 'y', 0), false);
	static	Language			sLanguageMAL(MAKE_OSTYPE('m', 'a', 'l', 0), false);
	static	Language			sLanguageMLT(MAKE_OSTYPE('m', 'l', 't', 0), false);
	static	Language			sLanguageMNC(MAKE_OSTYPE('m', 'n', 'c', 0), false);
	static	Language			sLanguageMDR(MAKE_OSTYPE('m', 'd', 'r', 0), false);
	static	Language			sLanguageMAN(MAKE_OSTYPE('m', 'a', 'n', 0), false);
	static	Language			sLanguageMNI(MAKE_OSTYPE('m', 'n', 'i', 0), false);
	static	Language			sLanguageGLV(MAKE_OSTYPE('g', 'l', 'v', 0), false);
	static	Language			sLanguageMAO(MAKE_OSTYPE('m', 'a', 'o', 0), false);
	static	Language			sLanguageARN(MAKE_OSTYPE('a', 'r', 'n', 0), false);
	static	Language			sLanguageMAR(MAKE_OSTYPE('m', 'a', 'r', 0), false);
	static	Language			sLanguageCHM(MAKE_OSTYPE('c', 'h', 'm', 0), false);
	static	Language			sLanguageMAH(MAKE_OSTYPE('m', 'a', 'h', 0), false);
	static	Language			sLanguageMWR(MAKE_OSTYPE('m', 'w', 'r', 0), false);
	static	Language			sLanguageMAS(MAKE_OSTYPE('m', 'a', 's', 0), false);
	static	Language			sLanguageMEN(MAKE_OSTYPE('m', 'e', 'n', 0), false);
	static	Language			sLanguageMIC(MAKE_OSTYPE('m', 'i', 'c', 0), false);
	static	Language			sLanguageDUM(MAKE_OSTYPE('d', 'u', 'm', 0), false);
	static	Language			sLanguageENM(MAKE_OSTYPE('e', 'n', 'm', 0), false);
	static	Language			sLanguageFRM(MAKE_OSTYPE('f', 'r', 'm', 0), false);
	static	Language			sLanguageGMH(MAKE_OSTYPE('g', 'm', 'h', 0), false);
	static	Language			sLanguageMGA(MAKE_OSTYPE('m', 'g', 'a', 0), false);
	static	Language			sLanguageMIN(MAKE_OSTYPE('m', 'i', 'n', 0), false);
	static	Language			sLanguageMWL(MAKE_OSTYPE('m', 'w', 'l', 0), false);
	static	Language			sLanguageGRE(MAKE_OSTYPE('g', 'r', 'e', 0), false);
	static	Language			sLanguageMOH(MAKE_OSTYPE('m', 'o', 'h', 0), false);
	static	Language			sLanguageMDF(MAKE_OSTYPE('m', 'd', 'f', 0), false);
	static	Language			sLanguageLOL(MAKE_OSTYPE('l', 'o', 'l', 0), false);
	static	Language			sLanguageMON(MAKE_OSTYPE('m', 'o', 'n', 0), false);
	static	Language			sLanguageCNR(MAKE_OSTYPE('c', 'n', 'r', 0), false);
	static	Language			sLanguageMOS(MAKE_OSTYPE('m', 'o', 's', 0), false);
	static	Language			sLanguageMUL(MAKE_OSTYPE('m', 'u', 'l', 0), false);
	static	Language			sLanguageNQO(MAKE_OSTYPE('n', 'q', 'o', 0), false);
	static	Language			sLanguageNAU(MAKE_OSTYPE('n', 'a', 'u', 0), false);
	static	Language			sLanguageNAV(MAKE_OSTYPE('n', 'a', 'v', 0), false);
	static	Language			sLanguageNDO(MAKE_OSTYPE('n', 'd', 'o', 0), false);
	static	Language			sLanguageNAP(MAKE_OSTYPE('n', 'a', 'p', 0), false);
	static	Language			sLanguageNEP(MAKE_OSTYPE('n', 'e', 'p', 0), false);
	static	Language			sLanguageNEW(MAKE_OSTYPE('n', 'e', 'w', 0), false);
	static	Language			sLanguageNIA(MAKE_OSTYPE('n', 'i', 'a', 0), false);
	static	Language			sLanguageNIU(MAKE_OSTYPE('n', 'i', 'u', 0), false);
	static	Language			sLanguageZXX(MAKE_OSTYPE('z', 'x', 'x', 0), false);
	static	Language			sLanguageNOG(MAKE_OSTYPE('n', 'o', 'g', 0), false);
	static	Language			sLanguageNDE(MAKE_OSTYPE('n', 'd', 'e', 0), false);
	static	Language			sLanguageFRR(MAKE_OSTYPE('f', 'r', 'r', 0), false);
	static	Language			sLanguageSME(MAKE_OSTYPE('s', 'm', 'e', 0), false);
	static	Language			sLanguageNOR(MAKE_OSTYPE('n', 'o', 'r', 0), false);
	static	Language			sLanguageNOB(MAKE_OSTYPE('n', 'o', 'b', 0), false);
	static	Language			sLanguageNNO(MAKE_OSTYPE('n', 'n', 'o', 0), false);
	static	Language			sLanguageNYM(MAKE_OSTYPE('n', 'y', 'm', 0), false);
	static	Language			sLanguageNYA(MAKE_OSTYPE('n', 'y', 'a', 0), false);
	static	Language			sLanguageNYN(MAKE_OSTYPE('n', 'y', 'n', 0), false);
	static	Language			sLanguageNYO(MAKE_OSTYPE('n', 'y', 'o', 0), false);
	static	Language			sLanguageNZI(MAKE_OSTYPE('n', 'z', 'i', 0), false);
	static	Language			sLanguageOCI(MAKE_OSTYPE('o', 'c', 'i', 0), false);
	static	Language			sLanguageARC(MAKE_OSTYPE('a', 'r', 'c', 0), false);
	static	Language			sLanguageOJI(MAKE_OSTYPE('o', 'j', 'i', 0), false);
	static	Language			sLanguageANG(MAKE_OSTYPE('a', 'n', 'g', 0), false);
	static	Language			sLanguageFRO(MAKE_OSTYPE('f', 'r', 'o', 0), false);
	static	Language			sLanguageGOH(MAKE_OSTYPE('g', 'o', 'h', 0), false);
	static	Language			sLanguageSGA(MAKE_OSTYPE('s', 'g', 'a', 0), false);
	static	Language			sLanguageNON(MAKE_OSTYPE('n', 'o', 'n', 0), false);
	static	Language			sLanguagePEO(MAKE_OSTYPE('p', 'e', 'o', 0), false);
	static	Language			sLanguagePRO(MAKE_OSTYPE('p', 'r', 'o', 0), false);
	static	Language			sLanguageORI(MAKE_OSTYPE('o', 'r', 'i', 0), false);
	static	Language			sLanguageORM(MAKE_OSTYPE('o', 'r', 'm', 0), false);
	static	Language			sLanguageOSA(MAKE_OSTYPE('o', 's', 'a', 0), false);
	static	Language			sLanguageOSS(MAKE_OSTYPE('o', 's', 's', 0), false);
	static	Language			sLanguageOTA(MAKE_OSTYPE('o', 't', 'a', 0), false);
	static	Language			sLanguagePAL(MAKE_OSTYPE('p', 'a', 'l', 0), false);
	static	Language			sLanguagePAU(MAKE_OSTYPE('p', 'a', 'u', 0), false);
	static	Language			sLanguagePLI(MAKE_OSTYPE('p', 'l', 'i', 0), false);
	static	Language			sLanguagePAM(MAKE_OSTYPE('p', 'a', 'm', 0), false);
	static	Language			sLanguagePAG(MAKE_OSTYPE('p', 'a', 'g', 0), false);
	static	Language			sLanguagePAN(MAKE_OSTYPE('p', 'a', 'n', 0), false);
	static	Language			sLanguagePAP(MAKE_OSTYPE('p', 'a', 'p', 0), false);
	static	Language			sLanguageNSO(MAKE_OSTYPE('n', 's', 'o', 0), false);
	static	Language			sLanguagePER(MAKE_OSTYPE('p', 'e', 'r', 0), false);
	static	Language			sLanguagePHN(MAKE_OSTYPE('p', 'h', 'n', 0), false);
	static	Language			sLanguagePON(MAKE_OSTYPE('p', 'o', 'n', 0), false);
	static	Language			sLanguagePOL(MAKE_OSTYPE('p', 'o', 'l', 0), false);
	static	Language			sLanguagePOR(MAKE_OSTYPE('p', 'o', 'r', 0), true);
	static	Language			sLanguagePUS(MAKE_OSTYPE('p', 'u', 's', 0), false);
	static	Language			sLanguageQUE(MAKE_OSTYPE('q', 'u', 'e', 0), false);
	static	Language			sLanguageRAJ(MAKE_OSTYPE('r', 'a', 'j', 0), false);
	static	Language			sLanguageRAP(MAKE_OSTYPE('r', 'a', 'p', 0), false);
	static	Language			sLanguageRAR(MAKE_OSTYPE('r', 'a', 'r', 0), false);
	static	Language			sLanguageRUM(MAKE_OSTYPE('r', 'u', 'm', 0), false);
	static	Language			sLanguageROH(MAKE_OSTYPE('r', 'o', 'h', 0), false);
	static	Language			sLanguageROM(MAKE_OSTYPE('r', 'o', 'm', 0), false);
	static	Language			sLanguageRUN(MAKE_OSTYPE('r', 'u', 'n', 0), false);
	static	Language			sLanguageRUS(MAKE_OSTYPE('r', 'u', 's', 0), true);
	static	Language			sLanguageSAM(MAKE_OSTYPE('s', 'a', 'm', 0), false);
	static	Language			sLanguageSMO(MAKE_OSTYPE('s', 'm', 'o', 0), false);
	static	Language			sLanguageSAD(MAKE_OSTYPE('s', 'a', 'd', 0), false);
	static	Language			sLanguageSAG(MAKE_OSTYPE('s', 'a', 'g', 0), false);
	static	Language			sLanguageSAN(MAKE_OSTYPE('s', 'a', 'n', 0), false);
	static	Language			sLanguageSAT(MAKE_OSTYPE('s', 'a', 't', 0), false);
	static	Language			sLanguageSRD(MAKE_OSTYPE('s', 'r', 'd', 0), false);
	static	Language			sLanguageSAS(MAKE_OSTYPE('s', 'a', 's', 0), false);
	static	Language			sLanguageSCO(MAKE_OSTYPE('s', 'c', 'o', 0), false);
	static	Language			sLanguageGLA(MAKE_OSTYPE('g', 'l', 'a', 0), false);
	static	Language			sLanguageSEL(MAKE_OSTYPE('s', 'e', 'l', 0), false);
	static	Language			sLanguageSRP(MAKE_OSTYPE('s', 'r', 'p', 0), false);
	static	Language			sLanguageSRR(MAKE_OSTYPE('s', 'r', 'r', 0), false);
	static	Language			sLanguageSHN(MAKE_OSTYPE('s', 'h', 'n', 0), false);
	static	Language			sLanguageSNA(MAKE_OSTYPE('s', 'n', 'a', 0), false);
	static	Language			sLanguageIII(MAKE_OSTYPE('i', 'i', 'i', 0), false);
	static	Language			sLanguageSCN(MAKE_OSTYPE('s', 'c', 'n', 0), false);
	static	Language			sLanguageSID(MAKE_OSTYPE('s', 'i', 'd', 0), false);
	static	Language			sLanguageBLA(MAKE_OSTYPE('b', 'l', 'a', 0), false);
	static	Language			sLanguageSND(MAKE_OSTYPE('s', 'n', 'd', 0), false);
	static	Language			sLanguageSIN(MAKE_OSTYPE('s', 'i', 'n', 0), false);
	static	Language			sLanguageSMS(MAKE_OSTYPE('s', 'm', 's', 0), false);
	static	Language			sLanguageDEN(MAKE_OSTYPE('d', 'e', 'n', 0), false);
	static	Language			sLanguageSLO(MAKE_OSTYPE('s', 'l', 'o', 0), false);
	static	Language			sLanguageSLV(MAKE_OSTYPE('s', 'l', 'v', 0), false);
	static	Language			sLanguageSOG(MAKE_OSTYPE('s', 'o', 'g', 0), false);
	static	Language			sLanguageSOM(MAKE_OSTYPE('s', 'o', 'm', 0), false);
	static	Language			sLanguageSNK(MAKE_OSTYPE('s', 'n', 'k', 0), false);
	static	Language			sLanguageNBL(MAKE_OSTYPE('n', 'b', 'l', 0), false);
	static	Language			sLanguageALT(MAKE_OSTYPE('a', 'l', 't', 0), false);
	static	Language			sLanguageSMA(MAKE_OSTYPE('s', 'm', 'a', 0), false);
	static	Language			sLanguageSOT(MAKE_OSTYPE('s', 'o', 't', 0), false);
	static	Language			sLanguageSPA(MAKE_OSTYPE('s', 'p', 'a', 0), true);
	static	Language			sLanguageSRN(MAKE_OSTYPE('s', 'r', 'n', 0), false);
	static	Language			sLanguageZGH(MAKE_OSTYPE('z', 'g', 'h', 0), false);
	static	Language			sLanguageSUK(MAKE_OSTYPE('s', 'u', 'k', 0), false);
	static	Language			sLanguageSUX(MAKE_OSTYPE('s', 'u', 'x', 0), false);
	static	Language			sLanguageSUN(MAKE_OSTYPE('s', 'u', 'n', 0), false);
	static	Language			sLanguageSUS(MAKE_OSTYPE('s', 'u', 's', 0), false);
	static	Language			sLanguageSWA(MAKE_OSTYPE('s', 'w', 'a', 0), false);
	static	Language			sLanguageSSW(MAKE_OSTYPE('s', 's', 'w', 0), false);
	static	Language			sLanguageSWE(MAKE_OSTYPE('s', 'w', 'e', 0), false);
	static	Language			sLanguageGSW(MAKE_OSTYPE('g', 's', 'w', 0), false);
	static	Language			sLanguageSYR(MAKE_OSTYPE('s', 'y', 'r', 0), false);
	static	Language			sLanguageTGL(MAKE_OSTYPE('t', 'g', 'l', 0), false);
	static	Language			sLanguageTAH(MAKE_OSTYPE('t', 'a', 'h', 0), false);
	static	Language			sLanguageTGK(MAKE_OSTYPE('t', 'g', 'k', 0), false);
	static	Language			sLanguageTMH(MAKE_OSTYPE('t', 'm', 'h', 0), false);
	static	Language			sLanguageTAM(MAKE_OSTYPE('t', 'a', 'm', 0), false);
	static	Language			sLanguageTAT(MAKE_OSTYPE('t', 'a', 't', 0), false);
	static	Language			sLanguageTEL(MAKE_OSTYPE('t', 'e', 'l', 0), false);
	static	Language			sLanguageTER(MAKE_OSTYPE('t', 'e', 'r', 0), false);
	static	Language			sLanguageTET(MAKE_OSTYPE('t', 'e', 't', 0), false);
	static	Language			sLanguageTHA(MAKE_OSTYPE('t', 'h', 'a', 0), false);
	static	Language			sLanguageTIB(MAKE_OSTYPE('t', 'i', 'b', 0), false);
	static	Language			sLanguageTIG(MAKE_OSTYPE('t', 'i', 'g', 0), false);
	static	Language			sLanguageTIR(MAKE_OSTYPE('t', 'i', 'r', 0), false);
	static	Language			sLanguageTEM(MAKE_OSTYPE('t', 'e', 'm', 0), false);
	static	Language			sLanguageTIV(MAKE_OSTYPE('t', 'i', 'v', 0), false);
	static	Language			sLanguageDGR(MAKE_OSTYPE('d', 'g', 'r', 0), false);
	static	Language			sLanguageTLI(MAKE_OSTYPE('t', 'l', 'i', 0), false);
	static	Language			sLanguageTPI(MAKE_OSTYPE('t', 'p', 'i', 0), false);
	static	Language			sLanguageTKL(MAKE_OSTYPE('t', 'k', 'l', 0), false);
	static	Language			sLanguageTOG(MAKE_OSTYPE('t', 'o', 'g', 0), false);
	static	Language			sLanguageTON(MAKE_OSTYPE('t', 'o', 'n', 0), false);
	static	Language			sLanguageTSI(MAKE_OSTYPE('t', 's', 'i', 0), false);
	static	Language			sLanguageTSO(MAKE_OSTYPE('t', 's', 'o', 0), false);
	static	Language			sLanguageTSN(MAKE_OSTYPE('t', 's', 'n', 0), false);
	static	Language			sLanguageTUM(MAKE_OSTYPE('t', 'u', 'm', 0), false);
	static	Language			sLanguageTUR(MAKE_OSTYPE('t', 'u', 'r', 0), false);
	static	Language			sLanguageTUK(MAKE_OSTYPE('t', 'u', 'k', 0), false);
	static	Language			sLanguageTVL(MAKE_OSTYPE('t', 'v', 'l', 0), false);
	static	Language			sLanguageTYV(MAKE_OSTYPE('t', 'y', 'v', 0), false);
	static	Language			sLanguageTWI(MAKE_OSTYPE('t', 'w', 'i', 0), false);
	static	Language			sLanguageUDM(MAKE_OSTYPE('u', 'd', 'm', 0), false);
	static	Language			sLanguageUGA(MAKE_OSTYPE('u', 'g', 'a', 0), false);
	static	Language			sLanguageUIG(MAKE_OSTYPE('u', 'i', 'g', 0), false);
	static	Language			sLanguageUKR(MAKE_OSTYPE('u', 'k', 'r', 0), false);
	static	Language			sLanguageUMB(MAKE_OSTYPE('u', 'm', 'b', 0), false);
	static	Language			sLanguageMIS(MAKE_OSTYPE('m', 'i', 's', 0), false);
	static	Language			sLanguageUND(MAKE_OSTYPE('u', 'n', 'd', 0), false);
	static	Language			sLanguageHSB(MAKE_OSTYPE('h', 's', 'b', 0), false);
	static	Language			sLanguageURD(MAKE_OSTYPE('u', 'r', 'd', 0), false);
	static	Language			sLanguageUZB(MAKE_OSTYPE('u', 'z', 'b', 0), false);
	static	Language			sLanguageVAI(MAKE_OSTYPE('v', 'a', 'i', 0), false);
	static	Language			sLanguageVEN(MAKE_OSTYPE('v', 'e', 'n', 0), false);
	static	Language			sLanguageVIE(MAKE_OSTYPE('v', 'i', 'e', 0), false);
	static	Language			sLanguageVOL(MAKE_OSTYPE('v', 'o', 'l', 0), false);
	static	Language			sLanguageVOT(MAKE_OSTYPE('v', 'o', 't', 0), false);
	static	Language			sLanguageWLN(MAKE_OSTYPE('w', 'l', 'n', 0), false);
	static	Language			sLanguageWAR(MAKE_OSTYPE('w', 'a', 'r', 0), false);
	static	Language			sLanguageWAS(MAKE_OSTYPE('w', 'a', 's', 0), false);
	static	Language			sLanguageWEL(MAKE_OSTYPE('w', 'e', 'l', 0), false);
	static	Language			sLanguageFRY(MAKE_OSTYPE('f', 'r', 'y', 0), false);
	static	Language			sLanguageWAL(MAKE_OSTYPE('w', 'a', 'l', 0), false);
	static	Language			sLanguageWOL(MAKE_OSTYPE('w', 'o', 'l', 0), false);
	static	Language			sLanguageXHO(MAKE_OSTYPE('x', 'h', 'o', 0), false);
	static	Language			sLanguageSAH(MAKE_OSTYPE('s', 'a', 'h', 0), false);
	static	Language			sLanguageYAO(MAKE_OSTYPE('y', 'a', 'o', 0), false);
	static	Language			sLanguageYAP(MAKE_OSTYPE('y', 'a', 'p', 0), false);
	static	Language			sLanguageYID(MAKE_OSTYPE('y', 'i', 'd', 0), false);
	static	Language			sLanguageYOR(MAKE_OSTYPE('y', 'o', 'r', 0), false);
	static	Language			sLanguageZAP(MAKE_OSTYPE('z', 'a', 'p', 0), false);
	static	Language			sLanguageZZA(MAKE_OSTYPE('z', 'z', 'a', 0), false);
	static	Language			sLanguageZEN(MAKE_OSTYPE('z', 'e', 'n', 0), false);
	static	Language			sLanguageZHA(MAKE_OSTYPE('z', 'h', 'a', 0), false);
	static	Language			sLanguageZUL(MAKE_OSTYPE('z', 'u', 'l', 0), false);
	static	Language			sLanguageZUN(MAKE_OSTYPE('z', 'u', 'n', 0), false);
	static	Language			sAll[] =
									{
										// Common
										sLanguageCHI,
										sLanguageENG,
										sLanguageFRE,
										sLanguageGER,
										sLanguageITA,
										sLanguageJPN,
										sLanguageKOR,
										sLanguagePOR,
										sLanguageRUS,
										sLanguageSPA,

										// Not common
										sLanguageABK,
										sLanguageACE,
										sLanguageACH,
										sLanguageADA,
										sLanguageADY,
										sLanguageAAR,
										sLanguageAFH,
										sLanguageAFR,
										sLanguageAIN,
										sLanguageAKA,
										sLanguageAKK,
										sLanguageALB,
										sLanguageALE,
										sLanguageAMH,
										sLanguageGRC,
										sLanguageANP,
										sLanguageARA,
										sLanguageARG,
										sLanguageARP,
										sLanguageARW,
										sLanguageARM,
										sLanguageASM,
										sLanguageAST,
										sLanguageAVA,
										sLanguageAVE,
										sLanguageAWA,
										sLanguageAYM,
										sLanguageAZE,
										sLanguageBAN,
										sLanguageBAL,
										sLanguageBAM,
										sLanguageBAS,
										sLanguageBAK,
										sLanguageBAQ,
										sLanguageBEJ,
										sLanguageBEL,
										sLanguageBEM,
										sLanguageBEN,
										sLanguageBHO,
										sLanguageBIK,
										sLanguageBYN,
										sLanguageBIN,
										sLanguageBIS,
										sLanguageZBL,
										sLanguageBOS,
										sLanguageBRA,
										sLanguageBRE,
										sLanguageBUG,
										sLanguageBUL,
										sLanguageBUA,
										sLanguageBUR,
										sLanguageCAD,
										sLanguageCAT,
										sLanguageCEB,
										sLanguageCHG,
										sLanguageCHA,
										sLanguageCHE,
										sLanguageCHR,
										sLanguageCHY,
										sLanguageCHB,
										sLanguageCHN,
										sLanguageCHP,
										sLanguageCHO,
										sLanguageCHU,
										sLanguageCHK,
										sLanguageCHV,
										sLanguageNWC,
										sLanguageSYC,
										sLanguageCOP,
										sLanguageCOR,
										sLanguageCOS,
										sLanguageCRE,
										sLanguageMUS,
										sLanguageCRH,
										sLanguageHRV,
										sLanguageCZE,
										sLanguageDAK,
										sLanguageDAN,
										sLanguageDAR,
										sLanguageDEL,
										sLanguageDIV,
										sLanguageDIN,
										sLanguageDOI,
										sLanguageDUA,
										sLanguageDUT,
										sLanguageDYU,
										sLanguageDZO,
										sLanguageFRS,
										sLanguageEFI,
										sLanguageEGY,
										sLanguageEKA,
										sLanguageELX,
										sLanguageMYV,
										sLanguageEPO,
										sLanguageEST,
										sLanguageEWE,
										sLanguageEWO,
										sLanguageFAN,
										sLanguageFAT,
										sLanguageFAO,
										sLanguageFIJ,
										sLanguageFIL,
										sLanguageFIN,
										sLanguageFON,
										sLanguageFUR,
										sLanguageFUL,
										sLanguageGAA,
										sLanguageCAR,
										sLanguageGLG,
										sLanguageLUG,
										sLanguageGAY,
										sLanguageGBA,
										sLanguageGEZ,
										sLanguageGEO,
										sLanguageGIL,
										sLanguageGON,
										sLanguageGOR,
										sLanguageGOT,
										sLanguageGRB,
										sLanguageGRN,
										sLanguageGUJ,
										sLanguageGWI,
										sLanguageHAI,
										sLanguageHAT,
										sLanguageHAU,
										sLanguageHAW,
										sLanguageHEB,
										sLanguageHER,
										sLanguageHIL,
										sLanguageHIN,
										sLanguageHMO,
										sLanguageHIT,
										sLanguageHMN,
										sLanguageHUN,
										sLanguageHUP,
										sLanguageIBA,
										sLanguageICE,
										sLanguageIDO,
										sLanguageIBO,
										sLanguageILO,
										sLanguageSMN,
										sLanguageIND,
										sLanguageINH,
										sLanguageINA,
										sLanguageILE,
										sLanguageIKU,
										sLanguageIPK,
										sLanguageGLE,
										sLanguageJAV,
										sLanguageJRB,
										sLanguageJPR,
										sLanguageKBD,
										sLanguageKAB,
										sLanguageKAC,
										sLanguageKAL,
										sLanguageXAL,
										sLanguageKAM,
										sLanguageKAN,
										sLanguageKAU,
										sLanguageKAA,
										sLanguageKRC,
										sLanguageKRL,
										sLanguageKAS,
										sLanguageCSB,
										sLanguageKAW,
										sLanguageKAZ,
										sLanguageKHA,
										sLanguageKHM,
										sLanguageKHO,
										sLanguageKIK,
										sLanguageKMB,
										sLanguageKIN,
										sLanguageKIR,
										sLanguageTLH,
										sLanguageKOM,
										sLanguageKON,
										sLanguageKOK,
										sLanguageKOS,
										sLanguageKPE,
										sLanguageKUA,
										sLanguageKUM,
										sLanguageKUR,
										sLanguageKRU,
										sLanguageKUT,
										sLanguageLAD,
										sLanguageLAH,
										sLanguageLAM,
										sLanguageLAO,
										sLanguageLAT,
										sLanguageLAV,
										sLanguageLEZ,
										sLanguageLIM,
										sLanguageLIN,
										sLanguageLIT,
										sLanguageJBO,
										sLanguageNDS,
										sLanguageDSB,
										sLanguageLOZ,
										sLanguageLUB,
										sLanguageLUA,
										sLanguageLUI,
										sLanguageSMJ,
										sLanguageLUN,
										sLanguageLUO,
										sLanguageLUS,
										sLanguageLTZ,
										sLanguageRUP,
										sLanguageMAC,
										sLanguageMAD,
										sLanguageMAG,
										sLanguageMAI,
										sLanguageMAK,
										sLanguageMLG,
										sLanguageMAY,
										sLanguageMAL,
										sLanguageMLT,
										sLanguageMNC,
										sLanguageMDR,
										sLanguageMAN,
										sLanguageMNI,
										sLanguageGLV,
										sLanguageMAO,
										sLanguageARN,
										sLanguageMAR,
										sLanguageCHM,
										sLanguageMAH,
										sLanguageMWR,
										sLanguageMAS,
										sLanguageMEN,
										sLanguageMIC,
										sLanguageDUM,
										sLanguageENM,
										sLanguageFRM,
										sLanguageGMH,
										sLanguageMGA,
										sLanguageMIN,
										sLanguageMWL,
										sLanguageGRE,
										sLanguageMOH,
										sLanguageMDF,
										sLanguageLOL,
										sLanguageMON,
										sLanguageCNR,
										sLanguageMOS,
										sLanguageMUL,
										sLanguageNQO,
										sLanguageNAU,
										sLanguageNAV,
										sLanguageNDO,
										sLanguageNAP,
										sLanguageNEP,
										sLanguageNEW,
										sLanguageNIA,
										sLanguageNIU,
										sLanguageZXX,
										sLanguageNOG,
										sLanguageNDE,
										sLanguageFRR,
										sLanguageSME,
										sLanguageNOR,
										sLanguageNOB,
										sLanguageNNO,
										sLanguageNYM,
										sLanguageNYA,
										sLanguageNYN,
										sLanguageNYO,
										sLanguageNZI,
										sLanguageOCI,
										sLanguageARC,
										sLanguageOJI,
										sLanguageANG,
										sLanguageFRO,
										sLanguageGOH,
										sLanguageSGA,
										sLanguageNON,
										sLanguagePEO,
										sLanguagePRO,
										sLanguageORI,
										sLanguageORM,
										sLanguageOSA,
										sLanguageOSS,
										sLanguageOTA,
										sLanguagePAL,
										sLanguagePAU,
										sLanguagePLI,
										sLanguagePAM,
										sLanguagePAG,
										sLanguagePAN,
										sLanguagePAP,
										sLanguageNSO,
										sLanguagePER,
										sLanguagePHN,
										sLanguagePON,
										sLanguagePOL,
										sLanguagePUS,
										sLanguageQUE,
										sLanguageRAJ,
										sLanguageRAP,
										sLanguageRAR,
										sLanguageRUM,
										sLanguageROH,
										sLanguageROM,
										sLanguageRUN,
										sLanguageSAM,
										sLanguageSMO,
										sLanguageSAD,
										sLanguageSAG,
										sLanguageSAN,
										sLanguageSAT,
										sLanguageSRD,
										sLanguageSAS,
										sLanguageSCO,
										sLanguageGLA,
										sLanguageSEL,
										sLanguageSRP,
										sLanguageSRR,
										sLanguageSHN,
										sLanguageSNA,
										sLanguageIII,
										sLanguageSCN,
										sLanguageSID,
										sLanguageBLA,
										sLanguageSND,
										sLanguageSIN,
										sLanguageSMS,
										sLanguageDEN,
										sLanguageSLO,
										sLanguageSLV,
										sLanguageSOG,
										sLanguageSOM,
										sLanguageSNK,
										sLanguageNBL,
										sLanguageALT,
										sLanguageSMA,
										sLanguageSOT,
										sLanguageSRN,
										sLanguageZGH,
										sLanguageSUK,
										sLanguageSUX,
										sLanguageSUN,
										sLanguageSUS,
										sLanguageSWA,
										sLanguageSSW,
										sLanguageSWE,
										sLanguageGSW,
										sLanguageSYR,
										sLanguageTGL,
										sLanguageTAH,
										sLanguageTGK,
										sLanguageTMH,
										sLanguageTAM,
										sLanguageTAT,
										sLanguageTEL,
										sLanguageTER,
										sLanguageTET,
										sLanguageTHA,
										sLanguageTIB,
										sLanguageTIG,
										sLanguageTIR,
										sLanguageTEM,
										sLanguageTIV,
										sLanguageDGR,
										sLanguageTLI,
										sLanguageTPI,
										sLanguageTKL,
										sLanguageTOG,
										sLanguageTON,
										sLanguageTSI,
										sLanguageTSO,
										sLanguageTSN,
										sLanguageTUM,
										sLanguageTUR,
										sLanguageTUK,
										sLanguageTVL,
										sLanguageTYV,
										sLanguageTWI,
										sLanguageUDM,
										sLanguageUGA,
										sLanguageUIG,
										sLanguageUKR,
										sLanguageUMB,
										sLanguageMIS,
										sLanguageUND,
										sLanguageHSB,
										sLanguageURD,
										sLanguageUZB,
										sLanguageVAI,
										sLanguageVEN,
										sLanguageVIE,
										sLanguageVOL,
										sLanguageVOT,
										sLanguageWLN,
										sLanguageWAR,
										sLanguageWAS,
										sLanguageWEL,
										sLanguageFRY,
										sLanguageWAL,
										sLanguageWOL,
										sLanguageXHO,
										sLanguageSAH,
										sLanguageYAO,
										sLanguageYAP,
										sLanguageYID,
										sLanguageYOR,
										sLanguageZAP,
										sLanguageZZA,
										sLanguageZEN,
										sLanguageZHA,
										sLanguageZUL,
										sLanguageZUN,
									};
	static	TArray<Language>	sAllArray = TSARRAY_FROM_C_ARRAY(Language, sAll);

	return sAllArray;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SLocalization::Language> SLocalization::Language::getFor(OSType iso639_2_Code)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all
	for (TIteratorD<Language> iterator = getAll().getIterator(); iterator.hasValue(); iterator.advance()) {
		// Check
		if (iterator->getISO639_2_Code() == iso639_2_Code)
			// Found
			return OV<Language>(*iterator);
	}

	return OV<Language>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SLocalization::Language> SLocalization::Language::getFor(const CString& iso639_2_CodeString)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate
	if (iso639_2_CodeString.getLength() != 3)
		// Must be 3 characters
		return OV<Language>();

	return getFor((iso639_2_CodeString + CString::mNull).getOSType());
}

//----------------------------------------------------------------------------------------------------------------------
const SLocalization::Language& SLocalization::Language::getDefault()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	Language	sLanguageENG(MAKE_OSTYPE('e', 'n', 'g', 0), true);

	return sLanguageENG;
}

//----------------------------------------------------------------------------------------------------------------------
CString SLocalization::Language::getDisplayName(const TArray<Language>& languages)
//----------------------------------------------------------------------------------------------------------------------
{
	return CString(TNArray<CString>(languages, (TNArray<CString>::MapProc) sGetDisplayNameForLocalizationLanguage));
}

//----------------------------------------------------------------------------------------------------------------------
bool SLocalization::Language::equals(const TArray<Language>& languages1, const TArray<Language>& languages2)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check count
	if (languages1.getCount() != languages2.getCount())
		// Counts differ
		return false;

	// Check items
	for (CArray::ItemIndex i = 0; i < languages1.getCount(); i++) {
		// Check item
		if (!(languages1[i] == languages2[i]))
			// Differ
			return false;
	}

	return true;
}
