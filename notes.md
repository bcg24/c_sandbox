# Chapter 1

## Preprocessor Statements

Statements like **#include <stdio.h>** let the pre-processor, which only operates on text, to paste the code that is contained in the file in the "<>" in the file. This is how you use libraries

It can also be used for contant definitions like:
**#define PI 22.0/7.0**
What this is doing in the preprocessor is replacing every textual use of PI with the *characters* 22.0/7.0, which then compiles as you'd expect. It's normal to see these constants written in all caps, though not strictly necessary. These are always declared in a single line

## Hosted Environment Conventions

External linkage means the function/variable is global; you can use it throughout the program, e.g. library functions

A hosted environment is one with the standard libraries. In a hosted environment, **main()** is a reserved keyword for the main funcion of a program. 

## 

A compound statement is a way of linking together multiple statements, which is created with **{}**, effectively turning them into one statement. You'll often see these after statements like while() or if() if there is more than one statement that should execute in a given condition, or to clear up the following ambiguity with one statement:

## If and While

One thing that's a bit weird is the fact that in:

if(1 > 0)
    if(1 < 0)
        statement1
else
    statement2

statement2 *will* execute. That's because the else statement pairs with the nearest preceeding **if()**. If we want to  code the expected behavior, we need to use a compound statement, for example:

if(1 > 0){
    if(1 < 0)
        statement1
}
else
    statement2

## Function and Variable Declaration

Before you use a function or variable, you have to tell the compiler that you plan to use it.

For a function example:

void showMessage(void);

Lets the compiler know that you plan to use a function of type **void** that returns a value **void**, so a function that takes no argument and returns no output. The use of the parentheses **()** let's the compiler know that it's a function

For a variable:

int count;

## Strings

A string is a single element, like and identifier, and therefore must be a single line:

"This is NOT
a valid string"

Everywhere in C, backslash end-of-line totally dissapears:

"So this \
works"

"but also"
"does this"

Only trigraphs are processed beforehand.

**printf** works the exact same way as MATLAB's fprintf.

%d: int
%c: char
%f: float

## Function Returns

By default, a function returns type int, but variables do not have a default type. Returning a value of 0 means that the function ran successfully. Because this is confusing sometimes, EXIT_FAILURE and EXIT_SUCCESS from <stdlib.h> also works

When the main loop returns, it's the same as the **exit()** command, except exit() can be called from anywhere

## Arithmetic Operations

Division is integer and truncates if both operands are int, otherwise normal. Modulo **%** returns the integer truncated by division

## Input

Simplest method is **getchar**, which reads single chars from the input and returns and integer. These integer representations change from system to system so the only thing they are useful for is direct comparison. The only thing that can be reliably infered is that the integer representations of the digits will be consecutive

## Arrays

When declaring, use the size, but they are 0-indexed

int testArray[5];

firstElement = testArray[0];

## Objects

Things that hold values and functions, both 'objects'

# Chapter 2

## Alphabet

Trigraphs are ways of representing some of the symbols that are not presnet in more limited alphabets. 

# ??=
[ ??(
] ??)
{ ??<
} ??>
\ ??/
| ??!
~ ??-
^ ??'

If for some reason you typed ???=, only the pair of question marks to the right form the trigraph. 

C is byte-oriented, which means that the smallest unit of storage that you can use in C is 8 bits. 

A byte whos value is zero is always a null character regardless of character set. 

If you want to use Greek or other charater sets in your program, you have to use the wchar_t type, as other character sets use more than the 8 bits in the char type. You have to include the <stddef.h> header in order to use this type.

## Textual Structure

Space (including tab) can be used for indentation anywhere except in identifies or keywords. Newlines work the same way everywhere except preprocessor command lines, which have a line-by-line structure.

Again, to reiterate, newlines are not allowed in strings or character constants. Therefore:

printf("This is a very very very \
long string\n");

can NEVER be:

printf("This is a very very very \
    long string\n");

Because it puts a tab character in the string, so it's best to use the string joining feature:

printf("This is a very very very"
    "very long string\n");

## Comments

/* This is how you right a comment */

and /**/ becomes a space.

## Translation Phases

The most important steps of processing occur in the following order:

1. Trigraph translation
2. Line joining
3. Translate comment to space (but not in strings or character constants). Multiple whitespaces may be condensed (i.e. tab may become space).
4. Translate the program

