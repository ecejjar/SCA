#include <windows.h>
#include <resolver.h>

extern HWND        hwnd;
extern HANDLE      hr, hw;
extern long        mem_global;
extern unsigned    mem_local;
extern short       cyLine, nVposScrl, minimizada, Fin, Abort;
extern char        frase2[], frase3[], frase4[], frase5[];
extern unsigned    orden, gendep, gendepv, gendepi;
extern double huge *r,
	      huge *w;
extern unsigned    *fg;
extern char        nom_fich[];
extern double      Tact;

extern int         diagonaliza_real (unsigned orden, unsigned max_orden);
extern double huge *RLC (double huge *puntero, unsigned i, unsigned j);
extern gen         (*G (unsigned i))[];

/*Variables globales*/

extern HANDLE      hl, hc;
extern double huge *l,
	      huge *c;
extern gen         (*g)[];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo del c¢digoÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Temporal_por_mallas (unsigned n_elems, elemento *elems,
			 unsigned n_ramas, rama *ramas,
			 unsigned n_ecs, char (*ecs)[100])

/* Esta funci¢n construye las matrices que componen las ecs. diferenciales
   del circuito, por el m‚todo de las mallas, en las posiciones apuntadas
   por 'r', 'l' y 'c', y luego lo pasa a su forma normal */
{
 /* Las variables 'orden',
		  'r', 'l', 'c',
		  'gendep', 'gendepv' y 'gendepi' son globales */

 double A, Z;
 int signo, elem_aux, elem_act, op;
 unsigned a, b, i, j, k;
 long tamano;
 char Zij[100], nombre_elem[5], *punt;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern char *obtener_comunes (char Hii[100], char Hjj[100], char Hij[100]);

 void exp_var_Temporal (unsigned n_elems, elemento *elems, unsigned e,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs, char (*ecs)[100],
			unsigned fila, char t);
 int formar_sistema_EDO (unsigned orden);

/*
 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º                                                                          º
 º   Comienzo de la rutina de an lisis por mallas en el dominio del tiempo  º
 º                                                                          º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/
 gendepv = gendepi = 0;

 /* Cuenta los generadores dependientes de V y de I */
 for (i = 0; i < n_elems; ++i)
   if ((elems[i].tipo[0] == 'A') || (elems[i].tipo[0] == 'B'))
     ++gendepv;
   else
     if ((elems[i].tipo[0] == 'D') || (elems[i].tipo[0] == 'E'))
       ++gendepi;

 gendep = gendepv + gendepi;     /* Calcula el n§ total de generadores dep. */

 /* El orden del sub-sistema de ecuaciones de malla ser  igual a n_ecs */
 orden = n_ecs;

 /* Creamos las matrices del circuito */
 tamano = (orden + gendep) * (orden + gendep) * sizeof (double);
 hr = GlobalAlloc (GMEM_MOVEABLE, tamano);
 r  = (double huge *) GlobalLock (hr);
 hl = GlobalAlloc (GMEM_MOVEABLE, tamano);
 l  = (double huge *) GlobalLock (hl);
 hc = GlobalAlloc (GMEM_MOVEABLE, tamano);
 c  = (double huge *) GlobalLock (hc);
 g  = (gen (*)[]) calloc (orden + gendep, 10 * sizeof (gen));
 fg = (unsigned *) malloc (gendep * sizeof (unsigned));

 mem_global += GlobalSize (hr) + GlobalSize (hl) + GlobalSize (hc);
 mem_local  += LocalSize (LocalHandle ((WORD) g)) +
	       LocalSize (LocalHandle ((WORD)fg));

 /* Inicializamos las matrices a todo ceros */
 _fmemset (r, 0, tamano);
 _fmemset (l, 0, tamano);
 _fmemset (c, 0, tamano);
 memset (g, 0, (orden + gendep) * 10 * sizeof (gen));

/* El elemento de la fila i, col. j est  compuesto por los elementos que son
   comunes a la malla i y a la malla j */

 /* Con el puntero 'i' recorremos todos los elems. de la diagonal princ. */
 for (i = 0; i < orden; ++i)
  {
   /* Buscamos generadores de corriente en la malla 'i' */
   if ((punt = strstr (ecs[i], "D")) == NIL)
     if ((punt = strstr (ecs[i], "E")) == NIL)
       punt = strstr (ecs[i], "I");

   /* Si los hay, la ec. a construir ser  "Ii = cte_gen x Idep", si el gen.
      es dependiente, o "Ii = valor_gen" si es independiente */
   if (punt != NIL)
    {
     /* Primero obtenemos su nombre (interno) en 'nombre_elem' */
     obtener_nombre (punt, nombre_elem);

     /* Despu‚s lo buscamos y obtenemos su posici¢n en 'elem_aux' */
     elem_aux = buscar_elem (n_elems, elems, nombre_elem);

     /* Finalmente obtenemos su valor ¢ su cte., con signo (+) */
     Z = elems[elem_aux].valor;

     /* Por la propiedades del algoritmo de construcci¢n de las mallas, el
	sentido de recorrido de una malla queda determinado por la posi-
	ci¢n de los nodos inicial y final de la rama de enlace que origina
	dicha malla.
	Como un gen. de corriente siempre es rama de enlace, el sentido de
	recorrido de la malla formada con una rama que contenga gen. dep.
	de corriente es siempre el mismo que el de la corriente generada
	por ‚ste.
	Finalmente conclu¡mos que si la rama com£n de las dos mallas en
	comparaci¢n (la normal y la que contiene gen. de corriente dep.)
	tiene el mismo signo en ambas, el t‚rmino "cte_gen x äZ" tendr 
	signo (+), y signo (-) en caso contrario.
	Esto hace innecesaria ninguna consideraci¢n sobre el signo que
	debe tener la cte. del gen., ya que este signo ya se le asigna
	posteriormente al äZ de la rama seg£n el mismo criterio anterior. */

     /* Tanto si el gen. es dep. o indep. el t‚rmino Ii valdr  1 */
     *RLC(r,i,i) = 1;

     /* Si el generador es independiente.. */
     if (*punt == 'I')
      {
       /* ..constru¡mos la ec. "Ii = valor_gen" */
       for (k = 0; ((*G(i))[k].puntero != 0) &&
		   ((*G(i))[k].puntero != elem_aux + 1); ++k);
       (*G(i))[k].coef[0] += 1;
       (*G(i))[k].puntero = elem_aux + 1;
      }

     /* Pero, si el generador es dependiente.. */
     else
      {
       /* ..constru¡mos la ec. "Ii - cte_gen x Idep = 0" */
       k = atoi ((char *) &elems[elem_aux].tipo[1]);

       /* Si el generador es de tipo 'D'.. */
       if (*punt == 'D')

	 /* ..su cte. ir  en (i, orden + gendepv + k - 1) */
	 k += orden + gendepv - 1;

       /* pero, si es de tipo 'E'.. */
       else

	 /* ..su cte. ir  en (i, orden + gendep - k) */
	 k = orden + gendep - k;

       /* Y, si es la 1¦ vez que se le procesa, ponemos la expresi¢n de
	  la variable de la que depende en la fila 'k' */
       if (*RLC(r, k, k) == 0)
	{
	 exp_var_Temporal (n_elems, elems, elem_aux, n_ramas, ramas, n_ecs,
			   ecs, k, 'Z');

	 /* y, adem s, un puntero al elemento en la matriz 'f' */
	 fg[k - orden] = elem_aux;
	}

       /* Finalmente a¤adimos el t‚rmino "-cte_gen x Idep" */
       *RLC(r,i,k) = -Z;
      }
    }

   /* Si no hay gens. de I en la malla, constru¡mos la fila normalmente */
   else
   /* Idem con 'columna', formando dos bucles anidados que construyen la
      matriz Z. El elemento que construimos en cada pasada es el (i,j) */

   for (j = 0; j < orden; ++j)
    {
     /* Obtenemos en Zij los nombres (internos) concatenados de todos los
	elementos comunes a las posiciones de la diagonal principal apuntados
	por 'i' y 'j' */

     if (i == j)
       strcpy (Zij, ecs[i]);
     else
       obtener_comunes (ecs[i], ecs[j], Zij);

     a = 0;                            /* Apuntamos a la 1¦ posici¢n de Zij */

     while (Zij[a] != 0)         /* Repetir hasta que no haya m s elementos */
     {
      if (Zij[a] == '-')
       {
	signo = -1;
	a += 2;
       }                          /* Primero, obtenemos el valor del signo, */
      else                        /* adem s de saltarnos el primer car cter */
       if (Zij[a] == '(')         /* , si es un '-' ¢ un ')'. El signo per- */
	{                         /* manece cte. hasta que se encuentra un  */
	 signo = 1;               /* nuevo signo ¢ un '('.                  */
	 ++a;
	}

      /* Obtenemos el nombre del siguiente elemento de la malla */
      obtener_nombre ((char *) &Zij[a], nombre_elem);

      /* Apuntamos al car cter siguiente al nombre */
      a += 4;

      if (Zij[a] == ')')                         /* Finalmente nos saltamos */
	++a;                                     /* el ')', si lo hay.      */

      /* Buscamos el nombre en la lista de elementos */
      elem_act = buscar_elem (n_elems, elems, nombre_elem);

      /* Obtenemos el valor del elemento */
      A = (nombre_elem[0] == 'C') ? 1 / elems[elem_act].valor
				  : elems[elem_act].valor;

      /* Si el elemento es pasivo... */
      if ((nombre_elem[0] == 'R') || (nombre_elem[0] == 'L') ||
	  (nombre_elem[0] == 'C'))
       {
	/* El signo s¢lo cuenta para los elems. que no est‚n en la diagonal */
	if (i != j)
	  A *= signo;

	/* A¤adimos el valor a la fila y columna correspondientes */
	if (nombre_elem[0] == 'R')
	  *RLC(r, i, j) += A;
	else
	  if (nombre_elem[0] == 'L')
	    *RLC(l, i, j) += A;
	  else
	    if (nombre_elem[0] == 'C')
	      *RLC(c, i, j) += A;
       }

      /* pero si es activo (generador)... */
      else

       /* ..s¢lo se le procesar  una vez; cuando fila = col. */
       if (i == j)
	{
	 /* De nuevo n¢tese que no pueden aparecer gen. de corriente en
	    la malla actual ya que, si los hay, la fila no se construye */

	 /* Entonces, si es de tensi¢n e independiente.. */
	 if (nombre_elem[0] == 'V')
	  {
	   /* ..a¤adimos su valor al t‚rmino independiente de la fila 'i',
	      cambiado de signo al cambiar de t‚rmino */
	   for (k = 0; ((*G(i))[k].puntero != 0) &&
		       ((*G(i))[k].puntero != elem_act + 1); ++k);
	   (*G(i))[k].puntero = elem_act + 1;
	   (*G(i))[k].coef[0] -= signo;
	  }

	 /* pero si no es independiente (por tanto es dependiente).. */
	 else
	  {
	   /* Leemos el n§ de orden del generador */
	   k = atoi ((char *) &nombre_elem[1]);

	   /* Si el generador es de tipo 'A'.. */
	   if (nombre_elem[0] == 'A')

	     /* ..su cte. ir  en la posici¢n (i, orden + k - 1) */
	     k += orden - 1;

	   /* , pero si es de tipo 'B'.. */
	   else

	     /* ..entonces la cte. ir  en (i, orden + gendepv - k) */
	     k = orden + gendepv - k;

	   /* Ponemos en la col. correspondiente la cte. del generador con el
	      signo adecuado, que es igual al de la rama dentro de la malla*/
	   *RLC(r, i, k) = signo * A;

	   /* Y si es la 1¦ vez que se le procesa, en la misma fila ponemos
	      la expresi¢n de la variable de la que depende */
	   if (*RLC(r, k, k) == 0)
	    {
	     exp_var_Temporal (n_elems, elems, elem_act, n_ramas, ramas, n_ecs,
			       ecs, k, 'Z');

	     /* y, adem s, un puntero al elemento en la matriz 'f' */
	     fg[k - orden] = elem_act;
	    }
	  }
	}
     }
    }
  }

 /* Pasamos el sistema a uno de EDO's */
 formar_sistema_EDO (orden + gendep);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Temporal_por_nudos (unsigned n_elems, elemento *elems,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs, char (*ecs)[100])

/* Esta funci¢n construye las matrices que componen las ecs. diferenciales
   del circuito, por el m‚todo de las mallas, en las posiciones apuntadas
   por 'r', 'l' y 'c', y luego lo pasa a su forma normal */
{
 /* Las variables 'orden',
		  'r', 'l', 'c',
		  'gendep', 'gendepv', 'gendepi' son globales */

 long     tamano;
 double   A, S, Zr, Zl;
 unsigned a, i, j, k, ramas_cond,
	  elem_act, elem_aux, rama_act;
 int      op;
 char     Yij[100], nombre_elem[5], *punt, *aux;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern int buscar_rama (unsigned n_ramas, rama *ramas, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern char *obtener_comunes (char Hii[100], char Hjj[100], char Hij[100]);

 void exp_var_Temporal (unsigned n_elems, elemento *elems, unsigned e,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs, char (*ecs)[100],
			unsigned fila, char t);
 double sumZ (char tipo, unsigned n_elems, elemento *elems,
			 rama *ramas, unsigned r);
 int formar_sistema_EDO (unsigned orden);

/*
 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º                                                                          º
 º   Comienzo de la rutina de an lisis por nudos en el dominio del Tiempo   º
 º                                                                          º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/

 /* Contamos el n§ de gens. dep. y de condensadores */
 gendepv = gendepi = ramas_cond = 0;

 for (i = 0; i < n_elems; ++i)
   if ((elems[i].tipo[0] == 'A') || (elems[i].tipo[0] == 'B'))                     /* tanto de V como de */
     ++gendepv;
   else
     if ((elems[i].tipo[0] == 'D') || (elems[i].tipo[0] == 'E'))
       ++gendepi;
     else
       if (elems[i].tipo[0] == 'C')
	 ++ramas_cond;

 gendep = gendepv + gendepi;     /* Calcula el n§ total de generadores dep. */

 /* El orden del sub-sistema de ecs. de nudo ser  el n§ de nudos menos uno */
 orden = n_ecs + ramas_cond;

 /* Creamos las matrices del circuito */
 tamano = (orden + gendep) * (orden + gendep) * sizeof (double);
 hr = GlobalAlloc (GMEM_MOVEABLE, tamano);
 r  = (double huge *) GlobalLock (hr);
 hl = GlobalAlloc (GMEM_MOVEABLE, tamano);
 l  = (double huge *) GlobalLock (hl);
 hc = GlobalAlloc (GMEM_MOVEABLE, tamano);
 c  = (double huge *) GlobalLock (hc);
 g = (gen (*)[]) calloc (orden + gendep, 10 * sizeof (gen));
 fg = (unsigned *) malloc (gendep * sizeof (unsigned));

 mem_global += GlobalSize (hr) + GlobalSize (hl) + GlobalSize (hc);
 mem_local  += LocalSize (LocalHandle ((WORD) g)) +
	       LocalSize (LocalHandle ((WORD)fg));

 /* Inicializamos las matrices a todo ceros */
 _fmemset (r, 0, tamano);
 _fmemset (l, 0, tamano);
 _fmemset (c, 0, tamano);
 memset (g, 0, (orden + gendep) * 10 * sizeof (gen));

 /* El elemento de la fila i, columna j est  compuesto por los elementos que
    son comunes al nodo i y al nodo j */

 /* Con el puntero 'i' recorremos todos los elems. de la diagonal princ. */
 for (i = 0; i < n_ecs; ++i)
  {
   /* Buscamos generadores de V en el nudo 'i' */
   if ((punt = strstr (ecs[i], "A")) == NIL)
     if ((punt = strstr (ecs[i], "B")) == NIL)
       punt = strstr (ecs[i], "V");

   /* Si hab¡a alguno.. */
   if (punt != NIL)
    {
     /* ..nos situamos en el primer car cter de la rama despu‚s del '(' */
     aux = punt;
     while (*(aux - 1) != '(')
       --aux;

     /* Recorremos la rama hasta encontrar alguna Z ¢ el fin de la rama */
     while ((*aux != 'R') && (*aux != 'L') && (*aux != 'C') && (*aux != ')'))
       aux += 4;
    }

   /* Si hab¡a alguno.. */
   if ((punt != NIL) && (*aux == ')'))
    {
     /* ..nos situamos en el primer car cter de la rama despu‚s del '(' */
     while (*(punt - 1) != '(')
       --punt;

     /* A¤adimos al sistema todos los generadores que haya en la rama */
     do
     {
      /* Primero obtenemos su nombre (interno) en 'nombre_elem' */
      obtener_nombre (punt, nombre_elem);

      /* Despu‚s lo buscamos y obtenemos su posici¢n en 'elem_aux' */
      elem_aux = buscar_elem (n_elems, elems, nombre_elem);

      /* Finalmente obtenemos su valor ¢ su cte., con signo (+) */
      Zr = elems[elem_aux].valor;

      /* Tanto como si el gen. es dep. ¢ indep. pondremos Vi a 1 y Vj a -1 */
      *RLC(r, i, i) = 1;
      rama_act = buscar_rama (n_ramas, ramas, nombre_elem);

      if ((ramas[rama_act].nodoi > 0) && (ramas[rama_act].nodof > 0))
       {
	if (ramas[rama_act].nodoi == (i + 1))
	  *RLC(r, i, ramas[rama_act].nodof - 1) = -1;
	else
	 if (ramas[rama_act].nodof == (i + 1))
	   *RLC(r, i, ramas[rama_act].nodoi - 1) = -1;
       }

      /* Si el generador es independiente.. */
      if (*punt == 'V')
       {
	/* ..buscamos un lugar libre en la matriz de generadores */
	for (k = 0; ((*G(i))[k].puntero != 0) &&
		    ((*G(i))[k].puntero != elem_aux + 1); ++k);

	/* Completamos la ec. "Vi - Vj = ñvalor_gen" */
	(*G(i))[k].coef[0] += (ramas[rama_act].nodoi == (i + 1)) ? 1 : -1;
	(*G(i))[k].puntero = elem_aux + 1;
       }

      /* pero, si el generador es dependiente.. */
      else
       {
	/* ..completamos la ec. "Vi = ñcte_gen x Vdep + Vj" */
	k = atoi ((char *) &elems[elem_aux].tipo[1]);

	/* Si el generador es de tipo 'A'.. */
	if (elems[elem_aux].tipo[0] == 'A')

	  /* ..su cte. ir  en la posici¢n 'orden + k - 1' */
	  k += orden - 1;

	/* pero, si el generador es de tipo 'B'.. */
	else

	  /* ..su cte. ir  en la posici¢n 'orden + gendepv - k' */
	  k = orden + gendepv - k;

	/* Si el nudo 'i' no es el inicial (y por tanto tampoco es el +).. */
	if (ramas[rama_act].nodoi != (i + 1))

	  /* ..a¤adimos el t‚rmino "+cte_gen" a la columna correspondiente */
	  *RLC(r, i, k) += Zr;

	/* pero, si es el inicial (y por tanto el +).. */
	else

	  /* ..a¤adimos el t‚rmino "-cte_gen" a la columna correspondiente */
	  *RLC(r, i, k) -= Zr;

	/* Y, si es la primera vez que se le procesa, ponemos su expresi¢n en
	   la fila 'k' */
	if (*RLC(r, k, k) == 0)
	 {
	  exp_var_Temporal (n_elems, elems, elem_aux, n_ramas, ramas, n_ecs,
			    ecs, k, 'Y');

	  /* y, adem s, un puntero al elemento en la matriz 'f' */
	  fg[k - orden] = elem_aux;
	 }
       }

      /* Avanzamos hasta el siguiente nombre (¢ el ')') */
      punt += 4;
     }
     while (*punt != ')');

     /* Generamos las expresiones de las variables de las que dependen los
	generadores dependientes que est‚n conectados al nudo */
     punt = ecs[i];
     do
     {
      /* Nos saltamos el '(' */
      if (*punt == '(')
	++punt;

      /* Obtenemos un nombre de elemento */
      obtener_nombre (punt, nombre_elem);

      /* Lo buscamos y obtenemos su posici¢n en 'elem_aux' */
      elem_aux = buscar_elem (n_elems, elems, nombre_elem);

      /* Obtenemos su n£mero de orden */
      k = atoi (&nombre_elem[1]);

      switch (nombre_elem[0])
      {
       case 'A':
	 k += orden - 1;
	 break;

       case 'B':
	 k = orden + gendepv - k;
	 break;

       case 'D':
	 k += orden + gendepv - 1;
	 break;

       case 'E':
	 k = orden + gendep - k;
	 break;

       default:
	 k = 0;
      }

      /* Obtenemos la expresi¢n de la variable */
      if ((*RLC(r,k,k) == 0) && (k != 0))
       {
	exp_var_Temporal (n_elems, elems, elem_aux, n_ramas, ramas,
			  n_ecs, ecs, k, 'Y');

	/* y un puntero al elemento en 'fg' */
	fg[k - orden] = elem_aux;
       }

      /* Avanzamos hasta el siguiente nombre */
      punt += 4;

      /* Nos saltamos el ')' */
      if (*punt == ')')
	++punt;
     }
     while (*punt != 0);
    }

   /* Si no hay generadores de este tipo, construimos la fila normalmente */
   else

   /* Idem con 'columna', formando dos bucles anidados que construyen la
      matriz Y. El elemento que construimos en cada pasada es el (i,j) */
   for (j = 0; j < n_ecs; ++j)
   {
    /* Obtenemos en Yij los nombres (internos) concatenados de todos los
       elementos comunes a las posiciones de la diagonal principal apuntados
       por 'i' y 'j' */

    if (i == j)
      strcpy (Yij, ecs[i]);
    else
      obtener_comunes (ecs[i], ecs[j], Yij);

    a = 0;                             /* Apuntamos a la 1¦ posici¢n de Yij */
    Zr = Zl = 0;       				    /* Ponemos el äZ a cero */

    while (Yij[a] != 0)          /* Repetir hasta que no haya m s elementos */
     {
      /* Obtenemos el nombre (interno) de un elemento en 'nombre_elem' */

      if (Yij[a] == '(')                            /* Nos saltamos el '(', */
       ++a;                                         /* si  lo  hay.         */

      /* Obtenemos el nombre del siguiente elemento de la ec. */
      obtener_nombre ((char *) &Yij[a], nombre_elem);

      /* Apuntamos al car cter siguiente al nombre */
      a += 4;

      /* Buscamos el nombre en la lista de elementos */
      elem_act = buscar_elem (n_elems, elems, nombre_elem);

      /* Obtenemos el valor del elemento */
      A = elems[elem_act].valor;

      /* Si el elemento es R ¢ L, a¤adimos su Z a la suma de Z's de la rama */
      if (nombre_elem[0] == 'R')
	Zr += A;
       else
	 if (nombre_elem[0] == 'L')
	   Zl += A;
	 else
	   if (nombre_elem[0] == 'C')
	    {
	     /* ..obtenemos su posici¢n en el sistema */
	     k = n_ecs + atoi (&elems[elem_act].tipo[1]) - 1;

	     /* Buscamos la rama que ocupa */
	     rama_act = buscar_rama (n_ramas, ramas, nombre_elem);

	     /* Ponemos el coef. de la I que lo atraviesa a +-1 */
	     *RLC(r, i, k) = (ramas[rama_act].nodoi == (i + 1)) ? 1 : -1;

	     /* Constru¡mos la ecuaci¢n de dicha corriente */
	     if (ramas[rama_act].nodoi > 0)
	       *RLC(l, k, ramas[rama_act].nodoi - 1) =  1;

	     if (ramas[rama_act].nodof > 0)
	       *RLC(l, k, ramas[rama_act].nodof - 1) = -1;

	     *RLC(r, k, k) = -1 / A;
	    }

      /* pero si es activo (generador ¢ modelo de diodo)... */
      else

       /* ..s¢lo se le debe procesar una vez; p. ej., cuando fila = col... */
       if (i == j)
	{
	 /* ..buscamos la rama en la que se encuentra el generador */
	 rama_act = buscar_rama (n_ramas, ramas, nombre_elem);

	 /* Le damos el signo que debe llevar; este es (+) si la corriente
	    sale del nudo (borna - est  en el nudo) y (-) en caso contrario */
	 if (((ramas[rama_act].nodoi != (i+1)) && ((nombre_elem[0] == 'I') ||
						   (nombre_elem[0] == 'D') ||
						   (nombre_elem[0] == 'E')))
	  || ((ramas[rama_act].nodoi == (i+1)) && ((nombre_elem[0] == 'V') ||
						   (nombre_elem[0] == 'A') ||
						   (nombre_elem[0] == 'B') ||
						   (nombre_elem[0] == 'Q'))))
	       A = -A;

	 /* Entonces, si es de corriente e independiente.. */
	 if (nombre_elem[0] == 'I')
	  {
	   /* ..a¤adimos su valor al t‚rmino independiente de la fila 'i',
	      cambiado de signo al cambiar de t‚rmino */
	   for (k = 0; ((*G(i))[k].puntero != 0) &&
		       ((*G(i))[k].puntero != elem_act + 1); ++k);

	   (*G(i))[k].coef[0] -= A / fabs(A);
	   (*G(i))[k].puntero = elem_act + 1;
	  }

	 /* pero, si es de tensi¢n e independiente.. */
	 else
	  if (nombre_elem[0] == 'V')
	   {
	    /* ..y a¤adimos su valor, dividido por la suma de Z's de la rama
	       y cambiado de signo al pasar del 1er t‚rmino al 2§, al t‚rmino
	       independiente de la fila 'i' */
	    for (k = 0; ((*G(i))[k].puntero != 0) &&
			((*G(i))[k].puntero != elem_aux + 1); ++k);

//	    if ((S = sumZ ('L', n_elems, elems, ramas, rama_act)) != 0)
//	      (*G(i))[k].coef[-1] -= (A / fabs(A)) / S;

	    if ((S = sumZ ('R', n_elems, elems, ramas, rama_act)) != 0)
	      (*G(i))[k].coef[0] -= (A / fabs(A)) / S;

	    if ((S = sumZ ('C', n_elems, elems, ramas, rama_act)) != 0)
	      (*G(i))[k].coef[1] -= (A / fabs(A)) / S;

	    (*G(i))[k].puntero = elem_act + 1;
	   }

	  /* pero si no es independiente (por tanto es dependiente).. */
	  else
	   {
	    /* ..leemos el n§ de orden del generador */
	    k = atoi ((char *) &nombre_elem[1]);

	    /* Si el generador es de tipo 'A'.. */
	    if (nombre_elem[0] == 'A')

	      /* ..su cte. ir  en la posici¢n (i, orden + k - 1) */
	      k += orden - 1;

	    /* , pero si es de tipo 'B'.. */
	    else
	      if (nombre_elem[0] == 'B')

		/* ..entonces la cte. ir  en (i, orden + gendepv - k) */
		k = orden + gendepv - k;

	      /* , pero si es de tipo 'D'.. */
	      else
		if (nombre_elem[0] == 'D')

		  /* ..entonces la cte. ir  en (i, orden + gendepv + k - 1) */
		  k += orden + gendepv - 1;

		/* pero, si es de tipo 'E'.. */
		else

		  /* ..entonces la cte. ir  en (i, orden + gendep - k) */
		  k = orden + gendep - k;

	    /* Ponemos en las cols. correspondientes la cte. del generador */
	    if (nombre_elem[0] > 'C')
	      *RLC(r, i, k) += A;
	    else
	     {
	      if ((S = sumZ ('L', n_elems, elems, ramas, rama_act)) != 0)
		*RLC(c,i,k) += A / S;

	      if ((S = sumZ ('R', n_elems, elems, ramas, rama_act)) != 0)
		*RLC(r,i,k) += A / S;

	      if ((S = sumZ ('C', n_elems, elems, ramas, rama_act)) != 0)
		*RLC(l,i,k) += A * S;
	     }

	    /* Y si es la 1¦ vez que se le procesa, en la misma fila ponemos
	       la expresi¢n de la variable de la que depende */
	    if (*RLC(r, k, k) == 0)
	     {
	      exp_var_Temporal (n_elems, elems, elem_act, n_ramas, ramas, n_ecs,
				ecs, k, 'Y');

	      /* y, adem s, un puntero al elemento en la matriz 'f' */
	      fg[k - orden] = elem_act;
	     }
	   }
	}

      if (Yij[a] == ')')
       {
	if (Zl != 0)
	 {
	  if (i != j)
	    Zl = -Zl;                       /* A¤adimos las admitancias a   */
					    /* las matrices 'r', 'l' y 'c'. */
	  *RLC(c,i,j) += 1 / Zl;            /* N¢tese que la Z inductiva Zl */
					    /* se a¤ade a la matriz 'c' y   */
	  Zl = 0;                           /* la capacitiva Zc se a¤ade a  */
	 }                                  /* la matriz 'l', debido a que  */
					    /* ahora un condensador deriva  */
	if (Zr != 0)                        /* y una bobina integra.        */
	 {
	  if (i != j)
	    Zr = -Zr;

	  *RLC(r,i,j) += 1 / Zr;

	  Zr = 0;
	 }

	++a;
       }
     }
   }
  }

 /* Pasamos el sistema a uno de EDO's */
 formar_sistema_EDO (orden + gendep);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double sumZ (char tipo, unsigned n_elems, elemento *elems,
			rama *ramas, unsigned r)

/* Calcula la Z de tipo 'tipo' (R, L ¢ C) en la rama 'rama' */
{
 double Z;
 char *pt;
 char nombre[5];
 unsigned i;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);

 Z = 0;                                     /* Inicializamos la suma de Z's */

 pt = ramas[r].elem;             /* Apuntamos al 1er car cter de los elems. */

 do                                                           /* Repetir... */
  {
   /* Tomamos un nombre interno de elemento de la rama */
   obtener_nombre (pt, nombre);
   pt += 4;

   /* Si el elemento es pasivo a¤adimos su valor a la suma de Z's */
   if (nombre[0] == tipo)
     Z += elems[buscar_elem (n_elems, elems, nombre)].valor;
  }
 while (*pt != 0);                    /* ...hasta que no haya m s elementos */

 return (Z);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void exp_var_Temporal (unsigned n_elems, elemento *elems, unsigned e,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs, char (*ecs)[100],
		       unsigned fila, char t)

/* Esta funci¢n pone en la fila 'fila' la expresi¢n de la variable de la que
   depende el generador apuntado por 'elem', es decir;
   Si el gen. depende de una 'Vdep':
	      Vdep = (L1+L2+..+Ln)ú(ñIi'ñIj'ñ..) + (R1+R2+..+Rm)ú(Ii+Ij+..)..
   Si el gen. depende de una 'Idep':
	      Idep = ñ Ii ñ Ij ñ ...                                        */
{
 double Zr, Zl, Zc, Yr, Yl, Yc, val;
 char *pt, *aux, nombre[5], cam[100];
 int op;
 unsigned ni, nf, b, i, j, elem_bus, rm;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern int buscar_rama (unsigned n_ramas, rama *ramas, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern int camino (unsigned n_ramas, rama *ramas,
		    unsigned salida, unsigned llegada, char elem[100]);

 /* Ponemos un 1 en 'fila' y la col. correspondiente a este gen., de modo que
    empezamos la ec. "Vdep = ..." (Vdep = V de la que dep. el gen.)
	      o bien "Idep = ..." (Idep = I "   "  "   "   "   "  ) */
  *RLC(r, fila, fila) = 1;

  /* Si queremos la expresi¢n en Z... */
  if (t == 'Z')
   {
    /* Si el generador depende de una corriente... */
    if ((elems[e].tipo[0] == 'B') || (elems[e].tipo[0] == 'E'))
     {
      /* ...ponemos en 'nombre' el del elem. por el que circula la Idep */
      strcpy (nombre, elems[e].caract.elem);

      /* Buscamos las mallas en las que aparece el elem. con la variable */
      for (i = 0; i < n_ecs; ++i)	     /* Recorremos todas las mallas */
       {
	/* Si el elem. con la var. de la que dep. el gen. est  en la malla i.. */
	if ((pt = strstr (ecs[i], nombre)) != NIL)
	 {
	  while (*(pt--) != '(');   /* Retrocedemos a la posici¢n del signo */

	  /* A¤adimos el valor correspondiente pero con el signo cambiado ya
	     que pasamos del 2§ t‚rmino al 1§ */
	  *RLC(r, fila, i) = (*pt == '-') ? 1 : -1;
	 }
       }
     }

    /* Pero si el gen. depende de una tensi¢n... */
    else
     {
      /* ..inicializamos las sumas de impedancias de la rama */
      Zr = Zl = Zc = 0;

      /* Buscamos un camino que una los nodos de medida de la misma y que no
	 contenga gen. de corriente */
      camino (n_ramas, ramas,
	      elems[e].caract.nodos[0], elems[e].caract.nodos[1], cam);

      /* Obtenemos la expresi¢n de la tensi¢n entre los nudos */
      pt = cam;
      do
      {
       /* Obtenemos el signo representativo del sentido de recorrido */
       op = (*pt == '-') ? RESTA : SUMA;
       pt += (*pt == '-') ? 2 : 1;

       /* Calculamos la suma de las Z's de la rama */
       Zr = Zl = Zc = 0;
       do
       {
	/* Tomamos un nombre interno de elemento de la rama */
	obtener_nombre (pt, nombre);

	/* Apuntamos al siguiente nombre */
	pt += 4;

	/* Obtenemos el valor del elemento cuyo nombre hemos tomado */
	elem_bus = buscar_elem (n_elems, elems, nombre);
	val = elems[elem_bus].valor;

	/* Si el elemento es pasivo, a¤adimos su Z a la que corresponda */
	if (nombre[0] == 'R')
	  Zr += val;
	else
	  if (nombre[0] == 'L')
	    Zl += val;
	  else
	    if (nombre[0] == 'C')
	      Zc += 1 / val;

	/* pero si es un generador.. */
	else

	  /* ..entonces, si es independiente.. */
	  if (nombre[0] == 'V')
	   {
	    /* ..ponemos su valor en el t‚rmino indep. con el signo normal */
	    for (j = 0; ((*G(fila))[j].puntero != 0) &&
			((*G(fila))[j].puntero != elem_bus + 1); ++j);
	    (*G(fila))[j].coef[0] = op;
	    (*G(fila))[j].puntero = elem_bus + 1;
	   }

	  /* Pero, si es dependiente.. */
	  else
	   {
	    j = atoi ((char *) &nombre[1]);      /* Obtenemos su n§ de orden */

	    /* Si el gen. es del tipo 'A'.. */
	    if (nombre[0] == 'A')

	      /* ..su valor ir  en la col. 'orden + j - 1' */
	      j += orden - 1;

	    /* Sin embargo, si es del tipo 'B'.. */
	    else

	      /* ..su valor ir  en la col. 'orden + gendep - j' */
	      j = orden + gendep - j;

	    /* Ponemos el valor en la col. correspondiente, cambiado de signo */
	    *RLC(r, fila, j) = -op * val;

	    /* y un puntero al elemento en la matriz 'f' */
	    fg[j - orden] = elem_bus;
	   }
       }
       while (*pt != ')');

      /* Ponemos la Z de esta rama en todos los t‚rminos de las corrientes
	 que la atraviesan. N¢tese que en 'nombre' est  el del £ltimo ele-
	 mento de la rama. */
      for (i = 0; i < n_ecs; ++i)

	/* Si la rama est  en la malla 'i'.. */
	if ((aux = strstr (ecs[i], nombre)) != NIL)
	 {
	  /* ..retrocedemos hasta la posici¢n del signo */
	  while (*(aux--) != '(');

	  /* Si la I de la malla tiene el mismo sentido que el de recorrido
	     del camino (op), la V en la rama ser  tomada positiva, y al pa-
	     sar del 2§ t‚rmino al 1§ se pondr  negativa */
	  *RLC(l, fila, i) += ((*aux == '-') ? SUMA : RESTA) * op * Zl;
	  *RLC(r, fila, i) += ((*aux == '-') ? SUMA : RESTA) * op * Zr;
	  *RLC(c, fila, i) += ((*aux == '-') ? SUMA : RESTA) * op * Zc;
	 }
      }
      while (*(++pt) != 0);
     }
   }

  /* pero, si queremos la expresi¢n en Y... */
  else
   {
    /* ..si el generador depende de una corriente... */
    if ((elems[e].tipo[0] == 'B') || (elems[e].tipo[0] == 'E'))
     {
      /* ...la ec. a construir es "Idep = (Vi - Vf - äEi) / äZ" */

      /* Buscamos en qu‚ rama est  el elem. del que depende el gen. */
      rm = buscar_rama (n_ramas, ramas, elems[e].caract.elem);

      /* Si en dicha rama hay a su vez un gen. de corriente independiente.. */
      if ((pt = strstr (ramas[rm].elem, "I")) != NIL)
       {
	/* ..la ec. ser  "Idep = Igen" */
	obtener_nombre (pt, nombre);
	pt += 4;
	elem_bus = buscar_elem (n_elems, elems, nombre);

	for (j = 0; ((*G(fila))[j].puntero != 0) &&
		    ((*G(fila))[j].puntero != elem_bus + 1); ++j);
	(*G(fila))[j].coef[0] = elems[elem_bus].valor;
	(*G(fila))[j].puntero = elem_bus + 1;
       }

      /* pero, si hay un gen. de corriente dependiente.. */
      else
       {
	if ((pt = strstr (ramas[rm].elem, "D")) == NIL)
	  pt = strstr (ramas[rm].elem, "E");

	if (pt != NIL)
	 {
	  /* ..la ec. ser  "Idep = cte_gen x Idep'" */

	  /* Tomamos el nombre interno del generador */
	  obtener_nombre (pt, nombre);

	  /* Lo buscamos en la lista */
	  elem_bus = buscar_elem (n_elems, elems, nombre);

	  /* Obtenemos su n§ de orden */
	  j = atoi ((char *) &nombre[1]);

	  /* Calculamos su col. asignada seg£n su tipo */
	  if (nombre[0] == 'D')
	    j += orden + gendepv - 1;
	  else
	    j = orden + gendep - j;

	  /* Ponemos en dicha col. el valor 'cte_gen', cambiado de signo */
	  *RLC(r, fila, j) -= elems[elem_bus].valor;

	  /* y un puntero al generador en 'fg' */
	  fg[j - orden] = elem_bus;
	 }

	/* y si, finalmente, no hay ning£n gen. de corriente en la rama.. */
	else
	{
	 /* Calculamos la suma de las Z's de la rama y su inversa */
	 Zl = sumZ ('L', n_elems, elems, ramas, rm);
	 Zr = sumZ ('R', n_elems, elems, ramas, rm);
	 Zc = sumZ ('C', n_elems, elems, ramas, rm);
	 Yl = (Zl == 0) ? 0 : 1 / Zl;
	 Yr = (Zr == 0) ? 0 : 1 / Zr;
	 Yc = (Zc == 0) ? 0 : 1 / Zc;

	 /* Ponemos -1/Z en 'fila' y la col. correspondiente a Vi */
	 if ((ni = ramas[rm].nodoi) > 0)
	  {
	   *RLC(l, fila, ni - 1) = -Yc;
	   *RLC(r, fila, ni - 1) = -Yr;
	   *RLC(c, fila, ni - 1) = -Yl;
	  }

	 /* Ponemos 1/Z en 'fila' y la col. correspondiente a Vf */
	 if ((nf = ramas[rm].nodof) > 0)
	  {
	   *RLC(l, fila, nf - 1) = Yc;
	   *RLC(r, fila, nf - 1) = Yr;
	   *RLC(c, fila, nf - 1) = Yl;
	  }

	 /* A¤adimos a la fila el äEi / äZ de la rama */
	 pt = ramas[rm].elem;
	 do
	 {
	  /* Tomamos un nombre interno de elemento de la rama */
	  obtener_nombre (pt, nombre);
	  pt += 4;

	  /* Si el elemento es activo.. */
	  if ((nombre[0] != 'R') && (nombre[0] != 'L') && (nombre[0] != 'C'))
	   {
	    /* ..a¤adimos su valor dividido por el äZ de la rama a 'fila' y
	       la col. que le corresponda */

	    /* Buscamos el elemento en la lista */
	    elem_bus = buscar_elem (n_elems, elems, nombre);

	    /* Obtenemos el valor del elemento */
	    val = elems[elem_bus].valor;

	    /* Si es independiente.. */
	    if (nombre[0] == 'V')
	     {
	      /* ..ponemos su valor con signo opuesto (pasamos de 2§ al 1§) */
	      for (j = 0; ((*G(i))[j].puntero != 0) &&
			  ((*G(i))[j].puntero != elem_bus + 1); ++j);

	      /* ¨Y qu‚ hacemos con la Yl? */
	      (*G(fila))[j].coef[0] -= val * Yr;
	      (*G(fila))[j].coef[1] -= val * Yc;
	      (*G(fila))[j].puntero = elem_bus + 1;
	     }

	    /* Pero, si es dependiente.. */
	    else
	     {
	      j = atoi ((char *) &nombre[1]);   /* Obtenemos su n§ de orden */

	      /* Si el gen. es del tipo 'A'.. */
	      if (nombre[0] == 'A')
	       {
		/* ..ponemos su valor en la col. 'orden + j - 1' */
		*RLC(l, fila, orden + j - 1) = val * Yc;
		*RLC(r, fila, orden + j - 1) = val * Yr;
		*RLC(c, fila, orden + j - 1) = val * Yl;
	       }

	      /* Sin embargo, si es del tipo 'B'.. */
	      else
	       {
		/* ..ponemos su valor en la col. 'orden + gendep - j' */
		*RLC(l, fila, orden + gendep - j) = val * Yc;
		*RLC(r, fila, orden + gendep - j) = val * Yr;
		*RLC(c, fila, orden + gendep - j) = val * Yl;
	       }

	      /* y un puntero all generador en 'fg' */
	      fg[j - orden] = elem_bus;
	     }
	   }
	 }
	 while (*pt != 0);
	}
       }
     }

    /* pero, si el gen. depende de una tensi¢n... */
    else
     {
      /* ...la ec. a construir es "Vdep = Vi - Vf" */

      /* poner un '-1' en 'fila' y la col. correspondiente a Vi */
      if ((ni = elems[e].caract.nodos[0]) > 0)
	*RLC(r, fila, ni - 1) = -1;

      /* poner un '1' en 'fila' y la col. correspondiente a Vf */
      if ((nf = elems[e].caract.nodos[1]) > 0)
	*RLC(r, fila, nf - 1) = 1;
     }
   }
 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int formar_sistema_EDO (unsigned orden)

/* Esta funci¢n pasa el sistema de ecs. del circuito, que contiene mezcla de
   ecs. integro-diferenciales con ecs. lineales ordinarias, en un sistema de
   Ecs. Diferenciales Ordinarias */
{
 int i, j, k;

 /* Recorremos todas las filas */
 for (i = 0; i < orden; ++i)
  {
   do
   {
    /* Recorremos los coefs. hasta encontrar uno <> 0 ¢ el £ltimo */
    for (j = 0; j < orden; ++j)
      if (*RLC(l, i, j) != 0)
	break;

    /* Si se lleg¢ al £ltimo coef.. */
    if (j >= orden)
     {
      /* ..entonces toda la fila es 0 y hay que derivarla */
      for (k = 0; k < orden; ++k)
       {
	*RLC(l, i, k) = *RLC(r, i, k);
	*RLC(r, i, k) = *RLC(c, i, k);
	*RLC(c, i, k) = 0;
       }

      for (k = 0; k < 10; ++k)
       {
	(*G(i))[k].coef[2] = (*G(i))[k].coef[1];
	(*G(i))[k].coef[1] = (*G(i))[k].coef[0];
	(*G(i))[k].coef[0] = 0;
       }
     }
   }
   while (j >= orden);
  }

 /* Despejamos las derivadas (cambiamos R y C de signo) */
 for (i = 0; i < orden; ++i)

   /* Cambiamos de signo la fila 'i' de R y C */
   for (j = 0; j < orden; ++j)
    {
     *RLC(r, i, j) = -*RLC(r, i, j);
     *RLC(c, i, j) = -*RLC(c, i, j);
    }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/