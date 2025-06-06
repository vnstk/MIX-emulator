// Vainstein K 2025may24

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
	if (! w.empty() && (p->_fields[0]._posR == 0)) // first field is just the sign??
		++iField;

    uint8_t const eff_nFieldsPop = w.empty() ? 5 : p->_nFieldsPop;
	for (; iField < eff_nFieldsPop; ++iField) {
		Field const *const pf = &p->_fields[iField];
        uint8_t const eff_nDatabytes = w.empty() ? 1 : pf->_nDataBytes;

        strcat(bufA, " --");
        for (int jSlot = 1; jSlot < eff_nDatabytes; ++jSlot)
			strcat(bufA, "+--");

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

	rX.populate(eSign::Negative, 2,4,6,58,9);
	linescatWord(rX, lines);

	linescatTag("   rX", lines);

	prLines(lines);

    // // // Test the LD* ops.
    resetWord(rA);

    Word& m2222 = g__state._memory[2222];
    resetWord(m2222);
    m2222.packField({0,0} ,-1).packField({1,2} ,80)
         .packField({3,3} ,3).packField({4,4} ,5).packField({5,5} ,4);

    char buf[64];
#define PR_rA(descript)                     \
    sprintf(buf, "/Ln%d/  %s", __LINE__,descript);  \
	memset(&lines, '\0', sizeof lines);     \
    linescatTag(buf, lines);                \
	linescatWord(rA, lines);                \
    resetWord(rA);                          \
	prLines(lines)

    op__loadField(rA, m2222, 1,5);
    PR_rA("    1:5  ");

    op__loadField(rA, m2222, 3,5);
    PR_rA("    3:5  ");

    op__loadField(rA, m2222, 0,3);
    PR_rA("    0:3  ");

    op__loadField(rA, m2222, 4,4);
    PR_rA("    4:4  ");

    op__loadField(rA, m2222, 0,0);
    PR_rA("    0:0  ");

    op__loadField(rA, m2222, 1,1);
    PR_rA("    1:1  ");


}
