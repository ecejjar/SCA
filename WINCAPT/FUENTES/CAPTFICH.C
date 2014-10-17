#include <capturar.h>
#include <wincapt.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de variables globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern char  *szExtFich[];
extern short *bloques,   *puertos, *textos,
	     *elementos, *nudos,   *cables,  *cuad;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL leer_diagrama (InfoBloque NEAR *npIB)
{
 char       nom[84];
 OFSTRUCT   of, ofTemp;
 int        hFichero;

 /* Abrimos el fichero de datos del diagrama de bloques */
 strcpy (nom, npIB->path);
 strcat (nom, npIB->nom_fich);
 hFichero = OpenFile (strcat (nom, ".BLQ"), &of, OF_READ);
 if (hFichero == -1)
   return (TRUE);

 /* Recuperar bloques */
 _lread (hFichero, (LPSTR) &(npIB->n_bloques), sizeof (npIB->n_bloques));

 bloques = (short *) LocalLock (npIB->hmemBloques);
 _lread (hFichero, (LPSTR) bloques, npIB->n_bloques * 6 * sizeof (short));
 LocalUnlock (npIB->hmemBloques);

 /* Recuperar cables */
 _lread (hFichero, (LPSTR) &(npIB->n_cables), sizeof (npIB->n_cables));

 cables = (short *) LocalLock (npIB->hmemCables);
 _lread (hFichero, (LPSTR) cables, npIB->n_cables * 6 * sizeof (short));
 LocalUnlock (npIB->hmemCables);

 /* Recuperar puertos */
 _lread (hFichero, (LPSTR) &(npIB->n_puertos), sizeof (npIB->n_puertos));

 puertos = (short *) LocalLock (npIB->hmemPuertos);
 _lread (hFichero, (LPSTR) puertos, npIB->n_puertos * 3 * sizeof (short));
 LocalUnlock (npIB->hmemPuertos);

 /* Recuperar textos */
 _lread (hFichero, (LPSTR) &(npIB->n_textos), sizeof (npIB->n_textos));

 textos = (short *) LocalLock (npIB->hmemTextos);
 _lread (hFichero, (LPSTR) textos, npIB->n_textos * 4 * sizeof (short));
 LocalUnlock (npIB->hmemTextos);

 /* Cerramos el fichero */
 _lclose (hFichero);

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL guardar_diagrama (InfoBloque NEAR *npIB)
{
 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL leer_esquema (InfoVentana NEAR *npIV)

/* Recupera un esquema del fichero del disco */
{
 elemento   datos_elem;
 cuadripolo datos_cuad;
 OFSTRUCT   of, ofTemp;
 int        hFichero, hFicheroTemp;
 short      i;
 char       nom[84];

 /* Abrimos el fichero de datos del esquema */
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 hFichero = OpenFile (strcat (nom, ".SCH"), &of, OF_READ);
 if (hFichero == -1)
   return (TRUE);

 /* Recuperar elementos */
 _lread (hFichero, (LPSTR) &(npIV->n_elems), sizeof (npIV->n_elems));

 elementos = (short *) LocalLock (npIV->hmemElems);
 _lread (hFichero, (LPSTR) elementos, npIV->n_elems * 6 * sizeof (short));
 LocalUnlock (npIV->hmemElems);

 /* Recuperar cables */
 _lread (hFichero, (LPSTR) &(npIV->n_cables), sizeof (npIV->n_cables));

 cables = (short *) LocalLock (npIV->hmemCables);
 _lread (hFichero, (LPSTR) cables, npIV->n_cables * 6 * sizeof (short));
 LocalUnlock (npIV->hmemCables);

 /* Recuperar nudos */
 _lread (hFichero, (LPSTR) &(npIV->n_nudos), sizeof (npIV->n_nudos));

 nudos = (short *) LocalLock (npIV->hmemNudos);
 _lread (hFichero, (LPSTR) nudos, npIV->n_nudos * 3 * sizeof (short));
 LocalUnlock (npIV->hmemNudos);

 /* Recuperar cuadripolos */
 _lread (hFichero, (LPSTR) &(npIV->n_cuad), sizeof (npIV->n_cuad));

 cuad = (short *) LocalLock (npIV->hmemCuad);
 _lread (hFichero, (LPSTR) cuad, npIV->n_cuad * 4 * sizeof (short));
 LocalUnlock (npIV->hmemCuad);

 /* Creamos el fichero temporal con los datos de los elementos */
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 hFicheroTemp = OpenFile (strcat (nom, ".TM1"), &ofTemp, OF_CREATE);
 if (hFicheroTemp == -1)
   return (TRUE);

 /* Volcamos los datos extra de elementos al fichero temporal */
 for (i = 0; i < npIV->n_elems; ++i)
  {
   _lread  (hFichero,     (LPSTR) &datos_elem, sizeof (datos_elem));
   _lwrite (hFicheroTemp, (LPSTR) &datos_elem, sizeof (datos_elem));
  }

 /* Cerramos el primer fichero temporal */
 _lclose (hFicheroTemp);

 /* Creamos el fichero temporal con los datos de los cuadripolos */
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 hFicheroTemp = OpenFile (strcat (nom, ".TM2"), &ofTemp, OF_CREATE);
 if (hFicheroTemp == -1)
   return (TRUE);

 /* Volcamos los datos extra de cuadripolos al fichero temporal */
 for (i = 0; i < npIV->n_cuad; ++i)
  {
   _lread  (hFichero,     (LPSTR) &datos_cuad, sizeof (datos_cuad));
   _lwrite (hFicheroTemp, (LPSTR) &datos_cuad, sizeof (datos_cuad));
  }

 /* Cerramos ambos ficheros */
 _lclose (hFicheroTemp);
 _lclose (hFichero);

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL guardar_esquema (InfoVentana NEAR *npIV)

/* Graba un esquema en el disco */
{
 elemento   datos_elem;
 cuadripolo datos_cuad;
 OFSTRUCT   of, ofTemp;
 int        hFichero, hFicheroTemp;
 short      i;
 char       nom[84];

 /* Abrimos el fichero de datos del esquema */
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 hFichero = OpenFile (strcat (nom, ".SCH"), &of, OF_CREATE);
 if (hFichero == -1)
   return (TRUE);

 /* Guardar elementos */
 _lwrite (hFichero, (LPSTR) &(npIV->n_elems), sizeof (npIV->n_elems));

 elementos = (short *) LocalLock (npIV->hmemElems);
 _lwrite (hFichero, (LPSTR) elementos, npIV->n_elems * 6 * sizeof (short));
 LocalUnlock (npIV->hmemElems);

 /* Guardar cables */
 _lwrite (hFichero, (LPSTR) &(npIV->n_cables), sizeof (npIV->n_cables));

 cables = (short *) LocalLock (npIV->hmemCables);
 _lwrite (hFichero, (LPSTR) cables, npIV->n_cables * 6 * sizeof (short));
 LocalUnlock (npIV->hmemCables);

 /* Guardar nudos */
 _lwrite (hFichero, (LPSTR) &(npIV->n_nudos), sizeof (npIV->n_nudos));

 nudos = (short *) LocalLock (npIV->hmemNudos);
 _lwrite (hFichero, (LPSTR) nudos, npIV->n_nudos * 3 * sizeof (short));
 LocalUnlock (npIV->hmemNudos);

 /* Guardar cuadripolos */
 _lwrite (hFichero, (LPSTR) &(npIV->n_cuad), sizeof (npIV->n_cuad));

 cuad = (short *) LocalLock (npIV->hmemCuad);
 _lwrite (hFichero, (LPSTR) cuad, npIV->n_cuad * 4 * sizeof (short));
 LocalUnlock (npIV->hmemCuad);

 /* Abrimos el fichero temporal de elementos */
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 hFicheroTemp = OpenFile (strcat (nom, ".TM1"), &ofTemp, OF_READ);
 if (hFicheroTemp == -1)
   return (TRUE);

 /* Volcamos los datos del fichero temporal de elementos */
 for (i = 0; i < npIV->n_elems; ++i)
  {
   _lread  (hFicheroTemp, (LPSTR) &datos_elem, sizeof (datos_elem));
   _lwrite (hFichero,     (LPSTR) &datos_elem, sizeof (datos_elem));
  }

 /* Cerramos el fichero temporal de elementos */
 _lclose (hFicheroTemp);

 /* Abrimos el fichero temporal de cuadripolos */
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 hFicheroTemp = OpenFile (strcat (nom, ".TM2"), &ofTemp, OF_READ);
 if (hFicheroTemp == -1)
   return (TRUE);

 /* Volcamos los datos del fichero temporal de cuadripolos */
 for (i = 0; i < npIV->n_cuad; ++i)
  {
   _lread  (hFicheroTemp, (LPSTR) &datos_cuad, sizeof (datos_cuad));
   _lwrite (hFichero,     (LPSTR) &datos_cuad, sizeof (datos_cuad));
  }

 /* Cerramos el fichero temporal de cuadripolos */
 _lclose (hFicheroTemp);

 /* Cerramos el fichero de esquema */
 _lclose (hFichero);

 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL leer_biblioteca (InfoBiblio NEAR *npIb)
{
 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL guardar_biblioteca (InfoBiblio NEAR *npIb)
{
 return (FALSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

BOOL crear_DCE (InfoVentana NEAR *npIV)

/* Graba un esquema en el disco con formato D.C.E. (nombre.FNT) */
{
 static struct { char tipo;
		 char keyword[15]; } medidas[] = { 'P', "POLARIZACION",
						   'T', "TIEMPO",
						   'F', "FRECUENCIA",
						   'R', "FOURIER",
						   'S', "SENSIBILIDAD" };
 elemento datos_elem;
 OFSTRUCT of, ofTemp;
 POINT    pt;
 int      hFichero, hFicheroTemp;
 short    i, j, k, ult_cable, ext, tipo_union, n_nudos_real = 0,
	  *lista_ramas;
 char     nom[256], aux[4];
 BOOL	  horiz, vert, espejo, fin;

 short _union (POINT pt, BOOL horiz, BOOL vert, short cable,
	       short tipo, short anterior);
 short pasa_por_nudo (InfoVentana NEAR *, short);

 /* Macro para acceder a la lista de ramas */
 #define lr(i,j) *(lista_ramas + 3*(i) + (j))

 /* Creamos el fichero fuente */
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 hFichero = OpenFile (strcat (nom, ".FNT"), &of, OF_CREATE);
 if (hFichero == -1)
   return (TRUE);

 /* Abrimos el fichero temporal para lectura */
 strcpy (nom, npIV->path);
 strcat (nom, npIV->nom_fich);
 hFicheroTemp = OpenFile (strcat (nom, ".TEM"), &ofTemp, OF_READ);
 if (hFicheroTemp == -1)
   return (TRUE);

 /* Creamos la secci¢n de descripci¢n de elementos */
 elementos = (short *) LocalLock (npIV->hmemElems);

 /* Recorremos la lista de elementos */
 for (i = 0; i < npIV->n_elems; ++i)
  {
   /* Obtenemos todos los datos de un elemento */
   _lread (hFicheroTemp, (LPSTR) &datos_elem, sizeof (datos_elem));

   /* Si el elemento no es un medidor.. */
   if (e(i,0) < VOLTIMETRO)
    {
     /* ..grabamos su nombre */
     sprintf (nom, "%s = %c\r\n{\r\n", datos_elem.nombre, datos_elem.tipo[0]);
     _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));

     /* Ahora, si es un/a.. */
     switch (e(i,0))
     {
      case RESISTENCIA:
      {
       sprintf (nom, "VALOR = %g\r\nCT = %g",
		     datos_elem.valor, datos_elem.CT);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       break;
      }

      case CONDENSADOR:
      case BOBINA:
      {
       sprintf (nom, "VALOR = %g\r\nCT = %g\r\nVALOR_INICIAL = %g",
		     datos_elem.valor, datos_elem.CT,
		     datos_elem.caract.cond_inic);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       break;
      }

      case TRANSISTOR_BP:
      case TRANSISTOR_BN:
      {
       sprintf (nom, "IS = %g\r\nISC = %g  ISE = %g\r\nBR = %g  BF = %g\r\n",
		     datos_elem.valor, datos_elem.f[2], datos_elem.f[3],
		     datos_elem.f[1], datos_elem.f[0]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       sprintf (nom, "VAR = %g  VAF = %g\r\nTR = %g  TF = %g\r\nCJC = %g  CJE = %g\r\n",
		     datos_elem.f[14], datos_elem.f[13], datos_elem.f[7],
		     datos_elem.f[6], datos_elem.f[4], datos_elem.f[5]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       sprintf (nom, "VJC = %g  VJE = %g\r\nMJC = %g  MJE = %g\r\nXTB = %g\r\n",
		     datos_elem.f[8], datos_elem.f[9], datos_elem.f[10],
		     datos_elem.f[11], datos_elem.f[15]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       sprintf (nom, "FC = %g\r\nEG = %g\r\nRB = %g  RC = %g  RE = %g",
		     datos_elem.f[12], datos_elem.f[16],
		     datos_elem.f[17], datos_elem.f[18], datos_elem.f[19]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       break;
      }

      case TRANSISTOR_JP:
      case TRANSISTOR_JN:
      {
       sprintf (nom, "IS = %g\r\nBETA = %g\r\nLANDA = %g\r\nVTO = %g\r\nISR = %g\r\n",
		     datos_elem.valor, datos_elem.f[0], datos_elem.f[1],
		     datos_elem.f[2], datos_elem.f[3]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       sprintf (nom, "CGS = %g  CGD = %g\r\nPB = %g\r\nM = %g\r\nFC = %g\r\n",
		     datos_elem.f[4], datos_elem.f[5], datos_elem.f[6],
		     datos_elem.f[7], datos_elem.f[8]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       sprintf (nom, "VTOTC = %g  XTB = %g  XTI = %g\r\nEG = %g\r\n",
		     datos_elem.f[9], datos_elem.f[10], datos_elem.f[11],
		     datos_elem.f[12]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       sprintf (nom, "RD = %g  RS = %g",
		     datos_elem.f[13], datos_elem.f[14]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));

       break;
      }

      case TRANSISTOR_MP:
      case TRANSISTOR_MN:
      {
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       break;
      }

      case DIODO:
      case DIODOZ:
      case DIODOV:
      {
       sprintf (nom, "IS = %g\r\nCJ = %g\r\nTT = %g\r\nRS = %g\r\nVJ = %g\r\nVBR = %g\r\n",
		     datos_elem.valor, datos_elem.f[2], datos_elem.f[3],
		     datos_elem.f[0], datos_elem.f[1], datos_elem.f[6]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       sprintf (nom, "IBV = %g\r\nFC = %g\r\nEG = %g\r\nMJ = %g",
		     datos_elem.f[7], datos_elem.f[5], datos_elem.f[8],
		     datos_elem.f[4]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       break;
      }

      case BATERIA:
      {
       sprintf (nom, "VALOR = %g\r\nFORMA = C\r\nCT = %g",
		     datos_elem.valor, datos_elem.CT);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       break;
      }

      case GENV:
      case GENI:
      {
       sprintf (nom, "VALOR = %g\r\nFORMA = %c\r\nRETARDO = %g\r\nDURACION = %g\r\n",
		     datos_elem.valor, datos_elem.signal, datos_elem.fase,
		     datos_elem.Tact);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       sprintf (nom, "FRECUENCIA = %g\r\nCT = %g",
		     datos_elem.caract.frec, datos_elem.CT);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       break;
      }

      case GENVDEPV:
      case GENIDEPV:
      {
       sprintf (nom, "VALOR = %g\r\nFUNCION = %c\r\nNUDOS = %u, %u",
		     datos_elem.valor, datos_elem.signal,
		     datos_elem.caract.nodos[0], datos_elem.caract.nodos[1]);
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       break;
      }

      case GENVDEPI:
      case GENIDEPI:
      {
       sprintf (nom, "VALOR = %g\r\nFUNCION = %c\r\nELEMENTO = %s\r\nSENTIDO = %u, %u",
		     datos_elem.valor, datos_elem.signal,
		     datos_elem.caract.elem, e(i,5), !e(i,5));
       _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
       break;
      }
     }

     strcpy (nom, "\r\n}\r\n\r\n");
     _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
    }
  }

 /* Creamos la separaci¢n entre las secciones 1 y 2 */
 strcpy (nom, "\r\n#\r\n");
 _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));

 /* Creamos la secci¢n de descripci¢n de ramas */
 cables = (short *) LocalLock (npIV->hmemCables);
 nudos = (short *) LocalLock (npIV->hmemNudos);

 /* Creamos la lista que contendr  con qu‚ nudos est  unido cada elemento */
 lista_ramas = (short *) calloc (npIV->n_elems, 3 * sizeof (short));

 /* Calculamos el n£mero m ximo de nudo que haya en la lista */
 for (i = 0; i < npIV->n_nudos; ++i)
   if (n(i,2) > n_nudos_real)
     n_nudos_real = n(i,2);

 for (i = 0; i < npIV->n_elems; ++i)
  {
   /* Comprobamos los terminales del elemento */
   for (ext = 0;
	ext < 2 + ((e(i,0) >= TRANSISTOR_BP) && (e(i,0) <= TRANSISTOR_MN));
	++ext)
    {
     /* Obtenemos algunas caracter¡sticas del elemento */
     horiz = e(i,3);
     vert  = e(i,4);
     espejo = e(i,5);

     if ((e(i,0) < TRANSISTOR_BP) || (e(i,0) > TRANSISTOR_MN))
      {
       pt.x = (ext == 0) ? e(i,1) : e(i,1) + 40*horiz;
       pt.y = (ext == 0) ? e(i,2) : e(i,2) + 40*vert;
      }
     else
      {
       pt.x = (ext == 0) ? e(i,1) + 30*horiz - 15*vert :
			   (ext == 1) ? e(i,1) + 5*horiz :
					e(i,1) + 30*horiz + 15*vert;
       pt.y = (ext == 0) ? e(i,2) + 15*horiz + 30*vert :
			   (ext == 1) ? e(i,2) + 5*vert :
					e(i,2) - 15*horiz + 30*vert;
       if (ext != 1)
	{
	 horiz = e(i,4);
	 vert  = e(i,3);
	}
      }

     fin = FALSE;
     ult_cable = npIV->n_cables;
     do
     {
      /* Primero averiguamos si se pasa por un nudo */
      if (ult_cable < npIV->n_cables)
	j = pasa_por_nudo (npIV, ult_cable);
      else
	for (j = 0; (j < npIV->n_nudos) &&
		    ((pt.x != n(j,0)) || (pt.y != n(j,1))); ++j);
      if (j < npIV->n_nudos)
       {
	/* Si el elemento no es generador, diodo ni medidor, o 'espejo' es 0.. */
	if ((e(i,0) < DIODO) || (!espejo))

	  /* ..el orden de los nudos en la sentencia ser  el normal */
	  lr(i,ext) = n(j,2);

	/* pero, en caso contrario.. */
	else

	  /* ..el orden de los nudos ser  el inverso */
	  lr (i, 1 - ext) = n(j,2);

	fin = TRUE;
       }
      else
       {
	/* En caso contrario, localizamos los cables unidos al terminal */
	for (j = 0; j < npIV->n_cables; ++j)

	  /* Evitamos comprobar el cable por el que venimos */
	  if (j != ult_cable)
	   {
	    /* Obtenemos el tipo de uni¢n que existe */
	    tipo_union = _union (pt, horiz, vert, j,
				 (ult_cable < npIV->n_cables) ? 0         : 1,
				 (ult_cable < npIV->n_cables) ? ult_cable : i);

	    /* Si el cable est  unido por el extremo inicial.. */
	    if ((tipo_union == UNION_INI) || (tipo_union == SUPERPOS_INI))
	     {
	      /* ..tomamos el extremo opuesto */
	      pt.x = c(j,2);
	      pt.y = c(j,3);

	      /* Si el cable es horizontal.. */
	      if (c(j,4))
		if (c(j,3) - c(j,1) == 0)   /* Si no se dobla, es decir, si */
		 {                          /* el  desplazamiento  vertical */
		  horiz = TRUE;             /* es 0, el extremo opuesto se- */
		  vert  = FALSE;            /* guir  siendo horizontal.     */
		 }
		else                       /* Pero, por el contrario, si se */
		 {                         /* dobla (deplazamiento vertical */
		  horiz = FALSE;           /* <> 0), entonces el extremo o- */
		  vert  = TRUE;            /* puesto ser  vertical.         */
		 }

	      /* pero, si el cable es vertical.. */
	      else
		if (c(j,2) - c(j,0) == 0)   /* Si no se dobla, es decir, si */
		 {                          /* el desplazamiento horizontal */
		  horiz = FALSE;            /* es 0, el extremo opuesto se- */
		  vert  = TRUE;             /* guir  siendo vertical.       */
		 }
		else                       /* Pero, por el contrario, si se */
		 {                         /* dobla (deplazamiento horizon. */
		  horiz = TRUE;            /* <> 0), entonces el extremo o- */
		  vert  = FALSE;           /* puesto ser  horizontal.       */
		 }

	      /* Recordamos el cable localizado y damos fin al bucle */
	      ult_cable = j;
	      j = npIV->n_cables;
	     }

	    /* pero, si el cable est  unido por el extremo final.. */
	    else
	      if ((tipo_union == UNION_FIN) || (tipo_union == SUPERPOS_FIN))
	       {
		/* ..tomamos el extremo inicial */
		pt.x  = c(j,0);
		pt.y  = c(j,1);

		/* El sentido del extremo inicial viene en sus datos */
		horiz = c(j,4);
		vert  = c(j,5);

		/* Recordamos el cable localizado y damos fin al bucle */
		ult_cable = j;
		j = npIV->n_cables;
	       }
	   }

	  /* Si no hemos localizado ning£n cable, probamos con los elementos */
	  if (tipo_union == 0)
	   {
	    for (j = 0; (j < npIV->n_elems) && (!fin); ++j)
	      if (j != i)
	       {
		/* Obtenemos el tipo de uni¢n que existe entre el elemento 'j'
		   y el cable 'ult_cable', si este £ltimo existe. Si no, com-
		   probamos £nicamente la uni¢n exacta del elemento 'j' con el
		   punto 'pt' */
		if (ult_cable < npIV->n_cables)
		 {
		  /* Comprobamos el extremo izqdo. ¢ superior del elemento */
		  pt.x  = e(j,1);
		  pt.y  = e(j,2);
		  horiz = e(j,3);
		  vert  = e(j,4);
		  if (_union (pt, horiz, vert, ult_cable, 1, j))
		    tipo_union = UNION_INI;
		  else
		   {
		    if ((e(j,0) < TRANSISTOR_BP) || (e(j,0) > TRANSISTOR_MN))
		     {
		      /* Comprobamos el extremo derecho */
		      pt.x += 40 * horiz;
		      pt.y += 40 * vert;
		      if (_union (pt, horiz, vert, ult_cable, 1, j))
			tipo_union = UNION_FIN;
		     }
		    else
		     {
		      /* Comprobamos el terminal inferior, al cual le asig-
			 namos el identificador SUPERPOS_INI */
		      pt.x = e(j,1) + 30 * e(j,3) - 15 * e(j,4);
		      pt.y = e(j,2) + 15 * e(j,3) + 30 * e(j,4);
		      horiz = e(j,4);
		      vert  = e(j,3);
		      if (_union (pt, horiz, vert, ult_cable, 1, j))
			tipo_union = SUPERPOS_INI;
		      else
		       {
			/* Comprobamos el terminal superior, al cual le asig-
			   namos el identificador SUPERPOS_FIN */
			pt.x = e(j,1) + 30 * e(j,3) + 15 * e(j,4);
			pt.y = e(j,2) - 15 * e(j,3) + 30 * e(j,4);
			if (_union (pt, horiz, vert, ult_cable, 1, j))
			  tipo_union = SUPERPOS_FIN;
		       }
		     }
		   }
		 }
		else
		  if ((pt.x == e(j,1)) && (pt.y == e(j,2)))
		    tipo_union = UNION_INI;
		  else
		    if ((e(j,0) < TRANSISTOR_BP) || (e(j,0) > TRANSISTOR_MN))
		     {
		      if ((pt.x == e(j,1) + 40*e(j,3)) &&
			  (pt.y == e(j,2) + 40*e(j,4)))
			tipo_union = UNION_FIN;
		     }
		    else
		      if (((pt.x == e(j,1) + 30*e(j,3) - 15*e(j,4)) &&
			   (pt.y == e(j,2) + 15*e(j,3) + 30*e(j,4))))
			tipo_union = SUPERPOS_INI;
		      else
			if (((pt.x == e(j,1) + 30*e(j,3) + 15*e(j,4)) &&
			     (pt.y == e(j,2) - 15*e(j,3) + 30*e(j,4))))
			  tipo_union = SUPERPOS_FIN;

		/* Si hemos llegado a alg£n elemento.. */
		if (tipo_union != 0)
		 {
		  /* ..entonces, si no ha sido procesado a£n.. */
		  if (j > i)
		   {
		    /* ..creamos un nudo nuevo y se lo asignamos */
		    if ((e(i,0) < DIODO) || (!espejo))
		      lr(i,ext) = ++n_nudos_real;
		    else
		      lr(i,ext-1) = ++n_nudos_real;
		   }
		  /* pero, si el elemento ya ha sido procesado.. */
		  else
		   {
		    /* ..le asignamos el nudo que fu‚ creado entonces */
		    if ((e(j,0) < TRANSISTOR_BP) || (e(j,0) > TRANSISTOR_MN))
		      k = (tipo_union == UNION_INI) ? 0 : 1;
		    else
		      k = (tipo_union == UNION_INI) ? 1 :
					  (tipo_union == SUPERPOS_INI) ? 0 : 2;

		    if ((e(i,0) < DIODO) || (!espejo))
		      lr(i,ext) = lr(j, k);
		    else
		      lr(i,ext-1) = lr(j, k);
		   }

		  /* Finalizamos el bucle de elementos y la b£squeda */
		  fin = TRUE;
		 }
	       }

	    /* Aunque no lleguemos a ning£n elemento, tenemos que finalizar
	       la b£squeda. En este caso, el terminal n§ 'ext' quedar  conec-
	       tado a un nudo "colgante" */
	    if (!fin)
	     {
	      if ((e(i,0) < DIODO) || (!espejo))
		lr(i,ext) = ++n_nudos_real;
	      else
		lr(i,ext-1) = ++n_nudos_real;

	      fin = TRUE;
	     }
	   }
       }
     }
     while (!fin);
    }
  }

 /* Nos colocamos al comienzo del fichero temporal */
 _llseek (hFicheroTemp, 0, 0);

 for (i = 0; i < npIV->n_elems; ++i)
  {
   /* Obtenemos todos los datos del elemento */
   _lread (hFicheroTemp, (LPSTR) &datos_elem, sizeof (datos_elem));

   /* Si el elemento no es de medida.. */
   if ((e(i,0) != VOLTIMETRO) && (e(i,0) != AMPERIMETRO))
    {
     /* ..constru¡mos el esqueleto de la sentencia */
     if ((e(i,0) < TRANSISTOR_BP) || (e(i,0) > TRANSISTOR_MN))
       strcpy (nom, "\r\nRAMA   = (   ,   ) { ");
     else
       strcpy (nom, "\r\nMODELO = (   ,   ,   ) { ");
     strcat (nom, datos_elem.nombre);
     strcat (nom, " }\r\n");

     /* Le ponemos los n£meros de nudo */
     for (j = 0;
	  j < 2 + ((e(i,0) >= TRANSISTOR_BP) && (e(i,0) <= TRANSISTOR_MN));
	  ++j)
      {
       itoa(lr(i,j), aux, 10);
       memcpy (&nom[12 + 4*j], aux, strlen (aux));
      }

     /* Guardamos la sentencia en el fichero */
     _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
    }
  }

 /* Creamos la separaci¢n entre las secciones 2 y 3 */
 strcpy (nom, "\r\n#\r\n");
 _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));

 /* Creamos la secci¢n de descripci¢n de medidas */
 for (ext = 0; ext < 5; ++ext)
  {
   /* Localizamos el primer medidor, que fija los par metros de la medida */
   _llseek (hFicheroTemp, 0, 0);
   do
   {
    _lread (hFicheroTemp, (LPSTR) &datos_elem, sizeof (datos_elem));
   }
   while ((datos_elem.tipo[1] != medidas[ext].tipo) && (!eof(hFicheroTemp)));

   if (datos_elem.tipo[1] == medidas[ext].tipo)
    {
     /* Fijamos la temperatura de an lisis */
     sprintf (nom, "\r\nTEMP = %g\r\n", datos_elem.CT);
     _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));

     /* Constru¡mos la definici¢n de la medida */
     switch (ext)
     {
      case 0:
	sprintf (nom, "\r\n%s\r\n{\r\n", medidas[ext].keyword);
	break;

      case 1:
	sprintf (nom, "\r\n%s = (%g, %g, %5.0f)\r\n{\r\n",
		      medidas[ext].keyword, datos_elem.f[0], datos_elem.f[1],
		      datos_elem.f[2]);
	break;

      case 2:
      case 3:
	sprintf (nom, "\r\n%s = (%g, %g, %5.0f, %c)\r\n{\r\n",
		      medidas[ext].keyword, datos_elem.f[0], datos_elem.f[1],
		      datos_elem.f[2], datos_elem.signal);
	break;

      case 4:
	sprintf (nom, "\r\n%s = (%s)\r\n{\r\n", medidas[ext].keyword,
		      datos_elem.caract.elem);
	break;
     }

     /* Almacenamos dicha definici¢n en el fichero fuente */
     _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));

     /* Nos colocamos al comienzo del fichero temporal */
     _llseek (hFicheroTemp, 0, 0);

     /* Recorremos la lista de elementos */
     for (i = 0; i < npIV->n_elems; ++i)
      {
       /* Obtenemos todos los datos del elemento */
       _lread (hFicheroTemp, (LPSTR) &datos_elem, sizeof (datos_elem));

       /* Si el elemento es de medida.. */
       if ((e(i,0) == VOLTIMETRO) || (e(i,0) == AMPERIMETRO))
	{
	 /* Si el tipo de medida coincide con el actualmente considerado.. */
	 if (datos_elem.tipo[1] == medidas[ext].tipo)
	  {
	   /* ..creamos el esqueleto de la sentencia */
	   if (e(i,0) == VOLTIMETRO)
	    {
	     strcpy (nom, "V");
	     strcat (nom, "(   ,   )\r\n");

	     /* Le ponemos los n£meros de nudo */
	     for (j = 0; j < 2; ++j)
	      {
	       itoa(lr(i,j), aux, 10);
	       memcpy (&nom[2 + 4*j], aux, strlen (aux));
	      }
	    }
	   else
	    {
	     strcpy (nom, "I");
	     strcat (nom, "(     )\r\n");

	     /* Buscamos el elemento que se encuentre en serie */
	     fin = 3;
	     for (j = 0; (j < 2) && (fin > 2); ++j)
	      {
	       /* 'fin' contar  el n§ de veces que encontramos el nudo 'j' */
	       fin = 0;

	       /* Contamos dicho n§ de veces */
	       for (k = 0; k < npIV->n_elems; ++k)
		 if ((lr(k, 0) == lr(i, j)) || (lr(k,1) == lr(i, j)))
		  ++fin;
	      }

	     /* Caso de aparecer s¢lo dos veces (una en el amper¡metro y
		otra en el elemento que est‚ en serie con ‚ste), localizamos
		la fila de la lista en la que aparece */
	     if (fin <= 2)
	       for (k = 0; (k < npIV->n_elems) && (lr(k, 0) != lr(i, j-1)) &&
			   (lr(k, 1) != lr(i, j-1)); ++k);

	     /* Le ponemos el nombre del elemento correspond. a dicha fila */
	     _llseek (hFicheroTemp, k * sizeof (datos_elem), 0);
	     _lread (hFicheroTemp, (LPSTR) &datos_elem, sizeof (datos_elem));
	     memcpy (&nom[2], datos_elem.nombre, strlen (datos_elem.nombre));
	     _llseek (hFicheroTemp, (i + 1) * sizeof (datos_elem), 0);
	    }

	   /* Guardamos la sentencia en el fichero */
	   _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
	  }
	}
      }

     /* Finalizamos la sentencia */
     strcpy (nom, "}\r\n");
     _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));
    }
  }

 /* Creamos la se¤al de fin del archivo */
 strcpy (nom, "\r\n#\r\n");
 _lwrite (hFichero, (LPSTR) nom, lstrlen (nom));

 /* Eliminamos la lista de conexiones de elementos */
 free (lista_ramas);

 /* Cerramos los ficheros */
 _lclose (hFicheroTemp);
 _lclose (hFichero);

 /* Desbloqueamos las listas */
 LocalUnlock (npIV->hmemElems);
 LocalUnlock (npIV->hmemCables);
 LocalUnlock (npIV->hmemNudos);

 return (FALSE);

 #undef lr(i,j)
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

short _union (POINT pt, BOOL horiz, BOOL vert, short j,
	      short tipo, short anterior)

/* Averigua si el cable cuya posici¢n en la lista de cables es 'j' est 
   unido ¢ superpuesto al punto de coordenadas 'pt', al que se llega con
   direcci¢n determinada por 'horiz' y 'vert', devolviendo los ids.
   UNION_xxx ¢ SUPERPOS_xxx en cada caso, respectivamente. 'xxx' puede ser
   'INI' ¢ 'FIN', seg£n que el extremo del cable sea el inicial ¢ el final.
   'tipo' se utiliza para saber si se viene de un elemento ¢ de otro cable,
    y 'anterior' se¤ala la posici¢n del mismo en su lista. */
{
 BOOL sentido;

 /* Primero verificamos las uniones exactas */
 if ((c(j,0) == pt.x) && (c(j,1) == pt.y))
   return (UNION_INI);
 else
   if ((c(j,2) == pt.x) && (c(j,3) == pt.y))
     return (UNION_FIN);
   else
     /* Despu‚s, las superposiciones de dos cables */
     if (tipo == 0)
      {
       sentido = horiz ? c(j,2) - c(j,0) < 0 : c(j,3) - c(j,1) < 0;
       if ((horiz && (c(j,1) == pt.y) &&
	    (((sentido == 1) && (c(j,0) >= pt.x) &&
	      (c(j,0) <= pt.x + abs(c(anterior,2) - c(anterior,0)))) ||
	     ((sentido == 0) && (c(j,0) <= pt.x) &&
	      (c(j,0) >= pt.x - abs(c(anterior,2) - c(anterior,0)))))) ||
	   (vert  && (c(j,0) == pt.x) &&
	    (((sentido == 1) && (c(j,1) >= pt.y) &&
	      (c(j,1) <= pt.y + abs(c(anterior,3) - c(anterior,1)))) ||
	     ((sentido == 0) && (c(j,1) <= pt.y) &&
	      (c(j,1) >= pt.y - abs(c(anterior,3) - c(anterior,1)))))))
	 return (SUPERPOS_INI);
       else
	 if ((horiz && (c(j,3) == pt.y) &&
	      (((sentido == 0) && (c(j,2) >= pt.x) &&
		(c(j,2) <= pt.x + abs(c(anterior,2) - c(anterior,0)))) ||
	       ((sentido == 1) && (c(j,2) <= pt.x) &&
		(c(j,2) >= pt.x - abs(c(anterior,2) - c(anterior,0)))))) ||
	     (vert  && (c(j,2) == pt.x) &&
	      (((sentido == 0) && (c(j,3) >= pt.y) &&
		(c(j,3) <= pt.y + abs(c(anterior,3) - c(anterior,1)))) ||
	       ((sentido == 1) && (c(j,3) <= pt.y) &&
		(c(j,3) >= pt.y - abs(c(anterior,3) - c(anterior,1)))))))
	   return (SUPERPOS_FIN);
      }

     /* Finalmente, las superposiciones de elemento y cable */
     else
      {
       /* Obtenemos el extremo que estamos considerando del elemento */
       if ((e(anterior,0) < TRANSISTOR_BP) || (e(anterior,0) > TRANSISTOR_MN))
	 sentido = ((pt.x == e(anterior,1)) && (pt.y == e(anterior,2))) ? 0 : 1;
       else
	{
	 /* El emisor es el que "sale", es decir, el que va hacia abajo. Por
	    eso ser  el £nico para el cual 'sentido' valdr  1 */
	 if ((pt.x == e(anterior,1) + 30 * e(anterior,3) - 15 * e(anterior,4)) &&
	     (pt.y == e(anterior,2) + 15 * e(anterior,3) + 30 * e(anterior,4)))
	   sentido = 1;
	 else
	   sentido = 0;

	 /* Para los extremos 0 ¢ 2 la direcc. estar  cambiada respecto
	    al 1 (central o base) */
	 if ((pt.x != e(anterior,1)) && (pt.y != e(anterior,2)))
	  {
	   horiz = e(anterior,4);
	   vert  = e(anterior,3);
	  }
	}

       if ((horiz && (c(j,1) == pt.y) &&
	    (((sentido == 0) && (c(j,0) >= pt.x) && (c(j,0) <= pt.x + 10)) ||
	     ((sentido == 1) && (c(j,0) <= pt.x) && (c(j,0) >= pt.x - 10)))) ||
	   (vert  && (c(j,0) == pt.x) &&
	    (((sentido == 0) && (c(j,1) >= pt.y) && (c(j,1) <= pt.y + 10)) ||
	     ((sentido == 1) && (c(j,1) <= pt.y) && (c(j,1) >= pt.y - 10)))))
	 return (SUPERPOS_INI);
       else
	 if ((horiz && (c(j,3) == pt.y) &&
	      (((sentido == 0) && (c(j,2) >= pt.x) && (c(j,2) <= pt.x + 10)) ||
	       ((sentido == 1) && (c(j,2) <= pt.x) && (c(j,2) >= pt.x - 10)))) ||
	     (vert  && (c(j,2) == pt.x) &&
	      (((sentido == 0) && (c(j,3) >= pt.y) && (c(j,3) <= pt.y + 10)) ||
	       ((sentido == 1) && (c(j,3) <= pt.y) && (c(j,3) >= pt.y - 10)))))
	   return (SUPERPOS_FIN);
      }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

short pasa_por_nudo (InfoVentana NEAR *npIV, short cable)

/* Detecta si el cable cuya posici¢n en la lista de cables es 'cable' pasa por
   alg£n nudo, devolviendo la posici¢n del mismo en la lista de nudos en caso
   afirmativo, y 0 en caso contrario */
{
 short j,
       cxy, cxini, cxfin, cyx, cyini, cyfin,
       nudo = npIV->n_nudos;

 /* Obtenemos las coordenadas de los segmentos horiz. y vertical */
 cxini = min (c(cable,0), c(cable,2));
 cxfin = max (c(cable,0), c(cable,2));
 cyini = min (c(cable,1), c(cable,3));
 cyfin = max (c(cable,1), c(cable,3));

 if (c(cable,4))
  {
   cxy = c(cable,1);
   cyx = c(cable,2);
  }
 else
  {
   cxy = c(cable,3);
   cyx = c(cable,0);
  }

 /* Recorremos la lista de nudos */
 for (j = 0; (j < npIV->n_nudos) && (nudo == npIV->n_nudos); ++j)
   if (n(j,0) == cyx)
    {
     if ((n(j,1) >= cyini) && (n(j,1) <= cyfin))
       nudo = j;
    }
   else
     if (n(j,1) == cxy)
      {
       if ((n(j,0) >= cxini) && (n(j,0) <= cxfin))
	 nudo = j;
      }

 return (nudo);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

short esta_sobre_cable (InfoVentana NEAR *npIV, short nudo)

/* Detecta si el nudo cuya posici¢n en la lista de nudos es 'nudo' est  sobre
   alg£n cable, devolviendo la posici¢n del mismo en la lista de cables en
   caso afirmativo, y 0 en caso contrario.
   Puesto que un nudo puede estar sobre m£ltiples cables a la vez, si el
   par metro 'nudo' vale -1, se localizar  el siguiente cable a partir del
   encontrado en la anterior llamada a la funci¢n */
{
 static short desde = 0;
 short j,
       cx, cy,
       cxy, cxini, cxfin, cyx, cyini, cyfin,
       cable = npIV->n_cables;

 /* Obtenemos el cable inicial de b£squeda */
 if (nudo > 0)
   desde = 0;

 /* Obtenemos las coordenadas del nudo */
 cx = n(nudo,0);
 cy = n(nudo,1);

 for (j = desde; (j < npIV->n_cables) && (cable == npIV->n_cables); ++j)
  {
   /* Obtenemos las coordenadas de los segmentos horiz. y vertical del cable */
   cxini = min (c(j,0), c(j,2));
   cxfin = max (c(j,0), c(j,2));
   cyini = min (c(j,1), c(j,3));
   cyfin = max (c(j,1), c(j,3));

   if (c(j,4))
    {
     cxy = c(j,1);
     cyx = c(j,2);
    }
   else
    {
     cxy = c(j,3);
     cyx = c(j,0);
    }

   if (cx == cyx)
    {
     if ((cy >= cyini) && (cy <= cyfin))
       cable = j;
    }
   else
     if (cy == cxy)
      {
       if ((cx >= cxini) && (cx <= cxfin))
	 cable = j;
      }
  }

 desde = cable + 1;
 return (cable);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del FicheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/