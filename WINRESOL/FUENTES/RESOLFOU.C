#include <resolver.h>

#ifndef UNO_DEFINIDO

complejo UNO = {1, 0};          /* Define el complejo 1 + 0j utilizado para */
#define UNO_DEFINIDO            /* inversiones complejas (1/z)              */

#endif

extern HANDLE        hp;
extern complejo huge *p;
extern unsigned      orden, gendep, gendepv, gendepi;
extern char          nom_fich[];
extern double        Tact;

extern int 	     diagonaliza_comp (unsigned orden);
extern complejo huge *M (unsigned i, unsigned j);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo del c¢digoÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void sensibilidad (float tpc, unsigned n_puntos,
		   char elem[5], resultado resp,
		   unsigned n_elems, elemento *elems,
		   unsigned n_ramas, rama *ramas,
		   unsigned n_ecs, char (*ecs)[100])

/* Esta funci¢n realiza el an lisis de sensibilidad de la respuesta 'resp'
   respecto del valor del elemento cuyo nombre viene en 'elem' */
{
 FILE *sens;
 complejo r;
 float valor, v_ini, v_fin, inc, w;
 unsigned num_comp, i, j, n;
 int *comp;
 char *pt, nom[84];

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);

 int Fourier_por_mallas (unsigned n_elems, elemento *elems,
			 unsigned n_ramas, rama *ramas,
			 unsigned n_ecs, char (*ecs)[100], float w);
 int Fourier_por_nudos (unsigned n_elems, elemento *elems,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs, char (*ecs)[100], float w);

 /* Abrimos el fichero que contendr  el resultado */
 strcpy (nom, nom_fich);
 strcat (nom, ".SNS");
 sens = fopen (nom, "wb");

 /* Localizamos el valor del elem. respecto del que realizamos el an lisis */
 n = buscar_elem (n_elems, elems, elem);
 valor = elems[n].valor;

 /* Calculamos los valores inicial y final del intervalo */
 v_ini = (1 - tpc) * valor;
 v_fin = (1 + tpc) * valor;
 inc = 2 * tpc * valor / n_puntos;

 /* Guardamos los datos de los rangos etc. */
 fwrite (&v_ini, sizeof (float), 1, sens);
 fwrite (&v_fin, sizeof (float), 1, sens);
 fwrite (&n_puntos, sizeof (unsigned), 1, sens);

 /* El array de enteros apuntado 'comp' contiene los componentes del resul-
    tado, es decir, i, j, k,... donde Vm = Vi - Vj ¢ Im = Ii ñ Ij ñ Ik ... */
 if (resp.tipo == 'V')
  {
   /* Si alguno de los nodos de medida es el de referencia.. */
   if ((resp.medida.nodos[0] == 0) || (resp.medida.nodos[1] == 0))

     /* ..la respuesta constar  s¢lo de una componente */
     num_comp = 1;

   /* Pero, si ninguno de los nodos de medida es el 0.. */
   else

     /* ..la respuesta constar  de dos componentes */
     num_comp = 2;

   /* Reservamos el espacio necesario para la tabla de componentes */
   comp = (int *) malloc (num_comp * sizeof (int));

   /* Colocamos los n£meros correspondientes a las componentes en la tabla */
   if (resp.medida.nodos[0] != 0)
     comp[0] = resp.medida.nodos[0];
   else
     comp[0] = -resp.medida.nodos[1];

   if (num_comp > 1)
     comp[1] = -resp.medida.nodos[1];
  }
 else
  {
   /* Averiguamos el n§ de mallas en que se encuentra el elemento */
   num_comp = 0;
   for (i = 0; i < n_ecs; ++i)
    {
     if (strstr (ecs[i], resp.medida.elem) != NIL)
       ++num_comp;
    }

   /* Reservamos el espacio necesario */
   comp = (int *) malloc (num_comp * sizeof (int));

   /* Obtenemos las mallas en las que se encuentra el elemento */
   j = 0;       /* 'j' apunta la posici¢n actual en la lista de componentes */

   /* Recorremos todas las mallas */
   for (i = 0; i < n_ecs; ++i);

    /* Si el elemento de medida est  en alguna de ellas.. */
    if ((pt = strstr (ecs[i], resp.medida.elem)) != NIL)
     {
      /* ..obtenemos el signo de la corriente por la rama */
      while (*(pt--) != '(');

      /* Incorporamos la componente con su signo */
      if (*pt == '-')
	comp[j] = -i - 1;      /* Es necesario a¤adir 1 al n§ de malla para */
      else                     /* homogeneizar este formato con el usado en */
	comp[j] = i + 1;       /* una tensi¢n entre nudos.                  */

      ++j;            /* Apuntamos a la posici¢n de la siguiente componente */
     }
  }

 /* Recorremos todo el intervalo de valores construyendo un sistema de ecs.
    diferente para cada valor y calculando el resultado */
 for (j = 0, valor = v_ini; j <= n_puntos; ++j, valor += inc)
  {
   /* Modificamos el valor del elemento */
   elems[n].valor = valor;

   /* Resolvemos por el m‚todo que nos convenga */
   if (resp.tipo == 'V')
     Fourier_por_nudos (n_elems, elems, n_ramas, ramas, n_ecs, ecs, w);
   else
     Fourier_por_mallas (n_elems, elems, n_ramas, ramas, n_ecs, ecs, w);

   /* Diagonalizamos la matriz */
   diagonaliza_comp (orden + gendep);

   /* Resolvemos para las variables necesarias */
   r.Re = r.Im = 0;
   for (i = 0; i < num_comp; ++i)
     if (comp[i] > 0)
       r = sum (r, cdiv (*M(comp[i]-1, orden + gendep),
			 *M(comp[i]-1, comp[i]-1)));
     else
       r = res (r, cdiv (*M(-comp[i]-1, orden + gendep),
			 *M(-comp[i]-1, -comp[i]-1)));

   /* La guardamos en el fichero */
   fwrite (&r, sizeof (complejo), 1, sens);
  }

 /* Finalmente, devolvemos el elemento a su valor inicial */
 elems[n].valor = (v_ini + v_fin) / 2;

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Fourier_por_mallas (unsigned n_elems, elemento *elems,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs, char (*ecs)[100], float w)

