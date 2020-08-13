
// eu_functions.c

// To Build: Use a modern "C" compiler

// GCC compiler is required for 64-bit version.

#include "pch.h"
#include "eu_functions.h"

static int CHUNK = 100; // must be less than 212 on most systems.

static int c_argc;
static char** c_argv;

static sequence eu_files = { 0,0,0 };

void myprint_int(integer i)
{
#ifdef BITS64
	printf("%lli", i);
#else
	printf("%i", i);
#endif
}

void eu_print_int(integer i)
{
	myprint_int(i);
	printf("\n");
}

void myprint_seq(sequence seq)
{
	sequence* ptr = SEQ_TYPE(seq.type_and_length) == EU_DUPLICATE ? seq.dup : &seq;
	size_t i, len = GET_LENGTH(ptr->type_and_length);
	printf("{");
	if (len > 0)
	{
		myprint(eu_at_readonly(*ptr, 1));
		for (i = 2; i <= len; i++)
		{
			printf(",");
			myprint(eu_at_readonly(*ptr, i));
		}
	}
	printf("}");
}

void eu_print_seq(sequence seq)
{
	myprint_seq(seq);
	printf("\n");
}

void myprint(object ob)
{
	if (IS_NOT_INTEGER(ob.data))
	{
		switch (GET_TYPE(ob.data))
		{
		case IS_ATOM:
			printf("%.40g", *(atom*)GET_PTR(ob.atom_ptr));
			break;
#ifdef BITS64
		case IS_POINTER:
			printf("%llu", *(uint64_t*)GET_PTR(ob.unsigned_ptr));
			break;
#endif
		case IS_SEQ:
		{
			myprint_seq(*(sequence*)GET_PTR(ob.seq_ptr));
			break;
		}
		default:
			break;
		}
	}
	else
	{
		myprint_int(get_integer_from_object(ob));
	}
}

void eu_print(object ob)
{
	myprint(ob);
	printf("\n");
}

void eu_abort(int errorlevel)
{
	if (errno > 0)
	{
		perror(*c_argv);
	}
	exit(errorlevel);
}

void error(int i)
{
	errno = i;
	eu_abort(0);
}

// Allocate, free, and deallocate functions:

void* mymalloc(size_t size)
{
	void* ret = malloc(size);
	if (ret == NULL)
	{
		error(12); // 12. Not enough space
	}
	return ret;
}

void* mycalloc(size_t count, size_t size)
{
	void* ret = calloc(count, size);
	if (ret == NULL)
	{
		error(12); // 12. Not enough space
	}
	return ret;
}

void myfree(void* ptr)
{
	free(ptr);
}

void deallocate_object_array(object* arr, size_t len)
{
	while (len > 0)
	{
		deallocate_object(*arr++);
		len--;
	}
}

int deallocate_sequence(sequence seq)
{
	// if returns false (0), then don't free variable: "seq"
	switch (SEQ_TYPE(seq.type_and_length))
	{
	case EU_SEQUENCE:
		if (seq.myarray)
		{
			integer i = get_integer_from_object(*seq.myarray);
			deallocate_object_array(seq.myarray + 1, GET_LENGTH(seq.type_and_length));
			if (i < 0) // is_prepend
			{
				seq.myarray += i; // adds a negative number "i"
			}
			myfree(seq.myarray);
		}
		break;
	case EU_REPEAT:
		free_object(seq.myarray);
		break;
	case EU_DUPLICATE:
		if (GET_LENGTH(seq.type_and_length) > 0)
		{
			seq.type_and_length--;
			return 0; // don't free the pointer.
		}
		free_sequence(seq.dup);
		break;
	case EU_BYTES:
		myfree(seq.bytes);
		break;
	default:
		break;
	}
	return 1; // must use myfree() on the pointer.
}

void free_sequence(sequence* seq)
{
	if (deallocate_sequence(*seq))
	{
		myfree(seq);
	}
}

