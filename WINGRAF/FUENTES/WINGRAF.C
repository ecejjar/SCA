#include <graficos.h>
#include <wingraf.h>

/*----------------Zona de definici¢n de variables globales------------------*/

HWND     hwnd;
char     szAppName[]     = "GRAFICOS",
	 szGrafClass[]   = "GRAFICA",
	 szColorClass[]  = "COLOR",
	 szCoordsClass[] = "COORDS";
short    cyLine, cyChar, nHposScrl, nVposScrl;
char     nom_fich[80];

extern HWND  hwndGraficas[10][5];
extern short numgraficas;
extern BOOL  Grafica, Tabla;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcVent           (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogAbrir    (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogTrazar   (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogAcerca   (HWND, WORD, WORD, LONG);

extern long FAR PASCAL _export ProcGrafica    (HWND, WORD, WORD, LONG);
extern long FAR PASCAL _export ProcVentColor  (HWND, WORD, WORD, LONG);
extern long FAR PASCAL _export ProcVentCoords (HWND, WORD, WORD, LONG);
extern int 		       Trazar_Grafica (int);

/* Las siguientes funciones duplican las est ndar de C para el ANSI Ch. Set */
LPSTR lstrchr  (LPSTR str, char ch);
LPSTR lstrrchr (LPSTR str, char ch);

/*-------------------Comienzo del programa principal------------------------*/

int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance,
		    LPSTR lpszCmdLine, int nCmdShow)
{
		  /* Definici¢n de variables locales */

 WNDCLASS    wndclass;
 MSG	     msg;

 FILE *config;

 /* Tomamos el path del circuito */
 if (config = fopen ("config.sca", "rt"))
  {
   fscanf (config, "%[^\n]", nom_fich);
   fclose (config);
  }
 else
   strcpy (nom_fich, PATH);

 /* Registramos la clase de la ventana principal y las de Gr fica y Color */
 if (!hPrevInstance)
  {
   wndclass.style	  = CS_HREDRAW | CS_VREDRAW;
   wndclass.lpfnWndProc   = ProcVent;
   wndclass.cbClsExtra    = 0;
   wndclass.cbWndExtra    = 0;
   wndclass.hInstance	  = hInstance;
   wndclass.hIcon	  = LoadIcon (hInstance, szAppName);
   wndclass.hCursor	  = LoadCursor (NULL, IDC_ARROW);
   wndclass.hbrBackground = GetStockObject (WHITE_BRUSH);
   wndclass.lpszMenuName  = NULL;
   wndclass.lpszClassName = szAppName;

   RegisterClass (&wndclass);

   wndclass.lpfnWndProc   = ProcGrafica;
   wndclass.cbWndExtra    = 10 * sizeof (WORD);
   wndclass.hIcon	  = LoadIcon (hInstance, szGrafClass);
   wndclass.hCursor       = LoadCursor (hInstance, "GRAFICA");
   wndclass.lpszClassName = szGrafClass;

   RegisterClass (&wndclass);

   wndclass.style         = CS_DBLCLKS;
   wndclass.lpfnWndProc   = ProcVentColor;
   wndclass.cbWndExtra    = 0;
   wndclass.hIcon         = NULL;
   wndclass.hCursor       = NULL;
   wndclass.hbrBackground = NULL;
   wndclass.lpszClassName = szColorClass;

   RegisterClass (&wndclass);

   wndclass.style         = CS_HREDRAW | CS_VREDRAW;
   wndclass.lpfnWndProc   = ProcVentCoords;
   wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
   wndclass.hbrBackground = GetStockObject (WHITE_BRUSH);
   wndclass.lpszClassName = szCoordsClass;

   RegisterClass (&wndclass);
  }

 /* Creamos la ventana principal */
 srand ((unsigned) hInstance);
 hwnd = CreateWindow (szAppName, "Osciloscopio Digital",
		      WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		      random (GetSystemMetrics(SM_CXSCREEN) - 310),
		      random (GetSystemMetrics(SM_CYSCREEN) - 310),
		      308, 293, NULL, NULL, hInstance, NULL);

 ShowWindow   (hwnd, nCmdShow);

 /* Si la l¡nea de comando existe, se toma */
 if (lstrlen (lpszCmdLine) > 0)
   lstrcat (nom_fich, lpszCmdLine);

 /* Invalidamos la ventana entera */
 InvalidateRect (hwnd, NULL, TRUE);

 /* Nos quedamos aqu¡ hasta que se elija 'Salir' del men£ */
 while (GetMessage (&msg, NULL, 0, 0))
 {
  TranslateMessage (&msg);
  DispatchMessage (&msg);
 }

 return (msg.wParam);
}

/*------------Comienzo de la zona de definici¢n de funciones----------------*/

long FAR PASCAL _export ProcVent (HWND hwnd, WORD message,
				  WORD wParam, LONG lParam)

/* Procedimiento de ventana principal */
{
 static short   cxChar, cxMax,
		nHmaxScrl, nVmaxScrl;
 static HANDLE  hInstance;
 static HBITMAP hbitmap;
 static HMENU   hMenu;
 static char    szNameAbrir[]    = "ABRIR",
		szNameTrazar[]   = "TRAZAR",
		szNameAcerca[]   = "ACERCA";
 HDC	        hdc, hdcMem1, hdcMem2;
 HBITMAP        hbitmap1, hviejobitmap;
 MSG		msg;
 RECT	        rect;
 TEXTMETRIC     tm;
 PAINTSTRUCT    ps;
 BITMAP	        bm1, bm2;
 FARPROC        lpfnProcDialog;
 FILE           *fuente;
 short          i, j, tipo;
 char           szBuffer[30], nom[84], *punt;

 switch (message)
 {
  case WM_CREATE:
  {
   /* Obtenemos el handle a la ocurrencia del m¢dulo */
   hInstance = ((LPCREATESTRUCT) lParam)->hInstance;

   /* Obtenemos los tama¤os del texto */
   hdc = GetDC (hwnd);
   GetTextMetrics (hdc, &tm);
   ReleaseDC (hwnd, hdc);

   cxChar = tm.tmAveCharWidth;
   cxMax  = ((tm.tmPitchAndFamily & 1) ? 3 : 2) * cxChar / 2;
   cyChar = tm.tmHeight + tm.tmExternalLeading;

   /* Asignamos el men£ a la ventana */
   hMenu = LoadMenu (hInstance, szAppName);
   SetMenu (hwnd, hMenu);

   return (0);
  }

  case WM_SIZE:
  {
   /* Si la ventana no ha sido minimizada.. */
   if (wParam != SIZEICONIC)
    {
     /* Calculamos el nuevo alto de l¡nea */
     cyLine = max (HIWORD(lParam) / 13, cyChar);

     /* Fijamos los rangos y posiciones de las barras de Scroll */
     nVmaxScrl = max (0, (15 * cyChar - (short) HIWORD(lParam)) / SCRLDESPL);
     nVposScrl = min (nVposScrl, nVmaxScrl);
     SetScrollRange (hwnd, SB_VERT, 0, nVmaxScrl, FALSE);
     SetScrollPos   (hwnd, SB_VERT, nVposScrl, TRUE);

     nHmaxScrl = max (0, (38 * cxChar - (short) LOWORD(lParam)) / SCRLDESPL);
     nHposScrl = min (nHposScrl, nHmaxScrl);
     SetScrollRange (hwnd, SB_HORZ, 0, nHmaxScrl, FALSE);
     SetScrollPos   (hwnd, SB_HORZ, nHposScrl, TRUE);

     /* Obtenemos un handle al contexto de dispositivo de pantalla */
     hdc = CreateIC ("DISPLAY", NULL, NULL, NULL);

     /* Creamos dos dispositivos de memoria compatibles con la pantalla */
     hdcMem1 = CreateCompatibleDC (hdc);
     hdcMem2 = CreateCompatibleDC (hdc);

     /* Liberamos el handle */
     DeleteDC (hdc);

     /* Cargamos el BitMap del disco */
     hbitmap1 = LoadBitmap (hInstance, szAppName);

     /* Obtenemos las caracter¡sticas del BitMap */
     GetObject (hbitmap1, sizeof (BITMAP), (LPSTR) &bm1);

     /* Las adaptamos a las caracter¡sticas de la ventana */
     bm2              = bm1;
     bm2.bmWidth      = LOWORD(lParam);
     bm2.bmHeight     = HIWORD(lParam);
     bm2.bmWidthBytes = ((bm2.bmWidth * bm2.bmBitsPixel + 15) / 16) * 2;

     /* Creamos un nuevo BitMap a partir dichas caracter¡sticas */
     DeleteObject (hbitmap);
     hbitmap = CreateBitmapIndirect (&bm2);

     /* Seleccionamos los BitMaps en los contextos de dispositivo */
     SelectObject (hdcMem1, hbitmap1);
     SelectObject (hdcMem2, hbitmap);

     /* Cambiamos el tama¤o del BitMap */
     StretchBlt (hdcMem2, 0, 0, bm2.bmWidth, bm2.bmHeight,
		 hdcMem1, 0, 0, bm1.bmWidth, bm1.bmHeight, SRCCOPY);

     /* Cleaning Up */
     DeleteDC (hdcMem1);
     DeleteDC (hdcMem2);
     DeleteObject (hbitmap1);

     return (0);
    }

   break;
  }

  case WM_PAINT:
  {
   /* ..creamos un contexto de dispositivo de memoria compatible */
   hdc = CreateIC ("DISPLAY", NULL, NULL, NULL);
   hdcMem1 = CreateCompatibleDC (hdc);
   DeleteDC (hdc);

   /* Seleccionamos el BitMap en el dispositivo de memoria */
   hviejobitmap = SelectObject (hdcMem1, hbitmap);

   /* Obtenemos el contexto de la ventana */
   hdc = BeginPaint (hwnd, &ps);

   /* Trasladamos el BitMap a la ventana */
   BitBlt (hdc,     ps.rcPaint.left, ps.rcPaint.top,
		    ps.rcPaint.right - ps.rcPaint.left + 1,
		    ps.rcPaint.bottom - ps.rcPaint.top + 1,
	   hdcMem1, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

   /* Seleccionamos el viejo bitmap en el contexto */
   SelectObject (hdcMem1, hviejobitmap);

   /* Liberamos el contexto de disp. de memoria */
   DeleteDC (hdcMem1);

   /* Liberamos el contexto de ventana */
   EndPaint (hwnd, &ps);

   return (0);
  }

  case WM_COMMAND:
  {
   /* El word bajo de lParam es 0 si el mensaje es de un men£ */
   if (LOWORD(lParam) == 0)
    {
     switch (wParam)
     {
      case IDM_ABRIR:
      {
       /* Obtenemos la ocurrencia del procedimiento din micamente */
       lpfnProcDialog = MakeProcInstance ((FARPROC) ProcDialogAbrir,
					  hInstance);

       /* Si la caja de di logo eligi¢ alg£n fichero.. */
       if (DialogBox (hInstance, szNameAbrir, hwnd, lpfnProcDialog))
	{
	 /* ..abrimos (si existe) el fichero fuente */
	 strcpy (nom, nom_fich);
	 strcat (nom, ".FNT");
	 if ((fuente = fopen (nom, "rt")) != NULL)
	  {
	   fclose (fuente);

	   /* Validamos las opciones del men£ */
	   EnableMenuItem (hMenu, IDM_VER,    MF_ENABLED);
	   EnableMenuItem (hMenu, IDM_TRAZAR, MF_ENABLED);
	  }
	}

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialog);

       return (0);
      }

      case IDM_VER:
       break;

      case IDM_TRAZAR:
      {
       /* Obtenemos la ocurrencia del procedimiento din micamente */
       lpfnProcDialog = MakeProcInstance ((FARPROC) ProcDialogTrazar,
					  hInstance);

       /* Si la caja de di logo finaliz¢ normalmente.. */
       tipo = DialogBox (hInstance, szNameTrazar, hwnd, lpfnProcDialog);
       if (tipo != 0)

	 /* ..trazamos la gr fica */
	 Trazar_Grafica (tipo - 1);

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialog);

       return (0);
      }

      case IDM_AYUDA:
       break;

      case IDM_ACERCA:
      {
       /* Obtenemos la ocurrencia del procedimiento din micamente */
       lpfnProcDialog = MakeProcInstance ((FARPROC) ProcDialogAcerca,
					  hInstance);
       /* Lanzamos la Caja de Di logo */
       DialogBox (hInstance, szNameAcerca, hwnd, lpfnProcDialog);

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialog);

       return (0);
      }

      case IDM_SALIR:
      {
       /* Enviamos un WM_DESTROY */
       DestroyWindow (hwnd);

       return (0);
      }
     }
    }

   /* pero, si el mensaje no es de un men£.. /*
   else
    {
    }

   return (0);
  }

  case WM_SYSCOMMAND:
  {
   /* Imitamos el comando 'Salir' del men£ */
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     DestroyWindow (hwnd);
     return (0);
    }

   break;
  }

  case WM_DESTROY:
  {
   /* Eliminamos el BitMap */
   DeleteObject (hbitmap);

   /* Eliminamos todas las ventanas de gr ficas */
   for (i = 0; i < numgraficas; ++i)
     for (j = 0; j < 5; ++j)
       if (hwndGraficas[i][j] != 0)
	 DestroyWindow (hwndGraficas[i][j]);

   PostQuitMessage (0);
   return (0);
  }
 }

 return (DefWindowProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogAbrir (HWND hDlg, WORD message,
					 WORD wParam, LONG lParam)

/* Procedimiento para la caja de di logo de Abrir Circuito */
{
 static   char EspecFich[84], NomFich[13] = "*.FNT";
 OFSTRUCT of;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos el valor del path inicial y el filtro por defecto */
   lstrcpy (EspecFich, nom_fich);

   /* Fijamos el texto de la caja edit */
   SetDlgItemText (hDlg, IDC_EDIT, NomFich);

   /* Hacemos la lista de directorios y unidades */
   DlgDirList (hDlg, EspecFich, IDC_LISTDIR, IDC_DIRACT, 0xC010);

   /* Hacemos la lista de ficheros */
   SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_DIR, 0, (LONG) NomFich);

   /* Damos el foco a la ventana edit */
   SetFocus (GetDlgItem (hDlg, IDC_EDIT));

   return (FALSE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_EDIT:
    {
     switch (HIWORD(lParam))
     {
      case EN_CHANGE:
      {
       /* Si no hay nada en la ventana edit, deshabilitamos OK */
       EnableWindow (GetDlgItem (hDlg, IDC_OK),
		     (BOOL) SendMessage (LOWORD (lParam),
					 WM_GETTEXTLENGTH, 0, 0));
       return (TRUE);
      }
     }

     return (FALSE);
    }

    case IDC_LISTFICH:
    {
     switch (HIWORD(lParam))
     {
      case LBN_SELCHANGE:
      {
       /* Obtenemos la seleci¢n actual */
       SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_GETTEXT,
			   SendDlgItemMessage (hDlg, IDC_LISTFICH,
					       LB_GETCURSEL, 0, 0),
			   (LONG) EspecFich);

       /* Le a¤adimos el nombre seleccionado en la lista */
       SetDlgItemText (hDlg, IDC_EDIT, EspecFich);

       return (TRUE);
      }

      case LBN_DBLCLK:
      {
       SendMessage (hDlg, WM_COMMAND, IDC_OK, 0L);

       return (TRUE);
      }
     }

     return (FALSE);
    }

    case IDC_LISTDIR:
    {
     switch (HIWORD(lParam))
     {
      case LBN_SELCHANGE:
      {
       /* Obtenemos el directorio ¢ unidad seleccionados */
       DlgDirSelect (hDlg, EspecFich, IDC_LISTDIR);

       /* Cambiamos el fichero al filtro por defecto */
       lstrcpy (NomFich, "*.FNT");

       /* Enviamos el nombre de fichero por defecto a la ventana edit */
       SetDlgItemText (hDlg, IDC_EDIT, lstrcat (EspecFich, NomFich));

       return (TRUE);
      }

      case LBN_DBLCLK:
      {
       /* Cambiamos de directorio */
       DlgDirList (hDlg, EspecFich, IDC_LISTDIR, IDC_DIRACT, 0xC010);

       /* Actualizamos la lista de ficheros */
       DlgDirList (hDlg, EspecFich, IDC_LISTFICH, NULL, 0);

       return (TRUE);
      }
     }

     return (FALSE);
    }

    case IDC_OK:
    {
     /* Obtenemos el texto de la ventana edit */
     GetDlgItemText (hDlg, IDC_EDIT, EspecFich, 80);

     /* Si es un nombre sin WILDCARDS.. */
     if ((!lstrchr (EspecFich, '*')) && (!lstrchr (EspecFich, '?')))
      {
       /* ..entonces, si el fichero existe.. */
       if (OpenFile (EspecFich, &of, OF_EXIST) != -1)
	{
	 /* ..obtenemos el nombre de fichero para el resto del programa */
	 strcpy (nom_fich, AnsiUpper (EspecFich));
	 *strrchr (nom_fich, '.') = 0;

	 /* Terminamos la caja de di logo */
	 EndDialog (hDlg, TRUE);
	}
       else
	 MessageBeep (0);
      }

     /* pero, si lleva WILDCARDS.. */
     else
      {
       /* .. actualizamos las ventanas de dir. actual y lista de dirs. */
       if (DlgDirList (hDlg, EspecFich, IDC_LISTDIR, IDC_DIRACT, 0xC010))
	{
	 DlgDirList (hDlg, EspecFich, IDC_LISTFICH, NULL, 0);
	}
      }

     return (TRUE);
    }

    case IDC_CANCELAR:
    {
     EndDialog (hDlg, FALSE);

     return (TRUE);
    }
   }

   return (FALSE);
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

