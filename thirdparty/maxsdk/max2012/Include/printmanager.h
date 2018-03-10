// ****************************************************************************
//
// DESCRIPTION: Declaration of the Print Manager
// PATTERN    : Singleton
// CREATED BY : Michael Pittman
// HISTORY    : 12/21/1998
//
// ****************************************************************************
#pragma once

#include "coreexp.h"
#include <WTypes.h>
#include "strbasic.h"

class PrintManager
{
private:
    static PrintManager* m_instance;    // Singleton instance
    HDC                  m_hdc;         // Device context of printer
    int                  m_ncopies;     // Number of copies to print
    bool                 m_landscape;   // Print in landscape?  

	PRINTDLG             m_pinfo;       // Printer info from PrintDlg
    PAGESETUPDLG         m_pgsetup;     // Page Setup info from PageSetupDlg
    DEVMODE              m_devmode;     // Device mode structure
    MCHAR                m_driver[128]; // Printer driver name
    MCHAR                m_device[33];  // Printer device name
    bool                 m_use_pgsetup; // User has chosen page setup

    PrintManager();

	bool SetupPrintFromDialog(HWND parent);
	bool SetupPrintExisting(void);
	bool SetupPrintDefault(void);

public:
	enum PrinterChoice
	{
		k_UseDefault,
		k_PromptUser,
		k_UseExisting
	};

    ~PrintManager();

	// Access to the singleton
    CoreExport static PrintManager* Instance(void);

	// Query methods
	CoreExport HDC GetPrinterDC(PrinterChoice getfrom = k_PromptUser);
    CoreExport HDC GetDefaultPrinterDC(void) { return GetPrinterDC(k_UseDefault); }
    CoreExport HDC GetExistingPrinterDC(void) { return GetPrinterDC(k_UseExisting); }
    CoreExport void ReleasePrinterDC(HDC hdc);
    CoreExport int NumberCopies(void) { return m_ncopies; }
    CoreExport bool DoLandscape(void) { return m_landscape; }

    // The standard print methods interface
    CoreExport bool OnPageSetup(HWND parent);
};

