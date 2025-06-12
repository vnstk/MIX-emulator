// Vainstein K 2025may24
#include "types.hpp"

extern VerbsRosterEntry verbsRoster[];
Instance g__state;

#include <cstring> //For memset
#include <cassert>
#include <cstdio>
#include <algorithm> //For std::max

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


oneByte_t to_fieldspec (IdealPosPair ipp) {
	return 8 * ipp._L + ipp._R;
}

void expand_fieldSpec (oneByte_t fs, idealPos_t& posL, idealPos_t& posR) {
	posL = fs / 8;
	posR = fs % 8;
}


template<size_t N>
bool Repr<N>::valid () const {
	if (! _nFieldsPop) // Empty doesn't mean corrupt.
		return true;
    assert(_nFieldsPop <= N);
    //
	reprPos_t prev_posR = 0xFF;
	for (int i = 0; i < _nFieldsPop; ++i) {
		Field const *const pf = &_fields[i];
        if (!_packed) {
            assert(pf->_nDatabytes == 1);
            assert(pf->_numVal < 64); // or 100?
        }
		if (i)
			assert(pf->_pos._L == prev_posR + 1);
		assert(pf->_pos._L <= pf->_pos._R);
		assert(pf->_pos._R <= N);
		prev_posR = pf->_pos._R;
	}
	assert(prev_posR == N);
	return true;
}