## Keywords and Identifiers

Keywords cannot be used as identifiers, assuming this means mostly variable and function names. None of them have capital letters in them.

auto        double  int         struct
break       else    long        switch
case        enum    register    typedef
char        extern  return      union
const       float   short       unsigned
continue    for     signed      void
default     goto    sizeof      volatile
do          if      static      while

## Identifiers

I guess I missed a few identifiers. The others are labels, and the 'tags' of structures, unions, and enums. Identifiers can have numbers, letters, and underscore, but must start with a letter. Identifiers can be more than 31 characters, but only the first 31 are checked for uniqueness -- used <=31 for portability or make sure your long names differ in the first 31 characters.

For the names of external libraries, only the first 6 characters are checked, and capitalization is ignored. Some systems may check for more but it's recommended that you account for 6 character uniqueness in external identifiers for portability.

## Declaration of Variables

The only thing that you don't need to name before using are functions that return int. Declarations introduce the name and type but allocates no storage. A definition differs by allocating space. 

variables declared inside functions are definitions unless 'extern' is used

## Exercises

2.1 Trigraphs are used when you are on a system that doesn't have the whole C alphabet.

2.2 You WOULDN'T use them to make your programs more portable; it's easier just to create a text preprocessor. You would expect to use them on a terminal that has a limited character set

2.3 A newline is not equivalent to a space or tab when ...

2.4 A backslash newline sequence can be used for string folding though its not best practice.

2.5 When two strings are put side-by-side, the string folding feature concatenates them into one string

2.6 Comments can't go inside other ones because the first pair of /**/ closes the comment

2.7 The longest names that can be safely used for variables are unique in the first 31 characters.

2.8 A declaration declares the name and type of an object

2.9 A definition declares the name, type, and allocates space for an object

## 2.6 Real Types

The real numbers are float, double, and long double. If the system has more or less capability, then they might all be the same, and C has no way of accessing more. float is supoosed to be small and fast (single), double gives more and long double even more precision. They might be treated the same arithmetically depending on the system, but different types and treated as such. C allows expressions to mix all of the scalar types, including pointers. There are implicit rules for arithmetic conversion that dictate what the output type will be, known as the *usual arithmetic conversions*.

Mixing the real types, when an object is defined as a type, it will cast the value to the specified type. When a lower precision type is in an arithmetic operation with a higher-precision type, the lower prec. object is converted to the higher one. When we put a high precision representation to a lower destination, that behavior is undefined -- you must specify how to round or truncate.

## Exercises

2.10 I assume the long double can hold the largest range of values

2.11 The long double is definitely the type of variable that can store values with the greatest precision.

2.12 There's no problem when assigning a lower precision value to a higher precision destination; the conversion is done implicitly.

2.13 Assigning a higher precision represenation to a lower precision destination results in undefined behavior; you should specify exactly how you want the value truncated or rounded.

2.14 There are no guarantees about how a program with undefined behavior will operate.

## Integral Types

C doesn't have special types for character manipulation or booleans; both are integral types

1. Plain Integers

The two flavours of undecorated integers are the 'signed' int and the unsigned int. There is no guarantee as to the number of bits over 16 that an int can hold, but they work for applications like counters in short loops. <limits.h> details the maximum and minimum values (+-32767) and (0 65535) for signed and unsigned respectively. You HAVE to be thinking about the potential portability and safety of the program.

## Character Variables

char is basically another sort of int, but characters do not take up nearly the same amount of range. char has at least 8 bits and can store a value of at least +127, and the minimum value is less than or equal to 0, so the guaranteed range is 0-127. Whether or not char is treated as signed or unsigned int is implementation defined. As a result, it can be used for arithmetic operation.

Character constants are formed by placing a character in single quotes, e.g. 'x'. Because it will always be implicitly converted to a higher precision representation, the constant turns into an int gut the value will be small enought to assign to char. 

The only reason that getchar() returns an int is because it also has to hold the newline character, which depending on the implementation might be out of range of char.

## Other Types

1. signed and unsigned ints

unsigned types have the advantage that they give an extra bit of precision and never overflow; adding one to the maximum value results in the (maximum unsigned integer value + 1) % (maximum unsigned integer value). Other types have undefined behavior when they overflow. 

Default behavior is signed unless otherwise specified. If int is dropped, int is implicit. So long specifies a signed long integer.