/* Esta funci¢n construye la matriz en r‚g. permanente del circuito por el
   m‚todo de las mallas, a partir de la posici¢n apuntada por 'p'. */
{
 /* Las variables 'orden'
		  'gendep', 'gendepv' y 'gendepi' son globales */

 complejo A, Z;
 int signo, elem_aux, elem_act, op;
 unsigned a, b, i, j, k;
 long tamano;
 char Zij[100], nombre_elem[5], *punt;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern char *obtener_comunes (char Hii[100], char Hjj[100], char Hij[100]);

 void exp_var_Fourier (unsigned n_elems, elemento *elems, unsigned e,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs, char (*ecs)[100],
		       unsigned fila, char t, float w);
 complejo valor (elemento *elems, unsigned e, float w);

/*
 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º                                                                          º
 º   Comienzo de la rutina de an lisis por mallas en el dominio de Fourier  º
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

 /* Creamos la matriz de transformadas */
 tamano = (orden + gendep) * (orden + gendep + 1) * sizeof (complejo);
 hp = GlobalLock (GMEM_MOVEABLE, tamano);
 p = (complejo huge *) GlobalLock (hp);

 /* Inicializamos la matriz a todo ceros */
 _fmemset (p, 0, tamano);

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

     /* Finalmente obtenemos su cte. transformada, con signo (+) */
     Z = valor (elems, elem_aux, w);

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
     M(i,i)->Re = 1;

     /* Si el generador es independiente.. */
     if (*punt == 'I')

	/* ..constru¡mos la ec. "Ii = valor_gen" */
	*M(i, orden + gendep) = Z;

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
       if ((M(k,k)->Re == 0) && (M(k,k)->Im == 0))
	 exp_var_Fourier (n_elems, elems, elem_aux, n_ramas, ramas, n_ecs,
			  ecs, k, 'Z', w);

       /* Finalmente a¤adimos el t‚rmino "-cte_gen x Idep" */
       M(i,k)->Re = -Z.Re;
       M(i,k)->Im = -Z.Im;
      }
    }

   /* Si no hay gens. de I en la malla, constru¡mos la fila normalmente */
   else
   /* Idem con 'columna', formando dos bucles anidados que construyen la
      matriz Z. El elemento que construimos en cada pasada es el (i,j).
      N¢tese que nunca podr n aparecer generadores de corriente en las
      primeras 'orden' mallas ya que el compilador se las arregla para
      poner las mallas que contienen generadores de corriente en las £ltimas
      'gendepi' posiciones del archivo de ramas. */

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

      /* Obtenemos el valor transformado del elemento */
      A = valor (elems, elem_act, w);

      /* Si el elemento es pasivo... */
      if ((nombre_elem[0] == 'R') || (nombre_elem[0] == 'L') ||
	  (nombre_elem[0] == 'C'))
       {
	/* El signo s¢lo cuenta para los elems. que no est‚n en la diagonal */
	if (i != j)
	 {
	  A.Re *= signo;
	  A.Im *= signo;
	 }

	/* A¤adimos el valor a la fila y columna correspondientes */
	*M(i,j) = sum (*M(i,j), A);
       }

      /* pero si es activo (generador)... */
      else

       /* ..s¢lo se le procesar  una vez; cuando fila = col. */
       if (i == j)
	{
	 /* Le damos el signo que debe llevar; este es igual al de la rama
	    de la malla en la que se encuentra. */
	 A.Re *= signo;
	 A.Im *= signo;

	 /* De nuevo n¢tese que no pueden aparecer gen. de corriente en
	    la malla actual ya que, si los hay, la fila no se construye */

	 /* Entonces, si es de tensi¢n e independiente.. */
	 if (nombre_elem[0] == 'V')

	   /* ..a¤adimos su valor al t‚rmino independiente de la fila 'i',
	      cambiado de signo al cambiar de t‚rmino */
	   *M(i, orden+gendep) = res (*M(i, orden+gendep), A);

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

	   /* Ponemos en la col. correspondiente la cte. del generador */
	   *M(i, k) = A;

	   /* Y si es la 1¦ vez que se le procesa, en la misma fila ponemos
	      la expresi¢n de la variable de la que depende */
	   if ((M(k,k)->Re == 0) && (M(k,k)->Im == 0))
	     exp_var_Fourier (n_elems, elems, elem_act, n_ramas, ramas, n_ecs,
			      ecs, k, 'Z', w);
	  }
	}
     }
    }
  }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Fourier_por_nudos (unsigned n_elems, elemento *elems,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs, char (*ecs)[100], float w)


