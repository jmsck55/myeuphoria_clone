
// eu_functions.h

// Contains: variables, functions, and procedures

// To Build: Use a modern "C" compiler

// GCC compiler is required for 64-bit version.

// In order for it to be fast, it has to be "4 byte aligned" on 32-bit,
// and "8 byte aligned" on 64-bit

// In this code: Don't change default alignment, the library depends on it.

#pragma once

// When debugging, define:
#define DEBUG

// On 64-bit, Set in project settings:
//#define BITS64

// NOTE: 4 byte aligned on 32bit
// NOTE: 8 byte aligned on 64bit

typedef char* cstring;

#ifdef BITS64

#define ENCODE_PTR(T)
// Can address: "8589934592 GB" of memory
#define GET_PTR(T) ((uint64_t)T & 0x1FFFFFFFFFFFFFFFull)
#define GET_TYPE(T) ((uint64_t)T & 0xE000000000000000ull)
#define IS_NOT_INTEGER(T) ((uint64_t)T & 0x8000000000000000ull)
#define IS_SEQ 0x8000000000000000ull
#define IS_ATOM 0xC000000000000000ull
#define IS_POINTER 0xE000000000000000ull

typedef int64_t integer;
typedef long double atom;

typedef union object_struct1 {
	struct type1 {
		uint64_t reserved : 61;
		uint64_t flags : 3;
	} debug;
	atom* atom_ptr; // 80-bit double, Needs to be compiled with 64-bit GCC
	uint64_t* unsigned_ptr; // Used to declare unsigned __int64
	struct sequence_struct1* seq_ptr;
	uint64_t data;
} object;

#define GET_LENGTH(T) (T & 0x3FFFFFFFFFFFFFFFull)
#define SEQ_TYPE(T) (T & 0xC000000000000000ull)
#define EU_SEQUENCE 0ull
#define EU_REPEAT 0x4000000000000000ull
#define EU_DUPLICATE 0x8000000000000000ull
#define EU_BYTES 0xC000000000000000ull

typedef struct sequence_struct1 {
	union {
		struct {
			uint64_t length : 62;
			uint64_t flags : 2; // seq=0, rep=1, dup=2, bytes=3
		} debug;
		uint64_t type_and_length;
	};
	union {
		object* myarray; // for seq or rep
		struct sequence_struct1* dup; // for dup only
		unsigned char* bytes; // for memory data
	};
} sequence;

#else


#define ENCODE_PTR(T) T >>= 1
#define GET_PTR(T) (((uint32_t)T << 1) & 0xFFFFFFFC)
#define GET_TYPE(T) ((uint32_t)T & 0x80000001)
#define IS_NOT_INTEGER(T) ((uint32_t)T & 0x80000000)
#define IS_SEQ 0x80000000
#define IS_ATOM 0x80000001

typedef int32_t integer;
typedef double atom;

typedef union object_struct1 {
	struct type1 {
		uint32_t low_flag : 1;
		uint32_t reserved : 30;
		uint32_t flag : 1;
	} debug;
	atom* atom_ptr; // Used to store unsigned __int32
	struct sequence_struct1* seq_ptr;
	uint32_t data;
} object;

#define GET_LENGTH(T) (T & 0x3FFFFFFFul)
#define SEQ_TYPE(T) (T & 0xC0000000ul)
#define EU_SEQUENCE 0ul
#define EU_REPEAT 0x40000000ul
#define EU_DUPLICATE 0x80000000ul
#define EU_BYTES 0xC0000000ul

typedef struct sequence_struct1 {
	union {
		struct {
			uint32_t length : 30;
			uint32_t flags : 2; // seq=0, rep=1, dup=2, memory==3
		} debug;
		uint32_t type_and_length;
	};
	union {
		object* myarray; // for seq or rep
		struct sequence_struct1* dup; // for dup only
		unsigned char* bytes; // for memory data
	};
} sequence;

#endif // BITS64

// Function Prototypes:

void myprint_int(integer i);
void eu_print_int(integer i);
void myprint_seq(sequence seq);
void eu_print_seq(sequence seq);
void myprint(object ob);
void eu_print(object ob);

void eu_abort(int errorlevel);
void error(int i);

// free and deallocate functions:

void* mymalloc(size_t size);
void* mycalloc(size_t count, size_t size);

void myfree(void* ptr);
void deallocate_object_array(object* arr, size_t len);
int deallocate_sequence(sequence seq);
void free_sequence(sequence* seq);
void deallocate_object(object ob);
void free_object(object* ob);

// Utility functions:

void set_object_to_atom(object* ob, atom num);
void set_object_to_pointer(object* ob, size_t num);
void set_object_to_integer(object* ob, integer intpart);
atom get_atom_from_object(object ob);
size_t get_pointer_from_object(object ob);
integer get_integer_from_object(object ob);
char get_char_from_object(object ob);

char* sequence_to_string(sequence seq);
sequence string_to_sequence(char* str);
sequence get_sequence_from_array_of_strings(size_t len, char* args[]);

void make_object_into_dup(object* a);
object eu_at_with_options(sequence seq, integer index, bool make_dup); // default is 1 for make_dup
object eu_at(sequence seq, integer index); // when in doubt, use this function, read/write to returned values, and you have to deallocate
const object eu_at_readonly(const sequence seq, integer index); // cannot write to returned values
sequence seq_soft_copy(sequence seq, int either_pend);
object obj_soft_copy(object ob);

void eu_append_to_self(sequence** seq, object x);

// Begin "eu" prefix functions:

void eu_cleanup(void);
void eu_init(int myargc, char** myargv);
sequence eu_command_line();

integer eu_length(sequence seq);

void c_puts(FILE* fn, char* str);
void eu_puts(integer fn, object st);

// Non-member functions:

// find(x,s,i), match(st,s,i)
bool eu_sequences_equal(sequence* a, sequence* b);
bool eu_equal(object a, object b);
integer eu_find(object x, sequence* s, integer i, integer stop_at);
integer eu_match(sequence* st, sequence* s, integer i);