You can use char as extra short shorts to save space, in which case you can choose signed or signed, but the default sign of the char is implementation defined.

All of the integral types can be printed a signed decimal numbers with %d or %ld for long. The behavior of printf is undefined if the wrong format is given.

%c          char
%d          decimal signed int, short, char
%u          decimal unsigned int, unsigned short, unsigned char
%x          hexidecimal int, short, char
%o          octal int, short, char
%ld         decimal signed long
%lu %lx %lo as above but for long

## 2.8 Expressions and Arithmetic

Operators are operations like =, +, * and -. The operand are the variables involved. Most operators in C are either binary and unary, which take two or one operands respectively. Some operators like * have binary and urnary forms, one meaning multiplication, the other denoting a pointer.

Operators may appear consecutively, e.g. x = a+b*(-c); is equivalent to x = a+b*-c; 

## Conversions

Types can be mixed with conversion happening implicitly. Aside: integral and floaing types are reffered to as arithmetic types, but there are others such as pointers. 

Conversions in arithmetic expressions:

1. Integral promotions
2. Conversions btwn integral types
3. Conversions btwn floating types (already discussed)
4. Conversions between floating and integral types

Value preserving rules - you have to know about the target to figure what the type of an expression will be. 

1. Integral promotions

No arithmetic is performed below int precision, Therefore, with a short or char, if an int can hold all of the values of the original type, then the value will be converted to int, otherwise an unsigned int. Therefore preserves the value and sign. 



...


5. Wide Characters

Shift-in shift-out doesn't work that well for multibyte characters in arrays, therefore there's a type wchar_t to store these extended characters, which is an integral type. 

1. wchar_t can hold all of the extended set with a unique value
2. It can hold the original char set with the same values
3. The null character is zero

To get strings with type wchar_t, simply prepend a "L", e.g. L"This is a string". Since strings are arrays, you cab index them as one, e.g. "This is a string"[4]

if <a> is an element of a extended character set, then in the following example:

L"abc°"[3]
"abc°"[3]

The first one returns wchar_t of c, whereas in the second one returns the type char but probably with the value of the shift-in marker

wide character constant: L'c'

If you make a multibyte character constant such as: '°', then its kinda like you wrote: 'xy' -- multiple characters in a constant because the extended character is multibyte. The wide character constant will simply be the repective wchar_t value.

##  Exercises

2.15 Assuming that chars, ints and longs are respectively 8, 16 and 32 bits long, and that char defaults to unsigned char on a given system, what is the resulting type of expressions involving the following combinations of variables, after the usual arithmetic conversions have been applied?

a.  Simply signed char.
b.  Simply unsigned char.
c.  int, unsigned int.
d.  unsigned int, long.
e.  int, unsigned long.
f.  char, long.
g.  char, float.
h.  float, float.
i.  float, long double.

6. Casts

Declaration     Cast        Type
int x;          (int)       int
float f;        (float)     float
char x[30];     (char [30]) array of char
int *ip;        (int *)     pointer to int
int (*f)();     (int (*)()) pointer to a function returning int

## Operators

1. The multiplicative operators: *, /, %

Work how you think for integral and real types, with integral division giving a rounded (towards 0) result. The remainder/modulo operator is only defined to work with integral types, becasue the division of real (float) numbers doesn't produce a remainder.

...

3. Bitwise operators

&   AND
|   OR
^   XOR
<<  Left shift
>>  Right shift
~   one's complement

Binary representation is whether or not we add the corresponding power of 2

(101)_2 = 2^2 + 2^1 + 2^0 = 4*(1) + 2(0) + 1(0) = 5_10

10_16 = 16_10
11_16 = 17_10

Every group of 4 binary digits can be converted directly from binary into hex:

(1001011100)_2 	= 	0010  	0101  	11002
  	            = 	2 	5 	C_16
  	            = 	25C_16

Double FF is going to select everything in a byte, so x >> 8 & 0xff selects everything in the second bit -- its a mask.

One's complement is the opposite of each individual bit, the two's complement is the one's complement adds a single bit. That's the negative representation of any integer. For ex, 0000 1000 = 8_10, ~8_10 = 1111 0111, 1111 0111 + 0000 0001 = 1111 1000 = -8_10 on an 8-bit system.

All ones, i.e. ~0, selects everything regardless of the number of bits. 

