#include <windows.h>

HWND hwndEdit;

int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance,
		    LPSTR lpszCmdLine, int nCmdShow)
{
		  /* Definici¢n de variables locales */

 HICON       hIcono;
 MSG         msg;
 WORD        error, Escritorio;
 char 	     szAppName[] = "Editor";

 error = WinExec ("C:\\WINDOWS\\NOTEPAD.EXE", SW_SHOW);

 hwndEdit = FindWindow (NULL, "Bloc - (Sin título)");

 SetWindowText (hwndEdit, "Editor Programas D.C.E. - (Sin título)");

 if (!hPrevInstance)
  {
   hIcono = LoadIcon (hInstance, szAppName);
   SetClassWord (hwndEdit, GCW_HICON, hIcono);
   while (IsWindow (hwndEdit))
    {
     if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
      {
       TranslateMessage (&msg);
       DispatchMessage (&msg);
      }
    }
  }

 return (0);
}
