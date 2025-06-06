// Vainstein K 2025may24

#include <cstdint> //For uint8_t etc
#include <utility> //For pair

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

// This could be address, or a relative address (i.e. delta).
typedef int16_t address_t; // Valid range is -4000..4000


calcNumVal_t mkSigned (reprNumVal_t qty, eSign sign);


struct Field {
	uint8_t _posL, _posR; // posL <= posR.
	uint8_t _nDataBytes; // Will be 0 if positions are 0:0 --- i.e. just signbit.
	reprNumVal_t _numVal; // UNK initially; -1 or +1 if positions are 0:0
	oneByte_t to_fieldSpec () const { return 8 * _posL + _posR; }
    static uint8_t calc_nDatabytes (uint8_t posL, uint8_t posR);
};
void decompose_fieldSpec (oneByte_t fs, uint8_t& posL, uint8_t& posR);

template<size_t N> // where N = # of MX-"data"-bytes; not incl the sign-byte.
struct Repr {
	Field   _fields[N+1];
	uint8_t _nFieldsPop; /* If _nFieldsPop==0, invalid. */
    bool    _packed; /* If ! _packed, must have _nFieldsPop==5. */
    eSign   _sign; /*If _packed, pertains only to **1st** data-bearing field. */
    int iFirstDataBearingField () const;
    bool valid () const;
    Field const *findField (uint8_t posL, uint8_t posR) const;
};


struct Word {
    static constexpr int N_DATABYTES = 5;
    Repr<N_DATABYTES>    _repr;
    bool empty () const { return _repr._nFieldsPop == 0; }
    void populate (eSign sign, oneByte_t b1, oneByte_t b2,
                   oneByte_t b3, oneByte_t b4, oneByte_t b5);
    Word& packField (std::pair<uint8_t,uint8_t> posLR, calcNumVal_t numVal);
    bool renderDatabytesArr (oneByte_t (&arr) [N_DATABYTES]) const;
    oneByte_t renderOneDataByte (int iByte) const;    
};


struct Index {
    static constexpr int N_DATABYTES = 2;
    Repr<N_DATABYTES>    _repr;
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


typedef uint8_t indexSpec_t; // Valid range is 1..6
const indexSpec_t NONE=0;
#if 0 // or, instead of indexSpec_t and NONE, maybe just eIndexSpec?
typedef enum eIndexSpec { NONE=0, I1, I2, I3, I4, I5, I6 };
#endif

typedef uint8_t opcode_t; // Valid range is 0..63

struct WordAsInstruction {
	address_t    _baseAddr;
	indexSpec_t  _indexSpec;
	uint8_t      _modification; // Most often, field spec representation.
	opcode_t     _opCode;
	address_t effectiveAddr () const;
	char const *verbName () const;
};


void populate (Field& f, uint8_t posL, uint8_t posR, reprNumVal_t numVal);
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
	char const *_symbName_fpVariant; // If non-NULL, such variant accessible by field==6.
	char const *_symbNames[10]; // At least 0th should alw be populated.
};
