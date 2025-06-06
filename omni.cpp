// Vainstein K 2025may24
#include "types.hpp"
#include "globals.cpp"

Instance g__state;

#include <string.h> //For memset
#include <assert.h>
#include <stdio.h>
#include <algorithm>

#ifndef DBG
#	define DBG 0
#endif

calcNumVal_t mkSigned (reprNumVal_t qty, eSign sign) {
	assert(qty != UNKv);
	assert(sign != eSign::Unk);
	assert((long)qty <= (long)INT32_MAX);
	calcNumVal_t rv = qty;
	rv *= (sign == eSign::Positive) ? +1 : -1;
	return rv;
}

void resetWord (Word& x) {
	memset(&x, '\0', sizeof(Word));
    x._repr._sign = eSign::Positive; // That's what STZ instru does.
}
void resetIndex (Index *const p) {
	memset(p, '\0', sizeof(Index)); // _sign becomes eSign::Unk
}

void resetEntireState (void) {
    memset(&g__state, '\0', sizeof(Instance));;
    g__state._rJ._repr._sign = eSign::Positive; // Alw pos.
}



void decompose_fieldSpec (oneByte_t fs, uint8_t& posL, uint8_t& posR) {
	posL = fs / 8;
	posR = fs % 8;
}


template<size_t N>
int Repr<N>::iFirstDataBearingField () const {
    if (_packed)
        if (_fields[0]._posR == 0)
            return 1;
    return 0;
}


template<size_t N>
bool Repr<N>::valid () const {
	if (! _nFieldsPop)
		return false;
    if (_sign == eSign::Unk)
        return false;
    assert(_nFieldsPop <= sizeof _fields / sizeof _fields[0]);
    //
	uint8_t prev_posR = 0xFF;
	for (int i = 0; i < _nFieldsPop; ++i) {
		Field const *const pf = &_fields[i];
        if (!_packed || (pf->_posL == pf->_posR && pf->_posR != 0)) {
            assert(pf->_nDataBytes == 1);
            assert(pf->_numVal < 64); // or 100?
        }
		if (i)// == 0)
            //			assert(pf->_posL == (_packed ? 0 : 1));
            //		else
			assert(pf->_posL == prev_posR + 1);
		assert(pf->_posL <= pf->_posR);
		assert(pf->_posR <= N);
		prev_posR = pf->_posR;
	}
	assert(prev_posR == N);
	return true;
}


bool Word::renderDatabytesArr (oneByte_t (&arr) [N_DATABYTES]) const {
    assert(! empty());
    if (_repr._packed)
        return UNKb;
    for (int i = 0; i < N_DATABYTES; ++i)
        arr[i] = _repr._fields[i]._numVal;
    return true;
}

oneByte_t Word::renderOneDataByte (int iByte) const {
    assert(iByte < N_DATABYTES);
    oneByte_t arr[N_DATABYTES];
    if (! renderDatabytesArr(arr))
        return UNKb;
    return arr[iByte];
}


address_t Index::relativeAddr () const {
    assert(_repr._nFieldsPop == 1);
    return mkSigned(_repr._fields[0]._numVal, _repr._sign);
}

address_t WordAsInstruction::effectiveAddr () const {
	address_t addr = _baseAddr;
	assert(_indexSpec <= 6);
	if (_indexSpec) {
		Index const *const p = &g__state._rI[_indexSpec];
		assert(! p->empty());
		addr += p->relativeAddr();
	}
	return addr;
}

char const *WordAsInstruction::verbName () const {
	assert(_opCode <= 63);
	VerbsRosterEntry const *p = &verbsRoster[_opCode];
	if (p->_symbName_fpVariant && _modification == 6)
		return p->_symbName_fpVariant;
	if (p->_treat_modifByte_as == eInterpretModifByte::AS_variant) {
		assert(p->_symbNames[_modification]);
		return p->_symbNames[_modification];
	}
	return p->_symbNames[0];
}

