//!\file k.c \brief bare minimum atw-style k interpreter for learning purposes \copyright (c) 2024 arthur whitney and the regents of kparc \license mit
#include"a.h"                                       //fF[+-!#,@|] atom/vector 1byteint 1bytetoken  no(parser tokens ..) cc -Os -oa a.c -w

                                                    //! above is a brief description of k/simple system by atw:
                                                    //! he says:       he means:
                                                    //! fF[+-!#,@]     we have 6 verbs [+ - ! # , @ |] in both monadic and dyadic contexts, total of 14
                                                    //!                (since monadic + and dyadic | are nyi, we actually have 12, feel free to implement f[+]/F[|])
                                                    //! atom/vector    k/simple sports atoms and vectors!
                                                    //! 1byteint       the only supported atom/vector type is 8bit integer, so beware of overflows
                                                    //! 1bytetoken     input tokenizer is spartan: a token can't be longer than one char
                                                    //! no(...)        no parser and multichar tokenizer are implemented
                                                    //! cc -w ..       minimal build instructions (which are much more stringent in provided makefile)

//!debug
def1(wu,printf("%lu\n",x))                          //!< (w)rite (u)ll: print unsigned long long (e.g. total memory allocation), useful for refcount debugging.
void wg(){FOR(26,x(globals[i],$(!ax,printf("%c[%d] %d\n",i+'a',nx,rx))))} //!< dump global namespace: varname, refcount, length (also useful for refcount debugging).

//!printing facilities
def1(w,write(1,ax?(u8*)&x:sx,ax?1:strlen(sx)))      //!< (w)rite to stdout: if x is an atom, print its decimal value, otherwise print x as ascii string.
static u8 pb[12];                                   //!< temporary string (b)uffer for formatting vector items. it's ok to declare it globally, since we only have one thread.
def1(si,sprintf(pb,"%d ",(int)(128>x?x:x-256));pb)  //!< (s)tring from (i)nteger: format a given atom x as decimal in range (-128..127) into buffer b using sprintf(3).
def1(wi,w(si(x)))                                   //!< (w)rite (i)nteger: format x and (w)rite it to stdout.
def1(W,Q(x)$(ax,wi(x))FOR(nx,wi(xi))w(10))          //!< pretty print x: if x is an atom, format and print it, otherwise print all items of vector x,
                                                    //!< separated by space. terminate output by a newline aka ascii 10.
def3(err,w(f);w(':');wi(x);w(y);w('\n');ERR)        //!< (err)or: print name of the C (f)unction where error occured, line number and error msg, return ERR.

//!malloc
def1(alloc,y(x+2,WS+=x;u8*s=malloc(y);*s++=0;*s++=x;s))//!< (a)llocate x bytes of memory for a vector of length x plus two extra bytes for preamble, set refcount to 0
                                                    //!< and vector length to x in the preamble, and return pointer to the 0'th element of a new vector \see a.h type system
def1(unalloc,WS-=nx;free(sx-2);0)                   //!< release memory allocated for vector x.
def3(move,(u)memcpy((u8*)x,(u8*)y,f))               //!< (m)ove: x and y are pointers to source and destination, f is number of bytes to be copied from x to y.
                                                    //!< \note memcpy(3) assumes that x/y don't overlap in ram, which in k/simple they can't, but \see memmove(3)
//!memory management
def1(incref,ax?x:(++rx,x))                          //!< increment refcount: if x is an atom, return x. if x is a vector, increment its refcount and return x.
def1(decref,ax?x                                    //!< decrement refcount: if x is an atom, return x.
       :rx?(--rx,x)                                 //!<   if x is a vector and its refcount is greater than 0, decrement it and return x.
          :unalloc(x))                              //!<   if refcount is 0, release memory occupied by x and return 0.

//!monadic verbs
def1(foo,decref(x);Qz(1);ERR)
def2(Foo,decref(x);Qz(1);ERR)                       //!< (foo)bar is a dummy monadic verb: for any x, throw nyi error and return error code ERR

