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
infinity		{ yylval.ival = 1; return INFINITY; }
inff*			{ yylval.ival = count(yytext, 'f'); return INFINITY; }
[0-9][0-9]*		{ yylval.ival = atoi(yytext); return NUMBER; }
"="			{ return EQUAL; }
"$"			{ return DOLLAR; }
","			{ return COMMA; }
"+"			{ yylval.oval = Plus; return PLUS; }
"-"			{ yylval.oval = Minus; return MINUS; }
"*"			{ yylval.oval = Times; return TIMES; }
"/"			{ yylval.oval = Divide; return DIVIDE; }
"%"			{ yylval.oval = Percent; return PERCENT; }
%[ \t\n]*of		{ yylval.oval = Percent; return PERCENTOF; }
width			return WIDTH;
height			return HEIGHT;
min			{ yylval.oval = Minimum; return MIN; }
max			{ yylval.oval = Maximum; return MAX; }
\\[a-zA-Z_][a-zA-Z0-9_]* { 
			    int i = yyleng;
			    while (!istoken (yytext[i-1], i == 0))
				i--;
			    yytext[i] = '\0';
			    yylval.qval = XrmStringToQuark (yytext+1);
 			    return NAME;
			}

[a-zA-Z_][a-zA-Z0-9_]*	{ 
			    int i = yyleng;
			    while (!istoken (yytext[i-1], i == 0))
				i--;
			    yytext[i] = '\0';
			    yylval.qval = XrmStringToQuark (yytext);
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
yysetsource(char *s)
{
    yysourcebase = yysource = s;
}

void
yyerror(char *s)
{
    char    *t;
    
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