void deallocate_object(object ob)
{
	if (IS_NOT_INTEGER(ob.data))
	{
#ifdef DEBUG
		atom* atom_ptr;
		uint64_t* pointer_ptr;
#endif
		void* ptr = (void*)GET_PTR(ob.data);
		switch (GET_TYPE(ob.data))
		{
		case IS_ATOM:
			// free atom (double or long double)
#ifdef DEBUG
			atom_ptr = (atom*)ptr;
#endif
			myfree((atom*)ptr);
			break;
#ifdef BITS64
		case IS_POINTER:
#ifdef DEBUG
			pointer_ptr = (uint64_t*)ptr;
#endif
			myfree((uint64_t*)ptr);
			break;
#endif
		case IS_SEQ:
			free_sequence((sequence*)ptr);
			break;
		default:
			break;
		}
	}
}

void free_object(object* ob)
{
	if (ob)
	{
		deallocate_object(*ob);
		myfree(ob);
	}
}

// End of free and deallocate functions.

// Utility (Data) functions:

void set_object_to_atom(object* ob, atom num)
{
	ob->atom_ptr = (atom*)mymalloc(sizeof(atom));
	*ob->atom_ptr = num;
	ENCODE_PTR(ob->data);
	ob->data |= IS_ATOM;
}

void set_object_to_pointer(object* ob, size_t num)
{
#ifdef BITS64
	ob->unsigned_ptr = (uint64_t*)mymalloc(sizeof(uint64_t));
	*ob->unsigned_ptr = (uint64_t)num;
	ENCODE_PTR(ob->data);
	ob->data |= IS_POINTER;
#else
	set_object_to_atom(ob, (atom)num);
#endif
}

void set_object_to_integer(object* ob, integer intpart)
{
#ifdef BITS64
	if ((intpart <= 0x3FFFFFFFFFFFFFFFll) && (intpart >= -0x4000000000000000ll))
	{
		ob->data = (uint64_t)(intpart + 0x4000000000000000ll);
	}
#else
	if ((intpart <= 0x3FFFFFFF) && (intpart >= -0x40000000))
	{
		ob->data = (uint32_t)(intpart + 0x40000000);
	}
#endif
	else
	{
		error(34); // 34. Result too large
	}
}

atom get_atom_from_object(object ob)
{
	if (IS_NOT_INTEGER(ob.data))
	{
		switch (GET_TYPE(ob.data))
		{
		case IS_ATOM:
			return *(atom*)GET_PTR(ob.data);
			break;
#ifdef BITS64
		case IS_POINTER:
			return (atom)get_pointer_from_object(ob);
			break;
#endif
		default:
			error(22); // 22. Invalid argument
			break;
		}
	}
	else
	{
		return (atom)get_integer_from_object(ob);
	}
}

size_t get_pointer_from_object(object ob)
{
#ifdef BITS64
	if (IS_NOT_INTEGER(ob.data))
	{
		if (GET_TYPE(ob.data) == IS_POINTER)
		{
			return *(size_t*)GET_PTR(ob.data);
		}
	}
#endif
	return (size_t)get_atom_from_object(ob);
}

integer get_integer_from_object(object ob)
{
#ifdef BITS64
	uint64_t u = ob.data;
	if (IS_NOT_INTEGER(u))
	{
		switch (GET_TYPE(u))
		{
		case IS_POINTER:
			u = *(uint64_t*)GET_PTR(u);
			if (u > 0x3FFFFFFFFFFFFFFFull)
			{
				// errorlevel
				errno = EDOM; // 33. Domain error
				u &= 0x3FFFFFFFFFFFFFFFull;
			}
			return (integer)u;
			break;
		case IS_ATOM:
		{
			long double intpart;
			integer ret;
			modfl(*(long double*)GET_PTR(u), &intpart); // use modf() on double
			ret = (integer)intpart;
			//if ((intpart <= 0x3FFFFFFF) && (intpart >= -0x40000000))
			if (ret > 0x3FFFFFFFFFFFFFFFll)
			{
				// errorlevel
				errno = EDOM; // 33. Domain error
				ret &= 0x3FFFFFFFFFFFFFFFll;
			}
			if (ret < -0x4000000000000000ll)
			{
				// errorlevel
				errno = EDOM; // 33. Domain error
				ret %= -0x4000000000000000ll;
			}
			return ret;
			break;
		}
		default:
			error(22); // 22. Invalid argument
			break;
		}
	}
	return ((integer)u) - 0x4000000000000000LL;
#else
	uint32_t u;
	u = ob.data;
	if (IS_NOT_INTEGER(u))
	{
		if (GET_TYPE(u) == IS_SEQ)
		{
			error(22); // 22. Invalid argument
		}
		// Must be an atom:
		double intpart;
		integer ret;
		modf(*(double*)GET_PTR(u), &intpart); // use modf() on double
		ret = (integer)intpart;
		//if ((intpart <= 0x3FFFFFFFFFFFFFFFll) && (intpart >= -0x4000000000000000ll))
		if (ret > 0x3FFFFFFF)
		{
			// errorlevel
			errno = EDOM; // 33. Domain error
			ret &= 0x3FFFFFFF;
		}
		if (ret < -0x40000000)
		{
			// errorlevel
			errno = EDOM; // 33. Domain error
			ret %= -0x40000000;
		}
		return ret;
	}
	return ((integer)u) - 0x40000000;
#endif
}


