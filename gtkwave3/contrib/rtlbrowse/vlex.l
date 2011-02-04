/*
 * Copyright (c) Tony Bybell 2006
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
         
%{
        
/*
 * vlex.l
 * 06apr06ajb
 */

/*
 * $Id: vlex.l,v 1.1.1.1 2007/05/30 04:25:41 gtkwave Exp $
 * $Log: vlex.l,v $
 * Revision 1.1.1.1  2007/05/30 04:25:41  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:10  gtkwave
 * initial release
 *
 */

#define YY_INPUT(buf, result, max_size) \
   result = v_preproc(buf, max_size);

#include "vlex.h"

int my_yylineno = 0;

static int yywrap(void)
{
return(1);
}

static int v_preproc_initialized = 0;
char *v_preproc_name = NULL;


static int v_preproc(char *buf, int max_size)
{
int ch;

if(!v_preproc_initialized)
	{
	yyin = fopen(v_preproc_name, "rb");
	if(!yyin) return(0);
	v_preproc_initialized = 1;
	my_yylineno = 1;	
	}

ch = fgetc(yyin);
if(ch!=EOF)
	{
	buf[0] = ch;
	buf[1] = 0;
	return(1);
	}
	else
	{
	fclose(yyin); yyin = NULL;
	v_preproc_initialized = 0;
	return (0);
	}
}

%}
  
%x CMNT
%x PREPROC

%%
"/*"            					{ BEGIN CMNT; return V_CMT; }
<CMNT>"/*"      					{ return V_CMT; }
<CMNT>"*/"      					{ BEGIN INITIAL; return V_CMT; }
<CMNT>[^\*\\/\n]+      					{ return V_CMT; }
<CMNT>.         					{ return V_CMT; }
<CMNT>\n        					{ my_yylineno++; return V_CMT; }

<INITIAL>always						{ return V_KW; }
and 							{ return V_KW; }
assign 							{ return V_KW; }
begin 							{ return V_KW; }
buf 							{ return V_KW; }
bufif0 							{ return V_KW; }
bufif1 							{ return V_KW; }
case 							{ return V_KW; }
casex 							{ return V_KW; }
casez 							{ return V_KW; }
cmos 							{ return V_KW; }
deassign 						{ return V_KW; }
default 						{ return V_KW; }
defparam 						{ return V_KW; }
disable 						{ return V_KW; }
edge 							{ return V_KW; }
else 							{ return V_KW; }
end 							{ return V_KW; }
endcase 						{ return V_KW; }
event 							{ return V_KW; }
for 							{ return V_KW; }
force 							{ return V_KW; }
forever 						{ return V_KW; }
fork 							{ return V_KW; }
highz0 							{ return V_KW; }
highz1 							{ return V_KW; }
if 							{ return V_KW; }
initial 						{ return V_KW; }
inout 							{ return V_KW; }
input 							{ return V_KW; }
integer 						{ return V_KW; }
join 							{ return V_KW; }
large 							{ return V_KW; }
medium 							{ return V_KW; }
nand 							{ return V_KW; }
negedge 						{ return V_KW; }
nmos 							{ return V_KW; }
nor 							{ return V_KW; }
not							{ return V_KW; }
notif0 							{ return V_KW; }
notif1 							{ return V_KW; }
or 							{ return V_KW; }
output 							{ return V_KW; }
parameter 						{ return V_KW; }
pmos 							{ return V_KW; }
posedge 						{ return V_KW; }
pull0 							{ return V_KW; }
pull1 							{ return V_KW; }
pulldown 						{ return V_KW; }
pullup 							{ return V_KW; }
rcmos 							{ return V_KW; }
real 							{ return V_KW; }
reg 							{ return V_KW; }
release 						{ return V_KW; }
repeat 							{ return V_KW; }
rnmos 							{ return V_KW; }
rpmos 							{ return V_KW; }
rtran 							{ return V_KW; }
rtranif0 						{ return V_KW; }
rtranif1 						{ return V_KW; }
scalared 						{ return V_KW; }
small 							{ return V_KW; }
specparam 						{ return V_KW; }
strong0 						{ return V_KW; }
strong1 						{ return V_KW; }
supply0 						{ return V_KW; }
supply1 						{ return V_KW; }
time 							{ return V_KW; }
tran 							{ return V_KW; }
tranif0 						{ return V_KW; }
tranif1 						{ return V_KW; }
tri 							{ return V_KW; }
tri0 							{ return V_KW; }
tri1 							{ return V_KW; }
triand 							{ return V_KW; }
trior 							{ return V_KW; }
trireg 							{ return V_KW; }
vectored 						{ return V_KW; }
wait 							{ return V_KW; }
wand 							{ return V_KW; }
weak0 							{ return V_KW; }
weak1 							{ return V_KW; }
while 							{ return V_KW; }
wire 							{ return V_KW; }
wor 							{ return V_KW; }
xnor 							{ return V_KW; }
xor 							{ return V_KW; }
task 							{ return V_KW; }
endtask 						{ return V_KW; }
function 						{ return V_KW; }
endfunction 						{ return V_KW; }
table 							{ return V_KW; }
endtable 						{ return V_KW; }
specify 						{ return V_KW; }
endspecify 						{ return V_KW; }


macromodule 						{ return V_MODULE; }
module 							{ return V_MODULE; }
primitive 						{ return V_MODULE; }

endmodule 						{ return V_ENDMODULE; }
endprimitive 						{ return V_ENDMODULE; }

<PREPROC>[^\n]+						{ return V_PREPROC_WS; }
<PREPROC>\n        					{ my_yylineno++; BEGIN INITIAL; return V_WS; }

`[_a-zA-Z][_a-zA-Z\$0-9]*				{ 
							if(is_builtin_define (yytext+1, yyleng-1))
								{
								BEGIN PREPROC;
								return(V_PREPROC);
								}

							return(V_MACRO);
							}


\\[^ \t\r\n]+						{ return V_ID; }
\$[_a-zA-Z][_a-zA-Z\$0-9]* 				{ return V_FUNC; }
[_a-zA-Z][_a-zA-Z\$0-9]* 				{ return V_ID; }
[_a-zA-Z][_a-zA-Z\$0-9]*(\.[_a-zA-Z][_a-zA-Z\$0-9]*)+	{ return V_ID; }
\.[_a-zA-Z][_a-zA-Z\$0-9]* 				{ return V_PORT; }
[0-9]+ 							{ return V_NUMBER; }
\'[dD][ \t]*[0-9][0-9_]* 				{ return V_NUMBER; }
\'[bB][ \t]*[xXzZ?01][xXzZ?01_]* 			{ return V_NUMBER; }
\'[oO][ \t]*[xXzZ0-7][xXzZ0-7_]* 			{ return V_NUMBER; }
\'[hH][ \t]*[xXzZ0-9A-Fa-f][xXzZ0-9A-Fa-f_]* 		{ return V_NUMBER; }
"*/"            					{ return V_IGNORE; }
[ \t\r]+        					{ return V_WS; }
\n              					{ my_yylineno++; return V_WS; }
\/\/[^\n]*      					{ return V_CMT; }
\"[^\"\r\n]*\"	 					{ return V_STRING; }
. 							{ return V_IGNORE; } 

%%
