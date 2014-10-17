#include "compilar.h"
#include "wincomp.h"

/*-----------"NOTAS SOBRE LOS CONVENIOS RESPECTO A GENERADORES"-------------

a) Generadores independientes;

   1) De tensi¢n: la borna positiva del generador debe coincidir siempre con
		  el nodo que se encuentre en primer lugar en el registro de
		  la rama, es decir, con 'nodoi'. Caso de no poderse cumplir
		  este requisito, el valor del generador debe especificarse
		  como negativo (p. ej., cuando hay dos generadores en serie
		  en una misma rama y tengan polaridades opuestas, aquel de
		  ellos que tenga su borna + conectada al nodo registrado
		  como 'nodoi' tendr  su valor real, pero al otro deber 
		  asign rsele su valor cambiado de signo).

   2) De corriente: el flujo de la corriente del generador debe ir siempre
		    del nodo que se encuentre en primer lugar en el registro
		    de la rama, es decir, de 'nodoi', al que se encuentre el
		    £ltimo, es decir, 'nodof'. Caso de no poderse cumplir
		    este requisito, el valor del generador debe especificarse
		    cambiado de signo.

b) Generadores dependientes;

   1) De tensi¢n dep. de tensi¢n:
	En cuento al generador vale lo dicho para los generadores de tensi¢n
	independientes. Respecto a la tensi¢n de rama de la que dependen,
	, esta tensi¢n debe tener su extremo positivo en 'nodoi' y el nega-
	tivo en 'nodof'. Caso contrario, el valor de la cte. del generador
	deber  cambiarse de signo.

   2) De tensi¢n dep. de corriente:
	Vale lo dicho para el anterior, excepto en que la corriente por la
	rama de la que depende el generador debe ir de 'nodoi' hacia 'nodof',
	, y si no pudiera cumplirse este requisito, el valor de la cte. del
	generador deber  cambiarse de signo.

   3) De corriente dep. de tensi¢n;
	Sus requisitos son, respecto al generador, los mismos que para los
	generadores de corriente independientes, y respecto a la variable de
	la que dependen, los mismos que para la variable de la que dependen
	los generadores de tensi¢n dep. de tensi¢n. Es decir, sentido de la
	corriente del generador de 'nodoi' a 'nodof', tensi¢n de la que de-
	penden positiva en 'nodoi' y negativa en 'nodof'.

   4) De corriente dep. de corriente;
	Sus requisitos son, respecto al generador, los mismos que para el
	anterior, y respecto a la variable de la que dependen, los mismos
	que para la variable de la que dependen los generadores de tensi¢n
	dep. de corriente. Es decir, sentido de la corriente del generador
	de 'nodoi' a 'nodof', corriente de la que dependen de 'nodoi' a
	'nodof'.                                                            */

/*----------------Zona de definici¢n de variables globales------------------*/

HWND       hwnd, hwndDebug;
BOOL       hay_fichero = FALSE;
char       szAppName[]         = "COMPILAR",
	   szDebugClass[]      = "DEPURAR";
FILE       *fuente, *listado, *config;
BOOL       Informe, Make, precalc, ayuda = FALSE;
reg_elem   *elem_ini;
reg_cuadri *cuad_ini;
registro   *rama_ini;
reg_medida *resp_ini;
float      T = 300;
unsigned   max_nudo = 0,
	   nMaxLin  = 0, nMaxCol = 0, nLinea   = 0, nCol = 0,
	   nAvisos  = 0, nErrores = 0,
	   errores = ERRORES;
short      cyLine, cyChar, nHposScrl, nVposScrl;
char       path[80], nom_fich[80], c, frase1[13], frase2[13];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

size_t     mi_fread (void *, size_t, size_t, FILE *);
void       error (int codigo);
void       no_orden (reg_elem *primero, reg_elem *elem);
reg_elem   *buscar (reg_elem *primero, char nombre[5]);
reg_cuadri *buscarc (reg_cuadri *primero, char nombre[11]);
int        buscar_siguiente (char inf, char sup, char *c, short avance);
int        comprimir (reg_elem *, registro *, reg_medida *);
double     convertir (char *c);

extern void       precalculo      (void);
extern reg_elem   *compilar_elems (reg_cuadri **primer_cuad);
extern registro   *compilar_ramas (void);
extern reg_medida *compilar_resp  (void);
extern void       analizar        (void);

