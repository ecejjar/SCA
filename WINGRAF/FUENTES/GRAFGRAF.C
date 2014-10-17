#include <graficos.h>
#include <wingraf.h>

/*----------------Zona de definici¢n de variables globales------------------*/

extern HWND hwnd;
extern char szGrafClass[], szCoordsClass[], nom_fich[80];

short            numgraficas = 0,
		 nHmaxScrl, nVmaxScrl;
HWND             hwndGraficas[10][5];
extremos_grafica extGraficas[10][3];
complejo         FAR *M0;
complejo         FAR *M1;
float            fescx, fescy;
unsigned         ndivsx, ndivsy, paso;
WORD             wColEsc, wColGraf;
BOOL             Grafica, Tabla, Tracking, Coords, Transf;
char      	 szCoords[25], *ext[] = { ".TMP", ".FRC", ".FOU" };

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

POINT            coords_handle    (HWND);
RECT             coords_ventana   (HWND);
char             *nombre_medida   (char fichero[9], char dominio, short n,
				   char *resultado);
datos_grafica    obtener_dg       (char *, int);
extremos_grafica obtener_ext      (char *, int, int);
int 		 coords_cursor    (HWND, RECT, POINT *, float *);
float            norm             (float);
int              trans_Fourier    (HWND hwnd, int tipo);

int Dibujar_PdeTrabajo   (HWND, HDC, PAINTSTRUCT);
int Dibujar_Tiempo       (HWND, HDC, PAINTSTRUCT);
int Dibujar_Frecuencia   (HWND, HDC, PAINTSTRUCT);
int Dibujar_Fourier      (HWND, HDC, PAINTSTRUCT);
int Dibujar_Sensibilidad (HWND, HDC, PAINTSTRUCT);