char get_char_from_object(object ob)
{
	return (char)get_integer_from_object(ob);
}

char* sequence_to_string(sequence seq)
{
	sequence* ptr = SEQ_TYPE(seq.type_and_length) == EU_DUPLICATE ? seq.dup : &seq;
	size_t count = GET_LENGTH(ptr->type_and_length);
	char* str = (char*)mycalloc(count + 1, sizeof(char));
	if (count != 0)
	{
		if (SEQ_TYPE(ptr->type_and_length) == EU_BYTES)
		{
			memcpy(str, ptr->bytes, count + 1);
		}
		else
		{
			char* pch = str;
			object* iter = ptr->myarray;
			switch (SEQ_TYPE(ptr->type_and_length))
			{
			case EU_SEQUENCE:
				iter++;
				while (count > 0)
				{
					*pch++ = get_char_from_object(*iter++); // unsigned char
					count--;
				}
				*pch = '\0';
				break;
			case EU_REPEAT:
				memset(str, get_char_from_object(*iter), count);
				*(str + count) = '\0';
				break;
			default:
				break;
			}
			// put a zero (0) at the end of "str"
		}
	}
	return str;
}

sequence string_to_sequence(char* str)
{
	size_t len = strlen(str);
	unsigned char* mem_address = (unsigned char*)mymalloc(len + 1);
	sequence seq = { .type_and_length = len | EU_BYTES,.bytes = mem_address };
	memcpy(mem_address, str, len + 1);
	return seq;
}

sequence get_sequence_from_array_of_strings(size_t len, char* args[])
{
	object* arr = (object*)mycalloc(len + 1, sizeof(object));
	sequence seq = { .type_and_length = len | EU_SEQUENCE,.myarray = arr };
	set_object_to_integer(arr++, 0);
	while (len > 0)
	{
		arr->seq_ptr = (sequence*)mymalloc(sizeof(sequence));
		*arr->seq_ptr = string_to_sequence(*args++);
		ENCODE_PTR(arr->data);
		arr->data |= IS_SEQ;
		arr++;
		len--;
	}
	return seq;
}

void make_object_into_dup(object* a)
{
	// assuming object is a sequence.
	// make object into dup.
	if (IS_NOT_INTEGER(a->data))
	{
		switch (GET_TYPE(a->data))
		{
		case IS_ATOM:
		{
			atom* b = (atom*)GET_PTR(a->atom_ptr);
			atom* c = (atom*)mymalloc(sizeof(atom));
			memcpy(c, b, sizeof(atom));
			a->atom_ptr = c;
			ENCODE_PTR(a->data);
			a->data |= IS_ATOM;
			break;
		}
#ifdef BITS64
		case IS_POINTER:
		{
			uint64_t* b = (uint64_t*)GET_PTR(a->unsigned_ptr);
			uint64_t* c = (uint64_t*)mymalloc(sizeof(uint64_t));
			memcpy(c, b, sizeof(uint64_t));
			a->unsigned_ptr = c;
			ENCODE_PTR(a->data);
			a->data |= IS_POINTER;
			break;
		}
#endif
		case IS_SEQ:
		{
			sequence* b = (sequence*)GET_PTR(a->seq_ptr);
			if (SEQ_TYPE(b->type_and_length) != EU_DUPLICATE)
			{
				sequence* c = (sequence*)mycalloc(1, sizeof(sequence));
				c->type_and_length = 1 | EU_DUPLICATE;
				c->dup = b;
				a->seq_ptr = c;
				ENCODE_PTR(a->data);
				a->data |= IS_SEQ;
 			}
			else
			{
				b->type_and_length++;
			}
			break;
		}
		default:
			break;
		}
	}
}