long FAR PASCAL _export ProcVent           (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogAbrir    (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogCompilar (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcDialogAcerca   (HWND, WORD, WORD, LONG);

extern long FAR PASCAL _export ProcDepurar (HWND, WORD, WORD, LONG);

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

 /* Tomamos el path del circuito */
 if (config = fopen ("config.sca", "rt"))
  {
   fscanf (config, "%[^\n]", path);
   fclose (config);
  }
 else
   strcpy (path, PATH);

 /* Registramos la clase de la ventana principal y las de depuraci¢n */
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

   wndclass.lpfnWndProc   = ProcDepurar;
   wndclass.cbWndExtra    = sizeof (WORD);
   wndclass.hIcon	  = NULL;
   wndclass.lpszClassName = szDebugClass;

   RegisterClass (&wndclass);
  }

 /* Creamos la ventana principal */
 srand ((unsigned) hInstance);
 hwnd = CreateWindow (szAppName, "Compilador D.C.E.",
		      WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		      random (GetSystemMetrics(SM_CXSCREEN) - 310),
		      random (GetSystemMetrics(SM_CYSCREEN) - 310),
		      308, 293, NULL, NULL, hInstance, NULL);

 ShowWindow   (hwnd, nCmdShow);

 /* Si la l¡nea de comando existe, se toma */
 if (lstrlen (lpszCmdLine) > 0)
  {
   lstrcat (nom_fich, lpszCmdLine);
   hay_fichero = TRUE;
  }

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
		szNameCompilar[] = "COMPILAR",
		szNameAcerca[]   = "ACERCA";
 HDC	        hdc, hdcMem1, hdcMem2;
 HBITMAP        hbitmap1, hviejobitmap;
 MSG		msg;
 RECT	        rect;
 TEXTMETRIC     tm;
 PAINTSTRUCT    ps;
 BITMAP	        bm1, bm2;
 FARPROC        lpfnProcDialog;
 OFSTRUCT       of;
 int            hFichero;
 FILE           *elementos;
 short          nIncScrl;
 char           szBuffer[30], nom[84], *punt, type;
 reg_elem       *elem_act, *elem_aux;
 reg_cuadri     *cuad_act, *cuad_aux;
 registro       *rama_act, *rama_aux;
 reg_medida     *resp_act, *resp_aux;

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

   /* Deshabilitamos las opciones "Compilar" y "Depurar" */
   EnableMenuItem (hMenu, IDM_VER,      MF_GRAYED);
   EnableMenuItem (hMenu, IDM_COMPILAR, MF_GRAYED);
   EnableMenuItem (hMenu, IDM_DEPURAR,  MF_GRAYED);

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
    }

   return (0);
  }

  case WM_PAINT:
  {
   if (!IsIconic (hwnd))
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

       /* Liberamos el contexto de disp. de memoria */
       DeleteDC (hdcMem1);
      }
     else
      {
       /* Obtenemos el contexto de la ventana */
       hdc = BeginPaint (hwnd, &ps);

       /* Obtenemos el tama¤o del  rea de cliente */
       GetClientRect (hwnd, &rect);

       TextOut (hdc, (rect.right - 10 * cxChar - 9 * cxMax) / 2 -
		     nHposScrl * SCRLDESPL,
		cyLine - nVposScrl * SCRLDESPL,
		szBuffer, sprintf (szBuffer, "Circuito : %s", frase1));

       TextOut (hdc, (rect.right - 17 * cxChar - 2 * cxMax) / 2 -
		     nHposScrl * SCRLDESPL,
		3 * cyLine - nVposScrl * SCRLDESPL,
		szBuffer, sprintf (szBuffer, "Estado : %s", frase2));

       TextOut (hdc, rect.right / 2 - 16 * cxChar - cxMax -
		     nHposScrl * SCRLDESPL,
		5 * cyLine - nVposScrl * SCRLDESPL,
		szBuffer, sprintf (szBuffer, "L¡neas totales : %5u", nMaxLin));

       TextOut (hdc, rect.right / 2 - 19 * cxChar - cxMax -
		     nHposScrl * SCRLDESPL,
		7 * cyLine - nVposScrl * SCRLDESPL,
		szBuffer, sprintf (szBuffer, "L¡neas Compiladas : %5u", nLinea));

       TextOut (hdc, rect.right / 2 - 9 * cxChar - cxMax -
		     nHposScrl * SCRLDESPL,
		9 * cyLine - nVposScrl * SCRLDESPL,
		szBuffer, sprintf (szBuffer, "Errores : %u", nErrores));

       TextOut (hdc, rect.right / 2 - 8 * cxChar - cxMax -
		     nHposScrl * SCRLDESPL,
		11 * cyLine - nVposScrl * SCRLDESPL,
		szBuffer, sprintf (szBuffer, "Avisos : %u", nAvisos));
      }

     /* Liberamos el contexto de ventana */
     EndPaint (hwnd, &ps);

     return (0);
    }

   break;
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
	 /* ..abrimos el fichero fuente */
	 strcpy (nom, nom_fich);
	 fuente = fopen (nom, "rt");

	 /* Activamos el flag */
	 hay_fichero = TRUE;

	 /* Calculamos el n§ de l¡neas, el n§ de nudos etc. */
	 precalculo ();
	 fclose (fuente);

	 /* Inicializamos las frases para la ventana */
	 punt = lstrrchr (nom_fich, '\\');
	 if (punt == NULL)
	   punt = nom_fich;
	 strcpy (frase1, AnsiUpper (punt));
	 *strrchr (frase1, '.') = 0;

	 strcpy (frase2, "Inactivo\0");

	 /* Inicializamos variables */
	 nLinea = nCol = nErrores = nAvisos = 0;

	 /* Invalidamos la ventana entera */
	 InvalidateRect (hwnd, NULL, TRUE);

	 /* Validamos las opciones del men£ */
	 EnableMenuItem (hMenu, IDM_VER,       MF_ENABLED);
	 EnableMenuItem (hMenu, IDM_COMPILAR,  MF_ENABLED);
	 EnableMenuItem (hMenu, IDM_DEPURAR,   MF_GRAYED);
	}

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialog);

       return (0);
      }

      case IDM_VERFUENTE:
       break;

      case IDM_VERCOMPIL:
       break;

      case IDM_COMPILAR:
      {
       /* Obtenemos la ocurrencia del procedimiento din micamente */
       lpfnProcDialog = MakeProcInstance ((FARPROC) ProcDialogCompilar,
					  hInstance);

       /* Si la caja de di logo finaliz¢ normalmente.. */
       if (DialogBox (hInstance, szNameCompilar, hwnd, lpfnProcDialog))
	{
	 /* Inicializamos variables */
	 nLinea = nCol = nErrores = nAvisos = 0;
	 strcpy (frase2, "Compilando");

	 GetClientRect (hwnd, &rect);
	 rect.top    = 3 * cyLine - nVposScrl * SCRLDESPL;
	 rect.bottom = rect.top + cyLine - 1;
	 InvalidateRect (hwnd, &rect, FALSE);
	 UpdateWindow (hwnd);

	 /* Abrimos y procesamos el fichero fuente (que tiene que existir) */
	 strcpy (nom, nom_fich);
	 fuente = fopen (nom, "rt");

	 /* Abrimos el fichero que contendr  los errores */
	 if (Informe)
	  {
	   strcpy (nom, nom_fich);
	   *strrchr (nom, '.') = 0;
	   strcat (nom, ".LST");
	   listado = fopen (nom, "wt");
	  }

	 /* Compilamos todo */
	 elem_ini = compilar_elems (&cuad_ini);
	 rama_ini = compilar_ramas ();
	 resp_ini = compilar_resp ();

	 /* Cerramos el fichero de errores, si es el caso */
	 if (Informe)
	   fclose (listado);

	 /* Si no se produjeron errores, guardamos todos los datos */
	 if (nErrores == 0)
	  {
	   /* Comprimimos la numeraci¢n de nudos, es decir, hacemos los cambios necesa-
	      rios para que dicha numeraci¢n sea cont¡nua */
	   comprimir (elem_ini, rama_ini, resp_ini);

	   /* Abrimos el fichero que contendr  los elementos compilados */
	   strcpy (nom, nom_fich);
	   *strrchr (nom, '.') = 0;
	   strcat (nom, ".ELM");
	   hFichero = OpenFile (nom, &of, OF_CREATE | OF_SHARE_EXCLUSIVE);

	   /* Grabamos los elementos, excepto los modelos */
	   for (elem_act = elem_ini; elem_act != NIL;
		elem_act = elem_act->siguiente)
	    {
	     type = elem_act->datos.tipo[0];

	     if (((type >= 'A') && (type <= 'E')) || (type == 'V') ||
		 (type == 'I') || (type == 'R') || (type == 'L') ||
		 (type == 'Q'))
	       _lwrite (hFichero, (LPSTR) &elem_act->datos, sizeof (elemento));
	    }

	   /* Cerramos el fichero de los elementos */
	   _lclose (hFichero);

	   /* Abrimos el fichero que contendr  los cuadripolos compilados */
	   strcpy (nom, nom_fich);
	   *strrchr (nom, '.') = 0;
	   strcat (nom, ".CUA");
	   hFichero = OpenFile (nom, &of, OF_CREATE | OF_SHARE_EXCLUSIVE);

	   /* Grabamos los elementos, excepto los modelos */
	   for (cuad_act = cuad_ini; cuad_act != NIL;
		cuad_act = cuad_act->siguiente)
	    {
	     _lwrite (hFichero, (LPSTR) &cuad_act->datos, sizeof (cuadripolo));
	    }

	   /* Cerramos el fichero de los cuadripolos */
	   _lclose (hFichero);

	   /* Abrimos el fichero que contendr  los datos del circuito compilados */
	   strcpy (nom, nom_fich);
	   *strrchr (nom, '.') = 0;
	   strcat (nom, ".CIR");
	   hFichero = OpenFile (nom, &of, OF_CREATE | OF_SHARE_EXCLUSIVE);

	   /* Guardamos los datos compilados */
	   for (rama_act = rama_ini; rama_act != NIL;
		rama_act = rama_act->siguiente)
	    {
	     _lwrite (hFichero, (LPSTR) &rama_act->datos, sizeof (rama));
	    }

	   /* Cerramos el fichero del circuito */
	   _lclose (hFichero);

	   /* Abrimos el fichero que contendr  los datos de medidas compilados */
	   strcpy (nom, nom_fich);
	   *strrchr (nom, '.') = 0;
	   strcat (nom, ".RES");
	   hFichero = OpenFile (nom, &of, OF_CREATE | OF_SHARE_EXCLUSIVE);

	   /* Guardamos los datos compilados */
	   for (resp_act = resp_ini; resp_act != NIL;
		resp_act = resp_act->siguiente)
	    {
	     _lwrite (hFichero, (LPSTR) &resp_act->datos, sizeof (respuesta));
	    }

	   /* Cerramos el fichero de medidas */
	   _lclose (hFichero);
	  }

	 /* pero, si ha habido errores.. */
	 else
	  {
	   /* ..eliminamos los ficheros objeto */
	   strcpy (nom, nom_fich);

	   *strrchr (nom, '.') = 0;
	   strcat (nom, ".ELM");
	   OpenFile (nom, &of, OF_DELETE);

	   *strrchr (nom, '.') = 0;
	   strcat (nom, ".CUA");
	   OpenFile (nom, &of, OF_DELETE);

	   *strrchr (nom, '.') = 0;
	   strcat (nom, ".CIR");
	   OpenFile (nom, &of, OF_DELETE);

	   *strrchr (nom, '.') = 0;
	   strcat (nom, ".RES");
	   OpenFile (nom, &of, OF_DELETE);
	  }

	 /* Cerramos el fichero fuente */
	 fclose (fuente);

	 /* Liberamos el espacio asignado a los elementos */
	 elem_act = elem_ini;
	 while (elem_act != NIL)
	 {
	  elem_aux = elem_act;
	  elem_act = elem_act->siguiente;
	  free (elem_aux);
	 };

	 /* Idem con los cuadripolos */
	 cuad_act = cuad_ini;
	 while (cuad_act != NIL)
	 {
	  cuad_aux = cuad_act;
	  cuad_act = cuad_act->siguiente;
	  free (cuad_aux);
	 };

	 /* Idem con las ramas */
	 rama_act = rama_ini;
	 while (rama_act != NIL)
	 {
	  rama_aux = rama_act;
	  rama_act = rama_act->siguiente;
	  free (rama_aux);
	 };

	 /* Idem con las medidas */
	 resp_act = resp_ini;
	 while (resp_act != NIL)
	 {
	  resp_aux = resp_act;
	  resp_act = resp_act->siguiente;
	  free (resp_aux);
	 };

	 /* Si no ha habido errores.. */
	 if (nErrores == 0)
	  {
	   /* ..analizamos el circuito por nudos y por mallas */
	   strcpy (frase2, "Analizando ");

	   InvalidateRect (hwnd, &rect, FALSE);
	   UpdateWindow (hwnd);

	   analizar ();
	  }

	 /* Una vez terminados todos los procesos, pasamos a inactivo */
	 strcpy (frase2, "Inactivo    ");
	 InvalidateRect (hwnd, NULL, TRUE);

	 /* Validamos la opci¢n de depuraci¢n, si es el caso */
	 if (((nErrores > 0) || (nAvisos > 0)) && (Informe))
	  {
	   MessageBox (hwnd, "Errores en la compilación", frase1,
		       MB_ICONINFORMATION | MB_OK);
	   EnableMenuItem (hMenu, IDM_DEPURAR, MF_ENABLED);
	  }
	 else
	  {
	   hay_fichero = FALSE;
	   EnableMenuItem (hMenu, IDM_COMPILAR, MF_GRAYED);
	   EnableMenuItem (hMenu, IDM_DEPURAR,  MF_GRAYED);
	   EnableMenuItem (hMenu, IDM_VER,      MF_GRAYED);
	  }
	}

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialog);

       return (0);
      }

      case IDM_DEPURAR:
      {
       /* Si la ventana no existe, la creamos */
       if (!IsWindow (hwndDebug))
	{
	 GetWindowRect (hwnd, &rect);

	 /* Creamos una ventana para depuraci¢n */
	 hwndDebug = CreateWindow (szDebugClass,
				   strcat (strcpy (szBuffer, "Depurando "),
					   frase1),
				   WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME |
				   WS_SYSMENU | WS_MAXIMIZEBOX,
				   cxChar,    rect.top + 2*cyChar,
				   min (70*cxChar, GetSystemMetrics (SM_CXSCREEN)),
				   min (20*cyChar, GetSystemMetrics (SM_CYSCREEN)),
				   NULL, NULL, hInstance, NULL);

	 ShowWindow (hwndDebug, SW_SHOW);
	}

       /* Renovamos los contenidos de la ventana */
       UpdateWindow (hwndDebug);

       return (0);
      }

      case IDM_AYUDA:
      {
       /* Invocamos la ayuda de WINDOWS */
       ayuda = WinHelp (hwnd, "WINCOMP.HLP", HELP_INDEX, 0);
       return (0);
      }

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
       /* Si la ayuda est  abierta, la cerramos */
       if (ayuda)
	 WinHelp (hwnd, "WINCOMP.HLP", HELP_QUIT, 0);

       /* Enviamos un WM_DESTROY */
       DestroyWindow (hwnd);

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
   /* Imitamos el comando 'Salir' del men£ */
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     DestroyWindow (hwnd);
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

   /* Cerramos la ventana de depuraci¢n, si la hay */
   if (IsWindow (hwndDebug))
     DestroyWindow (hwndDebug);

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
 static char EspecFich[84], NomFich[13] = "*.FNT";
 int         hFichero;
 OFSTRUCT    of;

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Obtenemos el valor del path inicial y el filtro por defecto */
   lstrcpy (EspecFich, path);

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
	 strcpy (nom_fich, EspecFich);
	 if (!strrchr (nom_fich, '.'))
	   strcat (nom_fich, ".FNT");

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

