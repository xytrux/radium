#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Shlobj.h>

void runDiskpart(const char *driveLetter)
{
    char cmd[512];
    sprintf(cmd, "echo select volume %s > diskpart.txt && echo clean >> diskpart.txt && echo create partition primary >> diskpart.txt && echo format fs=ntfs quick >> diskpart.txt && echo assign >> diskpart.txt && diskpart /s diskpart.txt", driveLetter);
    system(cmd);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static HWND hwndButton;
    static char driveLetter[MAX_PATH];

    switch (msg)
    {
    case WM_CREATE:
        hwndButton = CreateWindow("BUTTON", "Select Drive", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                  10, 10, 100, 30, hwnd, (HMENU)1, NULL, NULL);
        break;
    case WM_COMMAND:
        if (LOWORD(wp) == 1)
        {
            BROWSEINFO bi = {0};
            bi.lpszTitle = "Select a Drive:";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
            if (pidl != 0)
            {
                // Get the name of the folder
                SHGetPathFromIDList(pidl, driveLetter);

                // Free memory used
                IMalloc *imalloc = 0;
                if (SUCCEEDED(SHGetMalloc(&imalloc)))
                {
                    imalloc->lpVtbl->Free(imalloc, pidl);
                    imalloc->lpVtbl->Release(imalloc);
                }

                // Get the drive letter from the selected path
                driveLetter[3] = '\0';

                int msgboxID = MessageBox(NULL, "Are you sure you want to wipe this drive?", "Confirmation", MB_ICONEXCLAMATION | MB_YESNO);
                if (msgboxID == IDYES)
                {
                    runDiskpart(driveLetter);
                    MessageBox(NULL, "Drive wipe completed!", "Success", MB_ICONINFORMATION | MB_OK);
                }
            }
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "DiskpartWindowClass";

    WNDCLASS wc = {0};

    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Window registration failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    int width = 200;
    int height = 100;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int posX = (screenWidth - width) / 2;
    int posY = (screenHeight - height) / 2;

    HWND hwnd = CreateWindow(CLASS_NAME, "Diskpart GUI", WS_OVERLAPPEDWINDOW, posX, posY, width, height, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}