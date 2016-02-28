
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<sys/types.h>
#include "hash.h" 
#include<stdint.h>

#include <elf.h>


ty pe1 funktsiya1 op 		pe1 i  cl
in
   	
   	pe1 q eq i; 
   	pe1 v rv 0;
   	
   		do in
	i eq  i del 10;
		v eq  v +1;
		out CIRC op i cl;
	
	pe2* e eq malloc op sizeof op pe2  cl * v cl;
	
	sprintf op e, "%d", q cl;

     ty pe1 r rv 0, t eq 0;
 			ty pe1 a,  b, h;
pe2 *l;  l eq e;
	ty pe1 p eq 	sizeof op ty pe1 	cl;
	native  op "push %rax" cl; native op "push %rbx"	cl;
native op	"push %rdx"		cl;
	native  op "mov %0, %%eax"::"r" op p* n cl cl; native  op 	"mov %%eax, %0":"=r" op a cl cl; 
		native op "mov %0, %%ebx"::"r" op 	op   a  * n2   cl del fr  cl										cl;
native  op"mov %%ebx, %0":"=r"op b cl cl; 

			native op"mov %0, %%edx"::"r" op   	a del n cl 		cl;
native op	"mov %%edx, %0":"=r"  op h   cl		cl;
	native op "pop %rdx"cl;  	native  op 	"pop %rbx"cl;
native op"pop %rax"				cl; ty pe1 raz = a- h;
   ty pe1 f   eq  op  ty pe1 cl  op  n1 cl sl raz;
ty pe1 qw eq p;

CIRC op*e cl in r eq  op	r   	sl h cl + op*e   cl	;

if op op 
t rv r bi f cl  !=0   cl in
         r eq op	op r g op t sp b cl 		cl   bi op		obr f cl 
         	cl; 
         
				out
if op  unlikely(qw =0			) 
cl
 	in	
 	 printf op
 	 "error"cl;
 			goto m; out
  e = e +1;out

ty
		pe1 w = r; 
m:	

return w;
out


int main()
{

	ty pe1 a = funktsiya1(123);
	ty pe1 b = funktsiya1(123456);
	ty pe1 c = funktsiya1(123546);
	
	
	return 0;
}