/* Esta funci¢n construye la matriz en r‚g. permanente del circuito por el
   m‚todo de los nudos, a partir de la posici¢n apuntada por 't'. */
{
 /* Las variables 'orden'
		  'gendep', 'gendepv', 'gendepi' son globales */

 complejo A, Z;
 int elem_act, elem_aux, rama_act, op;
 unsigned a, b, i, j, k, ni, nf, gen_v;
 long tamano;
 char Yij[100], nombre_elem[5], *punt, *aux;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern int buscar_rama (unsigned n_ramas, rama *ramas, char tipo[5]);
 extern char *obtener_comunes (char Hii[100], char Hjj[100], char Hij[100]);
 extern char *obtener_nombre (char *punt, char nombre[5]);

 void exp_var_Fourier (unsigned n_elems, elemento *elems, unsigned e,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs, char (*ecs)[100],
		       unsigned fila, char t, float w);
 complejo valor (elemento *elems, unsigned e, float w);
 complejo sumZ (unsigned n_elems, elemento *elems, rama *ramas, unsigned r,
		float w);

/*
 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º                                                                          º
 º   Comienzo de la rutina de an lisis por nudos en el dominio de Fourier   º
 º                                                                          º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/
 gendepv = gendepi = 0;

 for (i = 0; i < n_elems; ++i)
  {                                                   /* Localiza todos los */
   if ((elems[i].tipo[0] == 'A') ||                   /* generadores dep.,  */
       (elems[i].tipo[0] == 'B'))                     /* tanto de V como de */
     ++gendepv;                                       /* I, que contiene el */
   else                                               /* circuito.          */
     if ((elems[i].tipo[0] == 'D') ||
	 (elems[i].tipo[0] == 'E'))
       ++gendepi;
  }

 gendep = gendepv + gendepi;     /* Calcula el n§ total de generadores dep. */

 /* El orden del sistema ser  el n§ de nudos del circuito menos uno (n_ecs) */
 orden = n_ecs;

 /* Creamos la matriz de transformadas */
 tamano =  (orden + gendep) * (orden + gendep + 1) * sizeof (complejo);
 hp = GlobalAlloc (GMEM_MOVEABLE, tamano);
 p = (complejo huge *) GlobalLock (hp);

 /* Inicializamos la matriz a todo ceros */
 _fmemset (p, 0, tamano);

 /* El elemento de la fila i, columna j est  compuesto por los elementos que
    son comunes al nodo i y al nodo j */

 /* Con el puntero 'i' recorremos todos los elems. de la diagonal princ. */
 for (i = 0; i < orden; ++i)
  {
   /* Buscamos generadores de V en el nudo 'i' */
   if ((punt = strstr (ecs[i], "A")) == NIL)
     if ((punt = strstr (ecs[i], "B")) == NIL)
       punt = strstr (ecs[i], "V");

   /* Si hab¡a alguno.. */
   if (punt != NIL)
    {
     /* Nos situamos en el primer car cter de la rama */
     aux = punt;
     while (*(aux - 1) != '(')
       --aux;

     /* Recorremos la rama hasta encontrar alguna Z ¢ el fin de la rama */
     while ((*aux != 'R') && (*aux != 'L') && (*aux != 'C') &&
	    (*aux != ')'))
       aux += 4;
    }

   /* Si hab¡a algun generador sin Z asociada.. */
   if ((punt != NIL) && (*aux == ')'))
    {
     /* ..nos situamos en el primer car cter de la rama */
     while (*(punt - 1) != '(')
       --punt;

     /* A¤adimos al sistema todos los generadores que haya en la rama */
     do
     {
      /* Primero obtenemos su nombre (interno) en 'nombre_elem' */
      obtener_nombre (punt, nombre_elem);

      /* Despu‚s lo buscamos y obtenemos su posici¢n en 'elem_aux' */
      elem_aux = buscar_elem (n_elems, elems, nombre_elem);

      /* Finalmente obtenemos su cte. transformada, con signo (+) */
      Z = valor (elems, elem_aux, w);

      /* Tanto como si el gen. es dep. ¢ indep. pondremos Vi a 1 y Vj a -1 */
      M(i,i)->Re = 1;
      rama_act = buscar_rama (n_ramas, ramas, elems[elem_aux].tipo);

      if ((ramas[rama_act].nodoi > 0) && (ramas[rama_act].nodof > 0))
       {
	if (ramas[rama_act].nodoi == (i + 1))
	  M(i, ramas[rama_act].nodof - 1)->Re = -1;
	else
	 if (ramas[rama_act].nodof == (i + 1))
	   M(i, ramas[rama_act].nodoi - 1)->Re = -1;
       }

      /* Si el generador es independiente.. */
      if (*punt == 'V')
       {
	/* ..completamos la ec. "Vi - Vj = ñvalor_gen" */
	/* Si el nudo 'i' es el inicial (y por tanto el +).. */
	if (ramas[rama_act].nodoi == (i + 1))

	  /* ..a¤adimos el t‚rmino "+valor_gen" al t‚rmino independiente */
	  *M(i, orden + gendep) = Z;

	/* pero, si no es el inicial (y por tanto tampoco el +).. */
	else

	  /* ..a¤adimos el t‚rmino "-valor_gen" al t‚rmino independiente */
	  *M(i,orden + gendep) = res (*M(i,orden + gendep), Z);
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
	  *M(i, k) = Z;

	/* pero, si es el inicial (y por tanto el +).. */
	else

	  /* ..a¤adimos el t‚rmino "-cte_gen" a la columna correspondiente */
	  *M(i, k) = res (*M(i, k), Z);

	/* Y, si es la primera vez que se le procesa, ponemos su expresi¢n en
	   la fila 'k' */
	if ((M(k,k)->Re == 0) && (M(k,k)->Im == 0))
	  exp_var_Fourier (n_elems, elems, elem_act, n_ramas, ramas, n_ecs,
			   ecs, k, 'Y', w);

       }

      /* Avanzamos hasta el siguiente nombre (¢ el ')') */
      punt += 4;
     }
     while (*punt != ')');
    }

   /* Si no hay generadores de este tipo, construimos la fila normalmente */
   else

   /* Idem con 'columna', formando dos bucles anidados que construyen la
      matriz Y. El elemento que construimos en cada pasada es el (i,j) */
   for (j = 0; j < orden; ++j)
   {
    /* Obtenemos en Yij los nombres (internos) concatenados de todos los
       elementos comunes a las posiciones de la diagonal principal apuntados
       por 'i' y 'j' */

    if (i == j)
      strcpy (Yij, ecs[i]);
    else
      obtener_comunes (ecs[i], ecs[j], Yij);

    a = 0;                             /* Apuntamos a la 1¦ posici¢n de Yij */
    memset (&Z, 0, sizeof (func_trans));            /* Ponemos el äZ a cero */

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

      /* Obtenemos el valor transformado del elemento */
      A = valor (elems, elem_act, w);

      /* Si el elemento es pasivo... */
      if ((nombre_elem[0] == 'R') || (nombre_elem[0] == 'L') ||
	  (nombre_elem[0] == 'C'))

	/* ..a¤adimos su Z a la suma de Z's de la rama actual */
	Z = sum (Z, A);

      /* pero si es activo (generador)... */
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
						   (nombre_elem[0] == 'B'))))
	  {
	   A.Re = -A.Re;
	   A.Im = -A.Im;
	  }

	 /* Entonces, si es de corriente e independiente.. */
	 if (nombre_elem[0] == 'I')

	   /* ..a¤adimos su valor al t‚rmino independiente de la fila 'i',
	      cambiado de signo al cambiar de t‚rmino */
	   *M(i, orden + gendep) = res (*M(i, orden + gendep), A);

	 /* pero si no es de corriente e independiente.. */
	 else

	  /* ..entonces, si es de tensi¢n e independiente.. */
	  if (nombre_elem[0] == 'V')

	    /* A¤adimos su valor, dividido por la suma de Z's de dicha rama
	       y cambiado de signo al pasar del 1er t‚rmino al 2§, al t‚rmino
	       independiente de la fila 'i' */
	    *M(i, orden + gendep) = res (*M(i,orden+gendep),
					 cdiv (A, sumZ (n_elems, elems,
							ramas, rama_act, w)));

	  /* pero si no es independiente (por tanto es dependiente).. */
	  else
	   {
	    /* ..si es de tensi¢n, el valor a a¤adir ser  cte_gen / äZ */
	    if (nombre_elem[0] < 'C')
	      A = cdiv (A, sumZ (n_elems, elems, ramas, rama_act, w));

	    /* Leemos el n§ de orden del generador */
	    k = atoi ((char *) &nombre_elem[1]);

	    /* Si el generador es de tipo 'A'.. */
	    if (nombre_elem[0] == 'A')

	      /* ..su cte. ir  en la posici¢n (i, orden + k - 1) */
	      k += orden - 1;

	    /* , pero si es de tipo 'B'.. *