object eu_at_with_options(sequence seq, integer index, bool make_dup)
{
	sequence* ptr = SEQ_TYPE(seq.type_and_length) == EU_DUPLICATE ? seq.dup : &seq;
	object a;
	if ((index > (integer)GET_LENGTH(ptr->type_and_length)) || (index <= 0))
	{
		error(1); // Operation not permitted: index is outside seq's length
	}
	if (SEQ_TYPE(ptr->type_and_length) == EU_BYTES)
	{
		set_object_to_integer(&a, (integer)ptr->bytes[index - 1]); // bytes[0] is NOT reserved
	}
	else
	{
		size_t type = SEQ_TYPE(ptr->type_and_length);
		if (type == EU_SEQUENCE)
		{
			a = ptr->myarray[index]; // element[0] is reserved
		}
		else if (type == EU_REPEAT)
		{
			a = *ptr->myarray;
		}
		else
		{
			error(22); // 22. Invalid argument
		}
		if (make_dup)
		{
			// Allows sequence to be written to,
			// and you have to deallocate it:
			make_object_into_dup(&a);
		}
	}
	return a;
}

object eu_at(sequence seq, integer index)
{
	return eu_at_with_options(seq, index, 1);
}

const object eu_at_readonly(const sequence seq, integer index)
{
	return eu_at_with_options(seq, index, 0);
}

sequence seq_soft_copy(sequence seq, int either_pend)
{
	sequence* ptr = SEQ_TYPE(seq.type_and_length) == EU_DUPLICATE ? seq.dup : &seq;
	size_t i, len = GET_LENGTH(ptr->type_and_length);
	object* arr = (object*)mycalloc(len + 1 + abs(either_pend), sizeof(object));
	sequence s;
	s.type_and_length = len | EU_SEQUENCE;
	if (either_pend < 0)
	{
		arr -= either_pend; // subtracts a negative number, actually adds its absolute value.
	}
	s.myarray = arr;
	set_object_to_integer(arr++, either_pend);
	for (i = 1; i <= len; i++)
	{
		*arr++ = eu_at(seq, i);
	}
	return s;
}

object obj_soft_copy(object ob)
{
	// NOTE: call this function for all toplevel objects.
	if (!IS_NOT_INTEGER(ob.data))
	{
		return ob;
	}
	switch (GET_TYPE(ob.data))
	{
	case IS_ATOM:
	{
		atom* src = (atom*)GET_PTR(ob.atom_ptr);
		ob.atom_ptr = (atom*)mymalloc(sizeof(atom));
		memcpy(ob.atom_ptr, src, sizeof(atom));
		ENCODE_PTR(ob.data);
		ob.data |= IS_ATOM;
		return ob;
		break;
	}
#ifdef BITS64
	case IS_POINTER:
	{
		uint64_t* src = (uint64_t*)GET_PTR(ob.atom_ptr);
		ob.unsigned_ptr = (uint64_t*)mymalloc(sizeof(uint64_t));
		memcpy(ob.unsigned_ptr, src, sizeof(uint64_t));
		ENCODE_PTR(ob.data);
		ob.data |= IS_POINTER;
		return ob;
		break;
	}
#endif
	case IS_SEQ:
	{
		sequence* s = (sequence*)mymalloc(sizeof(sequence));
		sequence* seq = (sequence*)GET_PTR(ob.seq_ptr);
		sequence* ptr = SEQ_TYPE(seq->type_and_length) == EU_DUPLICATE ? seq->dup : seq;
		size_t i, len = GET_LENGTH(ptr->type_and_length);
		switch (SEQ_TYPE(ptr->type_and_length))
		{
		case EU_SEQUENCE:
		{
			object* iter, * arr = (object*)mycalloc(len + 1, sizeof(object));
			s->myarray = arr;
			iter = ptr->myarray + 1;
			set_object_to_integer(arr++, 0);
			for (i = 1; i <= len; i++)
			{
				make_object_into_dup(iter); // element[0] is reserved);
				*arr++ = *iter++;
			}
			s->type_and_length = len | EU_SEQUENCE;
			break;
		}
		case EU_REPEAT:
		{
			object* arr = (object*)mycalloc(1, sizeof(object));
			*arr = *ptr->myarray;
			make_object_into_dup(arr);
			s->myarray = arr;
			s->type_and_length = len | EU_REPEAT;
			break;
		}
		case EU_BYTES:
		{
			unsigned char* mem_addr = (unsigned char*)mymalloc(len + 1);
			memcpy(mem_addr, ptr->bytes, len + 1); // copy "null" terminating byte
			s->type_and_length = len | EU_BYTES;
			s->bytes = mem_addr;
			break;
		}
		default:
			error(22); // 22. Invalid argument
			break;
		}
		ob.seq_ptr = s;
		ENCODE_PTR(ob.data);
		ob.data |= IS_SEQ;
		return ob;
		break;
	}
	default:
		error(22); // 22. Invalid argument
		break;
	}
}

