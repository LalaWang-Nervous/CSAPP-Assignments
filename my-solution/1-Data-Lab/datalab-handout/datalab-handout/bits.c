/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int m = (~x)&y;
  int n = (~y)&x;
  /*LalaWang's comment: 
  return (m|n)*/
  return ~((~m)&(~n));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  /*LalaWang's comment: return 0x8000000 */
  return 1<<31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  /*
  LalaWang's comment: the overflow of signed integer is undefined, decided by 
  specific complier, so here provides a solution that only works in some platforms,
  but not works in dlc:
  
  int isTmax(int x) {
    int y = x + 0x01; // should be 0x80000000
    int z = y + y; // undefined
    return !z & !!y;
  }

  */
  
  /*
  LalaWang's comment: for the INT_MAX, INT_MAX + 1 = ~INT_MAX
  but 0xFFFFFFFF has this property too, need to exclude it.
  */
  int notX = ~x;
  int y = x + 1;
  int isEqual = !(notX^y);
  return isEqual&(!!y);
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  /*LalaWang's comment: construct 0xAAAAAAAA */
  int base = 0xAA;  // 8 bits
  int target = (base<<8) + base;
  target = (target<<16) + target;
  /*LalaWang's comment: 
       if a number x, all odd-numbered bits in word set to 1
       then x&(0xAAAAAAAA) should be equal to 0xAAAAAAAA
  */
  return !(target^(target&x));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x)+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  /*LalaWang's comment:  
        larger = x - 0x30, and smaller = 0x39 - x
        they all should be >0
  */
  int larger = x + (~0x30) + 1;   
  int smaller = 0x39 + (~x) + 1;
  /*LalaWang's comment: construct 0x80000000 */
  int target = 1<<31;
  return !(target&larger) & !(target&smaller);
  
  /* LalaWang's comment: 
   although the above solution works, such way is not recommended, 
   because when calculate larger and smaller, do plus operation may cause overflow. 
   Here is a better solution:
       int isAsciiDigit(int x) {
	// 0x30 = 0011 0000b   0x39 = 0011 1001b
	int a = (x>>4) ^ 0x3;	// if the 5th and 6th bit is 1
	int b0 = (x>>3) & 1;	// if the 4th bit is 1
	int b1 = (x>>2) ^ 1;	// if the 3th bit is 1
	int b2 = (x>>1) ^ 1;	// if the 2th bit is 1
	return (!a) & ((!b0) | (b0&b1&b2)); // (5th and 6th bit is 1) AND ((4th bit is 0) OR (4th bit is 1，2th、3th bit is 0))
       }
  */
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /*LalaWang's comment: 
       if x = 0, flag = 0x00000000 (0)
       else, flag = 0x11111111 (-1)
  */
  int isZero = !(!(x)); // isZero = 0 or 1
  //LalaWang's comment:  construct 0x00000000 and 0x11111111
  int flag = (isZero<<1) + isZero; // 2 bits
  flag = (flag<<2) + flag; // 4 bits
  flag = (flag<<4) + flag; // 8 bits
  flag = (flag<<8) + flag; // 16 bits
  flag = (flag<<16) + flag; // 32 bits
  return (flag&y) | ((~flag)&z);
  
  /*
  LalaWang's comment: a better way to construct 0x00000000 or 0x11111111
      flag = (isZero<<31)>>31;
  */
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  /*LalaWang's comment: construct 0x80000000 */
  int target = 1<<31;
  int z = !((x^y)&target); 
  /*LalaWang's comment:
      so if z=0, we only need to confirm that y should be the positive one;
      if z=1, x,y has the same sign, and we can sure that y+(~x)+1 will not overflow. (THIS IS VERY IMPORTANT)
      and here I use the Short-circuit calculation principle of Boolean operation, small tricky。
  */
  return ((!z)&(!(y&target))) | (z&(!((y+(~x)+1)&target)));
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
   /*LalaWang's comment:
      if x = 0, then -x = 0 = x.
      for any other num, num != -num, so (num | (-num))'s most significant bit must be 1
   */
   return ((x|(~x+1))>>31)+1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  /*LalaWang's comment:
     for the positive num, we need to find the highest bit(nth, for example) that is 1, return (n+1)
     for the negative num, we need to find the highest bit(nth, for example) that is 0, return (n+1)
     because of that:
        -2^m + 2^(m-1) + 2^(m-2) + ... + 2^(n+1) + 2^n = -2^n 
  */
  int f0,f1,f2,f3,f4;
  int sign = (x&(1<<31))>>31; /*if x>=0, sign=0; else, sign=-1*/
  x = (sign&(~x)) |((~sign)&x); /*if x>=0, x=x; else x=~x*/
  f0 = !!((x>>16)^0); /*if f0=0, the higher 16 bits have no bit 1*/
  x = x>>(f0<<4);
  f1 = !!((x>>8)^0);
  x = x>>(f1<<3);
  f2 = !!((x>>4)^0);
  x = x>>(f2<<2);
  f3 = !!((x>>2)^0);
  x = x>>(f3<<1);
  f4 = !!((x>>1)^0);
  x = x>>(f4<<0);
  return (f0<<4)+(f1<<3)+(f2<<2)+(f3<<1)+f4+1+x;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  int sign = !!(uf&(1<<31)); //sign=0, positive; sign=1, negative
  int exp = (uf&((0xff)<<23))>>23; //(0xff)<<23 =0x7f800000; if exp=0, unregular; exp=0x000000ff, NaN; else,regular
  int frac = (uf&(~((1<<31)>>8))); //0x007fffff (~((1<<31)>>8))
  
  if(exp==255){
    return uf;
  } 
  else if(exp==0){
    return (sign<<31) + (((exp<<23)+frac)<<1);
  }
  exp = exp + 1;
  return (sign<<31) + (exp<<23) + frac;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  int E;
  int res;
  int sign = !!(uf&(1<<31)); //sign=0, positive; sign=1, negative
  int exp = (uf&((0xff)<<23))>>23; //(0xff)<<23 =0x7f800000; if exp=0, unregular; exp=0x000000ff, NaN; else,regular
  int frac = (uf&(~((1<<31)>>8))); //0x007fffff (~((1<<31)>>8))
  
  if(exp==0 || exp<127){ // to small
     return 0;
  }
  if(exp==255){ // NaN
     return 0x80000000u;
  }
  if(exp>=158){ //overflow  or downflow
     return 0x80000000u;
  }
  
  E = exp - 127;
  if(E>=23){
     frac = (frac<<(E-23));
  } else {
     frac = frac>>(23-E);
  }
  
  res = frac + (1<<E);
  if(sign==1){
     res = (~res)+1;
  }
  return res;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  if(x>=128){
    return (0xff)<<23;
  }
  else if(x<= -127){
    return 0;
  }
  return (0<<31) + ((127 + x)<<23);
}
