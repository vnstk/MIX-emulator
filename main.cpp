// Vainstein K 2025may24

#include "types.hpp"

extern Instance g__state;

#include <cassert>
#include <cstdio> //For sprintf
#include <cstring> //For memset



char const *prettySign (eSign sign) {
	return sign == eSign::Positive ? "\e[33;1m+\e[0m" : "\e[33;1m-\e[0m";
}

struct Lines {
	char _topLine[256];
	char _midLine[256];
	char _botLine[256];
};
//
void prLines (Lines& lines) {
	printf("%s\n%s\n%s\n\n", lines._topLine,lines._midLine,lines._botLine);
	memset(&lines, '\0', sizeof lines);
}


/* Assumes that lines points to at least 3 lines.
Will append some N chars (plus EOL) to each of the first 3 lines.
Will return the N.
*/
void linescatWord (Word const& w, Lines& lines)
{
	Repr<5> const *const p = &w._repr;
	char bufA[64] = {'\0',};
	char bufB[64] = {'\0',};

	// L border and the sign
	strcat(bufA, " ---");
	sprintf(bufB, "| %s ", prettySign(p->_sign));
	strcat(lines._midLine, bufB);

	int iField = 0;
	if (! w.empty() && (p->_fields[0]._pos._R == 0)) // just the sign??
		++iField;

    uint8_t const eff_nFieldsPop = w.empty() ? 5 : p->_nFieldsPop;
	for (; iField < eff_nFieldsPop; ++iField) {
		Field const *const pf = &p->_fields[iField];
        uint8_t const eff_nDatabytes = w.empty() ? 1 : pf->_nDatabytes;

        strcat(bufA, " --");
        for (int jSlot = 1; jSlot < eff_nDatabytes; ++jSlot)
			strcat(bufA, "+--");

		if (pf->_ownsSign)
			strcat(lines._midLine, " ");
		else
			strcat(lines._midLine, "|"); // Print partition bar

        char bufC[16] = {'\0',};
        if (pf->_numVal == UNKv)
            strcpy(bufC, "?");
        else
            sprintf(bufC, "%u", pf->_numVal);
        int const numstringWidth = strlen(bufC);
        int const availWidth = eff_nDatabytes * 2 + (eff_nDatabytes - 1) * 1;
        assert(availWidth >= numstringWidth);
        int const marginWidth = (availWidth - numstringWidth) / 2;
			//  %7.1d   means "total widfth 7, squeeze to Rmost, pad w spaces".
        sprintf(bufB, "\e[1m%*s\e[0m%*s",
			    availWidth - marginWidth, bufC, marginWidth, "");
        strcat(lines._midLine, bufB);
	}
	// R border
	strcat(bufA, " ");
	strcat(lines._midLine, "|");

	strcat(lines._topLine, bufA);
	strcat(lines._botLine, bufA);
} /* Aiming at
				 --- -- -- --+-- --
				| + |63| 1|  1  | 0|
				 --- -- -- --+-- --
    if packed.

*/

/* Aiming at
				 ---+--+-- -- -- ----
				| +  2000 | 1|22|CMPA|
				 ---+--+-- -- -- ----
*/
//Note, with instructions sign is alw part of address, so
//The only always-present internal boundary is betw sign and byte0.
void linescatInstruction (WordAsInstruction const& wai, Lines& lines)
{
#define HORIZ_BORD " ---+--+-- -- -- ---- "
	strcat(lines._topLine, HORIZ_BORD);
	char buf[64];
	sprintf(buf, "| %s  %4.1hd |%2.1hhu|%2.1hhu|%4s|",
		 prettySign((wai._baseAddr >= 0) ? eSign::Positive : eSign::Negative),
	     wai._baseAddr, wai._indexSpec, wai._modification, wai.verbName());
	strcat(lines._midLine, buf);
	strcat(lines._botLine, HORIZ_BORD);
#undef HORIZ_BORD
}

void linescatTag (char const *const tag, Lines& lines)
{
	char buf[64];
	const int taglen = strlen(tag);
	assert(taglen);
	assert((taglen + 12) < sizeof buf);

	memset(buf, '\0', sizeof buf);
	sprintf(buf, "%*s", taglen, "");
	strcat(lines._topLine, buf);
	strcat(lines._botLine, buf);

	memset(buf, '\0', sizeof buf);
	sprintf(buf, "\e[36;1m%s\e[0m", tag);
	strcat(lines._midLine, buf);
}


int main ()
{
	resetEntireState();
    Word& rA = g__state._rA, rX = g__state._rX;

	Lines lines;
	memset(&lines, '\0', sizeof lines);

    // // // Demo printing.
	linescatTag("rA   ", lines);

    rA.packField({0,3} ,-1000).packField({4,5} ,7);
	linescatWord(rA, lines);

	linescatTag("        ", lines);

	rX.populate(eSign::Negative, 2,4,6,58,9); // Needn't reset() first.
	linescatWord(rX, lines);

	linescatTag("   rX", lines);

	prLines(lines);


	char buf[64];
	Word& m2222 = g__state._memory[2222];

#define BEGIN_section(s)                                   \
	printf("\n\n\e[32;1m~~~~~~~~~~~~~~~ \e[22;7m%s\e[23m ~~~~~~~~~~\e[0m\n\n",s)


	//======================================================================//
	BEGIN_section("LD* examples: 1.3.1, p129"); //==========================//
    resetWord(m2222);
    m2222.packField({0,0} ,-1).packField({1,2} ,80)
         .packField({3,3} ,3).packField({4,4} ,5).packField({5,5} ,4);
    resetWord(rA);

#define PR_one(descript)                     \
    sprintf(buf, "/Ln%d/      %s  ", __LINE__,descript);  \
    linescatTag(buf, lines);                \
	linescatWord(rA, lines);                \
    resetWord(rA);                          \
	prLines(lines)

    op__loadField(rA, m2222, {1,5});    PR_one("1:5");
    op__loadField(rA, m2222, {3,5});    PR_one("3:5");
    op__loadField(rA, m2222, {0,3});    PR_one("0:3");
    op__loadField(rA, m2222, {4,4});    PR_one("4:4");
    op__loadField(rA, m2222, {0,0});    PR_one("0:0");
    op__loadField(rA, m2222, {1,1});    PR_one("1:1");
#undef PR_one


	//======================================================================//
	BEGIN_section("ST* examples: 1.3.1, p130"); //==========================//
	rA.populate(eSign::Positive, 6,7,8,9,0);
	m2222.populate(eSign::Negative, 1,2,3,4,5);

#define PR_one(descript)                     \
    sprintf(buf, "/Ln%d/      %s  ", __LINE__,descript);  \
    linescatTag(buf, lines);                \
	linescatWord(m2222, lines);             \
	prLines(lines);                         \
	rA.populate(eSign::Positive, 6,7,8,9,0);	\
	m2222.populate(eSign::Negative, 1,2,3,4,5)

	op__storeField(rA, m2222, {1,5});    PR_one("1:5");
	op__storeField(rA, m2222, {5,5});    PR_one("5:5");
	op__storeField(rA, m2222, {2,2});    PR_one("2:2");
	op__storeField(rA, m2222, {2,3});    PR_one("2:3");
	op__storeField(rA, m2222, {0,1});    PR_one("0:1");
#undef PR_one


	//======================================================================//
	BEGIN_section("arith op examples: 1.3.1, pp132-3"); //==================//


	
}
