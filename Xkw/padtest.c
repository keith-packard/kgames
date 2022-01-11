# include	<X11/Intrinsic.h>
# include	<X11/StringDefs.h>
# include	<X11/Shell.h>
# include	<X11/Xos.h>
# include	<Xkw/Pad.h>
# include	<X11/Xutil.h>
# include	<ctype.h>
# include	<sys/ioctl.h>
# include	<stdlib.h>

Widget	toplevel;
Widget	pad;

static int  row, col;

void
CheckScroll (Widget w)
{
    Arg	arg[1];
    Dimension	rows;

    XtSetArg (arg[0], XtNnumRows, &rows);
    XtGetValues (w, arg, 1);
    while (row >= rows)
    {
	XkwPadScroll (w, 0, rows, -1);
	row--;
    }
}

static void
append (XtPointer closure, int *source, XtInputId *inputid)
{
    Widget  w = *(Widget *)closure;
    int	    fd = *(int *) source;
    char    buf[1024], c;
    int	    n, count;

    (void) inputid;
    c = XkwPadNormal;
    XkwPadAttributes (w, row, col, &c, 1);
    XkwPadUpdate (w);
    do {
	count = read (fd, buf, sizeof (buf));
	if (count <= 0)
	    exit (0);
	for (n = 0; n < count; n++)
	{
	    c = buf[n];
	    if (c == '\n')
	    {
		col = 0;
		row++;
		CheckScroll (w);
	    } else if (isprint (c)) {
		XkwPadText (w, row, col, &c, 1);
		col++;
	    }
	}
	ioctl (fd, FIONREAD, &count);
    } while (count > 0);
    c = XkwPadInverse;
    XkwPadAttributes (w, row, col, &c, 1);
    XkwPadUpdate (w);
}

int
main (int argc, char **argv)
{
#ifdef APPDEFAULTS
    setenv("XAPPLRESDIR", APPDEFAULTS, 1);
#endif

    toplevel = XtInitialize (argv[0], "PadTest", 0, 0, &argc, argv);

    pad = XtCreateManagedWidget ("pad", padWidgetClass, toplevel, NULL, 0);
    XtAddInput (0, (XtPointer) (intptr_t) XtInputReadMask, append, (XtPointer) &pad);
    XtRealizeWidget (toplevel);
    XtMainLoop ();
}