def1(sub,ax?(u8)-x:_x(NEW(nx,-xi)))                 //!< monadic (sub)tract is also known as (neg)ation, or -x: if x is atom, return its additive inverse.
                                                    //!< if x is a vector, return a new vector same as x only with sign of its every element flipped.

def1(til,Qr(!ax)(NEW(x,i)))                         //!< monadic til is !x aka her majesty apl iota. for a given atom x, it returns a vector
                                                    //!< of x integers from 0 to x-1. if x is not an atom, til throws a rank error.

def1(cnt,Qr(ax)nx)                                  //!< monadic (c)ou(nt) is #x. it returns the length of a given vector and throws rank error for atoms.

def1(cat,Qr(!ax)NEW(1,x))                           //!< monadic (cat)enate is (enl)ist, or comma-x: wraps a given atom x into a new vector of length 1 whose
                                                    //!< only item holds the value of that atom. if x is a vector, enlist will throw a rank error.

def1(rev,Qr(ax)_x(NEW(nx,sx[nx-i-1])))              //!< monadic (rev)erse is |x and simply returns a mirror copy of vector x.

//!dyadic verbs
def2(Add,                                           //!< dyadic f+y is add. operands can be both atoms and verbs, ie. a+a, a+v, v+a, v+v are all valid.
  ax?af?(u8)(f+x)                                   //!< case a+a: if (f,x) are atoms, compute their sum and handle possible overflows by downcasting it to u8.
       :Add(x,f)                                    //!< case v+a: if f is a vector and x is an atom, make a recursive call with operands swapped, i.e. a+v.
    :af?_x(NEW(nx,f+xi))                            //!< case a+v: if f is an atom, return a new vector constructed by adding f to every element of x.
       :nx-nf?(_x(_f(Ql())))                        //!< case v+v: if (f,x) are vectors, first make sure they are of the same length, throw length error if not.
             :_f(_x(NEW(nx,xi+fi))))                //!<           if lengths are the same, return a new vector holding their pairwise sum.
                                                    //!< \note by convention, atwc uses x-y for inequality test, which has the same effect as nx!=nf.

def2(Sub,Add(f,sub(x)))                             //!< dyadic f-x is subtract. since we already have Add() and sub(), we get Sub() for free by negating x.
def2(Mod,Qr(!f||!af)ax?x%f:_x(NEW(nx,xi%f)))        //!< dyadic f!x is x (mod)ulo f, aka remainder operation. f must be an non-zero atom, x can be anything.
def2(Tak,Qr(!af)_f(NEW(f,ax?x:sx[i%nx])))           //!< dyadic f#x is (tak)e, which has two variants based on the type of right operand (left must be atom):
                                                    //!<  if x is a vector, return first f items of x. if f exceeds the size of x, wrap around from the start.
                                                    //!<  if x is an atom, return a vector of length f filled with x.

def2(Cat,                                           //!< dyadic f,x is (cat)enate: a) join two vectors b) join an atom to vector c) make a vector from two atoms.
  f=af?cat(f):f;                                    //!< if f is an atom, enlist it \see cat()
  x=ax?cat(x):x;                                    //!< ditto for x
  u r=alloc(nf+nx);                                 //!< (a)llocate array r long enough to hold f and x.
  move(nx,r+nf,x);                                  //!< (m)ove contents of x to the end of r.
  move(nf,r,f);decref(x);decref(f);r)               //!< (m)ove contents of f to the beginning of r, try to release f and x, and return pointer to r.

def2(At,Qr(af)                                      //!< dyadic f@x is "needle at x in the haystack f" and has two modes based on the type of x (f must be a vector):
  ax?x>nf?Ql():sf[x]                                //!<  if x is an atom, return the x'th item of f.
    :_x(_f(NEW(nx,sf[xi]))))                        //!<  if x is a vector, return a vector containg items from f at indices listed in x.
                                                    //!< \note that the second mode currently doesn't perform the boundary check, fell free to implement it!

def1(at,At(x,0))                                    //!< monadic @x is simply (f)ir(st): return the head element of x, or throw a rank error if x is an atom.