extern BOOL FAR PASCAL _export ProcDialogParams  (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogEscalas (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogColores (HWND, WORD, WORD, LONG);

/*-------------------Comienzo de la zona de funciones-----------------------*/

long FAR PASCAL _export ProcGrafica (HWND hwnd, WORD message,
				     WORD wParam, LONG lParam)

/* Procedimiento para la clase de ventana "GRAFICA" */
{
 static HANDLE hInstance;
 static short  cxChar, cxMax, cyChar,
	       cx1, cx2;
 static char   szNameParams[]  = "PARAMETROS",
	       szNameEscalas[] = "ESCALAS",
	       szNameColores[] = "COLORES";
 static BOOL   derivar = FALSE, integrar = FALSE;
 HDC 	       hdc;
 HMENU	       hMenu;
 FARPROC       lpfnProcDialog;
 POINT         ch, pt;
 RECT	       r;
 TEXTMETRIC    tm;
 PAINTSTRUCT   ps;
 datos_grafica dg;
 float         cr[2];
 short         nPosScrl, nIncScrl, i, j, recolocar, cxCliente, cyCliente;
 char          nom[9], szBuffer[80], szOldNomyExt[13], szNewNomyExt[13],
	       szPrinterDriver[9], szPrinterName[30], szPrinterPort[6];

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
   hMenu = LoadMenu (hInstance, szGrafClass);
   SetMenu (hwnd, hMenu);

   /* Modificamos el men£ de sistema */
   hMenu = GetSystemMenu (hwnd, FALSE);
   AppendMenu (hMenu, MF_SEPARATOR, 0, NULL);
   AppendMenu (hMenu, MF_ENABLED,   SC_ARRANGE, "Reco&locar");

   /* Eliminamos las barras de Scroll */
   SetScrollRange (hwnd, SB_HORZ, 0, 0, TRUE);
   SetScrollRange (hwnd, SB_VERT, 0, 0, TRUE);

   return (0);
  }

  case WM_SIZE:
  {
   cxCliente = LOWORD(lParam);
   cyCliente = HIWORD(lParam);

   /* Si la ventana no est  siendo minimizada.. */
   if (wParam != SIZEICONIC)
    {
     /* ..cambiamos de sitio la ventana de coordenadas */
     if (GetWindowWord (hwnd, COORDS))
      {
       MoveWindow (GetWindowWord (hwnd, HWNDCOORDS),
		   cxCliente - 19*cxChar, cyCliente - 3*cyChar,
		   19 * cxChar,           3 * cyChar, TRUE);
       return (0);
      }
    }

   /* Si la ventana se est  minim. ¢ maxim., deshabilitamos "Recolocar" */
   if ((wParam == SIZEICONIC) || (wParam == SIZEFULLSCREEN))
     EnableMenuItem (GetSystemMenu (hwnd, FALSE), SC_ARRANGE, MF_GRAYED);
   else
     EnableMenuItem (GetSystemMenu (hwnd, FALSE), SC_ARRANGE, MF_ENABLED);

   break;
  }

  case WM_PAINT:
  {
   /* Obtenemos el contexto de dispositivo */
   hdc = BeginPaint (hwnd, &ps);

   switch (GetWindowWord (hwnd, 0))
   {
    case PTRABAJO:
    {
     Dibujar_PdeTrabajo (hwnd, hdc, ps);
     break;
    }

    case TIEMPO:
    {
     Dibujar_Tiempo (hwnd, hdc, ps);
     break;
    }

    case FRECUENCIA:
    {
     Dibujar_Frecuencia (hwnd, hdc, ps);
     break;
    }

    case FOURIER:
    {
     Dibujar_Fourier (hwnd, hdc, ps);
     break;
    }

    case SENSIBILIDAD:
    {
     Dibujar_Sensibilidad (hwnd, hdc, ps);
     break;
    }
   }

   /* Validamos la ventana */
   EndPaint (hwnd, &ps);

   return (0);
  }

  case WM_MOUSEMOVE:
  {
   if ((GetWindowWord (hwnd, TIPO) > PTRABAJO) &&
       (GetWindowWord (hwnd, TIPO) < SENSIBILIDAD))
    {
     /* Obtenemos el tama¤o de la ventana */
     GetClientRect (hwnd, &r);
     r.right  -= 5 + 5*cxChar + r.right / 3 * GetWindowWord (hwnd, TABLA);
     r.bottom -= 5 + 2*cyChar;

     /* Obtenemos la posici¢n del rat¢n */
     pt = MAKEPOINT(lParam);

     if ((pt.x >= 5) && (pt.x <= 5 + r.right) &&
	 (pt.y >= 5) && (pt.y <= 5 + r.bottom))
      {
       /* Obtenemos el string de coordenadas */
       coords_cursor (hwnd, r, &pt, cr);
       if (GetWindowWord (hwnd, COORDS))
	 sprintf (szCoords, " x : %+05.4e \n y : %+05.4e ", cr[0], cr[1]);

       /* Invalidamos la ventana de coordenadas */
       InvalidateRect (GetWindowWord (hwnd, HWNDCOORDS), NULL, FALSE);
       UpdateWindow (GetWindowWord (hwnd, HWNDCOORDS));

       /* Desplazamos la l¡nea de Integrar ¢ Derivar */
       if (integrar || derivar)
	{
	 /* Obtenemos un contexto para dibujar en la ventana */
	 hdc = GetDC (hwnd);

	 /* Borramos la l¡nea anterior */
	 SetROP2 (hdc, R2_NOTXORPEN);
	 MoveTo (hdc, cx1, 5);
	 LineTo (hdc, cx1, 5 + r.bottom);

	 /* Dibujamos la nueva */
	 cx1 = pt.x;
	 MoveTo (hdc, cx1, 5);
	 LineTo (hdc, cx1, 5 + r.bottom);

	 /* Eliminamos el contexto de dispositivo */
	 ReleaseDC (hwnd, hdc);
	}

       /* Bloqueamos el movimiento del cursor */
       if (GetWindowWord (hwnd, TRACKING))
	{
	 ClientToScreen (hwnd, &pt);
	 SetCursorPos (pt.x, pt.y);
	}
      }

     return (0);
    }

   break;
  }

  case WM_LBUTTONDOWN:
  {
   if ((GetWindowWord (hwnd, TIPO) > PTRABAJO) &&
       (GetWindowWord (hwnd, TIPO) < SENSIBILIDAD))
    {
     /* Obtenemos la posici¢n del rat¢n */
     pt = MAKEPOINT (lParam);

     return (0);
    }

   break;
  }

  case WM_COMMAND:
  {
   /* Si el comando es de un men£ (word bajo de lParam = 0).. */
   if (LOWORD(lParam) == 0)
    {
     integrar = derivar = FALSE;

     switch (wParam)
     {
      case IDM_PARAMETROS:
      {
       /* Obtenemos la ocurrencia del procedimiento din micamente */
       lpfnProcDialog = MakeProcInstance ((FARPROC) ProcDialogParams,
					  hInstance);

       /* Si la caja de di logo finaliz¢ con OK.. */
       if (DialogBox (hInstance, szNameParams, hwnd, lpfnProcDialog))
	{
	 /* ..pasamos los cambios a la ventana */
	 if ((GetWindowWord (hwnd, NDIVSX) != ndivsx) ||
	     (GetWindowWord (hwnd, NDIVSY) != ndivsy))
	  {
	   SetWindowWord (hwnd, NDIVSX, ndivsx);
	   SetWindowWord (hwnd, NDIVSY, ndivsy);

	   InvalidateRect (hwnd, NULL, TRUE);
	  }

	 SetWindowWord (hwnd, TRACKING, Tracking);
	 SetWindowWord (hwnd, COORDS,   Coords);

	 /* Hacemos aparecer ¢ desaparecer la ventana de coordenadas */
	 GetClientRect (hwnd, &r);
	 MoveWindow (GetWindowWord (hwnd, HWNDCOORDS),
		     r.right - 19*cxChar, r.bottom - 3*cyChar,
		     19 * cxChar,	 3 * cyChar, TRUE);
	 ShowWindow (GetWindowWord (hwnd, HWNDCOORDS),
		     (Coords) ? SW_SHOWNORMAL : SW_HIDE);
	}

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialog);

       return (0);
      }

      case IDM_ESCALAS:
      {
       /* Obtenemos la ocurrencia del procedimiento din micamente */
       lpfnProcDialog = MakeProcInstance ((FARPROC) ProcDialogEscalas,
					  hInstance);

       /* Si la caja de di logo finaliz¢ con OK.. */
       if (DialogBox (hInstance, szNameEscalas, hwnd, lpfnProcDialog))
	{
	 /* ..pasamos los cambios a la tabla de datos de las gr ficas */
	 ch = coords_handle (hwnd);
	 if ((extGraficas[ch.y][ch.x - 1].escalax != fescx) ||
	     (extGraficas[ch.y][ch.x - 1].escalay != fescy))
	  {
	   extGraficas[ch.y][ch.x - 1].escalax = fescx;
	   extGraficas[ch.y][ch.x - 1].escalay = fescy;

	   /* Fijamos los rangos y posiciones de scroll */
	   nHmaxScrl = max (0, (float) GetWindowWord (hwnd, NDIVSX) *
			       (fescx - 1));
	   SetScrollRange (hwnd, SB_HORZ, 0, nHmaxScrl, FALSE);
	   SetScrollPos (hwnd, SB_HORZ, 0, TRUE);

	   nVmaxScrl = max (0, (float) GetWindowWord (hwnd, NDIVSY) *
			       (fescy - 1));
	   SetScrollRange (hwnd, SB_VERT, 0, nVmaxScrl, FALSE);
	   SetScrollPos (hwnd, SB_VERT, nVmaxScrl, TRUE);

	   InvalidateRect (hwnd, NULL, TRUE);
	  }
	}

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialog);

       return (0);
      }

      case IDM_COLORES:
      {
       /* Obtenemos la ocurrencia del procedimiento din micamente */
       lpfnProcDialog = MakeProcInstance ((FARPROC) ProcDialogColores,
					  hInstance);

       /* Si la caja de di logo finaliz¢ con OK.. */
       if (DialogBox (hInstance, szNameColores, hwnd, lpfnProcDialog))
	{
	 /* ..pasamos los cambios a la ventana */
	 if ((GetWindowWord (hwnd, COLORESC) != wColEsc) ||
	     (GetWindowWord (hwnd, COLORGRF) != wColGraf))
	  {
	   SetWindowWord (hwnd, COLORESC, wColEsc);
	   SetWindowWord (hwnd, COLORGRF, wColGraf);

	   InvalidateRect (hwnd, NULL, TRUE);
	  }
	}

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialog);

       return (0);
      }

      case IDM_DERIVAR:
      case IDM_INTEGRAR:
      {
       /* Obtenemos el tama¤o de la ventana */
       GetClientRect (hwnd, &r);
       r.right  -= 5 + 5*cxChar + r.right / 3 * GetWindowWord (hwnd, TABLA);
       r.bottom -= 5 + 2*cyChar;

       /* Inicializamos el trazado de la l¡nea indicadora (si no estaba
	  siendo trazada antes por el proceso de integrar) */
       if ((!derivar) && (!integrar))
	{
	 cx1 = 5;
	 hdc = GetDC (hwnd);
	 MoveTo (hdc, cx1, 5);
	 LineTo (hdc, cx1, 5 + r.bottom);
	 ReleaseDC (hwnd, hdc);
	}

       /* Activamos el flag de derivaci¢n y desactivamos el de integ. */
       derivar = (wParam == IDM_DERIVAR) ? TRUE : FALSE;
       integrar = !derivar;

       return (0);
      }

      case IDM_TIEMPO:
      {
       /* Deshabilitamos la opci¢n del men£ para que no pueda reseleccionarse
	  hasta que se pase a frecuencia de nuevo */
       EnableMenuItem (GetSubMenu (GetMenu (hwnd), 2),
		       IDM_TIEMPO, MF_GRAYED);
       EnableMenuItem (GetSubMenu (GetMenu (hwnd), 2),
		       IDM_FRECUENCIA, MF_ENABLED);

       /* Obtenemos de la ventana el nombre de fichero */
       GetWindowText (hwnd, (LPSTR) nom, 9);
       *strchr (nom, ' ') = 0;

       /* Si la gr fica en la ventana es una respuesta temporal.. */
       if (GetWindowWord (hwnd, TIPO) == TIEMPO)
	{
	 /* ..renombramos los ficheros a los nombres originales */
	 strcpy (szOldNomyExt, nom);
	 strcat (szOldNomyExt, ".TMP");
	 strcpy (szNewNomyExt, nom);
	 strcat (szNewNomyExt, ".DFT");
	 rename (szOldNomyExt, szNewNomyExt);

	 strcpy (szOldNomyExt, nom);
	 strcat (szOldNomyExt, ".$$$");
	 strcpy (szNewNomyExt, nom);
	 strcat (szNewNomyExt, ".TMP");
	 rename (szOldNomyExt, szNewNomyExt);
	}

       /* pero, si la gr fica es una respuesta frecuencial.. */
       else
	{
	 /* ..calculamos la transformada inversa */
	 trans_Fourier (hwnd, INVERSA);

	 /* Renombramos los ficheros */
	 strcpy (szOldNomyExt, nom);
	 if (GetWindowWord (hwnd, TIPO) == FRECUENCIA)
	   strcat (szOldNomyExt, ".FRC");
	 else
	   strcat (szOldNomyExt, ".FOU");
	 strcpy (szNewNomyExt, nom);
	 strcat (szNewNomyExt, ".$$$");
	 rename (szOldNomyExt, szNewNomyExt);

	 strcpy (szOldNomyExt, nom);
	 strcat (szOldNomyExt, ".IFT");
	 strcpy (szNewNomyExt, nom);
	 if (GetWindowWord (hwnd, TIPO) == FRECUENCIA)
	   strcat (szNewNomyExt, ".FRC");
	 else
	   strcat (szNewNomyExt, ".FOU");
	 rename (szOldNomyExt, szNewNomyExt);
	}

       /* Localizamos la posici¢n del handle de la ventana en la tabla */
       ch = coords_handle (hwnd);

       /* Obtenemos las extensiones en x e y de la nueva gr fica */
       strcpy (szNewNomyExt, nom);
       extGraficas[ch.y][ch.x - TIEMPO] = obtener_ext (szNewNomyExt, ch.x, 0);

       /* Preparamos su visualizaci¢n */
       InvalidateRect (hwnd, NULL, TRUE);

       return (0);
      }

      case IDM_FRECUENCIA:
      {
       /* Deshabilitamos la opci¢n del men£ para que no pueda reseleccionarse
	  hasta que se pase a tiempo de nuevo */
       EnableMenuItem (GetSubMenu (GetMenu (hwnd), 2),
		       IDM_FRECUENCIA, MF_GRAYED);
       EnableMenuItem (GetSubMenu (GetMenu (hwnd), 2),
		       IDM_TIEMPO, MF_ENABLED);

       /* Obtenemos de la ventana el nombre de fichero */
       GetWindowText (hwnd, (LPSTR) nom, 9);
       *strchr (nom, ' ') = 0;

       /* Si la gr fica es una respuesta frecuencial.. */
       if (GetWindowWord (hwnd, TIPO) != TIEMPO)
	{
	 /* ..renombramos los ficheros a los nombres originales */
	 strcpy (szOldNomyExt, nom);
	 if (GetWindowWord (hwnd, TIPO) == FRECUENCIA)
	   strcat (szOldNomyExt, ".FRC");
	 else
	   strcat (szOldNomyExt, ".FOU");
	 strcpy (szNewNomyExt, nom);
	 strcat (szNewNomyExt, ".IFT");
	 rename (szOldNomyExt, szNewNomyExt);

	 strcpy (szOldNomyExt, nom);
	 strcat (szOldNomyExt, ".$$$");
	 strcpy (szNewNomyExt, nom);
	 if (GetWindowWord (hwnd, TIPO) == FRECUENCIA)
	   strcat (szNewNomyExt, ".FRC");
	 else
	   strcat (szNewNomyExt, ".FOU");
	 rename (szNewNomyExt, szNewNomyExt);
	}

       /* pero, si la gr fica es una respuesta temporal.. */
       else
	{
	 /*..calulamos la transformada */
	 trans_Fourier (hwnd, DIRECTA);

	 /* Renombramos los ficheros */
	 strcpy (szOldNomyExt, nom);
	 strcat (szOldNomyExt, ".TMP");
	 strcpy (szNewNomyExt, nom);
	 strcat (szNewNomyExt, ".$$$");
	 rename (szOldNomyExt, szNewNomyExt);

	 strcpy (szOldNomyExt, nom);
	 strcat (szOldNomyExt, ".DFT");
	 strcpy (szNewNomyExt, nom);
	 strcat (szNewNomyExt, ".TMP");
	 rename (szOldNomyExt, szNewNomyExt);
	}

       /* Preparamos la presentaci¢n de la nueva gr fica */
       ch = coords_handle (hwnd);
       strcpy (szNewNomyExt, nom);
       extGraficas[ch.y][ch.x - TIEMPO] = obtener_ext (szNewNomyExt, ch.x, 0);
       InvalidateRect (hwnd, NULL, TRUE);
       return (0);
      }

      case IDM_GRANDE:
      case IDM_PEQUENO:
      {
       /* Obtenemos los datos de la impresora del archivo "WIN.INI" */
       GetProfileString ("Windows", "device", NULL, szBuffer, 80);
       lstrcpy (szPrinterName,   strtok (szBuffer, ","));
       lstrcpy (szPrinterDriver, strtok (NULL,     ","));
       lstrcpy (szPrinterPort,   strtok (NULL,     ","));

       /* Creamos un contexto de dispositivo para la impresora */
       hdc = CreateDC (szPrinterDriver, szPrinterName, szPrinterPort, NULL);

       /* Activamos la recogida de datos en la p gina */
       Escape (hdc, STARTDOC, lstrlen (nom_fich), nom_fich,NULL);

       /* Enviamos la Gr fica a la p gina */
       GetClientRect (hwnd, &ps.rcPaint);
       switch (GetWindowWord (hwnd, 0))
       {
	case PTRABAJO:
	{
	 Dibujar_PdeTrabajo (hwnd, hdc, ps);
	 break;
	}

	case TIEMPO:
	{
	 Dibujar_Tiempo (hwnd, hdc, ps);
	 break;
	}

	case FRECUENCIA:
	{
	 Dibujar_Frecuencia (hwnd, hdc, ps);
	 break;
	}

	case FOURIER:
	{
	 Dibujar_Fourier (hwnd, hdc, ps);
	 break;
	}

	case SENSIBILIDAD:
	{
	 Dibujar_Sensibilidad (hwnd, hdc, ps);
	 break;
	}
       }


       /* Finalizamos la pagina */
       Escape (hdc, NEWFRAME, NULL, NULL, NULL);

       /* Finalizamos la recogida de datos */
       Escape (hdc, ENDDOC, NULL, NULL, NULL);

       /* Eliminamos el contexto */
       DeleteDC (hdc);

       return (0);
      }
     }
    }

   /* pero, si el mensaje no es de un men£.. */
   else
    {
    }

   return (0);
  }

  case WM_SYSCOMMAND:
  {
   switch (wParam & 0xFFF0)
   {
    case SC_CLOSE:
    {
     /* Eliminamos el handle */
     ch = coords_handle (hwnd);
     hwndGraficas[ch.y][ch.x] = 0;

     /* Si es la £ltima gr fica del circuito, comprimimos */
     for (j = 0; j < 5; ++j)
       if (hwndGraficas[ch.y][j] != 0)
	 break;

     if (j >= 5)
      {
       /* Reducimos el n£mero de gr ficas */
       --numgraficas;

       /* Si el circuito no ocupaba la £ltima posici¢n.. */
       if (ch.y < numgraficas)
	{
	 /* ..preguntamos si queremos reposicionar las ventanas */
	 recolocar = MessageBox (hwnd, "¿Reposicionar todas las gráficas?",
				 "Cerrar ventana de gráfica",
				 MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO |
				 MB_DEFBUTTON1);

	 /* Recorremos la tabla desde la fila a eliminar hasta la £ltima */
	 for (i = ch.y; i < numgraficas; ++i)
	  {
	   /* Comprimimos */
	   memcpy (hwndGraficas[i], hwndGraficas[i+1], 5 * sizeof (HWND));
	   memcpy (extGraficas[i],  extGraficas[i+1],
		   3 * sizeof (extremos_grafica));

	   /* Reposicionamos todas las ventanas, si es el caso */
	   if (recolocar == IDYES)
	     for (j = 0; j < 5; ++j)
	       if (hwndGraficas[i][j] != 0)
		 SendMessage (hwndGraficas[i][j],
			      WM_SYSCOMMAND, SC_ARRANGE, 0);
	  }
	}
      }

     DestroyWindow (hwnd);
     return (0);
    }

    case SC_ARRANGE:
    {
     /* Recolocamos la ventana */
     r = coords_ventana (hwnd);
     MoveWindow (hwnd, r.left, r.top, r.right, r.bottom, TRUE);

     return (0);
    }
   }

   break;
  }

  case WM_HSCROLL:
  {
   nPosScrl = GetScrollPos (hwnd, SB_HORZ);

   switch (wParam)
   {
    case SB_PAGEUP:
    {
     nIncScrl = -GetWindowWord (hwnd, NDIVSX);
     break;
    }

    case SB_PAGEDOWN:
    {
     nIncScrl = GetWindowWord (hwnd, NDIVSX);
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

    case SB_THUMBPOSITION:
    {
     nIncScrl = LOWORD(lParam) - nPosScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   if (nIncScrl = max (-nPosScrl, min (nIncScrl, nHmaxScrl - nPosScrl)))
    {
     nPosScrl += nIncScrl;
     GetClientRect (hwnd, &r);
     r.left   -= 5 + 5*cxChar + r.left * GetWindowWord (hwnd, TABLA) / 3;
     r.bottom -= 5 + 2*cyChar;
     ScrollWindow (hwnd, -(r.right - r.left) / GetWindowWord (hwnd, NDIVSX) *
			 nIncScrl,
		   0, NULL, NULL);
     SetScrollPos (hwnd, SB_HORZ, nPosScrl, TRUE);
     GetClientRect (hwnd, &r);
     MoveWindow (GetWindowWord (hwnd, HWNDCOORDS),
		 r.right - 19*cxChar, r.bottom - 3*cyChar,
		 19 * cxChar,           3 * cyChar, TRUE);
     UpdateWindow (hwnd);
    }

   return (0);
  }

  case WM_VSCROLL:
  {
   nPosScrl = GetScrollPos (hwnd, SB_VERT);

   switch (wParam)
   {
    case SB_TOP:
    {
     nIncScrl = -nPosScrl;
     break;
    }

    case SB_BOTTOM:
    {
     nIncScrl = nVmaxScrl - nPosScrl;
     break;
    }

    case SB_PAGEUP:
    {
     nIncScrl = -GetWindowWord (hwnd, NDIVSY);
     break;
    }

    case SB_PAGEDOWN:
    {
     nIncScrl = GetWindowWord (hwnd, NDIVSY);
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

    case SB_THUMBPOSITION:
    {
     nIncScrl = LOWORD(lParam) - nPosScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   if (nIncScrl = max (-nPosScrl, min (nIncScrl, nVmaxScrl - nPosScrl)))
    {
     nPosScrl += nIncScrl;
     GetClientRect (hwnd, &r);
     r.left   -= 5 + 5*cxChar + r.left * GetWindowWord (hwnd, TABLA) / 3;
     r.bottom -= 5 + 2*cyChar;
     ScrollWindow (hwnd, 0,
			 -(r.bottom - r.top) / GetWindowWord (hwnd, NDIVSY) *
			 nIncScrl,
		   NULL, NULL);
     SetScrollPos (hwnd, SB_VERT, nPosScrl, TRUE);
     GetClientRect (hwnd, &r);
     MoveWindow (GetWindowWord (hwnd, HWNDCOORDS),
		 r.right - 19*cxChar, r.bottom - 3*cyChar,
		 19 * cxChar,           3 * cyChar, TRUE);
     UpdateWindow (hwnd);
    }

   return (0);
  }

  case WM_DESTROY:
  {
   break;
  }
 }

 return (DefWindowProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Trazar_Grafica (int tipo)

/* Presenta los resultados pedidos de tipo 'tipo' */
{
 static char *Titulos[] = { " - Punto de Trabajo", " - Respuesta Temporal",
			    " - Respuesta Frecuencial", " - Tda. de Fourier",
			    " - Sensibilidad" };
 RECT  r;
 short i, j;
 char  Texto[10], szBuffer[30], *punt;

 /* Obtenemos el nombre del circuito al que pertenece la gr fica */
 punt = strrchr (nom_fich, '\\');
 strcpy (Texto, (punt == NULL) ? nom_fich : punt);

 /* Recorremos la tabla de gr ficas buscando las de este circuito */
 for (i = 0; i < numgraficas; ++i)
  {
   for (j = 0; j < 5; ++j)
     if (hwndGraficas[i][j] != 0)
      {
       /* Obtenemos el circuito del t¡tulo de la ventana */
       GetWindowText (hwndGraficas[i][j], szBuffer, 9);
       *strchr (szBuffer, ' ') = 0;

       /* Si la gr fica pertenece a este circuito.. */
       if (strcmp (Texto, szBuffer) == 0)
	{
	 /* ..entonces, si son del mismo tipo.. */
	 if (j == tipo)

	   /* ..nos cambiamos a su ventana */
	   SetActiveWindow (hwndGraficas[i][j]);

	 /* Aunque no sean del mismo tipo, nos salimos del bucle */
	 break;
	}
      }

   if (strcmp (Texto, szBuffer) == 0)
     break;
  }

 /* Si la gr fica no est  ya hecha.. */
 if ((i >= numgraficas) || ((i < numgraficas) && (j != tipo)))
  {
   /* ..incrementamos el n§ de gr ficas que hay, si es el caso y se puede */
   if (i >= numgraficas)
     ++numgraficas;

   if (numgraficas < 10)
    {
     /* Obtenemos los extremos y escalas de la gr fica */
     if ((tipo >= TIEMPO) && (tipo <= FOURIER))
       extGraficas[i][tipo - TIEMPO] = obtener_ext (Texto, tipo, 0);

     /* Creamos la ventana */
     hwndGraficas[i][tipo] =
       CreateWindow (szGrafClass,
		     strcat (strcpy (szBuffer, Texto), Titulos[tipo]),
		     WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL |
		     WS_CLIPCHILDREN,
		     0, 0, 0, 0,
		     NULL, NULL, GetWindowWord (hwnd, GWW_HINSTANCE), NULL);

     /* Fijamos sus par metros */
     SetWindowWord (hwndGraficas[i][tipo], TIPO,      tipo);
     SetWindowWord (hwndGraficas[i][tipo], GRAFICA,   Grafica);
     SetWindowWord (hwndGraficas[i][tipo], TABLA,     Tabla);
     SetWindowWord (hwndGraficas[i][tipo], NDIVSX,    5);
     SetWindowWord (hwndGraficas[i][tipo], NDIVSY,    5);
     SetWindowWord (hwndGraficas[i][tipo], TRACKING,  FALSE);
     SetWindowWord (hwndGraficas[i][tipo], COORDS,    FALSE);
     SetWindowWord (hwndGraficas[i][tipo], COLORESC,  AZUL);
     SetWindowWord (hwndGraficas[i][tipo], COLORGRF,  ROJO);
     SetWindowWord (hwndGraficas[i][tipo], HWNDCOORDS,
		    CreateWindow (szCoordsClass, NULL,
				  WS_CHILDWINDOW | WS_BORDER,
				  0, 0, 0, 0, hwndGraficas[i][tipo], 1,
				  GetWindowWord (hwnd, GWW_HINSTANCE), NULL));

     /* Si es preciso, alteramos el men£ de la ventana */
     if ((tipo == PTRABAJO) || (tipo == SENSIBILIDAD))
       for (j = 0; j < 3; ++j)
	 EnableMenuItem (GetMenu (hwndGraficas[i][tipo]), j,
			 MF_BYPOSITION | MF_GRAYED);
     else
       EnableMenuItem (GetSubMenu (GetMenu (hwndGraficas[i][tipo]), 2),
		       (tipo == TIEMPO) ? IDM_TIEMPO : IDM_FRECUENCIA,
		       MF_GRAYED);

     /* Colocamos la ventana en la pantalla */
     SendMessage (hwndGraficas[i][tipo], WM_SYSCOMMAND, SC_ARRANGE, 0);

     ShowWindow (hwndGraficas[i][tipo], SW_SHOWNORMAL);
     UpdateWindow (hwndGraficas[i][tipo]);
    }
   else
     MessageBox (hwnd, "No hay espacio para m s gr ficas",
		 "Osciloscopio Digital", MB_ICONSTOP | MB_OK);
  }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Dibujar_PdeTrabajo (HWND hwnd, HDC hdc, PAINTSTRUCT ps)

/* Traza f¡sicamente los datos del punto de trabajo sobre la ventana de
   handle 'hwnd', con el contexto 'hdc' y ayud ndose de 'ps' */
{
 HCURSOR    hCursor;
 TEXTMETRIC tm;
 OFSTRUCT   of;
 double     valor;
 int        hFichero;
 short      cxChar, cyChar, contador, no_medidas = 0;
 char       szBuffer[50], nom[13], medida[16];

 /* Cambiamos el cursor al reloj */
 hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
 ShowCursor (TRUE);

 /* Obtenemos el nombre del circuito a partir de la ventana */
 GetWindowText (hwnd, (LPSTR) nom, 9);

 /* Abrimos el fichero de resultados */
 *strchr (nom, ' ') = 0;
 if ((hFichero = OpenFile (strcat (nom, ".PTR"), &of, OF_READ)) == -1)
   return (1);

 /* Obtenemos los tama¤os del texto */
 GetTextMetrics (hdc, &tm);
 cxChar = tm.tmAveCharWidth;
 cyChar = tm.tmHeight + tm.tmExternalLeading;

 *strrchr (nom, '.') = 0;
 do
 {
  if (strlen (nombre_medida (nom, 'P', no_medidas, medida)) > 0)
   {
    _lread (hFichero, (LPSTR) &valor, sizeof (valor));
    TextOut (hdc, 2*cxChar, no_medidas * cyChar, szBuffer,
	     sprintf (szBuffer, "%-10s = %+e", medida, valor));
    ++no_medidas;
   }
 }
 while (strlen (medida) > 0);

 _lclose (hFichero);

 /* Restauramos el cursor */
 ShowCursor (FALSE);
 SetCursor (hCursor);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Dibujar_Tiempo (HWND hwnd, HDC hdc, PAINTSTRUCT ps)

/* Traza f¡sicamente la gr fica de la respuesta temporal */
{
 HCURSOR       hCursor;
 HPEN          hPen, hViejoPen;
 HRGN          hRgn;
 TEXTMETRIC    tm;
 RECT          rect, rcPintar, rcAux;
 POINT         ptPoshandle;
 COLORREF      cr, crViejoColorTexto;
 FILE          *temp;
 datos_grafica dg;
 extremos_grafica extgr;
 double        leido;
 float         t, inc, x, y, stepx, stepy, escalax, escalay,
	       offsetx, offsety;
 DWORD         dwOrg;
 unsigned      n_divs_x, n_divs_y, divs_por_num_x, divs_por_num_y;
 short         nHposScrl, nVposScrl, i, cxChar, cyChar, numgraficas;
 BYTE          ColorEscala, ColorGrafica;
 char          nom[84], szBuffer[10], szPrimera[10], formato[5], *punt;

 /* Cambiamos el cursor al reloj */
 hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
 ShowCursor (TRUE);

 if (GetWindowWord (hwnd, GRAFICA))
  {
   /* Obtenemos la posici¢n del handle dentro de la gr fica */
   ptPoshandle = coords_handle (hwnd);

   /* Obtenemos los datos sobre extensiones y escalas */
   extgr = extGraficas[ptPoshandle.y][ptPoshandle.x - TIEMPO];

   /* Obtenemos las escalas */
   escalax = extgr.escalax;
   escalay = extgr.escalay;

   /* Obtenemos las divisiones de los ejes */
   n_divs_x = GetWindowWord (hwnd, NDIVSX);
   n_divs_y = GetWindowWord (hwnd, NDIVSY);

   /* Obtenemos los colores en forma de bits */
   ColorEscala  = (BYTE) GetWindowWord (hwnd, COLORESC);
   ColorGrafica = (BYTE) GetWindowWord (hwnd, COLORGRF);

   /* Obtenemos las posiciones de scroll */
   nHposScrl = GetScrollPos (hwnd, SB_HORZ);
   nVposScrl = GetScrollPos (hwnd, SB_VERT);

   /* Obtenemos los tama¤os del texto */
   GetTextMetrics (hdc, &tm);
   cxChar = tm.tmAveCharWidth;
   cyChar = tm.tmHeight + tm.tmExternalLeading;

   /* Obtenemos el tama¤o de la ventana y el de la regi¢n de gr fica */
   GetClientRect (hwnd, &rect);
   if (GetWindowWord (hwnd, TABLA))
     rect.right = min (max (rect.right - 22 * cxChar, 2 * rect.right / 3),
		       rect.right - 15 * cxChar);
   rect.right  -= 5 * cxChar + 5;
   rect.bottom -= 2 * cyChar + 5;
   hRgn = CreateRectRgn (5, 5, 5 + rect.right + 1, 5 + rect.bottom + 1);

   /* Calculamos cu ntos n£meros entran en los ejes */
   divs_por_num_x = max (1, ceil ((float)n_divs_x /
				  ((float)rect.right / (float)(4*cxChar))));
   divs_por_num_y = max (1, ceil ((float)n_divs_y /
				  ((float)rect.bottom / (float)cyChar)));

   /* Obtenemos los datos de la gr fica */
   GetWindowText (hwnd, (LPSTR) nom, 9);
   *strchr (nom, ' ') = 0;
   dg = obtener_dg (nom, TIEMPO);

   /* Seleccionamos el color para los ejes */
   if (ColorEscala < 8)
     cr = RGB((ColorEscala & ROJO)	 * 255,
	      ((ColorEscala & VERDE) >> 1) * 255,
	      ((ColorEscala & AZUL)  >> 2) * 255);
   else
     cr = RGB((ColorEscala & ROJO)	 * 128,
	      ((ColorEscala & VERDE) >> 1) * 128,
	      ((ColorEscala & AZUL)  >> 2) * 128);

   hPen = CreatePen (PS_SOLID, 1, cr);
   hViejoPen = SelectObject (hdc, hPen);

   /* Cambiamos el modo de mapeo */
   SetMapMode (hdc, MM_ANISOTROPIC);

   /* Fijamos las escalas */
   SetWindowExt   (hdc, rect.right,  rect.bottom);
   SetViewportExt (hdc, rect.right, -rect.bottom);
   stepx = (float) rect.right / (float) n_divs_x;
   stepy = (float) rect.bottom / (float) n_divs_y;

   /* Fijamos el origen del viewport */
   SetViewportOrg (hdc, 5, 5 + rect.bottom);

   /* Establecemos las posiciones de inicio y fin de la pintada */
   rcAux = ps.rcPaint;
   DPtoLP (hdc, (LPPOINT) &rcAux, 2);
   rcPintar.left   = max (0,        ceil  ((float) rcAux.left   / stepx));
   rcPintar.right  = min (n_divs_x, floor ((float) rcAux.right  / stepx));
   rcPintar.top    = max (0,        ceil  ((float) rcAux.bottom / stepy));
   rcPintar.bottom = min (n_divs_y, floor ((float) rcAux.top    / stepy));

   /* Calculamos el inc. lineal de tiempo */
   inc = (dg.fin - dg.ini) / ((float) n_divs_x * escalax);

   /* Dibujamos la escala del eje x. Dividimos la ventana en zonas iguales en
      base al valor de stepx. */
   for (x = rcPintar.left, t = dg.ini + (rcPintar.left + nHposScrl) * inc;
	x <= rcPintar.right; ++x, t += inc)
    {
     MoveTo (hdc, x * stepx, min (rcAux.top, (float) n_divs_y * stepy));
     LineTo (hdc, x * stepx, max (rcAux.bottom, 0));

     if (((int)x % divs_por_num_x) == 0)
       TextOut (hdc, x * stepx - cxChar, -cyChar/2,
		szBuffer, sprintf (szBuffer, "%3.2f", norm (t)));
    }

   /* Calculamos el inc. lineal de se¤al */
   inc = (extgr.max - extgr.min) / ((float) n_divs_y * escalay);

   /* Dibujamos la escala vertical */
   for (y = rcPintar.top, t = extgr.min +
			      (rcPintar.top + (nVmaxScrl - nVposScrl)) * inc;
	y <= rcPintar.bottom; ++y, t += inc)
    {
     MoveTo (hdc, max (0, 		       rcAux.left),  y * stepy);
     LineTo (hdc, min ((float) n_divs_x * stepx, rcAux.right), y * stepy);

     if (((int)y % divs_por_num_y) == 0)
       TextOut (hdc, rect.right + cxChar/2, y * stepy + cyChar/2,
		szBuffer, sprintf (szBuffer, "%3.2f", norm (t)));
    }

   /* Calculamos los offsets debidos a las posiciones de scroll */
   offsetx = (float) nHposScrl * stepx;
   offsety = (float) (nVmaxScrl - nVposScrl) * stepy;

   /* Fijamos el nuevo origen para la gr fica */
   SetViewportOrg (hdc, 5 - offsetx, 5 + rect.bottom + offsety);

   /* Limitamos la regi¢n de pintada */
   SelectClipRgn (hdc, hRgn);

   /* Fijamos las nuevas escalas para la gr fica */
   stepx = escalax * (float) rect.right / (float) dg.n_puntos;
   stepy = escalay * (float) rect.bottom / (extgr.max - extgr.min);

   /* Establecemos las posiciones de inicio y fin de la pintada */
   rcAux = ps.rcPaint;
   DPtoLP (hdc, (LPPOINT) &rcAux, 2);
   rcPintar.left   = max (1 + floor (offsetx / stepx),
			  floor ((float) rcAux.left  / stepx));
   rcPintar.right  = min (ceil (((float) rect.right + offsetx) / stepx),
			  ceil  ((float) rcAux.right / stepx));

   /* Abrimos el fichero de tiempo */
   if ((temp = fopen (strcat (nom, ".TMP"), "rb")) == NIL)
     return (1);

   /* Calculamos el n£mero de gr ficas que hay en el fichero */
   numgraficas = (filelength (fileno (temp)) - sizeof (datos_grafica)) /
		 (sizeof (double) * dg.n_puntos);

   /* Representamos todas las gr ficas */
   for (i = 0; i < numgraficas; ++i, ++ColorGrafica)
    {
     /* Eliminamos el l piz anterior */
     DeleteObject (hPen);

     /* Seleccionamos el color para la gr fica */
     if (ColorGrafica < 8)
       cr = RGB((ColorGrafica & ROJO)	 * 255,
		((ColorGrafica & VERDE) >> 1) * 255,
		((ColorGrafica & AZUL)  >> 2) * 255);
     else
       cr = RGB((ColorGrafica & ROJO)	 * 128,
		((ColorGrafica & VERDE) >> 1) * 128,
		((ColorGrafica & AZUL)  >> 2) * 128);

     hPen = CreatePen (PS_SOLID, 1, cr);
     SelectObject (hdc, hPen);

     /* Nos vamos a las coords. de la muestra anterior a la primera a pintar */
     fseek (temp, sizeof(datos_grafica) + i * dg.n_puntos*sizeof(double) +
	    (rcPintar.left - 1) * sizeof (leido), SEEK_SET);
     fread (&leido, sizeof(leido), 1, temp);
     MoveTo (hdc, 0, (leido - extgr.min) * stepy);

     /* Unimos las coords. de las muestras con l¡neas */
     for (x = rcPintar.left; x <= rcPintar.right; ++x)
      {
       /* Obtenemos la coordenada vertical */
       fread (&leido, sizeof(leido), 1, temp);

       /* Trazamos la l¡nea */
       LineTo (hdc, x * stepx, (leido - extgr.min) * stepy);
      }

     /* Representamos la clave en la esquina sup. dcha. de la gr fica */
     *strrchr (nom, '.') = 0;
     if (strlen (nombre_medida (nom, 'T', i, szBuffer)) > 0)
      {
       crViejoColorTexto = SetTextColor (hdc, cr);
       dwOrg = SetViewportOrg (hdc, 5, 5 + rect.bottom);
       SetBkMode (hdc, TRANSPARENT);
       TextOut (hdc, rect.right - cxChar * strlen (szBuffer),
		     rect.bottom - i * cyChar,
		     szBuffer, strlen (szBuffer));
       SetBkMode (hdc, OPAQUE);
       SetViewportOrg (hdc, LOWORD(dwOrg), HIWORD(dwOrg));
       SetTextColor (hdc, crViejoColorTexto);

       /* Obtenemos el string de id. para la tabla */
       if (i == 0)
	 strcpy (szPrimera, szBuffer);
      }
    }

   /* Cerramos el fichero */
   fclose (temp);

   /* Reseleccionamos el antiguo l piz */
   SelectObject (hdc, hViejoPen);

   /* Eliminamos el l piz rojo */
   DeleteObject (hPen);

   /* Eliminamos la regi¢n de pintado */
   DeleteObject (hRgn);

   /* Reseleccionamos el antiguo modo de mapeo y sus caracter¡sticas */
   SetMapMode (hdc, MM_TEXT);
   SetWindowOrg   (hdc, 0, 0);
   SetViewportOrg (hdc, 0, 0);
  }

 /* Dibujamos la tabla de valores */
 if (GetWindowWord (hwnd, TABLA))
  {
   /* Calculamos las coordenadas del rect ngulo que ocupa la tabla */
   GetClientRect (hwnd, &rect);
   if (GetWindowWord (hwnd, GRAFICA))
     rect.left   = min (max (rect.right - 22*cxChar + 5, 2 * rect.right / 3 + 5),
			rect.right - 15*cxChar + 5);
   else
     rect.left = 5;
   rect.top    = 5;
   rect.right  = rect.right - 5;
   rect.bottom = rect.bottom - 5;

   /* Obtenemos el string de formato por defecto para los datos */
   strcpy (formato, "%1.1e");

   /* Calculamos cu ntos caracteres caben en la columna menos 2 */
   i = min (8, (rect.right - rect.left) / 2 / cxChar - 8);

   /* Modificamos el string de formato para acomodar dichos caracteres */
   if (i >= 0)
     formato[3] = '1' + i;
   else
    {
     formato[4] = 'f';
     i = (rect.right - rect.left) / 2 / cxChar - 4;
     formato[3] = '1' + i;
    }

   /* Creamos una regi¢n y la seleccionamos en el contexto */
   hRgn = CreateRectRgn (rect.left, rect.top, rect.right, rect.bottom);
   SelectClipRgn (hdc, hRgn);

   /* Dibujamos el esqueleto de la tabla */
   Rectangle (hdc, rect.left, rect.top, rect.right, rect.bottom);
   MoveTo (hdc, rect.left,  rect.top + 3 * cyChar / 2);
   LineTo (hdc, rect.right, rect.top + 3 * cyChar / 2);
   MoveTo (hdc, rect.left + (rect.right - rect.left) / 2, rect.top);
   LineTo (hdc, rect.left + (rect.right - rect.left) / 2, rect.bottom);

   /* Escribimos el encabezamiento */
   TextOut (hdc, rect.left + (rect.right - rect.left) / 4 - cxChar / 2,
		 rect.top + cyChar / 4, "t", 1);
   TextOut (hdc, rect.left + 3 * (rect.right - rect.left) / 4 -
		 strlen (szPrimera) * cxChar / 2,
		 rect.top + cyChar / 4, szPrimera, strlen (szPrimera));

   /* Calculamos el rango de valores que caben en la tabla */
   rcPintar.left  = floor (offsetx / stepx);
   rcPintar.right = rcPintar.left + (rect.bottom - rect.top) / cyChar - 2;

   /* Obtenemos el salto de tiempo */
   inc = (dg.fin - dg.ini) / dg.n_puntos;

   /* Escribimos los n£meros en la tabla. Primero, se abre el fichero */
   temp = fopen (nom, "rb");

   /* Buscamos en el fichero el primer valor de la tabla */
   fseek (temp, sizeof (datos_grafica) + rcPintar.left * sizeof (double),
	  SEEK_SET);

   /* Escribimos los valores de tiempo y se¤al */
   for (x = rcPintar.left; x < rcPintar.right; ++x)
    {
     t = x * inc;
     fread (&leido, sizeof (leido), 1, temp);

     TextOut (hdc, rect.left + cxChar / 2,
		   rect.top + (2 + x - rcPintar.left) * cyChar,
		   szBuffer, sprintf (szBuffer, formato, t));
     TextOut (hdc, rect.left + (rect.right - rect.left) / 2 + cxChar / 2,
		   rect.top + (2 + x - rcPintar.left) * cyChar,
		   szBuffer, sprintf (szBuffer, formato, leido));
    }

   /* Finalmente, se cierra el fichero */
   fclose (temp);
  }

 /* Restauramos el cursor */
 ShowCursor (FALSE);
 SetCursor (hCursor);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Dibujar_Frecuencia (HWND hwnd, HDC hdc, PAINTSTRUCT ps)

/* Traza f¡sicamente la gr fica de la respuesta frecuencial */
{
 HCURSOR       hCursor;
 HPEN          hPen, hViejoPen;
 HRGN	       hRgn;
 TEXTMETRIC    tm;
 RECT          rect, rcPintar, rcAux;
 POINT         ptPoshandle;
 COLORREF      cr, crViejoColorTexto;
 FILE          *frec;
 datos_grafica dg;
 extremos_grafica extgr;
 complejo      leido;
 modarg        valor;
 float         t, inc, x, y, stepx, stepy, escalax, escalay,
	       offsetx, offsety;
 DWORD         dwOrg;
 unsigned      n_divs_x, n_divs_y, divs_por_num_x, divs_por_num_y;
 short         nHposScrl, nVposScrl, i, cxChar, cyChar, numgraficas;
 BYTE          ColorEscala, ColorGrafica;
 char          nom[84], szBuffer[10], szPrimera[10], formato[5], *punt;

 /* Cambiamos el cursor al reloj */
 hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
 ShowCursor (TRUE);

 if (GetWindowWord (hwnd, GRAFICA))
  {
   /* Obtenemos la posici¢n del handle dentro de la gr fica */
   ptPoshandle = coords_handle (hwnd);

   /* Obtenemos los datos sobre extensiones y escalas */
   extgr = extGraficas[ptPoshandle.y][ptPoshandle.x - TIEMPO];

   /* Obtenemos las escalas */
   escalax = extgr.escalax;
   escalay = extgr.escalay;

   /* Obtenemos las divisiones de los ejes */
   n_divs_x = GetWindowWord (hwnd, NDIVSX);
   n_divs_y = GetWindowWord (hwnd, NDIVSY);

   /* Obtenemos los colores en forma de bits */
   ColorEscala  = (BYTE) GetWindowWord (hwnd, COLORESC);
   ColorGrafica = (BYTE) GetWindowWord (hwnd, COLORGRF);

   /* Obtenemos las posiciones de scroll */
   nHposScrl = GetScrollPos (hwnd, SB_HORZ);
   nVposScrl = GetScrollPos (hwnd, SB_VERT);

   /* Obtenemos los tama¤os del texto */
   GetTextMetrics (hdc, &tm);
   cxChar = tm.tmAveCharWidth;
   cyChar = tm.tmHeight + tm.tmExternalLeading;

   /* Obtenemos el tama¤o de la ventana y el de la regi¢n de gr fica */
   GetClientRect (hwnd, &rect);
   if (GetWindowWord (hwnd, TABLA))
     rect.right = min (max (rect.right - 22 * cxChar, 2 * rect.right / 3),
		       rect.right - 15 * cxChar);
   rect.right  -= 5 * cxChar + 5;
   rect.bottom -= 2 * cyChar + 5;
   hRgn = CreateRectRgn (5, 5, 5 + rect.right + 1, 5 + rect.bottom + 1);

   /* Calculamos cu ntos n£meros entran en los ejes */
   divs_por_num_x = max (1, ceil ((float)n_divs_x /
				  ((float)rect.right / (float)(4*cxChar))));
   divs_por_num_y = max (1, ceil ((float)n_divs_y /
				  ((float)rect.bottom / (float)cyChar)));

   /* Obtenemos los datos de la gr fica */
   GetWindowText (hwnd, (LPSTR) nom, 9);
   *strchr (nom, ' ') = 0;
   dg = obtener_dg (nom, GetWindowWord (hwnd, TIPO));
   strcat (nom, ext[GetWindowWord (hwnd, TIPO) - TIEMPO]);

   /* Seleccionamos el color para los ejes */
   if (ColorEscala < 8)
     cr = RGB((ColorEscala & ROJO)	 * 255,
	      ((ColorEscala & VERDE) >> 1) * 255,
	      ((ColorEscala & AZUL)  >> 2) * 255);
   else
     cr = RGB((ColorEscala & ROJO)	 * 128,
	      ((ColorEscala & VERDE) >> 1) * 128,
	      ((ColorEscala & AZUL)  >> 2) * 128);

   hPen = CreatePen (PS_SOLID, 1, cr);
   hViejoPen = SelectObject (hdc, hPen);

   /* Cambiamos el modo de mapeo */
   SetMapMode (hdc, MM_ANISOTROPIC);

   /* Fijamos las escalas */
   SetWindowExt   (hdc, rect.right,  rect.bottom);
   SetViewportExt (hdc, rect.right, -rect.bottom);

   /* Fijamos el origen del viewport */
   SetViewportOrg (hdc, 5, 5 + rect.bottom);

   if (dg.escala == 'L')
    {
     /* Calculamos el incremento lineal por divisi¢n de la rejilla de escalas */
     stepx = (float) rect.right / log10 ((float) n_divs_x + 1);
     stepy = (float) rect.bottom / (float) n_divs_y;

     /* Modificamos las extensiones horizontal y vertical */
     extgr.max = (extgr.max <= 0) ? -100 : 20 * log10 (extgr.max);
     extgr.min = (extgr.min <= 0) ? -100 : 20 * log10 (extgr.min);

     /* Calculamos el inc. lineal de frecuencia */
     inc = (dg.fin - dg.ini) / ((float) n_divs_x * escalax);

     /* Dibujamos la escala del eje x. Dividimos la ventana en zonas iguales en
	base al valor de stepx. */
     for (x = 1, t = dg.ini + nHposScrl * inc;
	  x <= n_divs_x + 1; ++x, t += inc)
      {
       MoveTo (hdc, stepx * log10 (x), 0);
       LineTo (hdc, stepx * log10 (x), rect.bottom);

       if (((int)x % divs_por_num_x) == 0)
	 TextOut (hdc, stepx * log10 (x) - cxChar, -cyChar/2,
		  szBuffer, sprintf (szBuffer, "%3.2f", norm (t)));
      }

     /* Calculamos el inc. lineal de se¤al */
     inc = (extgr.max - extgr.min) / ((float) n_divs_y * escalay);

     /* Dibujamos la escala vertical */
     for (y = 0, t = extgr.min + nVposScrl * inc;
	  y <= n_divs_y; ++y, t += inc)
      {
       MoveTo (hdc, 0, 	    y * stepy);
       LineTo (hdc, rect.right, y * stepy);

       if (((int)y % divs_por_num_y) == 0)
	 TextOut (hdc, rect.right + cxChar/2, y * stepy + cyChar/2,
		  szBuffer, sprintf (szBuffer, "%3.2f", norm (t)));
      }
    }
   else
    {
     /* Calculamos el incremento por divisi¢n de la rejilla de escalas */
     stepx = (float) rect.right / (float) n_divs_x;
     stepy = (float) rect.bottom / (float) n_divs_y;

     /* Establecemos las posiciones de inicio y fin de la pintada */
     rcAux = ps.rcPaint;
     DPtoLP (hdc, (LPPOINT) &rcAux, 2);
     rcPintar.left   = max (0,        ceil  ((float) rcAux.left   / stepx));
     rcPintar.right  = min (n_divs_x, floor ((float) rcAux.right  / stepx));
     rcPintar.top    = max (0,        ceil  ((float) rcAux.bottom / stepy));
     rcPintar.bottom = min (n_divs_y, floor ((float) rcAux.top    / stepy));

     /* Calculamos el inc. lineal de frecuencia */
     inc = (dg.fin - dg.ini) / ((float) n_divs_x * escalax);

     /* Dibujamos la escala del eje x. Dividimos la ventana en zonas iguales en
	base al valor de stepx. */
     for (x = rcPintar.left, t = dg.ini + (rcPintar.left + nHposScrl) * inc;
	  x <= rcPintar.right; ++x, t += inc)
      {
       MoveTo (hdc, x * stepx, min (rcAux.top, (float) n_divs_y * stepy));
       LineTo (hdc, x * stepx, max (rcAux.bottom, 0));

       if (((int)x % divs_por_num_x) == 0)
	 TextOut (hdc, x * stepx - cxChar, -cyChar/2,
		  szBuffer, sprintf (szBuffer, "%3.2f", norm (t)));
      }

     /* Calculamos el inc. lineal de se¤al */
     inc = (extgr.max - extgr.min) / ((float) n_divs_y * escalay);

     /* Dibujamos la escala vertical */
     for (y = rcPintar.top, t = extgr.min +
				(rcPintar.top + (nVmaxScrl - nVposScrl)) * inc;
	  y <= rcPintar.bottom; ++y, t += inc)
      {
       MoveTo (hdc, max (0, 		       rcAux.left),  y * stepy);
       LineTo (hdc, min ((float) n_divs_x * stepx, rcAux.right), y * stepy);

       if (((int)y % divs_por_num_y) == 0)
	 TextOut (hdc, rect.right + cxChar/2, y * stepy + cyChar/2,
		  szBuffer, sprintf (szBuffer, "%3.2f", norm (t)));
      }
    }

   /* Calculamos los offsets debidos a las posiciones de scroll */
   offsetx = (float) nHposScrl * (float) rect.right / (float) n_divs_x;
   offsety = (float) (nVmaxScrl - nVposScrl) * stepy;

   /* Fijamos el nuevo origen para la gr fica */
   SetViewportOrg (hdc, 5 - offsetx, 5 + rect.bottom + offsety);

   /* Limitamos la regi¢n de pintada */
   SelectClipRgn (hdc, hRgn);

   /* Abrimos el fichero de frecuencia */
   if ((frec = fopen (nom, "rb")) == NIL)
     return (1);

   /* Calculamos el n£mero de gr ficas que hay en el fichero */
   numgraficas = (filelength (fileno (frec)) - sizeof (datos_grafica)) /
		 (sizeof (complejo) * dg.n_puntos);

   /* Representamos todas las gr ficas */
   for (i = 0; i < numgraficas; ++i, ++ColorGrafica)
    {
     /* Eliminamos el l piz anterior */
     DeleteObject (hPen);

     /* Seleccionamos el color para la gr fica */
     if (ColorGrafica < 8)
       cr = RGB((ColorGrafica & ROJO)	 * 255,
		((ColorGrafica & VERDE) >> 1) * 255,
		((ColorGrafica & AZUL)  >> 2) * 255);
     else
       cr = RGB((ColorGrafica & ROJO)	 * 128,
		((ColorGrafica & VERDE) >> 1) * 128,
		((ColorGrafica & AZUL)  >> 2) * 128);

     hPen = CreatePen (PS_SOLID, 1, cr);
     SelectObject (hdc, hPen);

     /* Nos vamos a las coords. de la muestra anterior a la primera a pintar */
     fseek (frec, sizeof(datos_grafica) + i * dg.n_puntos*sizeof(complejo) +
	    (0) * sizeof(leido), SEEK_SET);
     fread (&leido, sizeof(leido), 1, frec);
     valor = polar (leido);

     /* Fijamos las nuevas escalas para la gr fica */
     stepx = escalax * (float) rect.right / (float) dg.n_puntos;
     stepy = escalay * (float) rect.bottom / (extgr.max - extgr.min);

     /* Nos colocamos en la posici¢n de la gr fica correspondiente al
	1er punto de la misma */
     if (dg.escala == 'L')
       valor.mod = 20 * log10 (valor.mod);
     MoveTo (hdc, 0, (valor.mod - extgr.min) * stepy);

     /* Establecemos las posiciones de inicio y fin de la pintada */
     rcAux = ps.rcPaint;
     DPtoLP (hdc, (LPPOINT) &rcAux, 2);
     rcPintar.left   = max (1 + floor (offsetx / stepx),
			    floor ((float) rcAux.left  / stepx));
     rcPintar.right  = min (ceil (((float) rect.right + offsetx) / stepx),
			    ceil  ((float) rcAux.right / stepx));

     /* Unimos las coords. de las muestras con l¡neas */
     for (x = rcPintar.left; x <= rcPintar.right; ++x)
      {
       /* Obtenemos la coordenada vertical */
       fread (&leido, sizeof(leido), 1, frec);
       valor = polar (leido);

       /* Trazamos la l¡nea */
       if (dg.escala == 'L')
	 valor.mod = 20 * log10 (valor.mod);
       LineTo (hdc, x * stepx, (valor.mod - extgr.min) * stepy);
      }

     /* Representamos la clave en la esquina sup. dcha. de la gr fica */
     *strrchr (nom, '.') = 0;
     if (strlen (nombre_medida (nom, 'F', i, szBuffer)) > 0)
      {
       crViejoColorTexto = SetTextColor (hdc, cr);
       dwOrg = SetViewportOrg (hdc, 5, 5 + rect.bottom);
       SetBkMode (hdc, TRANSPARENT);
       TextOut (hdc, rect.right - cxChar * strlen (szBuffer),
		     rect.bottom - i * cyChar,
		     szBuffer, strlen (szBuffer));
       SetBkMode (hdc, OPAQUE);
       SetViewportOrg (hdc, LOWORD(dwOrg), HIWORD(dwOrg));
       SetTextColor (hdc, crViejoColorTexto);

       /* Obtenemos el string de id. para la tabla */
       if (i == 0)
	 strcpy (szPrimera, szBuffer);
      }
    }

   /* Cerramos el fichero */
   fclose (frec);

   /* Reseleccionamos el antiguo l piz */
   SelectObject (hdc, hViejoPen);

   /* Eliminamos el l piz rojo */
   DeleteObject (hPen);

   /* Eliminamos la regi¢n de pintado */
   DeleteObject (hRgn);

   /* Reseleccionamos el antiguo modo de mapeo y sus caracter¡sticas */
   SetMapMode (hdc, MM_TEXT);
   SetWindowOrg   (hdc, 0, 0);
   SetViewportOrg (hdc, 0, 0);
  }

 /* Dibujamos la tabla de valores */
 if (GetWindowWord (hwnd, TABLA))
  {
   /* Calculamos las coordenadas del rect ngulo que ocupa la tabla */
   GetClientRect (hwnd, &rect);
   if (GetWindowWord (hwnd, GRAFICA))
     rect.left   = min (max (rect.right - 22*cxChar + 5, 2 * rect.right / 3 + 5),
			rect.right - 15*cxChar + 5);
   else
     rect.left = 5;
   rect.top    = 5;
   rect.right  = rect.right - 5;
   rect.bottom = rect.bottom - 5;

   /* Obtenemos el string de formato por defecto para los datos */
   strcpy (formato, "%1.1e");

   /* Calculamos cu ntos caracteres caben en la columna menos 2 */
   i = min (8, (rect.right - rect.left) / 2 / cxChar - 8);

   /* Modificamos el string de formato para acomodar dichos caracteres */
   if (i >= 0)
     formato[3] = '1' + i;
   else
    {
     formato[4] = 'f';
     i = (rect.right - rect.left) / 2 / cxChar - 4;
     formato[3] = '1' + i;
    }

   /* Creamos una regi¢n y la seleccionamos en el contexto */
   hRgn = CreateRectRgn (rect.left, rect.top, rect.right, rect.bottom);
   SelectClipRgn (hdc, hRgn);

   /* Dibujamos el esqueleto de la tabla */
   Rectangle (hdc, rect.left, rect.top, rect.right, rect.bottom);
   MoveTo (hdc, rect.left,  rect.top + 3 * cyChar / 2);
   LineTo (hdc, rect.right, rect.top + 3 * cyChar / 2);
   MoveTo (hdc, rect.left + (rect.right - rect.left) / 2, rect.top);
   LineTo (hdc, rect.left + (rect.right - rect.left) / 2, rect.bottom);

   /* Escribimos el encabezamiento */
   TextOut (hdc, rect.left + (rect.right - rect.left) / 4 - cxChar / 2,
		 rect.top + cyChar / 4, "f", 1);
   TextOut (hdc, rect.left + 3 * (rect.right - rect.left) / 4 -
		 strlen (szPrimera) * cxChar / 2,
		 rect.top + cyChar / 4, szPrimera, strlen (szPrimera));

   /* Se abre el fichero */
   frec = fopen (nom, "rb");

   if (dg.escala == 'L')
    {
    }
   else
    {
     /* Calculamos el rango de valores que caben en la tabla */
     rcPintar.left  = floor (offsetx / stepx);
     rcPintar.right = rcPintar.left + (rect.bottom - rect.top) / cyChar - 2;

     /* Obtenemos el salto de frecuencia */
     inc = (dg.fin - dg.ini) / dg.n_puntos;

     /* Buscamos en el fichero el primer valor de la tabla */
     fseek (frec, sizeof (datos_grafica) + rcPintar.left * sizeof (complejo),
	    SEEK_SET);

     /* Escribimos los valores de frecuencia y se¤al */
     for (x = rcPintar.left; x < rcPintar.right; ++x)
      {
       t = x * inc;
       fread (&leido, sizeof (leido), 1, frec);
       valor = polar (leido);

       TextOut (hdc, rect.left + cxChar / 2,
		     rect.top + (2 + x - rcPintar.left) * cyChar,
		     szBuffer, sprintf (szBuffer, formato, t));
       TextOut (hdc, rect.left + (rect.right - rect.left) / 2 + cxChar / 2,
		     rect.top + (2 + x - rcPintar.left) * cyChar,
		     szBuffer, sprintf (szBuffer, formato, valor.mod));
      }
    }

   /* Finalmente, se cierra el fichero */
   fclose (frec);
  }

 /* Restauramos el cursor */
 ShowCursor (FALSE);
 SetCursor (hCursor);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Dibujar_Fourier (HWND hwnd, HDC hdc, PAINTSTRUCT ps)

/* Traza f¡sicamente la tda. de Fourier de la respuesta temporal */
{
 Dibujar_Frecuencia (hwnd, hdc, ps);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Dibujar_Sensibilidad (HWND hwnd, HDC hdc, PAINTSTRUCT ps)

/* Traza f¡sicamente los datos de la sensibilidad del circuito */
{

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

char *nombre_medida (char fichero[9], char dominio, short n, char *resultado)

/* Devuelve en 'resultado' un string que contiene el nombre de la medida
   n§ 'n' en el dominio 'dominio' del fichero cuyo nombre es 'fichero' */
{
 OFSTRUCT    of;
 respuesta   resp;
 int         hFichero;
 char        nom[13], aux[10];

 /* Obtenemos el nombre de fichero */
 strcpy (nom, fichero);

 /* Abrimos el fichero de petici¢n de resultados */
 strcat (nom, ".RES");
 if ((hFichero = OpenFile (nom, &of, OF_READ)) == -1)
   return (NULL);

 /* Localizamos la en‚sima respuesta en el dominio 'dominio' */
 ++n;     /* Aplicamos un offset de 1 porque n = 0 es la primera respuesta */
 do
 {
  _lread (hFichero, (LPSTR) &resp, sizeof (resp));
  if (resp.dominio == dominio)
   --n;
 }
 while ((n > 0) && (!eof (hFichero)));

 if (n == 0)
  {
   resultado[0] = resp.tipo;
   resultado[1] = '(';
   resultado[2] = 0;
   if (resp.tipo == 'V')
    {
     strcat (resultado, itoa (resp.medida.nodos[0], aux, 10));
     strcat (resultado, ",");
     strcat (resultado, itoa (resp.medida.nodos[1], aux, 10));
    }
   else
     strcat (resultado, resp.medida.elem);
   strcat (resultado, ")");
  }
 else
   resultado[0] = 0;

 _lclose (hFichero);

 return (resultado);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

datos_grafica obtener_dg (char *fichero, int tipo)

/* Obtiene del fichero 'fichero' los datos de la gr fica */
{
 static char   *ext[] = { ".TMP", ".FRC", ".FOU" };
 int           hfichero;
 OFSTRUCT      of;
 char          nom[13];
 datos_grafica dg;
 short         i;

 /* Inicializaci¢n de 'dg' */
 dg.n_puntos = 0;

 /* Obtenemos el nombre del fichero con su extensi¢n */
 strcpy (nom, fichero);
 strcat (nom, ext[tipo - TIEMPO]);

 if ((hfichero = OpenFile (nom, &of, OF_READ)) != -1)
  {
   _lread (hfichero, (LPSTR) &dg, sizeof (dg));
   _lclose (hfichero);
  }

 return (dg);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extremos_grafica obtener_ext (char *fichero, int tipo, int nIndice)

/* Obtiene del fichero 'fichero' los valores m ximo y m¡nimo de la gr fica
   n§ 'nIndice' */
{
 int              hfichero;
 HCURSOR          hCursor;
 OFSTRUCT         of;
 char             nom[13];
 datos_grafica    dg;
 extremos_grafica extgr;
 double           leidoT;
 complejo         leidoF;
 modarg           datoF;
 unsigned	  i;

 /* Cambiamos el cursor al reloj */
 hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
 ShowCursor (TRUE);

 /* Obtenemos el nombre del fichero con su extensi¢n */
 strcpy (nom, fichero);
 strcat (nom, ext[tipo - TIEMPO]);

 /* Si el fichero existe.. */
 if ((hfichero = OpenFile (nom, &of, OF_READ)) != -1)
  {
   /* ..inicializaci¢n de 'extgr' */
   extgr.max = -1.7e308;
   extgr.min = +1.7e308;
   extgr.escalax = extgr.escalay = 1;

   if (tipo == TIEMPO)
    {
     /* Avanzamos hasta la gr fica 'nIndice' */
     _lread (hfichero, (LPSTR) &dg, sizeof (dg));
     _llseek (hfichero, nIndice * dg.n_puntos * sizeof (leidoT), 1);

     for (i = 0; i < dg.n_puntos; ++i)
      {
       /* Leemos un valor del fichero */
       _lread (hfichero, (LPSTR) &leidoT, sizeof (leidoT));

       /* Actualizamos los valores m ximo y m¡nimo */
       extgr.max = max (extgr.max, leidoT);
       extgr.min = min (extgr.min, leidoT);
      }
    }
   else
    {
     /* Avanzamos hasta la gr fica 'nIndice' */
     _lread (hfichero, (LPSTR) &dg, sizeof (dg));
//   _llseek (hfichero, nIndice * dg.n_puntos * sizeof (leidoF), 1);

     /* Obtenemos los valores m ximo y m¡nimo de m¢dulo y fase del fichero */
     for (i = 0; i < dg.n_puntos; ++i)
      {
       /* Leemos un complejo del fichero */
       _lread (hfichero, (LPSTR) &leidoF, sizeof (leidoF));

       /* Lo convertimos a forma m¢dulo-fase */
       datoF = polar (leidoF);

       /* Actualizamos los valores de m¢dulo m ximo y m¡nimo */
       extgr.max = max (extgr.max, datoF.mod);
       extgr.min = min (extgr.min, datoF.mod);

       /* Actualizamos los valores de fase m xima y m¡nima */
      }
    }

   _lclose (hfichero);
  }

 /* Si ocurre que todos los valores son iguales, tomamos una extensi¢n
    vertical razonable */
 if (extgr.max == extgr.min)
  {
   extgr.max += 1e-6;
   extgr.min -= 1e-6;
  }

 /* Restauramos el cursor */
 ShowCursor (FALSE);
 SetCursor (hCursor);

 return (extgr);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

POINT coords_handle (HWND hwnd)

/* Busca 'hwnd' en la tabla de handles y devuelve su posici¢n */
{
 POINT p;

 for (p.y = 0; p.y < numgraficas; ++p.y)
  {
   for (p.x = 0; p.x < 5; ++p.x)
     if (hwndGraficas[p.y][p.x] == hwnd)
       break;

   if (hwndGraficas[p.y][p.x] == hwnd)
     break;
  }

 return (p);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

RECT coords_ventana (HWND hwnd)

/* Calcula las coordenadas y el tama¤o de la ventana de handle 'hwnd' en base
   a su posici¢n en la tabla de handles */
{
 HDC	    hdc;
 RECT       r;
 POINT      p;
 TEXTMETRIC tm;
 short      cxChar, cyChar;


 /* Primero la buscamos */
 p = coords_handle (hwnd);

 /* Obtenemos los tama¤os del texto */
 hdc = CreateIC ("DISPLAY", NULL, NULL, NULL);
 GetTextMetrics (hdc, &tm);
 DeleteDC (hdc);

 cxChar = tm.tmAveCharWidth;
 cyChar = tm.tmHeight + tm.tmExternalLeading;

 /* Obtenemos el tama¤o y posici¢n de la ventana */
 r.left = (GetSystemMetrics (SM_CXSCREEN) / 3) * (p.y % 3) +
	  cxChar * p.x;
 r.top  = (GetSystemMetrics (SM_CYSCREEN) / 3) * (p.y / 3) +
	  cyChar * p.x;
 r.right  = GetSystemMetrics (SM_CXSCREEN) / 3 - 4 * cxChar;
 r.bottom = GetSystemMetrics (SM_CYSCREEN) / 3 - 4 * cxChar;

 return (r);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int coords_cursor (HWND hwnd, RECT rect, POINT *p, float *c)

/* Calcula la posici¢n del cursor en modo Tracking en la ventana de handle
   'hwnd', estando en '*p' su posici¢n actual dentro de dicha ventana y en
   'rect' el tama¤o del  rea de gr fica. Tambi‚n calcula los valores de
   x e y correspondientes a la gr fica, devolvi‚ndolos en '*c', y el valor
   de la muestra de la se¤al que corresponde a la posici¢n horizontal del
   cursor dentro de la gr fica, devolvi‚ndolo en p->y */
{
 POINT            ch;
 OFSTRUCT      	  of;
 datos_grafica    dg;
 extremos_grafica extgr;
 complejo         leido;
 double           valor;
 float 	          stepx, stepy, offsetx, offsety;
 WORD             tipo;
 int              muestra, hFichero, nMin, nMax;
 char 	          nom[13];

 /* Obtenemos el fichero que da origen a la gr fica */
 GetWindowText (hwnd, nom, 9);
 *strchr (nom, ' ') = 0;

 /* Obtenemos el n§ que identifica el tipo de gr fica */
 tipo = GetWindowWord (hwnd, TIPO);

 /* Obtenemos los datos de la gr fica */
 dg = obtener_dg (nom, tipo);
 ch = coords_handle (hwnd);
 extgr = extGraficas[ch.y][ch.x - 1];

 /* Calculamos el paso. El paso y se calcula de forma inversa al paso x */
 stepx = (float) dg.n_puntos / ((float) rect.right * extgr.escalax);

 if ((tipo != TIEMPO) && (dg.escala == 'L'))
  {
   extgr.max = (extgr.max <= 0) ? -100 : 20 * log10 (extgr.max);
   extgr.min = (extgr.min <= 0) ? -100 : 20 * log10 (extgr.min);
  }
 stepy = (float) rect.bottom * extgr.escalay / (extgr.max - extgr.min);

 /* Calculamos las coordenadas de pantalla reales teniendo en cuenta las
    posiciones de Scroll y el desplazamiento del origen de la gr fica
    respecto del origen del  rea de cliente */
 ch = *p;
 ch.x += GetScrollPos (hwnd, SB_HORZ) *
	 rect.right / GetWindowWord (hwnd, NDIVSX) - 5;
 ch.y += GetScrollPos (hwnd, SB_VERT) *
	 rect.bottom / GetWindowWord (hwnd, NDIVSY) - 5;

 /* Obtenemos el n§ de muestra para la coordenada horizontal p->x */
 muestra = (int)((float) ch.x * stepx);

 /* Obtenemos los valores de x e y relativos a la gr fica */
 if ((tipo == TIEMPO) || (dg.escala == 'D'))
   c[0] = dg.ini + (dg.fin - dg.ini) *
		   (float) ch.x / (rect.right * extgr.escalax);
 else
   c[0] = dg.ini + (dg.fin - dg.ini) / 9 *
		   (pow (10, (float) ch.x / (rect.right * extgr.escalax)) - 1);
 c[1] = extgr.min + (rect.bottom * extgr.escalay - ch.y) / stepy;

 /* Leemos el dato del fichero para esta muestra */
 hFichero = OpenFile (strcat (nom, ext[tipo - 1]), &of, OF_READ);
 if (tipo == TIEMPO)
  {
   _llseek (hFichero, sizeof (datos_grafica) + muestra * sizeof (double), 0);
   _lread  (hFichero, (LPSTR) &valor, sizeof (double));
  }
 else
  {
   _llseek (hFichero, sizeof (datos_grafica) + muestra * sizeof (complejo), 0);
   _lread  (hFichero, (LPSTR) &leido, sizeof (complejo));
   if (dg.escala == 'L')
     valor = 20 * log10 (polar (leido).mod);
   else
     valor = polar (leido).mod;
  }

 _lclose (hFichero);

 /* A partir de ‚ste, obtenemos la coord. y */
 p->y = max (0,
	     5 + rect.bottom * extgr.escalay - (valor - extgr.min) * stepy);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcVentCoords (HWND hwnd, WORD message,
					WORD wParam, LONG lParam)

/* Procedimiento para gesti¢n de la ventana de coordenadas */
{
 HDC   	     hdc;
 HFONT       hViejoFont;
 PAINTSTRUCT ps;
 TEXTMETRIC  tm;
 POINT 	     ptCurPos;
 RECT	     rect;

 switch (message)
 {
  case WM_PAINT:
  {
   /* Obtenemos la posici¢n actual del cursor */
   GetCursorPos (&ptCurPos);

   /* Imprimimos el string que contiene las coordenadas */
   hdc = BeginPaint (hwnd, &ps);

   hViejoFont = SelectObject (hdc, GetStockObject (SYSTEM_FIXED_FONT));

   GetTextMetrics (hdc, &tm);
   GetClientRect (hwnd, &rect);

   rect.top += (tm.tmHeight + tm.tmExternalLeading) / 2;
   DrawText (hdc, szCoords, -1, &rect, DT_CENTER);

   SelectObject (hdc, hViejoFont);

   EndPaint (hwnd, &ps);

   return (0);
  }
 }

 return (DefWindowProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

float norm (float n)

/* Normaliza 'n' a 1 cifra entera */
{
 if (n != 0)
   if (fabs (n) >= 10)
     while (fabs (n) >= 10)
     {
      n /= 10;
     }
   else
     while (fabs (n) < 1)
     {
      n *= 10;
     }

 return (n);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int trans_Fourier (HWND hwnd, int tipo)

/* Esta funci¢n realiza la transformada directa o inversa de Fourier, seg£n
   el valor de tipo, y la almacena en un fichero */
{
 OFSTRUCT     of;
 HCURSOR      hCursor;
 HANDLE       hMemtbl1, hMemtbl2;
 int          hFichero;
 complejo FAR *tabla1;
 complejo FAR *tabla2;
 complejo     term1, term2, e;
 modarg       valor;
 float        ini, fin;
 int          lei;
 unsigned     N, i, j, k, pp, n_pasos, mascara, espejo;
 char         nom[9], nomyext[13], escala;

 complejo FAR *m0 (unsigned fila, unsigned columna);
 complejo FAR *m1 (unsigned fila, unsigned columna);

 /* Obtenemos una copia del nombre de fichero */
 GetWindowText (hwnd, (LPSTR) nom, 9);
 *strchr (nom, ' ') = 0;

 /* Si la transformada ya existe, no se vuelve a calcular */
 strcpy (nomyext, nom);
 if (tipo == DIRECTA)
   strcat (nomyext, ".DFT");
 else
   strcat (nomyext, ".IFT");

 if (OpenFile (nomyext, &of, OF_EXIST) != -1)
   return (0);

 /* Cambiamos el cursor */
 hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

 e.Re = 0;               /* 'e' contiene el exponente complejo de e^ñjú2ã/m */

 /* Obtenemos el nombre y extensi¢n del fichero con la se¤al a transformar */
 strcpy (nomyext, nom);
 if (tipo == DIRECTA)
   strcat (nomyext, ".TMP");
 else
   strcat (nomyext, ".FRC");

 /* Abrimos el fichero para lectura */
 hFichero = OpenFile (nomyext, &of, OF_READ);

 /* Averiguamos el n£mero de muestras */
 _lread (hFichero, (LPSTR) &escala, sizeof (char));
 _lread (hFichero, (LPSTR) &ini,    sizeof (float));
 _lread (hFichero, (LPSTR) &fin,    sizeof (float));
 _lread (hFichero, (LPSTR) &N,      sizeof (unsigned));

 /* Si el fichero es de frecuencia, debe tener el formato de los ficheros
    generados por  el algoritmo */
 if ((tipo == INVERSA) && (escala != 'D'))
  {
   MessageBox (hwnd, "Imposible realizar la transformada inversa.\nLa se¤al no ha sido muestreada linealmente.",
	       "Osciloscopio Digital", MB_ICONSTOP | MB_OK);
   return (1);
  }

 /* Calculamos el n§ de pasos que har  el algoritmo */
 n_pasos = ceil (log(N) / log(2));     /* Notar que log2(x) = ln(x) / ln(2) */

 /* Redondeamos el n£mero de muestras a la potencia de 2 m s pr¢xima */
 N = pow (2, n_pasos);

 /* Reservamos espacio para dos tablas en memoria global */
 hMemtbl1 = GlobalAlloc (GMEM_MOVEABLE, N * sizeof (complejo));
 hMemtbl2 = GlobalAlloc (GMEM_MOVEABLE, N * sizeof (complejo));
 tabla1 = (complejo FAR *) GlobalLock (hMemtbl1);
 tabla2 = (complejo FAR *) GlobalLock (hMemtbl2);

 /* Leemos los datos del fichero a memoria, reorden ndolos */
 for (i = 0; i < N; ++i)
  {
   /* Obtenemos la posici¢n que debe ocupar (espejo bit a bit de 'i') */
   espejo = 1 << (n_pasos - 1);
   j = 0;
   for (k = 0, mascara = 1; k < n_pasos; ++k, mascara <<= 1)
    {
     if ((i & mascara) != 0)
       j |= espejo;
     espejo >>= 1;
    }

   /* Leemos los datos del fichero, seg£n el tipo de transformada */
   _lread (hFichero, (LPSTR) &tabla1[j].Re, sizeof (double));
   if (tipo == INVERSA)
     _lread (hFichero, (LPSTR) &tabla1[j].Im, sizeof (double));
   else
     tabla1[j].Im = 0;
  }

 /* Cerramos el fichero ya que ya no lo necesitamos */
 _lclose (hFichero);

 /* Partimos con 'tabla1' como tabla inicial */
 M1 = tabla1;

 /* El algoritmo consta de log2(N) pasos */
 for (paso = 1; paso <= n_pasos; ++paso)
  {
   /* Calculamos el n£mero 2^t que ser  muy utilizado */
   pp = 1 << paso;   /* Notar que 2^x = 1 desplazado x posiciones a la izq. */

   /* Cambiamos de tablas */
   M0 = M1;
   M1 = (M1 == tabla1) ? tabla2 : tabla1;

   /* En cada paso tratamos con una matriz de N / 2^p filas... */
   for (i = 0; i < N / pp; ++i)
    {
     /* ...y con 2^p columnas */
     for (k = 0; k < pp / 2; ++k)            /* Notar que 2^(p-1) = 2^p / 2 */
      {
       /* Calculamos el exponente de la exponencial */
       if (tipo == DIRECTA)
	 e.Im = -2 * pi / pp * k;
       else
	 e.Im = 2 * pi / pp * k;

       /* Calculamos los coeficientes, que son suma de dos t‚rminos */
       term1 = *m0(2*i, k);
       term2 = cmul (*m0(2*i + 1, k), cexp (e));

       *m1(i, k) = sum (term1, term2);
       *m1(i, k + pp / 2) = res (term1, term2);
      }
    }
  }

 /* Obtenemos una copia del nombre de fichero */
 strcpy (nomyext, nom);

 /* Le ponemos la extensi¢n correcta */
 if (tipo == DIRECTA)
   strcat (nomyext, ".DFT");
 else
  {
   strcat (nomyext, ".IFT");

   /* La tda. inversa viene dividida por N */
   for (i = 0; i < N; ++i)
    {
     M1[i].Re /= N;
     M1[i].Im /= N;
    }
  }

 /* Creamos y abrimos el fichero que contendr  el resultado */
 hFichero = OpenFile (nomyext, &of, OF_CREATE);

 /* Calculamos los par metros de la respuesta */
 escala = 'D';
 fin = N / (2 * fin);
 ini = 0;

 /* Los guardamos al principio del fichero */
 _lwrite (hFichero, (LPSTR) &escala, sizeof (char));
 _lwrite (hFichero, (LPSTR) &ini,    sizeof (float));
 _lwrite (hFichero, (LPSTR) &fin,    sizeof (float));

 /* Volcamos los resultados al fichero */
 _lwrite (hFichero, (LPSTR) &N, sizeof (unsigned));
 for (i = 0; i < N; ++i)
  {
   if (tipo == DIRECTA)
    {
     valor = polar (M1[i]);
     _lwrite (hFichero, (LPSTR) &valor.mod, sizeof (double));
    }
   else
     _lwrite (hFichero, (LPSTR) &M1[i], sizeof (complejo));
  }

 /* Finalmente cerramos el fichero */
 _lclose (hFichero);

 /* Desbloqueamos y liberamos la memoria global utilizada */
 GlobalUnlock (hMemtbl1);
 GlobalUnlock (hMemtbl2);
 GlobalFree   (hMemtbl1);
 GlobalFree   (hMemtbl2);

 /* Restauramos el cursor */
 SetCursor (hCursor);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo FAR *m0 (unsigned fila, unsigned columna)
/* Calcula la direcci¢n de un elemento en la tabla apuntada por 'M0' y en
   funci¢n del n§ de paso en el que est‚ la FFT */
{
 /* Puesto que la tabla apuntada por 'M0' es la antigua, habr  que procesarla
    con un n§ de cols. correspondiente al paso actual menos uno */
 return (M0 + (fila * (1 << (paso-1))) + columna);
}

complejo FAR *m1 (unsigned fila, unsigned columna)
/* Calcula la direcci¢n de un elemento en la tabla apuntada por 'M1' y en
   funci¢n del n§ de paso en el que est‚ la FFT */
{
 return (M1 + (fila * (1 << paso)) + columna);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
