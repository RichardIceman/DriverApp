// FilterDriverApp.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "FilterDriverApp.h"

#define MAX_LOADSTRING 100
#define IOCTL_CHANGE_BLOCK_LIST 0

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

BOOL                InitInterface(HWND hWnd);
VOID                DoStartSvc(HWND hWnd, LPCWSTR szSvcName);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FILTERDRIVERAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FILTERDRIVERAPP));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FILTERDRIVERAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FILTERDRIVERAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
      CW_USEDEFAULT, 0, 600, 403, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void updateInfo(HWND hList, block_list_t* head)
{
    SendMessage(hList, LB_RESETCONTENT, 0, 0);

    if (head == NULL) return;

    SendMessage(hList, WM_SETREDRAW, FALSE, 0);

    LPWSTR buff = new WCHAR[20];

    while (head)
    {
        wsprintf(buff, L"%4.0d%4.0d%4.0d%4.0d", FOURTH_IPADDRESS(head->ip), THIRD_IPADDRESS(head->ip), SECOND_IPADDRESS(head->ip), FIRST_IPADDRESS(head->ip));
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buff);
        head = head->next;
    }

    SendMessage(hList, WM_SETREDRAW, TRUE, 0);
}

DWORD getIPfromEdit(HWND hEditIP)
{
    DWORD dwAddr;

    SendMessage(hEditIP, IPM_GETADDRESS, 0, (LPARAM)&dwAddr);

    DWORD reverse = 0;

    reverse |= FIRST_IPADDRESS(dwAddr);
    reverse |= SECOND_IPADDRESS(dwAddr) << 8;
    reverse |= THIRD_IPADDRESS(dwAddr) << 16;
    reverse |= FOURTH_IPADDRESS(dwAddr) << 24;

    return reverse;
}

block_list_t* addIP(HWND hList, block_list_t* head, DWORD ip)
{
    block_list_t* newElement = (block_list_t*)malloc(sizeof(block_list_t));
    newElement->ip = ip;
    newElement->next = head;

    updateInfo(hList, newElement);

    return newElement;
}



void removeList(HWND hList, block_list_t* head)
{
    if (head == NULL) return;

    block_list_t* next;

    while (head)
    {
        next = head->next;
        free(head);
        head = next;
    }

    updateInfo(hList, NULL);
}

block_list_t* deleteIP(HWND hList, block_list_t* head, DWORD IP)
{
    if (head == NULL) return head;

    block_list_t* current = head;

    if (current->ip == IP)
    {
        current = current->next;
        free(head);

        updateInfo(hList, current);

        return current;
    }

    block_list_t* prev = head;
    current = head->next;

    while (current)
    {
        if (current->ip == IP)
        {
            prev->next = current->next;
            free(current);

            updateInfo(hList, head);

            return head;
        }
        current = current->next;
    }
    return head;
}

BOOLEAN isContain(block_list_t* head, DWORD ip)
{
    while (head)
    {
        if (head->ip == ip) return TRUE;
        head = head->next;
    }
    return FALSE;
}