void populateFrom (WordAsInstruction& wai, Word const& w) {
    assert(! w.empty());
    assert(w._repr.valid());
    assert(! w._repr._packed);
    //
    oneByte_t bytes[Word::N_DATABYTES];
    if (! w.renderDatabytesArr(bytes))
        assert(false);
    //
	wai._baseAddr = mkSigned(bytes[0] * 64 + bytes[1], (eSign) w._repr._sign);
	wai._indexSpec = bytes[2];
	wai._modification = bytes[3];
	wai._opCode = bytes[4];
}

void populate (Field& f, uint8_t posL, uint8_t posR, reprNumVal_t numVal)
{
	f._posL = posL;
	f._posR = posR;
	if (posL == 0 && posR == 1)
		f._nDataBytes = 0;
	else
		f._nDataBytes = posR - posL + 1;
	f._numVal = numVal;
}


void Word::populate (eSign sign, oneByte_t b1, oneByte_t b2,
                     oneByte_t b3, oneByte_t b4, oneByte_t b5)
{
    _repr._packed = false;
    _repr._sign = sign;
    for (uint8_t i = 1; i <= N_DATABYTES; ++i) {
        Field& f = _repr._fields[_repr._nFieldsPop++];
        f._posL = i, f._posR = i;
        f._nDataBytes = 1;
    }
	assert(b1 < 64);
	assert(b2 < 64);
	assert(b3 < 64);
	assert(b4 < 64);
	assert(b5 < 64);
	//
    _repr._fields[0]._numVal = b1;
    _repr._fields[1]._numVal = b2;
    _repr._fields[2]._numVal = b3;
    _repr._fields[3]._numVal = b4;
    _repr._fields[4]._numVal = b5;
}


uint8_t Field::calc_nDatabytes (uint8_t const posL, uint8_t const posR) {
    if (posL==0 && posR == 0)
		return 0;
	else if (posL==0)
		return posR;
	else
		return posR - posL + 1;
}


Word& Word::packField (std::pair<uint8_t,uint8_t> posLR , calcNumVal_t const numVal) {
    const uint8_t posL = posLR.first, posR = posLR.second;
	assert(posL <= 5);
	assert(posR <= 5);
	assert(posL <= posR);
    //
    _repr._packed = true;
	//
	Field& f = _repr._fields[_repr._nFieldsPop++];
	f._posL = posL, f._posR = posR;
    f._nDataBytes = Field::calc_nDatabytes(posL,posR);
	if (posL == 0) {
		_repr._sign = (numVal >= 0) ? eSign::Positive : eSign::Negative;
	} else {
		assert(numVal >= 0);
	}
	f._numVal = abs(numVal);
    return *this; // So could chain calls on single line.
}


//          ***load***: into register, from memory.
//
void op__loadWord (Word& dstRegister, Word const& srcMainMem) {
    memcpy(&dstRegister, &srcMainMem, sizeof(Word));
}
//
/* If a partial field is taken as input, sign is used if it is part of
   that field; otherw, sign defaults to Pos.  Field is Rshifted to least-
   signif bytes of dst.
  Packed load: not-owritten bytes are zeroed.
 */
