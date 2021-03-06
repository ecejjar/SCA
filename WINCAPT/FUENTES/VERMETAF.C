#include <windows.h>
#include <string.h>

/* Funciones asociadas a las clases de ventana */
long FAR PASCAL _export ProcVentApp (HWND, WORD, WORD, LONG);

/*컴컴컴컴컴컴컴컴컴횯efinici줻 de variables globales컴컴컴컴컴컴컴컴컴컴컴�*/

HANDLE 		  hInstGlobal, hMF;
char              szAppName[] = "Visualizador", path[80], nom_fich[13];

/*컴컴컴컴컴컴컴컴컴횮omienzo del programa principal컴컴컴컴컴컴컴컴컴컴컴컴*/

int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance,
		    LPSTR lpszCmdLine, int nCmdShow)
{
		 /* Definiciones de variables locales */

 WNDCLASS wndclass;
 MSG      msg;
 BOOL	  flag;

 /* Obtenemos el fichero del metafile */
 lstrcpy (path, lpszCmdLine);

 /* Obtenemos el MetaFile */
 hMF = GetMetaFile (path);

 /* Obtenemos el handle de la aplicaci줻 */
 hInstGlobal = hInstance;

 if (!hPrevInstance)
  {
   wndclass.style	  = CS_HREDRAW | CS_VREDRAW;
   wndclass.lpfnWndProc   = (WNDPROC) ProcVentApp;
   wndclass.cbClsExtra    = 0;
   wndclass.cbWndExtra    = 0;
   wndclass.hInstance	  = hInstance;
   wndclass.hIcon	  = LoadIcon (hInstance, szAppName);
   wndclass.hCursor	  = LoadCursor (NULL, IDC_ARROW);
   wndclass.hbrBackground = COLOR_APPWORKSPACE + 1;
   wndclass.lpszMenuName  = NULL;
   wndclass.lpszClassName = szAppName;

   RegisterClass (&wndclass);
  }

 /* Estamos en el bucle de mensaje hasta que se escoja 'Salir'  */
 while (GetMessage (&msg, NULL, 0, 0))
 {
  TranslateMessage (&msg);
  DispatchMessage (&msg);
 }

 /* Eliminamos el MetaFile */
 DeleteMetaFile (hMF);

 return (msg.wParam);
}

/*컴컴컴컴컴컴컴컴컴횮omienzo de la zona de funciones컴컴컴컴컴컴컴컴컴컴컴�*/

long FAR PASCAL _export ProcVentApp (HWND hwnd, WORD message,
				     WORD wParam, LONG lParam)
{
 static HANDLE      hInstance;
 PAINTSTRUCT        ps;

 switch (message)
 {
  case WM_CREATE:
  {
   /* Obtenemos el handle a la ocurrencia del m줰ulo, que se guarda forever */
   hInstance = ((LPCREATESTRUCT) lParam)->hInstance;

   return (0);
  }

  case WM_PAINT:
  {
   /* Obtenemos el contexto de la ventana */
   hdc = BeginPaint (hwnd, &ps);

   /* Redibujamos el MetaFile */
   PlayMetaFile (hdc, hMF);

   /* Liberamos el contexto */
   EndPaint (hwnd, &ps);

   return (0);
  }

  case WM_DESTROY:
  {
   PostQuitMessage (0);
   return (0);
  }
 }

 return (DefFrameProc (hwnd, hwndCliente, message, wParam, lParam));
}

/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴Fin del fichero컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�*/