#include <graficos.h>
#include <wingraf.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄZona de definici¢n de variables globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ-*/

extern HWND             hwndGraficas[10][5];
extern extremos_grafica extGraficas[10][3];
extern float            fescx, fescy;
extern unsigned         ndivsx, ndivsy;
extern WORD             wColEsc, wColGraf;
extern BOOL             Tracking, Coords, Transf;
extern char 		szColorClass[];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern POINT coords_handle (HWND);

long FAR PASCAL _export ProcVentColor (HWND, WORD, WORD, LONG);

COLORREF ColorRgb (int id);

/*-------------------Comienzo de la zona de funciones-----------------------*/

BOOL FAR PASCAL _export ProcDialogParams  (HWND hDlg, WORD message,
					   WORD wParam, LONG lParam)

/* Procedimiento para la caja de di logo de Par metros */
{
 HWND     hControl, hPadre;
 int      id;
 short    a;
 char     szBuffer[10];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos la ventana propietaria de la caja */
   hPadre = GetParent (hDlg);

   /* Inicializamos el control de divisiones en el eje horizontal */
   hControl = GetDlgItem (hDlg, IDC_EDIT);
   ndivsx = GetWindowWord (hPadre, NDIVSX);
   SetWindowText (hControl, itoa (ndivsx, szBuffer, 10));

   hControl = GetDlgItem (hDlg, IDC_SCRLX);
   SetScrollRange (hControl, SB_CTL, 1, 100, FALSE);
   SetScrollPos   (hControl, SB_CTL, ndivsx, TRUE);

   /* Inicializamos el control de divisiones en el eje vertical */
   hControl = GetDlgItem (hDlg, IDC_EDIT2);
   ndivsy = GetWindowWord (hPadre, NDIVSY);
   SetWindowText (hControl, itoa (ndivsy, szBuffer, 10));

   hControl = GetDlgItem (hDlg, IDC_SCRLY);
   SetScrollRange (hControl, SB_CTL, 1, 100, FALSE);
   SetScrollPos   (hControl, SB_CTL, ndivsy, TRUE);

   /* Inicializamos las CheckBoxes */
   Tracking = GetWindowWord (hPadre, TRACKING);
   CheckDlgButton (hDlg, IDC_TRACKING, Tracking);

   Coords = GetWindowWord (hPadre, COORDS);
   CheckDlgButton (hDlg, IDC_COORDS, Coords);

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_EDIT:
    case IDC_EDIT2:
    {
     if (HIWORD (lParam) == EN_CHANGE)
      {
       /* Obtenemos la cantidad de divisiones fijada */
       a = GetWindowText (GetDlgItem (hDlg, wParam), szBuffer, 4);

       /* Si supera el l¡mite, recortamos */
       if (atoi (szBuffer) > 100)
	{
	 szBuffer[a] = 0;
	 SetWindowText (GetDlgItem (hDlg, wParam), szBuffer);
	 MessageBeep (0);
	}

       return (TRUE);
      }

     break;
    }

    case IDC_OK:
    {
     /* Obtenemos todos los datos */
     ndivsx = max(1, min(GetDlgItemInt (hDlg, IDC_EDIT, NULL, FALSE),  100));
     ndivsy = max(1, min(GetDlgItemInt (hDlg, IDC_EDIT2, NULL, FALSE), 100));

     Tracking = (BOOL) IsDlgButtonChecked (hDlg, IDC_TRACKING);

     Coords   = (BOOL) IsDlgButtonChecked (hDlg, IDC_COORDS);

     EndDialog (hDlg, TRUE);
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

  case WM_HSCROLL:
  {
   /* Obtenemos el handle del control */
   hControl = HIWORD(lParam);

   /* Obtenemos el ID del control */
   id = GetWindowWord (hControl, GWW_ID);

   switch (id)
   {
    case IDC_SCRLX:
    case IDC_SCRLY:
    {
     a = (id == IDC_SCRLX) ? ndivsx : ndivsy;

     switch (wParam)
     {
      case SB_LINEUP:
      {
       a -= 1;
       break;
      }

      case SB_LINEDOWN:
      {
       a += 1;
       break;
      }

      case SB_THUMBTRACK:
      {
       a = LOWORD(lParam);
       break;
      }
     }

     /* Limitamos el valor fijado */
     a = max (1, min (100, a));

     /* Cambiamos la posici¢n de la barra de scroll y el texto del control */
     if (a != GetScrollPos (hControl, SB_CTL))
      {
       SetScrollPos (hControl, SB_CTL, a, TRUE);
       SetDlgItemInt (hDlg, (id == IDC_SCRLX) ? IDC_EDIT : IDC_EDIT2, a,
		      FALSE);
      }

     if (id == IDC_SCRLX)
       ndivsx = a;
     else
       ndivsy = a;
    }
   }

   return (TRUE);
  }

  case WM_SYSCOMMAND:
  {
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     EndDialog (hDlg, FALSE);
     return (TRUE);
    }
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogEscalas (HWND hDlg, WORD message,
					   WORD wParam, LONG lParam)

/* Procedimiento para la caja de di logo de Escalas */
{
 static BOOL   mult, divid;
 HWND  	       hControl, hPadre;
 POINT 	       ch;
 unsigned long a;
 int   	       id;
 short 	       pseudoescalax, pseudoescalay;
 char  	       szBuffer[10];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   ch = coords_handle (GetParent (hDlg));

   /* Inicializamos el control de escala horizontal */
   hControl      = GetDlgItem (hDlg, IDC_EDIT);
   pseudoescalax = extGraficas[ch.y][ch.x - 1].escalax * 100;
   SetWindowText (hControl, itoa (pseudoescalax, szBuffer, 10));

   hControl = GetDlgItem (hDlg, IDC_SCRLX);
   SetScrollRange (hControl, SB_CTL, 1, 10000, FALSE);
   SetScrollPos   (hControl, SB_CTL, pseudoescalax, TRUE);

   /* Inicializamos el control de divisiones en el eje vertical */
   hControl      = GetDlgItem (hDlg, IDC_EDIT2);
   pseudoescalay = extGraficas[ch.y][ch.x - 1].escalay * 100;
   SetWindowText (hControl, itoa (pseudoescalay, szBuffer, 10));

   hControl = GetDlgItem (hDlg, IDC_SCRLY);
   SetScrollRange (hControl, SB_CTL, 1, 10000, FALSE);
   SetScrollPos   (hControl, SB_CTL, pseudoescalay, TRUE);

   /* Inicializamos el resto de variables */
   mult = divid = FALSE;

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_EDIT:
    case IDC_EDIT2:
    {
     mult = divid = FALSE;

     if (HIWORD (lParam) == EN_CHANGE)
      {
       /* Obtenemos la escala fijada */
       a = GetWindowText (GetDlgItem (hDlg, wParam), szBuffer, 5);

       /* Si supera el l¡mite, recortamos */
       if (atol (szBuffer) > 10000)
	{
	 szBuffer[a] = 0;
	 SetWindowText (GetDlgItem (hDlg, wParam), szBuffer);
	 MessageBeep (0);
	}
      }

     return (TRUE);
    }

    case IDC_MULT:
    {
     mult  = TRUE;
     divid = FALSE;
     return (TRUE);
    }

    case IDC_DIV:
    {
     mult  = FALSE;
     divid = TRUE;
     return (TRUE);
    }

    case IDC_POR2:
    case IDC_POR10:
    {
     a = GetDlgItemInt (hDlg, IDC_EDIT, NULL, FALSE);
     if (mult)
       a *= (wParam == IDC_POR2) ? 2 : 10;
     else
       if (divid)
	 a /= (wParam == IDC_POR2) ? 2 : 10;

     a = GetDlgItemInt (hDlg, IDC_EDIT2, NULL, FALSE);
     if (mult)
       a *= (wParam == IDC_POR2) ? 2 : 10;
     else
       if (divid)
	 a /= (wParam == IDC_POR2) ? 2 : 10;

     SetDlgItemInt (hDlg, IDC_EDIT,  max (1, min (a, 10000)), FALSE);
     SetDlgItemInt (hDlg, IDC_EDIT2, max (1, min (a, 10000)), FALSE);

     mult = divid = FALSE;

     return (TRUE);
    }

    case IDC_OK:
    {
     /* Obtenemos los datos */
     fescx = (float) max(1,
			 min(GetDlgItemInt (hDlg, IDC_EDIT,  NULL, FALSE),
			     10000)) / 100;
     fescy = (float) max(1,
			 min(GetDlgItemInt (hDlg, IDC_EDIT2, NULL, FALSE),
			     10000)) / 100;

     EndDialog (hDlg, TRUE);
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

  case WM_VSCROLL:
  {
   /* Obtenemos el handle del control que env¡a el mensaje */
   hControl = HIWORD(lParam);

   /* Obtenemos la ID de dicho control */
   id = GetWindowWord (hControl, GWW_ID);

   if ((id == IDC_SCRLX) || (id == IDC_SCRLY))
    {
     a = GetScrollPos (hControl, SB_CTL);

     switch (wParam)
     {
      case SB_TOP:
      {
       a = 1;
       break;
      }

      case SB_BOTTOM:
      {
       a = 10000;
       break;
      }

      case SB_PAGEUP:
      {
       a /= 10;
       break;
      }

      case SB_PAGEDOWN:
      {
       a *= 10;
       break;
      }

      case SB_LINEUP:
      {
       --a;
       break;
      }

      case SB_LINEDOWN:
      {
       ++a;
       break;
      }

      case SB_THUMBTRACK:
      {
       a = LOWORD(lParam);
       break;
      }
     }

     a = max (1, min (a, 10000));

     if (a != GetScrollPos (hControl, SB_CTL))
      {
       SetScrollPos  (hControl, SB_CTL, a, TRUE);
       SetDlgItemInt (hDlg, (id == IDC_SCRLX) ? IDC_EDIT : IDC_EDIT2,
		      a, FALSE);
      }
    }

   return (TRUE);
  }

  case WM_SYSCOMMAND:
  {
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     EndDialog (hDlg, FALSE);
     return (TRUE);
    }
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogColores (HWND hDlg, WORD message,
					   WORD wParam, LONG lParam)

/* Procedimiento para la caja de di logo de Colores */
{
 static HBRUSH hBrocha;
 HDC           hdc;
 HWND          hControl, hPadre;
 POINT	       point;
 RECT          rect;
 TEXTMETRIC    tm;
 int           id;
 short         cxInc, cyInc;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos el handle a la propietaria de la caja de di logo */
   hPadre = GetParent (hDlg);

   /* Obtenemos los colores actuales */
   wColEsc  = GetWindowWord (hPadre, COLORESC);
   wColGraf = GetWindowWord (hPadre, COLORGRF);

   /* Obtenemos las escalas para el sistema de coordenadas de las D.B. */
   hdc = CreateIC ("DISPLAY", NULL, NULL, NULL);
   GetTextMetrics (hdc, &tm);
   DeleteDC (hdc);

   cxInc = cyInc = (tm.tmHeight + tm.tmExternalLeading) / 8;

   /* Creamos las 34 ventanas hijas */
   for (id = 0; id < 16; ++id)
    {
     CreateWindow (szColorClass, NULL,
		   WS_CHILDWINDOW | WS_VISIBLE,
		   (12 + 10*(id/8)) * cxInc, (19 + 10*(id%8)) * cyInc,
		   10 * cxInc, 10 * cyInc,
		   hDlg, id + 1 + IDC_COLORESC,
		   GetWindowWord (hPadre, GWW_HINSTANCE), NULL);

     CreateWindow (szColorClass, NULL,
		   WS_CHILDWINDOW | WS_VISIBLE,
		   (54 + 10*(id/8)) * cxInc, (19 + 10*(id%8)) * cyInc,
		   10 * cxInc, 10 * cyInc,
		   hDlg, id + 1 + IDC_COLORGRF,
		   GetWindowWord (hPadre, GWW_HINSTANCE), NULL);
    }

   /* Creamos una brocha lisa y blanca */
   hBrocha = CreateSolidBrush (RGB (255, 255, 255));

   return (TRUE);
  }

  case WM_CTLCOLOR:
  {
   /* Si el mensaje lo env¡a un control est tico.. */
   if (HIWORD(lParam) == CTLCOLOR_STATIC)
    {
     /* ..obtenemos su handle */
     hControl = LOWORD(lParam);

     /* ..obtenemos su ID */
     id = GetWindowWord (hControl, GWW_ID);

     /* Si es de los controles del color.. */
     if ((id == IDC_COLORESC) || (id == IDC_COLORGRF))
      {
       /* ..le cambiamos la brocha de fondo */
       DeleteObject (hBrocha);
       hBrocha = CreateSolidBrush (ColorRgb (id));

       SelectObject (wParam, hBrocha);

       /* Le dibujamos un rect ngulo */
       GetClientRect (hControl, &rect);
       Rectangle (wParam, rect.left, rect.top, rect.right, rect.bottom);

       return (hBrocha);
      }
    }

   return (FALSE);
  }

  case WM_COMMAND:
  {
   if ((wParam == IDC_OK) || (wParam == IDC_CANCELAR))
    {
     DeleteObject (hBrocha);
     EndDialog (hDlg, (wParam == IDC_OK));
     return (TRUE);
    }

   return (FALSE);
  }

  case WM_KEYDOWN:
  {
   break;
  }

  case WM_SYSCOMMAND:
  {
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     SendMessage (hDlg, WM_COMMAND, IDC_CANCELAR, 0);
     return (TRUE);
    }
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcVentColor (HWND hwnd, WORD message,
				       WORD wParam, LONG lParam)

/* Procedimiento para gesti¢n de las ventanas de color */
{
 HDC 	     hdc;
 HWND        hControl;
 HBRUSH      hBrocha;
 RECT        rect;
 PAINTSTRUCT ps;
 int         id;

 switch (message)
 {
  case WM_PAINT:
  {
   hdc = BeginPaint (hwnd, &ps);

   hBrocha = CreateSolidBrush (ColorRgb (GetWindowWord (hwnd, GWW_ID)));
   SelectObject (hdc, hBrocha);

   GetClientRect (hwnd, &rect);
   Rectangle (hdc, rect.left, rect.top, rect.right, rect.bottom);

   EndPaint (hwnd, &ps);
   DeleteObject (hBrocha);

   return (0);
  }

  case WM_LBUTTONDOWN:
  {
   /* Obtenemos la ID de la ventana clickeada */
   id = GetWindowWord (hwnd, GWW_ID);

   /* Si la ventana es de color de escalas.. */
   if (id < IDC_COLORGRF)
    {
     /* ..fijamos el color de la escala */
     wColEsc = id - IDC_COLORESC - 1;
     hControl = GetDlgItem (GetParent (hwnd), IDC_COLORESC);
    }
   /* pero si no lo era (y por tanto era de gr fica).. */
   else
    {
     /* ..fijamos el color de la gr fica */
     wColGraf = id - IDC_COLORGRF - 1;
     hControl = GetDlgItem (GetParent (hwnd), IDC_COLORGRF);
    }

   /* Cambiamos la ventana de color actual */
   InvalidateRect (hControl, NULL, TRUE);
   UpdateWindow (hControl);

   return (0);
  }

  case WM_LBUTTONDBLCLK:
  {
   SendMessage (GetParent (hwnd), WM_COMMAND, IDC_OK, 0);
   return (0);
  }
 }

 return (DefWindowProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

COLORREF ColorRgb (int id)

/* Funci¢n que calcula el color s¢lido correspondiente al control est tico
   de la caja de di logo de Colores 'hDlg' */
{
 COLORREF cr;

 /* Obtenemos un ¡ndice entre 0 y 33 */
 id = id - IDC_COLORESC;

 /* Si el ¡ndice es 0 ¢ 17, devolvemos el color actual de fondo ¢ gr fica */
 if (id == 0)
   id = wColEsc;
 else
   if (id == 17)
     id = wColGraf;

   /* sino, ajustamos el offset a 0 */
   else
     id = (id % 17) - 1;

 /* Si el control est  entre 0 y 7, devolvemos el color puro corresp. */
 if (id < 8)
   cr = RGB((id & ROJO) * 255, ((id & VERDE) >> 1) * 255,
			       ((id & AZUL)  >> 2) * 255);

 /* Si no, devolvemos el color puro en HighLight */
 else
  {
   id -= 8;
   cr = RGB((id & ROJO) * 128, ((id & VERDE) >> 1) * 128,
			       ((id & AZUL)  >> 2) * 128);
  }

 return (cr);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/