void op__loadField (Word& dst, Word const& src,
                    uint8_t const src_posL, uint8_t const src_posR)
{
	if (src_posL==0 && src_posR==5) {
        op__loadWord(dst, src);
		return;
	}

	// src's sign is used if it's part of field; otherw, assume +.
	dst._repr._sign = (src_posL == 0) ? src._repr._sign : eSign::Positive;

    if (src_posL==0 && src_posR==0) // Only job in this case, is to set sign.
        return;

    uint8_t tot_n_src_databytes_loading = 0;
    uint8_t srciField_first{UNKb}, srciField_last{UNKb};
    for (uint8_t srciField = 0; srciField < src._repr._nFieldsPop; ++srciField) {
        Field const& fsrc = src._repr._fields[srciField];
        if (fsrc._posL==0 && fsrc._posR==0)
            continue;
        if (fsrc._posL > src_posR)
            assert(false); // shouldn't happen
        if (fsrc._posL < src_posL)
            continue;
        // At this point, we know that we shall be copying this fsrc.
        //        tot_n_src_databytes_loading += fsrc._nDataBytes;
        tot_n_src_databytes_loading += Field::calc_nDatabytes
            (std::max(fsrc._posL,src_posL),
             std::min(fsrc._posR,src_posR)); // Deal w partial case.
        if (srciField_first==UNKb)
            srciField_first = srciField;
        if (fsrc._posR >= src_posR) {
            srciField_last = srciField;
            break;
        }
    }
#if DBG
    printf("tot_n_src_databytes_loading=%hhu srciField_first=%hhu,_last=%hhu\n",
           tot_n_src_databytes_loading,      srciField_first,srciField_last);
#endif

    uint8_t const tot_n_dst_databytes_toSkip = 5 - tot_n_src_databytes_loading;
    uint8_t tot_n_dst_databytes_skipped = 0;
    uint8_t dstiField_beginOverwrite, dst_posL_nextField;
    bool const dst_had_been_empty = dst.empty();
    if (dst_had_been_empty) { // dst empty initially?
        uint8_t jPos = 1;
        uint8_t dstiField = 0;
        for (; dstiField < 5; ++dstiField) {
            if (tot_n_dst_databytes_skipped + 1 > tot_n_dst_databytes_toSkip)
                break;
            Field& f = dst._repr._fields[dst._repr._nFieldsPop++];
            f._posL = f._posR = jPos;
            ++jPos;
            f._numVal = 0;
            f._nDataBytes = 1;
            tot_n_dst_databytes_skipped += f._nDataBytes;
        }
        dstiField_beginOverwrite = dst._repr._nFieldsPop;
        dst_posL_nextField = jPos;
    } else {          // dst had been populated previously?
        dst_posL_nextField =
            dst._repr._fields[dst._repr.iFirstDataBearingField()]._posR + 1;
        uint8_t dstiField = 0;
        for (; dstiField < dst._repr._nFieldsPop; ++dstiField) {
            Field& f = dst._repr._fields[dstiField];
            dst_posL_nextField = f._posR + 1;
            tot_n_dst_databytes_skipped += f._nDataBytes;
            if (tot_n_dst_databytes_skipped >= tot_n_dst_databytes_toSkip)
                break;
        }
        dstiField_beginOverwrite = dstiField;
    }
#if DBG
    printf("tot_n_dst_databytes_skipped=%hhu dstiField_beginOverwrite=%hhu "
           "dst_posL_nextField=%hhu dst_had_been_empty=%c\n",
           tot_n_dst_databytes_skipped, dstiField_beginOverwrite,
           dst_posL_nextField, dst_had_been_empty?'T':'F');
#endif
    dst._repr._packed = true; // XXX sure??  Always?

    // // Now, finally copy over the 1+ fields that fall into [posL,posR].
    /*TODO, special logic for when *part* of a src field is transferred
     */
    assert(srciField_first != UNKb);
    assert(srciField_first != UNKb);
    assert(srciField_first <= srciField_last);
    uint8_t srciField = srciField_first;
    for (; srciField <= srciField_last; ++srciField) {
        Field const& fsrc = src._repr._fields[srciField];
        Field& fdst = dst._repr._fields[dstiField_beginOverwrite++];
        if ((fsrc._posR - fsrc._posL) > (src_posR - src_posL)) { // Partial??
            fdst._numVal = UNKv;
            fdst._nDataBytes = Field::calc_nDatabytes(src_posL,src_posR);
        } else {
            fdst._numVal = fsrc._numVal;
            fdst._nDataBytes = fsrc._nDataBytes;
        }
        fdst._posL = dst_posL_nextField;
        fdst._posR = fdst._posL + fdst._nDataBytes - 1;
        dst_posL_nextField = fdst._posR + 1;
        if (dst_had_been_empty)
            dst._repr._nFieldsPop++;
    }
}


//          ***store***: from register, into memory.
//
void op__storeWord (Word const& srcRegister, Word& dstMainMem) {
    memcpy(&dstMainMem, &srcRegister, sizeof(Word));
}
//
void op__storeField (Word const& src, uint8_t src_posL, uint8_t src_posR  ,  Word& dest) {

}


#include "main.cpp"
