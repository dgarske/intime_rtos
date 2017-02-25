/*****************************************************************************
* wolfCryptTest.c -	This is the main program module
\*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <rt.h>

#include "wolfCryptTest.h"
#include "wolfssl/wolfcrypt/settings.h"
#include "wolfcrypt/test/test.h"

// configuration
#define WOLFCRYPT_TEST_STACK	131072

// global variables
RTHANDLE			hRootProcess;
DWORD				dwKtickInUsecs;
INIT_STRUCT			gInit;

typedef struct func_args {
	int    argc;
	char** argv;
	int    return_code;
} func_args;

// polling thread function
void wolfCryptTestThread(void* param)
{
	func_args args = { 0 };
	int test_num = 0;

	do
	{
		printf("\nCrypt Test %d:\n", test_num);
		wolfcrypt_test(&args);
		printf("Crypt Test %d: Return code %d\n", test_num, args.return_code);

		test_num++;
	} while (args.return_code == 0);

}


/*****************************************************************************
* FUNCTION:			main
*
* DESCRIPTION:
*  This is the main program module.
*  It creates global objects  and all threads.
*  The main thread then waits for notifications and acts accordingly
\*****************************************************************************/
int main(int argc, char* argv[])
{
	SYSINFO			sysinfo;
	EVENTINFO		eiEventInfo;
#ifdef _DEBUG
	fprintf(stderr, "wolfCrypt_test started\n");
#endif

	// obtain handle of root process (cannot fail)
	hRootProcess	= GetRtThreadHandles(ROOT_PROCESS);

	// initialize the structure for cleaning up
	memset(&gInit, 0, sizeof(gInit));
	gInit.state		= BEFORE_INIT;

	// get low level tick length in usecs
	if (!CopyRtSystemInfo(&sysinfo))
		Fail("Cannot copy system info");
	dwKtickInUsecs	= 10000 / sysinfo.KernelTickRatio;
	if (dwKtickInUsecs == 0)
		Fail("Invalid low level tick length");

	// adjust process max priority (ignore error)
	// TODO adjust the 2nd parameter to a value closer to zero if you want to allow more priorities
	SetRtProcessMaxPriority(NULL_RTHANDLE, 150);

	// obtain main thread's handle
	gInit.hMain		= GetRtThreadHandles(THIS_THREAD);
	gInit.state		= INIT_BUSY;

	// attempt to catalog the thread but ignore error
	Catalog(NULL_RTHANDLE, gInit.hMain, "TMain");

	// catalog the handle of this process in the root process
	if (!Catalog(hRootProcess, GetRtThreadHandles(THIS_PROCESS), "wolfCryptTes"))
		Fail("Cannot catalog process name");
	gInit.bCataloged = TRUE;

	// create poll threads
	if (BAD_RTHANDLE == CreateRtThread(170, (LPPROC)wolfCryptTestThread, WOLFCRYPT_TEST_STACK, 0))
		Fail("Cannot create poll thread Poll1");

	// indicate that initialization has finished
	gInit.state		= INIT_DONE;
#ifdef _DEBUG
	fprintf(stderr, "wolfCrypt_test finished initialization\n");
#endif

	// wait for notifications
	while (RtNotifyEvent(RT_SYSTEM_NOTIFICATIONS | RT_EXIT_NOTIFICATIONS,
		WAIT_FOREVER, &eiEventInfo))
	{
		switch(eiEventInfo.dwNotifyType)
		{
		case TERMINATE:
			// TODO: this process should terminate
			// cleanup the environment
			Cleanup();  // does not return

		case NT_HOST_UP:
			// TODO: react to a Windows host that has come back
			break;

		case NT_BLUESCREEN:
			// TODO: react to a Windows blue screen
			break;

		case KERNEL_STOPPING:
			// TODO: react to the INtime kernel stopping
			break;

		case NT_HOST_HIBERNATE:
			// TODO: react to the Windows host going in hibernation
			break;

		case NT_HOST_STANDBY:
			// TODO: react to the Windows host going in standby mode
			break;

		case NT_HOST_SHUTDOWN_PENDING:
			// TODO: react to a Windows host that is about to shutdown
			break;
		}
	}
	Fail("Notify failed");
	return 0;
}
