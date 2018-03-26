/***********************************************************************
 * Proof of Concept:  redirect STDERR to List widget (XmList class) of
 *                    Command widget (XmCommand class) in X-Windows/Xm
 *                    application.
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
 *   This example program is a proof of concept only; this approach may
 *   not work in all situations.  These limitations could be handled in
 *   various ways with additional code, but that is beyond the scope of
 *   this proof of concept.
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

#define MYWRAP 180   /* Maximum line length in List widget; arbitrary */


/***********************************************************************
 * STDERR writer callback:  called when the button in the WorkArea widget
 *                          is pressed.
 *
 * - This is a proxy for an actual application that logs informational
 *   and/or error messages to STDERR.  This routine would not be
 *   required when applying this proof-of-concept to an actual
 *   application that already generates output to STDERR.
 *
 * - Writes 2 lines of data, to STDERR, per WorkArea button click
 */
void
stderr_writer_CB(Widget pb, XtPointer client_data, XtPointer call_data) {
static unsigned long iNext;

  /* Write two lines, comprising a counter and the current system time
   * (seconds past the Unix Epoch of 1970-01-01 00:00:00 +0000 (UTC)) to
   * STDERR.  The FFLUSH probably not needed as STDERR is not buffered.
   */
  fprintf(stderr, "Push offset %lu to STDERR:\n  => time = <%lu>\n", iNext, (unsigned long) time(0));
  fflush(stderr);

  /* Log to STDOUT; FFLUSH probably not needed as STDOUT is line-buffered */
  fprintf(stdout, "[STDOUT:] Wrote [Push offset %lu\\n  => time = <%lu>\\n] to STDERR\n", iNext++, (unsigned long) time(0));
  fflush(stdout);

  return;
} /* void stderr_writer_CB(...) */
/**********************************************************************/


/***********************************************************************
 * Create WorkArea widget, with one button, and a callback on that
 * button to stderr_writer_CB.  This WorkArea represents a proxy for an
 * application that writes to STDERR, and will not be needed in an
 * actual implementation of this proof of concept.
 *
 * This routine would not be required in applying this proof-of-concept
 * to an actual application that generates output to STDERR.
 */
Widget
add_work_area(Widget parentW) {
Widget work_areaW, pushBW;

  /* Create WorkArea as a RowColumn widget, placed in parent widget */
  work_areaW = XtVaCreateManagedWidget( "input_work_area"
                                      , xmRowColumnWidgetClass, parentW
                                      , XmNpacking, XmPACK_COLUMN
                                      , XmNnumColumns, 1
                                      , XmNorientation, XmHORIZONTAL
                                      , NULL);

  /* Add PushButton widget to WorkArea */
  pushBW = XtVaCreateManagedWidget( "Click to write to STDERR"
                                  , xmPushButtonWidgetClass, work_areaW
                                  , NULL);

  /* Add PushButton callback to, which writes to STDERR
   * - The callback needs no client data
   */
  XtAddCallback(pushBW, XmNactivateCallback, stderr_writer_CB, 0);

  return work_areaW;
} /* void add_work_area(Widget parentW) */
/**********************************************************************/


/***********************************************************************
 * XtAppAddInput callback, reads from *fid, the read end of the pipe
 * connected to STDERR; puts the text read into the List widget
 */
void
get_pipe_input_CB(XtPointer client_data, int* fid, XtInputId* id) {
Widget listW = (Widget) client_data;
char buf[BUFSIZ+1];
char* pStart;
char* pStop;
char* pFinalStop;
int nbytes;
XmString xmstring;

  /* Read from pipe; exit on error; return if nothing was read */
  if ((nbytes=read(*fid,buf,BUFSIZ)) == -1) { perror("get_pipe_input_CB"); }
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
} /* void get_pipe_input_CB(...) */
/**********************************************************************/


/**********************************************************************/
/* Proof of concept application for re-directing STDERR to a List widget
 *
 * - Only the six statements in [PART 1], the six statements in [PART 4]
 *   and [PART 4.1], and the get_pipe_input_CB(...) callback above, are
 *   all that is required to add this proof of concept to an actual
 *   application.
 *
 * - [PART 3] and [PART 6] are normal parts of X-Window/Motif
 *
 * - [PART 2], [PART 5], the add_work_area(...) call above, and the
 *   stderr_writer_CB(...) call above, are not required to implement this
 *   proof of concept; they are part of a synthetic proxy to represent a
 *   typical application that writes data to STDERR, which data one
 *   wishes to be re-directed to a List widget.
 */