//! note how Sub() and at() are implemented in terms of other verbs, and especially how Add() cuts corners by calling itself with operands swapped.
//! in fact, we can use Add() as a template to implement of a whole bunch of additional dyadic verbs, provided that they also hold commutative property.
//! so let's generalize Add(), first in pseudocode:

//! function fn(f,x) implementing a commutative OP:
//!  1. if both operands f and x are atoms, return (f) OP (x)
//!  2. if f is an atom and x is a vector, return fn(x,f)
//!  3. if both operands are vectors, ensure they are the same length.
//!  4. allocate a (r)esult vector of the same length as x, then:
//!  5. depending on type of x, each i'th element of r becomes either:
//!     5.1 (atom x) OP (i'th element of f)
//!     5.2 (i'th element of x) OP (i'th element of f)
//!  6. finally, attempt to release memory of f and x, and return r.

#define defop(fn,OP)  u fn(u f, u x) { \
    if (ax && af) return (u8)(f OP x); \
    if (!ax && af) return fn(x,f); \
    if (nx != nf) return Ql(); \
    return _f(_x(NEW(nx, sx[i] OP sf[i]))); \
}

defop(Eql,==)
defop(Not,!=)
defop(And,&)
defop(Or,|)
defop(Prd,*)                //!< et voila, we have definitions of dyadic equal, not equal, and, or and product for free.

//!verb dispatch
char*verbs=" +-!#,@=~&|*";                                        //!< verbs is an array of tokens of all supported k verbs. 0'th item (space) stands for "not a verb".
u(*verbs1[])(u  )={0,foo,sub,til,cnt,cat,at,foo,foo,foo,rev,foo};  //!< f[] is an array of pointers to C functions which implement monadic versions of k verbs listed in verbs.
u(*verbs2[])(u,u)={0,Add,Sub,Mod,Tak,Cat,At,Eql,Not,And,Or, Prd};  //!< F[] is ditto for dyadic versions of verbs listed in verbs.
// verbs:             +   -   !   #   ,   @  =   ~   &   |   *

//!adverbs
def2(Ovr,ax?x:_x(r(*sx,FOR(nx-1,r=verbs2[f](r,sx[i+1])))))                       //!< adverb over: recursively fold all elements of vector x using dyadic verb f going left to right.
def2(Scn,ax?x:_x(r(alloc(nx),*sr=*sx;FOR(nx-1,sr[i+1]=verbs2[f](sr[i],sx[i+1]))))) //!< adverb scan: same as over, but produces a vector of intermediate results.

//!adverb dispatch
char*adverbs=" /\\";
u(*adverbs2[])(u,u)={0,Ovr,Scn};                    //!< adverbs[]/adverbs2[] is the same as verbs[]/verbs2[], only for adverbs.

//!globals, verbs, nouns, adverbs
def1(isglobal,x>='a'&&x<='z')                       //!< is x a valid (g)lobal variable identifier?
def1(getglobal, globals[x])
def2(setglobal,y(globals[f],!ay?unalloc(y):x;incref(globals[f]=x)))    //!< release no longer referenced global object at globals[f], and replace it with object x.
                                                    //!< \note rarely seen ternary form x?:y, which is just a shortcut for x?x:y in c.
def1(verb,(strchr(verbs,x)?:verbs)-verbs)           //!< is x a valid verb from verbs? if so, return its index, otherwise return 0.
def1(adverb,(strchr(adverbs,x)?:adverbs)-adverbs)   //!< same as verb() for adverbs.
def1(noun,10>x-'0' ? x-'0'                          //!< is x a (n)oun?  if x is a digit, e.g. '7', return its decimal value.
        : isglobal(x) ? incref(getglobal(x-'a'))    //!< if x is a varname, e.g. 'a', return its value from globals[26] and increment its refcount.
        : ERR)                                      //!< ..anything else is an error.

