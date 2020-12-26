%{
#undef input
#undef unput

#include    <X11/Xlib.h>
#include    <X11/Xresource.h>
#include    <X11/IntrinsicP.h>
#include    <X11/StringDefs.h>
#include    <ctype.h>

#include    "LayoutP.h"
#include    "laygram.h"
static char *yysourcebase, *yysource;

void
yyerror(char *s);

static int
count (char *s, char c);

static int
istoken (char c, int first);

#ifdef FLEX_SCANNER
#define YY_INPUT(buf,result,max_size) \
    { \
	int c; \
	result = 0; \
	while (result < max_size) { \
	    c = *yysource; \
	    if (c == '\0') \
		break; \
	    buf[result++] = c; \
	    yysource++; \
	} \
    }
#else
#define input() (*yysource++)
#define unput(c)    (--yysource)
#endif

%}
%%
vertical		return VERTICAL;
horizontal		return HORIZONTAL;
overlay			return OVERLAY;
"{"			return OC;
"}"			return CC;
"("			return OP;
")"			return CP;
"<"			return OA;
">"			return CA;
infinity		{ LayYYlval.ival = 1; return INFINITY; }
inff*			{ LayYYlval.ival = count(yytext, 'f'); return INFINITY; }
[0-9][0-9]*		{ LayYYlval.ival = atoi(yytext); return NUMBER; }
"="			{ return EQUAL; }
"$"			{ return DOLLAR; }
","			{ return COMMA; }
"+"			{ LayYYlval.oval = Plus; return PLUS; }
"-"			{ LayYYlval.oval = Minus; return MINUS; }
"*"			{ LayYYlval.oval = Times; return TIMES; }
"/"			{ LayYYlval.oval = Divide; return DIVIDE; }
"%"			{ LayYYlval.oval = Percent; return PERCENT; }
%[ \t\n]*of		{ LayYYlval.oval = Percent; return PERCENTOF; }
width			return WIDTH;
height			return HEIGHT;
min			{ LayYYlval.oval = Minimum; return MIN; }
max			{ LayYYlval.oval = Maximum; return MAX; }
\\[a-zA-Z_][a-zA-Z0-9_]* { 
			    int i = yyleng;
			    while (!istoken (yytext[i-1], i == 0))
				i--;
			    yytext[i] = '\0';
			    LayYYlval.qval = XrmStringToQuark (yytext+1);
 			    return NAME;
			}

[a-zA-Z_][a-zA-Z0-9_]*	{ 
			    int i = yyleng;
			    while (!istoken (yytext[i-1], i == 0))
				i--;
			    yytext[i] = '\0';
			    LayYYlval.qval = XrmStringToQuark (yytext);
 			    return NAME;
			}
" "			;
"\t"			;
"\n"			;
.			fprintf (stderr, "ignoring %c\n", *yytext);
%%

static int
istoken (char c, int first)
{
    if (c == '_')
	return 1;
    if ('a' <= c && c <= 'z')
	return 1;
    if ('A' <= c && c <= 'Z')
	return 1;
    if (!first && '0' <= c && c <= '9')
	return 1;
    return 0;
}

static int
count (char *s, char c)
{
    int	i = 0;
    while (*s)
	if (*s++ == c)
	    i++;
    return i;
}

void
LayYYsetsource(char *s)
{
    yysourcebase = yysource = s;
}

void
LayYYerror(char *s)
{
    char    *t;

    (void) input;
    (void) yyunput;
    fprintf (stderr, "%s\n", s);
    t = yysource - 50;
    if (t < yysourcebase)
	t = yysourcebase;
    while (*t && t < yysource + 50) {
	if (t == yysource)
	    putc ('@', stderr);
	putc (*t++, stderr);
    }
    if (t == yysource)
	putc ('@', stderr);
    if (!*t)
	fprintf (stderr, "<EOF>");
    fprintf (stderr, "\n");
}
