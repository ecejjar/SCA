#include <windows.h>
#include <resolver.h>
#include <winresol.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de variables globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern unsigned mem_local;
extern long     mem_global;
extern char     nom_fich[80];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

/* Las siguientes funciones duplican las est ndar de C para el ANSI Ch. Set */
LPSTR lstrchr  (LPSTR str, char ch);
LPSTR lstrrchr (LPSTR str, char ch);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL FAR PASCAL _export ProcDialogAbrir (HWND hDlg, WORD message,
					 WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo */
{
 static char EspecFich[84], NomFich[13] = "*.FNT";
 FILE   *fuente;
 char   path[13];

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
       if ((fuente = fopen (EspecFich, "rt")) != NULL)
	{
	 /* ..cerramos el fichero */
	 fclose (fuente);

	 /* Obtenemos el nombre de fichero para el resto del programa */
	 strcpy (nom_fich, EspecFich);
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

BOOL FAR PASCAL _export ProcDialogAcerca (HWND hDlg, WORD message,
					  WORD wParam, LONG lParam)

/* Procedimiento de la Caja de Di logo de Acerca de.. */
{
 char szBuffer[10];

 switch (message)
 {
  case WM_INITDIALOG:
  {
   /* Pintamos todos los tama¤os de memoria */
   itoa (mem_local / 1024, szBuffer, 10);
   SetDlgItemText (hDlg, IDC_TEXTO1, strcat (szBuffer, " Kb"));

   itoa (64 - mem_local / 1024, szBuffer, 10);
   SetDlgItemText (hDlg, IDC_TEXTO2, strcat (szBuffer, " Kb"));

   itoa (mem_global / 1024, szBuffer, 10);
   SetDlgItemText (hDlg, IDC_TEXTO3, strcat (szBuffer, " Kb"));

   itoa (GetFreeSpace(0) / 1024, szBuffer, 10);
   SetDlgItemText (hDlg, IDC_TEXTO4, strcat (szBuffer, " Kb"));

   return (TRUE);
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