Bits shifted off either end simply disappear. Shifting by more than the availible bits leads to an implementation-defined behavior. Shifting left guarantees that zeros are guaranteed in the lowest bits. Shifting right has the option for logical or arithmetic shift. A logical shift shifts zeros into the most significant bits, whereas an arithmetic shift copies the current contents of the most significant bits into the zeros that would otherwise be there.

Logical:    11111011 >> 2 = 00111110
Arithmetic: 11111011 >> 2 = 11111110

A bit-shifted unsigned integer stays positive no matter what, but a bit-shifted signed integer might change from negative to positive in logical mode and remain negative in arithmetic mode. The result of the shift has the same type of the thing that got shifted (after integral promotions). 

To create some pattern, e.g. 0x0f0 of low bits, regardless of word length:

int some_variable;
some_variable = ~0xf0f;

++a and a++ are both equivalent to a = a + 1, however when used in an expression, puting the '++' first indicates that one is added, then used in the expression, whereas putting it after indicates that the old value of the variable is used in the expression, then one is added afterward. It's not safe to use these operations more than once in an iteration, as the compiler might "save up" the values until the end of the iteration.

# Exercise 2.16

a. i1 % i2
b. i1 % (int)f1
c. The sign of the divisor doesn't matter, the sign of the left-hand operand determines the sign of the result in C99, but previously implementation defined.
d. '-' has a unary form as well as a binary form, where the unary form is multiplicative complement and where the binary form is subtraction.
e. i1 &= 0xf;
f. i1 |= 0xf;
g. i1 &= ~0xf;
h. temp1 = i1 = ~0xff; temp2 = i2 & 0xff; i1 = temp1 | temp2;
i. The ++ operator is not defined for floats, as well as the fact that you're not supposed to use the increment operator more than once per iteration.

5. Side Effects

It's unsafe to use a variable more than once in an expression if the expression changes the variable and the new value could affect the result of the expression.

## Constants

You can put an 'l' or 'L' at the end of an integer to force it to be long. 'u' or 'U' will force it to be unsigned. 

If you start an integer expression with '0', you're using octals, beware.

A plain decimal constant will be fit into the first of the int long or unsigned long that will hold the value.

A hex or octal constant has the order: int, unsigned int, long, unsigned long.

If the constant is suffixed by L or U, then we get long, unsigned long, or unsigned int, unsigned long respectively.

Remember that %d is expecting an int, whos bitlength is implementation defined.

There are no negative constants; -23 is a int and an operator

Character constants have a type int and are placed in single quotes 'like this', and wide character constants are simply prefixed by L. Multicharacter constants are there so that '<SI> a <SO>' can yield a plain multicharater constant.

Escape sequences are meant to represent characters that might be hard to get in a character constant or hard to read:

Sequence Represents
\a  audible alarm
\b  backspace
\f  form feed
\n  newline
\r  carriage return
\t  tab
\v  vertical tab
\\  backslash
\'  quote
\"  double quote
\?  question mark

You can also use escape sequences to represent internal character codes in either octal or hex, e.g. \ooo or \xhhhh, where ooo is UP TO 3 octal digits and hhhh can be any number of hex digits. For ex, \033 is the code for esc on a ASCII machine. Hex is greedy, so any number of characters in that sequence will be assumed to be part of the hex value. If you want to have normal characters afterward, use string joining: "\xff" "f"

One purpose of the escap sequences is to express certain characters unambiguously. The other is to control the motion of the printing devices, which are implementation defined:

\a  Ring the bell if there is one. Do not move.
\b  Backspace.
\f  Go to the first position on the ‘next page’, whatever that may mean for theoutput device.
\n  Go to the start of the next line.
\r  Go back to the start of the current line.
\t  Go to the next horizontal tab position.
\   Go to the start of the line at the next vertical tab position.

# Real Constants

They are all double by default unless the are suffixed by f or F to indicate float or l or L so as to specify a long double.

# Excercise 2.17

[...]

## Chapter 3

do
    statement
while(expression);

This makes sure that the controlling statement is evaluated before the executed before the controlling expression is evaluated, guaranteeing at least one trip around the loop.

for(initialize; check; update) statement

The loop terminates when the check evaluates to zero.

Always use the for loop when you have some sort of counter. while is better suited when you don't know the number of cycles. Any of the initialize, check, and update expresions can be omitted, provided that the semicolons remain, e.g. for(;;) or while(1) which makes an infinite loop.

