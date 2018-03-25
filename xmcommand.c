/***********************************************************************
 * Redirect STDERR to List widget (XmList) of Command widget (XmCommand)
 *
 * Compile:
 *
 *   gcc -o xmcommand xmcommand.c -I/usr/include -lXm -lXtst -lXt -lX11
 *
 * Run:
 *
 *   ./xcommand
 *
 * Limitations:
 *
 *   This example program is a proof-of concept only; this approach may
 *   not work in all situations.
 *
 *   N.B. it may be possible for this program to hang if a write to
 *        STDERR blocks e.g. if there is a limit how much may be written
 *        to STDERR and/or the pipe.
 *
 *   N.B. no provision has been made to limit the number of entries in
 *        the List widget; presumably there is a practical limit to that
 *        number, although it may be large.
 */
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/Command.h>
#include <Xm/RowColumn.h>

#define MYWRAP 180     /* Maximum line length in List widget */

/***********************************************************************
 * WorkArea callback; writes 2 lines to STDERR per WorkArea button click
 * - This will be re-directed to to the pipe, read by get_pipe_input, and
 *   added to the List widget
 */
void
work_area_CB(Widget pb, XtPointer client_data, XtPointer call_data) {
static unsigned long iNext;

  /* Write to STDERR; FFLUSH probably not needed as STDERR is not buffered */
  fprintf(stderr, "Push offset %lu to STDERR:\n  => time = <%lu>\n", iNext, (unsigned long) time(0));
  fflush(stderr);

  /* Log to STDOUT; FFLUSH probably not needed as STDOUT is line-buffered */
  fprintf(stdout, "[STDOUT:] Wrote [Push offset %lu\\n  => time = <%lu>\\n] to STDERR\n", iNext++, (unsigned long) time(0));
  fflush(stdout);

  return;
}

/***********************************************************************
 * XtAppAddInput callback, reads from *fid, the input end of the pipe
 * connected to STDERR; puts the text read into the List widget
 */
void
get_pipe_input(XtPointer client_data, int* fid, XtInputId* id) {
Widget listW = (Widget) client_data;
char buf[BUFSIZ+1];
char* pStart;
char* pStop;
char* pFinalStop;
int nbytes;
XmString xmstring;

  /* Read from pipe; exit on error; return if nothing was read */
  if ((nbytes=read(*fid,buf,BUFSIZ)) == -1) { perror("get_pipe_input"); }
  if (!nbytes) return;

  /* Null-terminate the buffer */
  *(pFinalStop=buf+nbytes) = '\0';

  /* Loop over buffer */
  for (pStart = buf; pStart<pFinalStop; ) {
  char saveChar;

    /* Find end of next string in buffer:  newline or null terminator */
    if (!(pStop=strchr(pStart, (int)'\n'))) {
      pStop = pStart + strlen(pStart);
    }

    /* Use no more than MYWRAP characters in buffer */
    if (MYWRAP < (pStop-pStart)) {
      pStop = pStart + MYWRAP;
    }

    /* Save stop character, null-terminate string */
    saveChar = *pStop;
    *pStop = '\0';

    /* Add string at, and scroll to, bottom of List widget */
    xmstring = XmStringCreateLocalized(pStart);
    XmListAddItemUnselected(listW, xmstring, 0);
    XmStringFree(xmstring);
    XmListSetBottomPos(listW, 0);

    /* Replace saved character that became null terminator */
    *pStop = saveChar;
    /* Advance pointer to start of next string, either to one past
     * newline or null terminator, or to saved character
     */
    pStart = pStop + ((saveChar == '\n' || saveChar == '\0') ? 1 : 0);
  }
  return;
}

/***********************************************************************
 * Create WorkArea widget, with one button
 */
void
make_work_area(Widget parentW) {
Widget work_areaW, pushBW;

  /* RowColumn widget */
  work_areaW = XtVaCreateManagedWidget( "input_work_area"
                                      , xmRowColumnWidgetClass, parentW
                                      , XmNpacking, XmPACK_COLUMN
                                      , XmNnumColumns, 1
                                      , XmNorientation, XmHORIZONTAL
                                      , NULL);
  /* PushButton widget in WorkArea */
  pushBW = XtVaCreateManagedWidget( "Click to write to STDERR"
                                  , xmPushButtonWidgetClass, work_areaW
                                  , NULL);
  /* Add callback which writes to STDERR; needs no client data */
  XtAddCallback(pushBW, XmNactivateCallback, work_area_CB, 0);

  /* Manage work area */
  return;
}

int
main(int argc, char** argv) {
XtAppContext app_context;
Widget appshell;
Widget the_cb;
Widget listW;
char sBase[MYWRAP];
int i;
int pypes2[2];

  /* Create pipe; pypes2[0] is for input; pypes2[1] is for output */
  if (pipe(pypes2) == -1) {
    fprintf(stderr, "Error creating pipe\n");
    return -1;
  }
  /* Redirect STDERR to output side of pipe, close unused fileno */
  dup2(pypes2[1], fileno(stderr));
  close(pypes2[1]);

  /* After this point, anything written to STDERR may be read from the
   * input side of the pipe via fileno pypes2[0]
   *
   * N.B. it is possible for this program to hang if a write to STDERR
   *      blocks e.g. there is a limit to how much may be written to
   *      STDERR and/or the pipe.
   */

  /* Create application shell and Command widget; add WorkArea to Command
   * widget; manage Command widget and its children
   */
  appshell = XtVaAppInitialize( &app_context, argv[0], NULL, 0, &argc, argv, NULL, NULL);
  the_cb = XmCreateCommand(appshell, "TheCmd", NULL, 0);
  make_work_area(the_cb);
  XtManageChild(the_cb);

  /* Hide Text and Selection widgets in Command widget */
  XtUnmanageChild(XtNameToWidget(the_cb, "Text"));
  XtUnmanageChild(XtNameToWidget(the_cb, "Selection"));

  /* Get List widget in Command widget, pass as client_data, along with
   * fileno pypes[0] (input side of pipe) to add [get_pipe_input] as an
   * input-triggered callback
   */
  listW = XtNameToWidget(the_cb, "ItemsListSW.ItemsList");
  XtAppAddInput(app_context, pypes2[0], (XtPointer) XtInputReadMask, get_pipe_input, listW);

  /* Write some test data to STDERR, which will not show up in the terminal
   * N.B. if MYWRAP and/or the amount of text written per pass through this
   *       loop are too large, this could block
   */
  for (i=0; i<(MYWRAP/5); ++i) {
    sprintf(sBase+i, "%c%c", (char) ('0'+(i%50)), '\0');
    fprintf(stderr, "%05d:[%s]\n", i, sBase);
  }
  /* Realize the app shell, start main loop, and return on exit from loop */
  XtRealizeWidget(appshell);
  XtAppMainLoop(app_context);
  return 0;
}