bool Word::renderDatabytesArr (oneByte_t (&arr) [N_DATABYTES]) const {
    assert(! empty());
    if (_repr._packed)
        return false;
    for (int i = 0; i < N_DATABYTES; ++i)
        arr[i] = _repr._fields[i]._numVal;
    return true;
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


void Word::populate (eSign sign, oneByte_t b1, oneByte_t b2,
                     oneByte_t b3, oneByte_t b4, oneByte_t b5)
{
    _repr._packed = false;
    _repr._sign = sign;
	_repr._nFieldsPop = 0;
    for (uint8_t i = 1; i <= N_DATABYTES; ++i) {
        Field& f = _repr._fields[_repr._nFieldsPop++];
        f._pos._L = i, f._pos._R = i;
		f._ownsSign = false;
        f._nDatabytes = 1;
    }
	assert(_repr._nFieldsPop == N_DATABYTES);
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


uint8_t Field::calc_nDatabytes (reprPos_t const posL, reprPos_t const posR) {
	return posR - posL + 1;
}
uint8_t Field::calc_nDatabytes (ReprPosPair rpp) {
	return calc_nDatabytes(rpp._L, rpp._R);
}
uint8_t Field::calc_nDatabytes (IdealPosPair ipp) {
	return ipp._R + 1 - (ipp._L ? : 1);
	// Add 1 first, lest intermediate sum drop below 0.
}


Word& Word::packField (IdealPosPair ipp, calcNumVal_t const numVal) {
	assert(ipp._L <= 5 && ipp._R <= 5 && ipp._L <= ipp._R);
    //
    _repr._packed = true;
	//
	if (ipp.affectsSignOnly()) {
		_repr._sign = (numVal >= 0) ? eSign::Positive : eSign::Negative;
		return *this;
	}
	Field& f = _repr._fields[_repr._nFieldsPop++];
	if (ipp._L == 0) {
		f._pos._L = 1;
		f._pos._R = ipp._R;
		f._ownsSign = true;
		_repr._sign = (numVal >= 0) ? eSign::Positive : eSign::Negative;
	} else {
		f._pos._L = ipp._L;
		f._pos._R = ipp._R;
		f._ownsSign = false;
		assert(numVal >= 0);
	}
	f._nDatabytes = f._pos._R - f._pos._L + 1;
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
void op__loadField (Word& dst, Word const& src, IdealPosPair src_ipp)
{
#if DBG
    printf("loadField(dst,src, {%hhu,%hhu})\n",src_ipp._L,src_ipp._R);
#endif
	if (src_ipp._L==0 && src_ipp._R==5) {
        op__loadWord(dst, src);
		return;
	}
	// src's sign is used if it's part of field; otherw, assume +.
	dst._repr._sign = (src_ipp._L == 0) ? src._repr._sign : eSign::Positive;
    if (src_ipp.affectsSignOnly())
        return;

    uint8_t tot_n_src_databytes_loading = 0;
    uint8_t srciField_first{UNKb}, srciField_last{UNKb};
    for (uint8_t srciField = 0; srciField < src._repr._nFieldsPop; ++srciField) {
        Field const& fsrc = src._repr._fields[srciField];
        assert(fsrc._pos._L <= src_ipp._R);
        if (fsrc._pos._L < src_ipp._L)
            continue;
        // At this point, we know that we shall be copying this fsrc.
        //        tot_n_src_databytes_loading += fsrc._nDatabytes;
        tot_n_src_databytes_loading += Field::calc_nDatabytes
            (std::max(fsrc._pos._L, src_ipp._L),
             std::min(fsrc._pos._R, src_ipp._R)); // Deal w partial case.
        if (srciField_first==UNKb)
            srciField_first = srciField;
        if (fsrc._pos._R >= src_ipp._R) {
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
			assert(dst._repr._nFieldsPop < 5);//XXX
            Field& f = dst._repr._fields[dst._repr._nFieldsPop++];
            f._pos._L = f._pos._R = jPos;
            ++jPos;
            f._numVal = 0;
            f._nDatabytes = 1;
            tot_n_dst_databytes_skipped += f._nDatabytes;
        }
        dstiField_beginOverwrite = dst._repr._nFieldsPop;
        dst_posL_nextField = jPos;
    } else {          // dst had been populated previously?
        dst_posL_nextField = 1;
        uint8_t dstiField = 0;
        for (; dstiField < dst._repr._nFieldsPop; ++dstiField) {
            Field& f = dst._repr._fields[dstiField];
            dst_posL_nextField = f._pos._R + 1;
            tot_n_dst_databytes_skipped += f._nDatabytes;
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
    assert(srciField_first != UNKb);
    assert(srciField_first != UNKb);
    assert(srciField_first <= srciField_last);
    uint8_t srciField = srciField_first;
    for (; srciField <= srciField_last; ++srciField) {
        Field const& fsrc = src._repr._fields[srciField];
		assert(dstiField_beginOverwrite < 5);//XXX
        Field& fdst = dst._repr._fields[dstiField_beginOverwrite++];
        if ((fsrc._pos._R - fsrc._pos._L) > (src_ipp._R - (src_ipp._L ? : 1))) {
			// Deal w the case of partial-field load.
            fdst._numVal = UNKv;
            fdst._nDatabytes = Field::calc_nDatabytes(src_ipp);
        } else {
            fdst._numVal = fsrc._numVal;
            fdst._nDatabytes = fsrc._nDatabytes;
        }
        fdst._pos._L = dst_posL_nextField;
        fdst._pos._R = fdst._pos._L + fdst._nDatabytes - 1;
        dst_posL_nextField = fdst._pos._R + 1;
        if (dst_had_been_empty)
            dst._repr._nFieldsPop++;
    }
}


//          ***store***: from register, into memory.
//
void op__storeWord (Word const& srcRegister, Word& dstMainMem) {
    memcpy(&dstMainMem, &srcRegister, sizeof(Word));
}


void op__storeField (Word const& src, Word& dst, IdealPosPair dst_ipp)
{
	assert(! src.empty());
	assert(! dst.empty());
#if DBG
    printf("Entr --- storeField(src,dst, {%hhu,%hhu})\n",
		   dst_ipp._L,dst_ipp._R);
#endif
	if (dst_ipp._L==0 && dst_ipp._R==5) {
        op__storeWord(src, dst);
		return;
	}

	// From description atop p130, **seems** that with ST* needn't bother
	// with packed src or packed dst.  True??
	assert(! src._repr._packed);
	assert(! dst._repr._packed);

	// dst's sign is altered only if is part of the ipp; otherw, leave as was.
	dst._repr._sign = (dst_ipp._L == 0) ? src._repr._sign : dst._repr._sign;
    if (dst_ipp.affectsSignOnly())
        return;

	uint8_t const n_bytes_toCopy = Field::calc_nDatabytes(dst_ipp);
	// Take src's n_bytes_toCopy Rmost bytes.
	uint8_t srciField = Word::N_DATABYTES - n_bytes_toCopy;

	uint8_t dstiField =  dst_ipp._L ? dst_ipp._L - 1 : 0;
#if DBG
	printf("Ini, srciField=%hhu dstiField=%hhu n_bytes_toCopy=%hhu\n",
		   srciField,dstiField,n_bytes_toCopy);
#endif
	for (uint8_t i = 0; i < n_bytes_toCopy; ++i) {
		assert(dstiField < 5);//XXX
		assert(srciField < 5);//XXX
		dst._repr._fields[dstiField++]._numVal =
		src._repr._fields[srciField++]._numVal;
	}
#if DBG
	printf("Fin, srciField=%hhu dstiField=%hhu\n", srciField,dstiField);
#endif
}