# Switch Statement

switch (expression){
    case const1:    statements
    case const2:    statements
    default:        statements
}

consts are unique, strictly integral constant expressions. That means it doesn't involve a value-changing operation, function calls or comma operators. They must all be integer, character, enumeration constants, sizeof expressions, or floaitng point constants athat are the immediate operands of casts. Any cast operation must result in integral types.

# The break Statement

Occurs in the body of a switch, do, while, or for statement. Jumsp to the statement following the body of the statement containing the break. It's basically necessary in a switch statement, or else all of the remaining cases will also be executed.

# The continue Statement

The rules are the same as for break except it doesn't apply to switch statements. Starts the next iteration of the smallest enclosing do, while, or for statement immediately. It only has any place in a switch statement if its contained in a loop.

In a while loop, a continue will go immediately to testing the controlling expression, whereas in the for loop, first the update expression is evaluated, then the controlling expression is evaluated.

# goto and labels

Jumps to the labeled statement. It is used to escape form multiple nested loops or to go to error handling at the end of a function. It needs a label:

goto L1;
/* whatever you want */
L1: /* anything else */

The name space ('linkage?') only exists for the function containing the label, so names can be repeated across functions, but can't jump from one function to another one. It can also be used before being declare by menioning it in a goto statement.

Labels must be part of a full statement. Only matters when you're trying to put a label at the end of a compound statement:

label_at_end: ; /* empty statement */
}

For example, if you want a switch statement to do nothing, since the cases are technically labels, the correct syntax would be:

switch(x){
    case 1:
        ;
    default:
        printf("Not nothing\n");
}

## More logical expressions

C makes no distinction for logical values. In a control flow, anything other than zero means 'do'. while (a) means: while(a != 0)

Don't confuse && with bitwise & (or any other bool). Every sequence of logical operators stops eval as soon as the value can be determined.

