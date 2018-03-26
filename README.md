# RedirectStderrToXmList

## (There is no software task that cannot be achieved with another layer of indirection ;-)

### Proof of concept:  redirect I/O to Motif List widget (XmList) in Command widget (XmCommand)

Refer to comments in xmcommand.c

The genesis for this idea is from a legacy X-Window/Motif GUI application (a descendant of [Orbit](https://github.com/drbitboy/Orbit)) that is run from the command line, and which has over time accumulated various informational and error messages written to the terminal via stderr, using fprintf(stderr, ...) calls.

I am investigating replacing the terminal/stderr output with a logging window controlled by the application, e.g. replace terminal output an XmList widget using X/Motif.  There are a few hundred of these messages, so changing all of the fprintf() statements to Xm calls would be a lot of work, and the necessity of using variable-length string buffers and ensuring no buffer overruns means it could not be easily done with either macros or search-and-replace.

The approach deomnstrated here goes a long way toward achieving the desired result without massive changes to the code:  STDERR is redirected (via dup()) into the input side of a pipe (created with pipe()), and the X event loop (XtAppMainLoop) is directed to watch for data on the output side of that same pipe (presumably via a select(..) call or similar), and to put those data into a List widget.

The advantage of this approach is that none of the existing fprintf(...) statements need to change.  By redirecting, in just a few lines, the stderr I/O from the terminal to the pipe is monitored by the X event handler loop, and all stderr I/O ends up in the List widget.

This is similar to several similar examples available on the Web, and I gratefully acknowledge my indebtedness to those works.  One difference between this approach and those is that this approach involves one thread (no forks) redirecting and using its own STDERR output; that works because single-threaded X-Windows-based applications like this do all their work in event handlers.
