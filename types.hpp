// Vainstein K 2025may24

#ifndef EMU_TYPES__HPP
#define EMU_TYPES__HPP

#include <cstdint> //For uint8_t etc

constexpr int N_memoryWords = 4000;

enum class eToggle : uint8_t { Off=0, On };
enum class eCmp : uint8_t { Unset=0, LESS, EQUAL, GREATER };
enum class eSign : uint8_t { Unk=0, Positive, Negative }; /* Really only Positive
   and Negative are allowable, but I wish to catch uninitialized-style errors. */

typedef uint8_t oneByte_t; // Valid range is 0..N, where 63<=N<=99
constexpr oneByte_t UNKb = UINT8_MAX;

typedef int32_t calcNumVal_t; //Used during calculations

typedef uint32_t reprNumVal_t; //As stored
constexpr reprNumVal_t UNKv = UINT32_MAX;

calcNumVal_t mkSigned (reprNumVal_t qty, eSign sign);

// This could be address, or a relative address (i.e. delta).
typedef int16_t address_t; // Valid range is -4000..4000

typedef uint8_t idealPos_t; // Valid range is 0..6
typedef uint8_t reprPos_t;  // Valid range is 1..5


struct IdealPosPair {
	idealPos_t const  _L, _R;
	IdealPosPair (idealPos_t L, idealPos_t R) : _L(L),_R(R) {}
	IdealPosPair () =delete;
	bool affectsSignOnly () const { return _L==0 && _R==0; }
};
//
oneByte_t to_fieldspec (IdealPosPair);
void expand_fieldspec (oneByte_t fs, idealPos_t& posL, idealPos_t& posR);

struct ReprPosPair {
	reprPos_t  _L, _R;
};


struct Field {
	ReprPosPair   _pos;
	bool          _ownsSign;
	uint8_t       _nDatabytes;
	reprNumVal_t  _numVal; // UNK initially
	static uint8_t calc_nDatabytes (reprPos_t posL, reprPos_t posR);
	static uint8_t calc_nDatabytes (ReprPosPair rpp);
	static uint8_t calc_nDatabytes (IdealPosPair ipp);
};

template<size_t N>
struct Repr {
	Field   _fields[N];
	uint8_t _nFieldsPop; /* If _nFieldsPop==0, empty. */
    bool    _packed; /* If ! _packed, must have _nFieldsPop==5. */
    eSign   _sign; /*If ! _packed, pertains to all fields. */
	bool signIsUnaffiliated () const {
		return _packed && _nFieldsPop && ! _fields[0]._ownsSign; }
    bool valid () const;
};


struct Word {
    static constexpr int N_DATABYTES = 5;
    Repr<N_DATABYTES>   _repr;
    bool empty () const { return _repr._nFieldsPop == 0; }
    void populate (eSign sign, oneByte_t b1, oneByte_t b2,
                   oneByte_t b3, oneByte_t b4, oneByte_t b5);
    Word& packField (IdealPosPair ipp, calcNumVal_t numVal);
    bool renderDatabytesArr (oneByte_t (&arr) [N_DATABYTES]) const;
};

struct Index {
    static constexpr int N_DATABYTES = 2;
    Repr<N_DATABYTES>   _repr;
    bool empty () const { return _repr._nFieldsPop == 0; }
	address_t relativeAddr () const;
};



struct Instance {
	eToggle _overflow;
	eCmp    _cmpIndicator;
	Word    _rA, _rX;
	Index   _rI[7]; // Meant to be indexed by an indexSpec_t; rI[0] is dummy.
	Index   _rJ; // But rJ's sign is always positive.
	Word    _memory[N_memoryWords];
};

void resetWord (Word&);
void resetIndex (Index *);
void resetEntireState (void);


typedef uint8_t indexSpec_t; // Valid range is 1..6
constexpr indexSpec_t NONE=0;

typedef uint8_t opcode_t; // Valid range is 0..63

struct WordAsInstruction {
	address_t    _baseAddr;
	indexSpec_t  _indexSpec;
	uint8_t      _modification; // Most often, field spec representation.
	opcode_t     _opCode;
	address_t effectiveAddr () const;
	char const *verbName () const;
};

void populateFrom (WordAsInstruction& wai, Word const& w);


enum class eInterpretModifByte : uint8_t {
	AS_variant=1,
	AS_fieldSpec5,
	AS_fieldSpec2,
	AS_xferSzInWords,
	AS_whichIoUnit,
};

struct VerbsRosterEntry {
	opcode_t  _opCode;
	eInterpretModifByte  _treat_modifByte_as;
	char const *_symbName_fpVariant; // If non-NULL, variant accessible by field 6.
	char const *_symbNames[10]; // At least 0th should alw be populated.
};


void op__loadWord (Word& dstRegister, Word const& srcMainMem);  // reg <-- mem
void op__loadField (Word& dst, Word const& src, IdealPosPair srcIdealPP);

void op__storeWord (Word const& srcRegister, Word& dstMainMem); // reg --> mem
void op__storeField (Word const& src, Word& dest, IdealPosPair dstIdealPP);


#endif // EMU_TYPES__HPP