if(a&b == c){...

Remember precedence rules. b is compared for equality with c, then the reslut is anded with a.

## Strange operators

expression1?expression2:expression3

if expression 1 is true, then the result of the whole expression is expression2, otherwise it is expression3. depending on the value of expression1, only one of the expressions will be evaluated.

The types of the expressions and the whole expression.

## Exercises

1. The type and value of the result of relational values is integer.
2. The type and value of the logical operators is integer.
3. What is ...
4. Break is useful in switch statments to make them mutually exclusive.
5. Continue only goes to the next iteration from within loops.
6. When using continue in a while statment, it moght skip the counter update.
7. I am not sure, but you definitely can't use goto

## Chapter 4

# Types of Functions

What is called a procedure in other languages is simply a function that returns no value, which should be declared to return void.

Functions can't reuturn arrays and functions, but can return anything else like pointers and structures. All functions are recursive, meaning they can call themselves from inside itself.

# Declaring Functions

For a function, it turns from a declaration to a definition when you provide a function body. Within a function, the names used inside the function are its paramenter, and the arguments are what is used when the functino is called; they're the values that the formal parameters will have on entry to the function.

Parameter and argument are somewhat interchangable terminology.

Using an undeclared function is allowed, but all functions should be declared before their use in practice. 

What makes a declaration a function prototype is adding information about the type of the arguments -- you don't even need to specify the names.

varargin is declared: 

void f_name (int,double,...);

If a function is called with different argument types that its prototype, then they are converted to the types of the formal argument 'as if by assignment'.

The only time that you might not use a prototype to declare the type and number of arguments to a function is if you are using a variable number of arguments.

If there is no prototype, then the defaul promotions are applied. If the nargs differs from the formal parameters, undefined behavior. 

# Compound statements and declarations

Variables in a compound statement can have the same name as a variable outside the statement. C restricts variable declarations to the head of the compound statement -- once any other type of statementment is encountered, declarations are not allowed after it.

Scope of a nmae is the range in which it has meaning. Starts from where it is mentioned and continues from there to the end of the block in which it was declared. If it is outside any function, then its meaning persists to the end of the file. If it is inside a function, then its scope ends at the end of the block.

The scope of a name can be suspended by redeclaring in a sub-block.

main(){}
int i;
f () {}
f2() {}

f and f2 can use i, but main can't. 

The intitialization of the formal parameters is the last time any communication occurs between the caller and called function, except for the return value.

# Call by reference

A function can change a pointer, so in this way, it can change a value in the caller.

# Recursion

Each call of a function creates its own copy of the functions parameters. Including the keyword auto in the variable declaration means 'automaticaly allocated':

main(){
    auto int var_name;
}

Which means that the variables are allocated on function entry and freed on function return. The auto keyword is never declared because it is the default behavior for internal declarations and invalid externally. If a declared variable isn't given an explicit initial value, then the use of the variable before assignment leads to undefined behavior.

ungetc allows the last character read by getchar to be 'unread' and become the next character to be read.

## Linkage (p. 119)

Skipping this because the book told me to for now.

# Exercises

Skipping 2-4 for now, did 1 and 5

## Chapter 5

Array declaration: double ar[100];
Has indices 0-99

The size of an array has to be able to be figured out at compile time, not at run time, which means, for example, that you can't have a function that uses its argument as the size in a variable declaration. That doesn't mean that you can't have a integer variable as the size of the array, the value of it just has to be able to be figured out at compile time. Maybe this has been changed with more recent compilers.

# Multidimensional Arrays

int three_dee[5][4][2];
int t_d[2][3];

In the three_dee example, the hierarchy is left to right, so the first declaration is a five element array with members are each a four element array whose members are an array of two ints. 

So t_d[0] contains t_d[0][0], t_d[0][1], t_d[0][2]. Technically, you could access t_d[1][0] with t_d[0][3], because C has no array boound checking.

# 3. Pointers

The type preceding the asterisk in a pointer is in reference to the type that the pointer is pointing to, therefore:

int ar[5], *ip;

declares an array of ints and a pointer to an int, which is still uninitialized. 

ip = &ar[3];

Points to the fourth element of ar. ip is a pointer to int, and *ip refers to the thing being pointed to.

*& effectively cancel each other out. Constants dont have addresses so &1.5 is an error. What pointers do is get around the call-by-value restriction on functions. What this means is that a function doesn't have to return anything -- it can simply write to the address provided to it to make its output accessible outside the function.

It's much more common to used pointers to walk along arrays:

# Arrays and pointers

Using pointer arithmetic, we can add an integral balue to a pointer, resulting in another pointer of the same type. Adding n to an element gives a pointer which points to an elememnt n elements purther along the array that the original pointer that n was added to.

int ar[20], *ip;
for(ip = &ar[0]; ip < &ar[20]; ip++)
      *ip = 0;

Even though element 20 is out of bounds, taking its address is legal, but just by one out of bounds and no further. You must not try to access its value.

# Qualified types

Qualifiers can be applied to any declared type to modify its behaviour. If a declaration is prefixed with const, then it can't be changed or else undefined behaviour results. The benefits to doing this are to: 1. help the compiler know not to change them, even through pointers. and 2. help the compiler optimize.

Define is used a lot in array sizes because it wasn't allowed to use const in array sizes in older standards. Use a const whenever you want to debug and for type safety.

const int x = 1;        // x is constant
const float f = 3.5;    // f is constant
const char y[10];       // y is an array of 10 constant ints
                        // don't think about initializing it yet

This qualifier can be applied to pointers either as a pointer to const, or to make a constant pointer.

int i;                          // i is an ordinary int
const int ci = 1;               // ci is a constant int
int *pi;                        // pi is a pointer to an int
const int *pci                  // pc is a pointer to a constant int
int *const cpi = &i;            // cpi is a constant pointer to an int
const int *const cpci = &ci;    // cpci is a constant pointer to a constant int

Can't intermix a pointer to a int and a pointer to a constant int. In both, one can change the value of the pointer so that they point to other things, as long as those things are ints and constant ints respectively, but in the case of the pointer to the constant int, you are only allowed to inspect the value that it points to. If the pointers themselves are constant, as in the latter two examples, then you can't change the thing that they point to, that's the whole point, so they need to be initialized. In these examples, a non-constant pointer to a constant is itself not a qualified type, only ci, cpi, and cpci are qualified types.

# Pointer arithmetic

Pointers of the same type can be compared or subtracted as long as they both point in the same array. The difference is the number of array element separating them. As was said before, a pointer to an array is actually a pointer to the first element of the array.

float fa[10], *fp1, *fp2;

fp1 = fp2 = fa;

The difference between fa, which decays to: float * fa = &fa[0]; and &fa is that fa is float (*)[10], which is a pointer to an array of 10 floats. 

When taking the difference of two pointers in an array, the type of the result is implementation defined.

# Void, null, and dubious pointers

Generally, you can't use pointers of different types in the same expression. There is no conversion between them, unlike the arithmetic types.

If you want to force pointers of different types into the same expression, one can use a pointer to void. It can have the value of any other pointer type assigned to it, and can be assigned to any other pointer type.

Sometimes a pointer to nothing might also be useful -- this is where a null pointer comes in. Its useful in the case of an error in a function that returns a pointer.

The two ways of writing a null pointer are 1. integral constant with value zero, or 2. that value converted to type void * with a cast. If the null pointer constant is assigned or compared for equality to any other pointer, then it is first converted to the type of the other pointer. 

Other than assigning 0 to a pointer for the null pointer, the only values that can be assigned to a pointer is another pointer of the same type. 

int *ip;            // declaring a pointer called ip to int
ip = (int *)6;      // casting the integer 6 as a type "pointer to int" and assigning it to ip
*ip = 0xFF;         // setting the memory address that ip points to to 255

# 4. Character Handling

Character handling is done by moving characters in arrays "by hand". Setting a value to zero in a character array indicates that you are terminating the string -- the so-called 'null terminator'. It makes it more explicit and readable if '\0' is used.

strcmp works opposite of matlab, returns 0 when the strings are equal. Returns negative if the first string comes before the second, positive if the first string comes after the second.

# Strings

A string is an array of char, terminated by a null. "this is also a string" or characte array, and its the only thing in C where you can declare it at its point of use. Although strings are array of char, the Standard says that attempting to modify them results in undefined behaviour.

Whenever a string in quotes appears, it provides a declaration and a substitute for a name (the array has no name). It make a hidden declaration of a char array, whos contents are intialized to the character values in the string, terminated by a character whose int value is zero (the null terminator). So in this:

"a string"

What's behind the scenes is this:

char secret[9];
secret[0] = 'a';
secret[1] = ' ';
secret[2] = 's';
secret[3] = 't';
secret[4] = 'r';
secret[5] = 'i';
secret[6] = 'n';
secret[7] = 'g';
secret[8] = '0';

Assuming that we can't see the name 'secret' (remember, it doesn't have a name), how can you use it? The string itself is the name, so "a string" 's type is a pointer to the first element of the hidden unnamed character array, so the type is pointer to char.

