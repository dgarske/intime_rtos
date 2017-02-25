/*****************************************************************************
* Util.c - Utility functions for the project
\*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <rt.h>

#include "wolfCryptTest.h"

/*****************************************************************************
* FUNCTION:		Catalog
*
* PARAMETERS:	1. handle of the process whose object directory must be used
*				2. the object whose handle must be cataloged
*				3. the name to be used (upto 14 characters)
*
* RETURNS:		TRUE on success
*
* DESCRIPTION:	If the given name already exists,
*				and the existing name refers to a non-existing object,
*				then the existing name is removed before cataloging.
\*****************************************************************************/
BOOLEAN Catalog(
	RTHANDLE			hProcess,
	RTHANDLE			hObject,
	LPSTR				lpszName)
{
	RTHANDLE		hOld;

	if (CatalogRtHandle(hProcess, hObject, lpszName))
		return TRUE;

	// something wrong: check for the case mentioned above
	if (((hOld = LookupRtHandle(hProcess, lpszName, NO_WAIT)) != BAD_RTHANDLE) &&
		(GetRtHandleType(hOld) == INVALID_TYPE))
	{
		// this is the case mentioned above: remove the old entry and try again
		if (UncatalogRtHandle(hProcess, lpszName))
			return (CatalogRtHandle(hProcess, hObject, lpszName));
	}
	return FALSE;
}

/*****************************************************************************
* FUNCTION:   Cleanup (local function)
*
* DESCRIPTION:
*  Tell threads to delete themselves and wait a while;
*  if any thread still exists, kill it.
*  Remove all other objects as far as they have been created.
\*****************************************************************************/
void Cleanup(void)
{
	// indicate that we are cleaning up
	gInit.state		= CLEANUP_BUSY;
	gInit.bShutdown = TRUE;

#ifdef _DEBUG
  fprintf(stderr, "wolfCrypt_test started cleaning up\n");
#endif

	// remove our name from the root process
	if (gInit.bCataloged)
		if (!UncatalogRtHandle(hRootProcess, "wolfCrypt_te"))
			Fail("Cannot remove my own name");

#ifdef _DEBUG
	fprintf(stderr, "wolfCrypt_test finished cleaning up\n");
#endif

	// lie down
	exit(0);
}

/*****************************************************************************
* FUNCTION:     	Fail
*
* PARAMETERS:   	same parameters as expected by printf
*
* DESCRIPTION:
*  If in debug mode, prints the message, appending a new line and the error number.
*  Then the current process is killed graciously:
*  If the current thread is the main thread, this is done directly.
*  if the current thread is another one, a terminate request is sent and
*  the function returns to the calling thread.
\*****************************************************************************/
void Fail(LPSTR lpszMessage, ...)
{
	EXCEPTION		eh;
	RTHANDLE		hDelMbx;
	DWORD			dwTerminate;

#ifdef _DEBUG
	va_list			ap;

	va_start(ap, lpszMessage);
	vfprintf(stderr, lpszMessage, ap);
	va_end(ap);
	fprintf(stderr, "\nError nr=%x %s\n", GetLastRtError(), GetRtErrorText(GetLastRtError()));
#endif

	// make sure that exceptions are returned for inline handling
	GetRtExceptionHandlerInfo(THREAD_HANDLER, &eh);
	eh.ExceptionMode = 0;
	SetRtExceptionHandler(&eh);

	// if we had not started initializing yet, just get out
	if (BEFORE_INIT == gInit.state)
		exit(0);

	if (gInit.hMain == GetRtThreadHandles(THIS_THREAD))
	{
		// this is the main thread:
		// if we are busy initializing, then do Cleanup
		if (INIT_BUSY == gInit.state)
			Cleanup();  // does not return

		// this is the main thread, but we are not initializing: just return
		return;
	}

	// this is not the main thread:
	// ask main thread to do cleanup
	// (allow some time to setup the deletion mailbox, ignore errors)
	hDelMbx			= LookupRtHandle(NULL_RTHANDLE, "R?EXIT_MBOX", 5000);
	dwTerminate		= TERMINATE;
	SendRtData(hDelMbx, &dwTerminate, 4);
}

/*****************************************************************************
*
* FUNCTION:		UsecsToKticks
*
* PARAMETERS:	1. number of usecs
*
* RETURNS:		number of low level ticks
*
* DESCRIPTION:	returns the parameter if it is WAIT_FOREVER
*				otherwise rounds up to number of low level ticks
\*****************************************************************************/
DWORD				UsecsToKticks(
	DWORD				dwUsecs)
{
	if (dwUsecs == WAIT_FOREVER)
		return WAIT_FOREVER;

	return (dwUsecs + dwKtickInUsecs - 1) / dwKtickInUsecs;
}
