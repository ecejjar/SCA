#include "compilar.h"

HWND    hwndFuente, hwndErrores;
FARPROC lpfnProcFuente,  lpfnViejoProcFuente,
	lpfnProcErrores, lpfnViejoProcErrores;
short   cxChar, nFoco;

extern HWND     hwndDebug;
extern unsigned nMaxLin, nMaxCol, nErrores, nAvisos;
extern short    cyChar;
extern char     nom_fich[80], szDebugClass[];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraciones de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcFuente  (HWND, WORD, WORD, LONG);
long FAR PASCAL _export ProcErrores (HWND, WORD, WORD, LONG);
short ScrollVert   (HWND, WORD, LONG, short, short, short);
short ScrollHoriz  (HWND, WORD, LONG, short, short, short);
void  TeclasScroll (HWND, WORD, LONG);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcDepurar (HWND hwnd, WORD message,
				     WORD wParam, LONG lParam)

/* Procedimiento para la ventana de depuraci¢n */
{
 HDC	      hdc;
 HMENU        hMenu;
 HANDLE       hInstance;
 TEXTMETRIC   tm;
 RECT         rect;

 switch (message)
 {
  case WM_CREATE:
  {
   /* Si la ventana es la principal de depuraci¢n (no es hija).. */
   if (GetParent (hwnd) == NULL)
    {
     /* ..obtenemos el handle de la ocurrencia del m¢dulo */
     hInstance = ((LPCREATESTRUCT) lParam)->hInstance;

     /* Creamos las ventanas hijas */
     hwndFuente = CreateWindow (szDebugClass, NULL,
				WS_CHILD | WS_HSCROLL | WS_VSCROLL |
				WS_VISIBLE | WS_BORDER,
				0, 0, 0, 0,
				hwnd, 1, hInstance, NULL);

     hwndErrores = CreateWindow (szDebugClass, NULL,
				 WS_CHILD | WS_HSCROLL | WS_VSCROLL |
				 WS_VISIBLE | WS_BORDER,
				 0, 0, 0, 0,
				 hwnd, 1, hInstance, NULL);

     /* Fijamos las funciones call-back */
     lpfnProcFuente = MakeProcInstance ((FARPROC) ProcFuente, hInstance);
     lpfnViejoProcFuente = (FARPROC) SetWindowLong (hwndFuente, GWL_WNDPROC,
						    (LONG) lpfnProcFuente);

     lpfnProcErrores = MakeProcInstance ((FARPROC) ProcErrores, hInstance);
     lpfnViejoProcErrores = (FARPROC) SetWindowLong (hwndErrores, GWL_WNDPROC,
						     (LONG) lpfnProcErrores);
     /* Damos el foco a la ventana de errores */
     nFoco = 1;
    }

   return (0);
  }

  case WM_SIZE:
  {
   /* Si la ventana no es hija de nadie.. */
   if (GetParent (hwnd) == NULL)
    {
     /* Movemos las ventanas hijas */
     MoveWindow (hwndFuente, 0, 0, LOWORD(lParam), 2*HIWORD(lParam)/3, TRUE);
     MoveWindow (hwndErrores, 0, 2*HIWORD(lParam)/3 + 1, LOWORD(lParam),
			      HIWORD(lParam) - 2*HIWORD(lParam)/3, TRUE);
     return (0);
    }

   break;
  }

  case WM_SETFOCUS:
  {
   /* Le damos el foco a la ventana que lo tuviese */
   SetFocus ((nFoco == 0) ? hwndFuente : hwndErrores);
   return (0);
  }

  case WM_SYSCOMMAND:
  {
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     DestroyWindow (hwnd);
     return (0);
    }

   break;
  }
 }

 return (DefWindowProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcFuente  (HWND hwnd, WORD message,
				     WORD wParam, LONG lParam)

/* Procedimiento para la ventana de c¢digo fuente */
{
 static short nLineasFuente, nHposScrl, nVposScrl, nHmaxScrl, nVmaxScrl;
 HDC	      hdc;
 PAINTSTRUCT  ps;
 TEXTMETRIC   tm;
 COLORREF     ViejoColor;
 FILE         *fuente;
 WORD         nl;
 short 	      cxCliente, cyCliente, nLinini, nLinfin, nIncScrl;
 char 	      nom[84], TextLin[256], aux;

 switch (message)
 {
  case WM_SIZE:
  {
   /* Obtenemos el nuevo tama¤o del  rea de cliente */
   cxCliente = LOWORD(lParam);
   cyCliente = HIWORD(lParam);

   /* Obtenemos los tama¤os del texto */
   hdc = GetDC (hwnd);
   GetTextMetrics (hdc, &tm);
   ReleaseDC (hwnd, hdc);

   cxChar = tm.tmAveCharWidth;
   cyChar = tm.tmHeight + tm.tmExternalLeading;

   /* Calculamos el n£mero de l¡neas que caben en la ventana */
   nLineasFuente = cyCliente / cyChar + 1;

   /* Calculamos los datos para las barras de Scroll */
   nVmaxScrl = max (0, (short)(nMaxLin - nLineasFuente + 2));
   nVposScrl = min (nVposScrl, nVmaxScrl);

   SetScrollRange (hwnd, SB_VERT, 0, nVmaxScrl, FALSE);
   SetScrollPos   (hwnd, SB_VERT, nVposScrl, TRUE);

   nHmaxScrl = max (0, (short)(nMaxCol - cxCliente / cxChar));
   nHposScrl = min (nHposScrl, nHmaxScrl);

   SetScrollRange (hwnd, SB_HORZ, 0, nHmaxScrl, FALSE);
   SetScrollPos   (hwnd, SB_HORZ, nHposScrl, TRUE);

   return (0);
  }

  case WM_PAINT:
  {
   hdc = BeginPaint (hwnd, &ps);

   /* Obtenemos los tama¤os del texto */
   GetTextMetrics (hdc, &tm);
   cxChar = tm.tmAveCharWidth;
   cyChar = tm.tmHeight + tm.tmExternalLeading;

   /* Calculamos en qu‚ l¡nea se empieza a pintar y en cu l se termina */
   nLinini = nVposScrl + ps.rcPaint.top / cyChar;
   nLinfin = min (nMaxLin, nVposScrl + ps.rcPaint.bottom / cyChar);

   /* Abrimos el fichero fuente */
   strcpy (nom, nom_fich);
   *strrchr (nom, '.') = 0;
   strcat (nom, ".FNT");
   fuente = fopen (nom, "rt");

   for (nl = 0; nl <= nLinfin; ++nl)
    {
     fscanf (fuente, "%[^\n]", TextLin);
     fscanf (fuente, "%*c", aux);
     if ((nl >= nLinini) && (!feof (fuente)))
      {
       /* Convertimos el texto del fichero en Ansi */
       OemToAnsi (TextLin, TextLin);

       /* Si la l¡nea es la actualmente seleccionada, ponemos fondo verde */
       if ((nl + 1) == GetWindowWord (hwnd, 0))
	 ViejoColor = (COLORREF) SetBkColor (hdc, RGB (0, 255, 0));

       TextOut (hdc, -nHposScrl * cxChar, (nl - nVposScrl) * cyChar,
		TextLin, strlen (TextLin));

       /* Restauramos el color de fondo, si es el caso */
       if ((nl + 1) == GetWindowWord (hwnd, 0))
	 SetBkColor (hdc, ViejoColor);
      }
    }

   fclose (fuente);

   EndPaint (hwnd, &ps);

   return (0);
  }

  case WM_HSCROLL:
  {
   nHposScrl = ScrollHoriz (hwnd, wParam, lParam,
			    nHposScrl, nHmaxScrl, cxCliente / cxChar);
   return (0);
  }

  case WM_VSCROLL:
  {
   nVposScrl = ScrollVert (hwnd, wParam, lParam,
			   nVposScrl, nVmaxScrl, nLineasFuente);
   return (0);
  }

  case WM_KEYDOWN:
  {
   if (wParam == VK_TAB)
    {
     nFoco = 1 - nFoco;
     SetFocus (GetParent (hwnd));
    }
   else
     TeclasScroll (hwnd, wParam, lParam);

   return (0);
  }
 }

 return (CallWindowProc (lpfnViejoProcFuente, hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcErrores (HWND hwnd, WORD message,
				     WORD wParam, LONG lParam)

/* Procedimiento para la depuraci¢n de errores */
{
 static short nLineasError, nHposScrl, nVposScrl, nHmaxScrl, nVmaxScrl;
 HDC	      hdc;
 PAINTSTRUCT  ps;
 TEXTMETRIC   tm;
 COLORREF     ViejoColor;
 FILE         *errores;
 WORD         nl;
 short        cxCliente, cyCliente, nLinini, nLinfin, nIncScrl;
 char	      nom[84], TextLin[256], aux;

 switch (message)
 {
  case WM_SIZE:
  {
   /* Obtenemos el nuevo tama¤o del  rea de cliente */
   cxCliente = LOWORD(lParam);
   cyCliente = HIWORD(lParam);

   /* Obtenemos los tama¤os del texto */
   hdc = GetDC (hwnd);
   GetTextMetrics (hdc, &tm);
   ReleaseDC (hwnd, hdc);

   cxChar = tm.tmAveCharWidth;
   cyChar = tm.tmHeight + tm.tmExternalLeading;

   /* Calculamos el n£mero de l¡neas que caben en la ventana */
   nLineasError = cyCliente / cyChar + 1;

   /* Calculamos los datos para las barras de Scroll */
   nVmaxScrl = max (0, (short)(nErrores + nAvisos - nLineasError + 2));
   nVposScrl = min (nVposScrl, nVmaxScrl);

   SetScrollRange (hwnd, SB_VERT, 0, nVmaxScrl, FALSE);
   SetScrollPos   (hwnd, SB_VERT, nVposScrl, TRUE);

   nHmaxScrl = max (0, (short)(75 - cxCliente / cxChar));
   nHposScrl = min (nHposScrl, nHmaxScrl);

   SetScrollRange (hwnd, SB_HORZ, 0, nHmaxScrl, FALSE);
   SetScrollPos   (hwnd, SB_HORZ, nHposScrl, TRUE);

   return (0);
  }

  case WM_PAINT:
  {
   hdc = BeginPaint (hwnd, &ps);

   /* Obtenemos los tama¤os del texto */
   GetTextMetrics (hdc, &tm);
   cxChar = tm.tmAveCharWidth;
   cyChar = tm.tmHeight + tm.tmExternalLeading;

   /* Calculamos en qu‚ l¡nea se empieza a pintar y en cu l se termina */
   nLinini = nVposScrl + ps.rcPaint.top / cyChar;
   nLinfin = min (nAvisos + nErrores - 1,
		  nVposScrl + ps.rcPaint.bottom / cyChar);

   /* Abrimos el fichero de informe */
   strcpy (nom, nom_fich);
   *strrchr (nom, '.') = 0;
   strcat (nom, ".LST");
   errores = fopen (nom, "rt");

   for (nl = 0; nl <= nLinfin; ++nl)
    {
     fscanf (errores, "%[^\n]", TextLin);
     fscanf (errores, "%*c", &aux);
     if ((nl >= nLinini) && !feof (errores))
      {
       /* Convertimos el texto del fichero al Ansi */
       OemToAnsi (TextLin, TextLin);

       /* Si la l¡nea es la actualmente seleccionada, ponemos fondo rojo */
       if ((nl + 1) == GetWindowWord (hwnd, 0))
	 ViejoColor = (COLORREF) SetBkColor (hdc, RGB(255, 0, 0));

       TextOut (hdc, -nHposScrl * cxChar, (nl - nVposScrl) * cyChar,
		TextLin, strlen (TextLin));

       /* Restauramos el color de fondo, si es el caso */
       if ((nl + 1) == GetWindowWord (hwnd, 0))
	 SetBkColor (hdc, ViejoColor);
      }
    }

   fclose (errores);

   EndPaint (hwnd, &ps);

   return (0);
  }

  case WM_LBUTTONDOWN:
  {
   /* Fijamos la l¡nea actualmente apuntada en ambas ventanas */
   SetWindowWord (hwnd, 0, ((short) nVposScrl + HIWORD(lParam) / cyChar + 1));

   strcpy (nom, nom_fich);
   *strrchr (nom, '.') = 0;
   strcat (nom, ".LST");
   errores = fopen (nom, "rt");

   for (nl = 0; nl <= GetWindowWord (hwnd, 0); ++nl)
    {
     fscanf (errores, "%[^\n]", TextLin);
     fscanf (errores, "%*c", &aux);
    }

   fclose (errores);

   SetWindowWord (hwndFuente, 0, (WORD) atoi (strtok (&TextLin[15], ",")) + 1);

   SendMessage (hwndFuente, WM_VSCROLL, SB_THUMBPOSITION,
		MAKELONG(GetWindowWord (hwndFuente, 0) - 2, 0));

   InvalidateRect (hwnd, NULL, TRUE);
   UpdateWindow (hwnd);

   InvalidateRect (hwndFuente, NULL, TRUE);
   UpdateWindow (hwndFuente);

   return (0);
  }

  case WM_HSCROLL:
  {
   nHposScrl = ScrollHoriz (hwnd, wParam, lParam,
			    nHposScrl, nHmaxScrl, cxCliente / cxChar);
   return (0);
  }

  case WM_VSCROLL:
  {
   nVposScrl = ScrollVert (hwnd, wParam, lParam,
			   nVposScrl, nVmaxScrl, nLineasError);
   return (0);
  }

  case WM_KEYDOWN:
  {
   if (wParam == VK_TAB)
    {
     nFoco = 1 - nFoco;
     SetFocus (GetParent (hwnd));
    }
   else
     TeclasScroll (hwnd, wParam, lParam);

   return (0);
  }
 }

 return (CallWindowProc (lpfnViejoProcErrores, hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

short ScrollVert (HWND hwnd, WORD wParam, LONG lParam,
		  short nVposScrl, short nVmaxScrl, short numLines)

/* Funci¢n que calcula, en funci¢n de los par metros, el valor, en l¡neas de
   texto, del desplazamiento de scroll vertical y su nueva posici¢n */
{
 short nIncScrl;

 switch (wParam)
 {
  case SB_TOP:
  {
   nIncScrl = -nVposScrl;
   break;
  }

  case SB_BOTTOM:
  {
   nIncScrl = nVmaxScrl - nVposScrl;
   break;
  }

  case SB_LINEUP:
  {
   nIncScrl = -1;
   break;
  }

  case SB_LINEDOWN:
  {
   nIncScrl = 1;
   break;
  }

  case SB_PAGEUP:
  {
   nIncScrl = -numLines;
   break;
  }

  case SB_PAGEDOWN:
  {
   nIncScrl = numLines;
   break;
  }

  case SB_THUMBTRACK:
  case SB_THUMBPOSITION:
  {
   nIncScrl = LOWORD(lParam) - nVposScrl;
   break;
  }

  default:
    nIncScrl = 0;
 }

 if (nIncScrl = max (-nVposScrl, min (nIncScrl, nVmaxScrl - nVposScrl)))
  {
   nVposScrl += nIncScrl;
   ScrollWindow (hwnd, 0, -cyChar * nIncScrl, NULL, NULL);
   SetScrollPos (hwnd, SB_VERT, nVposScrl, TRUE);
  }

 return (nVposScrl);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

short ScrollHoriz (HWND hwnd, WORD wParam, LONG lParam,
		   short nHposScrl, short nHmaxScrl, short numCols)

/* Funci¢n que calcula, en funci¢n de los par metros, el valor, en cols. de
   texto, del desplazamiento de scroll horizontal y su nueva posici¢n */
{
 short nIncScrl;

 switch (wParam)
 {
  case SB_TOP:
  {
   nIncScrl = -nHposScrl;
   break;
  }

  case SB_BOTTOM:
  {
   nIncScrl = nHmaxScrl - nHposScrl;
   break;
  }

  case SB_LINEUP:
  {
   nIncScrl = -1;
   break;
  }

  case SB_LINEDOWN:
  {
   nIncScrl = 1;
   break;
  }

  case SB_PAGEUP:
  {
   nIncScrl = -numCols;
   break;
  }

  case SB_PAGEDOWN:
  {
   nIncScrl = numCols;
   break;
  }

  case SB_THUMBTRACK:
  case SB_THUMBPOSITION:
  {
   nIncScrl = LOWORD(lParam) - nHposScrl;
   break;
  }

  default:
    nIncScrl = 0;
 }

 if (nIncScrl = max (-nHposScrl, min (nIncScrl, nHmaxScrl - nHposScrl)))
  {
   nHposScrl += nIncScrl;
   ScrollWindow (hwnd, -cxChar * nIncScrl, 0, NULL, NULL);
   SetScrollPos (hwnd, SB_HORZ, nHposScrl, TRUE);
  }

 return (nHposScrl);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void TeclasScroll (HWND hwnd, WORD wParam, LONG lParam)

/* Simula el Scroll con rat¢n a trav‚s del teclado */
{
 WORD direccion, accion;

 switch (wParam)
 {
  case VK_HOME:
  {
   direccion = WM_VSCROLL;
   accion    = SB_TOP;
   break;
  }

  case VK_END:
  {
   direccion = WM_VSCROLL;
   accion    = SB_BOTTOM;
   break;
  }

  case VK_PRIOR:
  {
   direccion = WM_VSCROLL;
   accion    = SB_PAGEUP;
   break;
  }

  case VK_NEXT:
  {
   direccion = WM_VSCROLL;
   accion    = SB_PAGEDOWN;
   break;
  }

  case VK_UP:
  {
   direccion = WM_VSCROLL;
   accion    = SB_LINEUP;
   break;
  }

  case VK_DOWN:
  {
   direccion = WM_VSCROLL;
   accion    = SB_LINEDOWN;
   break;
  }

  case VK_LEFT:
  {
   direccion = WM_HSCROLL;
   accion    = SB_LINEUP;
   break;
  }

  case VK_RIGHT:
  {
   direccion = WM_HSCROLL;
   accion    = SB_LINEDOWN;
   break;
  }

  default:
    direccion = 0;
 }

 if (direccion != 0)
  {
   SendMessage (hwnd, direccion, accion, 0);
   UpdateWindow (hwnd);
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del FicheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/