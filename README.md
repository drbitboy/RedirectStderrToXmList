# RedirectStderrToXmList
Proof of concept:  redirect I/O to Motif List widget (XmList) in Command widget (XmCommand)

Refer to comments in xmcommand.c

The genesis for this idea is an X-Window/Motif GUI application that is run from the command line, and which has over time accumulated various informational and error messages the are sent to the terminal via stderr, using fprintf(stderr, ...) calls.

I am investigating replacing the terminal/stderr output with a logging window controlled by the applicate, e.g. an XmList widget using X/Motif.  There are a few hundred of these messages, so changing all of the fprintf() statements to Xm calls would be a lot of work, and the necessity of using variable-length string buffers and ensuring no buffer overruns means it could not be easily done with a macro or serch-and-replace.

The approach listed here goes a long way toward achieving the desired result:  STDERR is redirected (via dup()) into the input side of a pipe (created with pipe()), and the X event loop (XtAppMainLoop) is directed to watch for data on the output side of that same pipe, and to put those data into a List widget.

The advantage of this approach is that none of the existing fprintf(...) statements need to change.  By redirecting, in just a few line, the stderr I/O from the terminal to the pipe monitored by the X event handler loop, all stderr I/O ends up in the List widget.