void eu_append_to_self(sequence** seq, object x)
{
	sequence* tmp = *seq;
	object* ptr;
	bool flag = (SEQ_TYPE(tmp->type_and_length) != EU_SEQUENCE)
		|| (get_integer_from_object(*tmp->myarray) <= 0);
	if (flag)
	{
		*seq = (sequence*)mymalloc(sizeof(sequence));
		**seq = seq_soft_copy(*tmp, CHUNK); // leave in nested duplicates
	}
	ptr = &(*seq)->myarray[GET_LENGTH(++(*seq)->type_and_length)];
	//eu_print(x);
	*ptr = obj_soft_copy(x);
	//eu_print(*ptr);
	(*seq)->myarray->data--;
	//eu_print_seq(**seq);
	if (flag)
	{
		free_sequence(tmp);
	}
	//eu_print_seq(**seq);
}

// Begin "eu" prefix functions:

void eu_cleanup(void)
{
	deallocate_sequence(eu_files);
}

void eu_init(int myargc, char** myargv)
{
	//sequence* test;

	size_t len = 2;
	object* arr = (object*)mycalloc(len + 1, sizeof(object));
	sequence seq = { .type_and_length = len | EU_SEQUENCE,.myarray = arr };
	set_object_to_integer(arr++, 0);
	set_object_to_pointer(arr++, (size_t)stdout);
	set_object_to_pointer(arr, (size_t)stderr);
	
	// initialize file numbers (sequence of fn's), and register: eu_cleanup()
	eu_files = seq;
	atexit(eu_cleanup);

	// Test code:
/*	eu_print_seq(eu_files);
	eu_print(eu_at_readonly(eu_files, 1));

	test = (sequence*)mymalloc(sizeof(sequence));
	*test = seq_soft_copy(eu_files, 0);

	eu_append_to_self(&test, eu_at_readonly(eu_files, 1));
	eu_print_seq(*test);
	
	free_sequence(test);
*/
	// variables for command_line()
	c_argc = myargc;
	c_argv = myargv;
}

// command_line()

sequence eu_command_line()
{
	sequence ret = get_sequence_from_array_of_strings(c_argc, c_argv);

	return ret;
}

integer eu_length(sequence seq)
{
	sequence* ptr = SEQ_TYPE(seq.type_and_length) == EU_DUPLICATE ? seq.dup : &seq;

	return GET_LENGTH(ptr->type_and_length);
}

void c_puts(FILE* fn, char* str)
{
	fputs(str, fn);
}

void eu_puts(integer fn, object st)
{
	FILE* file;
	if (fn < 1)
	{
		error(22); // 22. Invalid argument
	}

	file = (FILE*)get_pointer_from_object(eu_at_readonly(eu_files, fn));

	if (GET_TYPE(st.data) == IS_SEQ)
	{
		char* str = sequence_to_string(*(sequence*)GET_PTR(st.seq_ptr));
		c_puts(file, str);
		myfree(str);
	}
	else
	{
		fputc(get_char_from_object(st), file);
	}
}

bool eu_sequences_equal(sequence* a, sequence* b)
{
	integer i, len;
	a = SEQ_TYPE(a->type_and_length) == EU_DUPLICATE ? a->dup : a;
	b = SEQ_TYPE(b->type_and_length) == EU_DUPLICATE ? b->dup : b;
	if (a == b)
	{
		return true;
	}
	len = GET_LENGTH(a->type_and_length);
	if (len != GET_LENGTH(b->type_and_length))
	{
		return false;
	}
	for (i = 1; i <= len; i++)
	{
		if (!eu_equal(eu_at_readonly(*a, i), eu_at_readonly(*b, i)))
		{
			return false;
		}
	}
	return true;
}

