//
// Eulang's "Object" class
//
// Front-End in C++
//
//
// Reference websites:
// http://www.cplusplus.com/doc/tutorial/classes2/
//

#pragma once

extern "C" {
#include "pch.h"
#include "eu_functions.h"
}

// Logical "xor" operator missing from C++, the macro for it is:
#define XOR(a,b) ((not (a and b)) and (a or b))

// NOTE: C++ front end.
class Object
{
public:
	object obj;
	Object(); // Default constructor
	~Object(); // Destructor
	Object(Object&); // Copy constructor
	Object& operator= (Object& x); // Copy assignment
	Object(Object&&) noexcept; // move-constructor
	Object& operator= (Object&&) noexcept; // move-assignment

	// My functions:
	bool IsAtom() { return IS_NOT_INTEGER(obj.data) && (GET_TYPE(obj.data) == IS_ATOM); }
	bool IsInteger() { return !IS_NOT_INTEGER(obj.data); }
	bool IsPointer() {
#ifdef BITS64
		return IS_NOT_INTEGER(obj.data) && (GET_TYPE(obj.data) == IS_POINTER);
#else
		return IsAtom();
#endif
	}
	bool IsSequence() { return IS_NOT_INTEGER(obj.data) && (GET_TYPE(obj.data) == IS_SEQ); }
	bool IsObject() { return true; }

	void SetToAtom(atom num) { set_object_to_atom(&obj, num); }
	void SetToInteger(integer intpart) { set_object_to_integer(&obj, intpart); }
	void SetToPointer(size_t num) { set_object_to_pointer(&obj, num); }

	atom GetAtom() { return get_atom_from_object(obj); }
	integer GetInteger() { return get_integer_from_object(obj); }
	size_t GetPointer() { return get_pointer_from_object(obj); }

	// The usual arithmetic operators are available : add, subtract, multiply, divide, unary minus, unary plus.

	// operators

// Operators:
// +a, -a, !a, ++a, --a

	Object operator-()
	{
		//INCOMPLETE: put code here
		return *this;
	}
	Object operator!()
	{
		//INCOMPLETE: put code here
		return *this;
	}
	Object operator++()
	{
		//INCOMPLETE: put code here
		return *this;
	}
	Object operator--()
	{
		//INCOMPLETE: put code here
		return *this;
	}

	// Operators:
	// a++, a--

	Object operator++(int)
	{
		//INCOMPLETE: put code here
		return *this;
	}
	Object operator--(int)
	{
		//INCOMPLETE: put code here
		return *this;
	}

	// Operators:
	// a@b: + - * / &(for concatenation) < > == != <= >= && ||

	Object operator + (const Object&);
	Object operator - (const Object&);
	Object operator * (const Object&);
	Object operator / (const Object&);
	// relational operators:
	// <   >   <=   >= == != each produce a 1 (true) or a 0 (false) result
	bool operator < (const Object&);
	bool operator > (const Object&);
	bool operator == (const Object&);
	bool operator != (const Object&);
	bool operator <= (const Object&);
	bool operator >= (const Object&);
	bool operator && (const Object&);
	bool operator || (const Object&);

	// Operators:
	// operator assignment: a@b: = += -= *= /= &= []

	Object operator += (const Object&);
	Object operator -= (const Object&);
	Object operator *= (const Object&);
	Object operator /= (const Object&);
	Object operator &= (const Object&);

	// Operators:
	// operator size_t()
	// operator integer()
	// operator atom()
	// operator sequence()
	// operator object()
	// operator char()
	// operator cstring() // null terminating

	operator size_t();
	operator integer();
	operator atom();
	operator sequence();
	operator object();
	operator char();
	operator cstring();


	// logical operators: and, or, xor, and not
	bool And (const Object& x);
	bool Or (const Object& x);
	bool Xor (const Object& x);
	bool Not (void);


	// operations on sequences

	// equal(), compare()
	bool Equal(const Object& x);
	int Compare(const Object& x);

	// subscripting of sequences [1], [$]
	Object operator [] (const Object&);

	// slicing of sequences [1..$]
	Object Slice(integer start, integer stop);

	// Concatenation of Sequences and Atoms
	Object operator & (const Object&); // NOTE: for concatenation

	// sequence formation {a, b, {c}}
	Object operator() (Object, Object...);

	// length(), repeat(), append(), prepend(), reverse()
	integer Length(void);
	Object Repeat(integer count);
	void Append(Object x);
	void Prepend(Object x);
	Object Reverse(void);



	//void test()
	//{
	//}

};


inline Object::Object()
{
	obj.atom_ptr = nullptr; // or NULL
}

inline Object::~Object()
{
	deallocate_object(obj);
}

inline Object::Object(Object& ob)
{
	obj = obj_soft_copy(ob.obj);
}

inline Object& Object::operator=(Object& ob)
{
	obj = obj_soft_copy(ob.obj);
	return *this;
}

inline Object::Object(Object&& ob) noexcept
{
	obj = ob.obj;
	ob.obj.atom_ptr = nullptr;
}

inline Object& Object::operator=(Object&& ob) noexcept
{
	deallocate_object(obj);
	obj = ob.obj;
	ob.obj.atom_ptr = nullptr;
	return *this;
}

inline Object Object::operator+(const Object&)
{
	//INCOMPLETE:
	return Object();
}

