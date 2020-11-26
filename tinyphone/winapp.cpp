#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>
#include <ctime>
#include "resource.h"
#include "guicon.h"
#include "server.h"
#include "utils.h"
#include "net.h"
#include "consts.h"
#include "config.h"
#include "log.h"
#include "splash.h"
#include "tpendpoint.h"
#include <iphlpapi.h>
#include <algorithm> 
#include "app.hpp"

#ifdef _DEBUG
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Debug-Static.lib")
#else
#pragma comment(lib, "libpjproject-i386-Win32-vc14-Release-Static.lib")
#endif

#pragma comment(lib, "IPHLPAPI.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#define WM_SYSICON  WM_APP
#define MAX_TOOLTIP_LENGTH 96

using namespace std;
using namespace pj;
using namespace tp;

/*variables*/
UINT WM_TASKBAR = 0;
HWND Hwnd;
HMENU Hmenu;
NOTIFYICONDATA notifyIconData;
TCHAR szTIP[MAX_TOOLTIP_LENGTH] = TEXT("Strowger TinyPhone");
char szClassName[] = "TinyPhone";
SPLASH splashScreen;

/*procedures  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void InitNotifyIconData();
void ExitApplication();


int WINAPI WinMain(HINSTANCE hThisInstance,		
	HINSTANCE hPrevInstance,
	LPSTR lpszArgument,
	int nCmdShow)
{
	MSG messages;
	Hwnd = CreateDialog(
		hThisInstance,
		MAKEINTRESOURCE(IDD_EMPTY_DIALOG),
		NULL,
		(DLGPROC)WindowProcedure
	);

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

#ifdef _DEBUG
	RedirectIOToConsole();
#endif

	/*Initialize the NOTIFYICONDATA structure only once*/
	InitNotifyIconData();

	splashScreen.Init(Hwnd, hThisInstance, IDB_SPLASH);
	splashScreen.Show();

	//Run the server in non-ui thread.
	std::thread thread_object([]() {
		tp::StartApp();
		ExitApplication();
		PostQuitMessage(0);
		exit(0);
	});

	splashScreen.Hide();

	/* Run the message loop. It will run until GetMessage() returns 0 */
	while (GetMessage(&messages, NULL, 0, 0))
	{
		/* Translate virtual-key messages into character messages */
		TranslateMessage(&messages);
		/* Send message to WindowProcedure */
		DispatchMessage(&messages);
	}

	return messages.wParam;
}

void BrowseToFile(LPCTSTR filename)
{
	ITEMIDLIST *pidl = ILCreateFromPath(filename);
	if (pidl) {
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_CLOSE:
		notifyIconData.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
		return 0;
		break;
	case WM_DESTROY:
		ExitApplication();
		exit(0);
		break;
		// Our user defined WM_SYSICON message.
	case WM_SYSICON:
	{

	case WM_CONTEXTMENU:
		switch (lParam)
		{
		case WM_LBUTTONUP:
			break;
		case WM_LBUTTONDBLCLK:
			break;
		case WM_RBUTTONDOWN:

			Hmenu = CreatePopupMenu();
			auto local_address = "Local IP: " + local_ip_address();
			AppendMenu(Hmenu, MF_STRING | MF_DISABLED, ID_TRAY_IP, TEXT(local_address.c_str()));
			AppendMenu(Hmenu, MF_SEPARATOR, 0, TEXT(""));
			AppendMenu(Hmenu, MF_STRING, ID_TRAY_LOGDIR, TEXT("View Logs"));
			AppendMenu(Hmenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));
			//DestroyMenu(hMenu);

			// Get current mouse position.
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(Hwnd);

			// TrackPopupMenu blocks the app until TrackPopupMenu returns
			UINT clicked = TrackPopupMenu(Hmenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);

			SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
			switch (clicked)
			{
			case ID_TRAY_EXIT:
				ExitApplication();
				PostQuitMessage(0);
				exit(0);
				break;
			case ID_TRAY_LOGDIR:
			{
				cout << "Open Folder " << GetLogDir() << endl;
				BrowseToFile(tp::sipLogFile.c_str());
			}
				break;
			default:
				break;
			}
		}
	}
	break;

	case WM_SYSCOMMAND:
		/*In WM_SYSCOMMAND messages, the four low-order bits of the wParam parameter
		are used internally by the system. To obtain the correct result when testing the value of wParam,
		an application must combine the value 0xFFF0 with the wParam value by using the bitwise AND operator.*/
		switch (wParam & 0xFFF0)
		{
		case SC_MINIMIZE:
		case SC_CLOSE:
			//minimize();
			return 0;
			break;
		}
		break;

		// intercept the hittest message..
	case WM_NCHITTEST:
	{
		UINT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
		if (uHitTest == HTCLIENT)
			return HTCAPTION;
		else
			return uHitTest;
	}


	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


void InitNotifyIconData()
{
	memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));

	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = Hwnd;
	notifyIconData.uID = ID_TRAY_APP_ICON;
	notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIconData.uCallbackMessage = WM_SYSICON; //Set up our invented Windows Message
	notifyIconData.hIcon = (HICON)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

	try {
		std::string productVersion;
		GetProductVersion(productVersion);
		char buff[MAX_TOOLTIP_LENGTH];
		snprintf(buff, sizeof(buff), "%s %s", szTIP, productVersion.c_str());
		strncpy_s(notifyIconData.szTip, buff, sizeof(buff));
	}
	catch (...) {
		strncpy_s(notifyIconData.szTip, szTIP, sizeof(szTIP));
	}

	Shell_NotifyIcon(NIM_ADD, &notifyIconData);
}

namespace tp {

std::vector<std::string> GetLocalDNSServers() {
	FIXED_INFO *pFixedInfo;
	ULONG ulOutBufLen;
	DWORD dwRetVal;
	IP_ADDR_STRING *pIPAddr;
	std::vector <std::string> dnsServers;


	pFixedInfo = (FIXED_INFO *)MALLOC(sizeof(FIXED_INFO));
	if (pFixedInfo == NULL) {
		printf("Error allocating memory needed to call GetNetworkParams\n");
		return dnsServers;
	}
	ulOutBufLen = sizeof(FIXED_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetNetworkParams(pFixedInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pFixedInfo);
		pFixedInfo = (FIXED_INFO *)MALLOC(ulOutBufLen);
		if (pFixedInfo == NULL) {
			printf("Error allocating memory needed to call GetNetworkParams\n");
			return dnsServers;
		}
	}

	if (dwRetVal = GetNetworkParams(pFixedInfo, &ulOutBufLen) == NO_ERROR) {

		printf("Host Name: %s\n", pFixedInfo->HostName);
		
		printf("DNS Servers:\n");
		dnsServers.push_back(pFixedInfo->DnsServerList.IpAddress.String);
		printf("\t%s\n", pFixedInfo->DnsServerList.IpAddress.String);

		pIPAddr = pFixedInfo->DnsServerList.Next;
		while (pIPAddr != NULL) {
			printf("\t%s\n", pIPAddr->IpAddress.String);
			dnsServers.push_back(pIPAddr->IpAddress.String);
			pIPAddr = pIPAddr->Next;
		}
	}
	else {
		printf("GetNetworkParams failed with error: %d\n", dwRetVal);
		return dnsServers;
	}


	if (pFixedInfo != NULL)
		FREE(pFixedInfo);


	return dnsServers;
}

}

void ExitApplication() {
#ifdef _DEBUG
	CloseConsole();
#endif
	notifyIconData.uFlags = 0;
	Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
	tp::StopApp();
}
