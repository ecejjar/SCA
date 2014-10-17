#include <windows.h>
#include <resolver.h>
#include <winresol.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcVent 	(HWND, WORD, WORD, LONG);
long FAR PASCAL _export ProcBarraPorcent (HWND, WORD, WORD, LONG);
long FAR PASCAL _export ProcManejoBoton  (HWND, WORD, WORD, LONG);

extern BOOL FAR PASCAL _export ProcDialogAbrir  (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogAcerca (HWND, WORD, WORD, LONG);

extern void Analisis (void);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDefinici¢n de variables globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

FARPROC  lpfnProcRealBoton[2];    /* Dirs. de los proc. de boton de Windows */
HWND	 hwndHija, 		       /* Handle a la ventana de porcentaje */
	 hwndBotonFin, hwndBotonAbort;  /* Handles a los botones de control */
short    minimizada,    		/* Informa del estado de la ventana */
	 Fin = 0, Abort = 0;     /* Flags de finalizar proceso y aplicaci¢n */
BOOL	 ayuda = FALSE;

extern HWND  hwnd;
extern short cyLine, nHposScrl, nVposScrl, TieneFoco;
extern char  szAppName[], szChildClass[], nom_fich[80],
	     frase1[21], frase2[28], frase3[32], frase4[38], frase5[5];
extern BOOL  hay_fichero;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcVent (HWND hwnd, WORD message,
				  WORD wParam, LONG lParam)
{
 static short   cxChar, cxMax, cyChar,
		nHmaxScrl, nVmaxScrl;
 static HANDLE  hInstance;
 static HICON   hIcono;
 static HMENU   hMenu;
 static HPEN    hPen;
 static HBRUSH  hBrush;
 static HBITMAP hbitmap;
 static char    szNameAbrir[]  = "LISTFICH",
		szNameAcerca[] = "ACERCA";
 HPEN           hOldPen;
 HBRUSH         hOldBrush;
 HDC 	        hdc, hdcMem1, hdcMem2;
 HBITMAP        hbitmap1, hviejobitmap;
 BITMAP         bm1, bm2;
 RECT	        rect;
 TEXTMETRIC     tm;
 PAINTSTRUCT    ps;
 OFSTRUCT       of;
 FARPROC        lpfnProcManejoBoton, lpfnProcDialogBox;
 short          nIncScrl;
 char           nom[84], szBuffer[60];

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

   /* Le ponemos el icono a la ventana */
   hIcono = LoadIcon (hInstance, szAppName);
   SetClassWord (hwnd, GCW_HICON, hIcono);

   /* Asignamos el men£ a la ventana */
   hMenu = LoadMenu (hInstance, szAppName);
   SetMenu (hwnd, hMenu);

   /* Obtenemos la dir. de nuestro proc. para botones */
   lpfnProcManejoBoton = MakeProcInstance ((FARPROC) ProcManejoBoton,
					   hInstance);

   /* Creamos la ventana hija para la barra de porcentaje */
   hwndHija =       CreateWindow (szChildClass, NULL,
				  WS_CHILD,
				  0, 0, 0, 0,
				  hwnd, BARRAPORCENT, hInstance, NULL);

   /* Creamos los botones de control en la pen£ltima l¡nea */
   hwndBotonFin =   CreateWindow ("button", "&Finalizar",
				  WS_CHILD | WS_DISABLED | BS_PUSHBUTTON,
				  0, 0, 0, 0,
				  hwnd, BOTONFIN, hInstance, NULL);

   lpfnProcRealBoton[0] = (FARPROC) GetWindowLong (hwndBotonFin,
						   GWL_WNDPROC);
   SetWindowLong (hwndBotonFin, GWL_WNDPROC, (LONG) lpfnProcManejoBoton);

   hwndBotonAbort = CreateWindow ("button", "A&bortar",
				  WS_CHILD | WS_DISABLED | BS_PUSHBUTTON,
				  0, 0, 0, 0,
				  hwnd, BOTONABORT, hInstance, NULL);

   lpfnProcRealBoton[1] = (FARPROC) GetWindowLong (hwndBotonAbort,
						   GWL_WNDPROC);
   SetWindowLong (hwndBotonAbort, GWL_WNDPROC, (LONG) lpfnProcManejoBoton);

   return (0);
  }

  case WM_SIZE:
  {
   /* Si la ventana ha sido minimizada.. */
   if (wParam == SIZEICONIC)
    {
     /* ..activamos el flag que indica la minimizaci¢n */
     minimizada = 1;

     /* Anulamos el alto de l¡nea */
     cyLine = 0;

     /* Damos el foco a la ventana principal */
     SetFocus (hwnd);

     /* Creamos el l piz para el marco */
     hPen = CreatePen (PS_SOLID, 4, RGB(255,0,0));

     /* Creamos la brocha para el fondo */
     hBrush = CreateSolidBrush (RGB(0,0,128));

     /* Invalidamos la ventana entera */
     InvalidateRect (hwnd, NULL, TRUE);
    }
   else
    {
     /* Calculamos el nuevo alto de una l¡nea */
     cyLine = max ((short) HIWORD(lParam) / 15, cyChar);

     /* Fijamos los rangos y posiciones de las barras de Scroll */
     nVmaxScrl = max (0, (15 * cyChar - (short) HIWORD(lParam)) / SCRLDESPL);
     nVposScrl = min (nVposScrl, nVmaxScrl);
     SetScrollRange (hwnd, SB_VERT, 0, nVmaxScrl, FALSE);
     SetScrollPos   (hwnd, SB_VERT, nVposScrl, TRUE);

     nHmaxScrl = max (0, (38 * cxChar - (short) LOWORD(lParam)) / SCRLDESPL);
     nHposScrl = min (nHposScrl, nHmaxScrl);
     SetScrollRange (hwnd, SB_HORZ, 0, nHmaxScrl, FALSE);
     SetScrollPos   (hwnd, SB_HORZ, nHposScrl, TRUE);

     /* Como puede que las barras de Scroll hayan desaparecido debido al
	c lculo anterior, obtenemos el nuevo  rea de cliente */
     GetClientRect (hwnd, &rect);

     /* Si no hay fichero abierto, pintamos la cortina de presentaci¢n */
     if (!hay_fichero)
      {
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
       bm2.bmWidth      = rect.right;
       bm2.bmHeight     = rect.bottom;
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
      }

     /* Reposicionamos la barra de porcentaje */
     MoveWindow (hwndHija, cxChar,          9 * cyLine,
		 rect.right - 2*cxChar, 2 * cyLine, TRUE);

     /* Reposicionamos los botones de control */
     MoveWindow (hwndBotonFin,
		 rect.right / 2 - (11*cxChar + cxMax),
		 12 * cyLine + (cyLine - cyChar) / 2,
		 10 * cxChar + cxMax, 2 * cyChar, TRUE);

     MoveWindow (hwndBotonAbort,
		 rect.right / 2 + cxChar,
		 12 * cyLine + (cyLine - cyChar) / 2,
		 10 * cxChar + cxMax, 2 * cyChar, TRUE);

     /* Si la ventana estaba minimizada.. */
     if (minimizada)
      {
       /* ..desactivamos el flag de minimizaci¢n */
       minimizada = 0;

       /* Restauramos el foco al bot¢n que lo ten¡a */
       SetFocus (hwnd);

       /* Destruimos el l piz para el marco y la brocha para el fondo */
       DeleteObject (hPen);
       DeleteObject (hBrush);

       /* Invalidamos la ventana de porcentaje entera */
       InvalidateRect (hwndHija, NULL, TRUE);
      }
    }
   return (0);
  }

  case WM_SETFOCUS:
  {
   /* Si la ventana principal no est  minimizada.. */
   if ((!minimizada) && (hay_fichero))

     /* ..damos el foco al bot¢n que le corresponda */
     SetFocus ((TieneFoco == BOTONFIN) ? hwndBotonFin : hwndBotonAbort);

   return (0);
  }

  case WM_PAINTICON:
  {
   if (hay_fichero)
    {
     rect.left   = 5;
     rect.top    = (GetSystemMetrics (SM_CYICON) - cyChar) / 2;
     rect.right  = rect.right - 5;
     rect.bottom = rect.top + cyChar - 1;
     InvalidateRect (hwnd, &rect, FALSE);

     /* Obtenemos el tama¤o del  rea de cliente */
     GetClientRect (hwnd, &rect);

     /* Obtenemos el contexto de la ventana */
     hdc = BeginPaint (hwnd, &ps);

     /* Seleccionamos los objetos apropiados */
     hOldPen = SelectObject (hdc, hPen);
     hOldBrush = SelectObject (hdc, hBrush);

     /* Dibujamos un marco
     MoveTo (hdc, rect.left,  rect.top);
     LineTo (hdc, rect.right, rect.top);
     LineTo (hdc, rect.right, rect.bottom);
     LineTo (hdc, rect.left,  rect.bottom);
     LineTo (hdc, rect.left,  rect.top); */
     Rectangle (hdc, rect.left, rect.top, rect.right, rect.bottom);

     /* Reseleccionamos el l piz anterior */
     SelectObject (hdc, hOldPen);

     strcpy (szBuffer, frase5);
     DrawText (hdc, strcat (szBuffer, "%"), -1, &rect,
	       DT_CENTER | DT_VCENTER | DT_SINGLELINE);

     /* Reseleccionamos la brocha anterior */
     SelectObject (hdc, hOldBrush);

     /* Validamos la ventana entera */
     EndPaint (hwnd, &ps);

     return (0);
    }

   break;
  }

  case WM_PAINT:
  {
   /* Obtenemos el tama¤o del  rea de cliente */
   GetClientRect (hwnd, &rect);

   /* Si la ventana no est  minimizada.. */
   if (!minimizada)
    {
     if (!hay_fichero)
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
	       hdcMem1, ps.rcPaint.left + nHposScrl * SCRLDESPL,
			ps.rcPaint.top  + nVposScrl * SCRLDESPL, SRCCOPY);

       /* Seleccionamos el viejo bitmap en el contexto */
       SelectObject (hdcMem1, hviejobitmap);

       /* Damos fin al contexto */
       EndPaint (hwnd, &ps);

       /* Liberamos el contexto de disp. de memoria */
       DeleteDC (hdcMem1);
      }
     else
      {
       /* Obtenemos el contexto de la ventana */
       hdc = BeginPaint (hwnd, &ps);

       /* En la l¡nea 2 pintamos lo que estamos resolviendo */
       TextOut (hdc,
		(rect.right - 11*cxChar - 9*cxMax) / 2 - nHposScrl * SCRLDESPL,
		cyLine + (cyLine - cyChar) / 2 - nVposScrl * SCRLDESPL,
		frase1, strlen (frase1));

       /* En la l¡nea 4, la respuesta que estamos calculando */
       TextOut (hdc,
		rect.right / 2 - (cxMax + 9*cxChar) - nHposScrl * SCRLDESPL,
		3*cyLine + (cyLine - cyChar) / 2 - nVposScrl * SCRLDESPL,
		frase2, strlen (frase2));

       /* En la l¡nea 6, los puntos que hay que calcular */
       TextOut (hdc,
		rect.right / 2 - (cxMax + 18*cxChar) - nHposScrl * SCRLDESPL,
		5*cyLine + (cyLine - cyChar) / 2 - nVposScrl * SCRLDESPL,
		frase3, strlen (frase3));

       /* En la l¡nea 8, los puntos ya calculados */
       TextOut (hdc,
		rect.right / 2 - (cxMax + 18*cxChar) - nHposScrl * SCRLDESPL,
		7*cyLine + (cyLine - cyChar) / 2 - nVposScrl * SCRLDESPL,
		frase4, strlen (frase4));

       /* En la ventana hija, aparecer  el porcentaje completado */
       memcpy (&rect, &ps.rcPaint, sizeof (RECT));
       ClientToScreen (hwnd,     (LPPOINT) &rect.left);
       ClientToScreen (hwnd,     (LPPOINT) &rect.right);
       ScreenToClient (hwndHija, (LPPOINT) &rect.left);
       ScreenToClient (hwndHija, (LPPOINT) &rect.right);
       rect.left   = max (0, rect.left);
       rect.top    = max (0, rect.top);
       rect.right  = max (0, rect.right);
       rect.bottom = max (0, rect.bottom);
       if ((rect.bottom > rect.top) || (rect.right > rect.left))
	 rect.left = 0;
       else
	 GetClientRect (hwndHija, &rect);

       InvalidateRect (hwndHija, &rect, FALSE);

       EndPaint (hwnd, &ps);
      }

     return (0);
    }

   break;
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case BOTONFIN:
    case BOTONABORT:
    {
     /* Fijamos la parte com£n del mensaje de finalizaci¢n */
     strcpy (szBuffer, "¨Est  ud. seguro de que quiere \x0");

     /* Se haya elegido FIN o ABORT, 'Fin' deber  activarse */
     Fin = 1;

     /* Activamos los flags de acuerdo con el control que haya sido pulsado */
     if (wParam == BOTONFIN)
       strcat (szBuffer, "FINALIZAR el c lculo actual?");
     else
       if (wParam == BOTONABORT)
	{
	 Abort = 1;
	 strcat (szBuffer, "ABORTAR la ejecuci¢n del programa?");
	}

     OemToAnsi ((LPSTR) szBuffer, (LPSTR) szBuffer);

     if (MessageBox (hwnd, (LPSTR) szBuffer, (LPSTR) frase1,
		     MB_ICONSTOP | MB_YESNO | MB_DEFBUTTON2) == IDNO)
       Fin = Abort = 0;

     return (0);
    }

    case IDM_ABRIR:
    {
     /* Obtenemos la dir. del proc. para la Caja de Di logo de Abrir */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogAbrir,
					   hInstance);

     /* Si se eligi¢ alg£n fichero.. */
     if (DialogBox (hInstance, szNameAbrir, hwnd, lpfnProcDialogBox))
      {
       /* ..lo abrimos, si es que existe */
       strcpy (nom, nom_fich);
       strcat (nom, ".FNT");
       if (OpenFile (nom, &of, OF_EXIST) != -1)
	{
	 /* Fijamos los valores de los flags */
	 hay_fichero = TRUE;
	 Fin = Abort = FALSE;

	 /* Obtenemos las frases para la ventana */
	 strcpy  (frase1, "Resolviendo \0");
	 lstrcat (frase1, AnsiUpper (nom_fich));
	 strcpy  (frase2, "Respuesta: \0");
	 strcpy  (frase3, "Puntos a calcular: \0");
	 strcpy  (frase4, "Puntos calculados: \0");
	 strcpy  (frase5, "0\0");

	 /* Hacemos aparecer las ventanas hijas */
	 ShowWindow (hwndHija,       SW_SHOW);
	 ShowWindow (hwndBotonFin,   SW_SHOW);
	 ShowWindow (hwndBotonAbort, SW_SHOW);

	 /* Validamos las opciones del men£ */
	 EnableMenuItem (hMenu, IDM_VER,      MF_ENABLED);
	 EnableMenuItem (hMenu, IDM_RESOLVER, MF_ENABLED);

	 /* Actualizamos la ventana */
	 InvalidateRect (hwnd, NULL, TRUE);
	}
      }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     return (0);
    }

    case IDM_RESOLVER:
    {
     /* Invalidamos las opciones 'Empezar', 'Cerrar' y 'Salir' */
     EnableMenuItem (hMenu, IDM_RESOLVER, MF_GRAYED);
     EnableMenuItem (hMenu, IDM_SALIR,    MF_GRAYED);
     EnableMenuItem (GetSystemMenu (hwnd, FALSE), SC_CLOSE, MF_GRAYED);

     /* Activamos los controles de bot¢n */
     EnableWindow (hwndBotonFin,   TRUE);
     EnableWindow (hwndBotonAbort, TRUE);

     /* Le damos el foco al primer bot¢n */
     SetFocus (hwndBotonFin);

     Analisis();

     /* Revalidamos las opciones anteriormente invalidadas */
     EnableMenuItem (hMenu, IDM_SALIR, MF_ENABLED);
     GetSystemMenu (hwnd, TRUE);

     /* Reponemos el icono de la ventana principal */
     SetClassWord (hwnd, GCW_HICON, hIcono);

     /* Ocultamos las ventanas hijas */
     ShowWindow (hwndHija,       SW_HIDE);
     ShowWindow (hwndBotonFin,   SW_HIDE);
     ShowWindow (hwndBotonAbort, SW_HIDE);

     /* Desactivamos los controles de bot¢n */
     EnableWindow (hwndBotonFin,   FALSE);
     EnableWindow (hwndBotonAbort, FALSE);

     /* Invalidamos las opciones correspondientes del men£ */
     EnableMenuItem (hMenu, IDM_VER,      MF_GRAYED);
     EnableMenuItem (hMenu, IDM_RESOLVER, MF_GRAYED);

     /* Indicamos que ya se ha resuelto el circuito */
     hay_fichero = FALSE;

     /* Invalidamos la ventana entera */
     InvalidateRect (hwnd, NULL, TRUE);

     return (0);
    }

    case IDM_AYUDA:
    {
     ayuda = WinHelp (hwnd, "WINRESOL.HLP", HELP_INDEX, 0);
     return (0);
    }

    case IDM_ACERCA:
    {
     /* Obtenemos la dir. del proc. para la Caja de Di logo de Acerca */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogAcerca,
					   hInstance);

     DialogBox (hInstance, szNameAcerca, hwnd, lpfnProcDialogBox);

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     return (0);
    }

    case IDM_SALIR:
    {
     /* Si la ayuda est  abierta, la cerramos */
     if (ayuda)
       WinHelp (hwnd, "WINCOMP.HLP", HELP_QUIT, 0);

     /* Eliminamos la ventana */
     DestroyWindow (hwnd);
     return (0);
    }
   }
  }

  case WM_SYSCOMMAND:
  {
   /* Si se ha elegido Cerrar en el men£ de sistema.. */
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     /* ..emulamos el proceso de 'Salir' */
     SendMessage (hwnd, WM_COMMAND, IDM_SALIR, 0);
     return (0);
    }

   break;
  }

  case WM_HSCROLL:
  {
   switch (wParam)
   {
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
     nIncScrl = -5;
     break;
    }

    case SB_PAGEDOWN:
    {
     nIncScrl = 5;
     break;
    }

    case SB_THUMBPOSITION:
    {
     nIncScrl = LOWORD(lParam) - nHposScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   if (nIncScrl = max (-nHposScrl,
		       min (nIncScrl, nHmaxScrl - nHposScrl)))
    {
     nHposScrl += nIncScrl;
     ScrollWindow (hwnd, -SCRLDESPL * nIncScrl, 0, NULL, NULL);
     SetScrollPos (hwnd, SB_HORZ, nHposScrl, TRUE);
    }

   return (0);
  }

  case WM_VSCROLL:
  {
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
     nIncScrl = -5;
     break;
    }

    case SB_PAGEDOWN:
    {
     nIncScrl = 5;
     break;
    }

    case SB_THUMBPOSITION:
    {
     nIncScrl = LOWORD(lParam) - nVposScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   if (nIncScrl = max (-nVposScrl,
		       min (nIncScrl, nVmaxScrl - nVposScrl)))
    {
     nVposScrl += nIncScrl;
     ScrollWindow (hwnd, 0, -SCRLDESPL * nIncScrl, NULL, NULL);
     SetScrollPos (hwnd, SB_VERT, nVposScrl, TRUE);
    }

   return (0);
  }

  case WM_DESTROY:
  {
   /* Eliminamos el BitMap */
   DeleteObject (hbitmap);

   PostQuitMessage (0);
   return (0);
  }
 }

 return (DefWindowProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcBarraPorcent (HWND hwnd, WORD message,
					  WORD wParam, LONG lParam)

/* Procedimiento para dibujar el porcentaje completado en la ventana hija
   est tica (hwndHija), mediante subclaseado de ventanas */
{
 static HBRUSH brocha;
 HDC	       hdc;
 PAINTSTRUCT   ps;
 RECT	       rect, rectBarra;
 char          szBuffer[5];

 switch (message)
 {
  case WM_CREATE:
  {
   SetWindowWord (hwnd, 0, 0);
   brocha = CreateSolidBrush (RGB(0,0,255));
   return (0);
  }

  case WM_PAINT:
  {
   /* Si el porcentaje en 'frase5' es menor que el que est  en la estructura
      de la ventana, invalidamos el rect ngulo entero */
   if (atoi (frase5) < (int) GetWindowWord (hwnd, 0))
     InvalidateRect (hwnd, NULL, TRUE);

   /* Obtenemos el tama¤o del  rea de cliente */
   GetClientRect (hwnd, &rect);

   hdc = BeginPaint (hwnd, &ps);

   MoveTo (hdc, rect.left,  rect.top);
   LineTo (hdc, rect.right - 1, rect.top);
   LineTo (hdc, rect.right - 1, rect.bottom - 1);
   LineTo (hdc, rect.left,  rect.bottom - 1);
   LineTo (hdc, rect.left,  rect.top);

   rectBarra.left = min (rect.right * (int) GetWindowWord (hwnd, 0) / 100 + 1,
			 ps.rcPaint.left);
   rectBarra.top    = 1;
   rectBarra.right  = rect.right * atoi (frase5) / 100 + 1;
   rectBarra.bottom = rect.bottom;
   FillRect (hdc, &rectBarra, brocha);

   strcpy (szBuffer, frase5);
   DrawText (hdc, strcat (szBuffer, "%"), -1, &rect,
	     DT_CENTER | DT_VCENTER | DT_SINGLELINE);
   EndPaint (hwnd, &ps);

   SetWindowWord (hwnd, 0, (WORD) atoi (frase5));

   return (0);
  }

  case WM_DESTROY:
  {
   DeleteObject (brocha);
   return (0);
  }
 }

 return (DefWindowProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcManejoBoton (HWND hwndHija, WORD message,
					 WORD wParam, LONG lParam)

/* Procedimiento para implementar un interfaz de teclado a los botones */
{
 switch (message)
 {
  case WM_KEYDOWN:
  {
   switch (wParam)
   {
    case VK_TAB:
     SetFocus ((TieneFoco == BOTONFIN) ? hwndBotonAbort : hwndBotonFin);
     break;

    case VK_HOME:
     SendMessage (hwnd, WM_VSCROLL, SB_TOP, 0);
     break;

    case VK_END:
     SendMessage (hwnd, WM_VSCROLL, SB_BOTTOM, 0);
     break;

    case VK_PRIOR:
     SendMessage (hwnd, WM_VSCROLL, SB_PAGEUP, 0);
     break;

    case VK_NEXT:
     SendMessage (hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
     break;

    case VK_UP:
     SendMessage (hwnd, WM_VSCROLL, SB_LINEUP, 0);
     break;

    case VK_DOWN:
     SendMessage (hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
     break;

    case VK_LEFT:
     SendMessage (hwnd, WM_HSCROLL, SB_LINEUP, 0);
     break;

    case VK_RIGHT:
     SendMessage (hwnd, WM_HSCROLL, SB_LINEDOWN, 0);
     break;
   }

   break;
  }

  case WM_SETFOCUS:
  {
   TieneFoco = GetWindowWord (hwndHija, GWW_ID);
   break;
  }
 }

 return (CallWindowProc (lpfnProcRealBoton[(TieneFoco == BOTONFIN) ? 0 : 1],
			 hwndHija, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/