LPSTR lstrchr (LPSTR str, char ch)

/* Localiza la posici¢n de ch en str ¢ devuelve NULL si ch no est  en str */
{
 while (*str)
 {
  if (ch == *str)
    return (str);

  str = AnsiNext (str);
 }

 return (NULL);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

LPSTR lstrrchr (LPSTR str, char ch)

/* Igual que lstrchr, pero empezando por el final de str */
{
 LPSTR strl = str + lstrlen (str);

 do
 {
  if (ch == *strl)
    return (strl);

  strl = AnsiPrev (str, strl);
 }
 while (strl > str);

 return (NULL);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogTrazar (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)

/* Procedimiento para la caja de di logo de Trazar Gr fica */
{
 static char  *ext[] = { ".PTR", ".TMP", ".FRC", ".FOU", ".SNS" };
 static short Dominio;
 OFSTRUCT     of;
 short        i;
 char         nom[84];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Inicializamos 'Dominio' */
   Dominio = 0;

   /* Obtenemos los resultados disponibles */
   for (i = PTRABAJO; i <= SENSIBILIDAD; ++i)
    {
     strcpy (nom, nom_fich);
     strcat (nom, ext[i]);
     if (OpenFile (nom, &of, OF_EXIST) == -1)
       EnableWindow (GetDlgItem (hDlg, IDC_PTRABAJO + i), FALSE);
    }

   Grafica = TRUE;
   CheckDlgButton (hDlg, IDC_GRAFICA, TRUE);

   Tabla = FALSE;
   CheckDlgButton (hDlg, IDC_TABLA, FALSE);

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_PTRABAJO:
    case IDC_TIEMPO:
    case IDC_FRECUENCIA:
    case IDC_FOURIER:
    case IDC_SENSIBILIDAD:
    {
     Dominio = wParam - IDC_PTRABAJO;

     CheckRadioButton (hDlg, IDC_PTRABAJO, IDC_SENSIBILIDAD, wParam);

     return (TRUE);
    }

    case IDC_EMPEZAR:
    {
     /* Obtenemos todos los datos */
     Grafica = (BOOL) IsDlgButtonChecked (hDlg, IDC_GRAFICA);

     Tabla =   (BOOL) IsDlgButtonChecked (hDlg, IDC_TABLA);

     EndDialog (hDlg, Dominio + 1);
     return (TRUE);
    }

    case IDC_CANCELAR:
    {
     EndDialog (hDlg, FALSE);
     return (TRUE);
    }
   }
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogAcerca (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)

/* Procedimiento para la caja de di logo de Acerca de... */
{
 switch (message)
 {
  case WM_INITDIALOG:
   return (TRUE);

  case WM_COMMAND:
  {
   EndDialog (hDlg, TRUE);
   return (TRUE);
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

