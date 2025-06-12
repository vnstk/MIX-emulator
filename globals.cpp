// Vainstein K 2025may24

#include "types.hpp"

using enum eInterpretModifByte;

VerbsRosterEntry verbsRoster[64] =
{
	{ 0, AS_variant,	NULL,	{"NOP",}}	,
	{ 1, AS_fieldSpec5,"FADD",	{"ADD",}}	,
	{ 2, AS_fieldSpec5,"FSUB",	{"SUB",}}	,
	{ 3, AS_fieldSpec5,"FMUL",	{"MUL",}}	,
	{ 4, AS_fieldSpec5,"FDIV",	{"DIV",}}	,
	{ 5, AS_variant,	NULL,	{"NUM","CHAR","HLT",}}	,
	{ 6, AS_variant,	NULL,	{"SLA","SRA","SLAX","SRAX","SLC","SRC",}}	,
	{ 7, AS_xferSzInWords,	NULL,	{"MOVE",}}	,
	{ 8, AS_fieldSpec5,	NULL,	{"LDA",}}	,		//// LD? --- load *M to r?
		{ 9, AS_fieldSpec5,	NULL,	{"LD1",}}	,
		{10, AS_fieldSpec5,	NULL,	{"LD2",}}	,
		{11, AS_fieldSpec5,	NULL,	{"LD3",}}	,
		{12, AS_fieldSpec5,	NULL,	{"LD4",}}	,
		{13, AS_fieldSpec5,	NULL,	{"LD5",}}	,
		{14, AS_fieldSpec5,	NULL,	{"LD6",}}	,
		{15, AS_fieldSpec5,	NULL,	{"LDX",}}	,
	{16, AS_fieldSpec5,	NULL,	{"LDAN",}}	,		//// LD?N --- load sign-flipped *M to r?
		{17, AS_fieldSpec5,	NULL,	{"LD1N",}}	,
		{18, AS_fieldSpec5,	NULL,	{"LD2N",}}	,
		{19, AS_fieldSpec5,	NULL,	{"LD3N",}}	,
		{20, AS_fieldSpec5,	NULL,	{"LD4N",}}	,
		{21, AS_fieldSpec5,	NULL,	{"LD5N",}}	,
		{22, AS_fieldSpec5,	NULL,	{"LD6N",}}	,
		{23, AS_fieldSpec5,	NULL,	{"LDXN",}}	,
	{24, AS_fieldSpec5,	NULL,	{"STA",}}	,		//// ST? --- store contents of r? into M
		{25, AS_fieldSpec5,	NULL,	{"ST1",}}	, 	// bytes 1..3 of rI? are 0
		{26, AS_fieldSpec5,	NULL,	{"ST2",}}	,
		{27, AS_fieldSpec5,	NULL,	{"ST3",}}	,
		{28, AS_fieldSpec5,	NULL,	{"ST4",}}	,
		{29, AS_fieldSpec5,	NULL,	{"ST5",}}	,
		{30, AS_fieldSpec5,	NULL,	{"ST6",}}	,
		{31, AS_fieldSpec5,	NULL,	{"STX",}}	,
		{32, AS_fieldSpec2,	NULL,	{"STJ",}}	,
		{33, AS_fieldSpec5,	NULL,	{"STZ",}}	, 	// Stores "+ 0"
	{34, AS_whichIoUnit,	NULL,	{"JBUS",}}	, // unit busy?
	{35, AS_whichIoUnit,	NULL,	{"IOC",}}	, // ioctl on unit
	{36, AS_whichIoUnit,	NULL,	{"IN",}}	,
	{37, AS_whichIoUnit,	NULL,	{"OUT",}}	,
	{38, AS_whichIoUnit,	NULL,	{"JRED",}}	, // unit ready?
	/* JMP --- rJ := *M; jump unconditional to *M.
	   JSJ --- leave rJ as was; jump unconditional to *M.
	   JOV --- If oflowToggle ON, then { oflowToggle := OFF; rJ := *M; jump to *M }.
	   JNV --- If oflowToggle OFF, then { rJ := *M; jump to *M } , else { oflowToggle := OFF }.
	*/
	{39, AS_variant,	NULL,	{"JMP","JSJ","JOV","JNOV",
	                	    	  "JL", "JE", "JG", "JGE", "JNE", "JLE",}}	,
	/* J?N  --- jump if r? negative
	     Z  ---        ... zero
	     P  ---        ... positive
	    NN  ---        ... nonneg
	    NZ  ---        ... nonzero
	    NP  ---        ... nonpos
	*/
	{40, AS_variant,	NULL,	{"JAN","JAZ","JAP","JANN","JANZ","JANP",}}	,
		{41, AS_variant,	NULL,	{"J1N","J1Z","J1P","J1NN","J1NZ","J1NP",}}	,
		{42, AS_variant,	NULL,	{"J2N","J2Z","J2P","J2NN","J2NZ","J2NP",}}	,
		{43, AS_variant,	NULL,	{"J3N","J3Z","J3P","J3NN","J3NZ","J3NP",}}	,
		{44, AS_variant,	NULL,	{"J4N","J4Z","J4P","J4NN","J4NZ","J4NP",}}	,
		{45, AS_variant,	NULL,	{"J5N","J5Z","J5P","J5NN","J5NZ","J5NP",}}	,
		{46, AS_variant,	NULL,	{"J6N","J6Z","J6P","J6NN","J6NZ","J6NP",}}	,
		{47, AS_variant,	NULL,	{"JXN","JXZ","JXP","JXNN","JXNZ","JXNP",}}	,
	/* 48 through 55 treat instruction's _baseAddr as it were a signed number, call it Z;
	   INC? incrs r? by Z, DEC? decrs r? by Z, ENT? overwrites r? with Z, ENN? overwrites r? with sign-flipped Z
	*/
	{48, AS_variant,	NULL,	{"INCA","DECA","ENTA","ENNA",}}	,
		{49, AS_variant,	NULL,	{"INC1","DEC1","ENT1","ENN1",}}	,
		{50, AS_variant,	NULL,	{"INC2","DEC2","ENT2","ENN2",}}	,
		{51, AS_variant,	NULL,	{"INC3","DEC3","ENT3","ENN3",}}	,
		{52, AS_variant,	NULL,	{"INC4","DEC4","ENT4","ENN4",}}	,
		{53, AS_variant,	NULL,	{"INC5","DEC5","ENT5","ENN5",}}	,
		{54, AS_variant,	NULL,	{"INC6","DEC6","ENT6","ENN6",}}	,
		{55, AS_variant,	NULL,	{"INCX","DECX","ENTX","ENNX",}}	,
	{56, AS_fieldSpec5,"FCMP",	{"CMPA",}}	,		//// Sets cmpIndicator to cmp(r?,*M)
		{57, AS_fieldSpec5,	NULL,	{"CMP1",}}	,
		{58, AS_fieldSpec5,	NULL,	{"CMP2",}}	,
		{59, AS_fieldSpec5,	NULL,	{"CMP3",}}	,
		{60, AS_fieldSpec5,	NULL,	{"CMP4",}}	,
		{61, AS_fieldSpec5,	NULL,	{"CMP5",}}	,
		{62, AS_fieldSpec5,	NULL,	{"CMP6",}}	,
		{63, AS_fieldSpec5,	NULL,	{"CMPX",}}	,
};

