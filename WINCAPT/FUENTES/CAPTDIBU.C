#include "capturar.h"
#include "wincapt.h"

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de variables globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern short  *bloques,   *textos, *puertos,
	      *elementos, *nudos,  *cables,  *cuad;
extern char   path[80], nom_fich[13],
	     *szExtFich[];
extern BOOL   pinta;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

/* Dibujos de la ventana de Esquema */
void  dibujar_circuito   (InfoVentana NEAR *, HDC, RECT);
void  dibujar_elemento   (InfoVentana NEAR *, HDC, short, short, short, short,
						   short, short, char [],
						   HANDLE, BOOL);
void  dibujar_nudo       (InfoVentana NEAR *, HDC, short, short);
void  dibujar_masa       (InfoVentana NEAR *, HDC, short, short, BOOL);
void  dibujar_cuadripolo (InfoVentana NEAR *, HDC, short, short, short, short,
						   short, char [], BOOL);
/* Dibujos de la ventana de Diagrama */
void  dibujar_diagrama   (InfoBloque  NEAR *, HDC, RECT);
void  dibujar_bloque     (InfoBloque  NEAR *, HDC, short, short, short, short,
						   short, short, HANDLE, char[],
						   BOOL);
void  dibujar_puerto     (InfoBloque  NEAR *, HDC, short, short, short, short,
						   short, char[], BOOL);
void  dibujar_texto      (InfoBloque  NEAR *, HDC, short, short, short, char[],
						   BOOL);

/* Dibujos comunes a ambas */
void  dibujar_cable      (void NEAR *, HDC, short, short, short, short,
					    short, short, BOOL);
void  dibujar            (void NEAR *, HDC, short, short, short, short,
					    short, HANDLE, BOOL);


/* Dibujos de la ventana de Biblioteca */
void  dibujar_biblioteca (InfoBiblio  NEAR *, HDC, RECT);
short dibujar_elembibl   (InfoBiblio  NEAR *, HDC, short, short);

/* Dibujos comunes a todas */
void  dibujar_recuadro   (InfoVentana NEAR *, HDC, short, short, short, short,
						   BOOL);
void  dibujar_rejilla    (HDC, RECT);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_circuito (InfoVentana NEAR *npIV, HDC hdc, RECT rect)

