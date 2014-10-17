#include "capturar.h"
#include "wincapt.h"

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern BOOL FAR PASCAL _export ProcDialogNueva    (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogAbrir    (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogGuardar  (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogAcerca   (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogCaract   (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogNudos    (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogCuad     (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogAddSym   (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogBuscSym  (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogListSym  (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogAddBibl  (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogSuprBibl (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogBloque   (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogPuerto   (HWND, WORD, WORD, LONG);
extern BOOL FAR PASCAL _export ProcDialogTexto    (HWND, WORD, WORD, LONG);

extern BOOL leer_diagrama      (InfoBloque  NEAR *);
extern BOOL guardar_diagrama   (InfoBloque  NEAR *);
extern BOOL leer_esquema       (InfoVentana NEAR *);
extern BOOL guardar_esquema    (InfoVentana NEAR *);
extern BOOL leer_biblioteca    (InfoBiblio  NEAR *);
extern BOOL guardar_biblioteca (InfoBiblio  NEAR *);

/* Todas las funciones 'dibujar' pueden ser globales ya que dibujan sobre el
   contexto de dispositivo que se les pasa en 'hdc' */
/* Dibujos de la ventana de Esquema */
extern void dibujar_circuito   (InfoVentana NEAR *, HDC, RECT);
extern void dibujar_elemento   (InfoVentana NEAR *, HDC,
				short, short, short, short, short, short,
				char [], HANDLE, BOOL);
extern void dibujar_nudo       (InfoVentana NEAR *, HDC, short, short);
extern void dibujar_masa       (InfoVentana NEAR *, HDC, short, short, BOOL);
extern void dibujar_cuadripolo (InfoVentana NEAR *, HDC, short, short, short, short,
						   short, char [], BOOL);
/* Dibujos de la ventana de Diagrama */
extern void dibujar_diagrama   (InfoBloque  NEAR *, HDC, RECT);
extern void dibujar_bloque     (InfoBloque  NEAR *, HDC,
				short, short, short, short, short, short,
				HANDLE, char[], BOOL);
extern void dibujar_puerto     (InfoBloque  NEAR *, HDC,
				short, short, short, short, short, char[], BOOL);
extern void dibujar_texto      (InfoBloque  NEAR *, HDC,
				short, short, short, char[], BOOL);

/* Dibujos comunes a ambas */
extern void dibujar_cable      (void NEAR *, HDC,
				short, short, short, short, short, short, BOOL);
extern void dibujar            (void NEAR *, HDC,
				short, short, short, short, short, HANDLE, BOOL);


/* Dibujos de la ventana de Biblioteca */
extern void  dibujar_biblioteca (InfoBiblio  NEAR *, HDC, RECT);
extern short dibujar_elembibl   (InfoBiblio  NEAR *, HDC, short, short);

/* Dibujos comunes a todas */
extern void dibujar_recuadro   (InfoVentana NEAR *, HDC,
				short, short, short, short, BOOL);
extern void dibujar_rejilla    (HDC, RECT);

/* Funciones asociadas a las clases de ventana */
long FAR PASCAL _export ProcVentApp (HWND, WORD, WORD, LONG);
BOOL FAR PASCAL _export ProcCerrar  (HWND, LONG);
long FAR PASCAL _export ProcVentBlq (HWND, WORD, WORD, LONG);
long FAR PASCAL _export ProcVentSch (HWND, WORD, WORD, LONG);
long FAR PASCAL _export ProcVentBbl (HWND, WORD, WORD, LONG);

/* Estas funciones pueden ser globales ya que emplean la informaci¢n apuntada
   por 'npIV', que debe apuntar a un bloque de datos de ventana de esquema
   previamente creado en memoria local y fijado mediante LocalLock */
int   asignar_memoria_diagrama   (InfoBloque  NEAR *);
int   liberar_memoria_diagrama   (InfoBloque  NEAR *);
int   asignar_memoria_esquema    (InfoVentana NEAR *);
int   liberar_memoria_esquema    (InfoVentana NEAR *);
void  registrar_elemento         (InfoVentana NEAR *, short);
void  registrar_cuadripolo       (InfoVentana NEAR *);
void  registrar_bloque           (InfoBloque  NEAR *);
void  registrar_texto            (InfoBloque  NEAR *);
int   buscar_por_coords          (void NEAR *, short, short, short);
short buscar_texto               (HFILE, BOOL, short);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDefinici¢n de variables globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

HWND  	    	  hwndMarco, hwndCliente, hwndActiva;
HMENU 	    	  hMenuInicial,
		  hMenuEsquema, hMenuEsquemaVentana,
		  hMenuBiblio,  hMenuBiblioVentana,
		  hMenuBloques, hMenuBloquesVentana;
HANDLE 		  hInstGlobal;
short             nBloques, nEsquemas, nBiblios,
		 *bloques, *puertos, *textos,
		 *elementos, *nudos, *cables, *cuad,
		  tipo_ventana, tvactiva;
char              szAppName[] = "Capturador", szBlqName[] = "Bloques",
		  szSchName[] = "Esquema",    szBblName[] = "Biblioteca",
		 *szExtFich[] = { "", ".BLQ", ".SCH", ".BBL",
                 		  ".TM1", ".TM2", ".TM3", ".TM4" },
		  path[80], nom_fich[13];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo del programa principalÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance,
		    LPSTR lpszCmdLine, int nCmdShow)
{
		 /* Definiciones de variables locales */

 WNDCLASS wndclass;
 MSG      msg;
 FILE    *config;
 char     i;
 BOOL	  flag;

 /* Tomamos el path del circuito */
 if (config = fopen ("config.sca", "rt"))
  {
   fscanf (config, "%[^\n]", path);
   fclose (config);
  }
 else
   strcpy (path, PATH);

 /* Fijamos el nombre por defecto */
 strcpy (nom_fich, NOMBRE);

 /* Obtenemos el handle de la aplicaci¢n */
 hInstGlobal = hInstance;

 if (!hPrevInstance)
  {
   /* Registramos la clase de la ventana marco */
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

   /* Registramos la clase de la ventana de Diagrama de Bloques */
   wndclass.lpfnWndProc   = (WNDPROC) ProcVentBlq;
   wndclass.cbWndExtra    = sizeof (LOCALHANDLE);
   wndclass.hIcon         = LoadIcon (hInstance, szBlqName);
   wndclass.hbrBackground = GetStockObject (WHITE_BRUSH);
   wndclass.lpszClassName = szBlqName;

   RegisterClass (&wndclass);

   /* Registramos la clase de la ventana de Esquema */
   wndclass.lpfnWndProc   = (WNDPROC) ProcVentSch;
   wndclass.cbWndExtra    = sizeof (LOCALHANDLE);
   wndclass.hIcon 	  = LoadIcon (hInstance, szSchName);
   wndclass.hbrBackground = GetStockObject (WHITE_BRUSH);
   wndclass.lpszClassName = szSchName;

   RegisterClass (&wndclass);

   /* Registramos la clase de la ventana de Biblioteca */
   wndclass.lpfnWndProc   = (WNDPROC) ProcVentBbl;
   wndclass.cbWndExtra    = sizeof (LOCALHANDLE);
   wndclass.hIcon         = LoadIcon (hInstance, szBblName);
   wndclass.hbrBackground = GetStockObject (LTGRAY_BRUSH);
   wndclass.lpszClassName = szBblName;

   RegisterClass (&wndclass);
  }

 /* Cargamos los men£s de ventana principal, bloques, esquema y biblioteca */
 hMenuInicial = LoadMenu (hInstance, szAppName);
 hMenuBloques = LoadMenu (hInstance, szBlqName);
 hMenuEsquema = LoadMenu (hInstance, szSchName);
 hMenuBiblio  = LoadMenu (hInstance, szBblName);

 /* Obtenemos handles a los submen£s "Ventana" */
 hMenuBloquesVentana = GetSubMenu (hMenuBloques, BLOQUES_SUBMENU_POS);
 hMenuEsquemaVentana = GetSubMenu (hMenuEsquema, ESQUEMA_SUBMENU_POS);
 hMenuBiblioVentana  = GetSubMenu (hMenuBiblio,  BIBLIO_SUBMENU_POS);

 /* Creamos la ventana marco */
 hwndMarco = CreateWindow (szAppName, "Captura de Esquemas",
			   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			   10, 10,
			   GetSystemMetrics(SM_CXSCREEN) - 20,
			   GetSystemMetrics(SM_CYSCREEN) - 100,
			   NULL, NULL, hInstance, NULL);

 /* Obtenemos el handle a la ventana cliente */
 hwndCliente = GetWindow (hwndMarco, GW_CHILD);

 /* Hacemos aparecer la ventana */
 ShowWindow   (hwndMarco, nCmdShow);
 UpdateWindow (hwndMarco);

 /* Estamos en el bucle de mensaje hasta que se escoja 'Salir'  */
 while (GetMessage (&msg, NULL, 0, 0))
 {
  TranslateMessage (&msg);
  DispatchMessage (&msg);
 }

 return (msg.wParam);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcVentApp (HWND hwnd, WORD message,
				     WORD wParam, LONG lParam)
{
 static HANDLE       hInstance, hwndCliente;
 static char         szNameAbrir[]    = "ABRIR",
		     szNameNueva[]    = "NUEVA",
		     szNameAcerca[]   = "ACERCA";
 HWND                hwndMDIActiva, hwndSig;
 HLOCAL              hMemI;
 InfoGeneral NEAR   *npI;
 CLIENTCREATESTRUCT  CreaCliente;
 MDICREATESTRUCT     CreaVentanaMDI;
 OFSTRUCT            of;
 FARPROC      	     lpfnEnum,
		     lpfnProcDialogBox;
 RECT		     rcTam;
 int                 mbFlag;
 char		     nom[84], szBuffer[71];

 switch (message)
 {
  case WM_CREATE:
  {
   /* Obtenemos el handle a la ocurrencia del m¢dulo, que se guarda forever */
   hInstance = ((LPCREATESTRUCT) lParam)->hInstance;

   /* Fijamos los par metros de la ventana cliente MDI */
   CreaCliente.hWindowMenu  = 0;
   CreaCliente.idFirstChild = IDM_PRIMERESQUEMA;

   /* Creamos la ventana cliente MDI */
   hwndCliente = CreateWindow ("MDICLIENT", NULL,
			       WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE |
			       WS_HSCROLL | WS_VSCROLL,
			       0, 0, 0, 0, hwnd, 1,
			       hInstance, (LPSTR) &CreaCliente);

   /* Inicializamos los n£meros de ventanas */
   nBloques = nEsquemas = nBiblios = 0;

   /* Asignamos el mini-men£ a la ventana marco */
   SetMenu (hwnd, hMenuInicial);

   /* Hacemos aparecer la caja de di logo de Acerca de.. */
   PostMessage (hwnd, WM_COMMAND, IDM_ACERCA, 0L);

   return (0);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDM_EMPEZAR:
    case IDM_ABRIR:
    {
     /* Fijamos el tipo de ventana */
     if ((nBloques + nBiblios + nEsquemas) == 0)
       tipo_ventana = TV_BLOQUES;
     else
       tipo_ventana = tvactiva;

     /* Obtenemos la dir. del proc. para la Caja de Di logo de Elegir */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogNueva,
					   hInstance);

     /* Hacemos aparecer la Caja de Di logo de Elegir */
     if (DialogBoxParam (hInstance, szNameNueva, hwnd, lpfnProcDialogBox,
			 MAKELONG(tipo_ventana, 0)))
      {
       /* Si elegimos abrir... */
       if (wParam == IDM_ABRIR)
	{
	 /* ..obtenemos la dir. del proc. para la Caja de Di logo de Abrir */
	 lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogAbrir,
					       hInstance);

	 /* Hacemos aparecer la Caja de Di logo de Abrir */
	 if (DialogBox (hInstance, szNameAbrir, hwnd, lpfnProcDialogBox))
	  {
	   strcpy (szBuffer, path);
	   strcat (szBuffer, nom_fich);
	   if (strlen (szBuffer) > 38)
	     strcpy (nom, "..");
	   strcat (nom, szBuffer + max (0, (int)(strlen (szBuffer) - 38)));
	  }
	 else
	  nom[0] = 0;

	 /* Liberamos el thunk a la Caja de Di logo */
	 FreeProcInstance (lpfnProcDialogBox);
	}
       else
	 strcpy (nom, NOMBRE);

       if (strlen (nom) > 0)
	{
	 /* Obtenemos el tama¤o del  rea de cliente */
	 GetClientRect (hwnd, &rcTam);

	 switch (tipo_ventana)
	 {
	  case TV_BLOQUES:
	  {
	   /* Inicializamos los datos de la nueva ventana de Diagrama de Bloques */
	   CreaVentanaMDI.szClass = szBlqName;
	   CreaVentanaMDI.szTitle = strcat (strcpy (szBuffer, "D. de Bloques - "),
					    nom);
	   CreaVentanaMDI.hOwner  = hInstance;
	   CreaVentanaMDI.x  	= 0.5 * rcTam.right;
	   CreaVentanaMDI.y  	= nBloques * GetSystemMetrics (SM_CYCAPTION);
	   CreaVentanaMDI.cx      = CreaVentanaMDI.cy = 0.5 * rcTam.right;
	   CreaVentanaMDI.style   = 0;
	   CreaVentanaMDI.lParam  = NULL;

	   /* Aumentamos el n§ de ventanas de este tipo */
	   ++nBloques;

	   break;
	  }

	  case TV_ESQUEMA:
	  {
	   /* Inicializamos los datos de la nueva ventana de Esquema */
	   CreaVentanaMDI.szClass = szSchName;
	   CreaVentanaMDI.szTitle = strcat (strcpy (szBuffer, "Esquema - "),
					    nom);
	   CreaVentanaMDI.hOwner  = hInstance;
	   CreaVentanaMDI.x       =
	   CreaVentanaMDI.y       = nEsquemas * GetSystemMetrics (SM_CYCAPTION);
	   CreaVentanaMDI.cx      = 0.8 * rcTam.right;
	   CreaVentanaMDI.cy      = 0.8 * rcTam.bottom;
	   CreaVentanaMDI.style   = 0;
	   CreaVentanaMDI.lParam  = NULL;

	   /* Aumentamos el n§ de ventanas de este tipo */
	   ++nEsquemas;

	   break;
	  }

	  case TV_BIBLIO:
	  {
	   /* Inicializamos los datos de la nueva ventana de Biblioteca */
	   CreaVentanaMDI.szClass = szBblName;
	   CreaVentanaMDI.szTitle = strcat (strcpy (szBuffer, "Biblioteca - "),
					    nom);
	   CreaVentanaMDI.hOwner  = hInstance;
	   CreaVentanaMDI.x       = 0;
	   CreaVentanaMDI.y       = 0.7 * rcTam.bottom -
				    nBiblios * GetSystemMetrics(SM_CYCAPTION);
	   CreaVentanaMDI.cx      =
	   CreaVentanaMDI.cy      = 0.3 * rcTam.bottom;
	   CreaVentanaMDI.style   = 0;
	   CreaVentanaMDI.lParam  = NULL;

	   /* Aumentamos el n§ de ventanas de este tipo */
	   ++nBiblios;

	   break;
	  }
	 }

	 /* Hacemos que la ventana cliente cree la nueva ventana */
	 hwndMDIActiva = SendMessage (hwndCliente, WM_MDICREATE, 0,
				    (LONG)(LPMDICREATESTRUCT) &CreaVentanaMDI);

	 if (wParam == IDM_ABRIR)
	  {
	   /* Obtenemos los datos referentes a la ventana */
	   hMemI = GetWindowWord (hwndMDIActiva, 0);
	   npI   = (InfoGeneral NEAR *) LocalLock (hMemI);
	   strcpy (npI->path, path);
	   strcpy (npI->nom_fich, nom_fich);

	   /* Leemos sus ficheros asociados */
	   if (tipo_ventana == TV_BLOQUES)
	     leer_diagrama ((InfoBloque NEAR *) npI);
	   else
	     if (tipo_ventana == TV_ESQUEMA)
	       leer_esquema ((InfoVentana NEAR *) npI);
	     else
	       leer_biblioteca ((InfoBiblio NEAR *) npI);

	   /* Desbloqueamos los datos referentes a la ventana */
	   LocalUnlock (hMemI);
	  }
	}
      }

     /* Liberamos el thunk a la caja de di logo */
     FreeProcInstance (lpfnProcDialogBox);

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

    case IDM_MOSAICO:
    {
     SendMessage (hwndCliente, WM_MDITILE, 0, 0L);
     return (0);
    }

    case IDM_CASCADA:
    {
     SendMessage (hwndCliente, WM_MDICASCADE, 0, 0L);
     return (0);
    }

    case IDM_ANTERIOR:
    {
     hwndSig = LOWORD(SendMessage (hwndCliente, WM_MDIGETACTIVE, 0, 0L));
     SendMessage (hwndCliente, WM_MDINEXT, hwndSig, 1L);
     return (0);
    }

    case IDM_SIGUIENTE:
    {
     hwndSig = LOWORD(SendMessage (hwndCliente, WM_MDIGETACTIVE, 0, 0L));
     SendMessage (hwndCliente, WM_MDINEXT, hwndSig, 0L);
     return (0);
    }

    case IDM_INDICE:
    {
     break;
    }

    case IDM_CERRAR:
    {
     /* Obtenemos el handle de la ventana actualmente activa */
     hwndMDIActiva = LOWORD (SendMessage (hwndCliente, WM_MDIGETACTIVE, 0, 0L));

     /* Si responde afirmativamente a QUERYENDSESSION, la cerramos */
     if (SendMessage (hwndMDIActiva, WM_QUERYENDSESSION, 0, 0L))
       SendMessage (hwndCliente, WM_MDIDESTROY, hwndMDIActiva, 0L);

     return (0);
    }

    case IDM_TERMINAR:
    {
     /* Creamos una ocurrencia del procedimiento de cierre de los esq. */
     lpfnEnum = MakeProcInstance ((FARPROC) ProcCerrar, hInstance);

     /* Aplicamos dicho procedimiento a los esquemas uno por uno */
     EnumChildWindows (hwndCliente, lpfnEnum, 0L);

     /* Eliminamos la ocurrencia del procedimiento */
     FreeProcInstance (lpfnEnum);

     /* Terminamos la ventana principal */
     DestroyWindow (hwnd);

     return (0);
    }

    default:
    {
     /* Si la ventana marco no procesa el mensaje, se lo mandamos a la activa */
     hwndMDIActiva = LOWORD(SendMessage (hwndCliente, WM_MDIGETACTIVE, 0, 0L));
     if (IsWindow (hwndMDIActiva))
       SendMessage (hwndMDIActiva, WM_COMMAND, wParam, lParam);

     /* Tras mandarlo, hay que pasarlo a DefFrameProc */
     break;
    }
   }

   break;
  }

  case WM_QUERYENDSESSION:
  case WM_CLOSE:
  {
   SendMessage (hwnd, WM_COMMAND, IDM_TERMINAR, 0L);

   if (GetWindow (hwndCliente, GW_CHILD) != NULL)
     return (0);

   break;
  }

  case WM_DESTROY:
  {
   PostQuitMessage (0);
   return (0);
  }
 }

 return (DefFrameProc (hwnd, hwndCliente, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL ProcCerrar (HWND hwnd, LONG lParam)
{
 if (GetWindow (hwnd, GW_OWNER))
   return (TRUE);

 /* Pedimos a la ventana cliente que restaure la ventana de esquema */
 SendMessage (GetParent (hwnd), WM_MDIRESTORE, hwnd, 0L);

 /* Enviamos a la ventana de esquema el mensaje de cierre */
 if (!SendMessage (hwnd, WM_QUERYENDSESSION, 0, 0L))
   return (TRUE);

 SendMessage (GetParent (hwnd), WM_MDIDESTROY, hwnd, 0L);

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcVentBlq (HWND hwnd, WORD message,
				     WORD wParam, LONG lParam)
{
 static HWND       hwndMarco, hwndCliente;
 static HANDLE     hInstance;
 static RECT  	   rcbloque, rcbloque_fijo;
 static char       szNameGuardar[] = "GUARDAR",
		   szNameBloque[]  = "BLOQUE",
		   szNameTexto[]   = "TEXTO",
		   szNamePuerto[]  = "PUERTO";
 static short      tipo_dibujo;
 static BOOL  	   horiz      = TRUE,  vert  = FALSE, espejo   = FALSE,
		   rejilla    = FALSE, pinta = FALSE, empezado = FALSE;
 HMENU        	   hMenu;
 HDC               hdc;
 HPEN         	   hPen, hViejoPen;
 HANDLE            hMF;
 HCURSOR           hCursor;
 PAINTSTRUCT       ps;
 OFSTRUCT          of;
 FARPROC      	   lpfnProcDialogBox;
 RECT              rect;
 TEXTMETRIC        tm;
 LOCALHANDLE       hMemIB;
 InfoBloque  NEAR *npIB;
 bloque            datos_bloque;
 texto             datos_texto;
 puerto            datos_puerto;
 char         	   nom[84], szBuffer[70], *txt;
 int          	   hFichero, resp;
 short 		   i, j, k, nIncScrl, nDesplx, nDesply, elem[6];

 switch (message)
 {
  case WM_CREATE:
  {
  /* Obtenemos el handle a la ocurrencia del m¢dulo, que se guarda forever */
   hInstance = ((LPCREATESTRUCT) lParam)->hInstance;

   /* Obtenemos handles a las ventanas padre y abuela */
   hwndCliente = GetParent (hwnd);
   hwndMarco   = GetParent (hwndCliente);

   /* Inicializamos los datos comunes a todas las ventanas */
   pinta    = FALSE;
   empezado = FALSE;

   /* Fijamos las checkmarks del men£ de Esquema */
   CheckMenuItem (hMenuBloques, IDM_HORIZ,   MF_BYCOMMAND | MF_CHECKED);
   CheckMenuItem (hMenuBloques, IDM_VERT,    MF_BYCOMMAND | MF_UNCHECKED);
   CheckMenuItem (hMenuBloques, IDM_ESPEJO,  MF_BYCOMMAND | MF_UNCHECKED);
   CheckMenuItem (hMenuBloques, IDM_REJILLA, MF_BYCOMMAND | MF_UNCHECKED);

   /* Inicializamos la estructura de datos de la ventana de diagrama */
   hMemIB = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (InfoBloque));
   npIB = (InfoBloque NEAR *) LocalLock (hMemIB);
   strcpy (npIB->path, path);
   strcpy (npIB->nom_fich, NOMBRE);
   asignar_memoria_diagrama (npIB);
   LocalUnlock (hMemIB);

   /* Transferimos dichos datos a la ventana */
   SetWindowWord (hwnd, 0, hMemIB);

   return (0);
  }

  case WM_SIZE:
  {
   /* Obtenemos los datos referentes a la ventana de esquema */
   hMemIB = GetWindowWord (hwnd, 0);
   npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

   /* Si la ventana est  siendo minimizada, activamos el flag */
   if (wParam == SIZE_MINIMIZED)
     npIB->minimizada = TRUE;

   /* pero, si no,.. */
   else
    {
     /* ..desactivamos el flag */
     npIB->minimizada = FALSE;

     /* Fijamos los rangos y posiciones de las barras de Scroll */
     npIB->nVmaxScrl = max (0, 255 - (short) HIWORD(lParam) / SCRLDESPL);
     npIB->nVposScrl = min (npIB->nVposScrl, npIB->nVmaxScrl);
     SetScrollRange (hwnd, SB_VERT, 0, npIB->nVmaxScrl, FALSE);
     SetScrollPos   (hwnd, SB_VERT, npIB->nVposScrl, TRUE);

     npIB->nHmaxScrl = max (0, 255 - (short) LOWORD(lParam) / SCRLDESPL);
     npIB->nHposScrl = min (npIB->nHposScrl, npIB->nHmaxScrl);
     SetScrollRange (hwnd, SB_HORZ, 0, npIB->nHmaxScrl, FALSE);
     SetScrollPos   (hwnd, SB_HORZ, npIB->nHposScrl, TRUE);
    }

   /* Desbloqueamos los datos */
   LocalUnlock (hMemIB);

   return (0);
  }

  case WM_MDIACTIVATE:
  {
   /* Si obtenemos el foco de entrada ponemos el men£ Diagramas */
   if (wParam == TRUE)
    {
     hwndActiva = hwnd;
     tvactiva   = TV_BLOQUES;
     SendMessage (hwndCliente, WM_MDISETMENU, 0,
		  MAKELONG(hMenuBloques, hMenuBloquesVentana));
    }

   /* pero, si perdemos el foco, ponemos el men£ Inicial */
   else
     SendMessage (hwndCliente, WM_MDISETMENU, 0, MAKELONG(hMenuInicial, 0));

   DrawMenuBar (hwndMarco);
   return (0);
  }

  case WM_PAINT:
  {
   /* Obtenemos el contexto de la ventana */
   hdc = BeginPaint (hwnd, &ps);

   /* Redibujamos lo que haga falta */
   hMemIB = GetWindowWord (hwnd, 0);
   npIB = (InfoBloque NEAR *) LocalLock (hMemIB);
   dibujar_diagrama (npIB, hdc, ps.rcPaint);
   if (rejilla)
     dibujar_rejilla (hdc, ps.rcPaint);
   LocalUnlock (hMemIB);

   /* Liberamos el contexto */
   EndPaint (hwnd, &ps);
  }

  case WM_LBUTTONUP:
  {
   /* Obtenemos los datos referentes a la ventana */
   hMemIB = GetWindowWord (hwnd, 0);
   npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

   nDesplx = LOWORD(lParam) + npIB->nHposScrl * SCRLDESPL;
   nDesply = HIWORD(lParam) + npIB->nVposScrl * SCRLDESPL;

   if (rejilla)
    {
     nDesplx = (nDesplx / TAMREJ + (nDesplx % TAMREJ >= 2)) * TAMREJ;
     nDesply = (nDesply / TAMREJ + (nDesply % TAMREJ >= 2)) * TAMREJ;
    }

   switch (tipo_dibujo)
   {
    case 0:
    {
     /* Buscamos un bloque en las coordenadas actuales */
     if ((i = buscar_por_coords (npIB, BLOQUE, nDesplx, nDesply)) >= 0)
      {
       /* Le dibujamos un recuadro al bloque y salvamos sus datos */
       hdc = GetDC (hwnd);
       bloques = (short *) LocalLock (npIB->hmemBloques);
       memcpy (elem, bloques + 4*i, 4*sizeof (short));
       dibujar_recuadro ((void NEAR *) npIB, hdc,
			 elem[0] - 5, elem[1] - 5,
			 elem[2] - elem[0] + 10, elem[3] - elem[1] + 10, FALSE);
       LocalUnlock (npIB->hmemBloques);
       ReleaseDC (hwnd, hdc);

       /* Obtenemos la dir. del proc. para la Caja de Di logo de Caracter. */
       lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogBloque,
					     hInstance);

       /* Si la caja de di logo termin¢ con Cancelar.. */
       if (!DialogBoxParam (hInstance, szNameBloque, hwnd, lpfnProcDialogBox,
			    MAKELONG ((WORD) i, (WORD) npIB)))
	{
	 /* ..borramos el recuadro al bloque */
	 hdc = GetDC (hwnd);
	 dibujar_recuadro ((void NEAR *) npIB, hdc,
			   elem[0] - 5, elem[1] - 5,
			   elem[2] - elem[0] + 10, elem[3] - elem[1] + 10, FALSE);
	 ReleaseDC (hwnd, hdc);

	 /* Restauramos los datos del bloque */
	 bloques = (short *) LocalLock (npIB->hmemBloques);
	 memcpy (bloques + 4*i, elem, 4*sizeof (short));
	 LocalUnlock (npIB->hmemBloques);
	}

       /* pero, si termin¢ con OK.. */
       else
	{
	 /* ..borramos el recuadro al bloque */
	 hdc = GetDC (hwnd);
	 dibujar_recuadro ((void NEAR *) npIB, hdc,
			   elem[0] - 5, elem[1] - 5,
			   elem[2] - elem[0] + 10, elem[3] - elem[1] + 10, FALSE);

	 /* Obtenemos los datos del bloque */
	 strcpy (nom, npIB->path);
	 strcat (nom, npIB->nom_fich);
	 hFichero = OpenFile (strcat (nom, szExtFich[TMPBLOQUES]), &of, OF_READ);
	 _llseek (hFichero, i * sizeof (datos_bloque), 0);
	 _lread (hFichero, (LPSTR) &datos_bloque, sizeof (datos_bloque));
	 _lclose (hFichero);

	 /* Obtenemos el MetaFile si es el caso */
	 if (strlen (datos_bloque.metafile) > 0)
	   hMF = GetMetaFile (datos_bloque.metafile);
	 else
	   hMF = NULL;

	 /* Dibujamos el nuevo */
	 bloques = (short *) LocalLock (npIB->hmemBloques);
	 dibujar_bloque (npIB, hdc, b(i,0), b(i,1), b(i,2), b(i,3),
			 datos_bloque.alinh, datos_bloque.alinv,
			 hMF, datos_bloque.texto, TRUE);
	 LocalUnlock (npIB->hmemBloques);

	 /* Eliminamos el MetaFile */
	 if (hMF != NULL)
	   DeleteMetaFile (hMF);

	 ReleaseDC (hwnd, hdc);
	}

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialogBox);
      }

     /* Si no hay bloque, buscamos un texto */
     else
       if ((i = buscar_por_coords (npIB, TEXTO, nDesplx, nDesply)) >= 0)
	{
	 /* Le dibujamos un recuadro al texto y salvamos sus datos */
	 hdc = GetDC (hwnd);
	 GetTextMetrics (hdc, &tm);
	 textos = (short *) LocalLock (npIB->hmemTextos);
	 memcpy (elem, textos + 5*i, 5 * sizeof (short));
	 dibujar_recuadro ((void NEAR *) npIB, hdc,
			   elem[0] - 1, elem[1] - 1,
			   elem[4]*tm.tmAveCharWidth + 2, tm.tmHeight + 2,
			   FALSE);
	 LocalUnlock (npIB->hmemTextos);
	 ReleaseDC (hwnd, hdc);

	 /* Obtenemos la dir. del proc. para la Caja de Di logo de Caracter. */
	 lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogTexto,
					       hInstance);

	 /* Si la caja de di logo termin¢ con Cancelar.. */
	 if (!DialogBoxParam (hInstance, szNameTexto, hwnd, lpfnProcDialogBox,
			      MAKELONG ((WORD) i, (WORD) npIB)))
	  {
	   /* ..borramos el recuadro al texto */
	   hdc = GetDC (hwnd);
	   GetTextMetrics (hdc, &tm);
	   dibujar_recuadro ((void NEAR *)npIB, hdc,
			     elem[0]-1, elem[1]-1,
			     elem[4]*tm.tmAveCharWidth, tm.tmHeight, FALSE);
	   ReleaseDC (hwnd, hdc);

	   /* Restauramos los datos del texto */
	   textos = (short *) LocalLock (npIB->hmemTextos);
	   memcpy (textos + 5*i, elem, 5*sizeof (short));
	   LocalUnlock (npIB->hmemTextos);
	  }

	 /* pero, si termin¢ con OK.. */
	 else
	  {
	   /* ..borramos el recuadro al texto */
	   hdc = GetDC (hwnd);
	   GetTextMetrics (hdc, &tm);
	   dibujar_recuadro ((void NEAR *) npIB, hdc,
			     elem[0] - 1, elem[1] - 1,
			     elem[4]*tm.tmAveCharWidth + 2, tm.tmHeight + 2,
			     FALSE);

	   /* Obtenemos el texto del fichero */
	   strcpy (nom, npIB->path);
	   strcat (nom, npIB->nom_fich);
	   hFichero = OpenFile (strcat (nom, szExtFich[TMPTEXTOS]), &of, OF_READ);
	   j = buscar_texto (hFichero, TRUE, i);
	   txt = (char *) malloc (j);
	   _lread (hFichero, txt, j);
	   _lclose (hFichero);

	   /* Dibujamos el nuevo */
	   textos = (short *) LocalLock (npIB->hmemTextos);
	   dibujar_texto (npIB, hdc, t(i,0), t(i,1), t(i,2), txt, TRUE);
	   LocalUnlock (npIB->hmemTextos);
	   free (txt);
	   ReleaseDC (hwnd, hdc);
	  }

	 /* Liberamos el thunk a la Caja de Di logo */
	 FreeProcInstance (lpfnProcDialogBox);
	}

       /* Y si no hay texto, buscamos un puerto */
       else
	 if ((i = buscar_por_coords (npIB, PUERTO, nDesplx, nDesply)) >= 0)
	  {
	   /* Le dibujamos un recuadro al puerto y salvamos sus datos */
	   hdc = GetDC (hwnd);
	   GetTextMetrics (hdc, &tm);
	   puertos = (short *) LocalLock (npIB->hmemPuertos);
	   memcpy (&datos_puerto, puertos + 11*i, 11 * sizeof (short));
	   dibujar_recuadro ((void NEAR *) npIB, hdc,
	   		     datos_puerto.x-1, datos_puerto.y-1,
			     datos_puerto.horiz *
			     (strlen(datos_puerto.nombre) * tm.tmAveCharWidth + 5) +
			     datos_puerto.vert  * (tm.tmMaxCharWidth + 4),
			     datos_puerto.horiz * (tm.tmHeight + 4) +
			     datos_puerto.vert  *
			     (strlen(datos_puerto.nombre) * tm.tmHeight + 5),
			     FALSE);
	   LocalUnlock (npIB->hmemPuertos);
	   ReleaseDC (hwnd, hdc);

	   /* Obtenemos la dir. del proc. para la Caja de Di logo de Caracter. */
	   lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogPuerto,
						 hInstance);

	   /* Si la caja de di logo termin¢ con Cancelar.. */
	   if (!DialogBoxParam (hInstance, szNamePuerto, hwnd, lpfnProcDialogBox,
				MAKELONG ((WORD) i, (WORD) npIB)))
	    {
	     /* ..borramos el recuadro al texto */
	     hdc = GetDC (hwnd);
	     GetTextMetrics (hdc, &tm);
	     dibujar_recuadro ((void NEAR *) npIB, hdc,
	     		       datos_puerto.x-1, datos_puerto.y-1,
			       datos_puerto.horiz *
			       (strlen(datos_puerto.nombre) * tm.tmAveCharWidth + 5) +
			       datos_puerto.vert  * (tm.tmMaxCharWidth + 4),
			       datos_puerto.horiz * (tm.tmHeight + 4) +
			       datos_puerto.vert  *
			       (strlen(datos_puerto.nombre) * tm.tmHeight + 5),
			       FALSE);
	     ReleaseDC (hwnd, hdc);

	     /* Restauramos los datos del puerto */
	     puertos = (short *) LocalLock (npIB->hmemPuertos);
	     memcpy (puertos + 11*i, &datos_puerto, 11 * sizeof (short));
	     LocalUnlock (npIB->hmemPuertos);
	    }

	   /* pero, si termin¢ con OK.. */
	   else
	    {
	     /* ..borramos el recuadro al puerto */
	     hdc = GetDC (hwnd);
	     GetTextMetrics (hdc, &tm);
	     dibujar_recuadro ((void NEAR *) npIB, hdc,
	     		       datos_puerto.x-1, datos_puerto.y-1,
			       datos_puerto.horiz *
			       (strlen(datos_puerto.nombre) * tm.tmAveCharWidth + 5) +
			       datos_puerto.vert  * (tm.tmMaxCharWidth + 4),
			       datos_puerto.horiz * (tm.tmHeight + 4) +
			       datos_puerto.vert  *
			       (strlen(datos_puerto.nombre) * tm.tmHeight + 5),
			       FALSE);
	     ReleaseDC (hwnd, hdc);

	     /* Dibujamos el nuevo */
	     puertos = (short *) LocalLock (npIB->hmemPuertos);
	     dibujar_puerto (npIB, hdc, p(i,0), p(i,1), p(i,2), p(i,3), p(i,4),
	     		     szBuffer, TRUE);
	     LocalUnlock (npIB->hmemTextos);
	     ReleaseDC (hwnd, hdc);
	    }

	   /* Liberamos el thunk a la Caja de Di logo */
	   FreeProcInstance (lpfnProcDialogBox);
	  }

     return (0);
    }

    case CABLE:
    {
     hdc = GetDC (hwnd);
     cables = (short *) LocalLock (npIB->hmemCables);

     empezado = 1 - empezado;
     if (empezado)
      {
       c(npIB->n_cables, 0) = nDesplx;
       c(npIB->n_cables, 1) = nDesply;
       c(npIB->n_cables, 2) = c(npIB->n_cables, 0) + 1;
       c(npIB->n_cables, 3) = c(npIB->n_cables, 1) + 1;
       dibujar_cable (npIB, hdc, c(npIB->n_cables,0), c(npIB->n_cables,1),
		      c(npIB->n_cables,2) - c(npIB->n_cables,0),
		      c(npIB->n_cables,3) - c(npIB->n_cables,1),
		      horiz, vert, FALSE);
      }
     else
      {
       c(npIB->n_cables, 2) = nDesplx;
       c(npIB->n_cables, 3) = nDesply;
       c(npIB->n_cables, 4) = horiz;
       c(npIB->n_cables, 5) = vert;
       dibujar_cable (npIB, hdc, c(npIB->n_cables,0), c(npIB->n_cables,1),
		      c(npIB->n_cables,2) - c(npIB->n_cables,0),
		      c(npIB->n_cables,3) - c(npIB->n_cables,1),
		      horiz, vert, TRUE);
       ++npIB->n_cables;
      }
     LocalUnlock (npIB->hmemCables);
     ReleaseDC (hwnd, hdc);
     break;
    }

    case BORRAR:
    {
     /* Obtenemos un contexto */
     hdc = GetDC (hwnd);

     /* Le asignamos un l piz del color del fondo */
     hPen = CreatePen (PS_SOLID, 1, RGB(255,255,255));
     hViejoPen = SelectObject (hdc, hPen);

     /* Buscamos jer rquicamente cable, texto, puerto y bloque */
     if ((i = buscar_por_coords (npIB, CABLE, nDesplx, nDesply)) >= 0)
      {
       --npIB->n_cables;
       cables = (short *) LocalLock (npIB->hmemCables);

       for (j = 0; j < 6; ++j)
	{
	 k = c(i,j);
	 c(i,j) = c(npIB->n_cables,j);
	 c(npIB->n_cables, j) = k;
	}

       dibujar (npIB, hdc, CABLE, c(npIB->n_cables,4), c(npIB->n_cables,5),
		0, 1, NULL, TRUE);
       LocalUnlock (npIB->hmemCables);
      }
     else
       if ((i = buscar_por_coords (npIB, TEXTO, nDesplx, nDesply)) >= 0)
	{
	 /* Reducimos el n£mero de textos */
	 --npIB->n_textos;

	 /* Intercambiamos el elemento a eliminar por el £ltimo de la
	    lista, evit ndonos as¡ reordenaciones */
	 textos = (short *) LocalLock (npIB->hmemTextos);

	 for (j = 0; j < 5; ++j)
	  {
	   k = t(i,j);
	   t(i,j) = t(npIB->n_textos,j);
	   t(npIB->n_textos, j) = k;
	  }

	 /* Borramos el texto */
	 dibujar_texto (npIB, hdc, t(npIB->n_textos,0), t(npIB->n_textos,1),
				   t(npIB->n_textos,2), szBuffer, TRUE);
	 LocalUnlock (npIB->hmemTextos);

	 /* Idem con sus datos almacenados en el fichero temporal */
	 strcpy (nom, npIB->path);
	 strcat (nom, npIB->nom_fich);
	 strcat (nom, szExtFich[TMPTEXTOS]);
	 hFichero = OpenFile (nom, &of, OF_READWRITE | OF_SHARE_EXCLUSIVE);

	 _llseek (hFichero, npIB->n_textos * sizeof (datos_texto), 0);
	 _lread  (hFichero, (LPSTR) &datos_texto, sizeof (datos_texto));
	 _llseek (hFichero, i * sizeof (datos_texto), 0);
	 _lwrite (hFichero, (LPSTR) &datos_texto, sizeof (datos_texto));

	 _lclose (hFichero);
	}
       else
	 if ((i = buscar_por_coords (npIB, PUERTO, nDesplx, nDesply)) >= 0)
	  {
	   /* Reducimos el n£mero de puertos */
	   --npIB->n_puertos;

	   /* Intercambiamos el puerto a eliminar por el £ltimo de la
	      lista, evit ndonos as¡ reordenaciones */
	   puertos = (short *) LocalLock (npIB->hmemPuertos);

	   for (j = 0; j < 11; ++j)
	    {
	     k = p(i,j);
	     p(i,j) = p(npIB->n_puertos,j);
	     p(npIB->n_puertos, j) = k;
	    }

	   /* Borramos el puerto */
	   dibujar_puerto (npIB, hdc,
			   p(npIB->n_puertos,0), p(npIB->n_puertos,1),
			   p(npIB->n_puertos,2), p(npIB->n_puertos,3),
			   p(npIB->n_puertos,4), "          ", TRUE);
	   LocalUnlock (npIB->hmemPuertos);
	  }
	 else
	   if ((i = buscar_por_coords (npIB, BLOQUE, nDesplx, nDesply)) >= 0)
	    {
	     /* Reducimos el n£mero de bloques */
	     --npIB->n_bloques;

	     /* Intercambiamos el elemento a eliminar por el £ltimo de la
		lista, evit ndonos as¡ reordenaciones */
	     bloques = (short *) LocalLock (npIB->hmemBloques);

	     for (j = 0; j < 4; ++j)
	      {
	       k = b(i,j);
	       b(i,j) = b(npIB->n_bloques,j);
	       b(npIB->n_bloques, j) = k;
	      }

	     /* Borramos el bloque */
	     dibujar_bloque (npIB, hdc,
			     b(npIB->n_bloques,0), b(npIB->n_bloques,1),
			     b(npIB->n_bloques,2), b(npIB->n_bloques,3),
			     DT_CENTER, DT_CENTER, NULL, "", TRUE);
	     LocalUnlock (npIB->hmemBloques);

	     /* Idem con sus datos almacenados en el fichero temporal */
	     strcpy (nom, npIB->path);
	     strcat (nom, npIB->nom_fich);
	     strcat (nom, szExtFich[TMPBLOQUES]);
	     hFichero = OpenFile (nom, &of, OF_READWRITE | OF_SHARE_EXCLUSIVE);

	     _llseek (hFichero, npIB->n_bloques * sizeof (datos_bloque), 0);
	     _lread  (hFichero, (LPSTR) &datos_bloque, sizeof (datos_bloque));
	     _llseek (hFichero, i * sizeof (datos_bloque), 0);
	     _lwrite (hFichero, (LPSTR) &datos_bloque, sizeof (datos_bloque));

	     _lclose (hFichero);
	    }

     /* Reseleccionamos el l piz anterior */
     SelectObject (hdc, hViejoPen);
     DeleteObject (hPen);

     /* Liberamos el contexto de dispositivo */
     ReleaseDC (hwnd, hdc);
     break;
    }

    case BLOQUE:
    case GRUPO:
    case MOVER:
    case DESPLAZAR:
    {
     hdc = GetDC (hwnd);

     /* Si a£n no est  empezado.. */
     if (!empezado)
      {
       /* ..empezamos el bloque */
       empezado = 1;

       /* Obtenemos las coordenadas del bloque y lo dibujamos */
       rcbloque.left = rcbloque.right  = nDesplx;
       rcbloque.top  = rcbloque.bottom = nDesply;
       dibujar_recuadro ((void NEAR *) npIB, hdc,
			 rcbloque.left, rcbloque.top,
			 rcbloque.right  - rcbloque.left,
			 rcbloque.bottom - rcbloque.top, FALSE);
      }

     /* pero, si ya est  empezado.. */
     else
      {
       /* ..ordenamos las coordenadas del bloque */
       rect = rcbloque;
       rcbloque.left   = min (rect.left, rect.right);
       rcbloque.right  = max (rect.left, rect.right);
       rcbloque.top    = min (rect.top,  rect.bottom);
       rcbloque.bottom = max (rect.top,  rect.bottom);

       /* Si estamos borrando ¢ moviendo (no seleccionando) un grupo.. */
       if ((tipo_dibujo == GRUPO) || (empezado == 2))
	{
	 /* ..registramos que hemos terminado el bloque */
	 empezado = 0;

	 /* Si estamos moviendo un bloque.. */
	 if ((tipo_dibujo == MOVER) || (tipo_dibujo == DESPLAZAR))
	  {
	   /* ..obtenemos el desplazamiento */
	   nDesplx = rcbloque.left - rcbloque_fijo.left;
	   nDesply = rcbloque.top  - rcbloque_fijo.top;

	   /* Y cambiamos al bloque que se se¤al¢ en un principio, y no al
	      se¤alado como destino, puesto que ‚ste £ltimo no contiene
	      objeto alguno */
	   rect          = rcbloque;
	   rcbloque      = rcbloque_fijo;
	   rcbloque_fijo = rect;
	  }

	 /* Borramos ¢ movemos bloques */
	 bloques = (short *) LocalLock (npIB->hmemBloques);

	 /* Abrimos el fichero temporal de bloques */
	 strcpy (nom, npIB->path);
	 strcat (nom, npIB->nom_fich);
	 strcat (nom, szExtFich[TMPBLOQUES]);
	 hFichero = OpenFile (nom, &of, OF_READWRITE | OF_SHARE_EXCLUSIVE);

	 for (i = 0; i < npIB->n_bloques; ++i)
	  {
	   if (tipo_dibujo == GRUPO)
	    {
	     while ((b(i,0) > rcbloque.left) && (b(i,2) < rcbloque.right)  &&
		    (b(i,1) > rcbloque.top)  && (b(i,3) < rcbloque.bottom) &&
		    (i < npIB->n_bloques))
	      {
	       --npIB->n_bloques;
	       memcpy (bloques + 4*i, bloques + 4*npIB->n_bloques,
		       4 * sizeof (short));

	       /* Sustitu¡r los datos del bloque a borrar por los del
		  £ltimo bloque, tambi‚n en el fichero temporal */
	       _llseek (hFichero, npIB->n_bloques  * sizeof (datos_bloque), 0);
	       _lread  (hFichero, (LPSTR) &datos_bloque, sizeof (datos_bloque));
	       _llseek (hFichero, i * sizeof (datos_bloque), 0);
	       _lwrite (hFichero, (LPSTR) &datos_bloque, sizeof (datos_bloque));
	      }
	    }
	   else /* tipo_dibujo == MOVER ¢ tipo_dibujo == DESPLAZAR */
	    {
	     if ((b(i,0) > rcbloque.left) && (b(i,2) < rcbloque.right)  &&
		 (b(i,1) > rcbloque.top)  && (b(i,3) < rcbloque.bottom))
	      {
	       b(i,0) += nDesplx;
	       b(i,1) += nDesply;
	      }
	    }
	  }
	 LocalUnlock (npIB->hmemBloques);

	 /* Cerramos el fichero temporal de bloques */
	 _lclose (hFichero);

	 /* Borramos ¢ movemos cables */
	 cables = (short *) LocalLock (npIB->hmemCables);
	 k = 0;
	 for (i = 0; i < npIB->n_cables; ++i)
	  {
	   if (tipo_dibujo == GRUPO)
	    {
	     while ((rcbloque.left   < min (c(i,0), c(i,2))) &&
		    (rcbloque.right  > max (c(i,0), c(i,2))) &&
		    (rcbloque.top    < min (c(i,1), c(i,3))) &&
		    (rcbloque.bottom > max (c(i,1), c(i,3))) &&
		    (i < npIB->n_cables))
	      {
	       --npIB->n_cables;
	       memcpy (cables + 6*i, cables + 6*npIB->n_cables, 6*sizeof(short));
	      }
	    }
	   else
	    {
	     if ((rcbloque.left   < min (c(i,0), c(i,2))) &&
		 (rcbloque.right  > max (c(i,0), c(i,2))) &&
		 (rcbloque.top    < min (c(i,1), c(i,3))) &&
		 (rcbloque.bottom > max (c(i,1), c(i,3))))
	      {
	       /* Si estamos Desplazando, creamos un cable entre las
		  coordenadas anteriores y las actuales */
	       if (tipo_dibujo == DESPLAZAR)
		{
		 /* Si el desplazamiento es a la derecha, el nuevo cable une
		    el extremo izquierdo del cable desplazado con su anterior
		    ubicaci¢n. Si es a la izquierda, se une el extremo dcho.
		    del cable. Igual para desplazamientos arriba ¢ abajo */
		 c(npIB->n_cables + k,0) = (nDesplx > 0) ? min (c(i,0), c(i,2)) :
							   max (c(i,0), c(i,2));
		 c(npIB->n_cables + k,1) = (nDesply > 0) ? min (c(i,1), c(i,3)) :
							   max (c(i,1), c(i,3));
		 c(npIB->n_cables + k,2) = c(npIB->n_cables + k,0) + nDesplx;
		 c(npIB->n_cables + k,3) = c(npIB->n_cables + k,1) + nDesply;
		 c(npIB->n_cables + k,4) = horiz;
		 c(npIB->n_cables + k,5) = vert;
		 ++k;
		}

	       c(i,0) += nDesplx;
	       c(i,2) += nDesplx;
	       c(i,1) += nDesply;
	       c(i,3) += nDesply;
	      }
	    }
	  }
	 npIB->n_cables += k;
	 LocalUnlock (npIB->hmemCables);

	 /* Borrar todo lo que caiga dentro del/los bloque/s */
	 if ((tipo_dibujo == MOVER) || (tipo_dibujo == DESPLAZAR))
	  {
	   rcbloque_fijo.left   -= npIB->nHposScrl * SCRLDESPL;
	   rcbloque_fijo.right  -= npIB->nHposScrl * SCRLDESPL - 1;
	   rcbloque_fijo.top    -= npIB->nVposScrl * SCRLDESPL;
	   rcbloque_fijo.bottom -= npIB->nVposScrl * SCRLDESPL - 1;
	   InvalidateRect (hwnd, &rcbloque_fijo, TRUE);
	  }

	 rcbloque.left   -= npIB->nHposScrl * SCRLDESPL;
	 rcbloque.right  -= npIB->nHposScrl * SCRLDESPL - 1;
	 rcbloque.top    -= npIB->nVposScrl * SCRLDESPL;
	 rcbloque.bottom -= npIB->nVposScrl * SCRLDESPL - 1;
	 InvalidateRect (hwnd, &rcbloque, TRUE);

	 UpdateWindow (hwnd);
	}

       /* pero, si estamos dibujando un bloque.. */
       else
	 if (tipo_dibujo == BLOQUE)
	  {
	   /* ..registramos que hemos terminado el bloque */
	   empezado = 0;

	   /* Guardamos los datos en la lista */
	   bloques = (short *) LocalLock (npIB->hmemBloques);

	   b(npIB->n_bloques,0) = rcbloque.left;
	   b(npIB->n_bloques,1) = rcbloque.top;
	   b(npIB->n_bloques,2) = rcbloque.right;
	   b(npIB->n_bloques,3) = rcbloque.bottom;
	   b(npIB->n_bloques,4) = horiz;

	   /* Dibujamos el bloque */
	   dibujar_bloque ((void NEAR *) npIB, hdc,
			   rcbloque.left, rcbloque.top,
			   rcbloque.right, rcbloque.bottom,
			   DT_CENTER, DT_CENTER, NULL, "", TRUE);

	   /* Registramos el bloque en el fichero temporal */
	   registrar_bloque (npIB);
	   ++npIB->n_bloques;
	  }

	 /* y si, finalmente, estamos seleccionando un grupo.. */
	 else
	  {
	   /* ..registramos que hemos terminado de seleccionar */
	   empezado = 2;

	   /* Guardamos el bloque seleccionado */
	   rcbloque_fijo = rcbloque;
	  }
      }

     /* Liberamos el contexto de dispositivo */
     ReleaseDC (hwnd, hdc);

     break;
    }
   }

   /* Desbloqueamos los datos de la ventana de Diagrama */
   LocalUnlock (hMemIB);

   return (0);
  }

  case WM_MOUSEMOVE:
  {
   /* Reaccionar s¢lo si es la ventana actualmente activa */
   if (hwnd == hwndActiva)
    {
     /* Obtenemos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Si no est  minimimizada.. */
     if (!npIB->minimizada)
      {
       /* ..obtenemos la posici¢n del cursor */
       nDesplx = LOWORD(lParam) + npIB->nHposScrl * SCRLDESPL;
       nDesply = HIWORD(lParam) + npIB->nVposScrl * SCRLDESPL;

       if (rejilla)
	{
	 nDesplx = (nDesplx / TAMREJ + (nDesplx % TAMREJ >= 2)) * TAMREJ;
	 nDesply = (nDesply / TAMREJ + (nDesply % TAMREJ >= 2)) * TAMREJ;
	}

       /* Empezamos la pintada */
       hdc = GetDC (hwnd);

       /* Borramos el elemento antiguo */
       dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

       switch (tipo_dibujo)
       {
	case CABLE:
	{
	 if (empezado)
	  {
	   /* Obtenemos las nuevas coordenadas */
	   cables = (short *) LocalLock (npIB->hmemCables);
	   c(npIB->n_cables, 2) = nDesplx;
	   c(npIB->n_cables, 3) = nDesply;
	   LocalUnlock (npIB->hmemCables);
	  }
	 break;
	}

	case BLOQUE:
	case GRUPO:
	case MOVER:
	case DESPLAZAR:
	{
	 if (empezado)
	  {
	   /* Si es un bloque, borramos su representaci¢n anterior */
	   if (tipo_dibujo == BLOQUE)
	     dibujar_bloque ((void NEAR *) npIB, hdc,
			     rcbloque.left,  rcbloque.top,
			     rcbloque.right, rcbloque.bottom,
			     DT_CENTER, DT_CENTER, NULL, "", FALSE);

	   /* pero si no lo es, borramos el recuadro anterior */
	   else
	     dibujar_recuadro ((void NEAR *) npIB, hdc,
			       rcbloque.left, rcbloque.top,
			       rcbloque.right  - rcbloque.left,
			       rcbloque.bottom - rcbloque.top, FALSE);

	   /* Obtenemos las nuevas coordenadas */
	   if (((tipo_dibujo == MOVER) || (tipo_dibujo == DESPLAZAR)) &&
	       (empezado == 2))
	    {
	     rcbloque.left  += nDesplx - rcbloque.right;
	     rcbloque.top   += nDesply - rcbloque.bottom;
	    }
	   rcbloque.right  = nDesplx;
	   rcbloque.bottom = nDesply;

	   /* Si es un bloque, dibujamos la nueva representaci¢n */
	   if (tipo_dibujo == BLOQUE)
	     dibujar_bloque ((void NEAR *) npIB, hdc,
			     rcbloque.left,  rcbloque.top,
			     rcbloque.right, rcbloque.bottom,
			     DT_CENTER, DT_CENTER, NULL, "", FALSE);

	   /* pero si no lo es, dibujamos el nuevo recuadro */
	   else
	     dibujar_recuadro ((void NEAR *) npIB, hdc,
			       rcbloque.left, rcbloque.top,
			       rcbloque.right  - rcbloque.left,
			       rcbloque.bottom - rcbloque.top, FALSE);
	  }
	 break;
	}

	case PUERTO:
	{
	 break;
	}

	case TEXTO:
	{
	 break;
	}
       }

       /* Pintamos el elemento nuevo */
       dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

       /* Liberamos el contexto */
       ReleaseDC (hwnd, hdc);

       return (0);
      }

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);
    }

   break;
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDM_GUARDAR:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Almacenamos el fichero de esquema */
     guardar_diagrama (npIB);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     return (0);
    }

    case IDM_GUARDARCOMO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Fijamos el tipo de ventana a guardar */
     tipo_ventana = TV_BLOQUES;

     /* Obtenemos la dir. del proc. para la Caja de Di logo de Guardar */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogGuardar,
					   hInstance);

     /* Mediante dicha caja pedimos el nombre para el esquema */
     strcpy (path,     npIB->path);
     strcpy (nom_fich, npIB->nom_fich);
     if (DialogBoxParam (hInstance, szNameGuardar, hwnd, lpfnProcDialogBox,
			 MAKELONG (0, (WORD) npIB)))
      {
       /* Ponemos el posiblemente nuevo nombre en la barra de t¡tulo */
       strcpy (npIB->path,     path);
       strcpy (npIB->nom_fich, nom_fich);
       strcpy (szBuffer, "D.Bloques - ");
       strcpy (nom, path);
       strcat (nom, nom_fich);
       if (strlen (nom) > 38)
	 strcat (szBuffer, "..");
       strcat (szBuffer, nom + max (0, (int)(strlen (nom) - 38)));
       SetWindowText (hwnd, szBuffer);
      }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     /* Desbloqueamos los datos referentes a la ventana */
     LocalUnlock (hMemIB);

     return (0);
    }

    case IDM_PUNTERO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Borramos lo que estuvi‚ramos dibujando antes */
     hdc = GetDC (hwnd);
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     tipo_dibujo = 0;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (NULL, IDC_ARROW));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_BLOQUE:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     hdc = GetDC (hwnd);
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     tipo_dibujo = BLOQUE;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (hInstance, "BLOQUE"));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_CABLE:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Borramos lo que estuvi‚ramos dibujando antes */
     hdc = GetDC (hwnd);
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     /* Fijamos el tipo de dibujo y su estado */
     tipo_dibujo = CABLE;
     empezado = FALSE;

     /* Cambiamos el cursor */
     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (hInstance, "ALICATE"));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_PUERTO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Borramos lo que estuvi‚ramos dibujando antes */
     hdc = GetDC (hwnd);
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     /* Fijamos el tipo de dibujo y su estado */
     tipo_dibujo = PUERTO;

     /* Cambiamos el cursor */
     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (hInstance, "PUERTO"));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_TEXTO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Borramos lo que estuvi‚ramos dibujando antes */
     hdc = GetDC (hwnd);
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     /* Fijamos el tipo de dibujo y su estado */
     tipo_dibujo = TEXTO;
     empezado = FALSE;

     /* Cambiamos el cursor */
     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (hInstance, "TEXTO"));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_OBJETO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Borramos lo que estuviera dibujado antes */
     hdc = GetDC (hwnd);
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     tipo_dibujo = BORRAR;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (hInstance, "BORRADOR"));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_GRUPO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Borramos lo que estuviera dibujado antes */
     hdc = GetDC (hwnd);
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     tipo_dibujo = GRUPO;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (NULL, IDC_CROSS));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_MOVER:
    case IDM_DESPLAZ:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Borramos lo que estuviera dibujado antes */
     hdc = GetDC (hwnd);
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     tipo_dibujo = (wParam == IDM_MOVER) ? MOVER : DESPLAZAR;
     empezado = FALSE;

     /* Cambiamos el cursor a la cruz horizontal */
     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (NULL, IDC_CROSS));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_HORIZ:
    case IDM_VERT:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     hdc = GetDC (hwnd);

     /* Borramos lo que estuviera dibujado antes */
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

     /* Redibujamos el dibujo anterior, con el nuevo estado */
     dibujar (npIB, hdc, tipo_dibujo, vert, horiz, espejo, empezado, hMF, FALSE);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     ReleaseDC (hwnd, hdc);

     /* Actualizamos el estado */
     horiz = 1 - horiz;
     vert  = 1 - vert;
     hMenu = GetMenu (hwndMarco);
     CheckMenuItem (GetSubMenu (hMenu, 1), IDM_HORIZ,
		    (horiz) ? MF_CHECKED : MF_UNCHECKED);
     CheckMenuItem (GetSubMenu (hMenu, 1), IDM_VERT,
		    (vert) ?  MF_CHECKED : MF_UNCHECKED);
     return (0);
    }

    case IDM_ESPEJO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     hdc = GetDC (hwnd);

     /* Borramos lo que estuviera dibujado antes */
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

     /* Redibujamos el dibujo anterior, con el nuevo estado */
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     ReleaseDC (hwnd, hdc);

     /* Actualizamos el estado */
     espejo = 1 - espejo;
     hMenu = GetMenu (hwndMarco);
     CheckMenuItem (GetSubMenu (hMenu, 1), IDM_ESPEJO,
		    (espejo) ? MF_CHECKED : MF_UNCHECKED);

     return (0);
    }

    case IDM_REJILLA:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIB = GetWindowWord (hwnd, 0);
     npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

     /* Borramos lo que estuviera dibujado antes */
     hdc = GetDC (hwnd);
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     rejilla = !rejilla;
     hMenu = GetMenu (hwndMarco);
     CheckMenuItem (GetSubMenu (hMenu, 1), IDM_REJILLA,
		    (rejilla) ? MF_CHECKED : MF_UNCHECKED);
     if (rejilla)
      {
       GetClientRect (hwnd, &rect);
       dibujar_rejilla (hdc, rect);
      }
     else
      {
       InvalidateRect (hwnd, NULL, TRUE);
       UpdateWindow (hwnd);
      }

     /* Redibujamos el dibujo anterior */
     dibujar (npIB, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIB);

     /* Liberamos el contexto de dispositivo */
     ReleaseDC (hwnd, hdc);

     return (0);
    }

    /* Otras selecciones del men£ */
   }

   break;
  }

  case WM_SYSCOMMAND:
  {
   /* Si se ha elegido Cerrar en el men£ de sistema.. */
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     /* ..emulamos el proceso de 'Salir' */
     SendMessage (hwnd, WM_CLOSE, 0, 0L);
     return (0);
    }

   break;
  }

  case WM_HSCROLL:
  {
   /* Obtenemos los datos de la ventana de Diagrama de Bloques */
   hMemIB = GetWindowWord (hwnd, 0);
   npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

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
     nIncScrl = LOWORD(lParam) - npIB->nHposScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   /* Modificamos las posiciones de Scroll */
   if (nIncScrl = max (-npIB->nHposScrl,
		       min (nIncScrl, npIB->nHmaxScrl - npIB->nHposScrl)))
    {
     npIB->nHposScrl += nIncScrl;
     ScrollWindow (hwnd, -SCRLDESPL * nIncScrl, 0, NULL, NULL);
     SetScrollPos (hwnd, SB_HORZ, npIB->nHposScrl, TRUE);
    }

   /* Desbloqueamos los datos de la ventana de esquema */
   LocalUnlock (hMemIB);

   return (0);
  }

  case WM_VSCROLL:
  {
   /* Obtenemos los datos de la ventana de esquema */
   hMemIB = GetWindowWord (hwnd, 0);
   npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

   switch (wParam)
   {
    case SB_TOP:
    {
     nIncScrl = -npIB->nVposScrl;
     break;
    }

    case SB_BOTTOM:
    {
     nIncScrl = npIB->nVmaxScrl - npIB->nVposScrl;
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
     nIncScrl = LOWORD(lParam) - npIB->nVposScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   /* Modificamos las posiciones de Scroll */
   if (nIncScrl = max (-npIB->nVposScrl,
		       min (nIncScrl, npIB->nVmaxScrl - npIB->nVposScrl)))
    {
     npIB->nVposScrl += nIncScrl;
     ScrollWindow (hwnd, 0, -SCRLDESPL * nIncScrl, NULL, NULL);
     SetScrollPos (hwnd, SB_VERT, npIB->nVposScrl, TRUE);
    }

   /* Desbloqueamos los datos de la ventana de esquema */
   LocalUnlock (hMemIB);

   return (0);
  }

  case WM_QUERYENDSESSION:
  case WM_CLOSE:
  {
   /* Obtenemos los datos referentes a la ventana de diagrama de bloques */
   hMemIB = GetWindowWord (hwnd, 0);
   npIB = (InfoBloque NEAR *) LocalLock (hMemIB);

   /* Guardamos los datos del diagrama, si se desea */
   if ((npIB->n_bloques > 0) || (npIB->n_cables  > 0) ||
       (npIB->n_textos  > 0) || (npIB->n_puertos > 0))
    {
     strcpy (szBuffer, "El Diagrama ");
     strcat (szBuffer, npIB->nom_fich);
     strcat (szBuffer, " va a cerrarse\n¿Desea registrar los cambios?");
     resp = MessageBox (hwnd, szBuffer,
			"Cerrando Diagrama de Bloques",
			MB_ICONINFORMATION | MB_YESNOCANCEL);

     if (resp == IDCANCEL)
      {
       LocalUnlock (hMemIB);
       return (0);
      }
     else
      {
       if (resp == IDYES)
	 SendMessage (hwnd, WM_COMMAND, IDM_GUARDAR, 0);

       /* Borramos los ficheros temporales */
       for (i = TMPBLOQUES; i <= TMPTEXTOS; ++i)
	{
	 strcpy (nom, npIB->path);
	 strcat (nom, npIB->nom_fich);
	 strcat (nom, szExtFich[i]);
	 OpenFile (nom, &of, OF_DELETE);
	}

       /* Anulamos la operaci¢n en que estuvi‚semos */
       tipo_dibujo = 0;
       empezado = FALSE;

       /* Liberamos la memoria usada */
       liberar_memoria_diagrama (npIB);
      }
    }

   /* Desbloqueamos los datos */
   LocalUnlock (hMemIB);

   /* Pasamos a DefMDIChildProc */
   break;
  }

  case WM_DESTROY:
  {
   hMemIB = GetWindowWord (hwnd, 0);
   LocalFree (hMemIB);
   --nBloques;
   return (0);
  }
 }

 return (DefMDIChildProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcVentSch (HWND hwnd, WORD message,
				     WORD wParam, LONG lParam)
{
 static HWND  	    hwndMarco, hwndCliente;
 static HANDLE      hInstance, hMF = NULL;
 static RECT  	    bloque, bloque_fijo;
 static char        szNameGuardar[]  = "GUARDAR",
		    szNameCaract[]   = "CARACT",
		    szNameNudos[]    = "CARNUDOS",
		    szNameCuad[]     = "CARCUAD",
		    elem_bibl[] = "";
 static short 	    tipo_dibujo;
 static BOOL  	    horiz      = TRUE,  vert  = FALSE, espejo   = FALSE,
		    rejilla    = FALSE, pinta = FALSE, empezado = FALSE;
 HMENU        	    hMenu;
 HDC          	    hdc;
 HPEN         	    hPen, hViejoPen;
 HCURSOR            hCursor;
 RECT	      	    rect;
 POINT        	    point;
 TEXTMETRIC   	    tm;
 PAINTSTRUCT  	    ps;
 OFSTRUCT           of;
 FARPROC      	    lpfnProcDialogBox;
 LOCALHANDLE        hMemIV;
 InfoVentana NEAR  *npIV;
 elemento     	    datos_elem;
 cuadripolo   	    datos_cuad;
 int          	    hFichero, resp;
 short        	    i, j, k, nIncScrl, nDesplx, nDesply, elem[6];
 char         	    nom[84], szBuffer[60],
		    szPrinterDriver[9], szPrinterName[30], szPrinterPort[6];

 switch (message)
 {
  case WM_CREATE:
  {
   /* Obtenemos el handle a la ocurrencia del m¢dulo, que se guarda forever */
   hInstance = ((LPCREATESTRUCT) lParam)->hInstance;

   /* Obtenemos handles a las ventanas padre y abuela */
   hwndCliente = GetParent (hwnd);
   hwndMarco   = GetParent (hwndCliente);

   /* Inicializamos los datos comunes a todas las ventanas */
   pinta    = FALSE;
   empezado = FALSE;

   /* Fijamos las checkmarks del men£ de Esquema */
   CheckMenuItem (hMenuEsquema, IDM_HORIZ,   MF_BYCOMMAND | MF_CHECKED);
   CheckMenuItem (hMenuEsquema, IDM_VERT,    MF_BYCOMMAND | MF_UNCHECKED);
   CheckMenuItem (hMenuEsquema, IDM_ESPEJO,  MF_BYCOMMAND | MF_UNCHECKED);
   CheckMenuItem (hMenuEsquema, IDM_REJILLA, MF_BYCOMMAND | MF_UNCHECKED);

   /* Inicializamos la estructura de datos de la ventana de esquema */
   hMemIV = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (InfoVentana));
   npIV = (InfoVentana NEAR *) LocalLock (hMemIV);
   strcpy (npIV->path, path);
   strcpy (npIV->nom_fich, NOMBRE);
   asignar_memoria_esquema (npIV);
   LocalUnlock (hMemIV);

   /* Transferimos dichos datos a la ventana */
   SetWindowWord (hwnd, 0, hMemIV);

   return (0);
  }

  case WM_SIZE:
  {
   /* Obtenemos los datos referentes a la ventana de esquema */
   hMemIV = GetWindowWord (hwnd, 0);
   npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

   /* Si la ventana est  siendo minimizada, activamos el flag */
   if (wParam == SIZE_MINIMIZED)
     npIV->minimizada = TRUE;

   /* pero, si no,.. */
   else
    {
     /* ..desactivamos el flag */
     npIV->minimizada = FALSE;

     /* Fijamos los rangos y posiciones de las barras de Scroll */
     npIV->nVmaxScrl = max (0, 255 - (short) HIWORD(lParam) / SCRLDESPL);
     npIV->nVposScrl = min (npIV->nVposScrl, npIV->nVmaxScrl);
     SetScrollRange (hwnd, SB_VERT, 0, npIV->nVmaxScrl, FALSE);
     SetScrollPos   (hwnd, SB_VERT, npIV->nVposScrl, TRUE);

     npIV->nHmaxScrl = max (0, 255 - (short) LOWORD(lParam) / SCRLDESPL);
     npIV->nHposScrl = min (npIV->nHposScrl, npIV->nHmaxScrl);
     SetScrollRange (hwnd, SB_HORZ, 0, npIV->nHmaxScrl, FALSE);
     SetScrollPos   (hwnd, SB_HORZ, npIV->nHposScrl, TRUE);
    }

   /* Desbloqueamos los datos */
   LocalUnlock (hMemIV);

   return (0);
  }

  case WM_MDIACTIVATE:
  {
   /* Si obtenemos el foco de entrada..*/
   if (wParam == TRUE)
    {
     /*..ponemos el men£ Esquema */
     SendMessage (hwndCliente, WM_MDISETMENU, 0,
		  MAKELONG(hMenuEsquema, hMenuEsquemaVentana));

     /* Y registramos cu l pasa a ser la ventana activa y su tipo */
     hwndActiva = hwnd;
     tvactiva   = TV_ESQUEMA;
    }

   /* pero, si perdemos el foco, ponemos el men£ Inicial */
   else
     SendMessage (hwndCliente, WM_MDISETMENU, 0, MAKELONG(hMenuInicial, 0));

   DrawMenuBar (hwndMarco);
   return (0);
  }

  case WM_PAINT:
  {
   /* Obtenemos el contexto de la ventana */
   hdc = BeginPaint (hwnd, &ps);

   /* Redibujamos lo que haga falta */
   hMemIV = GetWindowWord (hwnd, 0);
   npIV = (InfoVentana NEAR *) LocalLock (hMemIV);
   dibujar_circuito (npIV, hdc, ps.rcPaint);
   if (rejilla)
     dibujar_rejilla (hdc, ps.rcPaint);
   LocalUnlock (hMemIV);

   /* Liberamos el contexto */
   EndPaint (hwnd, &ps);
  }

  case WM_MOUSEMOVE:
  {
   /* Reaccionar s¢lo si es la ventana actualmente activa */
   if (hwnd == hwndActiva)
    {
     /* Obtenemos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     /* Si no est  minimimizada y estamos con algo que no sea un nudo.. */
     if ((!npIV->minimizada) && (tipo_dibujo != NUDO))
      {
       /* ..obtenemos la posici¢n del cursor */
       nDesplx = LOWORD(lParam) + npIV->nHposScrl * SCRLDESPL;
       nDesply = HIWORD(lParam) + npIV->nVposScrl * SCRLDESPL;

       if (rejilla)
	{
	 nDesplx = (nDesplx / TAMREJ + (nDesplx % TAMREJ >= 2)) * TAMREJ;
	 nDesply = (nDesply / TAMREJ + (nDesply % TAMREJ >= 2)) * TAMREJ;
	}

       /* Empezamos la pintada */
       hdc = GetDC (hwnd);

       /* Borramos el elemento antiguo */
       dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

       switch (tipo_dibujo)
       {
	case ELEMENTO:
	{
	 /* Obtenemos las nuevas coordenadas */
	 elementos = (short *) LocalLock (npIV->hmemElems);
	 e(npIV->n_elems, 1) = nDesplx;
	 e(npIV->n_elems, 2) = nDesply;
	 LocalUnlock (npIV->hmemElems);
	 break;
	}

	case CABLE:
	{
	 if (empezado)
	  {
	   /* Obtenemos las nuevas coordenadas */
	   cables = (short *) LocalLock (npIV->hmemCables);
	   c(npIV->n_cables, 2) = nDesplx;
	   c(npIV->n_cables, 3) = nDesply;
	   LocalUnlock (npIV->hmemCables);
	  }
	 break;
	}

	case MASA:
	{
	 /* Obtenemos las nuevas coordenadas */
	 nudos = (short *) LocalLock (npIV->hmemNudos);
	 n(npIV->n_nudos, 0) = nDesplx;
	 n(npIV->n_nudos, 1) = nDesply;
	 n(npIV->n_nudos, 2) = 0;
	 LocalUnlock (npIV->hmemCables);

	 break;
	}

	case CUADRIPOLO:
	case GRUPO:
	case MOVER:
	case DESPLAZAR:
	 {
	  if (empezado)
	   {
	    /* Borramos el bloque anterior */
	    dibujar_recuadro (npIV, hdc, bloque.left, bloque.top,
					 bloque.right - bloque.left,
					 bloque.bottom - bloque.top, FALSE);

	    /* Si es un cuadripolo, borramos la representaci¢n anterior */
	    if (tipo_dibujo == CUADRIPOLO)
	      dibujar_cuadripolo (npIV, hdc, bloque.left, bloque.top,
					     bloque.right  - bloque.left,
					     bloque.bottom - bloque.top, horiz,
				  "          ", FALSE);

	    /* Obtenemos las nuevas coordenadas */
	    if (((tipo_dibujo == MOVER)|| (tipo_dibujo == DESPLAZAR)) &&
		(empezado == 2))
	     {
	      bloque.left  += nDesplx - bloque.right;
	      bloque.top   += nDesply - bloque.bottom;
	     }
	    bloque.right  = nDesplx;
	    bloque.bottom = nDesply;

	    /* Pintamos el nuevo bloque */
	    dibujar_recuadro (npIV, hdc, bloque.left, bloque.top,
			      bloque.right - bloque.left,
			      bloque.bottom - bloque.top, FALSE);

	    /* Y, si es un cuadripolo, la nueva representaci¢n */
	    if (tipo_dibujo == CUADRIPOLO)
	      dibujar_cuadripolo (npIV, hdc, bloque.left, bloque.top,
					     bloque.right  - bloque.left,
					     bloque.bottom - bloque.top, horiz,
				  "          ", FALSE);
	   }
	  break;
	 }
       }

       /* Pintamos el elemento nuevo */
       dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

       /* Liberamos el contexto */
       ReleaseDC (hwnd, hdc);

       return (0);
      }

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);
    }

   break;
  }

  case WM_LBUTTONUP:
  {
   /* Obtenemos los datos referentes a la ventana */
   hMemIV = GetWindowWord (hwnd, 0);
   npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

   nDesplx = LOWORD(lParam) + npIV->nHposScrl * SCRLDESPL;
   nDesply = HIWORD(lParam) + npIV->nVposScrl * SCRLDESPL;

   if (rejilla)
    {
     nDesplx = (nDesplx / TAMREJ + (nDesplx % TAMREJ >= 2)) * TAMREJ;
     nDesply = (nDesply / TAMREJ + (nDesply % TAMREJ >= 2)) * TAMREJ;
    }

   switch (tipo_dibujo)
   {
    case 0:
    {
     /* Localizamos el elemento que se encuentre en las coordenadas */
     if ((i = buscar_por_coords (npIV, ELEMENTO, nDesplx, nDesply)) >= 0)
      {
       /* Le dibujamos un recuadro al elemento y copiamos sus caract. */
       hdc = GetDC (hwnd);
       elementos = (short *) LocalLock (npIV->hmemElems);
       memcpy (elem, elementos + 6*i, 6*sizeof (short));
       dibujar_recuadro (npIV, hdc, elem[1] - 15*elem[4], elem[2] - 15*elem[3],
			 40*elem[3] + 30*elem[4], 30*elem[3] + 40*elem[4],
			 FALSE);
       LocalUnlock (npIV->hmemElems);
       ReleaseDC (hwnd, hdc);

       /* Obtenemos la dir. del proc. para la Caja de Di logo de Caracter. */
       lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogCaract,
					     hInstance);

       /* Si la caja de di logo termin¢ con Cancelar.. */
       if (!DialogBoxParam (hInstance, szNameCaract, hwnd, lpfnProcDialogBox,
			    MAKELONG ((WORD) i, (WORD) npIV)))
	{
	 /* ..borramos el recuadro al elemento */
	 hdc = GetDC (hwnd);
	 dibujar_recuadro (npIV, hdc, elem[1] - 15*elem[4], elem[2] - 15*elem[3],
			   40*elem[3] + 30*elem[4], 30*elem[3] + 40*elem[4],
			   FALSE);
	 ReleaseDC (hwnd, hdc);

	 /* Restauramos los datos del elemento */
	 elementos = (short *) LocalLock (npIV->hmemElems);
	 memcpy (elementos + 6*i, elem, 6*sizeof (short));
	 LocalUnlock (npIV->hmemElems);
	}

       /* pero, si termin¢ con OK.. */
       else
	{
	 /* ..borramos el recuadro al elemento */
	 hdc = GetDC (hwnd);
	 dibujar_recuadro (npIV, hdc, elem[1] - 15*elem[4], elem[2] - 15*elem[3],
			   40*elem[3] + 30*elem[4], 30*elem[3] + 40*elem[4],
			   FALSE);

	 /* Actualizamos la representaci¢n. Primero borramos el antiguo */
	 hPen = CreatePen (PS_SOLID, 1, RGB(255,255,255));
	 hViejoPen = SelectObject (hdc, hPen);
	 dibujar_elemento (npIV, hdc, elem[0], elem[1], elem[2],
			   elem[3], elem[4], elem[5], "     ", hMF, TRUE);
	 SelectObject (hdc, hViejoPen);
	 DeleteObject (hPen);

	 /* Obtenemos el nombre del elemento */
	 strcpy (nom, npIV->path);
	 strcat (nom, npIV->nom_fich);
	 hFichero = OpenFile (strcat (nom, szExtFich[TMPELEMS]), &of, OF_READ);
	 _llseek (hFichero, i * sizeof (elemento), 0);
	 _lread (hFichero, (LPSTR) szBuffer, 6);
	 _lclose (hFichero);

	 /* Dibujamos el nuevo */
	 elementos = (short *) LocalLock (npIV->hmemElems);
	 dibujar_elemento (npIV, hdc, e(i,0), e(i,1), e(i,2),
			   e(i,3), e(i,4), e(i,5), szBuffer, hMF, TRUE);

	 LocalUnlock (npIV->hmemElems);
	 ReleaseDC (hwnd, hdc);
	}

       /* Liberamos el thunk a la Caja de Di logo */
       FreeProcInstance (lpfnProcDialogBox);
      }

     /* Si no hay elemento, buscamos un nudo */
     else
       if ((i = buscar_por_coords (npIV, NUDO, nDesplx, nDesply)) >= 0)
	{
	 /* Le dibujamos un recuadro al nudo y copiamos sus caract. */
	 hdc = GetDC (hwnd);
	 nudos = (short *) LocalLock (npIV->hmemNudos);
	 memcpy (elem, nudos + 3*i, 3*sizeof (short));
	 dibujar_recuadro (npIV, hdc, elem[0] - 5, elem[1] - 5, 10, 10, FALSE);
	 LocalUnlock (npIV->hmemNudos);
	 ReleaseDC (hwnd, hdc);

	 /* Obtenemos la dir. del proc. para la Caja de Di logo de N£meros */
	 lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogNudos,
					       hInstance);

	 /* Si la caja de di logo termin¢ con Cancelar.. */
	 if (!DialogBoxParam (hInstance, szNameNudos, hwnd, lpfnProcDialogBox,
			      MAKELONG ((WORD) i,(WORD) npIV)))
	  {
	   /* ..borramos el recuadro al nudo */
	   hdc = GetDC (hwnd);
	   dibujar_recuadro (npIV, hdc, elem[0] - 5, elem[1] - 5, 10, 10, FALSE);
	   ReleaseDC (hwnd, hdc);

	   /* Restauramos los datos del nudo */
	   nudos = (short *) LocalLock (npIV->hmemNudos);
	   memcpy (nudos + 3*i, elem, 3*sizeof (short));
	   LocalUnlock (npIV->hmemNudos);
	  }

	 /* pero, si termin¢ con OK.. */
	 else
	  {
	   /* ..s¢lamente borramos el recuadro al nudo */
	   hdc = GetDC (hwnd);
	   dibujar_recuadro (npIV, hdc, elem[0] - 5, elem[1] - 5, 10, 10, FALSE);
	   ReleaseDC (hwnd, hdc);
	  }

	 /* Liberamos el thunk a la Caja de Di logo */
	 FreeProcInstance (lpfnProcDialogBox);
	}

       /* Si no hay nudo, buscamos un cuadripolo */
       else
	 if ((i = buscar_por_coords (npIV, CUADRIPOLO, nDesplx, nDesply)) >= 0)
	  {
	   /* Le dibujamos un recuadro al cuadripolo y copiamos sus caract. */
	   hdc = GetDC (hwnd);
	   cuad = (short *) LocalLock (npIV->hmemCuad);
	   memcpy (elem, cuad + 5*i, 5*sizeof (short));
	   dibujar_recuadro (npIV, hdc, elem[0], elem[1],
			     elem[2] - elem[0], elem[3] - elem[1], FALSE);
	   LocalUnlock (npIV->hmemCuad);
	   ReleaseDC (hwnd, hdc);

	   /* Obtenemos la dir. del proc. para la Caja de Di logo de Caracter. */
	   lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogCuad,
						 hInstance);

	   /* Si la caja de di logo termin¢ con Cancelar.. */
	   if (!DialogBoxParam (hInstance, szNameCuad, hwnd, lpfnProcDialogBox,
				MAKELONG ((WORD) i, (WORD) npIV)))
	    {
	     /* ..borramos el recuadro al cuadripolo */
	     hdc = GetDC (hwnd);
	     dibujar_recuadro (npIV, hdc, elem[0], elem[1],
			       elem[2] - elem[0], elem[3] - elem[1], FALSE);
	     ReleaseDC (hwnd, hdc);

	     /* Restauramos los datos del cuadripolo */
	     cuad = (short *) LocalLock (npIV->hmemCuad);
	     memcpy (cuad + 5*i, elem, 5*sizeof (short));
	     LocalUnlock (npIV->hmemCuad);
	    }

	   /* pero, si termin¢ con OK.. */
	   else
	    {
	     /* ..borramos el recuadro al cuadripolo */
	     hdc = GetDC (hwnd);
	     dibujar_recuadro (npIV, hdc, elem[0], elem[1],
			       elem[2] - elem[0], elem[3] - elem[1], FALSE);

	     /* Obtenemos el nombre del cuadripolo */
	     strcpy (nom, npIV->path);
	     strcat (nom, npIV->nom_fich);
	     hFichero = OpenFile (strcat (nom, szExtFich[TMPCUADS]), &of, OF_READ);
	     _llseek (hFichero, i * sizeof (cuadripolo), 0);
	     _lread (hFichero, (LPSTR) szBuffer, 11);
	     _lclose (hFichero);

	     /* Dibujamos el nuevo */
	     cuad = (short *) LocalLock (npIV->hmemCuad);
	     dibujar_cuadripolo (npIV, hdc, C(i,0), C(i,1),
				 C(i,2) - C(i,0), C(i,3) - C(i,1),
				 C(i,4), szBuffer, TRUE);
	     LocalUnlock (npIV->hmemCuad);
	     ReleaseDC (hwnd, hdc);
	    }

	   /* Liberamos el thunk a la Caja de Di logo */
	   FreeProcInstance (lpfnProcDialogBox);
	  }

     return (0);
    }

    case ELEMENTO:
    {
     hdc = GetDC (hwnd);

     /* Guardamos los datos en la lista */
     elementos = (short *) LocalLock (npIV->hmemElems);

     e(npIV->n_elems, 1) = nDesplx + 15 * vert;
     e(npIV->n_elems, 2) = nDesply + 15 * horiz;
     e(npIV->n_elems, 3) = horiz;
     e(npIV->n_elems, 4) = vert;
     e(npIV->n_elems, 5) = espejo;

     /* Dibujamos el elemento definitivamente */
     dibujar_recuadro (npIV, hdc, nDesplx, nDesply,
		       40*horiz + 30*vert, 40*vert + 30*horiz, FALSE);
     dibujar_elemento (npIV, hdc, e(npIV->n_elems,0), e(npIV->n_elems,1),
		       e(npIV->n_elems,2), e(npIV->n_elems,3),
		       e(npIV->n_elems,4), e(npIV->n_elems,5),
		       "     ", hMF, TRUE);

     /* Registramos el elemento en el fichero temporal */
     registrar_elemento (npIV, e(npIV->n_elems,0));
     ++npIV->n_elems;

     /* Pintamos el elemento siguiente */
     e(npIV->n_elems, 0) = e(npIV->n_elems - 1, 0);
     e(npIV->n_elems, 1) = nDesplx;
     e(npIV->n_elems, 2) = nDesply;
     e(npIV->n_elems, 3) = horiz;
     e(npIV->n_elems, 4) = vert;
     e(npIV->n_elems, 5) = espejo;

     dibujar_recuadro (npIV, hdc, nDesplx, nDesply,
		       40*horiz + 30*vert, 40*vert + 30*horiz, FALSE);
     dibujar_elemento (npIV, hdc, e(npIV->n_elems,0),
		       e(npIV->n_elems,1) + 15*vert,
		       e(npIV->n_elems,2) + 15*horiz,
		       e(npIV->n_elems,3), e(npIV->n_elems,4),
		       e(npIV->n_elems,5), elem_bibl, hMF, FALSE);
     LocalUnlock (npIV->hmemElems);
     ReleaseDC (hwnd, hdc);
     break;
    }

    case CABLE:
    {
     hdc = GetDC (hwnd);
     cables = (short *) LocalLock (npIV->hmemCables);

     empezado = 1 - empezado;
     if (empezado)
      {
       c(npIV->n_cables, 0) = nDesplx;
       c(npIV->n_cables, 1) = nDesply;
       c(npIV->n_cables, 2) = c(npIV->n_cables, 0) + 1;
       c(npIV->n_cables, 3) = c(npIV->n_cables, 1) + 1;
       dibujar_cable (npIV, hdc, c(npIV->n_cables,0), c(npIV->n_cables,1),
		      c(npIV->n_cables,2) - c(npIV->n_cables,0),
		      c(npIV->n_cables,3) - c(npIV->n_cables,1),
		      horiz, vert, FALSE);
      }
     else
      {
       c(npIV->n_cables, 2) = nDesplx;
       c(npIV->n_cables, 3) = nDesply;
       c(npIV->n_cables, 4) = horiz;
       c(npIV->n_cables, 5) = vert;
       dibujar_cable (npIV, hdc, c(npIV->n_cables,0), c(npIV->n_cables,1),
		      c(npIV->n_cables,2) - c(npIV->n_cables,0),
		      c(npIV->n_cables,3) - c(npIV->n_cables,1),
		      horiz, vert, TRUE);
       ++npIV->n_cables;
      }
     LocalUnlock (npIV->hmemCables);
     ReleaseDC (hwnd, hdc);
     break;
    }

    case NUDO:
    {
     hdc = GetDC (hwnd);
     dibujar_nudo (npIV, hdc, nDesplx, nDesply);
     ReleaseDC (hwnd, hdc);
     nudos = (short *) LocalLock (npIV->hmemNudos);
     n(npIV->n_nudos, 0) = nDesplx;
     n(npIV->n_nudos, 1) = nDesply;
     n(npIV->n_nudos, 2) = npIV->n_nudos++;
     LocalUnlock (npIV->hmemNudos);

     break;
    }

    case MASA:
    {
     nudos = (short *) LocalLock (npIV->hmemNudos);
     hdc = GetDC (hwnd);
     dibujar_masa (npIV, hdc, nDesplx, nDesply, TRUE);
     ReleaseDC (hwnd, hdc);
     n(npIV->n_nudos, 0) = nDesplx;
     n(npIV->n_nudos, 1) = nDesply;
     n(npIV->n_nudos, 2) = 0;
     ++npIV->n_nudos;
     LocalUnlock (npIV->hmemNudos);

     break;
    }

    case BORRAR:
    {
     /* Obtenemos un contexto */
     hdc = GetDC (hwnd);

     /* Le asignamos un l piz del color del fondo */
     hPen = CreatePen (PS_SOLID, 1, RGB(255,255,255));
     hViejoPen = SelectObject (hdc, hPen);

     /* Buscamos jer rquicamente nudo, cable, elemento y cuadripolo */
     if ((i = buscar_por_coords (npIV, NUDO, nDesplx, nDesply)) >= 0)
      {
       --npIV->n_nudos;
       nudos = (short *) LocalLock (npIV->hmemNudos);

       for (j = 0; j < 3; ++j)
	{
	 k = n(i,j);
	 n(i,j) = n(npIV->n_nudos,j);
	 n(npIV->n_nudos, j) = k;
	}

       dibujar (npIV, hdc, NUDO, 0, 0, 0, 0, NULL, TRUE);
       LocalUnlock (npIV->hmemNudos);
      }
     else
       if ((i = buscar_por_coords (npIV, CABLE, nDesplx, nDesply)) >= 0)
	{
	 --npIV->n_cables;
	 cables = (short *) LocalLock (npIV->hmemCables);

	 for (j = 0; j < 6; ++j)
	  {
	   k = c(i,j);
	   c(i,j) = c(npIV->n_cables,j);
	   c(npIV->n_cables, j) = k;
	  }

	 dibujar (npIV, hdc, CABLE, c(npIV->n_cables,4), c(npIV->n_cables,5),
	 	  0, 1, NULL, TRUE);
	 LocalUnlock (npIV->hmemCables);
	}
       else
	 if ((i = buscar_por_coords (npIV, ELEMENTO, nDesplx, nDesply)) >= 0)
	  {
	   /* Reducimos el n£mero de elementos */
	   --npIV->n_elems;

	   /* Intercambiamos el elemento a eliminar por el £ltimo de la
	      lista, evit ndonos as¡ reordenaciones */
	   elementos = (short *) LocalLock (npIV->hmemElems);

	   for (j = 0; j < 6; ++j)
	    {
	     k = e(i,j);
	     e(i,j) = e(npIV->n_elems,j);
	     e(npIV->n_elems, j) = k;
	    }

	   /* Borramos el elemento */
	   dibujar_elemento (npIV, hdc, e(npIV->n_elems,0), e(npIV->n_elems,1),
			     e(npIV->n_elems,2), e(npIV->n_elems,3),
			     e(npIV->n_elems,4), e(npIV->n_elems,5),
			     "     ", NULL, TRUE);
	   LocalUnlock (npIV->hmemElems);

	   /* Idem con sus datos almacenados en el fichero temporal */
	   strcpy (nom, npIV->path);
	   strcat (nom, npIV->nom_fich);
	   strcat (nom, szExtFich[TMPELEMS]);
	   hFichero = OpenFile (nom, &of, OF_READWRITE | OF_SHARE_EXCLUSIVE);

	   _llseek (hFichero, npIV->n_elems * sizeof (datos_elem), 0);
	   _lread  (hFichero, (LPSTR) &datos_elem, sizeof (datos_elem));
	   _llseek (hFichero, i * sizeof (datos_elem), 0);
	   _lwrite (hFichero, (LPSTR) &datos_elem, sizeof (datos_elem));

	   _lclose (hFichero);
	  }
	 else
	   if ((i = buscar_por_coords (npIV, CUADRIPOLO, nDesplx, nDesply)) >= 0)
	    {
	     /* Reducimos el n£mero de cuadripolos */
	     --npIV->n_cuad;

	     /* Intercambiamos el elemento a eliminar por el £ltimo de la
		lista, evit ndonos as¡ reordenaciones */
	     cuad = (short *) LocalLock (npIV->hmemCuad);

	     for (j = 0; j < 5; ++j)
	      {
	       k = C(i,j);
	       C(i,j) = C(npIV->n_cuad,j);
	       C(npIV->n_cuad, j) = k;
	      }

	     /* Borramos el cuadripolo */
	     dibujar_cuadripolo (npIV, hdc, C(npIV->n_cuad,0), C(npIV->n_cuad,1),
				 C(npIV->n_cuad,2) - C(npIV->n_cuad,0),
				 C(npIV->n_cuad,3) - C(npIV->n_cuad,1),
				 C(npIV->n_cuad,4), "          ", TRUE);
	     LocalUnlock (npIV->hmemCuad);

	     /* Idem con sus datos almacenados en el fichero temporal */
	     strcpy (nom, npIV->path);
	     strcat (nom, npIV->nom_fich);
	     strcat (nom, szExtFich[TMPCUADS]);
	     hFichero = OpenFile (nom, &of, OF_READWRITE | OF_SHARE_EXCLUSIVE);

	     _llseek (hFichero, npIV->n_cuad  * sizeof (datos_cuad), 0);
	     _lread  (hFichero, (LPSTR) &datos_cuad, sizeof (datos_cuad));
	     _llseek (hFichero, i * sizeof (datos_cuad), 0);
	     _lwrite (hFichero, (LPSTR) &datos_cuad, sizeof (datos_cuad));

	     _lclose (hFichero);
	    }

     /* Reseleccionamos el l piz anterior */
     SelectObject (hdc, hViejoPen);
     DeleteObject (hPen);

     /* Liberamos el contexto de dispositivo */
     ReleaseDC (hwnd, hdc);
     break;
    }

    case CUADRIPOLO:
    case GRUPO:
    case MOVER:
    case DESPLAZAR:
    {
     hdc = GetDC (hwnd);

     /* Si a£n no est  empezado.. */
     if (!empezado)
      {
       /* ..empezamos el bloque */
       empezado = 1;

       /* Obtenemos las coordenadas del bloque y lo dibujamos */
       bloque.left = bloque.right  = nDesplx;
       bloque.top  = bloque.bottom = nDesply;
       dibujar_recuadro (npIV, hdc, bloque.left, bloque.top,
			 bloque.right - bloque.left,
			 bloque.bottom - bloque.top, FALSE);
      }

     /* pero, si ya est  empezado.. */
     else
      {
       /* ..ordenamos las coordenadas del bloque */
       rect = bloque;
       bloque.left   = min (rect.left, rect.right);
       bloque.right  = max (rect.left, rect.right);
       bloque.top    = min (rect.top,  rect.bottom);
       bloque.bottom = max (rect.top,  rect.bottom);

       /* Si estamos borrando ¢ moviendo (no seleccionando) un grupo.. */
       if ((tipo_dibujo == GRUPO) || (empezado == 2))
	{
	 /* ..registramos que hemos terminado el bloque */
	 empezado = 0;

	 /* Si estamos moviendo un bloque.. */
	 if ((tipo_dibujo == MOVER) || (tipo_dibujo == DESPLAZAR))
	  {
	   /* ..obtenemos el desplazamiento */
	   nDesplx = bloque.left - bloque_fijo.left;
	   nDesply = bloque.top  - bloque_fijo.top;

	   /* Y cambiamos al bloque que se se¤al¢ en un principio, y no al
	      se¤alado como destino, puesto que ‚ste £ltimo no contiene
	      objeto alguno */
	   rect = bloque;
	   bloque = bloque_fijo;
	   bloque_fijo = rect;
	  }

	 /* Borramos ¢ movemos elementos */
	 elementos = (short *) LocalLock (npIV->hmemElems);

	 /* Abrimos el fichero temporal de elementos */
	 strcpy (nom, npIV->path);
	 strcat (nom, npIV->nom_fich);
	 strcat (nom, szExtFich[TMPELEMS]);
	 hFichero = OpenFile (nom, &of, OF_READWRITE | OF_SHARE_EXCLUSIVE);

	 for (i = 0; i < npIV->n_elems; ++i)
	  {
	   if (tipo_dibujo == GRUPO)
	    {
	     while ((e(i,1) - 5*e(i,4) > bloque.left) &&
		    (e(i,1) + 5*e(i,4) + 40*e(i,3) < bloque.right) &&
		    (e(i,2) - 5*e(i,3) > bloque.top) &&
		    (e(i,2) + 5*e(i,3) + 40*e(i,4) < bloque.bottom) &&
		    (i < npIV->n_elems))
	      {
	       --npIV->n_elems;
	       memcpy (elementos + 6*i, elementos + 6*npIV->n_elems,
		       6*sizeof(short));

	       /* Sustitu¡r los datos del elemento a borrar por los del
		  £ltimo elemento, tambi‚n en el fichero temporal */
	       _llseek (hFichero, npIV->n_elems * sizeof (datos_elem), 0);
	       _lread  (hFichero, (LPSTR) &datos_elem, sizeof (datos_elem));
	       _llseek (hFichero, i * sizeof (datos_elem), 0);
	       _lwrite (hFichero, (LPSTR) &datos_elem, sizeof (datos_elem));
	      }
	    }
	   else
	    {
	     if ((e(i,1) - 5*e(i,4) > bloque.left) &&
		 (e(i,1) + 5*e(i,4) + 40*e(i,3) < bloque.right) &&
		 (e(i,2) - 5*e(i,3) > bloque.top) &&
		 (e(i,2) + 5*e(i,3) + 40*e(i,4) < bloque.bottom))
	      {
	       e(i,1) += nDesplx;
	       e(i,2) += nDesply;
	      }
	    }
	  }
	 LocalUnlock (npIV->hmemElems);

	 /* Cerramos el fichero temporal de elementos */
	 _lclose (hFichero);

	 /* Borramos ¢ movemos cuadripolos */
	 cuad = (short *) LocalLock (npIV->hmemCuad);

	 /* Abrimos el fichero temporal de cuadripolos */
	 strcpy (nom, npIV->path);
	 strcat (nom, npIV->nom_fich);
	 strcat (nom, szExtFich[TMPCUADS]);
	 hFichero = OpenFile (nom, &of, OF_READWRITE | OF_SHARE_EXCLUSIVE);

	 for (i = 0; i < npIV->n_cuad; ++i)
	  {
	   if (tipo_dibujo == GRUPO)
	    {
	     while ((C(i,0) > bloque.left) && (C(i,2) < bloque.right)  &&
		    (C(i,1) > bloque.top)  && (C(i,3) < bloque.bottom) &&
		    (i < npIV->n_cuad))
	      {
	       --npIV->n_cuad;
	       memcpy (cuad + 5*i, cuad + 5*npIV->n_cuad, 5 * sizeof(short));

	       /* Sustitu¡r los datos del cuadripolo a borrar por los del
		  £ltimo cuadripolo, tambi‚n en el fichero temporal */
	       _llseek (hFichero, npIV->n_cuad  * sizeof (datos_cuad), 0);
	       _lread  (hFichero, (LPSTR) &datos_cuad, sizeof (datos_cuad));
	       _llseek (hFichero, i * sizeof (datos_cuad), 0);
	       _lwrite (hFichero, (LPSTR) &datos_cuad, sizeof (datos_cuad));
	      }
	    }
	   else
	    {
	     if ((C(i,0) > bloque.left) && (C(i,2) < bloque.right)  &&
		 (C(i,1) > bloque.top)  && (C(i,3) < bloque.bottom))
	      {
	       C(i,0) += nDesplx;
	       C(i,1) += nDesply;
	      }
	    }
	  }
	 LocalUnlock (npIV->hmemCuad);

	 /* Cerramos el fichero temporal de cuadripolos */
	 _lclose (hFichero);

	 /* Borramos ¢ movemos cables */
	 cables = (short *) LocalLock (npIV->hmemCables);
	 k = 0;
	 for (i = 0; i < npIV->n_cables; ++i)
	  {
	   if (tipo_dibujo == GRUPO)
	    {
	     while ((bloque.left   < min (c(i,0), c(i,2))) &&
		    (bloque.right  > max (c(i,0), c(i,2))) &&
		    (bloque.top    < min (c(i,1), c(i,3))) &&
		    (bloque.bottom > max (c(i,1), c(i,3))) &&
		    (i < npIV->n_cables))
	      {
	       --npIV->n_cables;
	       memcpy (cables + 6*i, cables + 6*npIV->n_cables, 6*sizeof(short));
	      }
	    }
	   else
	    {
	     if ((bloque.left   < min (c(i,0), c(i,2))) &&
		 (bloque.right  > max (c(i,0), c(i,2))) &&
		 (bloque.top    < min (c(i,1), c(i,3))) &&
		 (bloque.bottom > max (c(i,1), c(i,3))))
	      {
	       /* Si estamos Desplazando, creamos un cable entre las
		  coordenadas anteriores y las actuales */
	       if (tipo_dibujo == DESPLAZAR)
		{
		 /* Si el desplazamiento es a la derecha, el nuevo cable une
		    el extremo izquierdo del cable desplazado con su anterior
		    ubicaci¢n. Si es a la izquierda, se une el extremo dcho.
		    del cable. Igual para desplazamientos arriba ¢ abajo */
		 c(npIV->n_cables + k,0) = (nDesplx > 0) ? min (c(i,0), c(i,2)) :
							   max (c(i,0), c(i,2));
		 c(npIV->n_cables + k,1) = (nDesply > 0) ? min (c(i,1), c(i,3)) :
							   max (c(i,1), c(i,3));
		 c(npIV->n_cables + k,2) = c(npIV->n_cables + k,0) + nDesplx;
		 c(npIV->n_cables + k,3) = c(npIV->n_cables + k,1) + nDesply;
		 c(npIV->n_cables + k,4) = horiz;
		 c(npIV->n_cables + k,5) = vert;
		 ++k;
		}

	       c(i,0) += nDesplx;
	       c(i,2) += nDesplx;
	       c(i,1) += nDesply;
	       c(i,3) += nDesply;
	      }
	    }
	  }
	 npIV->n_cables += k;
	 LocalUnlock (npIV->hmemCables);

	 /* Borramos ¢ movemos nudos */
	 nudos = (short *) LocalLock (npIV->hmemNudos);
	 for (i = 0; i < npIV->n_nudos; ++i)
	  {
	   if (tipo_dibujo == BLOQUE)
	    {
	     while ((n(i,0) > bloque.left) && (n(i,0) < bloque.right) &&
		    (n(i,1) > bloque.top) && (n(i,1) < bloque.bottom) &&
		    (i < npIV->n_nudos))
	      {
	       --npIV->n_nudos;
	       memcpy (nudos + 3*i, nudos + 3*npIV->n_nudos, 3*sizeof(short));
	       i = 0;
	      }
	    }
	   else
	    {
	     if ((n(i,0) > bloque.left) && (n(i,0) < bloque.right) &&
		 (n(i,1) > bloque.top) && (n(i,1) < bloque.bottom))
	      {
	       n(i,0) += nDesplx;
	       n(i,1) += nDesply;
	      }
	    }
	  }
	 LocalUnlock (npIV->hmemNudos);

	 /* Borrar todo lo que caiga dentro del/los bloque/s */
	 if ((tipo_dibujo == MOVER) || (tipo_dibujo == DESPLAZAR))
	  {
	   bloque_fijo.left   -= npIV->nHposScrl * SCRLDESPL;
	   bloque_fijo.right  -= npIV->nHposScrl * SCRLDESPL - 1;
	   bloque_fijo.top    -= npIV->nVposScrl * SCRLDESPL;
	   bloque_fijo.bottom -= npIV->nVposScrl * SCRLDESPL - 1;
	   InvalidateRect (hwnd, &bloque_fijo, TRUE);
	  }

	 bloque.left   -= npIV->nHposScrl * SCRLDESPL;
	 bloque.right  -= npIV->nHposScrl * SCRLDESPL - 1;
	 bloque.top    -= npIV->nVposScrl * SCRLDESPL;
	 bloque.bottom -= npIV->nVposScrl * SCRLDESPL - 1;
	 InvalidateRect (hwnd, &bloque, TRUE);

	 UpdateWindow (hwnd);
	}

       /* pero, si estamos definiendo un cuadripolo.. */
       else
	 if (tipo_dibujo == CUADRIPOLO)
	  {
	   /* ..registramos que hemos terminado el bloque */
	   empezado = 0;

	   /* Guardamos los datos en la lista */
	   cuad = (short *) LocalLock (npIV->hmemCuad);

	   C(npIV->n_cuad,0) = bloque.left;
	   C(npIV->n_cuad,1) = bloque.top;
	   C(npIV->n_cuad,2) = bloque.right;
	   C(npIV->n_cuad,3) = bloque.bottom;
	   C(npIV->n_cuad,4) = horiz;

	   /* Dibujamos el cuadripolo */
	   dibujar_recuadro   (npIV, hdc, bloque.left, bloque.top,
			       bloque.right  - bloque.left,
			       bloque.bottom - bloque.top, FALSE);
	   dibujar_cuadripolo (npIV, hdc, bloque.left, bloque.top,
			       bloque.right  - bloque.left,
			       bloque.bottom - bloque.top,
			       horiz, "          ", TRUE);

	   /* Registramos el cuadripolo en el fichero temporal */
	   registrar_cuadripolo (npIV);
	   ++npIV->n_cuad;
	  }

	 /* y si, finalmente, estamos seleccionando un grupo.. */
	 else
	  {
	   /* ..registramos que hemos terminado de seleccionar */
	   empezado = 2;

	   /* Guardamos el bloque seleccionado */
	   bloque_fijo = bloque;
	  }
      }

     /* Liberamos el contexto de dispositivo */
     ReleaseDC (hwnd, hdc);

     break;
    }
   }

   /* Desbloqueamos los datos de la ventana de esquema */
   LocalUnlock (hMemIV);

   return (0);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDM_GUARDAR:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     /* Almacenamos el fichero de esquema */
     guardar_esquema (npIV);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     return (0);
    }

    case IDM_GUARDARCOMO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     /* Obtenemos la dir. del proc. para la Caja de Di logo de Guardar */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogGuardar,
					   hInstance);

     /* Mediante dicha caja pedimos el nombre para el esquema */
     strcpy (path,     npIV->path);
     strcpy (nom_fich, npIV->nom_fich);
     if (DialogBoxParam (hInstance, szNameGuardar, hwnd, lpfnProcDialogBox,
			 MAKELONG (0, (WORD) npIV)))
      {
       /* Ponemos el posiblemente nuevo nombre en la barra de t¡tulo */
       strcpy (npIV->path,     path);
       strcpy (npIV->nom_fich, nom_fich);
       strcpy (nom, path);
       strcat (nom, nom_fich);
       if (strlen (nom) > 38)
	 strcpy (szBuffer, "..");
       else
	 szBuffer[0] = 0;
       strcat (szBuffer, nom + max (0, (int)(strlen (nom) - 38)));
       SetWindowText (hwnd, szBuffer);
      }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     /* Desbloqueamos los datos referentes a la ventana */
     LocalUnlock (hMemIV);

     return (0);
    }

    case IDM_PUNTERO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     /* Borramos lo que estuvi‚ramos dibujando antes */
     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     tipo_dibujo = 0;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (NULL, IDC_ARROW));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_RESIST:
    case IDM_CONDENS:
    case IDM_BOBINA:
    case IDM_BIPOLAR:
    case IDM_JFET:
    case IDM_MOSFET:
    case IDM_DIODO:
    case IDM_DIODOZ:
    case IDM_DIODOV:
    case IDM_BATERIA:
    case IDM_GENV:
    case IDM_GENI:
    case IDM_GENVDEPV:
    case IDM_GENVDEPI:
    case IDM_GENIDEPV:
    case IDM_GENIDEPI:
    case IDM_VOLT:
    case IDM_AMPER:
    case IDM_OTRO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     /* Borramos lo que estuvi‚ramos dibujando antes */
     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

     tipo_dibujo = ELEMENTO;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (hInstance, "PINZA"));
     DestroyCursor (hCursor);

     switch (wParam)
     {
      case IDM_RESIST:
	e(npIV->n_elems, 0) = RESISTENCIA;
	break;

      case IDM_CONDENS:
	e(npIV->n_elems, 0) = CONDENSADOR;
	break;

      case IDM_BOBINA:
	e(npIV->n_elems, 0) = BOBINA;
	break;

      case IDM_BIPOLAR:
	e(npIV->n_elems, 0) = TRANSISTOR_BN;
	break;

      case IDM_JFET:
	e(npIV->n_elems, 0) = TRANSISTOR_JN;
	break;

      case IDM_MOSFET:
	e(npIV->n_elems, 0) = TRANSISTOR_MN;
	break;

      case IDM_DIODO:
	e(npIV->n_elems, 0) = DIODO;
	break;

      case IDM_DIODOZ:
	e(npIV->n_elems, 0) = DIODOZ;
	break;

      case IDM_DIODOV:
	e(npIV->n_elems, 0) = DIODOV;
	break;

      case IDM_BATERIA:
	e(npIV->n_elems, 0) = BATERIA;
	break;

      case IDM_GENV:
	e(npIV->n_elems, 0) = GENV;
	break;

      case IDM_GENI:
	e(npIV->n_elems, 0) = GENI;
	break;

      case IDM_GENVDEPV:
	e(npIV->n_elems, 0) = GENVDEPV;
	break;

      case IDM_GENVDEPI:
	e(npIV->n_elems, 0) = GENVDEPI;
	break;

      case IDM_GENIDEPV:
	e(npIV->n_elems, 0) = GENIDEPV;
	break;

      case IDM_GENIDEPI:
	e(npIV->n_elems, 0) = GENIDEPI;
	break;

      case IDM_VOLT:
	e(npIV->n_elems, 0) = VOLTIMETRO;
	break;

      case IDM_AMPER:
	e(npIV->n_elems, 0) = AMPERIMETRO;
	break;

      case IDM_OTRO:
	e(npIV->n_elems, 0) = METAFILE;
	break;
     }

     /* Obtenemos las coordenadas del mouse */
     GetCursorPos (&point);
     ScreenToClient (hwnd, &point);
     e(npIV->n_elems, 1) = point.x;
     e(npIV->n_elems, 2) = point.y;

     /* Pintamos el elemento */
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     ReleaseDC (hwnd, hdc);
     return (0);
    }

    case IDM_CABLE:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     tipo_dibujo = CABLE;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (hInstance, "ALICATE"));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_NUDO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     tipo_dibujo = NUDO;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (hInstance, "SOLDADOR"));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_MASA:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     tipo_dibujo = MASA;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (NULL, IDC_CROSS));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_CUAD:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     tipo_dibujo = CUADRIPOLO;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (NULL, IDC_CROSS));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_HORIZ:
    case IDM_VERT:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     hdc = GetDC (hwnd);

     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     dibujar (npIV, hdc, tipo_dibujo, vert, horiz, espejo, empezado, hMF, FALSE);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     ReleaseDC (hwnd, hdc);

     horiz = 1 - horiz;
     vert  = 1 - vert;
     hMenu = GetMenu (hwndMarco);
     CheckMenuItem (GetSubMenu (hMenu, 1), IDM_HORIZ,
		    (horiz) ? MF_CHECKED : MF_UNCHECKED);
     CheckMenuItem (GetSubMenu (hMenu, 1), IDM_VERT,
		    (vert) ?  MF_CHECKED : MF_UNCHECKED);
     return (0);
    }

    case IDM_ESPEJO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     hdc = GetDC (hwnd);

     /* Borramos lo que estuviera dibujado antes */
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

     espejo = 1 - espejo;
     hMenu = GetMenu (hwndMarco);
     CheckMenuItem (GetSubMenu (hMenu, 1), IDM_ESPEJO,
		    (espejo) ? MF_CHECKED : MF_UNCHECKED);

     /* Redibujamos el dibujo anterior */
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     ReleaseDC (hwnd, hdc);

     return (0);
    }

    case IDM_OBJETO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     /* Borramos lo que estuviera dibujado antes */
     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     tipo_dibujo = BORRAR;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (hInstance, "BORRADOR"));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_GRUPO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     /* Borramos lo que estuviera dibujado antes */
     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     tipo_dibujo = GRUPO;
     empezado = FALSE;

     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (NULL, IDC_CROSS));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_MOVER:
    case IDM_DESPLAZ:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     /* Borramos lo que estuviera dibujado antes */
     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     ReleaseDC (hwnd, hdc);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     tipo_dibujo = (wParam == IDM_MOVER) ? MOVER : DESPLAZAR;
     empezado = FALSE;

     /* Cambiamos el cursor a la cruz horizontal */
     hCursor = GetClassWord (hwnd, GCW_HCURSOR);
     SetClassWord (hwnd, GCW_HCURSOR, LoadCursor (NULL, IDC_CROSS));
     DestroyCursor (hCursor);

     return (0);
    }

    case IDM_REJILLA:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

     /* Borramos lo que estuviera dibujado antes */
     hdc = GetDC (hwnd);
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);
     rejilla = !rejilla;
     hMenu = GetMenu (hwndMarco);
     CheckMenuItem (GetSubMenu (hMenu, 1), IDM_REJILLA,
		    (rejilla) ? MF_CHECKED : MF_UNCHECKED);
     if (rejilla)
      {
       GetClientRect (hwnd, &rect);
       dibujar_rejilla (hdc, rect);
      }
     else
      {
       InvalidateRect (hwnd, NULL, TRUE);
       UpdateWindow (hwnd);
      }

     /* Redibujamos el dibujo anterior */
     dibujar (npIV, hdc, tipo_dibujo, horiz, vert, espejo, empezado, hMF, FALSE);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIV);

     /* Liberamos el contexto de dispositivo */
     ReleaseDC (hwnd, hdc);

     return (0);
    }

    case IDM_GRANDE:
    case IDM_PEQUENO:
    {
     /* Obtenemos los datos de la impresora del archivo "WIN.INI" */
     GetProfileString ("Windows", "device", NULL, szBuffer, 30);
     lstrcpy (szPrinterName,   strtok (szBuffer, ","));
     lstrcpy (szPrinterDriver, strtok (NULL,     ","));
     lstrcpy (szPrinterPort,   strtok (NULL,     ","));

     /* Creamos un contexto de dispositivo para la impresora */
     hdc = CreateDC (szPrinterDriver, szPrinterName, szPrinterPort, NULL);

     /* Activamos la recogida de datos en la p gina */
     Escape (hdc, STARTDOC, lstrlen (nom_fich), nom_fich,NULL);

     /* Enviamos el circuito a la p gina */
     hMemIV = GetWindowWord (hwnd, 0);
     npIV = (InfoVentana NEAR *) LocalLock (hMemIV);
     rect.left   = -npIV->nHposScrl * SCRLDESPL;
     rect.top    = -npIV->nVposScrl * SCRLDESPL;
     rect.right  = 32767 - npIV->nHposScrl * SCRLDESPL;
     rect.bottom = 32767 - npIV->nVposScrl * SCRLDESPL;
     dibujar_circuito (npIV, hdc, rect);
     LocalUnlock (hMemIV);

     /* Finalizamos la pagina */
     Escape (hdc, NEWFRAME, NULL, NULL, NULL);

     /* Finalizamos la recogida de datos */
     Escape (hdc, ENDDOC, NULL, NULL, NULL);

     /* Eliminamos el contexto */
     DeleteDC (hdc);

     return (0);
    }
   }

   break;
  }

  case WM_SYSCOMMAND:
  {
   /* Si se ha elegido Cerrar en el men£ de sistema.. */
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     /* ..emulamos el proceso de 'Salir' */
     SendMessage (hwnd, WM_CLOSE, 0, 0L);
     return (0);
    }

   break;
  }

  case WM_HSCROLL:
  {
   /* Obtenemos los datos de la ventana de esquema */
   hMemIV = GetWindowWord (hwnd, 0);
   npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

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
     nIncScrl = LOWORD(lParam) - npIV->nHposScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   /* Modificamos las posiciones de Scroll */
   if (nIncScrl = max (-npIV->nHposScrl,
		       min (nIncScrl, npIV->nHmaxScrl - npIV->nHposScrl)))
    {
     npIV->nHposScrl += nIncScrl;
     ScrollWindow (hwnd, -SCRLDESPL * nIncScrl, 0, NULL, NULL);
     SetScrollPos (hwnd, SB_HORZ, npIV->nHposScrl, TRUE);
    }

   /* Desbloqueamos los datos de la ventana de esquema */
   LocalUnlock (hMemIV);

   return (0);
  }

  case WM_VSCROLL:
  {
   /* Obtenemos los datos de la ventana de esquema */
   hMemIV = GetWindowWord (hwnd, 0);
   npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

   switch (wParam)
   {
    case SB_TOP:
    {
     nIncScrl = -npIV->nVposScrl;
     break;
    }

    case SB_BOTTOM:
    {
     nIncScrl = npIV->nVmaxScrl - npIV->nVposScrl;
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
     nIncScrl = LOWORD(lParam) - npIV->nVposScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   /* Modificamos las posiciones de Scroll */
   if (nIncScrl = max (-npIV->nVposScrl,
		       min (nIncScrl, npIV->nVmaxScrl - npIV->nVposScrl)))
    {
     npIV->nVposScrl += nIncScrl;
     ScrollWindow (hwnd, 0, -SCRLDESPL * nIncScrl, NULL, NULL);
     SetScrollPos (hwnd, SB_VERT, npIV->nVposScrl, TRUE);
    }

   /* Desbloqueamos los datos de la ventana de esquema */
   LocalUnlock (hMemIV);

   return (0);
  }

  case WM_QUERYENDSESSION:
  case WM_CLOSE:
  {
   /* Obtenemos los datos referentes a la ventana de esquema */
   hMemIV = GetWindowWord (hwnd, 0);
   npIV = (InfoVentana NEAR *) LocalLock (hMemIV);

   /* Guardamos los datos del esquema, si se desea */
   if ((npIV->n_elems > 0) || (npIV->n_cables > 0) ||
       (npIV->n_nudos > 0) || (npIV->n_cuad > 0))
    {
     resp = MessageBox (hwnd, "¿Desea registrar los cambios?",
			"Cerrando el Esquema",
			MB_ICONINFORMATION | MB_YESNOCANCEL);

     if (resp == IDCANCEL)
      {
       LocalUnlock (hMemIV);
       return (0);
      }
     else
      {
       if (resp == IDYES)
	 SendMessage (hwnd, WM_COMMAND, IDM_GUARDAR, 0);

       /* Borramos los ficheros temporales */
       for (i = TMPELEMS; i <= TMPCUADS; ++i)
	{
	 strcpy (nom, npIV->path);
	 strcat (nom, npIV->nom_fich);
	 strcat (nom, szExtFich[i]);
	 OpenFile (nom, &of, OF_DELETE);
	}

       /* Anulamos la operaci¢n en que estuvi‚semos */
       tipo_dibujo = 0;
       empezado = FALSE;

       /* Liberamos la memoria usada */
       liberar_memoria_esquema (npIV);

       /* Eliminamos el Font creado al seleccionar Abrir ¢ Nuevo */
       DeleteObject (npIV->hNuevoFont);
      }
    }

   /* Desbloqueamos los datos */
   LocalUnlock (hMemIV);

   /* Pasamos a DefMDIChildProc */
   break;
  }

  case WM_DESTROY:
  {
   hMemIV = GetWindowWord (hwnd, 0);
   LocalFree (hMemIV);
   --nEsquemas;
   return (0);
  }
 }

 return (DefMDIChildProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long FAR PASCAL _export ProcVentBbl (HWND hwnd, WORD message,
				     WORD wParam, LONG lParam)
{
 static HWND      hwndMarco, hwndCliente, hwndActiva;
 static HANDLE    hInstance;
 static char      szNameGuardar[]  = "GUARDAR",
		  szNameAddSym[]   = "ADDSYM",
		  szNameBuscSym[]  = "BUSCSYM",
		  szNameListSym[]  = "LISTSYM",
		  szNameSuprBibl[] = "SUPRBIBL";
 HDC		  hdc;
 HANDLE           hMF;
 PAINTSTRUCT      ps;
 OFSTRUCT         of;
 FARPROC      	  lpfnProcDialogBox;
 LOCALHANDLE      hMemIb;
 InfoBiblio NEAR *npIb;
 char         	  nom[84], szBuffer[60];
 int		  resp;
 short		  nIncScrl;

 switch (message)
 {
  case WM_CREATE:
  {
   /* Obtenemos el handle a la ocurrencia del m¢dulo, que se guarda forever */
   hInstance = ((LPCREATESTRUCT) lParam)->hInstance;

   /* Obtenemos handles a las ventanas padre y abuela */
   hwndCliente = GetParent (hwnd);
   hwndMarco   = GetParent (hwndCliente);

   /* Creamos la estructura de datos de la ventana de Biblioteca */
   hMemIb = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (InfoBiblio));
   npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

   /* La inicializamos */
   npIb->nVmaxScrl = npIb->n_elems - 1;
   npIb->nVposScrl = 0;
   npIb->minimizada = 0;
   npIb->cambios    = 0;
   strcpy (npIb->path, path);
   strcpy (npIb->nom_fich, NOMBRE);

   /* Abrimos el fichero ¡ndice */
   strcpy (nom, path);
   strcat (nom, nom_fich);
   strcat (nom, szExtFich[TV_BIBLIO]);
   npIb->hIndice = OpenFile (nom, &of, OF_CREATE);
   npIb->nPos = 0;

   /* Transferimos dichos datos a la ventana */
   SetWindowWord (hwnd, 0, hMemIb);

   /* Desbloqueamos los datos */
   LocalUnlock (hMemIb);

   return (0);
  }

  case WM_SIZE:
  {
   /* Obtenemos los datos de la ventana */
   hMemIb = GetWindowWord (hwnd, 0);
   npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

   /* Fijamos los rangos y posiciones de las barras de Scroll */
   npIb->nHmaxScrl = max (0, MF_TAMHORZ - (short) LOWORD(lParam));
   npIb->nHposScrl = min (npIb->nHposScrl, npIb->nHmaxScrl);
   SetScrollRange (hwnd, SB_HORZ, 0, npIb->nHmaxScrl, FALSE);
   SetScrollPos   (hwnd, SB_HORZ, npIb->nHposScrl, TRUE);

   /* Desbloqueamos los datos */
   LocalUnlock (hMemIb);

   break;
  }

  case WM_PAINT:
  {
   /* Obtenemos el contexto de la ventana */
   hdc = BeginPaint (hwnd, &ps);

   /* Redibujamos lo que haga falta */
   hMemIb = GetWindowWord (hwnd, 0);
   npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);
   dibujar_biblioteca (npIb, hdc, ps.rcPaint);
   LocalUnlock (hMemIb);
    
   /* Liberamos el contexto */
   EndPaint (hwnd, &ps);
   break;
  }

  case WM_MDIACTIVATE:
  {
   /* Si obtenemos el foco de entrada ponemos el men£ Bibliotecas */
   if (wParam == TRUE)
    {
     hwndActiva = hwnd;
     tvactiva   = TV_BIBLIO;
     SendMessage (hwndCliente, WM_MDISETMENU, 0,
		  MAKELONG(hMenuBiblio, hMenuBiblioVentana));
    }

   /* pero, si perdemos el foco, ponemos el men£ Inicial */
   else
     SendMessage (hwndCliente, WM_MDISETMENU, 0, MAKELONG(hMenuInicial, 0));

   DrawMenuBar (hwndMarco);
   return (0);
  }

  case WM_COMMAND:
  {
   switch (wParam)
   {
    case IDM_GUARDAR:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIb = GetWindowWord (hwnd, 0);
     npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

     /* Almacenamos el fichero de esquema */
     guardar_biblioteca (npIb);

     return (0);
    }

    case IDM_GUARDARCOMO:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIb = GetWindowWord (hwnd, 0);
     npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

     /* Fijamos el tipo de ventana a guardar */
     tipo_ventana = TV_BIBLIO;

     /* Obtenemos la dir. del proc. para la Caja de Di logo de Guardar */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogGuardar,
					   hInstance);

     /* Mediante dicha caja pedimos el nombre para el esquema */
     strcpy (path,     npIb->path);
     strcpy (nom_fich, npIb->nom_fich);
     if (DialogBoxParam (hInstance, szNameGuardar, hwnd, lpfnProcDialogBox,
			 MAKELONG (0, (WORD) npIb)))
      {
       /* Ponemos el posiblemente nuevo nombre en la barra de t¡tulo */
       strcpy (npIb->path,     path);
       strcpy (npIb->nom_fich, nom_fich);
       strcpy (szBuffer, "Biblioteca - ");
       strcpy (nom, path);
       strcat (nom, nom_fich);
       if (strlen (nom) > 38)
	 strcat (szBuffer, "..");
       strcat (szBuffer, nom + max (0, (int)(strlen (nom) - 38)));
       SetWindowText (hwnd, szBuffer);
      }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     /* Desbloqueamos los datos referentes a la ventana */
     LocalUnlock (hMemIb);

     return (0);
    }

    case IDM_SUPRBIBL:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIb = GetWindowWord (hwnd, 0);
     npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

     /* Obtenemos la dir. del proc. para la Caja de Di logo de Eliminar
	Biblioteca */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogSuprBibl,
					   hInstance);

     /* Mediante dicha caja eliminamos una biblioteca */
     DialogBoxParam (hInstance, szNameSuprBibl, hwnd, lpfnProcDialogBox,
		     MAKELONG(0, (WORD) npIb));

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIb);

     return (0);
    }

    case IDM_ADDSYM:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIb = GetWindowWord (hwnd, 0);
     npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

     /* Obtenemos la dir. del proc. para la Caja de Di logo de A¤adir s¡mbolo */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogAddSym,
					   hInstance);

     /* Mediante dicha caja a¤adimos un elemento a una biblioteca */
     DialogBoxParam (hInstance, szNameAddSym, hwnd, lpfnProcDialogBox,
		     MAKELONG((WORD) 0, (WORD) npIb));

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIb);

     /* Invalidamos la ventana */
     InvalidateRect (hwnd, NULL, TRUE);

     return (0);
    }

    case IDM_BUSCSYM:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIb = GetWindowWord (hwnd, 0);
     npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

     /* Obtenemos la dir. del proc. para la Caja de Di logo de Buscar s¡mbolo */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogBuscSym,
					   hInstance);

     /* Mediante dicha caja localizamos un elemento en la biblioteca */
     if (DialogBoxParam (hInstance, szNameBuscSym, hwnd, lpfnProcDialogBox,
			 MAKELONG ((WORD) 0, (WORD) npIb)))
      {
       if (hMF != NULL)
	 DeleteMetaFile (hMF);
       hMF = GetMetaFile (strcat (strcpy (szBuffer, ""),
				  szExtFich[TV_METAFILE]));
       PostMessage (hwnd, WM_COMMAND, IDM_OTRO, 0L);
      }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIb);

     return (0);
    }

    case IDM_LISTSYM:
    {
     /* Bloqueamos los datos referentes a la ventana */
     hMemIb = GetWindowWord (hwnd, 0);
     npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

     /* Obtenemos la dir. del proc. para la Caja de Di logo de Listar s¡mbolos */
     lpfnProcDialogBox = MakeProcInstance ((FARPROC) ProcDialogListSym,
					   hInstance);

     /* Mediante dicha caja localizamos un elemento en la biblioteca */
     if (DialogBoxParam (hInstance, szNameListSym, hwnd, lpfnProcDialogBox,
			 MAKELONG ((WORD) 0, (WORD) npIb)))
      {
       if (hMF != NULL)
	 DeleteMetaFile (hMF);
       hMF = GetMetaFile (strcat (strcpy (szBuffer, ""),
       				  szExtFich[TV_METAFILE]));
       PostMessage (hwnd, WM_COMMAND, IDM_OTRO, 0L);
      }

     /* Liberamos el thunk a la Caja de Di logo */
     FreeProcInstance (lpfnProcDialogBox);

     /* Desbloqueamos los datos */
     LocalUnlock (hMemIb);

     return (0);
    }

    /* Otras opciones del men£ */
   }

   break;
  }

  case WM_LBUTTONUP:
  {
   break;
  }

  case WM_SYSCOMMAND:
  {
   /* Si se ha elegido Cerrar en el men£ de sistema.. */
   if ((wParam & 0xFFF0) == SC_CLOSE)
    {
     /* ..emulamos el proceso de 'Salir' */
     SendMessage (hwnd, WM_CLOSE, 0, 0L);
     return (0);
    }

   break;
  }

  case WM_HSCROLL:
  {
   /* Obtenemos los datos de la ventana de biblioteca */
   hMemIb = GetWindowWord (hwnd, 0);
   npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

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
     nIncScrl = LOWORD(lParam) - npIb->nHposScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   /* Modificamos las posiciones de Scroll */
   if (nIncScrl = max (-npIb->nHposScrl,
		       min (nIncScrl, npIb->nHmaxScrl - npIb->nHposScrl)))
    {
     npIb->nHposScrl += nIncScrl;
     ScrollWindow (hwnd, -nIncScrl, 0, NULL, NULL);
     SetScrollPos (hwnd, SB_HORZ, npIb->nHposScrl, TRUE);
    }

   /* Desbloqueamos los datos de la ventana de biblioteca */
   LocalUnlock (hMemIb);

   return (0);
  }

  case WM_VSCROLL:
  {
   /* Obtenemos los datos de la ventana de biblioteca */
   hMemIb = GetWindowWord (hwnd, 0);
   npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

   switch (wParam)
   {
    case SB_TOP:
    {
     nIncScrl = -npIb->nVposScrl;
     break;
    }

    case SB_BOTTOM:
    {
     nIncScrl = npIb->nVmaxScrl - npIb->nVposScrl;
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
     nIncScrl = LOWORD(lParam) - npIb->nVposScrl;
     break;
    }

    default:
      nIncScrl = 0;
   }

   /* Modificamos las posiciones de Scroll */
   if (nIncScrl = max (-npIb->nVposScrl,
		       min (nIncScrl, npIb->nVmaxScrl - npIb->nVposScrl)))
    {
     npIb->nVposScrl += nIncScrl;
     ScrollWindow (hwnd, 0, -MF_TAMVERT * nIncScrl, NULL, NULL);
     SetScrollPos (hwnd, SB_VERT, npIb->nVposScrl, TRUE);
    }

   /* Desbloqueamos los datos de la ventana de biblioteca */
   LocalUnlock (hMemIb);

   return (0);
  }

  case WM_QUERYENDSESSION:
  case WM_CLOSE:
  {
   /* Obtenemos los datos referentes a la ventana de Biblioteca */
   hMemIb = GetWindowWord (hwnd, 0);
   npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);

   /* Guardamos los datos de la biblioteca, si se desea */
   if (npIb->n_elems > 0)
    {
     resp = MessageBox (hwnd, "¿Desea registrar los cambios?",
			"Cerrando la Biblioteca",
			MB_ICONINFORMATION | MB_YESNOCANCEL);

     if (resp == IDCANCEL)
      {
       LocalUnlock (hMemIb);
       return (0);
      }
     else
       if (resp == IDYES)
	 SendMessage (hwnd, WM_COMMAND, IDM_GUARDAR, 0);
    }

   /* Desbloqueamos los datos */
   LocalUnlock (hMemIb);

   /* Pasamos a DefMDIChildProc */
   break;
  }

  case WM_DESTROY:
  {
   /* Cerramos el fichero ¡ndice */
   hMemIb = GetWindowWord (hwnd, 0);
   npIb = (InfoBiblio NEAR *) LocalLock (hMemIb);
   _lclose (npIb->hIndice);
   LocalUnlock (hMemIb);

   /* Liberamos la memoria usada */
   LocalFree (hMemIb);

   /* Actualizamos el n§ de ventanas de Biblioteca */
   --nBiblios;

   return (0);
  }
 }

 return (DefMDIChildProc (hwnd, message, wParam, lParam));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int buscar_por_coords (void NEAR *npI, short tipo, short cx, short cy)
{
 HDC         hdc;
 TEXTMETRIC  tm;
 InfoBloque  NEAR *npIB;
 InfoVentana NEAR *npIV;
 int   i;
 short xini, xfin, yini, yfin;

 switch (tipo)
 {
  case ELEMENTO:
  {
   npIV = (InfoVentana NEAR *) npI;
   elementos = (short *) LocalLock (npIV->hmemElems);

   for (i = 0; i < npIV->n_elems; ++i)
     if (e(i,3))
      {
       if ((cx > e(i,1) + 8) && (cx < e(i,1) + 32))
	 if ((cy > e(i,2) - 8) && (cy < e(i,2) + 8))
	   return (i);
      }
     else
      {
       if ((cx > e(i,1) - 8) && (cx < e(i,1) + 8))
	 if ((cy > e(i,2) + 8) && (cy < e(i,2) + 32))
	   return (i);
      }

   LocalUnlock (npIV->hmemElems);
   break;
  }

  case NUDO:
  {
   npIV = (InfoVentana NEAR *) npI;
   nudos = (short *) LocalLock (npIV->hmemNudos);

   for (i = 0; i < npIV->n_nudos; ++i)
    {
     if ((cx >= n(i,0) - 2) && (cx <= n(i,0) + 2))
       if ((cy >= n(i,1) - 2) && (cy <= n(i,1) + 2))
	 return (i);
    }

   LocalUnlock (npIV->hmemNudos);
   break;
  }

  case CABLE:
  {
   /* Aqu¡ da igual el tipo de puntero; tanto los 'InfoBloque' como los
      'InfoVentana' tienen 'hmemCables' y 'n_cables' en la misma posici¢n */
   npIV = (InfoVentana NEAR *) npI;
   cables = (short *) LocalLock (npIV->hmemCables);

   for (i = 0; i < npIV->n_cables; ++i)
    {
     xini = min (c(i,0), c(i,2));
     xfin = max (c(i,0), c(i,2));
     yini = min (c(i,1), c(i,3));
     yfin = max (c(i,1), c(i,3));

     /* Si el cable es horizontal.. */
     if (c(i,4))
      {
       if (((cy == c(i,1)) && (cx >= xini) && (cx <= xfin)) ||
	   ((cx == c(i,2)) && (cy >= yini) && (cy <= yfin)))
	 return (i);
      }

     /* pero, si es vertical.. */
     else
      {
       if (((cx == c(i,0)) && (cy >= yini) && (cy <= yfin)) ||
	   ((cy == c(i,3)) && (cx >= xini) && (cx <= xfin)))
	 return (i);
      }
    }
   LocalUnlock (npIV->hmemCables);
   break;
  }

  case CUADRIPOLO:
  {
   npIV = (InfoVentana NEAR *) npI;
   cuad = (short *) LocalLock (npIV->hmemCuad);

   for (i = 0; i < npIV->n_cuad; ++i)
     if ((cx > C(i,0)) && (cx < C(i,2)) && (cy > C(i,1)) && (cy < C(i,3)))
       return (i);

   LocalUnlock (npIV->hmemCuad);
   break;
  }

  case BLOQUE:
  {
   npIB = (InfoBloque NEAR *) npI;
   bloques = (short *) LocalLock (npIB->hmemBloques);

   for (i = 0; i < npIB->n_bloques; ++i)
     if ((cx > b(i,0)) && (cx < b(i,2)) && (cy > b(i,1)) && (cy < b(i,3)))
      return (i);

   LocalUnlock (npIB->hmemBloques);
   break;
  }

  case PUERTO:
  {
   npIB = (InfoBloque NEAR *) npI;
   puertos = (short *) LocalLock (npIB->hmemPuertos);

   for (i = 0; i < npIB->n_puertos; ++i)
     if (p(i,3))
      {
       if ((cx > p(i,0)) && (cx < p(i,0) + TAMHPORT) &&
	   (cy > p(i,1)) && (cy < p(i,1) + TAMVPORT))
	 return(i);
      }
     else
      {
       if ((cx > p(i,0)) && (cx < p(i,0) + TAMVPORT) &&
	   (cy > p(i,1)) && (cy < p(i,1) + TAMHPORT))
	 return (i);
      }

   LocalUnlock (npIB->hmemPuertos);
   break;
  }

  case TEXTO:
  {
   npIB = (InfoBloque NEAR *) npI;
   textos = (short *) LocalLock (npIB->hmemTextos);

   /* De momento, nos conformamos con las medidas est ndar */
   tm.tmAveCharWidth = 20;
   tm.tmHeight       = 20;

   for (i = 0; i < npIB->n_textos; ++i);
     if (t(i,3))
      {
       if ((cx > t(i,0)) && (cx < t(i,0) + t(i,5)*tm.tmAveCharWidth) &&
	   (cy > t(i,1)) && (cy < t(i,1) + tm.tmHeight))
	 return (i);
      }
     else
      {
       if ((cx > t(i,0)) && (cx < t(i,0) + tm.tmHeight) &&
	   (cy > t(i,1)) && (cy < t(i,1) + t(i,5)*tm.tmAveCharWidth))
	 return (i);
      }

   LocalUnlock (npIB->hmemTextos);
   break;
  }
 }

 return (-1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int asignar_memoria_diagrama (InfoBloque NEAR *npIB)
{
 npIB->hmemBloques = LocalAlloc (LMEM_NODISCARD | LMEM_MOVEABLE,
				 MAX_BLOQUES * 4 * sizeof (short));
 npIB->hmemCables  = LocalAlloc (LMEM_NODISCARD | LMEM_MOVEABLE,
				 MAX_CABLES  * 6 * sizeof (short));
 npIB->hmemTextos  = LocalAlloc (LMEM_NODISCARD | LMEM_MOVEABLE,
				 MAX_TEXTOS  * 5 * sizeof (short));
 npIB->hmemPuertos = LocalAlloc (LMEM_NODISCARD | LMEM_MOVEABLE,
				 MAX_PUERTOS * 11 * sizeof (short));

 return ((npIB->hmemBloques == NULL) || (npIB->hmemCables == NULL) ||
	 (npIB->hmemPuertos == NULL) || (npIB->hmemTextos == NULL));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int liberar_memoria_diagrama (InfoBloque NEAR *npIB)
{
 return (LocalFree (npIB->hmemBloques) || LocalFree (npIB->hmemCables) ||
	 LocalFree (npIB->hmemPuertos) || LocalFree (npIB->hmemTextos));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int asignar_memoria_esquema (InfoVentana NEAR *npIV)
{
 npIV->hmemElems =  LocalAlloc (LMEM_NODISCARD | LMEM_MOVEABLE,
				MAX_ELEMS  * 6 * sizeof (short));
 npIV->hmemCables = LocalAlloc (LMEM_NODISCARD | LMEM_MOVEABLE,
				MAX_CABLES * 6 * sizeof (short));
 npIV->hmemNudos =  LocalAlloc (LMEM_NODISCARD | LMEM_MOVEABLE,
				MAX_NUDOS  * 3 * sizeof (short));
 npIV->hmemCuad  =  LocalAlloc (LMEM_NODISCARD | LMEM_MOVEABLE,
				MAX_CUAD   * 5 * sizeof (short));

 return ((npIV->hmemElems == NULL) || (npIV->hmemCables == NULL) ||
	 (npIV->hmemNudos == NULL) || (npIV->hmemCuad   == NULL));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int liberar_memoria_esquema (InfoVentana NEAR *npIV)
{
 return (LocalFree (npIV->hmemElems) || LocalFree (npIV->hmemCables) ||
	 LocalFree (npIV->hmemNudos) || LocalFree (npIV->hmemCuad));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void registrar_elemento (InfoVentana NEAR *npIV, short tipo)
{
 static char tipos[] = { 'R', 'C', 'L', 'P', 'N', 'J', 'K', 'M', 'O', 'Q',
			 'Z', 'W', 'V', 'V', 'I', 'A', 'B', 'D', 'E', 'v', 'i'};
 static char nombs[] = { 'R', 'C', 'L', 'Q', 'Q', 'Q', 'Q', 'Q', 'Q', 'D',
			 'D', 'D', 'V', 'e', 'i', 'v', 'v', 'i', 'i', 'V', 'I'};
 static short nums[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
 OFSTRUCT of;
 elemento datos_elem;
 int      hFichero;
 short    n;
 char     nom[84], szBuffer[5];

 /* Inicializamos los par metros a valores por defecto */
 datos_elem.nombre[0] = nombs[tipo - 1];
 datos_elem.nombre[1] = 0;
 datos_elem.tipo[0]   = tipos[tipo - 1];
 datos_elem.tipo[1]   = 0;
 datos_elem.signal    = ((tipo == GENV) || (tipo == GENI)) ? 'S' : 'C';
 datos_elem.valor     = (((tipo >= TRANSISTOR_BP) && (tipo <= TRANSISTOR_MN)) ||
			 ((tipo >= DIODO) && (tipo <= DIODOV))) ? 1e-14 : 1;
 datos_elem.Tact      = 3600;
 datos_elem.fase      =
 datos_elem.caract.frec =
 datos_elem.caract.cond_inic =
 datos_elem.CT        = 0;

 switch (tipo)
 {
  case RESISTENCIA:
  case BOBINA:
  case CONDENSADOR:
  case BATERIA:
  {
   n = ++nums[tipo];
   break;
  }

  case DIODO:
  case DIODOZ:
  case DIODOV:
  {
   n = ++nums[4];
   datos_elem.f[0] = 0;        /* Rs  del diodo */
   datos_elem.f[1] = 0.4;      /* Vj  del diodo */
   datos_elem.f[2] = 2e-12;    /* Cj  del diodo */
   datos_elem.f[3] = 0;        /* tT  del diodo */
   datos_elem.f[4] = 0.5;      /* Mj  del diodo */
   datos_elem.f[5] = 0.5;      /* FC  del diodo */
   datos_elem.f[6] = INFINITO; /* Vbr del diodo */
   datos_elem.f[7] = 1e-10;    /* Ibv del diodo */
   datos_elem.f[8] = 1.11;     /* Eg  del diodo */

   break;
  }

  case TRANSISTOR_BP:
  case TRANSISTOR_BN:
  {
   n = ++nums[3];
   datos_elem.f[0] = 100;      /* Bf  del transistor */
   datos_elem.f[1] = 1;        /* Br  del transistor */
   datos_elem.f[2] =           /* Isc del transistor */
   datos_elem.f[3] = 0;        /* Ise del transistor */
   datos_elem.f[4] =           /* Cjc del transistor */
   datos_elem.f[5] = 2e-12;    /* Cje del transistor */
   datos_elem.f[6] =           /* tf  del transistor */
   datos_elem.f[7] = 0;        /* tr  del transistor */
   datos_elem.f[8] =           /* Vjc del transistor */
   datos_elem.f[9] = 0.75;     /* Vje del transistor */
   datos_elem.f[10] =          /* Mjc del transistor */
   datos_elem.f[11] = 0.33;    /* Mje del transistor */
   datos_elem.f[12] = 0.5;     /* FC  del transistor */
   datos_elem.f[13] =          /* Vaf del transistor */
   datos_elem.f[14] = INFINITO;/* Var del transistor */
   datos_elem.f[15] = 0;       /* XTB del transistor */
   datos_elem.f[16] = 1.11;    /* Eg  del transistor */
   datos_elem.f[17] =          /* Rb  del transistor */
   datos_elem.f[18] =          /* Rc  del transistor */
   datos_elem.f[19] = 0;       /* Re  del transistor */

   break;
  }


  case TRANSISTOR_JP:
  case TRANSISTOR_JN:
  {
   n = ++nums[3];
   datos_elem.f[0] = 1e-4;     /* BETA  del transistor JFET */
   datos_elem.f[1] = 0;        /* LANDA del transistor JFET */
   datos_elem.f[2] = -2;       /* Vto   del transistor JFET */
   datos_elem.f[3] = 0;        /* Isr   del transistor JFET */
   datos_elem.f[4] = 0;        /* Cgs   del transistor JFET */
   datos_elem.f[5] = 0;        /* Cgd   del transistor JFET */
   datos_elem.f[6] = 1;        /* PB    del transistor JFET */
   datos_elem.f[7] = 0.5;      /* M     del transistor JFET */
   datos_elem.f[8] = 0.5;      /* FC    del transistor JFET */
   datos_elem.f[9] = 0;        /* VtoTC del transistor JFET */
   datos_elem.f[10] = 0;       /* XTB   del transistor JFET */
   datos_elem.f[11] = 3;       /* XTI   del transistor JFET */
   datos_elem.f[12] = 1.11;    /* Eg    del transistor JFET */
   datos_elem.f[13] = 0;       /* Rd    del transistor JFET */
   datos_elem.f[14] = 0;       /* Rs    del transistor JFET */

   break;
  }

  case TRANSISTOR_MP:
  case TRANSISTOR_MN:
  {
   n = ++nums[3];
   break;
  }

  case GENV:
  {
   n = ++nums[5];
   break;
  }

  case GENI:
  case GENIDEPV:
  case GENIDEPI:
  {
   n = ++nums[6];
   break;
  }

  case GENVDEPV:
  case GENVDEPI:
  {
   n = ++nums[7];
   break;
  }

  case VOLTIMETRO:
  case AMPERIMETRO:
  {
   n = (tipo == VOLTIMETRO) ? ++nums[8] : ++nums[9];
   datos_elem.tipo[1] = 'T';
   datos_elem.tipo[2] = 0;
   datos_elem.f[0] =
   datos_elem.f[1] =
   datos_elem.f[2] = 0;
   datos_elem.signal = 'D';
   datos_elem.CT = 300;
  }
 }

 /* Registramos el elemento en el fichero temporal */
 strcat (datos_elem.nombre, ultoa (n, szBuffer, 10));
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 if ((hFichero = OpenFile (strcat (nom, szExtFich[TMPELEMS]), &of,
			   OF_WRITE | OF_SHARE_DENY_WRITE)) == -1)
   hFichero = OpenFile (nom, &of, OF_CREATE | OF_SHARE_DENY_WRITE);

 _llseek (hFichero, npIV->n_elems * sizeof (elemento), 0);
 _lwrite (hFichero, (LPSTR)&datos_elem, sizeof (datos_elem));
 _lclose (hFichero);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void registrar_cuadripolo (InfoVentana NEAR *npIV)
{
 OFSTRUCT   of;
 cuadripolo datos_cuad;
 int        hFichero;
 char       nom[84];

 /* Inicializamos los par metros a valores por defecto */
 memset (datos_cuad.nombre, 0, 11);
 datos_cuad.tipo_parms = 0;

 /* Funciones de transferencia de numerador y denominador de grado 0 */
 datos_cuad.Ti.num.grado = datos_cuad.Tr.num.grado =
 datos_cuad.Tf.num.grado = datos_cuad.To.num.grado =
 datos_cuad.Ti.den.grado = datos_cuad.Tr.den.grado =
 datos_cuad.Tf.den.grado = datos_cuad.To.den.grado = 0;

 /* Todos los numeradores a 0 */
 datos_cuad.Ti.num.coef[0] = datos_cuad.Tr.num.coef[0] =
 datos_cuad.Tf.num.coef[0] = datos_cuad.To.num.coef[0] = 0;

 /* Denominadores a 1 para evitar la divisi¢n por cero */
 datos_cuad.Ti.den.coef[0] = datos_cuad.Tr.den.coef[0] =
 datos_cuad.Tf.den.coef[0] = datos_cuad.To.den.coef[0] = 1;

 /* Registramos el cuadripolo en el fichero temporal */
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 if ((hFichero = OpenFile (strcat (nom, szExtFich[TMPCUADS]), &of,
			   OF_WRITE | OF_SHARE_DENY_WRITE)) == -1)
   hFichero = OpenFile (nom, &of, OF_CREATE | OF_SHARE_DENY_WRITE);

 _llseek (hFichero, npIV->n_cuad * sizeof (cuadripolo), 0);
 _lwrite (hFichero, (LPSTR)&datos_cuad, sizeof (datos_cuad));
 _lclose (hFichero);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void registrar_bloque (InfoBloque NEAR *npIB)
{
 OFSTRUCT   of;
 bloque     datos_bloque;
 int        hFichero;
 char       nom[84];

 /* Inicializamos los datos a valores por defecto */
 datos_bloque.texto[0]    = 0;
 datos_bloque.fichero[0]  = 0;
 datos_bloque.metafile[0] = 0;
 datos_bloque.alinh       = DT_CENTER;
 datos_bloque.alinv       = DT_CENTER;

 /* Registramos el bloque en el fichero temporal */
 strcpy (nom, npIB->path);
 strcat (nom, npIB->nom_fich);
 if ((hFichero = OpenFile (strcat (nom, szExtFich[TMPBLOQUES]), &of,
			   OF_WRITE | OF_SHARE_DENY_WRITE)) == -1)
   hFichero = OpenFile (nom, &of, OF_CREATE | OF_SHARE_DENY_WRITE);

 _llseek (hFichero, npIB->n_bloques * sizeof (bloque), 0);
 _lwrite (hFichero, (LPSTR)&datos_bloque, sizeof (datos_bloque));
 _lclose (hFichero);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void registrar_texto (InfoBloque NEAR *npIB)
{
 OFSTRUCT   of;
 texto      datos_texto;
 int        hFichero;
 short      i;
 char       nom[84];

 /* Registramos el texto en el fichero temporal */
 strcpy (nom, npIB->path);
 strcat (nom, npIB->nom_fich);
 if ((hFichero = OpenFile (strcat (nom, szExtFich[TMPTEXTOS]), &of,
			   OF_WRITE | OF_SHARE_DENY_WRITE)) == -1)
   hFichero = OpenFile (nom, &of, OF_CREATE | OF_SHARE_DENY_WRITE);

 /* Nos vamos al principio del fichero */
 _llseek (hFichero, 0, 0);

 /* Avanzamos hasta el £ltimo texto (seg£n npIB, que es lo que vale) */
 for (i = 0; i < npIB->n_textos; ++i)
  {
   _lread  (hFichero, (LPSTR) &datos_texto, sizeof (datos_texto.longitud));
   _llseek (hFichero, datos_texto.longitud * sizeof (char), 1);
  }

 /* Inicializamos los datos a valores por defecto */
 datos_texto.longitud = 0;
 datos_texto.txt = NULL;

 /* Registramos los datos (en principio, un 0 £nicamente) */
 _lwrite (hFichero, (LPSTR) &datos_texto, sizeof (datos_texto.longitud));
 _lclose (hFichero);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

short buscar_texto (HFILE hFichero, BOOL rst, short nPos)

/* Coloca el puntero del fichero apuntando al texto 'i' */
{
 short i, nl;

 if (rst)
   _llseek (hFichero, 0, 0);

 for (i = 0; i < nPos - 1; ++i)
  {
   _lread  (hFichero, &nl, sizeof (nl));
   _llseek (hFichero, nl, 1);
  }

 _lread  (hFichero, &nl, sizeof (nl));
 return (nl);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/