bool eu_equal(object a, object b)
{
	if (GET_TYPE(a.data) == IS_SEQ)
	{
		if (GET_TYPE(b.data) == IS_SEQ)
		{
			// both are sequences
			return eu_sequences_equal((sequence*)GET_PTR(a.seq_ptr), (sequence*)GET_PTR(b.seq_ptr));
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (GET_TYPE(b.data) == IS_SEQ)
		{
			return false;
		}
	}
	// neither are sequences.
	if ((!IS_NOT_INTEGER(a.data)) && (!IS_NOT_INTEGER(b.data)))
	{
		return a.data == b.data;
	}
#ifdef BITS64
	if ((GET_TYPE(a.data) == IS_POINTER) && (GET_TYPE(b.data) == IS_POINTER))
	{
		return *(size_t*)GET_PTR(a.unsigned_ptr) == *(size_t*)GET_PTR(b.unsigned_ptr);
	}
#endif
	return get_atom_from_object(a) == get_atom_from_object(b);
}

integer eu_find(object x, sequence* s, integer i, integer stop_at)
{
	integer len;
	s = SEQ_TYPE(s->type_and_length) == EU_DUPLICATE ? s->dup : s;
	if (stop_at)
	{
		len = stop_at;
	}
	else
	{
		len = GET_LENGTH(s->type_and_length);
	}
	while (i <= len)
	{
		if (eu_equal(x, eu_at_readonly(*s, i)))
		{
			return i;
		}
		i++;
	}
	return 0;
}

integer eu_match(sequence* st, sequence* s, integer i)
{
	integer len, st_len, f, j, k;
	bool found;
	st = SEQ_TYPE(st->type_and_length) == EU_DUPLICATE ? st->dup : st;
	st_len = GET_LENGTH(st->type_and_length);
	if (st_len == 0)
	{
		error(22); // 22. Invalid argument
	}
	if (i <= 0)
	{
		error(22); // 22. Invalid argument
	}
	s = SEQ_TYPE(s->type_and_length) == EU_DUPLICATE ? s->dup : s;
	len = GET_LENGTH(s->type_and_length);
	if (len < i - 1)
	{
		error(22); // 22. Invalid argument
	}
	if (len < i - 1 + st_len)
	{
		return 0; // length of 's' is not large enough to have a slice of 'st'
	}
	do {
		f = eu_find(eu_at_readonly(*st, 1), s, i, len - (st_len - 1));
		if (f)
		{
			if (len - f < st_len - 1)
			{
				return 0;
			}
			found = true;
			k = f;
			for (j = 2; j <= st_len; j++)
			{
				k++;
				if (!eu_equal(eu_at_readonly(*st, j), eu_at_readonly(*s, k)))
				{
					found = false;
					break;
				}
			}
			if (found)
			{
				return f; // found slice in sequence, at this position
			}
		}
		i = f + 1;
	} while (f);
	return 0;
}
/*
with trace
trace(1)

function eu_find(object x, sequence st, integer start, integer stop)
	integer f
	f = find(x, st[start..stop])
	if f then
		return f + start - 1
	end if
	return f
end function

function eu_match(sequence st, sequence s, integer i)
	integer len, st_len, f, k, found
	st_len = length(st)
	if st_len = 0 then
		abort(22)
	end if
	if i <= 0 then
		abort(22)
	end if
	len = length(s)
	if len < i - 1 then
		abort(22)
	end if
	if len < i - 1 + st_len then
		return 0
	end if
	f = 1
	while f do
		f = eu_find(st[1], s, i, len - (st_len - 1))
		if f then
			if len - f < st_len - 1 then
				return 0
			end if
			found = 1
			k = f
			for j = 2 to st_len do
				k += 1
				if not equal(st[j], s[k]) then
					found = 0
					exit
				end if
			end for
			if found then
				return f
			end if
		end if
		i = f + 1
	end while
	return 0
end function

? match("ab", "abcabcab", 2)
? eu_match("ab", "abcabcab", 2)
*/
