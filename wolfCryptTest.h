// main include file for the wolfCrypt_test project

// support functions for all threads
BOOLEAN				Catalog(RTHANDLE hProcess, RTHANDLE hObject, LPSTR lpszName);
void				Cleanup(void);
void				Fail(LPSTR lpszMessage, ...);
DWORD				UsecsToKticks(DWORD dwUsecs);

// global type definitions
typedef enum {
	BEFORE_INIT,
	INIT_BUSY,
	INIT_DONE,
	CLEANUP_BUSY 
}					INIT_STATE;

typedef struct {
	RTHANDLE			hMain;		// RTHANDLE of main thread
	INIT_STATE			state;		// main thread state
	BOOLEAN				bCataloged;	// TRUE if we cataloged process name in root
	BOOLEAN				bShutdown;	// TRUE if all threads have to terminate
}					INIT_STRUCT;

// global variables
extern RTHANDLE		hRootProcess;	// RTHANDLE of root process
extern DWORD		dwKtickInUsecs;	// length of one low level tick in usecs
extern INIT_STRUCT	gInit;			// structure describing all global objects