/* Dibuja la porci•n de circuito que est‚ dentro de 'rect' */
{
 elemento   datos_elem;
 cuadripolo datos_cuad;
 OFSTRUCT   of;
 RECT       rcPlano, rcElem, rcDest;
 POINT      pt;
 HANDLE     hMF;
 int        hFichero;
 short      i, elem, coordx, coordy, horiz, vert, espejo,
	    xin, yin, longx, longy;
 char	    nom[84], szMetaFile[13];

 /* Convertimos las coordenadas del rect ngulo inv lido de pantalla a plano */
 rcPlano.left   = rect.left   + npIV->nHposScrl * SCRLDESPL;
 rcPlano.right  = rect.right  + npIV->nHposScrl * SCRLDESPL;
 rcPlano.top    = rect.top    + npIV->nVposScrl * SCRLDESPL;
 rcPlano.bottom = rect.bottom + npIV->nVposScrl * SCRLDESPL;

 /* Dibujar elementos */
 if (npIV->n_elems > 0)
  {
   elementos = (short *) LocalLock (npIV->hmemElems);

   /* Abrimos el fichero temporal de elementos */
   strcpy (nom, path);
   strcat (nom, nom_fich);
   hFichero = OpenFile (strcat (nom, szExtFich[TMPELEMS]), &of, OF_READ);

   /* Recorremos la lista de elementos */
   for (i = 0; i < npIV->n_elems; ++i)
    {
     /* Obtenemos las caracter¡sticas del elemento */
     elem     = e(i,0);
     coordx   = e(i,1);
     coordy   = e(i,2);
     horiz    = e(i,3);
     vert     = e(i,4);
     espejo   = e(i,5);

     /* Obtenemos las esquinas sup. izq. e inf. dcha. del rect ngulo que lo
	contiene */
     if (horiz)
      {
       rcElem.left   = coordx;
       rcElem.top    = coordy - 20;
       rcElem.right  = coordx + 40;
       rcElem.bottom = coordy + 20;
      }
     else
      {
       rcElem.left   = coordx - 20;
       rcElem.top    = coordy;
       rcElem.right  = coordx + 20;
       rcElem.bottom = coordy + 40;
      }

     /* Lo dibujamos si se encuentra dentro del  rea de dibujo */
     if (IntersectRect (&rcDest, &rcPlano, &rcElem))
      {
       _llseek (hFichero, i * sizeof (datos_elem), 0);
       _lread (hFichero, (LPSTR) &datos_elem, sizeof (datos_elem));
       dibujar_elemento (npIV, hdc, elem, coordx, coordy, horiz, vert,
			 espejo, datos_elem.nombre, hMF, TRUE);
      }
    }

   LocalUnlock (npIV->hmemElems);

   /* Cerrar el fichero temporal de elementos */
   _lclose (hFichero);
  }

 /* Dibujar cuadripolos */
 if (npIV->n_cuad > 0)
  {
   cuad = (short *) LocalLock (npIV->hmemCuad);

   /* Abrimos el fichero temporal de cuadripolos */
   strcpy (nom, path);
   strcat (nom, nom_fich);
   hFichero = OpenFile (strcat (nom, szExtFich[TMPCUADS]), &of, OF_READ);

   /* Recorremos la lista de cuadripolos */
   for (i = 0; i < npIV->n_cuad; ++i)
    {
     /* Obtenemos las caracter¡sticas del cuadripolo */
     xin = rcElem.left = C(i, 0);
     yin = rcElem.top  = C(i, 1);
     rcElem.right  = C(i, 2);
     rcElem.bottom = C(i, 3);
     horiz = C(i,4);
     longx = rcElem.right  - xin;
     longy = rcElem.bottom - yin;

     /* Lo dibujamos si se encuentra dentro del  rea de dibujo */
     if (IntersectRect (&rcDest, &rcPlano, &rcElem))
      {
       _llseek (hFichero, i * sizeof (datos_cuad), 0);
       _lread (hFichero, (LPSTR)&datos_cuad, sizeof (datos_cuad));

       dibujar_cuadripolo (npIV, hdc, xin, yin, longx, longy, horiz,
			   datos_cuad.nombre, TRUE);
      }
    }

   LocalUnlock (npIV->hmemCuad);

   /* Cerramos el fichero temporal de cuadripolos */
   _lclose (hFichero);
  }

 /* Dibujar cables */
 cables = (short *) LocalLock (npIV->hmemCables);

 for (i = 0; i < npIV->n_cables; ++i)
  {
   xin   = c(i,0);
   yin   = c(i,1);
   longx = c(i,2) - xin;
   longy = c(i,3) - yin;
   horiz = c(i,4);
   vert  = 1 - horiz;

   dibujar_cable (npIV, hdc, xin, yin, longx, longy, horiz, vert, TRUE);
  }

 LocalUnlock (npIV->hmemCables);

 /* Dibujar nudos */
 nudos = (short *) LocalLock (npIV->hmemNudos);

 for (i = 0; i < npIV->n_nudos; ++i)
  {
   pt.x = n(i,0);
   pt.y = n(i,1);

   if (PtInRect (&rcPlano, pt))
     if (n(i,2) == 0)
       dibujar_masa (npIV, hdc, pt.x, pt.y, TRUE);
     else
       dibujar_nudo (npIV, hdc, pt.x, pt.y);
  }

 LocalUnlock (npIV->hmemNudos);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar (void NEAR *npI,
	      HDC hdc, short tipo, short horiz, short vert, short espejo,
	      short empezado, HANDLE hMF, BOOL pinta)

/* Dibuja el objeto que se est‚ utilizando actualmente */
{
 InfoVentana NEAR *npIV;
 InfoBloque  NEAR *npIB;

 switch (tipo)
 {
  case ELEMENTO:
  {
   npIV = (InfoVentana NEAR *) npI;
   elementos = (short *) LocalLock (npIV->hmemElems);

   dibujar_recuadro (npIV, hdc, e(npIV->n_elems,1), e(npIV->n_elems,2),
		     40*horiz + 30*vert, 40*vert + 30*horiz, pinta);

   dibujar_elemento (npIV, hdc, e(npIV->n_elems,0),
				e(npIV->n_elems,1) + 15*vert,
				e(npIV->n_elems,2) + 15*horiz,
				horiz, vert, espejo, "        ", hMF, pinta);
   LocalUnlock (npIV->hmemElems);
   break;
  }

  case CABLE:
  {
   if (empezado)
    {
     /* Aqu¡ el tipo de puntero da igual, las variables son las mismas */
     npIV = (InfoVentana NEAR *) npI;

     cables = (short *) LocalLock (npIV->hmemCables);
     dibujar_cable (npIV, hdc, c(npIV->n_cables,0), c(npIV->n_cables,1),
			       c(npIV->n_cables,2) - c(npIV->n_cables,0),
			       c(npIV->n_cables,3) - c(npIV->n_cables,1),
			       horiz, vert, pinta);
     LocalUnlock (npIV->hmemCables);
    }
   break;
  }

  case NUDO:
  {
   npIV = (InfoVentana NEAR *) npI;
   nudos = (short *) LocalLock (npIV->hmemNudos);
   dibujar_nudo (npIV, hdc, n(npIV->n_nudos,0), n(npIV->n_nudos,1));
   LocalUnlock (npIV->hmemNudos);
   break;
  }

  case MASA:
  {
   npIV = (InfoVentana NEAR *) npI;
   nudos = (short *) LocalLock (npIV->hmemNudos);
   dibujar_masa (npIV, hdc, n(npIV->n_nudos,0), n(npIV->n_nudos,1), pinta);
   LocalUnlock (npIV->hmemNudos);
   break;
  }

  case CUADRIPOLO:
  {
   npIV = (InfoVentana NEAR *) npI;
   cuad = (short *) LocalLock (npIV->hmemCuad);
   dibujar_cuadripolo (npIV, hdc, C(npIV->n_cuad,0), C(npIV->n_cuad,1),
				  C(npIV->n_cuad,2) - C(npIV->n_cuad,0),
				  C(npIV->n_cuad,3) - C(npIV->n_cuad,1),
				  C(npIV->n_cuad,4), "        ", pinta);
   LocalUnlock (npIV->hmemElems);
   break;
  }

  case BLOQUE:
  {
   if (empezado)
    {
     npIB = (InfoBloque NEAR *) npI;
     bloques = (short *) LocalLock (npIB->hmemBloques);
     dibujar_bloque (npIB, hdc, b(npIB->n_bloques,0), b(npIB->n_bloques,1),
				b(npIB->n_bloques,2), b(npIB->n_bloques,3),
				DT_CENTER, DT_CENTER, NULL, "", pinta);
     LocalUnlock (npIB->hmemBloques);
    }
   break;
  }

  case TEXTO:
  {
   npIB = (InfoBloque NEAR *) npI;
   textos = (short *) LocalLock (npIB->hmemTextos);
   LocalUnlock (npIB->hmemTextos);
   break;
  }

  case PUERTO:
  {
   npIB = (InfoBloque NEAR *) npI;
   puertos = (short *) LocalLock (npIB->hmemPuertos);
   dibujar_puerto (npIB, hdc, p(npIB->n_puertos,0), p(npIB->n_puertos,1),
			      p(npIB->n_puertos,2), p(npIB->n_puertos,3),
			      p(npIB->n_puertos,4), "", pinta);
   LocalUnlock (npIB->hmemPuertos);
   break;
  }
 }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_recuadro (InfoVentana NEAR *npIV,
		       HDC hdc, short xin, short yin,
				short longitud, short altura, BOOL pinta)
{
 HPEN   hPen, hViejoPen;

 /* Pasamos del sistema de coordenadas reales al sistema de la ventana */
 xin -= npIV->nHposScrl * SCRLDESPL;
 yin -= npIV->nVposScrl * SCRLDESPL;

 /* Si no pinta, ponemos un l piz rojo discont¡nuo */
 if (!pinta)
  {
   hPen = CreatePen (PS_DOT, 1, RGB(255,0,0));
   hViejoPen = SelectObject (hdc, hPen);
  }

 /* Poner el modo de dibujo a XOR ¢ normal, seg£n el valor de 'pinta' */
 SetROP2 (hdc, (pinta) ? R2_COPYPEN : R2_NOTXORPEN);

 MoveTo (hdc, xin,            yin);
 LineTo (hdc, xin + longitud, yin);
 LineTo (hdc, xin + longitud, yin + altura);
 LineTo (hdc, xin,            yin + altura);
 LineTo (hdc, xin,            yin);

 /* Si no pinta, restauramos el l piz original */
 if (!pinta)
  {
   SelectObject (hdc, hViejoPen);
   DeleteObject (hPen);
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_elemento (InfoVentana NEAR *npIV,
		       HDC hdc, short elem, short coordx, short coordy,
				short horiz, short vert, short espejo,
				char nombre[], HANDLE hMF, BOOL pinta)
{
 TEXTMETRIC tm;
 HPEN       hPen, hViejoPen;
 HFONT      hViejoFont;
 DWORD      dwViejoOrigen;
 int        nViejoModo;
 short      bucle;
 char       szMetaFile[13];

 /* Pasamos del sistema de coordenadas reales al sistema de la ventana */
 coordx -= npIV->nHposScrl * SCRLDESPL;
 coordy -= npIV->nVposScrl * SCRLDESPL;

 /* Modificamos el modo de mapeo de la ventana */
 nViejoModo = SetMapMode (hdc, MM_ISOTROPIC);

 SetWindowExt   (hdc, 1, 1);
 SetViewportExt (hdc, 1 - 2*espejo*horiz, 1 - 2*espejo*vert);

 /* Si estamos en modo espejo, empezamos a pintar en el extremo opuesto */
 if (espejo)
  {
   coordx = (horiz) ? -(coordx + 39) : coordx;
   coordy = (vert)  ? -(coordy + 39) : coordy;
  }

 /* Si no pinta, ponemos un l piz discont¡nuo */
 if (!pinta)
  {
   hPen = CreatePen (PS_DOT, 1, RGB(0,0,255));
   hViejoPen = SelectObject (hdc, hPen);
  }

 /* Poner el modo de dibujo a XOR ¢ normal, seg£n el valor de 'pinta' */
 SetROP2 (hdc, (pinta) ? R2_COPYPEN : R2_NOTXORPEN);

 switch (elem)
 {
  case RESISTENCIA:
  {
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 8*horiz, coordy + 8*vert);
   for (bucle = 1; bucle <= 3; ++bucle)
    {
     LineTo (hdc, coordx + (8*bucle + 2)*horiz - 4*vert,
		  coordy - 4*horiz + (8*bucle + 2)*vert);
     LineTo (hdc, coordx + (8*bucle + 6)*horiz + 4*vert,
		  coordy + 4*horiz + (8*bucle + 6)*vert);
     LineTo (hdc, coordx + (8*bucle + 8)*horiz,
		  coordy + (8*bucle + 8)*vert);
    }

   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);

   break;
  }

  case CONDENSADOR:
  {
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 16*horiz, coordy + 16*vert);
   Rectangle (hdc, coordx + 16*horiz - 5*vert, coordy - 5*horiz + 16*vert,
		   coordx + 18*horiz + 5*vert, coordy + 5*horiz + 18*vert);
   Rectangle (hdc, coordx + 22*horiz - 5*vert, coordy - 5*horiz + 22*vert,
		   coordx + 24*horiz + 5*vert, coordy + 5*horiz + 24*vert);
   MoveTo (hdc, coordx + 24*horiz, coordy + 24*vert);
   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);

   break;
  }

  case BOBINA:
  {
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 10*horiz, coordy + 10*vert);
   Arc    (hdc, coordx + 8*horiz - 2*vert, coordy - 6*horiz + 8*vert,
		coordx + 16*horiz + 6*vert, coordy + 2*horiz + 16*vert,
		coordx + 14*horiz, coordy + 14*vert,
		coordx + 10*horiz, coordy + 10*vert);
   Arc    (hdc, coordx + 12*horiz - 2*vert, coordy - 6*horiz + 12*vert,
		coordx + 20*horiz + 6*vert, coordy + 2*horiz + 20*vert,
		coordx + 18*horiz, coordy + 18*vert,
		coordx + 14*horiz, coordy + 14*vert);
   Arc    (hdc, coordx + 16*horiz - 2*vert, coordy - 6*horiz + 16*vert,
		coordx + 24*horiz + 6*vert, coordy + 2*horiz + 24*vert,
		coordx + 22*horiz, coordy + 22*vert,
		coordx + 18*horiz, coordy + 18*vert);
   Arc    (hdc, coordx + 20*horiz - 2*vert, coordy - 6*horiz + 20*vert,
		coordx + 28*horiz + 6*vert, coordy + 2*horiz + 28*vert,
		coordx + 26*horiz, coordy + 26*vert,
		coordx + 22*horiz, coordy + 22*vert);
   MoveTo (hdc, coordx + 26*horiz, coordy + 26*vert);
   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);

   break;
  }

  case TRANSISTOR_BP:
  case TRANSISTOR_BN:
  {
   Ellipse (hdc, coordx + 10*horiz - 13*vert, coordy - 13*horiz + 10*vert,
		 coordx + 35*horiz + 13*vert, coordy + 13*horiz + 35*vert);
   MoveTo (hdc, coordx + 5*horiz, coordy + 5*vert);
   LineTo (hdc, coordx + 19*horiz, coordy + 19*vert);
   Rectangle (hdc, coordx + 19*horiz - 5*vert, coordy - 5*horiz + 19*vert,
		   coordx + 22*horiz + 5*vert, coordy + 5*horiz + 22*vert);
   MoveTo (hdc, coordx + 30*horiz - 15*vert, coordy - 15*horiz + 30*vert);
   LineTo (hdc, coordx + 30*horiz - 8*vert, coordy - 8*horiz + 30*vert);
   LineTo (hdc, coordx + 22*horiz - 2*vert, coordy - 2*horiz + 22*vert);
   MoveTo (hdc, coordx + 22*horiz + 2*vert, coordy + 2*horiz + 22*vert);
   LineTo (hdc, coordx + 30*horiz + 8*vert, coordy + 8*horiz + 30*vert);
   LineTo (hdc, coordx + 30*horiz + 15*vert, coordy + 15*horiz + 30*vert);

   if (elem == TRANSISTOR_BP)
    {
     MoveTo (hdc, coordx + 22*horiz + 2*vert, coordy + 2*horiz + 22*vert);
     LineTo (hdc, coordx + 26*horiz + 2*vert, coordy + 2*horiz + 26*vert);
     LineTo (hdc, coordx + 22*horiz + 6*vert, coordy + 6*horiz + 22*vert);
    }
   else
    {
     MoveTo (hdc, coordx + 30*horiz + 8*vert, coordy + 8*horiz + 30*vert);
     LineTo (hdc, coordx + 30*horiz + 4*vert, coordy + 4*horiz + 30*vert);
     LineTo (hdc, coordx + 26*horiz + 8*vert, coordy + 8*horiz + 26*vert);
     LineTo (hdc, coordx + 30*horiz + 8*vert, coordy + 8*horiz + 30*vert);
    }

   break;
  }

  case TRANSISTOR_JP:
  case TRANSISTOR_JN:
  {
   Ellipse (hdc, coordx + 10*horiz - 13*vert, coordy - 13*horiz + 10*vert,
		 coordx + 35*horiz + 13*vert, coordy + 13*horiz + 35*vert);
   MoveTo (hdc, coordx + 5*horiz, coordy + 5*vert);
   LineTo (hdc, coordx + 19*horiz, coordy + 19*vert);
   Rectangle (hdc, coordx + 19*horiz - 7*vert, coordy - 7*horiz + 19*vert,
		   coordx + 22*horiz + 7*vert, coordy + 7*horiz + 22*vert);
   MoveTo (hdc, coordx + 22*horiz +  4*vert, coordy -  4*horiz + 22*vert);
   LineTo (hdc, coordx + 25*horiz +  4*vert, coordy -  4*horiz + 25*vert);
   LineTo (hdc, coordx + 25*horiz + 15*vert, coordy - 15*horiz + 25*vert);
   MoveTo (hdc, coordx + 22*horiz -  4*vert, coordy +  4*horiz + 22*vert);
   LineTo (hdc, coordx + 25*horiz -  4*vert, coordy +  4*horiz + 25*vert);
   LineTo (hdc, coordx + 25*horiz - 15*vert, coordy + 15*horiz + 25*vert);

   if (elem == TRANSISTOR_JP)
    {
     MoveTo (hdc, coordx + 19*horiz, coordy + 19*vert);
     LineTo (hdc, coordx + 15*horiz - 4*vert, coordy - 4*horiz + 15*vert);
     LineTo (hdc, coordx + 15*horiz + 4*vert, coordy + 4*horiz + 22*vert);
     LineTo (hdc, coordx + 19*horiz, coordy + 19*vert);
    }
   else
    {
     MoveTo (hdc, coordx + 11*horiz, coordy + 11*vert);
     LineTo (hdc, coordx + 15*horiz - 4*vert, coordy - 4*horiz + 15*vert);
     LineTo (hdc, coordx + 15*horiz + 4*vert, coordy + 4*horiz + 15*vert);
     LineTo (hdc, coordx + 11*horiz, coordy + 11*vert);
    }

   break;
  }

  case TRANSISTOR_MP:
  case TRANSISTOR_MN:
  {
   Ellipse (hdc, coordx + 10*horiz - 13*vert, coordy - 13*horiz + 10*vert,
		 coordx + 35*horiz + 13*vert, coordy + 13*horiz + 35*vert);
   MoveTo (hdc, coordx + 5*horiz, coordy + 5*vert);
   LineTo (hdc, coordx + 19*horiz, coordy + 19*vert);
   Rectangle (hdc, coordx + 19*horiz - 8*vert, coordy - 8*horiz + 19*vert,
		   coordx + 22*horiz + 8*vert, coordy + 8*horiz + 22*vert);
   MoveTo (hdc, coordx + 24*horiz - 8*vert, coordy - 8*horiz + 24*vert);
   LineTo (hdc, coordx + 24*horiz - 4*vert, coordy - 5*horiz + 24*vert);
   MoveTo (hdc, coordx + 24*horiz - 2*vert, coordy - 2*horiz + 24*vert);
   LineTo (hdc, coordx + 24*horiz + 2*vert, coordy + 2*horiz + 24*vert);
   MoveTo (hdc, coordx + 24*horiz + 4*vert, coordy + 4*horiz + 24*vert);
   LineTo (hdc, coordx + 24*horiz + 8*vert, coordy + 8*horiz + 24*vert);
   MoveTo (hdc, coordx + 24*horiz - 6*vert, coordy - 6*horiz + 24*vert);
   LineTo (hdc, coordx + 30*horiz - 6*vert, coordy - 6*horiz + 30*vert);
   LineTo (hdc, coordx + 30*horiz - 15*vert, coordy - 15*horiz + 30*vert);
   MoveTo (hdc, coordx + 24*horiz,          coordy           + 24*vert);
   LineTo (hdc, coordx + 30*horiz,          coordy           + 30*vert);
   LineTo (hdc, coordx + 30*horiz + 6*vert, coordy + 6*horiz + 30*vert);
   MoveTo (hdc, coordx + 24*horiz + 6*vert, coordy + 6*horiz + 24*vert);
   LineTo (hdc, coordx + 30*horiz + 6*vert, coordy + 6*horiz + 30*vert);
   LineTo (hdc, coordx + 30*horiz + 15*vert, coordy + 15*horiz + 30*vert);

   break;
  }

  case DIODO:
  case DIODOZ:
  case DIODOV:
  {
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 15*horiz, coordy + 15*vert);
   MoveTo (hdc, coordx + 25*horiz, coordy + 25*vert);
   LineTo (hdc, coordx + 15*horiz + 5*vert, coordy + 5*horiz + 15*vert);
   LineTo (hdc, coordx + 15*horiz - 5*vert, coordy - 5*horiz + 15*vert);
   LineTo (hdc, coordx + 25*horiz, coordy + 25*vert);
   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);
   MoveTo (hdc, coordx + 25*horiz - 5*vert, coordy - 5*horiz + 25*vert);
   LineTo (hdc, coordx + 25*horiz + 5*vert, coordy + 5*horiz + 25*vert);

   if (elem == DIODOZ)
    {
     MoveTo (hdc, coordx + 25*horiz - 5*vert, coordy - 5*horiz + 25*vert);
     LineTo (hdc, coordx + 23*horiz - 7*vert, coordy - 7*horiz + 23*vert);
     MoveTo (hdc, coordx + 25*horiz + 5*vert, coordy + 5*horiz + 25*vert);
     LineTo (hdc, coordx + 27*horiz + 7*vert, coordy + 7*horiz + 27*vert);
    }
   else
     if (elem == DIODOV)
      {
       MoveTo (hdc, coordx + 28*horiz - 5*vert, coordy - 5*horiz + 28*vert);
       LineTo (hdc, coordx + 28*horiz + 5*vert, coordy + 5*horiz + 28*vert);
      }

   break;
  }

  case BATERIA:
  {
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 16*horiz, coordy + 16*vert);
   MoveTo (hdc, coordx + 16*horiz - 8*vert, coordy - 8*horiz + 16*vert);
   LineTo (hdc, coordx + 16*horiz + 8*vert, coordy + 8*horiz + 16*vert);
   Rectangle (hdc, coordx + 20*horiz - 4*vert, coordy - 4*horiz + 20*vert,
		   coordx + 24*horiz + 4*vert, coordy + 4*horiz + 24*vert);
   MoveTo (hdc, coordx + 24*horiz, coordy + 24*vert);
   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);

   break;
  }

  case GENV:
  {
   Ellipse (hdc, coordx + 12*horiz - 8*vert, coordy - 8*horiz + 12*vert,
		 coordx + 28*horiz + 8*vert, coordy + 8*horiz + 28*vert);
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 12*horiz, coordy + 12*vert);
   MoveTo (hdc, coordx + 28*horiz, coordy + 28*vert);
   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);
   MoveTo (hdc, coordx + 15*horiz - 5*vert, coordy + 20*vert);
   LineTo (hdc, coordx + 16*horiz - 4*vert, coordy - 2*horiz + 17*vert);
   LineTo (hdc, coordx + 17*horiz - 3*vert, coordy - 3*horiz + 16*vert);
   LineTo (hdc, coordx + 18*horiz - 3*vert, coordy - 3*horiz + 16*vert);
   LineTo (hdc, coordx + 19*horiz - 1*vert, coordy - 2*horiz + 17*vert);
   LineTo (hdc, coordx + 21*horiz + 1*vert, coordy + 2*horiz + 23*vert);
   LineTo (hdc, coordx + 22*horiz + 2*vert, coordy + 3*horiz + 24*vert);
   LineTo (hdc, coordx + 23*horiz + 3*vert, coordy + 3*horiz + 24*vert);
   LineTo (hdc, coordx + 24*horiz + 4*vert, coordy + 2*horiz + 23*vert);
   LineTo (hdc, coordx + 25*horiz + 5*vert, coordy + 20*vert);
   MoveTo (hdc, coordx + 7*horiz - 3*vert,  coordy - 3*horiz + 7*vert);
   LineTo (hdc, coordx + 11*horiz - 3*vert, coordy - 3*horiz + 11*vert);
   MoveTo (hdc, coordx + 9*horiz - 5*vert, coordy - 5*horiz + 9*vert);
   LineTo (hdc, coordx + 9*horiz - vert,   coordy - horiz   + 9*vert);

   break;
  }

  case GENI:
  {
   Ellipse (hdc, coordx + 12*horiz - 8*vert, coordy - 8*horiz + 12*vert,
		 coordx + 28*horiz + 8*vert, coordy + 8*horiz + 28*vert);
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 12*horiz, coordy + 12*vert);
   MoveTo (hdc, coordx + 28*horiz, coordy + 28*vert);
   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);
   MoveTo (hdc, coordx + 15*horiz, coordy + 15*vert);
   LineTo (hdc, coordx + 25*horiz, coordy + 25*vert);
   LineTo (hdc, coordx + 22*horiz - 2*vert, coordy - 2*horiz + 22*vert);
   LineTo (hdc, coordx + 22*horiz + 2*vert, coordy + 2*horiz + 22*vert);
   LineTo (hdc, coordx + 25*horiz, coordy + 25*vert);

   break;
  }

  case GENVDEPV:
  case GENVDEPI:
  {
   Ellipse (hdc, coordx + 12*horiz - 8*vert, coordy - 8*horiz + 12*vert,
		 coordx + 28*horiz + 8*vert, coordy + 8*horiz + 28*vert);
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 12*horiz, coordy + 12*vert);
   MoveTo (hdc, coordx + 28*horiz, coordy + 28*vert);
   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);
   MoveTo (hdc, coordx + 7*horiz - 3*vert,  coordy - 3*horiz + 7*vert);
   LineTo (hdc, coordx + 11*horiz - 3*vert, coordy - 3*horiz + 11*vert);
   MoveTo (hdc, coordx + 9*horiz - 5*vert, coordy - 5*horiz + 9*vert);
   LineTo (hdc, coordx + 9*horiz - vert,   coordy - horiz   + 9*vert);

   break;
  }

  case GENIDEPV:
  case GENIDEPI:
  {
   Ellipse (hdc, coordx + 12*horiz - 8*vert, coordy - 8*horiz + 12*vert,
		 coordx + 28*horiz + 8*vert, coordy + 8*horiz + 28*vert);
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 12*horiz, coordy + 12*vert);
   MoveTo (hdc, coordx + 28*horiz, coordy + 28*vert);
   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);
   MoveTo (hdc, coordx + 35*horiz, coordy + 35*vert);
   LineTo (hdc, coordx + 32*horiz - 2*vert, coordy - 2*horiz + 32*vert);
   LineTo (hdc, coordx + 32*horiz + 2*vert, coordy + 2*horiz + 32*vert);
   LineTo (hdc, coordx + 35*horiz, coordy + 35*vert);

   break;
  }

  case VOLTIMETRO:
  case AMPERIMETRO:
  {
   Ellipse (hdc, coordx + 12*horiz - 8*vert, coordy - 8*horiz + 12*vert,
		 coordx + 28*horiz + 8*vert, coordy + 8*horiz + 28*vert);
   MoveTo (hdc, coordx, coordy);
   LineTo (hdc, coordx + 12*horiz, coordy + 12*vert);
   MoveTo (hdc, coordx + 28*horiz, coordy + 28*vert);
   LineTo (hdc, coordx + 40*horiz, coordy + 40*vert);

   if (elem == VOLTIMETRO)
   {
    MoveTo (hdc, coordx + 16*horiz - 4*vert, coordy - 4*horiz + 16*vert);
    LineTo (hdc, coordx + 20*horiz, coordy + 4*horiz + 24*vert);
    LineTo (hdc, coordx + 24*horiz + 4*vert, coordy - 4*horiz + 16*vert);
    MoveTo (hdc, coordx + 7*horiz - 3*vert,  coordy - 3*horiz + 7*vert);
    LineTo (hdc, coordx + 11*horiz - 3*vert, coordy - 3*horiz + 11*vert);
    MoveTo (hdc, coordx + 9*horiz - 5*vert, coordy - 5*horiz + 9*vert);
    LineTo (hdc, coordx + 9*horiz - vert,   coordy - horiz   + 9*vert);
   }
   else
   {
    MoveTo (hdc, coordx + 18*horiz - 2*vert, coordy - 4*horiz + 16*vert);
    LineTo (hdc, coordx + 22*horiz + 2*vert, coordy - 4*horiz + 16*vert);
    MoveTo (hdc, coordx + 20*horiz,          coordy - 4*horiz + 16*vert);
    LineTo (hdc, coordx + 20*horiz,          coordy + 4*horiz + 24*vert);
    MoveTo (hdc, coordx + 18*horiz - 2*vert, coordy + 4*horiz + 24*vert);
    LineTo (hdc, coordx + 22*horiz + 2*vert, coordy + 4*horiz + 24*vert);
    MoveTo (hdc, coordx + 35*horiz, coordy + 35*vert);
    LineTo (hdc, coordx + 32*horiz - 2*vert, coordy - 2*horiz + 32*vert);
    LineTo (hdc, coordx + 32*horiz + 2*vert, coordy + 2*horiz + 32*vert);
    LineTo (hdc, coordx + 35*horiz, coordy + 35*vert);
   }

   break;
  }

  case METAFILE:
  {
   /* Representamos el MetaFile, si existe */
   if (hMF != NULL)
    {
     dwViejoOrigen = SetWindowOrg (hdc, coordx, coordy);
     PlayMetaFile (hdc, hMF);
     SetWindowOrg (hdc, LOWORD(dwViejoOrigen), HIWORD(dwViejoOrigen));
    }

   break;
  }
 }

 /* Si no pinta, restauramos el l piz original */
 if (!pinta)
  {
   SelectObject (hdc, hViejoPen);
   DeleteObject (hPen);
  }

 /* Restauramos el modo de mapeo original */
 SetMapMode (hdc, nViejoModo);

 /* Si estamos en modo espejo, restauramos las coordenadas originales */
 if (espejo)
  {
   coordx = (horiz) ? -(coordx + 39) : coordx;
   coordy = (vert)  ? -(coordy + 39) : coordy;
  }

 /* Le ponemos el nombre, si lo trae */
 if ((nombre != NULL) && (pinta))
  {
   hViejoFont = SelectObject (hdc, npIV->hNuevoFont);
   GetTextMetrics (hdc, &tm);
   if ((elem < TRANSISTOR_BP) || (elem > TRANSISTOR_MN))
     TextOut (hdc,
	      coordx + (20 - strlen (nombre)*tm.tmAveCharWidth/2)*horiz + 15*vert,
	      coordy + 10*horiz + (20 - tm.tmHeight/2)*vert,
	      nombre, strlen (nombre));
   else
     TextOut (hdc,
	      coordx + (25 - strlen (nombre)*tm.tmAveCharWidth)*horiz + 15*vert,
	      coordy + 15*horiz + (25 - tm.tmHeight)*vert,
	      nombre, strlen (nombre));
   SelectObject (hdc, hViejoFont);
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_cable (void NEAR *npI,
		    HDC hdc, short xin, short yin, short longx, short longy,
			     short horiz, short vert, BOOL pinta)
{
 InfoVentana NEAR *npIV;
 HPEN              hPen, hViejoPen;

 /* El tipo de puntero no importa; tanto 'InfoBloque' como 'InfoVentana'
    tienen las variables 'nXposScrl' en las mismas posiciones */
 npIV = (InfoVentana NEAR *) npI;

 /* Pasamos del sistema de coordenadas reales al sistema de la ventana */
 xin -= npIV->nHposScrl * SCRLDESPL;
 yin -= npIV->nVposScrl * SCRLDESPL;

 /* Si no pinta, ponemos un l piz azul discont¡nuo */
 if (!pinta)
  {
   hPen = CreatePen (PS_DOT, 1, RGB(0,0,255));
   hViejoPen = SelectObject (hdc, hPen);
  }

 /* Poner el modo de dibujo a XOR ¢ normal, seg£n el valor de 'pinta' */
 SetROP2 (hdc, (pinta) ? R2_COPYPEN : R2_NOTXORPEN);

 MoveTo (hdc, xin, yin);
 LineTo (hdc, xin + longx*horiz, yin + longy*vert);
 LineTo (hdc, xin + longx, yin + longy);

 /* Si no pinta, restauramos el l piz original */
 if (!pinta)
  {
   SelectObject (hdc, hViejoPen);
   DeleteObject (hPen);
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_nudo (InfoVentana NEAR *npIV, HDC hdc, short xc, short yc)
{
 /* Pasamos del sistema de coordenadas reales al sistema de la ventana */
 xc -= npIV->nHposScrl * SCRLDESPL;
 yc -= npIV->nVposScrl * SCRLDESPL;

 /* Poner el modo de dibujo a normal */
 SetROP2 (hdc, R2_COPYPEN);

 Ellipse (hdc, xc - 2, yc - 2, xc + 2, yc + 2);
 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_masa (InfoVentana NEAR *npIV,
		   HDC hdc, short xc, short yc, BOOL pinta)
{
 HPEN hPen, hViejoPen;

 /* Pasamos del sistema de coordenadas reales al sistema de la ventana */
 xc -= npIV->nHposScrl * SCRLDESPL;
 yc -= npIV->nVposScrl * SCRLDESPL;

 /* Si no pinta, ponemos un l piz azul discont¡nuo */
 if (!pinta)
  {
   hPen = CreatePen (PS_DOT, 1, RGB(0,0,255));
   hViejoPen = SelectObject (hdc, hPen);
  }

 /* Poner el modo de dibujo a XOR ¢ normal, seg£n el valor de 'pinta' */
 SetROP2 (hdc, (pinta) ? R2_COPYPEN : R2_NOTXORPEN);

 MoveTo (hdc, xc, yc);
 LineTo (hdc, xc, yc + 10);
 MoveTo (hdc, xc - 5, yc + 10);
 LineTo (hdc, xc + 5, yc + 10);
 MoveTo (hdc, xc - 3, yc + 13);
 LineTo (hdc, xc + 3, yc + 13);
 MoveTo (hdc, xc - 1, yc + 16);
 LineTo (hdc, xc + 1, yc + 16);

 /* Si no pinta, restauramos el l piz original */
 if (!pinta)
  {
   SelectObject (hdc, hViejoPen);
   DeleteObject (hPen);
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_cuadripolo (InfoVentana NEAR *npIV,
			 HDC hdc, short xin, short yin,
				  short longx, short longy,
				  short horiz, char nombre[], BOOL pinta)
{
 TEXTMETRIC tm;
 HPEN       hPen, hViejoPen;

 /* Pasamos del sistema de coordenadas reales al sistema de la ventana */
 xin -= npIV->nHposScrl * SCRLDESPL;
 yin -= npIV->nVposScrl * SCRLDESPL;

 /* Si no pinta, ponemos un l piz azul discont¡nuo */
 if (!pinta)
  {
   hPen = CreatePen (PS_DOT, 1, RGB(0,0,255));
   hViejoPen = SelectObject (hdc, hPen);
  }

 /* Poner el modo de dibujo a XOR ¢ normal, seg£n el valor de 'pinta' */
 SetROP2 (hdc, (pinta) ? R2_COPYPEN : R2_NOTXORPEN);

 /* Dibujamos el cuadripolo */
 MoveTo (hdc, xin + !horiz * longx/4, yin + horiz * longy/4);
 LineTo (hdc, xin + !horiz * longx/4  + horiz * longx/10,
	      yin + !horiz * longy/10 + horiz * longy/4);
 MoveTo (hdc, xin + !horiz * 3*longx/4, yin + horiz * 3*longy/4);
 LineTo (hdc, xin + !horiz * 3*longx/4 + horiz * longx/10,
	      yin + !horiz * longy/10  + horiz * 3*longy/4);
 MoveTo (hdc, xin + !horiz * longx/4    + horiz * 9*longx/10,
	      yin + !horiz * 9*longy/10 + horiz * longy/4);
 LineTo (hdc, xin + !horiz * longx/4 + horiz * longx,
	      yin + !horiz * longy   + horiz * longy/4);
 MoveTo (hdc, xin + !horiz * 3*longx/4  + horiz * 9*longx/10,
	      yin + !horiz * 9*longy/10 + horiz * 3*longy/4);
 LineTo (hdc, xin + !horiz * 3*longx/4 + horiz * longx,
	      yin + !horiz * longy     + horiz * 3*longy/4);
 dibujar_recuadro (npIV, hdc, xin + horiz * longx/10, yin + !horiz * longy/10,
			      (8 + 2*(!horiz)) * longx / 10,
			      (8 + 2*horiz)    * longy / 10, pinta);

 /* Le ponemos el nombre, si lo trae */
 if ((nombre != NULL) && (pinta))
  {
   GetTextMetrics (hdc, &tm);
   TextOut (hdc, xin + (longx - strlen (nombre) * tm.tmAveCharWidth) / 2,
		 yin + (longy - tm.tmHeight) / 2,
		 nombre, strlen (nombre));
  }

 /* Si no pinta, restauramos el l piz original */
 if (!pinta)
  {
   SelectObject (hdc, hViejoPen);
   DeleteObject (hPen);
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_diagrama (InfoBloque NEAR *npIB, HDC hdc, RECT rect)
{
 bloque     datos_bloque;
 texto      datos_texto;
 TEXTMETRIC tm;
 OFSTRUCT   of;
 RECT       rcPlano, rcElem, rcDest;
 POINT      pt;
 HANDLE     hMF;
 int        hFichero;
 short      i, coordx, coordy, horiz, vert, espejo,
	    xin, yin, longx, longy, nTam;
 char	    nom[84], szMetaFile[84], *szBuffer;

 /* Convertimos las coordenadas del rect ngulo inv lido de pantalla a plano */
 rcPlano.left   = rect.left   + npIB->nHposScrl * SCRLDESPL;
 rcPlano.right  = rect.right  + npIB->nHposScrl * SCRLDESPL;
 rcPlano.top    = rect.top    + npIB->nVposScrl * SCRLDESPL;
 rcPlano.bottom = rect.bottom + npIB->nVposScrl * SCRLDESPL;

 /* Dibujar bloques */
 if (npIB->n_bloques > 0)
  {
   bloques = (short *) LocalLock (npIB->hmemBloques);

   /* Abrimos el fichero temporal de bloques */
   strcpy (nom, path);
   strcat (nom, nom_fich);
   hFichero = OpenFile (strcat (nom, szExtFich[TMPBLOQUES]), &of, OF_READ);

   /* Recorremos la lista de bloques */
   for (i = 0; i < npIB->n_bloques; ++i)
    {
     /* Obtenemos las caracter¡sticas del bloque */
     xin = rcElem.left = b(i,0);
     yin = rcElem.top  = b(i,1);
     rcElem.right  = b(i,2);
     rcElem.bottom = b(i,3);
     longx = rcElem.right  - xin;
     longy = rcElem.bottom - yin;

     /* Lo dibujamos si se encuentra dentro del  rea de dibujo */
     if (IntersectRect (&rcDest, &rcPlano, &rcElem))
      {
       _llseek (hFichero, i * sizeof (datos_bloque), 0);
       _lread (hFichero, (LPSTR)&datos_bloque, sizeof (datos_bloque));

       if (strlen (datos_bloque.metafile) > 0)
	 hMF = GetMetaFile (datos_bloque.metafile);
       else
	 hMF = NULL;

       dibujar_bloque (npIB, hdc, xin, yin, xin + longx, yin + longy,
		       datos_bloque.alinh, datos_bloque.alinv,
		       hMF, datos_bloque.texto, TRUE);

       if (hMF != NULL)
	 DeleteMetaFile (hMF);
      }
    }

   LocalUnlock (npIB->hmemBloques);

   /* Cerramos el fichero temporal de bloques */
   _lclose (hFichero);
  }

 /* Dibujar cables */
 cables = (short *) LocalLock (npIB->hmemCables);

 for (i = 0; i < npIB->n_cables; ++i)
  {
   xin   = c(i,0);
   yin   = c(i,1);
   longx = c(i,2) - xin;
   longy = c(i,3) - yin;
   horiz = c(i,4);
   vert  = 1 - horiz;

   dibujar_cable (npIB, hdc, xin, yin, longx, longy, horiz, vert, TRUE);
  }

 LocalUnlock (npIB->hmemCables);

 /* Dibujar puertos */
 puertos = (short *) LocalLock (npIB->hmemPuertos);

 for (i = 0; i < npIB->n_puertos; ++i)
  {
   xin    = p(i,0);
   yin    = p(i,1);
   horiz  = p(i,2);
   vert   = p(i,3);
   espejo = p(i,4);

   dibujar_puerto (npIB, hdc, xin, yin, horiz, vert, espejo,
		   (char *)(puertos + 11*i + 5), TRUE);
  }

 LocalUnlock (npIB->hmemPuertos);

 /* Dibujar textos */
 if (npIB->n_textos > 0)
  {
   textos = (short *) LocalLock (npIB->hmemTextos);

   /* Abrimos el fichero temporal de textos */
   strcpy (nom, path);
   strcat (nom, nom_fich);
   hFichero = OpenFile (strcat (nom, szExtFich[TMPTEXTOS]), &of, OF_READ);

   /* Recorremos la lista de textos */
   for (i = 0; i < npIB->n_textos; ++i)
    {
     /* Obtenemos las caracter¡sticas del texto */
     GetTextMetrics (hdc, &tm);
     xin = rcElem.left = t(i,0);
     yin = rcElem.top  = t(i,1);
     horiz = t(i,2);
     vert  = t(i,3);
     longx = (t(i,4) * tm.tmAveCharWidth * horiz) + (tm.tmAveCharWidth * vert);
     longy = (tm.tmHeight * horiz) + (t(i,4) * tm.tmHeight * vert);
     rcElem.right  = xin + longx;
     rcElem.bottom = yin + longy;

     /* Lo dibujamos si se encuentra dentro del  rea de dibujo */
     if (IntersectRect (&rcDest, &rcPlano, &rcElem))
      {
       _lread (hFichero, (LPSTR) &nTam, sizeof (nTam));
       szBuffer = (char *) malloc (nTam);
       _lread (hFichero, (LPSTR) szBuffer, nTam);
       dibujar_texto (npIB, hdc, xin, yin, horiz, szBuffer, TRUE);
       free (szBuffer);
      }
    }

   LocalUnlock (npIB->hmemTextos);

   /* Cerramos el fichero temporal de textos */
   _lclose (hFichero);
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_bloque (InfoBloque NEAR *npIB,
		     HDC hdc, short xin, short yin, short xfin, short yfin,
			      short alh, short alv, HANDLE hMF, char texto[],
			      BOOL pinta)
{
 HPEN  hPen, hViejoPen;
 RECT  rc;
 DWORD dwViejoOrg;

 /* Pasamos del sistema de coordenadas reales al sistema de la ventana */
 xin -= npIB->nHposScrl * SCRLDESPL;
 yin -= npIB->nVposScrl * SCRLDESPL;

 /* Si no pinta, creamos un l piz azul discont¡nuo */
 if (!pinta)
   hPen = CreatePen (PS_DOT, 1, RGB(0,0,255));

 /* pero, si pinta, creamos un l piz normal gordo */
 else
   hPen = CreatePen (PS_SOLID, 2, RGB(0,0,0));

 /* Seleccionamos el l piz */
 hViejoPen = SelectObject (hdc, hPen);

 /* Poner el modo de dibujo a XOR ¢ normal, seg£n el valor de 'pinta' */
 SetROP2 (hdc, (pinta) ? R2_COPYPEN : R2_NOTXORPEN);

 /* Dibujamos el recuadro */
 MoveTo (hdc, xin,  yin);
 LineTo (hdc, xfin, yin);
 LineTo (hdc, xfin, yfin);
 LineTo (hdc, xin,  yfin);
 LineTo (hdc, xin,  yin);

 /* Restauramos el l piz original y eliminamos el otro */
 SelectObject (hdc, hViejoPen);
 DeleteObject (hPen);

 /* Si 'pinta', dibujamos el texto y el MetaFile */
 if (pinta)
  {
   if (hMF != NULL)
    {
     dwViejoOrg = SetWindowOrg (hdc, xin, yin);
     PlayMetaFile (hdc, hMF);
     SetWindowOrg (hdc, LOWORD(dwViejoOrg), HIWORD(dwViejoOrg));
    }

   rc.left   = xin  + 5;
   rc.right  = xfin - 5;
   rc.top    = yin  + 5;
   rc.bottom = yfin - 5;
   if (alv != DT_TOP)
    {
     DrawText (hdc, texto, -1, (LPRECT) &rc, DT_WORDBREAK | DT_CALCRECT);
     rc.left   = xin  + 5;
     rc.right  = xfin - 5;
     if (alv == DT_CENTER)
       rc.top  = yin + ((yfin - yin - 10) - (rc.bottom - rc.top)) / 2 + 5;
     else
       rc.top  = yfin - (rc.bottom - rc.top) - 5;
     rc.bottom = yfin - 5;
    }

   DrawText (hdc, texto, -1, (LPRECT) &rc, DT_WORDBREAK | alh);
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_puerto (InfoBloque NEAR *npIB,
		     HDC hdc, short xin, short yin, short horiz, short vert,
		     short espejo, char texto[], BOOL pinta)
{
 TEXTMETRIC tm;
 HPEN       hPen, hViejoPen;
 char       szBuf[2];
 short      i, longx, longy;

 /* Pasamos del sistema de coordenadas reales al sistema de la ventana */
 xin -= npIB->nHposScrl * SCRLDESPL;
 yin -= npIB->nVposScrl * SCRLDESPL;

 /* Si no pinta, creamos un l piz azul discont¡nuo */
 if (!pinta)
   hPen = CreatePen (PS_DOT, 1, RGB(0,0,255));

 /* pero, si pinta, creamos un l piz normal */
 else
   hPen = CreatePen (PS_SOLID, 1, RGB(0,0,0));

 /* Seleccionamos el l piz */
 hViejoPen = SelectObject (hdc, hPen);

 /* Poner el modo de dibujo a XOR ¢ normal, seg£n el valor de 'pinta' */
 SetROP2 (hdc, (pinta) ? R2_COPYPEN : R2_NOTXORPEN);

 /* Obtenemos el tama¤o del puerto */
 GetTextMetrics (hdc, &tm);
 longx = ((strlen (texto) * tm.tmAveCharWidth + 9) * horiz) +
	 ((tm.tmAveCharWidth + 4) * vert);
 longy = ((tm.tmHeight + 4) * horiz) +
	 ((strlen (texto) * tm.tmHeight + 9) * vert);

 /* Dibujamos el puerto */
 MoveTo (hdc, xin, yin);
 LineTo (hdc, xin + longx, yin);
 LineTo (hdc, xin + longx + 5*horiz, yin + longy/2*horiz + longy*vert);
 LineTo (hdc, xin + longx - longx/2*vert, yin + longy + 5*vert);
 LineTo (hdc, xin, yin + longy);
 LineTo (hdc, xin, yin);

 /* Restauramos el l piz original y eliminamos el otro */
 SelectObject (hdc, hViejoPen);
 DeleteObject (hPen);

 /* Le ponemos el nombre, si lo trae */
 if ((texto != NULL) && pinta)
   if (horiz)
     TextOut (hdc, xin + 2, yin + 2, texto, strlen (texto));
   else
    {
     szBuf[1] = 0;
     for (i = 0; i < strlen (texto); ++i)
      {
       szBuf[0] = texto[i];
       TextOut (hdc, xin + 2, yin + 2 + i*tm.tmHeight, szBuf, 1);
      }
    }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_texto (InfoBloque NEAR *npIB,
		    HDC hdc, short x, short y, short horiz, char texto[],
		    BOOL pinta)
{
 TEXTMETRIC tm;
 char       szBuf[2];
 short      i;

 /* Pasamos del sistema de coordenadas reales al sistema de la ventana */
 x -= npIB->nHposScrl * SCRLDESPL;
 y -= npIB->nVposScrl * SCRLDESPL;

 /* Escribimos el texto */
 if (horiz)
   TextOut (hdc, x, y, texto, strlen (texto));
 else
  {
   GetTextMetrics (hdc, &tm);
   szBuf[1] = 0;
   for (i = 0; i < strlen (texto); ++i)
    {
     szBuf[0] = texto[i];
     TextOut (hdc, x, y + i*tm.tmHeight, szBuf, 1);
    }
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

short dibujar_elembibl (InfoBiblio NEAR *npIb, HDC hdc, short x, short y)
{
 /* Devuelve el tama¤o vertical del MetaFile correspondiente, que de momento
    tomaremos como una constante */
 HPEN      hPen, hViejoPen;
 HMETAFILE hMF;
 char      nom[9], nom_fich[128];

 /* Localizamos el elemento en el ¡ndice y leemos su nombre */
 _llseek (npIb->hIndice, 8*npIb->nPos, 0);
 _lread  (npIb->hIndice, (LPSTR) nom, 8);

 /* Escribimos el nombre en verde claro */
 hPen = CreatePen (PS_SOLID, 1, RGB(0, 255, 0));
 hViejoPen = SelectObject (hdc, hPen);
 TextOut (hdc, x, y, (LPSTR) nom, strlen (nom));
 SelectObject (hdc, hViejoPen);
 DeleteObject (hPen);

 /* Representamos el s¡mbolo */
 strcpy (nom_fich, npIb->path);
 strcat (nom_fich, nom);
 strcat (nom_fich, szExtFich[TV_METAFILE]);
 hMF = GetMetaFile (nom_fich);
 SetWindowOrg (hdc, x, y);
 PlayMetaFile (hdc, hMF);
 DeleteMetaFile (hMF);

 return (MF_TAMVERT);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_biblioteca (InfoBiblio NEAR *npIb, HDC hdc, RECT rect)
{
 /* De momento no tenemos en cuenta 'rect' y tampoco el que pueda haber
    varias columnas */
 POINT ptWndOrg;
 short x = 0, y = 0;

 /* Salvamos el origen de la ventana */
 ptWndOrg.x = LOWORD (GetWindowOrg (hdc));
 ptWndOrg.y = HIWORD (GetWindowOrg (hdc));

 /* Dibujamos todos los elementos que nos quepan en 'rect' */
 for (npIb->nPos = npIb->nPos;
      (npIb->nPos < npIb->n_elems) && (y < rect.bottom); ++npIb->nPos)
   y += dibujar_elembibl (npIb, hdc, x, y);

 /* Ajustamos nPos, que est  desajustada por el bucle anterior */
 --npIb->nPos;

 /* Restauramos el origen */
 SetWindowOrg (hdc, ptWndOrg.x, ptWndOrg.y);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void dibujar_rejilla (HDC hdc, RECT rect)
{
 HPEN  hPen, hViejoPen;
 short i, j;

 /* Ponemos un l piz magenta */
 hPen = CreatePen (PS_SOLID, 1, RGB(255,0,255));
 hViejoPen = SelectObject (hdc, hPen);

 /* Poner el modo de dibujo a XOR */
 SetROP2 (hdc, R2_NOTXORPEN);

 /* Dibujamos la rejilla */
 for (i = ((short)((float) rect.left / TAMREJ)) * TAMREJ; i <= rect.right;
      i += TAMREJ)
   for (j = ((short)((float) rect.top / TAMREJ)) * TAMREJ; j <= rect.bottom;
	j += TAMREJ)
    {
     MoveTo (hdc, i,     j);
     LineTo (hdc, i + 1, j + 1);
    }

 /* Eliminamos el l piz y reseleccionamos el anterior */
 SelectObject (hdc, hViejoPen);
 DeleteObject (hPen);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/