int
main(int argc, char** argv) {
XtAppContext app_context;
Widget appshell;
Widget the_cbW;
Widget listW;
Widget work_areaW;
char sBase[MYWRAP];
int i;
int pypes2[2];


  /********************************************************************/
  /********   PART 1:  redirect STDERR   ******************************/
  /********************************************************************/

  /* Create pipe; pypes2[0] is for reading; pypes2[1] is for writing */
  if (pipe(pypes2) == -1) {
    fprintf(stderr, "Error creating pipe\n");
    return -1;
  }

  /* Redirect STDERR to the write end of pipe, close unused fileno */
  dup2(pypes2[1], fileno(stderr));
  close(pypes2[1]);

  /* After this point, anything written to STDERR may be read from the
   * read end of the pipe via fileno pypes2[0]
   *
   * N.B. it is possible for this program to hang if a write to STDERR
   *      blocks e.g. there is a limit to how much may be written to
   *      STDERR and/or the pipe.
   */


  /********************************************************************/
  /******   PART 2:  write some initial test data to STDERR   *********/
  /******   ****N.B. this section would not be needed in an   *********/
  /******            actual implementation of this proof of   *********/
  /******            concept.                                 *********/
  /********************************************************************/

  /* Write some test data to STDERR.  These data will be re-directed,
   * via the dup2(...) call above, to the write end of the pipe, which
   * was created by the pipe(...) call above.  Those data will
   * eventually be read from the read end of the pipe by the
   * get_pipe_input_CB(...) callback, which will in turn add those
   * redirected STDERR data to the List widget
   */
  for (i=0; i<(MYWRAP/5); ++i) {

    /* Expand sBase string with ASCII chars on each pass through loop */
    sBase[i]   = (char) ('0'+(i%50));
    sBase[i+1] = '\0';

    /* Write the loop counter and the sBase string to STDERR
     * N.B. if MYWRAP and/or the amount of text written per pass through
     *      this loop are too large, this fprint(...) call could block
     *      if this fills the pipe's buffer, because this
     *      proof-of-concept approach is single-threaded and these data
     *      will not be be read from the read end of the pipe until
     *      later.
     */
    fprintf(stderr, "%05d:[%s]\n", i, sBase);
  }


  /********************************************************************/
  /******   PART 3:  Initialize application shell   *******************/
  /********************************************************************/

  appshell = XtVaAppInitialize( &app_context, argv[0], NULL, 0, &argc, argv, NULL, NULL);


  /********************************************************************/
  /******   PART 4:  create application widgets   *********************/
  /********************************************************************/

  /* Initialize application shell; create and manage Command widget; 
   * un-manage unused children of Command widhet
   */
  the_cbW = XmCreateCommand(appshell, "STDERR_log", NULL, 0);
  XtManageChild(the_cbW);

  /* Hide Text and Selection widgets in Command widget */
  XtUnmanageChild(XtNameToWidget(the_cbW, "Text"));
  XtUnmanageChild(XtNameToWidget(the_cbW, "Selection"));


  /********************************************************************/
  /******   PART 4.1:  create callback to read pipe   *****************/
  /********************************************************************/

  /* Get List widget in Command widget, pass as client_data, along with
   * fileno pypes[0] (read end of pipe) to add [get_pipe_input_CB] as an
   * input-triggered callback.
   */
  listW = XtNameToWidget(the_cbW, "ItemsListSW.ItemsList");
  XtAppAddInput(app_context, pypes2[0], (XtPointer) XtInputReadMask, get_pipe_input_CB, listW);


  /********************************************************************/
  /******   PART 5:  add WorkArea as proxy for actual application   ***/
  /********************************************************************/

  /* Add user-defined WorkArea to that Command widget
   *
   * N.B. the WorkArea and its callback stderr_writer_CB(...) compose a
   *      proxy for an application that writes to STDERR; this WorkArea
   *      would not be needed in an actual implementation of thie proof
   *      of concept.
   */
  work_areaW = add_work_area(the_cbW);


  /********************************************************************/
  /******   PART 6:  start application event loop   *******************/
  /********************************************************************/

  /* Realize the app shell, start the main loop; return on loop exit */
  XtRealizeWidget(appshell);
  XtAppMainLoop(app_context);
  return 0;
} /* main(...) */
/**********************************************************************/