# Pointers and Increment Operators

++(*p)  ++*p    pre-increment thing pointed to
(*p)++  (*p)++  post-increment thing pointed to
*(p++)  *p++    access via pointer, post-increment pointer
*(++p)  *++p    access via pointer which has already been incremented

TODO: Review the prescedence tables and memorize


# Untyped Pointers

I thought that using a pointer to void was how we got pointers from one type to another, but maybe that's just for comparison. Turns out, we can cast pointers from one type to another with the following syntax:

(type *) expression

There are some subtleties to this that I am not delving into now.

There are other scenarios when you want to use a generic pointer, such as in the malloc function, which allocates storage for objects that haven't been declared yet; it passes back a pointer to enough storage for what you specified and then you cast it to what you want:

float *fp;
fp = (float *)malloc(4);

malloc finds 4 bytes of storage for you and passes back a generic pointer void *

void *malloc();
float *fp;
fp = (float *)malloc(4);

Technically, there is no need to cast the malloc output, as we have already specified the type of pointer for the destination, but its done for clarity. On different machines, its hard to tell how much memory to allocate since the size of the various types are implementation-defined -- that's what sizeof is for.

# 5. sizeof and Storage Allocation

The sizeof operator returns the size of its operand in bytes either in unsigned int or long depending on the implementation:

#include <stdlib.h>
float *fp
fp = (float *)malloc(sizeof(float));

The parentheses only have to be around the operand if its a type, otherwise the name of the object can be used without them:

#include <stdlib.h>
int *ip, ar[100];
ip = (int *)malloc(sizeof ar);

The fundamental unit of storage is the 8 bit/1 byte char:

sizeof(char) = 1

So malloc(10) can hold 10 chars, but malloc(sizeof(int[10])) would be needed to store 10 ints. If malloc can't find enough free space, then a null pointer is returned. stdio.h contains a NULL constant, but a 0 or (void *)0 could be used too.

The fprintf statement takes an extra first argument which says where the output should go: 1. stdout, which is the standard output and 2. stderr, the programs standard error stream

