#include <windows.h>
#include <resolver.h>

extern HWND        hwnd;
extern HANDLE      hr, hw;
extern short       cyLine, nVposScrl, minimizada, Fin, Abort;
extern char        frase2[], frase3[], frase4[], frase5[];
extern unsigned    orden, gendep, gendepv, gendepi;
extern double huge *r,
	      huge *w;
extern long        mem_global;
extern unsigned    mem_local, *fg;
extern char        nom_fich[];
extern double      Tact;

extern int         diagonaliza_real (unsigned orden, unsigned max_orden);
extern double huge *C(unsigned i, unsigned j);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void punto_de_trabajo (unsigned n_elems, elemento *elems,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs_Z, char (*ecs_Z)[100],
		       unsigned n_ecs_Y, char (*ecs_Y)[100])

/* Funci¢n que calcula el punto de trabajo del circuito (las condiciones
   iniciales) y almacena los resultados pedidos en las respuestas en el
   fichero de extensi¢n '.PTR' */
{
 FILE      *pdetr;
 elemento  safe;
 resultado *resp = NIL;
 double    v = 0, *yZ = NIL, *yY = NIL;
 unsigned  n_resp, i, j, a, b, nv;
 char      nom[84], *pt;

 extern unsigned recupera_respuestas (char tipo, resultado **respuestas);
 extern unsigned recupera_cond_inic (char tipo,
				     unsigned no_vars, double **cond_inic);
 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);
 extern void temperatura (float T, unsigned n_elems, elemento *elems);

 int cond_inic_por_mallas (unsigned no_vars,
			   unsigned n_elems, elemento *elems,
			   unsigned n_ramas, rama *ramas,
			   unsigned n_ecs_Z, char (*ecs_Z)[100],
			   unsigned n_ecs_Y, char (*ecs_Y)[100]);
 int cond_inic_por_nudos  (unsigned no_vars,
			   unsigned n_elems, elemento *elems,
			   unsigned n_ramas, rama *ramas,
			   unsigned n_ecs_Z, char (*ecs_Z)[100],
			   unsigned n_ecs_Y, char (*ecs_Y)[100]);

 /* 'nv' significa 'n§ de variables' y su valor es 2*orden + gendep, si en
    el circuito hay bobinas, u orden + gendep en caso contrario, ya que si no
    hay bobinas, la matriz de C's ser  (0) y no habr  cambios de variable */

 gendep = 0;

 b = 1;

 for (a = 0; a < n_elems; ++a)
  {
   if (elems[a].tipo[0] == 'L')
     b = 2;
   else
     if (((elems[a].tipo[0] >= 'A') && (elems[a].tipo[0] <= 'E') &&
	  (elems[a].tipo[0] != 'C')) || (elems[a].tipo[0] == 'Q'))
       ++gendep;
  }

 /* Calculamos las condiciones iniciales a la Tnom */
 nv = b*n_ecs_Z + gendep;
 cond_inic_por_mallas (nv, n_elems, elems, n_ramas, ramas,
		       n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

 Fin = 0;

 nv = b*n_ecs_Y + gendep;
 cond_inic_por_nudos  (nv, n_elems, elems, n_ramas, ramas,
		       n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

 /* Si el c lculo finaliz¢ anormalmente, regresamos inmediatamente */
 if (Fin)
   return;

 /* Extraemos las respuestas de polarizaci¢n (tipo 'P') del fichero */
 n_resp = recupera_respuestas ('P', &resp);

 /* Si no queremos ninguna respuesta, simplemente regresamos */
 if (n_resp == 0)
   return;

 /* Recuperamos las condiciones iniciales */
 recupera_cond_inic ('Z', nv, &yZ);
 recupera_cond_inic ('Y', nv, &yY);

 /* Abrimos el fichero que contendr  los resultados */
 strcpy (nom, nom_fich);
 strcat (nom , ".PTR");
 pdetr = fopen (nom, "wb");

 /* Recorremos la tabla de respuestas */
 for (a = 0; a < n_resp; ++a)
  {
   /* Si hay cambios de temperatura, calculamos las condiciones iniciales */
   if (resp[a].temp != Tact)
    {
     /* Actualizamos la temperatura de an lisis */
     Tact = resp[a].temp;

     /* Modificamos los valores del circuito */
     temperatura (Tact, n_elems, elems);

     /* Calculamos las condiciones iniciales del circuito por mallas */
     nv = b*n_ecs_Z + gendep;
     cond_inic_por_mallas (nv, n_elems, elems, n_ramas, ramas,
			   n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);
     nv = b*n_ecs_Y + gendep;
     cond_inic_por_nudos  (nv, n_elems, elems, n_ramas, ramas,
			   n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

     /* Si el c lculo termin¢ anormalmente, regresamos inmediatamente */
     if (Fin)
       return;

     /* Las obtenemos en 'y' */
     recupera_cond_inic ('Z', nv, &yZ);
     recupera_cond_inic ('Y', nv, &yY);
    }

   /* El array de enteros apuntado 'comp' contiene los componentes del resul-
      tado, es decir, i, j, k,... donde Vm = Vi - Vj ¢ Im = Ii ñ Ij ñ Ik ...
      N¢tese que, al volver de 'cond_inic', las variables globales 'orden' y
      'gendep' contienen los valores correctos */

   /* Si la respuesta deseada es una V.. */
   if (resp[a].tipo == 'V')
    {
     /* ..obtenemos su valor */
     if (resp[a].medida.nodos[0] > 0)
       v = yY[resp[a].medida.nodos[0] - 1];
     else
       v = 0;

     if (resp[a].medida.nodos[1] > 0)
       v -= yY[resp[a].medida.nodos[1] - 1];
    }
   else
    {
     /* Obtenemos el valor de la corriente por el elemento */
     v = 0;          /* Inicializamos la var. que contendr  dicha corriente */

     /* Recorremos todas las mallas */
     for (i = 0; i < n_ecs_Z; ++i)

      /* Si el elemento de medida est  en alguna de ellas.. */
      if ((pt = strstr (ecs_Z[i], resp[a].medida.elem)) != NIL)
       {
	/* ..obtenemos el signo de la corriente por la rama */
	while (*(pt--) != '(');

	/* Incorporamos la corriente con su signo */
	v += (*pt == '-') ? -yZ[i] : yZ[i];
       }
    }

   /* Guardamos la respuesta en el fichero */
   fwrite (&v, sizeof (double), 1, pdetr);
  }

 /* Cerramos el fichero de respuestas */
 fclose (pdetr);

 /* Liberamos el espacio que ya no nos es £til */
 mem_local -= LocalSize (LocalHandle ((WORD) yZ)) +
	      LocalSize (LocalHandle ((WORD) yY));
 free (yZ);
 free (yY);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int cond_inic_por_mallas (unsigned no_vars,
			  unsigned n_elems, elemento *elems,
			  unsigned n_ramas, rama *ramas,
			  unsigned n_ecs_Z, char (*ecs_Z)[100],
			  unsigned n_ecs_Y, char (*ecs_Y)[100])

/* Esta funci¢n calcula las condiciones iniciales del circuito, almacenando
   el resultado en el fichero de extensi¢n ".INI".
   En la matriz del circuito, van primero las 'n_ecs_Z' ecuaciones de malla,
   a continuaci¢n las 'ramas_cond' ecuaciones de las ramas con condensadores
   y las zonas de carga aisladas, y despu‚s las 'gendep' ecuaciones de los
   generadores dependientes. */
{
 RECT        rect;
 FILE        *datos_inic;
 double huge *actual_r;
 double      *y, A, Z, *q;
 int 	     signo, elem_aux, elem_act, op, actual_orden;
 unsigned    a, b, i, j, k, ramas_cond = 0, nudos = 0, *nu;
 long 	     tamano;
 char 	     t, Zij[100], nom[84], nombre_elem[5], *punt, *aux;

 extern unsigned recupera_lista_ramas (rama **ramas);
 extern unsigned recupera_matriz      (char tipo, char (**ecs)[100]);
 extern int convertir (int sentido, unsigned n_elems, elemento *elems,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs_Z, char (*ecs_Z)[100],
		       unsigned n_ecs_Y, char (*ecs_Y)[100]);
 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern int buscar_rama (unsigned n_ramas, rama *ramas, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern char *obtener_comunes (char Hii[100], char Hjj[100], char Hij[100]);
 extern unsigned buscador (unsigned tipo_busqueda,
			   unsigned n_ramas, rama *ramas,
			   unsigned no_volver[], unsigned salida,
			   unsigned llegada, char elem[100]);

 void exp_var_condinic (unsigned n_elems, elemento *elems, unsigned e,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs_Z, char (*ecs)[100],
			unsigned fila, char t);
 int resolver_Newton (unsigned orden, double *y, elemento *elems);
 double capacidad_difusion (unsigned n_elems, elemento *elems, unsigned i);

/*
 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º                                                                          º
 º Comienzo de la rutina de an lisis por mallas de las condiciones inicialesº
 º                                                                          º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/
 /* Enviamos a la ventana lo que estamos haciendo */
 strcpy (frase2, "Respuesta: Punto de Trabajo\0");
 strcpy (frase3, "Puntos a calcular: \0");
 strcpy (frase4, "Puntos calculados: \0");
 strcpy (frase5, "0\0");
 InvalidateRect (hwnd, NULL, TRUE);

 /* Transformamos el circuito a la forma necesaria */
 convertir (A_MALLAS, n_elems, elems, n_ramas, ramas,
	    n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

 /* Anulamos los generadores de se¤al del circuito. Adem s, contamos el
    n§ de generadores dependientes */
 gendepv = gendepi = 0;

 for (i = 0; i < n_elems; ++i)
  {
   if ((elems[i].tipo[0] == 'V') || (elems[i].tipo[0] == 'I'))
    {
     if (elems[i].signal != 'C')
       elems[i].valor = 0;
    }
   else
     if ((elems[i].tipo[0] == 'A') || (elems[i].tipo[0] == 'B'))
       ++gendepv;
     else
       if ((elems[i].tipo[0] == 'D') || (elems[i].tipo[0] == 'E'))
	 ++gendepi;
  }

 gendep = gendepv + gendepi;     /* Calcula el n§ total de generadores dep. */

 /* Contamos el n§ de ramas con condensadores y el n§ de nudos */
 for (i = 0; i < n_ramas; ++i)
  {
   if (strchr (ramas[i].elem, 'C') != NIL)
     ++ramas_cond;
   if (ramas[i].nodoi > nudos)
     nudos = ramas[i].nodoi;
   else
     if (ramas[i].nodof > nudos)
       nudos = ramas[i].nodof;
  }

 /* El orden del sub-sistema de ecuaciones de malla ser  igual a n_ecs_Z,
    m s el n£mero de ramas con condensadores */
 orden = n_ecs_Z + ramas_cond;

 /* Creamos las matrices del circuito y la matriz de soluciones */
 tamano = (orden + gendep) * (orden + gendep + 1) * sizeof (double);
 hr = GlobalAlloc (GMEM_MOVEABLE, tamano);
 r = (double huge *) GlobalLock (hr);
 y = (double *) calloc (2*n_ecs_Z + gendep, sizeof (double));
 q = (double *) calloc (n_ecs_Z, (n_ecs_Z + 1) * sizeof (double));
 fg = (unsigned *) malloc (gendep * sizeof (unsigned));

 mem_global += GlobalSize (hr);
 mem_local  += LocalSize (LocalHandle ((WORD) q)) +
	       LocalSize (LocalHandle ((WORD) y)) +
	       LocalSize (LocalHandle ((WORD)fg));

 /* Las inicializamos a 0 */
 _fmemset (r, 0, tamano);
 memset (y, 0, 2*n_ecs_Z + gendep);
 memset (q, 0, n_ecs_Z * (n_ecs_Z + 1) * sizeof (double));

 /* El elemento de la fila i, col. j est  compuesto por los elementos que son
    comunes a la malla i y a la malla j */

 /* Con el puntero 'i' recorremos todos los elems. de la diagonal princ. */
 for (i = 0; i < n_ecs_Z; ++i)
  {
   /* Buscamos generadores de corriente en la malla 'i' */
   if ((punt = strchr (ecs_Z[i], 'D')) == NIL)
     if ((punt = strchr (ecs_Z[i], 'E')) == NIL)
       punt = strchr (ecs_Z[i], 'I');

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
     *C(i,i) = 1;

     /* Si el generador es independiente.. */
     if (*punt == 'I')

       /* ..constru¡mos la ec. "Ii = valor_gen" */
       *C(i, orden + gendep) = Z;

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
       if (*C(k, k) == 0)
	{
	 exp_var_condinic (n_elems, elems, elem_aux, n_ramas, ramas, n_ecs_Z,
			   ecs_Z, k, 'Z');

	 /* y un puntero al elemento en la matriz 'f' */
	 fg[k - orden] = elem_aux;
	}

       /* Finalmente a¤adimos el t‚rmino "-cte_gen x Idep" */
       *C(i,k) = -Z;
      }
    }

   /* Si no hay gens. de I en la malla, constru¡mos la fila normalmente */
   else

   /* Idem con 'columna', formando dos bucles anidados que construyen la
      matriz Z. El elemento que construimos en cada pasada es el (i,j) */
   for (j = 0; j < n_ecs_Z; ++j)
    {
     /* Obtenemos en Zij los nombres (internos) concatenados de todos los
	elementos comunes a las posiciones de la diagonal principal apuntados
	por 'i' y 'j' */

     if (i == j)
       strcpy (Zij, ecs_Z[i]);
     else
       obtener_comunes (ecs_Z[i], ecs_Z[j], Zij);

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
      A = elems[elem_act].valor;

      /* Si el elemento es pasivo... */
      if ((nombre_elem[0] == 'R') || (nombre_elem[0] == 'L') ||
	  (nombre_elem[0] == 'C'))
       {
	/* El signo s¢lo cuenta para los elems. que no est‚n en la diagonal */
	if (i != j)
	  A *= signo;

	/* A¤adimos el valor a la fila y columna correspondientes. N¢tese
	  que, si el elemento es una bobina, no se a¤ade nada */
	if (nombre_elem[0] == 'R')
	  *C(i, j) += A;
	else
	  if ((nombre_elem[0] == 'C') && (i == j))
	   {
	    *C(i, n_ecs_Z + atoi ((char *)&nombre_elem[1]) - 1) = signo / A;

	    /* Adem s, si es un condensador, a¤adimos su valor inicial al
	       t‚rmino independiente con el signo cambiado */
	    *C(i, orden + gendep) -= signo *
			elems[buscar_elem (n_elems, elems,
					   nombre_elem)].caract.cond_inic / A;
	   }
       }

      /* pero si es activo (un generador)... */
      else

       /* ..s¢lo se le procesar  una vez; cuando fila = col. */
       if (i == j)
	{
	 /* Le damos el signo que debe llevar, que es igual al de la malla
	    en la que se encuentra */
	 A *= signo;

	 /* De nuevo n¢tese que no pueden aparecer gen. de corriente en
	    la malla actual ya que, si los hay, la fila no se construye */

	 /* Entonces, si es de tensi¢n e independiente.. */
	 if (nombre_elem[0] == 'V')

	   /* ..a¤adimos su valor al t‚rmino independiente de la fila 'i',
	      cambiado de signo al cambiar de t‚rmino */
	   *C(i, orden + gendep) -= A;

	 /* pero si no es independiente.. */
	 else
	  {
	   /* ..leemos el n§ de orden del elemento */
	   k = atoi ((char *) &nombre_elem[1]);

	   /* ..si es de tipo 'A'.. */
	   if (nombre_elem[0] == 'A')

	     /* ..su cte. ir  en la posici¢n (i, orden + k - 1) */
	     k += orden - 1;

	   /* , pero si es de tipo 'B'.. */
	   else

	     /* ..entonces la cte. ir  en (i, orden + gendepv - k) */
	     k = orden + gendepv - k;

	   /* Ponemos en la col. correspondiente la cte. del generador */
	   *C(i, k) = A;

	   /* Y si es la 1¦ vez que se le procesa, en la misma fila ponemos
	      la expresi¢n de la variable de la que depende */
	   if (*C(k, k) == 0)
	     exp_var_condinic (n_elems, elems, elem_act, n_ramas, ramas,
			       n_ecs_Z, ecs_Z, k, 'Z');

	   /* y un puntero al elemento en la matriz 'f' */
	   fg[k - orden] = elem_act;
	  }
	}
     }
    }
  }

 /* Obtenemos las ecuaciones de las zonas de carga aisladas y de las corrien-
    tes por los condensadores del circuito. Estas £ltimas van a continuaci¢n
    de las ecs. de malla, es decir, las corrientes por C1 van en la fila
    'n_ecs_Z + 1', las corrientes por C2 van en 'n_ecs_Z + 2', etc. Si hay una
    zona de carga aislada relacionando varios condensadores, las ecs. que se
    obtienen al igualar a 0 las corrientes por las ramas que contienen a estos
    condensadores son dependientes, por lo que habremos de eliminar una de
    ellas, que es sustitu¡da por la ec. de la zona de carga aislada. */

 /* Creamos la matriz de nudos usados para 'buscador' */
 nu = (unsigned *) malloc (nudos * sizeof (unsigned));
 mem_local += LocalSize (LocalHandle ((WORD)nu));

 /* Vamos por todas las ramas buscando condensadores */
 for (i = 0; i < n_ramas; ++i)
  {
   if ((strchr (ramas[i].elem, 'C') != NIL) && (ramas[i].marca == 0))
    {
     /* Marcamos como usadas las ramas con condensadores */
     for (j = 0; j < n_ramas; ++j)
      {
       if (strchr (ramas[j].elem, 'C') != NIL)
	 ramas[j].usada = 1;
       else
	 ramas[j].usada = 0;
      }

     /* Inicializamos Zij que contendr  las ramas que bloquean el camino */
     memset (Zij, 0, sizeof (Zij));

     /* Inicializamos nu que contendr  los nodos usados, para evitar lazos */
     memset (nu, 0, nudos * sizeof (unsigned));
     nu[0] = ramas[i].nodof;
     nu[1] = FIN_NU;

     /* Si no se puede formar camino de armadura a armadura.. */
     if (buscador (1, n_ramas, ramas, nu,
		   ramas[i].nodof, ramas[i].nodoi, Zij) == 0)
      {
       /* ..constru¡mos la zona de carga aislada. Los condensadores que
	  formen parte de esta no se considerar n ya puesto que, aunque uno
	  de ellos forme parte de dos zonas distintas, al probar con uno de
	  los que formen parte de la otra zona ya se encontrar  este. Para
	  ello, se ponen las 'marca' de las ramas que los contienen a 1 */
       punt = Zij;

       /* 'b' dice en qu‚ fila ir  la ec. de la zona de carga aislada, que
	  ser  la que corresponder¡a al condensador del cual se ignora la
	  expresi¢n de las corrientes que lo atraviesan (el £ltimo) */
       aux = punt + strlen (Zij);
       while (*(--aux) != 'C');
       obtener_nombre (aux, nombre_elem);
       b = n_ecs_Z + atoi ((char *)&nombre_elem[1]) - 1;

       /* Este bucle recorre el string devuelto por 'buscador' en modo 1 */
       do
       {
	/* Localizamos el siguiente condensador */
	while (*(++punt) != 'C');

	/* Localizamos el signo que lleva */
	aux = punt - 1;
	while (*(aux--) != '(');
	if (*aux == '-')
	  signo = -1;
	else
	  signo = 1;

	/* Extraemos su nombre del string */
	obtener_nombre (punt, nombre_elem);

	/* Obtenemos su n§ de orden */
	k = atoi ((char *) &nombre_elem[1]);

	/* Ponemos ñ1 en la fila de la zona y la col. corresponde a este C */
	*C(b, n_ecs_Z + k - 1) = signo;

	/* A¤adimos su carga inicial al t‚rmino independiente, con su signo */
	*C(b, orden + gendep) += signo * elems[buscar_elem (n_elems, elems,
					       nombre_elem)].caract.cond_inic;

	/* Localizamos la rama en la que se encuentra el condensador */
	a = buscar_rama (n_ramas, ramas, nombre_elem);

	/* Ponemos su 'marca' a 1 para que no se la considere m s */
	ramas[a].marca = 1;

	/* Obtenemos la ec. "Ii ñ Ij ñ ... ñ Ik = 0", en 'r' y en 'q', a no
	   ser que se trate del £ltimo condensador de la zona. La ec. de este
	   condensador se ignora, ya que es dependiente de las anteriores */
	if (strchr (punt + 1, 'C') != NIL)
	  for (j = 0; j < n_ecs_Z; ++j)

	    /* Si la rama con el condensador se encuentra en la malla 'j'.. */
	    if ((aux = strstr (ecs_Z[j], ramas[a].elem)) != NIL)
	     {
	      /* ..retrocedemos hasta la posici¢n del signo (si lo hay) */
	      aux -= 2;

	      /* A¤adimos la corriente a la ecuaci¢n con su signo */
	      if (*aux == '-')
	       *C(n_ecs_Z + k - 1, j) =
	       *(q + (n_ecs_Z+1) * (k-1) + j) = -1;
	      else
	       *C(n_ecs_Z + k - 1, j) =
	       *(q + (n_ecs_Z+1) * (k-1) + j) = 1;
	     }

	/* Apuntamos al car cter siguiente al condensador localizado */
	punt += 4;
       }
       while (*(punt + 1) != 0);
      }

     /* Pero, si existe camino entre las armaduras .. */
     else
      {
       /* ..obtenemos la ec. "Ii ñ Ij ñ ... ñ Ik = 0" */

       /* Obtenemos el nombre interno del condensador */
       obtener_nombre (strchr (ramas[i].elem, 'C'), nombre_elem);

       /* Obtenemos su n§ de orden */
       b = atoi ((char *)&nombre_elem[1]);

       for (j = 0; j < n_ecs_Z; ++j)
	{
	 /* Si la rama con el condensador se encuentra en la malla 'j'.. */
	 if ((aux = strstr (ecs_Z[j], ramas[i].elem)) != NIL)
	  {
	   /* ..retrocedemos hasta la posici¢n del signo (si lo hay) */
	   aux -= 2;

	   /* A¤adimos la corriente con su signo a la fila correspondiente */
	   if (*aux == '-')
	     *C(n_ecs_Z + b - 1, j) =
	     *(q + (n_ecs_Z + 1)*(b - 1) + j) = -1;
	   else
	     *C(n_ecs_Z + b - 1, j) =
	     *(q + (n_ecs_Z + 1)*(b - 1) + j) = 1;
	  }
	}
      }
    }
  }

 /* Tomamos una precauci¢n especial contra las posibles filas de 0's */
 for (i = 0; i < orden; ++i)
  {
   /* Localizamos el 1er coef. <> 0 en la fila 'i' */
   k = orden;
   for (j = 0; j < orden; ++j)
     if (*C(i,j) != 0)
      {
       k = j;
       j = orden;
      }

   /* Si no hab¡a ninguno, asumimos la ecuaci¢n "xi = 0" */
   if (k >= orden)
    {
     *C(i,i) = 1;
     *C(i,orden) = 0;
    }
  }

 /* El £nico punto donde puede interrumpirse el proceso es en 'resolver..' */
 Fin = Abort = 0;

 /* Resolvemos el sistema */
 if (resolver_Newton (orden + gendep, y, elems) != 0)
  {
   MessageBox (hwnd, "Iteración no convergente\ncalculando condiciones iniciales por mallas\nDando Fin al Cálculo",
		     "Resolutor S.C.A.",
		     MB_ICONSTOP | MB_OK);
   Fin = 1;
  }

 /* Si hemos escogido Finalizar ¢ Abortar, el programa deber  finalizar, ya
    que no puede hacerse ning£n otro an lisis sin el del punto de trabajo */
 if (Fin)
  {
   Abort = 1;

   mem_global -= GlobalSize (hr);
   mem_local  -= LocalSize (LocalHandle ((WORD) q)) +
		 LocalSize (LocalHandle ((WORD) y)) +
		 LocalSize (LocalHandle ((WORD)fg)) +
		 LocalSize (LocalHandle ((WORD)nu));

   GlobalFree (hr);
   free (q);
   free (y);
   free (fg);
   free (nu);

   convertir (A_INICIAL, n_elems, elems, n_ramas, ramas, n_ecs_Z, ecs_Z,
			 n_ecs_Y, ecs_Y);
   return (0);
  }

 /* Abrimos el fichero de elementos para devolverles los valores iniciales */
 strcpy (nom, nom_fich);
 strcat (nom, ".ELM");
 datos_inic = fopen (nom, "rb");

 /* Ponemos en C1, C2 etc. los valores iniciales (q1, q2,..) y en los gen.
    dep. los valores (Vdep1, Vdep2,..) al tiempo que restauramos los elems. */
 for (i = 0; i <n_elems; ++i)
  {
   /* Obtenemos el n§ de orden y el tipo del elemento */
   j = atoi (&elems[i].tipo[1]);
   t = elems[i].tipo[0];

   /* Preservamos su valor, que debido a la T§ puede no coincidir con el */
   Z = elems[i].valor;   		     /* almacenado en el fichero */

   /* Obtenemos el valor de su condici¢n inicial */
   switch (elems[i].tipo[0])
   {
    case 'C':
    {
     A = y[n_ecs_Z + j - 1];
     break;
    }

    case 'A':
    {
     A = y[orden + j - 1];
     break;
    }

    case 'B':
    {
     A = y[orden + gendepv - j];
     break;
    }

    case 'D':
    {
     A = y[orden + gendepv + j - 1];
     break;
    }

    case 'E':
    {
     A = y[orden + gendep - j];
     break;
    }

    case 'L':
    {
     /* La condici¢n inicial de la bobina es la corriente por la misma */
     A = 0;
     for (k = 0; k < n_ecs_Z; ++k)
       if (strstr (ecs_Z[k], elems[i].tipo) != NIL)
	 A += y[k];
     break;
    }
   }

   /* Restauramos el elemento al original almacenado en el fichero */
   fread (elems + i, sizeof (elemento), 1, datos_inic);

   /* Le ponemos el valor verdadero, excepto si es un generador de se¤al, en
      cuyo caso hay que restaurar su valor */
   if ((t != 'V') && (t != 'I'))
     elems[i].valor = Z;

   /* Le ponemos la condici¢n inicial */
   if ((t != 'R') && (t != 'V') && (t != 'I'))
     if ((t == 'C') || (t == 'L'))
       elems[i].caract.cond_inic = A;
     else
       elems[i].CT = A;
  }

 /* Calculamos los valores reales de las capacidades de difusi¢n */
 for (i = 0; i < n_elems; ++i)

   /* Si el elemento es de la forma "CcNNN" ¢ "CdNNN" ¢ "CeNNN".. */
   if ((elems[i].nombre[0] == 'C') && (elems[i].nombre[1] >= 'c') &&
       (elems[i].nombre[1] <= 'e'))
    {
     /* ..calculamos el valor de su capacidad real, salvando la antigua */
     A = elems[i].valor;
     elems[i].valor = capacidad_difusion (n_elems, elems, i);

     /* Calculamos la nueva carga inicial */
     elems[i].caract.cond_inic *= elems[i].valor / A;
    }

 /* Cerramos el fichero de elementos */
 fclose (datos_inic);

 /* Reordenamos las variables de modo que 'y' quede de la forma siguiente:
     - 'n_ecs_Z' variables correspondientes a i1, i2, i3...
     - 'gendep' variables correspondientes a las variables de los gens. dep.
     - 'n_ecs_Z' variables correspondientes a la integrales de i1, i2, i3..
    N¢tese que no importa perder las q1,q2 etc. porque ya est n en los datos
    de cada condensador */

 for (i = 0; i < gendep; ++i)
   y[n_ecs_Z + i] = y[orden + i];

 for (i = n_ecs_Z + gendep; i < 2*n_ecs_Z + gendep; ++i)
   y[i] = 0;

 /* Si necesitamos los valores iniciales de las integrales de i1,i2,.. */
 if (no_vars > n_ecs_Z + gendep)
 {
  /* Ahora tenemos las cargas en cada condensador, pero necesitamos encontrar
     cu nta carga se debe a cada una de las corrientes que lo atraviesan, es
     decir, los valores iniciales de las integrales de i1,i2,i3,...,in. Para
     ello integraremos las ecs. de la matriz Q */
  actual_r = r;
  r = (double huge *) q;
  actual_orden = orden;
  orden = n_ecs_Z - gendep;

  /* Ponemos la carga de cada condensador en el t‚rmino independiente, salt n-
     donos aquellas filas que sean todo 0's, es decir, las filas de los con-
     densadores ignorados a causa de que hay zona de carga aislada */
  for (i = n_ecs_Z; i < actual_orden - gendep; ++i)
   {
    for (j = 0; (j < n_ecs_Z) && (*C(i - n_ecs_Z, j) == 0); ++j);
    if (j < n_ecs_Z)
     {
      for (j = 0; (elems[j].tipo[0] != 'C') ||
		  (atoi (&elems[j].tipo[1]) != i - n_ecs_Z + 1); ++j);
      *C(i - n_ecs_Z, n_ecs_Z) = elems[j].caract.cond_inic;
     }
   }

  /* Localizamos las intersecciones de filas y columnas de 0's (habr  tantas
     como zonas de carga aisladas), para ponerlas a 1. Las filas de 0's son
     producto de las zonas de carga aisladas, y las columnas de 0's lo son de
     las corrientes que no pasan por ning£n condensador */
  for (j = 0; j < n_ecs_Z; ++j)
   {
    /* Buscamos t‚rminos != 0 en la columna 'j' */
    for (i = 0; (i < n_ecs_Z) && (*C(i,j) == 0); ++i);

    /* Si no hay ninguno (la columna es 0).. */
    if (i >= n_ecs_Z)

      /* Buscamos la 1¦ fila que haya con todo 0's */
      for (a = 0; a < n_ecs_Z; ++a)
       {
	for (b = 0; (b < n_ecs_Z) && (*C(a,b) == 0); ++b);
	if (b >= n_ecs_Z)
	 {
	  /* Hacemos la intersecci¢n de la fila y la columna igual a 1 */
	  *C(a,j) = 1;
	  break;
	 }
       }
   }

  /* Resolvemos el sistema obteniendo las integrales de i1,i2,i3.. etc. */
  diagonaliza_real (orden + gendep, orden + gendep);

  /* Ponemos los valores en las £ltimas 'n_ecs_Z' soluciones */
  for (i = 0; i < n_ecs_Z; ++i)
    y[n_ecs_Z + gendep + i] = *C(i, orden + gendep) / *C(i,i);

  /* Restauramos los valores anteriores para 'r' y 'orden' */
  r = actual_r;
  orden = actual_orden;
 }

 /* Guardamos los resultados en el fichero */
 strcpy (nom, nom_fich);
 strcat (nom, ".CIZ");
 datos_inic = fopen (nom, "wb");
 fwrite (y, sizeof (double), 2*n_ecs_Z + gendep, datos_inic);
 fclose (datos_inic);

 /* Liberamos el espacio utilizado */
 mem_global -= GlobalSize (hr);
 mem_local  -= LocalSize (LocalHandle ((WORD) q)) +
	       LocalSize (LocalHandle ((WORD) y)) +
	       LocalSize (LocalHandle ((WORD)fg)) +
	       LocalSize (LocalHandle ((WORD)nu));

 GlobalFree (hr);
 free (q);
 free (y);
 free (fg);
 free (nu);

 /* Restauramos las listas de ramas y ecuaciones a las originales */
 recupera_lista_ramas (&ramas);
 recupera_matriz ('Z', &ecs_Z);
 recupera_matriz ('Y', &ecs_Y);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int cond_inic_por_nudos (unsigned no_vars,
			 unsigned n_elems, elemento *elems,
			 unsigned n_ramas, rama *ramas,
			 unsigned n_ecs_Z, char (*ecs_Z)[100],
			 unsigned n_ecs_Y, char (*ecs_Y)[100])

/* Esta funci¢n calcula las condiciones iniciales del circuito, almacenando
   el resultado en el fichero de extensi¢n ".INI".
   En la matriz del circuito, van primero las 'n_ecs_Y' ecuaciones de nudo,
   a continuaci¢n las 'ramas_bob' ecuaciones de las ramas con inductancias
   y las zonas de corriente aisladas, y despu‚s las 'gendep' ecuaciones de
   los generadores dependientes. */
{
 RECT 	     rect;
 FILE 	     *datos_inic;
 double      *y, A, Z;
 int 	     signo, elem_aux, elem_act, rama_act, op, actual_orden;
 unsigned    a, b, i, j, k, ramas_bob = 0;
 long 	     tamano;
 char 	     t, Yij[100], nom[84], nombre_elem[5], *punt, *aux;

 extern unsigned recupera_lista_ramas (rama **ramas);
 extern unsigned recupera_matriz      (char tipo, char (**ecs)[100]);
 extern int convertir (int sentido, unsigned n_elems, elemento *elems,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs_Z, char (*ecs_Z)[100],
		       unsigned n_ecs_Y, char (*ecs_Y)[100]);
 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern int buscar_rama (unsigned n_ramas, rama *ramas, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern char *obtener_comunes (char Hii[100], char Hjj[100], char Hij[100]);

 void exp_var_condinic (unsigned n_elems, elemento *elems, unsigned e,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs_Z, char (*ecs)[100],
			unsigned fila, char t);
 double sumR (unsigned n_elems, elemento *elems, rama *ramas, unsigned r);
 int resolver_Newton (unsigned orden, double *y, elemento *elems);
 double capacidad_difusion (unsigned n_elems, elemento *elems, unsigned i);

/*
 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º                                                                          º
 º Comienzo de la rutina de an lisis por nudos de las condiciones iniciales º
 º                                                                          º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/
 /* Enviamos a la ventana lo que estamos haciendo */
 strcpy (frase2, "Respuesta: Punto de Trabajo\0");
 strcpy (frase3, "Puntos a calcular: \0");
 strcpy (frase4, "Puntos calculados: \0");
 strcpy (frase5, "0\0");
 InvalidateRect (hwnd, NULL, TRUE);

 /* Transformamos el circuito a la forma necesaria */
 convertir (A_NUDOS, n_elems, elems, n_ramas, ramas,
	    n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

 /* Anulamos los generadores de se¤al del circuito. Adem s, contamos el
    n§ de generadores dependientes */
 gendepv = gendepi = 0;

 for (i = 0; i < n_elems; ++i)
  {
   if ((elems[i].tipo[0] == 'V') || (elems[i].tipo[0] == 'I'))
    {
     if (elems[i].signal != 'C')
       elems[i].valor = 0;
    }
   else
     if ((elems[i].tipo[0] == 'A') || (elems[i].tipo[0] == 'B'))
       ++gendepv;
     else
       if ((elems[i].tipo[0] == 'D') || (elems[i].tipo[0] == 'E'))
	 ++gendepi;
  }

 gendep = gendepv + gendepi;     /* Calcula el n§ total de generadores dep. */

 /* Contamos el n§ de ramas con inductancias */
 for (i = 0; i < n_ramas; ++i)
   if (strchr (ramas[i].elem, 'L') != NIL)
     ++ramas_bob;

 /* El orden del sub-sistema de ecuaciones de nudo ser  igual a n_ecs_Y,
    m s el n£mero de ramas con inductancias */
 orden = n_ecs_Y + ramas_bob;

 /* Creamos las matrices del circuito y la matriz de soluciones */
 tamano = (orden + gendep) * (orden + gendep + 1) * sizeof (double);
 hr = GlobalAlloc (GMEM_MOVEABLE, tamano);
 r = (double huge *) GlobalLock (hr);
 y = (double *) calloc (2*n_ecs_Y + gendep, sizeof (double));
 fg = (unsigned *) malloc (gendep * sizeof (unsigned));

 mem_global += GlobalSize (hr);
 mem_local  += LocalSize (LocalHandle ((WORD) y)) +
	       LocalSize (LocalHandle ((WORD)fg));

 /* Las inicializamos a 0 */
 _fmemset (r, 0, tamano);
 memset (y, 0, 2*n_ecs_Y + gendep);

 /* El elemento de la fila i, col. j est  compuesto por los elementos que son
    comunes al nudo i y al nudo j */

 /* Con el puntero 'i' recorremos todos los elems. de la diagonal princ. */
 for (i = 0; i < n_ecs_Y; ++i)
  {
   /* Buscamos generadores de V en el nudo 'i' */
   if ((punt = strchr (ecs_Y[i], 'A')) == NIL)
     if ((punt = strchr (ecs_Y[i], 'B')) == NIL)
       punt = strchr (ecs_Y[i], 'V');

   /* Si hab¡a alguno.. */
   if (punt != NIL)
    {
     /* ..nos situamos en el primer car cter de la rama */
     aux = punt;
     while (*(aux - 1) != '(')
       --aux;

     /* Recorremos la rama hasta encontrar alguna R ¢ el fin de la rama */
     while ((*aux != 'R') && (*aux != ')'))
       aux += 4;
    }

   /* Si hab¡a algun generador sin resistencia asociada.. */
   if ((punt != NIL) && (*aux == ')'))
    {
     /* ..nos movemos al comienzo de la rama, despu‚s del '(' */
     while (*(punt - 1) != '(')
       --punt;

     /* A¤adimos al sistema todos los generadores que haya en la rama */
     do
     {
      /* Primero obtenemos su nombre (interno) en 'nombre_elem' */
      obtener_nombre (punt, nombre_elem);

      /* Si es un generador.. */
      if ((*punt == 'V') || (*punt == 'A') || (*punt == 'B'))
       {
	/* ..lo buscamos y obtenemos su posici¢n en 'elem_aux' */
	elem_aux = buscar_elem (n_elems, elems, nombre_elem);

	/* Finalmente obtenemos su cte., con signo (+) */
	Z = elems[elem_aux].valor;

	/* Tanto como si el gen. es dep. ¢ indep. pondremos Vi a 1 y Vj a -1 */
	*C(i,i) = 1;
	rama_act = buscar_rama (n_ramas, ramas, elems[elem_aux].tipo);

	if ((ramas[rama_act].nodoi > 0) && (ramas[rama_act].nodof > 0))
	 {
	  if (ramas[rama_act].nodoi == (i + 1))
	    *C(i, ramas[rama_act].nodof - 1) = -1;
	  else
	   if (ramas[rama_act].nodof == (i + 1))
	     *C(i, ramas[rama_act].nodoi - 1) = -1;
	 }

	/* Si el generador es independiente.. */
	if (*punt == 'V')
	 {
	  /* ..completamos la ec. "Vi - Vj = ñvalor_gen" */
	  /* Si el nudo 'i' es el inicial (y por tanto el +).. */
	  if (ramas[rama_act].nodoi == (i + 1))

	    /* ..a¤adimos el t‚rmino "+valor_gen" al t‚rmino independiente */
	    *C(i, orden + gendep) += Z;

	  /* pero, si no es el inicial (y por tanto tampoco el +).. */
	  else

	    /* ..a¤adimos el t‚rmino "-valor_gen" al t‚rmino independiente */
	    *C(i, orden + gendep) -= Z;
	 }

	/* pero, si el generador es dependiente.. */
	else
	 {
	  /* ..completamos la ec. "Vi = ñcte_gen x f(Vdep) + Vj" */
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
	    *C(i, k) = Z;

	  /* pero, si es el inicial (y por tanto el +).. */
	  else

	    /* ..a¤adimos el t‚rmino "-cte_gen" a la columna correspondiente */
	    *C(i, k) = -Z;

	  /* Y, si es la primera vez que se le procesa, ponemos su expresi¢n
	     en la fila 'k' */
	  if (*C(k,k) == 0)
	   {
	    exp_var_condinic (n_elems, elems, elem_aux, n_ramas, ramas,
			      n_ecs_Y, ecs_Y, k, 'Y');

	    /* y un puntero al elemento en 'fg' */
	    fg[k - orden] = elem_aux;
	   }
	 }
       }

      /* Avanzamos hasta el siguiente nombre (¢ el ')') */
      punt += 4;
     }
     while (*punt != ')');

     /* Generamos las expresiones de las variables de las que dependen los
	generadores dependientes que est‚n conectados al nudo */
     punt = ecs_Y[i];
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
      if ((*C(k,k) == 0) && (k != 0))
       {
	exp_var_condinic (n_elems, elems, elem_aux, n_ramas, ramas,
			  n_ecs_Y, ecs_Y, k, 'Y');

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
   for (j = 0; j < n_ecs_Y; ++j)
   {
    /* Obtenemos en Yij los nombres (internos) concatenados de todos los
       elementos comunes a las posiciones de la diagonal principal apuntados
       por 'i' y 'j' */

    if (i == j)
      strcpy (Yij, ecs_Y[i]);
    else
      obtener_comunes (ecs_Y[i], ecs_Y[j], Yij);

    a = 0;                             /* Apuntamos a la 1¦ posici¢n de Yij */
    Z = 0;				      /* Ponemos la suma de Z's a 0 */

    while (Yij[a] != 0)          /* Repetir hasta que no haya m s elementos */
     {
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

      /* Si el elemento es pasivo... */
      if ((nombre_elem[0] == 'R') || (nombre_elem[0] == 'L') ||
	  (nombre_elem[0] == 'C'))
       {
	/* A¤adimos el valor a la suma de Z's de la rama. N¢tese que, si el
	   elemento es un condensador, no se a¤ade nada */
	if (nombre_elem[0] == 'R')
	  Z += A;
	else
	  if ((nombre_elem[0] == 'L') && (i == j))
	   {
	    /* Si es una bobina, a¤adimos su valor inicial al
	       t‚rmino independiente con el signo cambiado */
	    rama_act = buscar_rama (n_ramas, ramas, nombre_elem);
	    if (ramas[rama_act].nodoi == (i + 1))
	     {
	      *C(i, n_ecs_Y + atoi ((char *)&nombre_elem[1]) - 1) = 1;
	      *C(i, orden + gendep) -= elems[elem_act].caract.cond_inic;
	     }
	    else
	     {
	      *C(i, n_ecs_Y + atoi ((char *)&nombre_elem[1]) - 1) = -1;
	      *C(i, orden + gendep) += elems[elem_act].caract.cond_inic;
	     }
	   }
       }

      /* pero si es activo (un generador)... */
      else

       /* ..s¢lo se le procesar  una vez; cuando fila = col... */
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
						   (nombre_elem[0] == 'B'))))
	    A = -A;

	 /* Entonces, si es de corriente e independiente.. */
	 if (nombre_elem[0] == 'I')

	   /* ..a¤adimos su valor al t‚rmino independiente de la fila 'i',
	      cambiado de signo al cambiar de t‚rmino */
	   *C(i, orden + gendep) -= A;

	 /* pero si no es de corriente e independiente.. */
	 else

	  /* ..entonces, si es de tensi¢n e independiente.. */
	  if (nombre_elem[0] == 'V')

	    /* A¤adimos su valor, dividido por la suma de Z's de la rama
	       y cambiado de signo al pasar del 1er t‚rmino al 2§, al t‚rmino
	       independiente de la fila 'i' */
	    *C(i, orden + gendep) -= A / sumR (n_elems, elems, ramas, rama_act);

	  /* pero si no es independiente (por tanto es dependiente).. */
	  else
	   {
	    /* ..si es de tensi¢n, el valor a a¤adir ser  cte_gen / äZ */
	    if (nombre_elem[0] < 'C')
	      A /= sumR (n_elems, elems, ramas, rama_act);

	    /* Leemos el n§ de orden del generador */
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

	    /* Ponemos en la col. correspondiente la cte. del generador */
	    *C(i, k) = A;

	    /* Y si es la 1¦ vez que se le procesa, en la misma fila ponemos
	       la expresi¢n de la variable de la que depende */
	    if (*C(k,k) == 0)
	     {
	      exp_var_condinic (n_elems, elems, elem_act, n_ramas, ramas,
				n_ecs_Y, ecs_Y, k, 'Y');

	      /* y un puntero al elemento en la matriz 'f' */
	      fg[k - orden] = elem_act;
	     }
	   }
	}

      if (Yij[a] == ')')
       {
	if (Z != 0)
	 {
	  /* La a¤adimos a la fila y col. correspondientes */
	  if (i == j)
	    *C(i,j) += 1 / Z;
	  else                                      /* Nos saltamos el ')', */
	    *C(i,j) -= 1 / Z;		            /* si  lo  hay, y  como */
	 }                                          /* pasamos a  una nueva */
	Z = 0;                                      /* rama,  a¤adimos   la */
	++a;                                        /* suma de Z's actual a */
       }                                            /* la pos. (i,j) e ini- */
     }                                              /* -cializamos la  suma */
   }                                                /* de Z's  a 0.         */
  }

 /* Constru¡mos las ecs. de las tensiones en las bobinas */

 /* Vamos por todas las ramas buscando bobinas */
 for (i = 0; i < n_ramas; ++i)
   if ((punt = strchr (ramas[i].elem, 'L')) != NIL)
    {
     /* Obtenemos el nombre de la bobina */
     obtener_nombre (punt, nombre_elem);

     /* Obtenemos su n£mero de orden */
     k = atoi (&nombre_elem[1]) - 1;

     /* Constru¡mos la ec. en la fila 'n_ecs_Y + k' de 'r' */
     *C(n_ecs_Y + k, ramas[i].nodoi - 1) =  1;
     *C(n_ecs_Y + k, ramas[i].nodof - 1) = -1;
    }

 /* El £nico punto donde puede interrumpirse el proceso es en 'resolver..' */
 Fin = Abort = 0;

 /* Resolvemos el sistema */
 if (resolver_Newton (orden + gendep, y, elems) != 0)
  {
   MessageBox (hwnd, "Iteración no convergente\ncalculando condiciones iniciales por nudos\nDando Fin al Cálculo",
		     "Error de Cálculo",
		     MB_ICONSTOP | MB_OK);
   Fin = 1;
  }

 /* Si hemos escogido Finalizar ¢ Abortar, el programa deber  finalizar, ya
    que no puede hacerse ning£n otro an lisis sin el del punto de trabajo */
 if (Fin)
  {
   Abort = 1;

   mem_global -= GlobalSize (hr);
   mem_local  -= LocalSize (LocalHandle ((WORD) y)) +
		 LocalSize (LocalHandle ((WORD)fg));

   GlobalFree (hr);
   free (y);
   free (fg);

   convertir (A_INICIAL, n_elems, elems, n_ramas, ramas, n_ecs_Z, ecs_Z,
			 n_ecs_Y, ecs_Y);
   return (0);
  }

 /* Abrimos el fichero de elementos para devolverles los valores iniciales */
 strcpy (nom, nom_fich);
 strcat (nom, ".ELM");
 datos_inic = fopen (nom, "rb");

 /* Ponemos en L1, L2 etc. los valores iniciales (iL1, iL2,..) y en los gen.
    dep. los valores Vdep1, Vdep2,.. al tiempo que restauramos los elems. */
 for (i = 0; i <n_elems; ++i)
  {
   /* Obtenemos el n§ de orden y el tipo del elemento */
   j = atoi (&elems[i].tipo[1]);
   t = elems[i].tipo[0];

   /* Preservamos su valor, que debido a la T§ puede no coincidir con el */
   Z = elems[i].valor;   		     /* almacenado en el fichero */

   /* Obtenemos el valor de su condici¢n inicial */
   switch (elems[i].tipo[0])
   {
    case 'L':
    {
     A = y[n_ecs_Y + j - 1];
     break;
    }

    case 'A':
    {
     A = y[orden + j - 1];
     break;
    }

    case 'B':
    {
     A = y[orden + gendepv - j];
     break;
    }

    case 'D':
    {
     A = y[orden + gendepv + j - 1];
     break;
    }

    case 'E':
    {
     A = y[orden + gendep - j];
     break;
    }

    case 'C':
    {
     /* Buscamos la rama en la que se encuentra el condensador */
     rama_act = buscar_rama (n_ramas, ramas, elems[i].tipo);

     /* Obtenemos la tensi¢n en sus bornas */
     A = 0;
     if (ramas[rama_act].nodoi > 0)
       A += y[ramas[rama_act].nodoi - 1];
     if (ramas[rama_act].nodof > 0)
       A -= y[ramas[rama_act].nodof - 1];

     /* La condici¢n inicial es la carga q = CúV */
     A *= elems[i].valor;

     break;
    }
   }

   /* Restauramos el elemento al original almacenado en el fichero */
   fread (elems + i, sizeof (elemento), 1, datos_inic);

   /* Le ponemos el valor verdadero, excepto si es un generador de se¤al, en
      cuyo caso hay que restaurar su valor */
   if ((t != 'V') && (t != 'I'))
     elems[i].valor = Z;

   /* Le ponemos la condici¢n inicial */
   if ((t != 'R') && (t != 'V') && (t != 'I'))
     if ((t == 'C') || (t == 'L'))
       elems[i].caract.cond_inic = A;
     else
       elems[i].CT = A;
  }

 /* Calculamos los valores reales de las capacidades de difusi¢n */
 for (i = 0; i < n_elems; ++i)

   /* Si el elemento es de la forma "CcNNN" ¢ "CdNNN" ¢ "CeNNN".. */
   if ((elems[i].nombre[0] == 'C') && (elems[i].nombre[1] >= 'c') &&
       (elems[i].nombre[1] <= 'e'))
    {
     /* ..calculamos el valor de su capacidad real, salvando la antigua */
     A = elems[i].valor;
     elems[i].valor = capacidad_difusion (n_elems, elems, i);

     /* Calculamos la nueva carga inicial */
     elems[i].caract.cond_inic *= elems[i].valor / A;
    }

 /* Cerramos el fichero de elementos */
 fclose (datos_inic);

 /* Reordenamos las variables de modo que 'y' quede de la forma siguiente:
     - 'n_ecs_Y' variables correspondientes a v1, v2, v3...
     - 'gendep' variables correspondientes a las variables de los gens. dep.
     - 'n_ecs_Y' variables correspondientes a la integrales de v1, v2, v3..
    N¢tese que no importa perder las i1,i2 etc. porque ya est n en los datos
    de cada bobina */

 for (i = 0; i < gendep; ++i)
   y[n_ecs_Y + i] = y[orden + i];

 for (i = n_ecs_Y + gendep; i < 2*n_ecs_Y + gendep; ++i)
   y[i] = 0;

 /* Guardamos los resultados en el fichero */
 strcpy (nom, nom_fich);
 strcat (nom, ".CIY");
 datos_inic = fopen (nom, "wb");
 fwrite (y, sizeof (double), 2*n_ecs_Y + gendep, datos_inic);
 fclose (datos_inic);

 /* Liberamos el espacio utilizado */
 mem_global -= GlobalSize (hr);
 mem_local  -= LocalSize (LocalHandle ((WORD) y)) +
	       LocalSize (LocalHandle ((WORD)fg));

 GlobalFree (hr);
 free (y);
 free (fg);

 /* Restauramos las listas de ramas y ecuaciones a las originales */
 recupera_lista_ramas (&ramas);
 recupera_matriz ('Z', &ecs_Z);
 recupera_matriz ('Y', &ecs_Y);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double capacidad_difusion (unsigned n_elems, elemento *elems, unsigned i)

/* Calcula el valor real de la capacidad de difusi¢n que est  en la posici¢n
   'i' de la lista de elementos. Previamente deber  encontrarse en dicha
    lista el valor de la tensi¢n inicial de cada uno de los diodos. */
{
#define Vd elems[j].CT
#define TT elems[j].fase
#define VJ elems[j].f.coef[1]
#define FC elems[j].f.coef[4]
#define MJ elems[j].f.coef[6]

 double v = elems[i].valor;
 int j;
 char nombre_elem[6];

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);

 /* Si el condensador pertenece a un diodo.. */
 if (elems[i].nombre[1] == 'd')
  {
   /* ..obtenemos el "QNNN" (nombre interno del diodo al que pertenece) */
   strcpy (nombre_elem, "Q\x0");
   strcat (nombre_elem, &elems[i].nombre[2]);

   /* Buscamos este diodo */
   j = buscar_elem (n_elems, elems, nombre_elem);
  }

 /* pero, si pertenece a una uni¢n de un transistor.. */
 else
  {
   /* ..obtenemos el "DfNNN" ¢ "DrNNN" (nombre del diodo al que pertenece) */
   if (elems[i].nombre[1] == 'e')
     strcpy (nombre_elem, "Df\x0");
   else
     strcpy (nombre_elem, "Dr\x0");
   strcat (nombre_elem, &elems[i].nombre[2]);

   /* Buscamos el diodo de dicha uni¢n */
   for (j = 0; (j < n_elems) && (strcmp (elems[j].nombre, nombre_elem) != 0);
	++j);
  }

 /* Si el diodo al que pertenece el condensador se ha encontrado.. */
 if ((j >= 0) && (j < n_elems))
  {
   /* ..calculamos la capacidad de la uni¢n seg£n el valor de Vd */
   v *= (Vd > VJ*FC) ? (1 - (1 + MJ)*FC + MJ*Vd/VJ) / pow (1 - FC, 1 + MJ) :
		       1 / pow (1 - Vd / VJ, MJ);

   /* Le a¤adimos la capacidad de transici¢n TTúId' */
   v += TT * valor_gen (j, Vd, 0, 0, 0, 0, 1, elems);
  }

 return (v);

#undef Vd
#undef TT
#undef VJ
#undef FC
#undef MJ
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void exp_var_condinic (unsigned n_elems, elemento *elems, unsigned e,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs, char (*ecs)[100],
		       unsigned fila, char t)

/* Esta funci¢n pone en la fila 'fila' la expresi¢n de la variable de la que
   depende el generador apuntado por 'elem', es decir;
   Si el gen. depende de una 'Vdep':
	      Vdep = (R1+R2+..+Rm)ú(Ii+Ij+..) + qi/Ci + qj/Cj + ..
   Si el gen. depende de una 'Idep', o es un diodo:
	      Idep = ñ Ii ñ Ij ñ ... 					   */
{
 double Z, Y, val;
 char *pt, *aux, nombre[5], cam[100];
 int op;
 unsigned ni, nf, b, i, j, elem_bus, r;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern int buscar_rama (unsigned n_ramas, rama *ramas, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern int camino (unsigned n_ramas, rama *ramas,
		    unsigned salida, unsigned llegada, char elem[100]);

 double sumR (unsigned n_elems, elemento *elems, rama *ramas, unsigned r);

 /* Ponemos un 1 en 'fila' y la col. correspondiente a este gen., de modo que
    empezamos la ec. "Vdep = ..." (Vdep = V de la que dep. el gen.)
	      o bien "Idep = ..." (Idep = I "   "  "   "   "   "  ) */

 /* Comenzamos la ec. "Vdep  = ..." */
 *C(fila, fila) = 1;

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
	 while (*(pt--) != '(');     /* Retrocedemos a la posici¢n del signo */

	 /* A¤adimos el valor correspondiente pero con el signo cambiado ya
	    que pasamos del 2§ t‚rmino al 1§ */
	 *C(fila, i) = (*pt == '-') ? 1 : -1;
	}
      }
    }

   /* Pero si el gen. depende de una tensi¢n... */
   else
    {
     /* ..buscamos un camino que una los nodos de medida de la misma y que
	no contenga gen. de corriente */
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
      Z = 0;
      do
      {
       /* Tomamos un nombre interno de elemento de la rama */
       obtener_nombre (pt, nombre);

       /* Apuntamos al siguiente nombre */
       pt += 4;

       /* Obtenemos el valor del elemento cuyo nombre hemos tomado */
       elem_bus = buscar_elem (n_elems, elems, nombre);
       val = elems[elem_bus].valor;

       /* Si el elemento es una resistencia.. */
       if (nombre[0] == 'R')

	 /* ..a¤adimos su R a la suma de Z's */
	 Z += val;

       /* pero, si es un condensador, entonces Vdep = q / C */
       else
	 if (nombre[0] == 'C')
	  {
	   /* Ponemos 1/C en la columna correspondiente a este condensador
	      y cambiado de signo al pasar del 2§ t‚rmino al 1§ */
	   j = atoi ((char *) &nombre[1]);
	   *C(fila, n_ecs + j - 1) = -op / val;

	   /* Ponemos el valor inicial de este C en el t‚rmino indep. */
	   *C(fila, orden + gendep) += op *
				       elems[elem_bus].caract.cond_inic / val;
	  }

	 /* pero si es un generador.. */
	 else

	   /* ..entonces, si es independiente.. */
	   if (nombre[0] == 'V')

	     /* ..ponemos su valor en el t‚rmino indep. con el signo normal */
	     *C(fila, orden + gendep) += op * val;

	   /* Pero, si es dependiente.. */
	   else
	    {
	     j = atoi ((char *) &nombre[1]);    /* Obtenemos su n§ de orden */

	     /* Si el gen. es del tipo 'A'.. */
	     if (nombre[0] == 'A')

	       /* ..ponemos su valor en la col. 'orden + j - 1' */
	       *C(fila, orden + j - 1) = -op * val;

	     /* Sin embargo, si es del tipo 'B'.. */
	     else

	       /* ..ponemos su valor en la col. 'orden + gendepv - j' */
	       *C(fila, orden + gendepv - j) = -op * val;
	    }
      }
      while (*pt != ')');

      /* Ponemos la Z de esta rama en todos los t‚rminos de las corrientes
	 que la atraviesan. N¢tese que en 'nombre' est  el del £ltimo ele-
	 mento de la rama. */
      if (Z != 0)
       {
	for (i = 0; i < n_ecs; ++i)

	  /* Si la rama est  en la malla 'i'.. */
	  if ((aux = strstr (ecs[i], nombre)) != NIL)
	   {
	    /* ..retrocedemos hasta la posici¢n del signo */
	    while (*(aux--) != '(');

	    /* Si la I de la malla tiene el mismo sentido que el de recorrido
	       del camino (op), la V en la rama ser  tomada positiva, y al pa-
	       sar del 2§ t‚rmino al 1§ se pondr  negativa */
	    *C(fila, i) += ((*aux == '-') ? SUMA : RESTA) * op * Z;
	   }
       }
     }
     while (*(++pt) != 0);
    }
  }

 /* pero, si queremos la expresi¢n en Y... */
 else
  {
   /* Si el generador depende de una corriente... */
   if ((elems[e].tipo[0] == 'B') || (elems[e].tipo[0] == 'E'))
    {
     /* ...la ec. a construir es "Idep = (Vi - Vf - äEi) / äZ" */

     /* Buscamos en qu‚ rama est  el elem. del que depende el gen. */
     r = buscar_rama (n_ramas, ramas, elems[e].caract.elem);

     /* Si en dicha rama hay a su vez un gen. de corriente independiente.. */
     if ((pt = strstr (ramas[r].elem, "I")) != NIL)
      {
       /* ..la ec. ser  "Idep = Igen" */
       obtener_nombre (pt, nombre);
       pt += 4;
       *C(fila, orden+gendep) = elems[buscar_elem (n_elems,
						   elems, nombre)].valor;
      }

     /* pero, si hay un gen. de corriente dependiente.. */
     else
      {
       if ((pt = strstr (ramas[r].elem, "D")) == NIL)
	 pt = strstr (ramas[r].elem, "E");

       if (pt != NIL)
	{
	 /* ..la ec. ser  "Idep = cte_gen x f(Idep')" */

	 /* Tomamos el nombre interno del generador */
	 obtener_nombre (pt, nombre);

	 /* Obtenemos su n§ de orden */
	 j = atoi ((char *) &nombre[1]);

	 /* Calculamos su col. asignada seg£n su tipo */
	 if (nombre[0] == 'D')
	   j += orden + gendepv - 1;
	 else
	   j = orden + gendep - j;

	 /* Ponemos en dicha col. el valor 'cte_gen', cambiado de signo */
	 *C(fila, j) -= elems[buscar_elem (n_elems, elems, nombre)].valor;
	}

       /* y si, finalmente, no hay ning£n gen. de corriente en la rama.. */
       else
       {
	/* Calculamos la suma de las Z's de la rama */
	Z = sumR (n_elems, elems, ramas, r);
	if (Z != 0)
	  Y = 1 / Z;

	/* Ponemos -1/Z en 'fila' y la col. correspondiente a Vi */
	if ((ni = ramas[r].nodoi) > 0)
	  *C(fila, ni - 1) = -Y;

	/* Ponemos un '1' en 'fila' y la col. correspondiente a Vf */
	if ((nf = ramas[r].nodof) > 0)
	  *C(fila, nf - 1) = Y;

	/* A¤adimos a la fila el äEi / äZ de la rama */
	pt = ramas[r].elem;
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

	   /* Obtenemos su valor, dividido por el äZ de la rama */
	   val = elems[buscar_elem(n_elems, elems, nombre)].valor /
		 sumR (n_elems, elems, ramas, r);

	   /* Si es independiente.. */
	   if (nombre[0] == 'V')

	     /* ..ponemos su valor con signo opuesto (pasamos de 2§ al 1§) */
	     *C(fila, orden + gendep) = -val;

	   /* Pero, si es dependiente.. */
	   else
	    {
	     j = atoi ((char *) &nombre[1]);   /* Obtenemos su n§ de orden */

	     /* Si el gen. es del tipo 'A'.. */
	     if (nombre[0] == 'A')

	       /* ..ponemos su valor en la col. 'orden + j - 1' */
	       *C(fila, orden + j - 1) = val;

	     /* Sin embargo, si es del tipo 'B'.. */
	     else

	       /* ..ponemos su valor en la col. 'orden + gendep - j' */
	       *C(fila, orden + gendep - j) = val;
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
       *C(fila, ni - 1) = -1;

     /* poner un '1' en 'fila' y la col. correspondiente a Vf */
     if ((nf = elems[e].caract.nodos[1]) > 0)
       *C(fila, nf - 1) = 1;
    }
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int resolver_Newton (unsigned orden, double *y, elemento *elems)

/* Esta funci¢n resuelve el sistema de ecuaciones apuntado por 'r', utilizan-
   do el m‚todo de Newton. Todas las ecs. son lineales excepto en los t‚rmi-
   nos correspondientes a los generadores dep. no lineales */
{
 MSG         msg;
 RECT        rect;
 int         i, j, error, k = 0, num_iters = 0;
 long        tamano = (orden + 1) * orden * sizeof (double);
 double huge *aux;
 double      coef, h = 1, v;
 char        num[4];

 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);
 extern double huge *J (unsigned i, unsigned j);

 /* Enviamos a la ventana el n§ m ximo de iteraciones */
 strcpy (frase3, "Puntos a calcular: ");
 strcat (frase3, itoa (NUMITERS, num, 10));

 GetClientRect (hwnd, &rect);
 rect.top    = 5 * cyLine - nVposScrl * SCRLDESPL;
 rect.bottom = rect.top + cyLine - 1;
 InvalidateRect (hwnd, &rect, FALSE);

 /* Creamos el espacio para calcular el jacobiano de 'r' */
 hw = GlobalAlloc (GMEM_MOVEABLE, tamano);
 w = (double huge *) GlobalLock (hw);

 mem_global += GlobalSize (hw);

 /* Aplicamos el m‚todo de Newton */
 do
 {
  /* Inicializamos todas las variables a 0 */
  for (i = 0; i < orden; ++i)
    y[i] = 0;

  /* Comenzamos la iteraci¢n */
  num_iters = 0;
  do
  {
   /* Enviamos a la ventana el n§ de iteraciones y el % completado */
   sprintf (frase4, "Puntos calculados: %03u", num_iters);
   sprintf (frase5, "%3u", num_iters / 2);

   if (minimizada)
     InvalidateRect (hwnd, NULL, FALSE);
   else
    {
     GetClientRect (hwnd, &rect);
     rect.top    = 7 * cyLine - nVposScrl * SCRLDESPL;
     rect.bottom = rect.top + 4 * cyLine - 1;
     InvalidateRect (hwnd, &rect, FALSE);
    }

   /* Desbloqueamos la memoria antes de procesar mensajes */
   GlobalUnlock (hw);

   /* Recogemos y procesamos los siguientes mensajes en la cola */
   while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
    {
     TranslateMessage (&msg);
     DispatchMessage  (&msg);
    }

   /* Rebloqueamos la memoria */
   w = (double huge *) GlobalLock (hw);

   if (!Fin)
    {
     /* 'error' marca si superamos el error establecido (0.001) */
     error = 0;

     /* Obtenemos la copia de la matriz apuntada por 'r' en 'c' */
     _fmemcpy (w, r, tamano);

     /* Calculamos los t‚rminos no lineales del Jacobiano de 'r'. Estos ser n
	todos aquellos en las £ltimas 'gendep' columnas, excepto los de la
	diagonal principal, ya que las £ltimas 'gendep' ecs. son "Vdep = ... " */
     for (i = 0; i < orden; ++i)
       for (j = orden - gendep; j < orden; ++j)
	{
	 v = *J(i, j);
	 if ((v != 0) && (i != j))
	   *J(i, j) = v * valor_gen (fg[j - (orden - gendep)], y[j], 0,
				     0, 0, 0, 1, elems);
	}

     /* Calculamos los t‚rminos "-f (i     )" para el m‚todo de Newton
				   i  i,k-1                            */
     for (i = 0; i < orden; ++i)
      {
       /* Primero a¤adimos los t‚rminos lineales */
       for (j = 0; j < orden - gendep; ++j)
	 *J(i, orden) -= *C(i,j) * y[j];

       /* Despu‚s los no lineales */
       for (j = orden - gendep; j < orden; ++j)
	 if (*C(i, j) != 0)
	   if (i != j)
	     *J(i, orden) -= *C(i, j) * valor_gen (fg[j - (orden-gendep)], y[j],
						   0, 0, 0, 0, 0, elems);
	   else
	     *J(i, orden) -= *C(i, j) * y[j];

       /* No a¤adimos los t‚rminos independientes porque ya est n incluidos
	  con el signo correcto al copiar la matriz 'r' a la 'c' */
      }

     /* Resolvemos el sistema en 'c' para obtener las hi */
     aux = r;
     r = w;
     diagonaliza_real (orden, orden);
     r = aux;

     /* Calculamos el siguiente valor de todas las variables */
     for (i = 0; i < orden; ++i)
      {
       /* Calculamos el nuevo valor de la variable n§ 'i' */
       v = y[i] + *J(i, orden) / *J(i, i) * h;

       /* Comprobamos que no rebasemos los 2v. en un generador tipo 'Shockley' */
       if (i >= orden - gendep)
	 if ((v > 2) && (elems[fg[i - (orden - gendep)]].signal == 'S'))
	   v = 2;

       /* Lo comparamos con el anterior para obtener el error */
       if (fabs (v) > I_MIN)
	{
	 if (fabs (v) >= INFINITO)
	   num_iters = NUMITERS;
	 else
	   if (fabs ((v - y[i]) / v) > 0.001)
	     error = -1;
	}
       else
	 if (fabs (y[i]) > I_MIN)
	   error = -1;

       y[i] = v;
      }
    }
  }
  while ((error) && (++num_iters < NUMITERS) && (!Fin));

  if (error)
   {
    h /= 2;
//  printf ("\nIteraci¢n para condiciones iniciales no converge. Reduzco");
   }
 }
 while ((error) && (++k < 10) && (!Fin));

 mem_global -= GlobalSize (hw);
 GlobalFree (hw);

 return (error);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double sumR (unsigned n_elems, elemento *elems, rama *ramas, unsigned r)

/* Funci¢n que devuelve la suma de los valores de las impedancias en 'r' */
{
 double Z;
 char *pt;
 char nombre[5];
 unsigned i;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);

 Z = 0;                           	    /* Inicializamos la suma de Z's */

 pt = ramas[r].elem;             /* Apuntamos al 1er car cter de los elems. */

 do                                                           /* Repetir... */
  {
   /* Tomamos un nombre interno de elemento de la rama */
   obtener_nombre (pt, nombre);
   pt += 4;

   /* Si el elemento es una resistencia, a¤adimos su valor a la suma de Z's */
   if (nombre[0] == 'R')
     Z += elems[buscar_elem (n_elems, elems, nombre)].valor;
  }
 while (*pt != 0);                    /* ...hasta que no haya m s elementos */

 return (Z);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