//!fio
static char*line;
u linemax=99;
FILE*fp;                                            //!< line is a line buffer, linemax is its max length, fp is input stream handle.
defstr(readln,line=line?:malloc(linemax);           //!< readln: reset linemax to max line length, allocate buffer line of size linemax if not yet allocated.
   P(!s,line[read(0,line,linemax)-1]=0)             //!< (r)ead: if no filename s is given, read line from stdin up to linemax bytes, clamp trailing \n and return 0.
   fp=fp?:fopen(s,"r");Qs(!fp,s)                    //!< open file s for reading if not yet open, throw error in case of problems.
   r(getline(&line,&linemax,fp),                    //!< read next line from stream fp into line up to linemax bytes.
     r=r<linemax?line[r-('\n'==line[r-1])]=0:ERR))  //!< if reached end of file, return ERR, otherwise clamp trailing \n and return 0.

//!eval
defstr(eval,                                        //!< (e)val: recursively evaluate input tape s in reverse order (left of right), and return the final result:
   u8*t=s;u8 i=*t++;                                //!< t is a temporary pointer to s. read the current token into i and advance temporary tape pointer.
   !*t?x(noun(i),Qp()x)                                //!< if next token after i is null (ie end of tape): final token must be a noun, so return it, otherwise:
      :verb(i)                                      //!< in case if i is a valid verb:
           ?adverb(*t)?x(eval(t+1),Q(x)             //!<   if the verb is followed by an adverb, recursively evaluate token after adverb into x. bail out on error.
                    adverbs2[adverb(*t)](verb(i),x))//!<     dispatch an adverb: first argument is the index of the verb, second is the operand.
           :x(eval(t),Q(x)                          //!<   otherwise, recursively evaluate next token after verb and put resulting noun into x. bail out on error.
              verbs1[verb(i)](x))                   //!<   apply monadic verb i to the operand x and return the result, which can be either nounmn or error.
           :y(                                      //!< in case if i is not a verb, it must be a valid noun, and the next token after a noun should be a verb,
              eval(t+1),Q(y)                        //!<   recursively evaluate next token to the right of the verb and put result into y. bail out on error.
              ':'==*t                               //!<   special case: if y is preceded by a colon instead of a verb, it is an inline assignment (eg 1+a:1),
                    ?x(isglobal(i),Qp()setglobal(i-'a',y)) //!<   so i should be a (g)lobal varname a..z. if so, increment y's refcount, store it in globals[26], and return it.
                    :x(noun(i),Qp()                 //!<   x is a noun to the left of the verb. throw parse error if it is invalid.
                         u8 f=verb(*t);Qd(!f)       //!<   f is the index of the verb to the left of noun y. if it's not a valid verb, throw domain error.
                         verbs2[f](x,y))))          //!< apply dyadic verb f to nouns x and y (e.g. 2+3) and return result (noun or error).

//!repl/batch
int main(int argc,char**argv){u batch=2==argc;      //!< entry point: batch=0 is repl mode, batch=1 is batch mode i.e. when a filename is passed.
  batch?:printf("%s",BA);                           //!< system banner is only printed in interactive mode.
  while(batch?:w(32),ERR!=readln(argv[1]))          //!< enter infinite read-eval-print loop until ctrl+c is pressed, error or EOF is reached.
   if(*line){                                       //!< write prompt (single space), then wait for input from stdin which is read into b.
    $('\\'==*line&&!line[2],                        //!< if buffer starts with backslash and is two bytes long:
     $('\\'==line[1],break)                         //!<   if buffer is a double backslash, exit repl and terminate process.
      $('w'==line[1],wu(WS))                        //!<   if buffer is a \w, print workspace usage and cycle repl.
       $('v'==line[1],wg()))                        //!<   if buffer is a \v, print globals and their refcounts (useful for debug).
        $('/'==*line,continue)                      //!< if buffer starts with /, treat the rest of the line as comment and cycle repl.
         x(eval(line),                              //!< else, evaluate buffer b[] and put result into x, then:
           ':'==line[1]?x                           //!<   if b starts with a global assignment e.g. a:7, suppress output and cycle repl.
                   :_x(W(x)));}                     //!<   otherwise, pretty print evaluation result to stdout, then cycle repl.
  return free(line),fclose(fp),0;}                  //!< in C, return value of main() is the exit code of the process, 0 is success.

//:~