BOOL FAR PASCAL _export ProcDialogCompilar (HWND hDlg, WORD message,
					    WORD wParam, LONG lParam)

/* Procedimiento para la caja de di logo de Compilar Circuito */
{
 switch (message)
 {
  case WM_INITDIALOG:
  {
   Informe = Make = TRUE;
   CheckDlgButton (hDlg, IDC_INFORME, TRUE);
   CheckDlgButton (hDlg, IDC_MAKE,    TRUE);

   return (TRUE);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDC_INFORME:
    {
     Informe = (BOOL) SendMessage (GetDlgItem (hDlg, IDC_INFORME),
				   BM_GETCHECK, 0, 0);
     return (TRUE);
    }

    case IDC_MAKE:
    {
     Make = (BOOL) SendMessage (GetDlgItem (hDlg, IDC_MAKE),
				BM_GETCHECK, 0, 0);
     return (TRUE);
    }

    case IDC_EMPEZAR:
    case IDC_CANCELAR:
    {
     EndDialog (hDlg, (wParam == IDC_EMPEZAR));
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

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int comprimir (reg_elem *elem_ini, registro *rama_ini, reg_medida *resp_ini)

/* Elimina los huecos en la numeraci¢n de nudos */
{
 reg_elem   *elem_act;
 registro   *rama_act;
 reg_medida *resp_act;
 unsigned   i, j, num_ceros = 0;
 int        *tabla_nudos;

 /* Construimos la lista de veces que aparece cada nudo */
 tabla_nudos = (int *) malloc ((max_nudo + 1) * sizeof (int));
 memset (tabla_nudos, 0, (max_nudo + 1) * sizeof (int));
 for (rama_act = rama_ini; rama_act != NIL; rama_act = rama_act->siguiente)
  {
   ++tabla_nudos[rama_act->datos.nodoi];
   ++tabla_nudos[rama_act->datos.nodof];
  }

 /* Procesamos todos los n£meros */
 for (i = 0; i <= max_nudo; ++i)
   if (tabla_nudos[i] == 0)        /* Si encontramos un cero, incrementamos */
     ++num_ceros;                  /* el n£mero de ceros rebasados.         */
   else
     if (num_ceros > 0)
      {
       /* Retrasamos todos los nudos hasta el siguiente cero */
       j = i;
       while ((tabla_nudos[++j] != 0) && (j <= max_nudo));
       memcpy (&tabla_nudos[i - num_ceros], &tabla_nudos[i],
	       (j - i) * sizeof (unsigned));

       /* Realizamos las modificaciones en los gen. dep. de V */
       elem_act = elem_ini;
       while (elem_act != NIL)
	{
	 if ((elem_act->datos.tipo[0] == 'A') ||
	     (elem_act->datos.tipo[0] == 'D'))
	  {
	   if ((elem_act->datos.caract.nodos[0] >= i) &&
	       (elem_act->datos.caract.nodos[0] < j))
	     elem_act->datos.caract.nodos[0] -= num_ceros;

	   if ((elem_act->datos.caract.nodos[1] >= i) &&
	       (elem_act->datos.caract.nodos[1] < j))
	     elem_act->datos.caract.nodos[1] -= num_ceros;
	  }

	 elem_act = elem_act->siguiente;
	}

       /* Realizamos las modificaciones en la lista de ramas */
       rama_act = rama_ini;
       while (rama_act != NIL)
	{
	 if ((rama_act->datos.nodoi >= i) && (rama_act->datos.nodoi < j))
	   rama_act->datos.nodoi -= num_ceros;

	 if ((rama_act->datos.nodof >= i) && (rama_act->datos.nodof < j))
	   rama_act->datos.nodof -= num_ceros;

	 rama_act = rama_act->siguiente;
	}

       /* Realizamos las modificaciones en la lista de medidas */
       resp_act = resp_ini;
       while (resp_act != NIL)
	{
	 if ((resp_act->datos.medida.nodos[0] >= i) &&
	     (resp_act->datos.medida.nodos[0] < j))
	   resp_act->datos.medida.nodos[0] -= num_ceros;

	 if ((resp_act->datos.medida.nodos[1] >= i) &&
	     (resp_act->datos.medida.nodos[1] < j))
	   resp_act->datos.medida.nodos[1] -= num_ceros;

	 resp_act = resp_act->siguiente;
	}

       /* Nos pasamos al siguiente cero directamente */
       i = j - 1;
      }

 /* Liberamos el espacio utilizado */
 free (tabla_nudos);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void no_orden (reg_elem *primero, reg_elem *elem)

/* Funci¢n que calcula el n§ de orden del elemento apuntado por 'elem' */
{
 reg_elem *actual = primero;
 unsigned r, n = 0;
 char no[2];

 do
 {
  if (actual->datos.tipo[0] == elem->datos.tipo[0])
    ++n;

  actual = actual->siguiente;
 }
 while (actual != NIL);

 for (r = 100; r > 0; r /= 10)
 {
  itoa (n / r, no, 10);
  n %= r;
  strcat (elem->datos.tipo, no);
 }
 return;
}

/*--------------------------------------------------------------------------*/

reg_elem *buscar (reg_elem *primero, char nombre[6])

/* Esta funci¢n busca un elemento cuyo nombre dado por el usuario sea igual
   a 'nombre' y devuelve un puntero a dicho elemento. */
{
 reg_elem *puntero = NIL;

 do
  puntero = (puntero == NIL) ? primero : puntero->siguiente;
 while ((strcmp (nombre, puntero->datos.nombre) != 0) && (puntero != NIL));

 return (puntero);
}

/*--------------------------------------------------------------------------*/

reg_cuadri *buscarc (reg_cuadri *primero, char nombre[11])

/* Esta funci¢n busca un cuadripolo cuyo nombre dado por el usuario sea igual
   a 'nombre' y devuelve un puntero a dicho cuadripolo. */
{
 reg_cuadri *puntero = NIL;

 do
  puntero = (puntero == NIL) ? primero : puntero->siguiente;
 while ((strcmp (nombre, puntero->datos.nombre) != 0) && (puntero != NIL));

 return (puntero);
}

/*--------------------------------------------------------------------------*/

int buscar_siguiente (char inf, char sup, char *c, short avance)

/* Esta funci¢n avanza en el fichero de elementos hasta encontrar el siguien-
   te caracter comprendido entre 'inf' y 'sup', ambos inclusive */
{
 if (avance)
   mi_fread (c, 1, 1, fuente);

 while (((*c < inf) || (*c > sup)) && (*c != '#') && (!feof (fuente)))
 {
  if ((*c != ',') && (*c != ' ') && (*c != 10))
    error (E_CARINESP);

  mi_fread (c, 1, 1, fuente);
 }

 if (*c == '#')
   error (E_FINDESECCION);
 else
   if (feof (fuente))
     error (E_FINDEFICHERO);

 return ((feof (fuente)) || (*c == '#'));
}

/*--------------------------------------------------------------------------*/

double convertir (char *c)

/* Esta funci¢n convierte el siguiente n£mero en formato string que est‚ en
   el fichero a n£mero en doble precisi¢n */
{
 char numero[20];
 double n = 0;

 int buscar_siguiente (char inf, char sup, char *c, short avance);

 /* Localizamos el comienzo del n£mero */
 if (buscar_siguiente ('-', '9', c, AVANZAR) != 0)
   return (0);

 /* Inicializamos la variable que lo contendr  */
 memset (numero, 0, 20);

 /* Recogemos los caracteres que lo componen */
 while (((*c >= '-') && (*c <= '9')) || (*c == '+') ||
	(*c == 'e') || (*c == 'E'))
 {
  numero[strlen (numero)] = *c;
  mi_fread (c, 1, 1, fuente);
 }

 /* Aplicamos el cambio de unidades */
 switch (*c)
 {
  case ('G'):
  {
   n = atof (numero) * 1e9;
   break;
  }

  case ('M'):
  {
   n = atof (numero) * 1e6;
   break;
  }

  case ('K'):
  {
   n = atof (numero) * 1e3;
   break;
  }

  case ('m'):
  {
   n = atof (numero) * 1e-3;
   break;
  }

  case ('u'):
  {
   n = atof (numero) * 1e-6;
   break;
  }

  case ('n'):
  {
   n = atof (numero) * 1e-9;
   break;
  }

  case ('p'):
  {
   n = atof (numero) * 1e-12;
   break;
  }

  case ('t'):
  {
   n = atof (numero) * 1e-15;
   break;
  }

  case ('f'):
  {
   n = atof (numero) * 1e-18;
   break;
  }

  default   : n = atof (numero);
 }

 /* Pasamos al car cter siguiente al de unidades, si hab¡a dicho car cter */
 if (n != atof (numero))
   mi_fread (c, 1, 1, fuente);

 return (n);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

func_trans obt_func_trans (char *c)

/* Esta funci¢n obtiene una funci¢n de transferencia definida en formato del
   Lenguaje D.C.E., comenzando en la posici¢n del fichero fuente apuntada por
   c. Un polinomio estara obligatoriamente encerrado entre parentesis */
{
 func_trans T;
 double     n;
 short      i, j, signo = 1;

 int    buscar_siguiente (char inf, char sup, char *c, short avance);
 double convertir (char *c);

 /* Inicializamos la funci¢n de transferencia */
 T.num.grado = T.den.grado = 0;
 T.num.coef[0] = 0;
 T.den.coef[0] = 1;
 for (i = 1; i < 11; ++i)
   T.num.coef[i] = T.den.coef[i] = 0;

 /* Avanzamos hasta el comienzo de la definici¢n del numerador */
 if (buscar_siguiente ('(', '(', c, AVANZAR) != 0)
   return (T);

 i = 0;
 do
 {
  /* Obtenemos el coeficiente */
  n = convertir (c);

  /* Avanzamos hasta el operador (o el fin del polinomio) */
  if (buscar_siguiente (')', '-', c, NO_AVANZAR) != 0)
    return (T);

  /* Si habia un '*'.. */
  if (*c == '*')
   {
    /* ..buscamos la 's' asociada */
    if (buscar_siguiente ('s', 's', c, AVANZAR) != 0)
      return (T);

    /* Obtenemos el exponente de la 's' */
    j = (short) convertir (c);

    /* Colocamos el coeficiente en el lugar correspondiente */
    if ((j >= 0) && (j <= 10))
      T.num.coef[j] += n;
    else
      error (W_GRADOMAYOR10);

    /* Avanzamos hasta el operador (o el fin del polinomio) */
    if (buscar_siguiente (')', '-', c, NO_AVANZAR) != 0)
      return (T);
   }

  /* pero, si habia un '+', '-' o ')'.. */
  else

    /* ..colocamos el coeficiente en el lugar 0 */
    T.num.coef[0] += signo * n;

  /* Obtenemos el nuevo signo, si es el caso */
  signo = (*c == '-') ? -1 : 1;
 }
 while ((*c != ')') && (++i < 11));

 /* Detectamos el error de demasiados t‚rminos */
 if (*c != ')')
   error (W_MUCHOSTERMINOS);

 /* Avanzamos hasta el signo de divisi¢n (o lo que venga) */
 do
 {
  mi_fread (c, 1, 1, fuente);
  if ((*c > 32) && (*c != '/') && (*c != 'Z') && (*c != '}'))
    error (E_CARINESP);
 }
 while ((*c != '/') && (*c != 'Z') && (*c != '}') && (*c != '#') &&
	(!feof(fuente)));

 /* Si no vino nada, volvemos con lo que haya */
 if (feof (fuente))
   return (T);

 /* Si encontramos el signo de division.. */
 if (*c == '/')
  {
   /* ..avanzamos hasta el comienzo de la definici¢n del denominador */
   if (buscar_siguiente ('(', '(', c, AVANZAR) != 0)
     return (T);

   i = 0;
   do
   {
    /* Obtenemos el coeficiente */
    n = convertir (c);

    /* Avanzamos hasta el operador (o el fin del polinomio) */
    if (buscar_siguiente (')', '-', c, NO_AVANZAR) != 0)
      return (T);

    /* Si encontramos un '*'.. */
    if (*c == '*')
     {
      /* ..buscamos la 's' asociada */
      if (buscar_siguiente ('s', 's', c, AVANZAR) != 0)
	return (T);

      /* Obtenemos el exponente de la 's' */
      j = (short) convertir (c);

      /* Colocamos el coeficiente en el lugar correspondiente */
      if ((j >= 0) && (j <= 10))
	T.den.coef[j] += n;
      else
	error (W_GRADOMAYOR10);

      /* Avanzamos hasta el operador (o el fin del polinomio) */
      if (buscar_siguiente (')', '-', c, NO_AVANZAR) != 0)
	return (T);
     }

    /* pero, si habia un '+' o un ')'.. */
    else

      /* ..colocamos el coeficiente en el lugar 0 */
      T.num.coef[0] += n;

    /* Obtenemos el nuevo signo, si es el caso */
    signo = (*c == '-') ? -1 : 1;
   }
   while ((*c != ')') && (++i < 11));

   /* Detectamos el error de demasiados t‚rminos */
   if (*c != ')')
     error (W_MUCHOSTERMINOS);
  }

 return (T);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

size_t mi_fread (void *ptr, size_t size, size_t n, FILE *stream)

/* Funci¢n que sustituye a 'fread' */
{
 MSG    msg;
 RECT   rect;
 size_t f, i;

 f = fread (ptr, size, n, stream);

 for (i = 0; i < f; ++i)
   if (((char *)ptr)[i] == 10)
    {
     ++nLinea;
     nMaxCol = max (nMaxCol, nCol);
     nCol = 0;

     if (!precalc)
      {
       GetClientRect (hwnd, &rect);
       rect.top    = 7 * cyLine - nVposScrl * SCRLDESPL;
       rect.bottom = rect.top + cyLine - 1;
       InvalidateRect (hwnd, &rect, FALSE);
       UpdateWindow (hwnd);
      }
    }
   else
     ++nCol;

 return (f);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void error (int codigo)

/* Presenta un error en funci¢n del c¢digo recibido */
{
 RECT   rect;
 static char *mensajes[] = { "Nombre de elemento demasiado largo",
			     "Tipo de medida demasiado largo",
			     "Elemento sin conectar",
			     "No se definio nudo de referencia",
			     "Gen. de tension sin impedancia asociada",
			     "Demasiados terminos en el polinomio",
			     "Grado del polinomio mayor que 10",
     /* Primer error ---> */ "Tipo de elemento desconocido",
			     "Caracter inesperado",
			     "Parametro desconocido",
			     "Termino desconocido",
			     "Elemento no localizado",
			     "Fichero no existente",
			     "Especificacion de escala no valida",
			     "Fin de fichero alcanzado",
			     "Tipo de medida desconocido",
			     "Fin de seccion alcanzado",
			     "Definicion duplicada",
			     "Mas de 25 errores" };
 char mensaje[80], aux[6];

 if (errores == ERRORES)
  {
   /* Invalidamos la l¡nea de la ventana que corresponda */
   GetClientRect (hwnd, &rect);
   rect.top    = ((codigo < 25) ? 11 : 9) * cyLine - nVposScrl * SCRLDESPL;
   rect.bottom = rect.top + cyLine - 1;
   InvalidateRect (hwnd, &rect, FALSE);

   /* Si hemos llegado a 25 mensajes, generamos el £ltimo mensaje */
   if (nAvisos + nErrores == 25)
     codigo = 36;

   /* Generamos el mensaje si hay otros 25 mensajes como m ximo */
   if (nAvisos + nErrores <= 25)
    {
     if (codigo < 25)
      {
       strcpy (mensaje, "Aviso\0");
       ++nAvisos;
      }
     else
      {
       strcpy (mensaje, "Error\0");
       ++nErrores;

       codigo -= 18;         /* Eliminar esta l¡nea al completar los Warnings */
      }

     if (Informe)
      {
       strcat (mensaje, " en l¡nea %i, car cter n§ %i : %s\n");
       fprintf (listado, mensaje, nLinea, nCol, mensajes[codigo]);
      }
    }
  }

 return;
}
/*----------------------------Fin del fichero-------------------------------*/