BOOLEAN sendMsgToDraiver(HWND hWnd, TYPE_CHANGE type, DWORD IP)
{
    HANDLE hDriver = CreateFileA("\\\\.\\ndislwf", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hDriver == INVALID_HANDLE_VALUE)
    {
        MessageBox(hWnd, L"Ошибка подключения", L"", MB_OK);
        return FALSE;
    }

    CHANGE_LIST_IP_INFO*info = (CHANGE_LIST_IP_INFO*)malloc(sizeof(CHANGE_LIST_IP_INFO));

    info->type = type;
    info->IP = IP;

    DeviceIoControl(hDriver, IOCTL_CHANGE_BLOCK_LIST, info, sizeof(CHANGE_LIST_IP_INFO), 0, 0, 0, 0);
    CloseHandle(hDriver);

    return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hEditIPReceive;
    static HWND hEditIPSend;
    static HWND hListReceive;
    static HWND hListSend;
    static block_list_t* headReceive;
    static block_list_t* headSend;

    switch (message)
    {
    case WM_CREATE:
        if (!InitInterface(hWnd))
            return EXIT_FAILURE;

        hEditIPReceive = GetDlgItem(hWnd, ID_TEXTBOX_1);
        hEditIPSend = GetDlgItem(hWnd, ID_TEXTBOX_2);
        hListReceive = GetDlgItem(hWnd, ID_LISTBOX_1);
        hListSend = GetDlgItem(hWnd, ID_LISTBOX_2);
        headReceive = NULL;
        headSend = NULL;

        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case ID_BUTTON_ADD_1:
            {
                DWORD ip = getIPfromEdit(hEditIPReceive);
                if (!isContain(headReceive, ip))
                {
                    if (sendMsgToDraiver(hWnd, ADD_RECEIVE_IP, ip))
                    {
                        headReceive = addIP(hListReceive, headReceive, ip);
                    }
                }
            }
                break;
            case ID_BUTTON_DELETE_1:  
            {
                DWORD ip = getIPfromEdit(hEditIPReceive);

                headReceive = deleteIP(hListReceive, headReceive, ip);
                sendMsgToDraiver(hWnd, DELETE_RECEIVE_IP, ip);
            }
                break;
            case ID_BUTTON_CLEAR_1:
            {
                removeList(hListReceive,headReceive);
                headReceive = NULL;
                sendMsgToDraiver(hWnd, CLEAR_RECEIVE_IP, 0);
            }
            break;
            case ID_BUTTON_ADD_2:
            {
                DWORD ip = getIPfromEdit(hEditIPSend);
                if (!isContain(headSend, ip))
                {
                    if (sendMsgToDraiver(hWnd, ADD_SEND_IP, ip))
                    {
                        headSend = addIP(hListSend, headSend, ip);
                    }
                }
            }
            break;
            case ID_BUTTON_DELETE_2:
            {
                DWORD ip = getIPfromEdit(hEditIPSend);

                headSend = deleteIP(hListSend, headSend, ip);
                sendMsgToDraiver(hWnd, DELETE_SEND_IP, ip);
            }
            break;
            case ID_BUTTON_CLEAR_2:
            {
                removeList(hListSend, headSend);
                headSend = NULL;
                sendMsgToDraiver(hWnd, CLEAR_SEND_IP, 0);
            }
            break;
            case ID_START_SERVICE:
                DoStartSvc(hWnd, L"ndislwf");
                break;
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
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
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

BOOL InitInterface(HWND hWnd)
{
    RECT clientRect{};
    GetClientRect(hWnd, &clientRect);

    const int DEFAULT_HEIGHT = 20;
    const SIZE spaceBtwControls{ 5,5 };

    SIZE sizeLabel{ 120, DEFAULT_HEIGHT };
    SIZE sizeListBox{ (clientRect.right - spaceBtwControls.cx * 3)/2, clientRect.bottom - spaceBtwControls.cy * 2 - DEFAULT_HEIGHT * 2};
    SIZE sizeTextBox{ sizeListBox.cx - sizeLabel.cx - spaceBtwControls.cx , DEFAULT_HEIGHT };
    SIZE sizeButton{ (sizeListBox.cx - 2 * spaceBtwControls.cx) / 3, DEFAULT_HEIGHT };
  
    POINT posLabel1{ spaceBtwControls.cx, 0 };
    POINT posTextBox1{ posLabel1.x + sizeLabel.cx + spaceBtwControls.cx, posLabel1.y };
    POINT posListBox1{ posLabel1.x, posLabel1.y + sizeLabel.cy + sizeButton.cy + 2 * spaceBtwControls.cy };
    POINT posButtonClear1{ posLabel1.x,posLabel1.y + sizeLabel.cy + spaceBtwControls.cy };
    POINT posButtonDelete1{ posButtonClear1.x + sizeButton.cx + spaceBtwControls.cx, posButtonClear1.y };
    POINT posButtonAdd1{ posButtonDelete1.x + sizeButton.cx + spaceBtwControls.cx, posButtonClear1.y };

    POINT posLabel2{ posListBox1.x + sizeListBox.cx + spaceBtwControls.cx, 0 };
    POINT posTextBox2{ posLabel2.x + sizeLabel.cx + spaceBtwControls.cx, posLabel2.y };
    POINT posListBox2{ posLabel2.x, posLabel2.y + sizeLabel.cy + sizeButton.cy + 2 * spaceBtwControls.cy };
    POINT posButtonClear2{ posLabel2.x,posLabel2.y + sizeLabel.cy + spaceBtwControls.cy };
    POINT posButtonDelete2{ posButtonClear2.x + sizeButton.cx + spaceBtwControls.cx, posButtonClear2.y };
    POINT posButtonAdd2{ posButtonDelete2.x + sizeButton.cx + spaceBtwControls.cx, posButtonClear2.y };

    HWND label1 = CreateWindowEx(0, L"STATIC", L"IP на прием:", WS_CHILD | WS_VISIBLE, posLabel1.x, posLabel1.y, sizeLabel.cx, sizeLabel.cy, hWnd, reinterpret_cast<HMENU>(ID_LABEL_1), 0, 0);
    if (label1 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hEdit1 = CreateWindow(WC_IPADDRESS, NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, posTextBox1.x, posTextBox1.y, sizeTextBox.cx, sizeTextBox.cy, hWnd, reinterpret_cast<HMENU>(ID_TEXTBOX_1), hInst, 0);
    if (hEdit1 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hList1 = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, posListBox1.x, posListBox1.y, sizeListBox.cx, sizeListBox.cy, hWnd, reinterpret_cast<HMENU>(ID_LISTBOX_1), hInst, 0);
    if (hList1 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hwndButton1 = CreateWindow(L"BUTTON", L"Добавить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, posButtonAdd1.x, posButtonAdd1.y, sizeButton.cx, sizeButton.cy, hWnd, reinterpret_cast<HMENU>(ID_BUTTON_ADD_1), hInst, NULL);
    if (hwndButton1 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hwndButton2 = CreateWindow(L"BUTTON", L"Удалить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, posButtonDelete1.x, posButtonDelete1.y, sizeButton.cx, sizeButton.cy, hWnd, reinterpret_cast<HMENU>(ID_BUTTON_DELETE_1), hInst, NULL);
    if (hwndButton2 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hwndButton3 = CreateWindow(L"BUTTON", L"Очистить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, posButtonClear1.x, posButtonClear1.y, sizeButton.cx, sizeButton.cy, hWnd, reinterpret_cast<HMENU>(ID_BUTTON_CLEAR_1), hInst, NULL);
    if (hwndButton2 == INVALID_HANDLE_VALUE) return FALSE;

    
    
    HWND label2 = CreateWindowEx(0, L"STATIC", L"IP на передачу:", WS_CHILD | WS_VISIBLE, posLabel2.x, posLabel2.y, sizeLabel.cx, sizeLabel.cy, hWnd, reinterpret_cast<HMENU>(ID_LABEL_2), 0, 0);
    if (label1 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hEdit2 = CreateWindow(WC_IPADDRESS, NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, posTextBox2.x, posTextBox2.y, sizeTextBox.cx, sizeTextBox.cy, hWnd, reinterpret_cast<HMENU>(ID_TEXTBOX_2), hInst, 0);
    if (hEdit1 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hList2 = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, posListBox2.x, posListBox2.y, sizeListBox.cx, sizeListBox.cy, hWnd, reinterpret_cast<HMENU>(ID_LISTBOX_2), hInst, 0);
    if (hList1 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hwndButton4 = CreateWindow(L"BUTTON", L"Добавить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, posButtonAdd2.x, posButtonAdd2.y, sizeButton.cx, sizeButton.cy, hWnd, reinterpret_cast<HMENU>(ID_BUTTON_ADD_2), hInst, NULL);
    if (hwndButton4 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hwndButton5 = CreateWindow(L"BUTTON", L"Удалить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, posButtonDelete2.x, posButtonDelete2.y, sizeButton.cx, sizeButton.cy, hWnd, reinterpret_cast<HMENU>(ID_BUTTON_DELETE_2), hInst, NULL);
    if (hwndButton5 == INVALID_HANDLE_VALUE) return FALSE;

    HWND hwndButton6 = CreateWindow(L"BUTTON", L"Очистить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, posButtonClear2.x, posButtonClear2.y, sizeButton.cx, sizeButton.cy, hWnd, reinterpret_cast<HMENU>(ID_BUTTON_CLEAR_2), hInst, NULL);
    if (hwndButton6 == INVALID_HANDLE_VALUE) return FALSE;
}






VOID DoStartSvc(HWND hWnd, LPCWSTR szSvcName)
{
    SERVICE_STATUS_PROCESS ssStatus;
    DWORD dwOldCheckPoint;
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwBytesNeeded;
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
         
    // Get a handle to the SCM database. 

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // servicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 

    if (NULL == schSCManager)
    {
        return;
    }

    // Get a handle to the service.

    schService = OpenService(
        schSCManager,         // SCM database 
        szSvcName,            // name of service 
        SERVICE_ALL_ACCESS);  // full access 

    if (schService == NULL)
    {
        CloseServiceHandle(schSCManager);
        return;
    }

    // Check the status in case the service is not stopped. 

    if (!QueryServiceStatusEx(
        schService,                     // handle to service 
        SC_STATUS_PROCESS_INFO,         // information level
        (LPBYTE)&ssStatus,             // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &dwBytesNeeded))              // size needed if buffer is too small
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Check if the service is already running. It would be possible 
    // to stop the service here, but for simplicity this example just returns. 

    if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
    {
        MessageBox(hWnd, L"Service is already started.", L"", MB_OK);
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Save the tick count and initial checkpoint.

    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.

    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
    {
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 

        dwWaitTime = ssStatus.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        // Check the status until the service is no longer stop pending. 

        if (!QueryServiceStatusEx(
            schService,                     // handle to service 
            SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE)&ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded))              // size needed if buffer is too small
        {
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return;
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint)
        {
            // Continue to wait and check.

            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
            {
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return;
            }
        }
    }

    // Attempt to start the service.

    if (!StartService(
        schService,  // handle to service 
        0,           // number of arguments 
        NULL))      // no arguments 
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Check the status until the service is no longer start pending. 

    if (!QueryServiceStatusEx(
        schService,                     // handle to service 
        SC_STATUS_PROCESS_INFO,         // info level
        (LPBYTE)&ssStatus,             // address of structure
        sizeof(SERVICE_STATUS_PROCESS), // size of structure
        &dwBytesNeeded))              // if buffer too small
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return;
    }

    // Save the tick count and initial checkpoint.

    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
    {
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth the wait hint, but no less than 1 second and no 
        // more than 10 seconds. 

        dwWaitTime = ssStatus.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        // Check the status again. 

        if (!QueryServiceStatusEx(
            schService,             // handle to service 
            SC_STATUS_PROCESS_INFO, // info level
            (LPBYTE)&ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded))              // if buffer too small
        {
           
            break;
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint)
        {
            // Continue to wait and check.

            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
            {
                // No progress made within the wait hint.
                break;
            }
        }
    }

    // Determine whether the service is running.

    if (ssStatus.dwCurrentState == SERVICE_RUNNING)
    {
        MessageBox(hWnd, L"Service started successfully.", L"", MB_OK);
    }
    else
    {
        MessageBox(hWnd, L"Service not started.", L"", MB_OK);
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}