// SectionListSample.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "SectionListSample.h"
#include "STXSectionList.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
CSTXSectionList* pSectionList = nullptr;

void InitializeSectionList()
{
    int a0 = 0;
    int a1 = 0;
    int a2 = 0;
    int a3 = 0;

    LANGID lang = GetUserDefaultUILanguage();
    if (lang != 0x804)		//not Chinese
    {
        a0 = pSectionList->AddSection(_T("2014-1-25   Today          Click items to remove"));
        pSectionList->AddItemToSection(a0, _T("Apple"));
        pSectionList->AddItemToSection(a0, _T("Banana"));
        pSectionList->AddItemToSection(a0, _T("Bolwarra"));
        a1 = pSectionList->AddSection(_T("2014-1-26   Tomorrow"));
        pSectionList->AddItemToSection(a1, _T("Mangos"));
        pSectionList->AddItemToSection(a1, _T("Salak"));
        pSectionList->AddItemToSection(a1, _T("Genip"));
        pSectionList->AddItemToSection(a1, _T("Papayas"));
        pSectionList->AddItemToSection(a1, _T("Lapsi"));
        pSectionList->AddItemToSection(a1, _T("Lemon"));
        pSectionList->AddItemToSection(a1, _T("Lime"));
        pSectionList->AddItemToSection(a1, _T("Yellow Mombin"));
        a2 = pSectionList->AddSection(_T("2014-1-27   Please try resizing the window to see the items move with animation"));
        pSectionList->AddItemToSection(a2, _T("Youngberry"));
        a3 = pSectionList->AddSection(_T("2014-1-28"));
        pSectionList->AddItemToSection(a3, _T("Watermelon"));
    }
    else
    {
        a0 = pSectionList->AddSection(_T("2014年1月25日   今天    点击以下各项可删除"));
        pSectionList->AddItemToSection(a0, _T("Apple"));
        pSectionList->AddItemToSection(a0, _T("芭蕉"));
        pSectionList->AddItemToSection(a0, _T("菠萝蜜"));

        a1 = pSectionList->AddSection(_T("2014年1月26日   明天"));
        pSectionList->AddItemToSection(a1, _T("芒果"));
        pSectionList->AddItemToSection(a1, _T("山竹"));
        pSectionList->AddItemToSection(a1, _T("柑桔"));
        pSectionList->AddItemToSection(a1, _T("葡萄"));
        pSectionList->AddItemToSection(a1, _T("莲雾"));
        pSectionList->AddItemToSection(a1, _T("荔枝"));
        pSectionList->AddItemToSection(a1, _T("榴莲"));
        pSectionList->AddItemToSection(a1, _T("椰子"));

        a2 = pSectionList->AddSection(_T("2014年1月27日"));
        pSectionList->AddItemToSection(a2, _T("杨桃"));
        a3 = pSectionList->AddSection(_T("2014年1月28日"));
        pSectionList->AddItemToSection(a3, _T("Watermelon"));
    }

    pSectionList->SetItemTipCount(a0, 0, 6);
    pSectionList->SetItemTipCount(a0, 1, 12);
    pSectionList->SetItemTipCount(a0, 2, 0);
    pSectionList->SetItemTipCount(a1, 0, 3);
    pSectionList->SetItemTipCount(a1, 1, 2);
    pSectionList->SetItemTipCount(a1, 2, 0);
    pSectionList->SetItemTipCount(a1, 3, 20);
    pSectionList->SetItemTipCount(a1, 4, 0);
    pSectionList->SetItemTipCount(a1, 5, 5);
    pSectionList->SetItemTipCount(a1, 6, 16);
    pSectionList->SetItemTipCount(a1, 7, 32);
    pSectionList->SetItemTipCount(a2, 0, 9);
    pSectionList->SetItemTipCount(a3, 0, 0);

    pSectionList->SetItemTipLeftBottom(a0, 0, _T("$5.00"));
    pSectionList->SetItemTipLeftBottom(a0, 1, _T("$70.00"));
    pSectionList->SetItemTipLeftBottom(a0, 2, _T("$32.00"));
    pSectionList->SetItemTipLeftBottom(a1, 0, _T("$7.09"));
    pSectionList->SetItemTipLeftBottom(a1, 1, _T("$66.00"));
    pSectionList->SetItemTipLeftBottom(a1, 7, _T("$1999.00"));
    pSectionList->SetItemTipLeftBottom(a2, 0, _T("Free"));

    pSectionList->SetItemTipLeftTop(a0, 0, _T("A"));
    pSectionList->SetItemTipLeftTop(a0, 1, _T("B"));
    pSectionList->SetItemTipLeftTop(a0, 2, _T("B"));
    pSectionList->SetItemTipLeftTop(a1, 0, _T("M"));
    pSectionList->SetItemTipLeftTop(a1, 1, _T("S"));
    pSectionList->SetItemTipLeftTop(a1, 2, _T("G"));
    pSectionList->SetItemTipLeftTop(a1, 3, _T("P"));
    pSectionList->SetItemTipLeftTop(a1, 4, _T("L"));
    pSectionList->SetItemTipLeftTop(a1, 5, _T("L"));
    pSectionList->SetItemTipLeftTop(a1, 6, _T("L"));
    pSectionList->SetItemTipLeftTop(a1, 7, _T("Y"));
    pSectionList->SetItemTipLeftTop(a2, 0, _T("Y"));
    pSectionList->SetItemTipLeftTop(a3, 0, _T("W"));

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CoInitialize(nullptr);
    pSectionList = new CSTXSectionList();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SECTIONLISTSAMPLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        delete pSectionList;
        CoUninitialize();
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SECTIONLISTSAMPLE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    delete pSectionList;
    CoUninitialize();
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SECTIONLISTSAMPLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SECTIONLISTSAMPLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   RECT rcWindow;
   GetClientRect(hWnd, &rcWindow);
   pSectionList->Create(_T("SectionList"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | WS_CLIPCHILDREN,
       rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, hWnd, 1003);

   InitializeSectionList();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
    {
        RECT rcWindow;
        GetClientRect(hWnd, &rcWindow);
        MoveWindow(pSectionList->GetSafeHwnd(), rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
    }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
