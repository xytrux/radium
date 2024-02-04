#include <windows.h>
#include <commdlg.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>

#include <windows.h>

void wipeDrive(const char *drivePath) {
    HANDLE driveHandle = CreateFile(drivePath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (driveHandle == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, "Error opening drive", "Error", MB_ICONERROR | MB_OK);
        return;
    }

    char *buffer = calloc(1, 512);
    if (buffer == NULL) {
        MessageBox(NULL, "Error allocating memory", "Error", MB_ICONERROR | MB_OK);
        CloseHandle(driveHandle);
        return;
    }

    DWORD written, totalWritten = 0;
    BOOL result;
    do {
        result = WriteFile(driveHandle, buffer, 512, &written, NULL);
        totalWritten += written;
    } while (result && written == 512);

    if (!result) {
        MessageBox(NULL, "Error writing to drive", "Error", MB_ICONERROR | MB_OK);
    }

    CloseHandle(driveHandle);
    free(buffer);
}

void SelectFileAndWrite(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)==TRUE) {
        // Check if the selected path is a drive
        int length = strlen(ofn.lpstrFile);
        if (length != 3 || ofn.lpstrFile[1] != ':' || ofn.lpstrFile[2] != '\\') {
            MessageBox(NULL, "Selected path is not a drive", "Error", MB_ICONERROR | MB_OK);
            return;
        }

        int msgboxID = MessageBox(NULL, (LPCSTR)"WARNING: This will wipe the drive you just chose. Press \"OK\" to proceed.", (LPCSTR)"Confirmation", MB_ICONWARNING | MB_OKCANCEL);
        if (msgboxID == IDOK) {
            // Convert the drive path to a format that CreateFile can understand
            char drivePath[7] = "\\\\.\\";
            strncat(drivePath, ofn.lpstrFile, 2);
            wipeDrive(drivePath);
        }
    }
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static HWND hwndButton;

    switch(msg)
    {
        case WM_CREATE:
            hwndButton = CreateWindow("BUTTON", "Select Drive", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                                      10, 10, 100, 30, hwnd, (HMENU)1, NULL, NULL);
            break;
        case WM_COMMAND:
            if (LOWORD(wp) == 1) {
                SelectFileAndWrite(hwnd);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "radiumWindowClass";

    WNDCLASS wc = { 0 };

    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Window registration failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    int width = 300;
    int height = 300;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int posX = (screenWidth - width) / 2;
    int posY = (screenHeight - height) / 2;

    HWND hwnd = CreateWindow(CLASS_NAME, "radium", WS_OVERLAPPEDWINDOW, posX, posY, width, height, NULL, NULL, hInst, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}