#include "capturar.h"
#include "wincapt.h"

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern BOOL guardar_esquema (InfoVentana NEAR *);
extern BOOL crear_DCE       (InfoVentana NEAR *);

BOOL FAR PASCAL _export ProcDialogNueva     (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogAbrir     (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogGuardar   (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogAcerca    (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCaract    (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCarRLC    (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCarDiodo  (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCarTransB (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCarTransJ (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCarTransM (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCarGen    (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCarMedida (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogNudos     (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCuad      (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogEditFunc  (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogAddSym    (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogBuscSym   (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogListSym   (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogSuprBibl  (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogBloque    (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogPuerto    (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogTexto     (HWND, WORD, WORD, LONG);


/* Las siguientes funciones duplican las est ndar de C para el ANSI Ch. Set */
LPSTR lstrchr  (LPSTR str, char ch);
LPSTR lstrrchr (LPSTR str, char ch);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDefinici¢n de variables globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern HANDLE hInstGlobal;
extern short *bloques, *puertos, *textos,
	     *elementos, *nudos, *cables, *cuad,
	      tipo_ventana;
extern char   szAppName[], path[80], nom_fich[13],
	     *szExtFich[];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogNueva (HWND hDlg, WORD message,
					 WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo de Nueva */
{
 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos el tipo de ventana pasado como par metro */
   tipo_ventana = LOWORD(lParam);
   CheckRadioButton (hDlg, IDC_BOTON1, IDC_BOTON3,
		     IDC_BOTON1 + tipo_ventana - 1);

   return (FALSE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_BOTON1:
    case IDC_BOTON2:
    case IDC_BOTON3:
    {
     if (wParam == IDC_BOTON1)
       tipo_ventana = TV_BLOQUES;
     else
       if (wParam == IDC_BOTON2)
	 tipo_ventana = TV_ESQUEMA;
       else
	 tipo_ventana = TV_BIBLIO;

     CheckRadioButton (hDlg, IDC_BOTON1, IDC_BOTON3, wParam);

     return (TRUE);
    }

    case IDC_OK:
    case IDC_CANCELAR:
    {
     EndDialog (hDlg, wParam == IDC_OK);
     return (TRUE);
    }
   }

   return (FALSE);
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogAbrir (HWND hDlg, WORD message,
					 WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo de Abrir */
{
 static char EspecFich[84], NomFich[13];
 int         hFichero;
 OFSTRUCT    of;
 char        szTitulo[50] = "Abriendo un",
	    *szExtTitulo[] = { " Objeto", " Diagrama",
			       " Esquema", "a Biblioteca", " Simbolo" };

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Fijamos el t¡tulo de la ventana */
   SetWindowText (hDlg, lstrcat (szTitulo, szExtTitulo[tipo_ventana]));

   /* Fijamos el texto de la caja edit al filtro por defecto */
   strcpy  (NomFich, "*");
   lstrcat (NomFich, szExtFich[tipo_ventana]);
   SetDlgItemText (hDlg, IDC_EDIT, NomFich);

   /* Hacemos la lista de directorios y unidades */
   strcpy (EspecFich, path);
   strcat (EspecFich, NomFich);
   DlgDirList (hDlg, EspecFich, IDC_LISTDIR, IDC_DIRACT, 0xC010);

   /* Hacemos la lista de ficheros */
   SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_DIR, 0, (LONG) NomFich);

   /* Obtenemos el valor del path inicial */
   lstrcpy (EspecFich, path);

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
       /* Obtenemos la seleci¢n actual de la lista de ficheros */
       SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_GETTEXT,
			   SendDlgItemMessage (hDlg, IDC_LISTFICH,
					       LB_GETCURSEL, 0, 0),
			   (LONG) EspecFich);

       /* Fijamos el contenido de la caja edit */
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
       lstrcpy (NomFich, "*");
       lstrcat (NomFich, szExtFich[tipo_ventana]);

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

     /* Si no tiene extensi¢n, se la a¤adimos */
     if (!lstrchr (EspecFich, '.'))
       lstrcat (EspecFich, szExtFich[tipo_ventana]);

     /* Si es un nombre sin WILDCARDS.. */
     if ((!lstrchr (EspecFich, '*')) && (!lstrchr (EspecFich, '?')))
      {
       /* ..entonces, si el fichero existe... */
       if (OpenFile (EspecFich, &of, OF_EXIST) != -1)
	{
	 /* ...le quitamos la extensi¢n, si la tiene */
	 if (lstrrchr (EspecFich, '.') != NULL)
	   *lstrrchr (EspecFich, '.') = 0;

	 /* Si se ha especificado un camino absoluto.. */
	 if ((EspecFich[0] == '\\') ||
	     (EspecFich[1] == ':') && (EspecFich[2] == '\\'))
	  {
	   /* .. entonces tiene preferencia sobre el dir. actual */
	   lstrcpy (path, EspecFich);
	   *(lstrrchr (path, '\\') + 1) = 0;
	   lstrcpy (nom_fich, lstrrchr (EspecFich, '\\') + 1);
	  }

	 /* pero, si se especific¢ camino relativo.. */
	 else
	  {
	   /* ..obtenemos el directorio actual */
	   GetDlgItemText (hDlg, IDC_DIRACT, path, 80);

	   /* Le a¤adimos la parte de camino de la ventana edit */
	   lstrcat (path, EspecFich);
	   *(lstrrchr (path, '\\') + 1) = 0;

	   /* Tomamos el nombre de fichero */
	   if (lstrrchr (EspecFich, '\\') != NULL)
	     lstrcpy (nom_fich, lstrrchr (EspecFich, '\\') + 1);
	   else
	     lstrcpy (nom_fich, EspecFich);
	  }

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

BOOL FAR PASCAL _export ProcDialogAcerca (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo de Acerca de.. */
{
 HWND          hBitmapCtrl;
 HDC           hdc, hdcMem;
 HBITMAP       hBitmap, hViejoBitmap;
 PAINTSTRUCT   ps;
 char          szBuffer[10];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   sprintf (szBuffer, "%u", 0);
   SetDlgItemText (hDlg, IDC_TEXTBASE + 1, szBuffer);

   sprintf (szBuffer, "%u", 0);
   SetDlgItemText (hDlg, IDC_TEXTBASE + 3, szBuffer);

   sprintf (szBuffer, "%u", 0);
   SetDlgItemText (hDlg, IDC_TEXTBASE + 5, szBuffer);

   return (TRUE);
  }

  case WM_PAINT:
  {
   hBitmapCtrl = GetDlgItem (hDlg, IDC_BITMAP);

   /* Creamos un contexto de memoria de dispositivo compatible */
   hdc = CreateIC ("DISPLAY", NULL, NULL, NULL);
   hdcMem = CreateCompatibleDC (hdc);
   DeleteDC (hdc);

   /* Cargamos el Bitmap */
   hBitmap = LoadBitmap (hInstGlobal, szAppName);

   /* Seleccionamos el BitMap en el contexto de memoria */
   hViejoBitmap = SelectObject (hdcMem, hBitmap);

   /* Obtenemos el contexto del control */
   hdc = BeginPaint (hBitmapCtrl, &ps);

   /* Trasladamos el BitMap a la ventana */
   BitBlt (hdc,     ps.rcPaint.left, ps.rcPaint.top,
		    ps.rcPaint.right - ps.rcPaint.left + 1,
		    ps.rcPaint.bottom - ps.rcPaint.top + 1,
	   hdcMem,  ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);


   /* Liberamos el contexto del control */
   EndPaint (hBitmapCtrl, &ps);

   /* Seleccionamos el viejo bitmap en el contexto de memoria */
   SelectObject (hdcMem, hViejoBitmap);

   /* Destruimos el Bitmap */
   DeleteObject (hBitmap);

   /* Liberamos el contexto de memoria */
   DeleteDC (hdcMem);
  }

  case WM_COMMAND:
  {
   if (wParam == IDC_OK)
    {
     EndDialog (hDlg, TRUE);
     return (TRUE);
    }

   break;
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogGuardar (HWND hDlg, WORD message,
					   WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo de Guardar */
{
 static InfoVentana NEAR *npIV;
 OFSTRUCT of;
 char 	  nom[84], *p;
 BOOL     confirm;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos los datos de la ventana */
   npIV = (InfoVentana NEAR *) HIWORD(lParam);

   /* Prefijamos el texto del control Edit */
   lstrcpy (nom, nom_fich);
   SetDlgItemText (hDlg, IDC_EDIT, strcat (nom, szExtFich[tipo_ventana]));
   CheckDlgButton (hDlg, IDC_BOTON1, TRUE);
   SetFocus (GetDlgItem (hDlg, IDC_EDIT));

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_EDIT:
    {
     if (HIWORD(lParam) == EN_CHANGE)
      {
       /* Si no hay nada en la ventana edit, deshabilitamos OK */
       EnableWindow (GetDlgItem (hDlg, IDC_OK),
		     (BOOL) SendMessage (LOWORD (lParam),
					 WM_GETTEXTLENGTH, 0, 0));
       return (TRUE);
      }

     return (FALSE);
    }

    case IDC_OK:
    {
     /* Obtenemos el nombre de fichero de la caja Edit */
     GetDlgItemText (hDlg, IDC_EDIT, nom, 84);

     /* Si se ha seleccionado formato Esquema.. */
     if (IsDlgButtonChecked (hDlg, IDC_BOTON1))
      {
       /* ..le quitamos la extensi¢n al nombre de fichero, si la tiene */
       if ((p = (char *) lstrrchr (nom, '.')) != NULL)
	 *p = 0;

       /* Si el fichero de esquema ya existe.. */
       if (OpenFile (strcat (nom, szExtFich[tipo_ventana]),
		     &of, OF_EXIST) != -1)

	 /* ..pedimos confirmaci¢n de grabado */
	 confirm = MessageBox (hDlg, "El fichero especificado ya existe\n¿Desea sustituirlo por el esquema actual?",
			       "Guardando Fichero",
			       MB_ICONQUESTION | MB_YESNO);
       else
	 confirm = IDYES;

       if (confirm == IDYES)
	{
	 /* Cambiamos de nombre el fichero temporal de elementos */
	 if ((p = (char *) lstrrchr (nom,      '.')) != NULL)
	   *p = 0;
	 if ((p = (char *) lstrrchr (nom_fich, '.')) != NULL)
	   *p = 0;
	 rename (strcat (nom_fich, ".TM1"), strcat (nom, ".TM1"));

	 /* Cambiamos de nombre el fichero temporal de cuadripolos */
	 *lstrrchr (nom,      '.') = 0;
	 *lstrrchr (nom_fich, '.') = 0;
	 rename (strcat (nom_fich, ".TM2"), strcat (nom, ".TM2"));

	 /* Actualizamos el nombre */
	 *lstrrchr (nom, '.') = 0;
	 lstrcpy (nom_fich, nom);

	 /* Guardamos el esquema */
	 guardar_esquema (npIV);
	}
      }

     /* Si se ha seleccionado formato DCE.. */
     if (IsDlgButtonChecked (hDlg, IDC_BOTON2))
      {
       /* ..le quitamos la extensi¢n al nombre de fichero, si la tiene */
       if ((p = (char *) lstrrchr (nom, '.')) != NULL)
	 *p = 0;

       /* Si el fichero DCE ya existe.. */
       if (OpenFile (strcat (nom, ".FNT"), &of, OF_EXIST) != -1)

	 /* ..pedimos confirmaci¢n de grabado */
	 confirm = MessageBox (hDlg, "El fichero de Lenguaje D.C.E. ya existe\n¿Desea sustituirlo por el esquema actual?",
			       "Confirmar operación",
			       MB_ICONQUESTION | MB_YESNO);
       else
	 confirm = IDYES;

       if (confirm == IDYES)
	{
	 /* Creamos el fichero fuente en D.C.E. */
	 crear_DCE (npIV);
	}
      }

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
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogCaract (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo de caracter¡sticas de elemento */
{
 static char *tipos[]    = { "RESISTENCIA", "CONDENSADOR", "BOBINA",
			     "TRANS_BIP_P", "TRANS_BIP_N",
			     "TRANS_FET_P", "TRANS_FET_N",
			     "TRANS_MOS_P", "TRANS_MOS_N",
			     "DIODO", "DIODO_ZENER", "DIODO_VARIC",
			     "BATERIA", "GENV", "GENI",
			     "GENVDEPV", "GENVDEPI", "GENIDEPV", "GENIDEPI",
			     "VOLTIMETRO", "AMPERIMETRO" };
 static char  *szNames[] = { "CARRLC",
			     "CARTRANB",
			     "CARTRANJ",
			     "CARTRANM",
			     "CARDIODO",
			     "CARGEN",
			     "CARMEDIDA" };
 static HANDLE      hInstance;
 static HICON       hIcono;
 static elemento    datos_elem;
 static short       elem_act, elem[6];
 static InfoVentana NEAR *npIV;
 OFSTRUCT           of;
 PAINTSTRUCT        ps;
 RECT               rectW, rectD;
 POINT              pt, ptNuevaPos;
 FARPROC            lpfnProcDialogBox;
 HDC                hdc;
 HPEN               hPen, hViejoPen;
 elemento           elem_aux;
 int                hFichero;
 short              i;
 char               nom[84], szBuffer[12];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos el handle a la ocurrencia del m¢dulo */
   hInstance = GetWindowWord (GetParent (hDlg), GWW_HINSTANCE);

   /* Obtenemos los datos de la ventana */
   npIV = (InfoVentana NEAR *) HIWORD(lParam);

   /* Obtenemos el n£mero de elemento a partir de lParam */
   elem_act = (short) LOWORD(lParam);

   /* Obtenemos los datos del elemento */
   elementos = (short *) LocalLock (npIV->hmemElems);
   memcpy (elem, elementos + 6*elem_act, 6 * sizeof (short));
   LocalUnlock (npIV->hmemElems);

   strcpy (nom, npIV->path);
   strcat (nom, npIV->nom_fich);
   if ((hFichero = OpenFile (strcat (nom, ".TM1"), &of, OF_READ)) != -1)
    {
     _llseek (hFichero, elem_act * sizeof (datos_elem), 0);
     _lread  (hFichero, (LPSTR)&datos_elem, sizeof (datos_elem));
     _lclose (hFichero);
    }

   /* Obtenemos el icono correspondiente al elemento */
   strcpy (szBuffer, tipos[elem[0] - 1]);
   if ((elem[0] >= TRANSISTOR_BP) && (elem[0] <= DIODOV))
     szBuffer[5] = 0;
   else
    if ((elem[0] >= GENV) && (elem[0] <= GENIDEPI))
      szBuffer[3] = 0;
   hIcono = LoadIcon (hInstance, szBuffer);

   /* Inicializar control Edit */
   SetDlgItemText (hDlg, IDC_EDIT, datos_elem.nombre);

   /* Inicializar Combo-Box */
   for (i = 0; i < 21; ++i)
     SendDlgItemMessage (hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LONG)tipos[i]);

   SendDlgItemMessage (hDlg, IDC_COMBO1, CB_SETCURSEL, elem[0] - 1, 0);

   /* Inicializar botones de radio */
   CheckRadioButton (hDlg, IDC_HORIZ, IDC_VERT,
		     (elem[3] == 1) ? IDC_HORIZ : IDC_VERT);
   CheckDlgButton (hDlg, IDC_ESPEJO, elem[5]);

   /* Posicionamos la caja de di logo */
   GetWindowRect (hDlg, &rectD);
   ClientToScreen (GetParent (hDlg), (LPPOINT) &rectD.left);
   ClientToScreen (GetParent (hDlg), (LPPOINT) &rectD.right);

   pt.x = elem[1] - npIV->nHposScrl * SCRLDESPL;
   pt.y = elem[2] - npIV->nVposScrl * SCRLDESPL;
   ClientToScreen (GetParent (hDlg), &pt);

   ptNuevaPos.x = (pt.x > GetSystemMetrics (SM_CXSCREEN) -
			  40 - (rectD.right - rectD.left)) ?
				 max (0, pt.x - (rectD.right - rectD.left)) :
				 min (GetSystemMetrics (SM_CXSCREEN) -
				      (rectD.right - rectD.left), pt.x + 40);
   ptNuevaPos.y = (pt.y > GetSystemMetrics (SM_CYSCREEN) -
			  40 - (rectD.bottom - rectD.top)) ?
				 max (0, pt.y - (rectD.bottom - rectD.top)) :
				 min (GetSystemMetrics (SM_CYSCREEN) -
				      (rectD.bottom - rectD.top), pt.y + 40);

   MoveWindow (hDlg, ptNuevaPos.x, ptNuevaPos.y,
		     rectD.right - rectD.left + 1, rectD.bottom - rectD.top + 1,
	       TRUE);

   /* Inutilizamos las caracter¡sticas del MOSFET */
   if ((elem[0] == TRANSISTOR_MP) || (elem[0] == TRANSISTOR_MN))
     EnableWindow (GetDlgItem (hDlg, IDC_MAS), FALSE);

   return (TRUE);
  }

  case WM_PAINT:
  {
   hdc = BeginPaint (GetDlgItem (hDlg, IDC_ICONO), &ps);
   DrawIcon (hdc, 0, 0, hIcono);
   EndPaint (GetDlgItem (hDlg, IDC_ICONO), &ps);
   return (FALSE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
    {
     /* Actualizamos los datos del elemento */
     elementos = (short *) LocalLock (npIV->hmemElems);

     GetDlgItemText (hDlg, IDC_COMBO1, szBuffer, 12);
     for (i = 0; strcmp (szBuffer, tipos[i]) != 0; ++i);
     e(elem_act,0) = i + 1;

     e(elem_act,3) = IsDlgButtonChecked (hDlg, IDC_HORIZ);
     e(elem_act,4) = IsDlgButtonChecked (hDlg, IDC_VERT);
     e(elem_act,5) = IsDlgButtonChecked (hDlg, IDC_ESPEJO);

     LocalUnlock (npIV->hmemElems);

     /* Obtenemos el nombre de la caja de di logo */
     if (GetDlgItemText (hDlg, IDC_EDIT, szBuffer, 5) == 0)
       strcpy (szBuffer, "     ");
     strcpy (datos_elem.nombre, szBuffer);

     /* Actualizamos el nombre y el resto de datos en el fichero */
     strcpy (nom, nom_fich);
     hFichero = OpenFile (strcat (nom, ".TM1"), &of, OF_WRITE);
     _llseek (hFichero, elem_act * sizeof (datos_elem), 0);
     _lwrite (hFichero, (LPSTR)&datos_elem, sizeof (datos_elem));
     _lclose (hFichero);

     EndDialog (hDlg, TRUE);
     return (TRUE);
    }

    case IDC_CANCELAR:
    {
     EndDialog (hDlg, FALSE);
     return (TRUE);
    }

    case IDC_MAS:
    {
     /* Obtenemos la dir. del proc. para la Caja de Di logo Caract. Espec¡f. */
     switch (elem[0])
     {
      case RESISTENCIA:
      case CONDENSADOR:
      case BOBINA:
	lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogCarRLC,
					      hInstance);
	strcpy (szBuffer, szNames[0]);
	break;

      case TRANSISTOR_BP:
      case TRANSISTOR_BN:
	lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogCarTransB,
					      hInstance);
	strcpy (szBuffer, szNames[1]);
	break;

      case TRANSISTOR_JP:
      case TRANSISTOR_JN:
	lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogCarTransJ,
					      hInstance);
	strcpy (szBuffer, szNames[2]);
	break;


      case TRANSISTOR_MP:
      case TRANSISTOR_MN:
	lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogCarTransM,
					      hInstance);
	strcpy (szBuffer, szNames[3]);
	break;

      case DIODO:
      case DIODOZ:
      case DIODOV:
	lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogCarDiodo,
					      hInstance);
	strcpy (szBuffer, szNames[4]);
	break;

      case BATERIA:
      case GENV:
      case GENI:
      case GENVDEPV:
      case GENVDEPI:
      case GENIDEPV:
      case GENIDEPI:
	lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogCarGen,
					      hInstance);
	strcpy (szBuffer, szNames[5]);
	break;

      case VOLTIMETRO:
      case AMPERIMETRO:
	lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogCarMedida,
					      hInstance);
	strcpy (szBuffer, szNames[6]);
	break;
     }

     /* Obtenemos una copia de las caracter¡sticas del elemento */
     elem_aux = datos_elem;

     /* Si la caja de di logo termin¢ con OK, actualizamos las caract. */
     if (DialogBoxParam (hInstance, szBuffer, hDlg,
			 lpfnProcDialogBox, (LONG)&elem_aux))
      {
       datos_elem = elem_aux;

       /* Actualizamos el contenido de la Combo-Box */
       switch (datos_elem.tipo[0])
       {
	case 'P':
	  i = TRANSISTOR_BP;
	  break;

	case 'N':
	  i = TRANSISTOR_BN;
	  break;

	case 'J':
	  i = TRANSISTOR_JP;
	  break;

	case 'K':
	  i = TRANSISTOR_JN;
	  break;

	case 'M':
	  i = TRANSISTOR_MP;
	  break;

	case 'O':
	  i = TRANSISTOR_MN;
	  break;
       }

      SendDlgItemMessage (hDlg, IDC_EDIT2, CB_SETCURSEL, i - 1, 0);
     }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     return (TRUE);
    }

    case IDC_COMBO1:
    {
     /* Hay que cambiar el valor en elem[0] */
     GetDlgItemText (hDlg, IDC_COMBO1, szBuffer, 12);
     for (i = 0; strcmp (szBuffer, tipos[i]) != 0; ++i);
     e(elem_act,0) = i + 1;

     break;
    }
   }
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogCarRLC (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)
{
 static elemento *datos_elem;
 int    dec, sgn;
 char   szBuffer[30];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   datos_elem = (elemento *)lParam;

   sprintf (szBuffer, "%g", datos_elem->valor);
   SetDlgItemText (hDlg, IDC_EDITBASE, szBuffer);

   sprintf (szBuffer, "%g", datos_elem->CT);
   SetDlgItemText (hDlg, IDC_EDITBASE + 1, szBuffer);

   if (datos_elem->tipo[0] == 'R')
     EnableWindow (GetDlgItem (hDlg, IDC_EDITBASE + 2), FALSE);
   else
    {
     sprintf (szBuffer, "%g", datos_elem->caract.cond_inic);
     SetDlgItemText (hDlg, IDC_EDITBASE + 2, szBuffer);
    }
   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
    {
     GetDlgItemText (hDlg, IDC_EDITBASE, szBuffer, 30);
     datos_elem->valor = atof (szBuffer);

     GetDlgItemText (hDlg, IDC_EDITBASE + 1, szBuffer, 30);
     datos_elem->CT = atof (szBuffer);

     GetDlgItemText (hDlg, IDC_EDITBASE + 2, szBuffer, 30);
     datos_elem->caract.cond_inic = atof (szBuffer);

     EndDialog (hDlg, TRUE);
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

BOOL FAR PASCAL _export ProcDialogCarTransB (HWND hDlg, WORD message,
					     WORD wParam, LONG lParam)
{
 static elemento *datos_elem;
 int    i;
 char   szBuffer[30];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   datos_elem = (elemento *)lParam;

   sprintf (szBuffer, "%g", datos_elem->valor);
   SetDlgItemText (hDlg, IDC_EDITBASE, szBuffer);

   for (i = 0; i < 20; ++i)
    {
     sprintf (szBuffer, "%g", datos_elem->f[i]);
     SetDlgItemText (hDlg, IDC_EDITBASE + i + 1, szBuffer);
    }

   CheckRadioButton (hDlg, IDC_BOTON1, IDC_BOTON2,
		     (datos_elem->tipo[0] == 'P') ? IDC_BOTON1 : IDC_BOTON2);
   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
    {
     GetDlgItemText (hDlg, IDC_EDITBASE, szBuffer, 30);
     datos_elem->valor = atof (szBuffer);

     for (i = 0; i < 20; ++i)
      {
       GetDlgItemText (hDlg, IDC_EDITBASE + i + 1, szBuffer, 30);
       datos_elem->f[i] = atof (szBuffer);
      }

     datos_elem->tipo[0] = (IsDlgButtonChecked (hDlg, IDC_BOTON1)) ? 'P' : 'N';

     EndDialog (hDlg, TRUE);
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

BOOL FAR PASCAL _export ProcDialogCarTransJ (HWND hDlg, WORD message,
					     WORD wParam, LONG lParam)
{
 static elemento *datos_elem;
 int    i;
 char   szBuffer[30];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   datos_elem = (elemento *)lParam;

   sprintf (szBuffer, "%g", datos_elem->valor);
   SetDlgItemText (hDlg, IDC_EDITBASE, szBuffer);

   for (i = 0; i < 15; ++i)
    {
     sprintf (szBuffer, "%g", datos_elem->f[i]);
     SetDlgItemText (hDlg, IDC_EDITBASE + i + 1, szBuffer);
    }

   CheckRadioButton (hDlg, IDC_BOTON1, IDC_BOTON2,
		     (datos_elem->tipo[0] == 'J') ? IDC_BOTON1 : IDC_BOTON2);
   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
    {
     GetDlgItemText (hDlg, IDC_EDITBASE, szBuffer, 30);
     datos_elem->valor = atof (szBuffer);

     for (i = 0; i < 15; ++i)
      {
       GetDlgItemText (hDlg, IDC_EDITBASE + i + 1, szBuffer, 30);
       datos_elem->f[i] = atof (szBuffer);
      }

     datos_elem->tipo[0] = (IsDlgButtonChecked (hDlg, IDC_BOTON1)) ? 'J' : 'K';

     EndDialog (hDlg, TRUE);
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

BOOL FAR PASCAL _export ProcDialogCarTransM (HWND hDlg, WORD message,
					     WORD wParam, LONG lParam)
{
 switch (message)
 {
  case WM_COMMAND:
  {
   switch (wParam)
   {
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

BOOL FAR PASCAL _export ProcDialogCarDiodo (HWND hDlg, WORD message,
					    WORD wParam, LONG lParam)
{
 static elemento *datos_elem;
 int    i;
 char   szBuffer[30];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   datos_elem = (elemento *)lParam;

   sprintf (szBuffer, "%g", datos_elem->valor);
   SetDlgItemText (hDlg, IDC_EDITBASE, szBuffer);

   for (i = 0; i < 9; ++i)
    {
     sprintf (szBuffer, "%g", datos_elem->f[i]);
     SetDlgItemText (hDlg, IDC_EDITBASE + i + 1, szBuffer);
    }

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
    {
     GetDlgItemText (hDlg, IDC_EDITBASE, szBuffer, 30);
     datos_elem->valor = atof (szBuffer);

     for (i = 0; i < 9; ++i)
      {
       GetDlgItemText (hDlg, IDC_EDITBASE + i + 1, szBuffer, 30);
       datos_elem->f[i] = atof (szBuffer);
      }

     EndDialog (hDlg, TRUE);
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

BOOL FAR PASCAL _export ProcDialogCarGen (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)
{
 static char     *signals[] = { "CONTINUA", "PULSO", "RAMPA", "PARABOLA",
				"SENOIDAL", "POLINOM" };
 static char     *formas[]  = { "CONST.", "POLINOM" };
 static elemento *datos_elem;
 static BOOL     lista = FALSE;
 short  i;
 char	szBuffer[30], *p;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos los datos del elemento */
   datos_elem = (elemento *)lParam;

   /* Fijamos su valor */
   sprintf (szBuffer, "%g", datos_elem->valor);
   SetDlgItemText (hDlg, IDC_EDITBASE, szBuffer);

   /* Si es un generador independiente.. */
   if ((datos_elem->tipo[0] == 'V') || (datos_elem->tipo[0] == 'I'))
    {
     /* ..activamos los botones correspondientes */
     CheckDlgButton (hDlg, IDC_BOTON1, TRUE);

     /* Invalidamos los controles espec¡ficos de gen. dep. */
     EnableWindow (GetDlgItem (hDlg, IDC_BOTON3), FALSE);
     EnableWindow (GetDlgItem (hDlg, IDC_BOTON4), FALSE);
     EnableWindow (GetDlgItem (hDlg, IDC_EDITBASE + 5), FALSE);

     /* Fijamos el resto de par metros */
     sprintf (szBuffer, "%g", datos_elem->caract.frec);
     SetDlgItemText (hDlg, IDC_EDITBASE + 1, szBuffer);

     sprintf (szBuffer, "%g", datos_elem->Tact);
     SetDlgItemText (hDlg, IDC_EDITBASE + 2, szBuffer);

     sprintf (szBuffer, "%g", datos_elem->fase);
     SetDlgItemText (hDlg, IDC_EDITBASE + 3, szBuffer);

     sprintf (szBuffer, "%g", datos_elem->CT);
     SetDlgItemText (hDlg, IDC_EDITBASE + 4, szBuffer);

     /* Inicializar Combo-Box */
     for (i = 0; i < 6; ++i)
       SendDlgItemMessage (hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LONG)signals[i]);

     if (datos_elem->signal == 'C')
       i = 0;
     else
       if (datos_elem->signal == 'P')
	 i = 1;
       else
	 if (datos_elem->signal == 'R')
	   i = 2;
	 else
	   if (datos_elem->signal == 'T')
	     i = 3;
	   else
	     if (datos_elem->signal == 'S')
	       i = 4;
	     else
	       if (datos_elem->signal == 'D')
		 i = 5;

     SendDlgItemMessage (hDlg, IDC_COMBO1, CB_SETCURSEL, i, 0);
    }

   /* pero, si es dependiente.. */
   else
    {
     /* ..activamos los botones correspondientes */
     CheckDlgButton (hDlg, IDC_BOTON2, TRUE);

     if ((datos_elem->tipo[0] == 'A') || (datos_elem->tipo[0] == 'D'))
      {
       CheckDlgButton (hDlg, IDC_BOTON3, TRUE);
       sprintf (szBuffer, "%u, %u", datos_elem->caract.nodos[0],
				    datos_elem->caract.nodos[1]);
      }
     else
      {
       CheckDlgButton (hDlg, IDC_BOTON4, TRUE);
       strcpy (szBuffer, datos_elem->caract.elem);
      }

     SetDlgItemText (hDlg, IDC_EDITBASE + 5, szBuffer);

     /* Invalidamos los controles espec¡ficos de gen. indep. */
     for (i = 1; i <= 4; ++i)
       EnableWindow (GetDlgItem (hDlg, IDC_EDITBASE + i), FALSE);

     /* Inicializar Combo-Box */
     for (i = 0; i < 2; ++i)
       SendDlgItemMessage (hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LONG)formas[i]);

     if (datos_elem->signal == 'C')
       i = 0;
     else
       if (datos_elem->signal == 'D')
	 i = 1;

     SendDlgItemMessage (hDlg, IDC_COMBO1, CB_SETCURSEL, i, 0);
    }

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_BOTON1:
    case IDC_BOTON2:
    {
     CheckRadioButton (hDlg, IDC_BOTON1, IDC_BOTON2, wParam);
     return (TRUE);
    }

    case IDC_BOTON3:
    case IDC_BOTON4:
    {
     CheckRadioButton (hDlg, IDC_BOTON3, IDC_BOTON4, wParam);
     return (TRUE);
    }

    case IDC_OK:
    {
     /* Recoger todos los par metros */
     GetDlgItemText (hDlg, IDC_EDITBASE, szBuffer, 29);
     datos_elem->valor = atof (szBuffer);

     GetDlgItemText (hDlg, IDC_EDITBASE + 1, szBuffer, 29);
     datos_elem->caract.frec = atof (szBuffer);

     GetDlgItemText (hDlg, IDC_EDITBASE + 2, szBuffer, 29);
     datos_elem->Tact = atof (szBuffer);

     GetDlgItemText (hDlg, IDC_EDITBASE + 3, szBuffer, 29);
     datos_elem->fase = atof (szBuffer);

     GetDlgItemText (hDlg, IDC_EDITBASE + 4, szBuffer, 29);
     datos_elem->CT = atof (szBuffer);

     /*	Si el generador es independiente.. */
     if (IsDlgButtonChecked (hDlg, IDC_BOTON1))
      {
       /* ..obtenemos el tipo de se¤al generada */
       GetDlgItemText (hDlg, IDC_COMBO1, szBuffer, 8);
       if (strcmp (szBuffer, signals[0]) == 0)
	 datos_elem->signal = 'C';
       else
	 if (strcmp (szBuffer, signals[1]) == 0)
	   datos_elem->signal = 'P';
	 else
	   if (strcmp (szBuffer, signals[2]) == 0)
	     datos_elem->signal = 'R';
	   else
	     if (strcmp (szBuffer, signals[3]) == 0)
	       datos_elem->signal = 'T';
	     else
	       if (strcmp (szBuffer, signals[4]) == 0)
		 datos_elem->signal = 'S';
	       else
		 if (strcmp (szBuffer, signals[5]) == 0)
		   datos_elem->signal = 'D';
      }
     else
      {
       /* Obtenemos la funci¢n del generador */
       GetDlgItemText (hDlg, IDC_COMBO1, szBuffer, 8);
       if (strcmp (szBuffer, formas[0]) == 0)
	 datos_elem->signal = 'C';
       else
	 datos_elem->signal = 'D';

       /* Obtenemos el contenido de 'medida' */
       GetDlgItemText (hDlg, IDC_EDITBASE + 5, szBuffer, 29);

       /* Si en 'medida' se ha especificado un nombre de elemento.. */
       if (lstrchr (szBuffer, ',') == NULL)
	{
	 /* ..entonces, si el generador depende de una corriente.. */
	 if ((datos_elem->tipo[0] == 'B') || (datos_elem->tipo[0] == 'E'))
	  {
	   /* ..tomamos dicho nombre en los datos del generador */
	   strcpy (datos_elem->caract.elem, szBuffer);
	  }
	 else
	  {
	   /* pero, si no es as¡, pita y no finaliza */
	   MessageBeep (0);
	   return (TRUE);
	  }
	}

       /* pero, si se especific¢ un par de nudos separados por una coma.. */
       else
	{
	 /* ..entonces, si el generador depende de una tensi¢n.. */
	 if ((datos_elem->tipo[0] == 'A') || (datos_elem->tipo[0] == 'D'))
	  {
	   /* ..tomamos dichos nudos en los datos del generador */
	   if ((p = strtok (szBuffer, ",")) != NULL)
	     datos_elem->caract.nodos[0] = atoi (p);

	   if ((p = strtok (NULL, ",")) != NULL)
	     datos_elem->caract.nodos[1] = atoi (p);
	  }
	 else
	  {
	   /* pero, si no es as¡, pita y no finaliza */
	   MessageBeep (0);
	   return (TRUE);
	  }

	}
      }

     EndDialog (hDlg, TRUE);
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

BOOL FAR PASCAL _export ProcDialogCarMedida (HWND hDlg, WORD message,
					     WORD wParam, LONG lParam)
{
 static char     *dominios[] = { "POLARIZACION", "TIEMPO", "FRECUENCIA",
				 "FOURIER", "SENSIBILIDAD" };
 static elemento *datos_elem;
 short  i;
 char   szBuffer[20];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos los datos del elemento */
   datos_elem = (elemento *)lParam;

   /* Inicializamos los puntos inicial y final, y el n£mero de puntos */
   if ((datos_elem->tipo[1] != 'P') && (datos_elem->tipo[1] != 'S'))
    {
     sprintf (szBuffer, "%g", datos_elem->f[0]);
     SetDlgItemText (hDlg, IDC_EDITBASE, szBuffer);

     sprintf (szBuffer, "%g", datos_elem->f[1]);
     SetDlgItemText (hDlg, IDC_EDITBASE + 1, szBuffer);

     sprintf (szBuffer, "%.0f", datos_elem->f[2]);
     SetDlgItemText (hDlg, IDC_EDITBASE + 2, szBuffer);
    }
   else
     for (i = 0; i <= 2; ++i)
       EnableWindow (GetDlgItem (hDlg, IDC_EDITBASE + i), FALSE);

   /* Inicializamos la temperatura */
   sprintf (szBuffer, "%g", datos_elem->CT);
   SetDlgItemText (hDlg, IDC_EDITBASE + 3, szBuffer);

   /* Inicializamos la escala */
   if ((datos_elem->tipo[1] == 'F') || (datos_elem->tipo[1] == 'R'))
    {
     if (datos_elem->signal == 'D')
       CheckDlgButton (hDlg, IDC_BOTON1, TRUE);
     else
       CheckDlgButton (hDlg, IDC_BOTON2, TRUE);
    }
   else
    {
     EnableWindow (GetDlgItem (hDlg, IDC_BOTON1), FALSE);
     EnableWindow (GetDlgItem (hDlg, IDC_BOTON2), FALSE);
    }

   /* Inicializamos la combo-box del dominio */
   for (i = 0; i < 5; ++i)
     SendDlgItemMessage (hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LONG)dominios[i]);

   if (datos_elem->tipo[1] == 'P')
     i = 0;
   else
     if (datos_elem->tipo[1] == 'T')
       i = 1;
     else
       if (datos_elem->tipo[1] == 'F')
	 i = 2;
       else
	 if (datos_elem->tipo[1] == 'R')
	   i = 3;
	 else
	   if (datos_elem->tipo[1] == 'S')
	     i = 4;

   SendDlgItemMessage (hDlg, IDC_COMBO1, CB_SETCURSEL, i, 0);

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_BOTON1:
    case IDC_BOTON2:
    {
     CheckRadioButton (hDlg, IDC_BOTON1, IDC_BOTON2, wParam);
     return (TRUE);
    }

    case IDC_OK:
    {
     /* Obtenemos el dominio */
     GetDlgItemText (hDlg, IDC_COMBO1, szBuffer, 12);
     if (strcmp (szBuffer, dominios[0]) == 0)
       datos_elem->tipo[1] = 'P';
     else
       if (strcmp (szBuffer, dominios[1]) == 0)
	 datos_elem->tipo[1] = 'T';
       else
	 if (strcmp (szBuffer, dominios[2]) == 0)
	   datos_elem->tipo[1] = 'F';
	 else
	   if (strcmp (szBuffer, dominios[3]) == 0)
	     datos_elem->tipo[1] = 'R';
	   else
	     if (strcmp (szBuffer, dominios[4]) == 0)
	       datos_elem->tipo[1] = 'S';

     /* Recoger todos los par metros */
     if ((datos_elem->tipo[1] != 'P') && (datos_elem->tipo[1] != 'S'))
      {
       GetDlgItemText (hDlg, IDC_EDITBASE, szBuffer, 19);
       datos_elem->f[0] = atof (szBuffer);

       GetDlgItemText (hDlg, IDC_EDITBASE + 1, szBuffer, 19);
       datos_elem->f[1] = atof (szBuffer);

       GetDlgItemText (hDlg, IDC_EDITBASE + 2, szBuffer, 19);
       datos_elem->f[2] = max (1, atof (szBuffer));
      }

     GetDlgItemText (hDlg, IDC_EDITBASE + 3, szBuffer, 19);
     datos_elem->CT = atof (szBuffer);

     /* Obtenemos la escala */
     if ((datos_elem->tipo[1] == 'F') || (datos_elem->tipo[1] == 'R'))
       if (IsDlgButtonChecked (hDlg, IDC_BOTON1))
	 datos_elem->signal = 'D';
       else
	 datos_elem->signal = 'L';

     EndDialog (hDlg, TRUE);
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

BOOL FAR PASCAL _export ProcDialogNudos (HWND hDlg, WORD message,
					 WORD wParam, LONG lParam)
{
 static InfoVentana NEAR *npIV;
 static short 		  nudo_act, nudo[3];
 BOOL edit_error;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos los datos de la ventana */
   npIV = (InfoVentana NEAR *) HIWORD(lParam);

   /* Obtenemos el n£mero de nudo a partir de lParam */
   nudo_act = (short) LOWORD(lParam);

   /* Obtenemos los datos del nudo */
   nudos = (short *) LocalLock (npIV->hmemNudos);
   memcpy (nudo, nudos + 3*nudo_act, 3 * sizeof (short));
   LocalUnlock (npIV->hmemNudos);

   /* Inicializamos los campos de la Caja de Di logo */
   SetDlgItemInt (hDlg, IDC_EDITBASE, nudo[2], FALSE);

   SetScrollRange (GetDlgItem (hDlg, IDC_SCROLL), SB_CTL, 0, MAX_NUDOS, FALSE);
   SetScrollPos (GetDlgItem (hDlg, IDC_SCROLL), SB_CTL, nudo[2], TRUE);

   SetDlgItemInt (hDlg, IDC_EDITBASE + 4, nudo[0], FALSE);
   SetDlgItemInt (hDlg, IDC_EDITBASE + 5, nudo[1], FALSE);

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_SCROLL:
    {

     return (TRUE);
    }

    case IDC_OK:
    {
     /* Actualizamos los datos del nudo */
     if ((nudo[2] =
	  GetDlgItemInt (hDlg, IDC_EDITBASE, &edit_error, FALSE)) < MAX_NUDOS)
      {
       nudos = (short *) LocalLock (npIV->hmemNudos);
       memcpy (nudos + 3*nudo_act, nudo, 3 * sizeof (short));
       LocalUnlock (npIV->hmemNudos);

       EndDialog (hDlg, TRUE);
       return (TRUE);
      }
     else
       MessageBeep (0);

     return (FALSE);
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

BOOL FAR PASCAL _export ProcDialogCuad (HWND hDlg, WORD message,
					WORD wParam, LONG lParam)
{
 static InfoVentana NEAR *npIV;
 static HANDLE     	  hInstance;
 static cuadripolo 	  datos_cuad;
 static short      	  cuad_act, cuadripolo[5];
 static char       	  szNameEditFunc[] = "EDITFUNC";
 OFSTRUCT    of;
 PAINTSTRUCT ps;
 RECT        rectW, rectD;
 POINT       pt, ptNuevaPos;
 FARPROC     lpfnProcDialogBox;
 HDC         hdc;
 HPEN        hPen, hViejoPen;
 func_trans  cuad_aux;
 int         hFichero;
 short       i;
 char        nom[84], szBuffer[12],
	    *txtbtn[3][4] = { "Ve/Ie", "Vs/Ie", "Ve/Is", "Vs/Is",
			      "Ie/Ve", "Ie/Vs", "Is/Ve", "Is/Vs",
			      "Ve/Ie", "Ve/Vs", "Is/Ie", "Is/Vs" };

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos el handle a la ocurrencia del m¢dulo */
   hInstance = GetWindowWord (GetParent (hDlg), GWW_HINSTANCE);

   /* Obtenemos los datos de la ventana */
   npIV = (InfoVentana NEAR *) HIWORD(lParam);

   /* Obtenemos el n£mero de cuadripolo a partir de lParam */
   cuad_act = (short) LOWORD(lParam);

   /* Obtenemos los datos del cuadripolo */
   cuad = (short *) LocalLock (npIV->hmemCuad);
   memcpy (cuadripolo, cuad + 5*cuad_act, 5 * sizeof (short));
   LocalUnlock (npIV->hmemCuad);

   strcpy (nom, npIV->path);
   strcat (nom, npIV->nom_fich);
   if ((hFichero = OpenFile (strcat (nom, szExtFich[TMPCUADS]), &of,
			     OF_READ)) != -1)
    {
     _llseek (hFichero, cuad_act * sizeof (datos_cuad), 0);
     _lread  (hFichero, (LPSTR)&datos_cuad, sizeof (datos_cuad));
     _lclose (hFichero);
    }

   /* Inicializar control Edit */
   SetDlgItemText (hDlg, IDC_EDIT, datos_cuad.nombre);

   /* Inicializar botones de radio */
   CheckRadioButton (hDlg, IDC_BOTON1, IDC_BOTON3,
			   IDC_BOTON1 + datos_cuad.tipo_parms);

   /* Posicionamos la caja de di logo */
   GetWindowRect (hDlg, &rectD);
   ClientToScreen (GetParent (hDlg), (LPPOINT) &rectD.left);
   ClientToScreen (GetParent (hDlg), (LPPOINT) &rectD.right);

   pt.x = cuadripolo[0] - npIV->nHposScrl * SCRLDESPL;
   pt.y = cuadripolo[1] - npIV->nVposScrl * SCRLDESPL;
   ClientToScreen (GetParent (hDlg), &pt);

   ptNuevaPos.x = (pt.x > GetSystemMetrics (SM_CXSCREEN) -
			  40 - (rectD.right - rectD.left)) ?
				 max (0, pt.x - (rectD.right - rectD.left)) :
				 min (GetSystemMetrics (SM_CXSCREEN) -
				      (rectD.right - rectD.left), pt.x + 40);
   ptNuevaPos.y = (pt.y > GetSystemMetrics (SM_CYSCREEN) -
			  40 - (rectD.bottom - rectD.top)) ?
				 max (0, pt.y - (rectD.bottom - rectD.top)) :
				 min (GetSystemMetrics (SM_CYSCREEN) -
				      (rectD.bottom - rectD.top), pt.y + 40);

   MoveWindow (hDlg, ptNuevaPos.x, ptNuevaPos.y,
		     rectD.right - rectD.left + 1, rectD.bottom - rectD.top + 1,
	       TRUE);

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
    {
     /* Obtenemos el nombre de la caja de di logo */
     if (GetDlgItemText (hDlg, IDC_EDIT, szBuffer, 11) == 0)
       strcpy (szBuffer, "          ");
     strcpy (datos_cuad.nombre, szBuffer);

     /* Actualizamos el nombre y el resto de datos en el fichero */
     strcpy (nom, npIV->path);
     strcat (nom, npIV->nom_fich);
     hFichero = OpenFile (strcat (nom, szExtFich[TMPCUADS]), &of, OF_WRITE);
     _llseek (hFichero, cuad_act * sizeof (datos_cuad), 0);
     _lwrite (hFichero, (LPSTR)&datos_cuad, sizeof (datos_cuad));
     _lclose (hFichero);

     EndDialog (hDlg, TRUE);
     return (TRUE);
    }

    case IDC_CANCELAR:
    {
     EndDialog (hDlg, FALSE);
     return (TRUE);
    }

    case IDC_Ti:
    case IDC_Tr:
    case IDC_Tf:
    case IDC_To:
    {
     /* Obtenemos la dir. del proc. para la Caja de Di logo Editor de Funciones */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogEditFunc,
					   hInstance);

     /* Obtenemos una copia del parametro seleccionado del cuadripolo */
     if (wParam == IDC_Ti)
       cuad_aux = datos_cuad.Ti;
     else
       if (wParam == IDC_Tr)
	 cuad_aux = datos_cuad.Tr;
       else
	 if (wParam == IDC_Tf)
	   cuad_aux = datos_cuad.Tf;
	 else
	   cuad_aux = datos_cuad.To;

     /* Si la caja de di logo termin¢ con OK, actualizamos el parametro */
     if (DialogBoxParam (hInstance, szNameEditFunc, hDlg,
			 lpfnProcDialogBox, (LONG)&cuad_aux))
      {
       if (wParam == IDC_Ti)
	 datos_cuad.Ti = cuad_aux;
       else
	 if (wParam == IDC_Tr)
	   datos_cuad.Tr = cuad_aux;
	 else
	   if (wParam == IDC_Tf)
	     datos_cuad.Tf = cuad_aux;
	   else
	     datos_cuad.To = cuad_aux;
      }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     return (TRUE);
    }

    case IDC_BOTON1:
    case IDC_BOTON2:
    case IDC_BOTON3:
    {
     datos_cuad.tipo_parms = wParam - IDC_BOTON1;
     CheckRadioButton (hDlg, IDC_BOTON1, IDC_BOTON3, wParam);

     for (i = 0; i < 4; ++i)
      {
       strcpy (nom, txtbtn[wParam - IDC_BOTON1][i]);
       SetDlgItemText (hDlg, IDC_Ti + i, strcat (nom, " >>"));
      }

     return (TRUE);
    }
   }
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogEditFunc (HWND hDlg, WORD message,
					    WORD wParam, LONG lParam)
{
 static func_trans *T;
 int    i;
 char   szBuffer[30];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos un puntero a la funcion de transferencia */
   T = (func_trans *)lParam;

   /* Inicializamos las Edit boxes */
   for (i = 0; i < 10; ++i)
    {
     if (i <= T->num.grado)
       sprintf (szBuffer, "%g", T->num.coef[i]);
     else
       strcpy (szBuffer, "0");

     SetDlgItemText (hDlg, IDC_EDITBASE + i, szBuffer);

     if (i <= T->den.grado)
       sprintf (szBuffer, "%g", T->den.coef[i]);
     else
       strcpy (szBuffer, "0");

     SetDlgItemText (hDlg, IDC_EDITBASE + 10 + i, szBuffer);
    }

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
    {
     for (i = 0; i < 10; ++i)
      {
       GetDlgItemText (hDlg, IDC_EDITBASE + i , szBuffer, 30);
       if ((T->num.coef[i] = atof (szBuffer)) != 0)
	 T->num.grado = i;

       GetDlgItemText (hDlg, IDC_EDITBASE + 10 + i , szBuffer, 30);
       if ((T->den.coef[i] = atof (szBuffer)) != 0)
	 T->den.grado = i;
      }

     EndDialog (hDlg, TRUE);
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

BOOL FAR PASCAL _export ProcDialogSuprBibl (HWND hDlg, WORD message,
					   WORD wParam, LONG lParam)
{
 OFSTRUCT of;
 char     nombre[13],
	  *p;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Inicializamos la lista de Bibliotecas */
   DlgDirList (hDlg, "*.BBL", IDC_LISTFICH, 0, 0);

   /* Si no hay Bibliotecas, retornamos */
   if (SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_GETCOUNT, 0, 0L) == 0)
    {
     MessageBox (hDlg, "No existen Bibliotecas", "Borrando Biblioteca",
		 MB_ICONEXCLAMATION | MB_OK);
     EndDialog (hDlg, FALSE);
     return (FALSE);
    }

   /* Seleccionamos la primera biblioteca de la lista */
   SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_SETCURSEL, 0, 0L);

   /* Inicializamos la ventana edit */
   SendDlgItemMessage (hDlg, IDC_EDIT, EM_LIMITTEXT, 8, 0L);
   SetDlgItemText (hDlg, IDC_EDIT, "");

   /* Le damos el foco */
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
			   (LONG) nombre);

       /* Colocamos en la ventana edit la Biblioteca seleccionada */
       SetDlgItemText (hDlg, IDC_EDIT, nombre);

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

    case IDC_OK:
    {
     /* Obtenemos el nombre de la biblioteca */
     GetDlgItemText (hDlg, IDC_EDIT, nombre, 9);
     if ((p = (char *) lstrchr (nombre, '.')) != NULL)
       *p = 0;
     strcat (nombre, ".BBL");

     /* Si la Biblioteca no existe, avisamos */
     if (OpenFile (nombre, &of, OF_EXIST) == -1)
       MessageBox (hDlg, "Esta Biblioteca no existe",
		   "Error borrando Biblioteca", MB_ICONINFORMATION | MB_OK);

     /* pero si existe, pedimos confirmaci¢n */
     else
       if (MessageBox (hDlg, "¨Est  seguro de que quiere\nborrar esta Biblioteca?",
		       "Borrando Biblioteca",
		       MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
	 OpenFile (nombre, &of, OF_DELETE);

     /* Damos fin a la caja de Di logo */
     EndDialog (hDlg, TRUE);

     return (TRUE);
    }

    case IDC_CANCELAR:
    {
     /* Damos fin a la caja de Di logo */
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

BOOL FAR PASCAL _export ProcDialogAddSym (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)
{
 static char             seleccion[13];
 static InfoBiblio NEAR *npIb;
 HDC      		 hdc;
 HWND     		 hSimbolo;
 HANDLE   		 hMF;
 char 	     	  	 nom[128], *p;
 BOOL                    flag;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos los datos de la biblioteca */
   npIb = (InfoBiblio NEAR *) HIWORD (lParam);

   /* Creamos el filtro de ficheros */
   strcpy (nom, npIb->path);
   strcat (nom, "*.WMF");

   /* Inicializamos la List Box (tambi‚n nos cambia el directorio) */
   DlgDirList (hDlg, nom, IDC_LISTFICH, NULL, 0);

   /* Seleccionamos el primer s¡mbolo de la lista */
   SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_SETCURSEL, 0, 0L);

   /* Obtenemos la seleci¢n actual */
   SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_GETTEXT, 0, (LONG) seleccion);

   /* Inicializamos la ventana edit */
   SendDlgItemMessage (hDlg, IDC_EDIT, EM_LIMITTEXT, 12, 0L);
   SetDlgItemText (hDlg, IDC_EDIT, seleccion);

   /* Le damos el foco */
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
			   (LONG) seleccion);

       /* Colocamos en la ventana edit el elemento seleccionado */
       SetDlgItemText (hDlg, IDC_EDIT, seleccion);

       /* Representamos el metafile en el recuadro */
       hSimbolo = GetDlgItem (hDlg, IDC_ICONO);
       InvalidateRect (hSimbolo, NULL, TRUE);
       UpdateWindow (hSimbolo);
       if ((hMF = GetMetaFile (seleccion)) != NULL)
	{
	 hdc = GetDC (hSimbolo);
	 PlayMetaFile (hdc, hMF);
	 DeleteMetaFile (hMF);
	 ReleaseDC (hSimbolo, hdc);
	}
       else
	 SetWindowText (hSimbolo, "Sin simbolo");

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

    case IDC_OK:
    {
     /* Obtenemos el nombre del elemento */
     GetDlgItemText (hDlg, IDC_EDIT, seleccion, 12);
     if ((p = lstrchr (seleccion, '.')) != NULL)
       *p = 0;

     /* Buscamos el nombre en el ¡ndice */
     flag = FALSE;
     _llseek (npIb->hIndice, 0, 0);
     while ((_lread (npIb->hIndice, (LPSTR) nom, 9) > 0) && !flag)
       flag = (strcmp (seleccion, nom) == 0);

     /* Si el nombre existe, l¢gicamente, no se a¤ade */
     if (flag)
       MessageBox (hDlg, "Este elemento ya existe",
			 "Añadiendo elemento",
			 MB_ICONINFORMATION | MB_OK);

     /* pero, si no existe,.. */
     else
      {
       /* ..hacemos nPos igual al nuevo 'n_elems' menos 1 */
       npIb->nPos = npIb->n_elems;

       /* Fijamos el nuevo 'n_elems' */
       ++npIb->n_elems;

       /* Archivamos el nombre del elemento en el ¡ndice */
       _lwrite (npIb->hIndice, (LPSTR) seleccion, 9);

       /* Damos fin a la caja de di logo */
       EndDialog (hDlg, TRUE);
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

BOOL FAR PASCAL _export ProcDialogBuscSym (HWND hDlg, WORD message,
					   WORD wParam, LONG lParam)
{
 static struct ffblk bloquefich;
 static char *nombre;
 static BOOL match = FALSE;
 OFSTRUCT of;
 HDC      hdc;
 HWND     hSimbolo;
 HANDLE   hMF;
 int      hFichero;
 char     seleccion[20],
	  nombre1[13], nombre2[13],
	  *p;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos la direcci¢n del string en el que devolveremos el nombre */
   nombre = (char *)lParam;

   /* Localizamos el primer fichero ¡ndice de librer¡a, y si no existe.. */
   if (findfirst ("*.BBL", &bloquefich, FA_ARCH))
    {
     /* ..informamos del error */
     MessageBox (hDlg, "No existen Bibliotecas", "Buscando simbolo",
			MB_ICONEXCLAMATION | MB_OK);

     /* Damos fin a la caja de di logo */
     PostMessage (hDlg, WM_COMMAND, IDC_CANCELAR, 0L);
     return (FALSE);
    }

   /* pero, si el ¡ndice existe.. */
   else
    {
     /* ..inicializamos la ventana edit */
     SendDlgItemMessage (hDlg, IDC_EDIT, EM_LIMITTEXT, 8, 0L);
     SetDlgItemText (hDlg, IDC_EDIT, "");

     /* Le damos el foco */
     SetFocus (GetDlgItem (hDlg, IDC_EDIT));
    }

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

       /* Al cambiar el nombre, ya no encajar  con el anterior */
       match = FALSE;

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
			   (LONG) seleccion);

       /* Colocamos en la ventana edit el elemento seleccionado */
       SetDlgItemText (hDlg, IDC_EDIT, seleccion);

       /* Obtenemos el nombre del MetaFile */
       *strchr (seleccion, ' ') = 0;
       strcat (seleccion, ".WMF");

       /* Representamos el metafile en el recuadro */
       hSimbolo = GetDlgItem (hDlg, IDC_ICONO);
       SetWindowText (hSimbolo, "");
       InvalidateRect (hSimbolo, NULL, TRUE);
       UpdateWindow (hSimbolo);
       if ((hMF = GetMetaFile (seleccion)) != NULL)
	{
	 hdc = GetDC (hSimbolo);
	 PlayMetaFile (hdc, hMF);
	 DeleteMetaFile (hMF);
	 ReleaseDC (hSimbolo, hdc);
	}
       else
	 SetWindowText (hSimbolo, "Sin simbolo");

       return (TRUE);
      }

      case LBN_DBLCLK:
      {
       SendMessage (hDlg, WM_COMMAND, IDC_BOTON1, 0L);

       return (TRUE);
      }
     }

     return (FALSE);
    }

    case IDC_BOTON1:
    case IDC_BOTON2:
    {
     /* Si se encontr¢ un elemento, damos fin a la caja de di logo */
     if (match)
      {
       GetDlgItemText (hDlg, IDC_EDIT, nombre, 9);
       if ((p = strchr (nombre, '.')) != NULL)
	 *p = 0;

       EndDialog (hDlg, TRUE);
      }

     /* si no, buscamos */
     else
      {
       /* Obtenemos el texto pista */
       GetDlgItemText (hDlg, IDC_EDIT, nombre1, 8);
       if ((p = strchr (nombre1, '.')) != NULL)
	 *p = 0;

       /* Reseteamos el contenido de la lista */
       SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_RESETCONTENT, 0, 0L);

       do
       {
	/* Abrimos el fichero ¡ndice */
	hFichero = OpenFile (bloquefich.ff_name, &of, OF_READ);

	/* A¤adimos a la lista todos los nombres que coincidan */
	while (_lread (hFichero, nombre2, 9) > 0)
	  if (lstrlen (nombre2) >= lstrlen (nombre1))
	    if (strncmp (nombre1, nombre2, lstrlen (nombre1)) == 0)
	     {
	      strcpy (seleccion, nombre2);
	      strcat (seleccion, " -> ");
	      strcat (seleccion, bloquefich.ff_name);
	      *strrchr (seleccion, '.') = 0;
	      SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_ADDSTRING, 0,
				  (LONG) seleccion);
	     }

	/* Cerramos el fichero ¡ndice */
	_lclose (hFichero);
       }
       while (findnext (&bloquefich) == 0);

       /* Fijamos el primer elemento de la lista como la selecci¢n actual */
       SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_SETCURSEL, 0, 0L);
       SendMessage (hDlg, WM_COMMAND, IDC_LISTFICH, MAKELONG(0, LBN_SELCHANGE));

       /* Si s¢lo hay un elemento coincidente, activamos match */
       if (SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_GETCOUNT, 0, 0L) == 1)
	 match = TRUE;
      }

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

BOOL FAR PASCAL _export ProcDialogListSym (HWND hDlg, WORD message,
					   WORD wParam, LONG lParam)
{
 static char *nombre;
 OFSTRUCT of;
 HDC      hdc;
 HWND     hSimbolo;
 HANDLE   hMF;
 int      hFichero;
 char     seleccion1[13], seleccion2[13],
	  *p;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos la direcci¢n del string en el que devolveremos el nombre */
   nombre = (char *)lParam;

   /* Inicializamos la lista de Bibliotecas */
   DlgDirList (hDlg, "*.BBL", IDC_COMBO1, 0, 0);

   /* Si no hay Bibliotecas, retornamos */
   if (SendDlgItemMessage (hDlg, IDC_COMBO1, LB_GETCOUNT, 0, 0L) == 0)
    {
     MessageBox (hDlg, "No existen Bibliotecas", "Listando simbolos",
		 MB_ICONEXCLAMATION | MB_OK);

     PostMessage (hDlg, WM_COMMAND, IDC_CANCELAR, 0L);
     return (FALSE);
    }

   /* Seleccionamos la primera biblioteca */
   SendDlgItemMessage (hDlg, IDC_COMBO1, LB_SETCURSEL, 0, 0L);

   /* Le damos el foco */
   SetFocus (GetDlgItem (hDlg, IDC_COMBO1));

   /* Inicializamos la lista de elementos */
   SendMessage (hDlg, WM_COMMAND, IDC_COMBO1, MAKELONG (0, LBN_SELCHANGE));

   return (FALSE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_COMBO1:
    {
     switch (HIWORD(lParam))
     {
      case LBN_SELCHANGE:
      {
       /* Obtenemos la selecci¢n actual */
       SendDlgItemMessage (hDlg, IDC_COMBO1, LB_GETTEXT,
			   SendDlgItemMessage (hDlg, IDC_COMBO1,
					       LB_GETCURSEL, 0, 0),
			   (LONG) seleccion1);

       /* Inicializamos la lista de elementos */
       SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_RESETCONTENT, 0, 0L);
       hFichero = OpenFile (seleccion1, &of, OF_READ);
       while (_lread (hFichero, (LPSTR) seleccion2, 9) > 0)
	 SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_ADDSTRING, 0,
			     (LONG) seleccion2);
       _lclose (hFichero);
       SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_SETCURSEL, 0, 0L);
       SendMessage (hDlg, WM_COMMAND, IDC_LISTFICH, MAKELONG(0, LBN_SELCHANGE));

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
       /* Obtenemos la selecci¢n actual */
       SendDlgItemMessage (hDlg, IDC_LISTFICH, LB_GETTEXT,
			   SendDlgItemMessage (hDlg, IDC_LISTFICH,
					       LB_GETCURSEL, 0, 0),
			   (LONG) seleccion2);

       /* Copiamos el nombre en el string de retorno */
       strcpy (nombre, seleccion2);

       /* Obtenemos el nombre del MetaFile */
       strcat (seleccion2, ".WMF");

       /* Representamos el metafile en el recuadro */
       hSimbolo = GetDlgItem (hDlg, IDC_ICONO);
       SetWindowText (hSimbolo, "");
       InvalidateRect (hSimbolo, NULL, TRUE);
       UpdateWindow (hSimbolo);
       if ((hMF = GetMetaFile (seleccion2)) != NULL)
	{
	 hdc = GetDC (hSimbolo);
	 PlayMetaFile (hdc, hMF);
	 DeleteMetaFile (hMF);
	 ReleaseDC (hSimbolo, hdc);
	}
       else
	 SetWindowText (hSimbolo, "Sin simbolo");

       return (TRUE);
      }

      case LBN_DBLCLK:
      {
       SendMessage (hDlg, WM_COMMAND, IDC_BOTON1, 0L);

       return (TRUE);
      }
     }

     return (FALSE);
    }

    case IDC_BOTON1:
    case IDC_BOTON2:
    {
     /* Damos fin a la caja de di logo */
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
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogBloque (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo de Bloque */
{
 static InfoBloque NEAR *npIB;
 static HANDLE     	 hInstance;
 static bloque        	 datos_bloque;
 static short      	 bloque_act, bl[4];
 static char             szNameAbrir[] = "ABRIR";
 OFSTRUCT    of;
 PAINTSTRUCT ps;
 RECT        rectW, rectD;
 POINT       pt, ptNuevaPos;
 FARPROC     lpfnProcDialogBox;
 HDC         hdc;
 HPEN        hPen, hViejoPen;
 int         hFichero;
 char        nom[84], szBuffer[51];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos el handle a la ocurrencia del m¢dulo */
   hInstance = GetWindowWord (GetParent (hDlg), GWW_HINSTANCE);

   /* Obtenemos los datos de la ventana */
   bloque_act = (short) LOWORD(lParam);
   npIB = (InfoBloque NEAR *) HIWORD(lParam);

   /* Obtenemos los datos del cuadripolo */
   bloques = (short *) LocalLock (npIB->hmemBloques);
   memcpy (bl, bloques + 4*bloque_act, 4 * sizeof (short));
   LocalUnlock (npIB->hmemBloques);

   strcpy (nom, npIB->path);
   strcat (nom, npIB->nom_fich);
   if ((hFichero = OpenFile (strcat (nom, szExtFich[TMPBLOQUES]), &of,
			     OF_READ)) != -1)
    {
     _llseek (hFichero, bloque_act * sizeof (datos_bloque), 0);
     _lread  (hFichero, (LPSTR)&datos_bloque, sizeof (datos_bloque));
     _lclose (hFichero);
    }

   /* Inicializar controles Edit */
   SetDlgItemText (hDlg, IDC_EDIT,     datos_bloque.texto);
   SetDlgItemText (hDlg, IDC_EDIT2,    datos_bloque.fichero);
   SetDlgItemText (hDlg, IDC_EDITBASE, datos_bloque.metafile);

   /* Inicializar Radio Buttons */
   CheckRadioButton (hDlg, IDC_BOTONBASE, IDC_BOTONBASE + 2,
		     IDC_BOTONBASE + 0 * (datos_bloque.alinh == DT_LEFT) +
				     1 * (datos_bloque.alinh == DT_RIGHT) +
				     2 * (datos_bloque.alinh == DT_CENTER));
   CheckRadioButton (hDlg, IDC_BOTONBASE + 3, IDC_BOTONBASE + 5,
		     IDC_BOTONBASE + 3 * (datos_bloque.alinv == DT_TOP) +
				     4 * (datos_bloque.alinv == DT_BOTTOM) +
				     5 * (datos_bloque.alinv == DT_CENTER));

   /* Posicionamos la caja de di logo */
   GetWindowRect (hDlg, &rectD);
   ClientToScreen (GetParent (hDlg), (LPPOINT) &rectD.left);
   ClientToScreen (GetParent (hDlg), (LPPOINT) &rectD.right);

   pt.x = bl[0] - npIB->nHposScrl * SCRLDESPL;
   pt.y = bl[1] - npIB->nVposScrl * SCRLDESPL;
   ClientToScreen (GetParent (hDlg), &pt);

   ptNuevaPos.x = (pt.x > GetSystemMetrics (SM_CXSCREEN) -
			  40 - (rectD.right - rectD.left)) ?
				 max (0, pt.x - (rectD.right - rectD.left)) :
				 min (GetSystemMetrics (SM_CXSCREEN) -
				      (rectD.right - rectD.left), pt.x + 40);
   ptNuevaPos.y = (pt.y > GetSystemMetrics (SM_CYSCREEN) -
			  40 - (rectD.bottom - rectD.top)) ?
				 max (0, pt.y - (rectD.bottom - rectD.top)) :
				 min (GetSystemMetrics (SM_CYSCREEN) -
				      (rectD.bottom - rectD.top), pt.y + 40);

   MoveWindow (hDlg, ptNuevaPos.x, ptNuevaPos.y,
		     rectD.right - rectD.left + 1, rectD.bottom - rectD.top + 1,
	       TRUE);

   return (FALSE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
    {
     /* Obtenemos el texto de la caja de di logo */
     GetDlgItemText (hDlg, IDC_EDIT, szBuffer, 50);
     strcpy (datos_bloque.texto, szBuffer);

     /* Obtenemos el fichero de esquema asociado */
     GetDlgItemText (hDlg, IDC_EDIT2, szBuffer, 50);
     strcpy (datos_bloque.fichero, szBuffer);

     /* Obtenemos el metafile asociado */
     GetDlgItemText (hDlg, IDC_EDITBASE, szBuffer, 50);
     strcpy (datos_bloque.metafile, szBuffer);

     /* Obtenemos las alineaciones */
     datos_bloque.alinh = DT_LEFT   * IsDlgButtonChecked (hDlg, IDC_BOTONBASE) +
			  DT_RIGHT  * IsDlgButtonChecked (hDlg, IDC_BOTONBASE+1) +
			  DT_CENTER * IsDlgButtonChecked (hDlg, IDC_BOTONBASE+2);
     datos_bloque.alinv = DT_TOP    * IsDlgButtonChecked (hDlg, IDC_BOTONBASE+3) +
			  DT_BOTTOM * IsDlgButtonChecked (hDlg, IDC_BOTONBASE+4) +
			  DT_CENTER * IsDlgButtonChecked (hDlg, IDC_BOTONBASE+5);

     /* Guardamos los datos del bloque */
     strcpy (nom, npIB->path);
     strcat (nom, npIB->nom_fich);
     hFichero = OpenFile (strcat (nom, szExtFich[TMPBLOQUES]), &of, OF_WRITE);
     _llseek (hFichero, bloque_act * sizeof (datos_bloque), 0);
     _lwrite (hFichero, (LPSTR)&datos_bloque, sizeof (datos_bloque));
     _lclose (hFichero);

     EndDialog (hDlg, TRUE);
     return (TRUE);
    }

    case IDC_CANCELAR:
    {
     EndDialog (hDlg, FALSE);
     return (TRUE);
    }

    case IDC_BOTON3:
    {
     /* Fijamos el tipo de ventana */
     tipo_ventana = TV_ESQUEMA;

     /* Obtenemos la dir. del proc. para la Caja de Di logo de Abrir */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogAbrir,
					   hInstance);

     /* Si la caja de di logo termin¢ con OK, actualizamos el fichero */
     strcpy (szBuffer, "Elegir un Circuito");
     if (DialogBoxParam (hInstance, szNameAbrir, hDlg,
			 lpfnProcDialogBox, (LONG) szBuffer))
      {
       strcpy (nom, path);
       strcat (nom, nom_fich);
       SetDlgItemText (hDlg, IDC_EDIT2, nom);
      }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     return (TRUE);
    }

    case IDC_BOTON4:
    {
     /* Fijamos el tipo de ventana */
     tipo_ventana = TV_METAFILE;

     /* Obtenemos la dir. del proc. para la Caja de Di logo de Abrir */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogAbrir,
					   hInstance);

     /* Si la caja de di logo termin¢ con OK, actualizamos el fichero */
     strcpy (szBuffer, "Elegir un Gráfico");
     if (DialogBoxParam (hInstance, szNameAbrir, hDlg,
			 lpfnProcDialogBox, (LONG) szBuffer))
      {
       strcpy (nom, path);
       strcat (nom, nom_fich);
       SetDlgItemText (hDlg, IDC_EDITBASE, nom);
      }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     return (TRUE);
    }
   }

   return (FALSE);
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogPuerto (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo de Puerto */
{
 switch (message)
 {
  case WM_INITDIALOG:
  {
   return (FALSE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
      EndDialog (hDlg, TRUE);
      return (TRUE);

    case IDC_CANCELAR:
      EndDialog (hDlg, FALSE);
      return (TRUE);
   }

   return (FALSE);
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogTexto (HWND hDlg, WORD message,
					 WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo de Texto */
{
 switch (message)
 {
  case WM_INITDIALOG:
  {
   return (FALSE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_OK:
      EndDialog (hDlg, TRUE);
      return (TRUE);

    case IDC_CANCELAR:
      EndDialog (hDlg, FALSE);
      return (TRUE);
   }

   return (FALSE);
  }
 }

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del FicheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/