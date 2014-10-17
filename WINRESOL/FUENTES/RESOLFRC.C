#include <windows.h>
#include <resolver.h>

extern HWND          hwnd;
extern HANDLE        hp;
extern short         cyLine, nVposScrl, minimizada, Fin, Abort;
extern char          frase2[], frase3[], frase4[], frase5[];
extern unsigned      orden, gendep, gendepv, gendepi;
extern complejo huge *p;
extern int 	     analisis;
extern char 	     nom_fich[];
extern double 	     Tact;

extern int 	     diagonaliza_comp (unsigned orden);
extern complejo huge *M (unsigned i, unsigned j);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄVariables y funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

HANDLE          ht;
func_trans huge *t;

func_trans huge *T (unsigned i, unsigned j);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo del c¢digoÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void resp_en_frecuencia (char gen_ind[5],
			 unsigned n_elems, elemento *elems,
			 unsigned n_ramas, rama *ramas,
			 unsigned n_ecs_Z, char (*ecs_Z)[100],
			 unsigned n_ecs_Y, char (*ecs_Y)[100])

/* Funci¢n que calcula la respuesta en frecuencia del circuito y la almacena
   en un fichero de extensi¢n ".FRC" ¢ ".FOU".
   Si (analisis == PERM), el an lisis se realiza respecto al gen. independ.
   cuyo nombre interno se pasa en 'gen_ind', y si (analisis == TRAN), el an -
   lisis que se realiza es de Fourier, con todos los generadores transforma-
   dos.
   El tipo de resultado viene determinado por el contenido de 'resp'.
   El algoritmo realiza un an lisis en r‚g. permanente del circuito para cada
   valor de frecuencia entre 'resp.ini' y 'resp.fin', y con un incr. decimal
   o logar¡tmico seg£n el valor de 'resp.escala' (D, L). */
{
 RECT      rect;
 MSG	   msg;
 FILE      *frec;
 resultado *resp = NIL;
 complejo  r, z;
 float     f_lin, f, inc;
 unsigned  a, b, i, j, m, s, num_comp, n_resp, n_respI, n_respV;
 int       *comp, *compI, *compV,
	   generador;
 char      *pt, nom[84], num[10];
 BOOL      sing;

 extern unsigned recupera_lista_elems (elemento **elems);
 extern unsigned recupera_respuestas  (char tipo, resultado **respuestas);
 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern void temperatura (float T, unsigned n_elems, elemento *elems);
 extern int cond_inic_por_mallas (unsigned no_vars,
				  unsigned n_elems, elemento *elems,
				  unsigned n_ramas, rama *ramas,
				  unsigned n_ecs_Z, char (*ecs_Z)[100],
				  unsigned n_ecs_Y, char (*ecs_Y)[100]);
 extern int cond_inic_por_nudos  (unsigned no_vars,
				  unsigned n_elems, elemento *elems,
				  unsigned n_ramas, rama *ramas,
				  unsigned n_ecs_Z, char (*ecs_Z)[100],
				  unsigned n_ecs_Y, char (*ecs_Y)[100]);

 int Laplace_por_mallas (unsigned n_elems, elemento *elems,
			 unsigned n_ramas, rama *ramas,
			 unsigned n_ecs, char (*ecs)[100]);
 int Laplace_por_nudos (unsigned n_elems, elemento *elems,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs, char (*ecs)[100]);
 void sistema_en_z (complejo Z);

 /* Enviamos a la ventana lo que estamos haciendo */
 strcpy (frase2, "Respuesta: Frecuencial\0");
 strcpy (frase3, "Puntos a calcular: \x0");
 strcpy (frase4, "Puntos calculados: \x0");
 strcpy (frase5, "0\x0");

 InvalidateRect (hwnd, NULL, TRUE);
 UpdateWindow (hwnd);

 /* Inicializamos los flags de terminaci¢n */
 Fin = Abort = 0;

 /* Extraemos las respuestas en frecuencia (tipo 'F') ¢ de Fourier (tipo 'R')
    del fichero */
 if (analisis == PERM)
   n_resp = recupera_respuestas ('F', &resp);
 else
   n_resp = recupera_respuestas ('R', &resp);

 /* Si no queremos ninguna respuesta, simplemente regresamos */
 if (n_resp == 0)
   return;

 /* Obtenemos las respuestas de tensi¢n y las de corriente */
 n_respV = n_respI = 0;
 for (i = 0; i < n_resp; ++i)
   if (resp[i].tipo == 'V')
     ++n_respV;
   else
     ++n_respI;

 /* Reservamos el espacio necesario para las tablas de componentes */
 compV = (int *) calloc (n_respV, 2 * sizeof (int));
 memset (compV, 0, n_respV * 2 * sizeof (int));

 compI = (int *) calloc (n_respI, n_ecs_Z * sizeof (int));
 memset (compI, 0, n_respI * n_ecs_Z * sizeof (int));

 /* Abrimos el fichero que contendr  el resultado */
 strcpy (nom, nom_fich);
 if (analisis == PERM)
   strcat (nom, ".FRC");
 else
   strcat (nom, ".FOU");
 frec = fopen (nom, "wb");

 /* Guardamos los datos de los rangos etc. */
 fwrite (&resp[0].escala, sizeof (char), 1, frec);
 fwrite (&resp[0].ini, sizeof (float), 1, frec);
 fwrite (&resp[0].fin, sizeof (float), 1, frec);
 fwrite (&resp[0].pasos, sizeof (unsigned), 1, frec);

 /* Si queremos la respuesta en r‚gimen permanente, anulamos los gen. indep.
    del circuito excepto aquel respecto al cual calculamos la respuesta */
 if (analisis == PERM)
   for (i = 0; i < n_elems; ++i)
    {
     if ((elems[i].tipo[0] == 'V') || (elems[i].tipo[0] == 'I'))
       if (strcmp (gen_ind, elems[i].tipo) != 0)
	 elems[i].valor = 0;
       else
	{
	 elems[i].signal = 'S';
	 elems[i].valor = 1;
	}
    }

 /* Calculamos los efectos de la temperatura */
 if (resp[0].temp != Tact)
  {
   Tact = resp[0].temp;
   temperatura (Tact, n_elems, elems);
   cond_inic_por_mallas (n_ecs_Z, n_elems, elems, n_ramas, ramas,
			 n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);
   cond_inic_por_nudos  (n_ecs_Z, n_elems, elems, n_ramas, ramas,
			 n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);
  }

 /* El array de enteros apuntado por 'comp?', siendo ? una V ¢ una I, contiene
    los componentes del resultado, es decir, i, j, k,... donde Vm = Vi - Vj ¢
    Im = Ii ñ Ij ñ Ik ... */

 /* Primero, obtenemos las expresiones de las medidas de tensi¢n */
 b = 0;
 for (i = 0; i < n_resp; ++i)
   if (resp[i].tipo == 'V')
    {
     /* Colocamos los n£meros correspondientes a las componentes en la tabla */
     if (resp[i].medida.nodos[0] != 0)
      {
       *(compV + b*2)     =  resp[i].medida.nodos[0];
       *(compV + b*2 + 1) = -resp[i].medida.nodos[1];
      }
     else
       *(compV + b*2)     = -resp[i].medida.nodos[0];

     /* Apuntamos a la siguiente fila de 'compV' */
     ++b;
    }

 /* A continuaci¢n, las expresiones de las medidas de corriente */
 b = 0;
 for (i = 0; i < n_resp; ++i)
   if (resp[i].tipo == 'I')
    {
     /* Obtenemos las mallas en las que se encuentra el elemento */
     a = 0;       /* 'a' apunta la posici¢n actual en la lista de componentes */

     /* Recorremos todas las mallas */
     for (j = 0; j < n_ecs_Z; ++j)

      /* Si el elemento de medida est  en alguna de ellas.. */
      if ((pt = strstr (ecs_Z[j], resp[i].medida.elem)) != NIL)
       {
	/* ..obtenemos el signo de la corriente por la rama */
	while (*(pt--) != '(');

	/* Incorporamos la componente con su signo */
	if (*pt == '-')
	  *(compI + b*n_ecs_Z + a) = -j - 1; /* Es necesario a¤adir 1 al n§ de malla para */
	else                                 /* homogeneizar este formato con el usado en */
	  *(compI + b*n_ecs_Z + a) = j + 1;  /* una tensi¢n entre nudos.                  */

	++a;            /* Apuntamos a la posici¢n de la siguiente componente */
       }

     /* Apuntamos a la siguiente fila de 'compI' */
     ++b;
    }

 /* Este bucle se realiza dos veces, una con las respuestas tipo V y otra
    con las respuestas tipo I */
 for (m = 0; m < 2; ++m)
 {
  /* Escogemos las respuestas de tipo tensi¢n ¢ de tipo corriente */
  if (m == 0)
   {
    n_resp   = n_respV;
    num_comp = 2;
    comp     = compV;
   }
  else
   {
    n_resp   = n_respI;
    num_comp = n_ecs_Z;
    comp = compI;
   }

  if (n_resp > 0)
   {
    if (m == 0)
      Laplace_por_nudos (n_elems, elems, n_ramas, ramas, n_ecs_Y, ecs_Y);
    else
      Laplace_por_mallas (n_elems, elems, n_ramas, ramas, n_ecs_Z, ecs_Z);

    /* Enviamos a la ventana el n§ de pasos a calcular */
    strcpy (frase3, "Puntos a calcular: \x0");
    strcat (frase3, itoa (resp[0].pasos, num, 10));
    GetClientRect (hwnd, &rect);
    rect.top    = 5 * cyLine - nVposScrl * SCRLDESPL;
    rect.bottom = rect.top + cyLine - 1;
    InvalidateRect (hwnd, &rect, TRUE);

    /* Calculamos la frec. lineal inicial y el inc. lineal de frecuencia */
    if (resp[0].escala == 'D')
     {
      f_lin = resp[0].ini;
      inc = (resp[0].fin - resp[0].ini) / resp[0].pasos;
     }
    else
     {
      f_lin = log10 (resp[0].ini);
      inc = log10 (resp[0].fin / resp[0].ini) / resp[0].pasos;
     }

    /* Asignamos a 'f' el valor inicial de frecuencia */
    f = resp[0].ini;

    /* Particularizaremos para s = 0 + jw */
    z.Re = 0;
    z.Im = 2 * pi * f;

    /* Creamos la matriz compleja */
    hp = GlobalAlloc (GMEM_MOVEABLE,
		      (orden + gendep) * (orden + gendep + 1) * sizeof (complejo));
    p = (complejo huge *) GlobalLock (hp);

    /* Recorremos el espectro linealmente desde 'resp.ini' hasta 'resp.fin' */
    for (s = 0; (s < resp[0].pasos) && (!Fin); ++s)
     {
      /* Enviamos a la ventana el n§ de paso */
      strcpy (frase4, "Puntos calculados: \x0");
      strcat (frase4, itoa (s, num, 10));
      strcpy (frase5, itoa (s * 100 / resp[0].pasos, num, 10));

      if (minimizada)
	InvalidateRect (hwnd, NULL, FALSE);
      else
       {
	GetClientRect (hwnd, &rect);
	rect.top    = 7 * cyLine - nVposScrl * SCRLDESPL;
	rect.bottom = rect.top + 4 * cyLine - 1;
	InvalidateRect (hwnd, &rect, FALSE);
       }

      /* Desbloqueamos la memoria antes de entrar en el bucle de mensaje */
      GlobalUnlock (hp);

      /* Recogemos y procesamos los mensajes que haya en la cola */
      while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
       {
	TranslateMessage (&msg);
	DispatchMessage  (&msg);
       }

      /* Rebloqueamos la memoria */
      p = (complejo huge *) GlobalLock (hp);

      if (!Fin)
       {
	/* Constru¡mos la matriz compleja para la frecuencia 'f' */
	sistema_en_z (z);

	/* La diagonalizamos */
	sing = diagonaliza_comp (orden + gendep);

	/* Resolvemos para las variables necesarias */
	for (b = 0; b < n_resp; ++b)
	 {
	  /* Si la matriz del sistema no es singular.. */
	  if (!sing)
	   {
	    /* ..calculamos la respuesta */
	    r.Re = r.Im = 0;
	    for (i = 0; i < num_comp; ++i)
	      if (*(comp + b*num_comp + i) > 0)
		r = sum (r, cdiv (*M(*(comp + b*num_comp + i) - 1,
				     orden + gendep),
				  *M(*(comp + b*num_comp + i) - 1,
				     *(comp + b*num_comp + i) - 1)));
	      else
		if (*(comp + b*num_comp + i) < 0)
		  r = res (r, cdiv (*M(-*(comp + b*num_comp + i) - 1,
				       orden + gendep),
				    *M(-*(comp + b*num_comp + i) - 1,
				       -*(comp + b*num_comp + i) - 1)));
	   }

	  /* La guardamos en el fichero */
	  fseek (frec, sizeof(char) + 2*sizeof(float) + sizeof(unsigned) +
		       ((m * n_respV + b) * resp[0].pasos + s) * sizeof (complejo),
		       SEEK_SET);
	  fwrite (&r, sizeof (r), 1, frec);
	 }


	/* Incrementamos la frecuencia lineal 'f_lin' */
	f_lin += inc;

	/* Variamos la frec. real de an lisis 'f' */
	f = (resp[0].escala == 'D') ? f_lin : pow (10, f_lin);

	/* Variamos la s */
	z.Im = 2 * pi * f;
       }
     }

    /* Si hemos terminado prematuramente, actualizamos el n§ de muestras */
    if (Fin)
     {
      fseek  (frec, sizeof (resp[0].escala) + sizeof (resp[0].ini), SEEK_SET);
      fwrite (&f, sizeof (float), 1, frec);
      fwrite (&s, sizeof (unsigned), 1, frec);
     }

    /* Liberamos el espacio utilizado */
    GlobalFree (hp);
    GlobalFree (ht);
   }
 }

 /* Liberamos el resto del espacio utilizado */
 free (compV);
 free (compI);
 free (resp);

 /* Cerramos el fichero */
 fclose (frec);

 /* Restauramos la tabla de elementos original (si la hemos cambiado) */
 if (analisis == PERM)
   n_elems = recupera_lista_elems (&elems);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Laplace_por_mallas (unsigned n_elems, elemento *elems,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs, char (*ecs)[100])

/* Esta funci¢n construye la matriz de transformadas del circuito por el
   m‚todo de las mallas, a partir de la posici¢n apuntada por 't'. */
{
 /* Las variables 'orden'
		  't'
		  'gendep', 'gendepv' y 'gendepi' son globales */

 func_trans A, Z;
 int signo, elem_aux, elem_act, op;
 unsigned a, b, i, j, k;
 long tamano;
 char Zij[100], nombre_elem[5], *punt;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern char *obtener_comunes (char Hii[100], char Hjj[100], char Hij[100]);
 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);

 void exp_var_Laplace (unsigned n_elems, elemento *elems, unsigned e,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs, char (*ecs)[100],
		       unsigned fila, char t);
 func_trans valor_tdo (elemento *elems, unsigned e);

/*
 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º                                                                          º
 º   Comienzo de la rutina de an lisis por mallas en el dominio de Lapl…ce  º
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
 tamano = (orden + gendep) * (orden + gendep + 1) * sizeof (func_trans);
 ht = GlobalAlloc (GMEM_MOVEABLE, tamano);
 t = (func_trans huge *) GlobalLock (ht);

 /* Inicializamos la matriz a todo ceros */
 _fmemset (t, 0, tamano);

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
     Z = valor_tdo (elems, elem_aux);

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
     T(i,i)->num.coef[0] = T(i,i)->den.coef[0] = 1;

     /* Si el generador es independiente.. */
     if (*punt == 'I')

	/* ..constru¡mos la ec. "Ii = valor_gen" */
	*T(i, orden + gendep) = Z;

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
       if (cero (T(k,k)->num))
	 exp_var_Laplace (n_elems, elems, elem_aux, n_ramas, ramas, n_ecs,
			  ecs, k, 'Z');

       /* Finalmente a¤adimos el t‚rmino "-cte_gen x Idep" */
       *T(i,k) = operar_func (RESTA, *T(i,k), Z);
      }
    }

   /* Si no hay gens. de I en la malla, constru¡mos la fila normalmente */
   else
   /* Idem con 'columna', formando dos bucles anidados que construyen la
      matriz Z. El elemento que construimos en cada pasada es el (i,j).
      N¢tese que nunca podr n aparecer generadores de corriente en las
      mallas, ya que si los hubiera la malla no se construir¡a. */

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
      A = valor_tdo (elems, elem_act);

      /* Si el elemento es un diodo.. */
      if (nombre_elem[0] == 'Q')
       {
	/* ..a¤adimos su resistencia de peque¤a se¤al */
	Z.num.grado = Z.den.grado = 0;
	Z.num.coef[0] = 1 / (elems[elem_act].valor * valor_gen (elem_act,
				      elems[elem_act].CT, 0, 0, 0, 0, 1,
				      elems));
	Z.den.coef[0] = 1;

	/* Afectamos la Rd por el signo si no estamos en la diagonal */
	if (i != j)
	  for (b = 0; b <= Z.num.grado; ++b)
	    Z.num.coef[b] *= signo;

	/* A¤adimos el valor a la fila y columna correspondientes */
	*T(i,j) = operar_func (SUMA, *T(i,j), Z);
       }

      /* Si el elemento es pasivo... */
      if ((nombre_elem[0] == 'R') || (nombre_elem[0] == 'L') ||
	  (nombre_elem[0] == 'C'))
       {
	/* El signo s¢lo cuenta cuando no estamos en la diagonal */
	if (i != j)
	  for (b = 0; b <= A.num.grado; ++b)
	    A.num.coef[b] *= signo;

	/* A¤adimos el valor a la fila y columna correspondientes */
	*T(i,j) = operar_func (SUMA, *T(i,j), A);
       }

      /* pero si es activo (generador ¢ modelo de diodo)... */
      else

       /* ..s¢lo se le procesar  una vez; cuando fila = col. */
       if (i == j)
	{
	 /* Le damos el signo que debe llevar; este es igual al de la rama
	    de la malla en la que se encuentra. */
	 for (b = 0; b <= A.num.grado; ++b)
	   A.num.coef[b] *= signo;

	 /* De nuevo n¢tese que no pueden aparecer gen. de corriente en
	    la malla actual ya que, si los hay, la fila no se construye */

	 /* Entonces, si es de tensi¢n e independiente.. */
	 if ((nombre_elem[0] == 'V') || (nombre_elem[0] == 'Q'))

	   /* ..a¤adimos su valor al t‚rmino independiente de la fila 'i',
	      cambiado de signo al cambiar de t‚rmino */
	   *T(i, orden+gendep) = operar_func (RESTA, *T(i, orden+gendep), A);

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
	   *T(i, k) = A;

	   /* Y si es la 1¦ vez que se le procesa, en la misma fila ponemos
	      la expresi¢n de la variable de la que depende */
	   if (cero (T(k,k)->num))
	     exp_var_Laplace (n_elems, elems, elem_act, n_ramas, ramas, n_ecs,
			      ecs, k, 'Z');
	  }
	}
     }
    }
  }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int Laplace_por_nudos (unsigned n_elems, elemento *elems,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs, char (*ecs)[100])


/* Esta funci¢n construye la matriz de transformadas del circuito por el
   m‚todo de los nudos, a partir de la posici¢n apuntada por 't'. */
{
 /* Las variables 'orden'
		  't'
		  'gendep', 'gendepv', 'gendepi' son globales */

 polinomio pol_aux;
 func_trans A, Z, D;
 int elem_act, elem_aux, rama_act, op;
 unsigned a, b, i, j, k, ni, nf;
 long tamano;
 char Yij[100], nombre_elem[5], *punt, *aux;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern int buscar_rama (unsigned n_ramas, rama *ramas, char tipo[5]);
 extern char *obtener_comunes (char Hii[100], char Hjj[100], char Hij[100]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);

 void exp_var_Laplace (unsigned n_elems, elemento *elems, unsigned e,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs, char (*ecs)[100],
		       unsigned fila, char t);
 func_trans valor_tdo (elemento *elems, unsigned e);
 func_trans sumZ_tda (unsigned n_elems, elemento *elems,
		      rama *ramas, unsigned r);
/*
 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º                                                                          º
 º   Comienzo de la rutina de an lisis por nudos en el dominio de Lapl…ce   º
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
 tamano =  (orden + gendep) * (orden + gendep + 1) * sizeof (func_trans);
 ht = GlobalAlloc (GMEM_MOVEABLE, tamano);
 t = (func_trans huge *) GlobalLock (ht);

 /* Inicializamos la matriz a todo ceros */
 _fmemset (t, 0, tamano);

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
     /* ..nos situamos en el primer car cter de la rama despu‚s del '(' */
     aux = punt;
     while (*(aux - 1) != '(')
       --aux;

     /* Recorremos la rama hasta encontrar alguna Z ¢ el fin de la rama */
     while ((*aux != 'R') && (*aux != 'L') && (*aux != 'C') && (*aux != ')'))
       aux += 4;
    }

   /* Si hab¡a algun generador sin Z asociada.. */
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

      /* Finalmente obtenemos su cte. transformada, con signo (+) */
      Z = valor_tdo (elems, elem_aux);

      /* Tanto como si el gen. es dep. ¢ indep. pondremos Vi a 1 y Vj a -1 */
      T(i,i)->num.coef[0] = T(i,i)->den.coef[0] = 1;
      rama_act = buscar_rama (n_ramas, ramas, nombre_elem);

      if ((ramas[rama_act].nodoi > 0) && (ramas[rama_act].nodof > 0))
       {
	if (ramas[rama_act].nodoi == (i + 1))
	 {
	  T(i, ramas[rama_act].nodof - 1)->num.coef[0] = -1;
	  T(i, ramas[rama_act].nodof - 1)->den.coef[0] = 1;
	 }
	else
	 if (ramas[rama_act].nodof == (i + 1))
	  {
	   T(i, ramas[rama_act].nodoi - 1)->num.coef[0] = -1;
	   T(i, ramas[rama_act].nodoi - 1)->den.coef[0] = 1;
	  }
       }

      /* Si el generador es independiente.. */
      if (*punt == 'V')
       {
	/* ..completamos la ec. "Vi - Vj = ñvalor_gen" */
	/* Si el nudo 'i' es el inicial (y por tanto el +).. */
	if (ramas[rama_act].nodoi == (i + 1))

	  /* ..a¤adimos el t‚rmino "+valor_gen" al t‚rmino independiente */
	  *T(i, orden + gendep) = Z;

	/* pero, si no es el inicial (y por tanto tampoco el +).. */
	else

	  /* ..a¤adimos el t‚rmino "-valor_gen" al t‚rmino independiente */
	  *T(i,orden + gendep) = operar_func (RESTA, *T(i,orden + gendep), Z);
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
	  *T(i, k) = Z;

	/* pero, si es el inicial (y por tanto el +).. */
	else

	  /* ..a¤adimos el t‚rmino "-cte_gen" a la columna correspondiente */
	  *T(i, k) = operar_func (RESTA, *T(i, k), Z);

	/* Y, si es la primera vez que se le procesa, ponemos su expresi¢n en
	   la fila 'k' */
	if (cero (T(k,k)->num))
	  exp_var_Laplace (n_elems, elems, elem_act, n_ramas, ramas, n_ecs,
			   ecs, k, 'Y');

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
      A = valor_tdo (elems, elem_act);

      /* Si el elemento es un diodo.. */
      if (nombre_elem[0] == 'Q')
       {
	/* ..obtenemos su resistencia en peque¤a se¤al */
	D.num.grado = D.den.grado = 0;
	D.num.coef[0] = 1 / (elems[elem_act].valor * valor_gen (elem_act,
				      elems[elem_act].CT, 0, 0, 0, 0, 1,
				      elems));
	D.den.coef[0] = 1;

	/* La a¤adimos a la suma de Z's de la rama actual */
	Z = operar_func (SUMA, Z, D);
       }

      /* Si el elemento es pasivo... */
      if ((nombre_elem[0] == 'R') || (nombre_elem[0] == 'L') ||
	  (nombre_elem[0] == 'C'))

	/* ..a¤adimos su Z a la suma de Z's de la rama actual */
	Z = operar_func (SUMA, Z, A);

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
	     for (b = 0; b <= A.num.grado; ++b)
	       A.num.coef[b] = -A.num.coef[b];

	 /* Entonces, si es de corriente e independiente.. */
	 if (nombre_elem[0] == 'I')

	   /* ..a¤adimos su valor al t‚rmino independiente de la fila 'i',
	      cambiado de signo al cambiar de t‚rmino */
	   *T(i, orden + gendep) = operar_func (RESTA,
						*T(i, orden + gendep), A);

	 /* pero, si es de tensi¢n e independiente (¢ modelo de diodo).. */
	 else
	  if ((nombre_elem[0] == 'V') || (nombre_elem[0] == 'Q'))

	    /* A¤adimos su valor, dividido por la suma de Z's de dicha rama
	       y cambiado de signo al pasar del 1er t‚rmino al 2§, al t‚rmino
	       independiente de la fila 'i' */
	    *T(i, orden + gendep) = operar_func (RESTA, *T(i,orden+gendep),
				     div_func (A, sumZ_tda (n_elems, elems,
							    ramas, rama_act)));

	  /* pero si no es independiente (por tanto es dependiente).. */
	  else
	   {
	    /* ..si es de tensi¢n, el valor a a¤adir ser  cte_gen / äZ */
	    if (nombre_elem[0] < 'C')
	      A = div_func (A, sumZ_tda (n_elems, elems, ramas, rama_act));

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
	    *T(i, k) = A;

	    /* Y si es la 1¦ vez que se le procesa, en la misma fila ponemos
	       la expresi¢n de la variable de la que depende */
	    if (cero (T(k,k)->num))
	      exp_var_Laplace (n_elems, elems, elem_act, n_ramas, ramas, n_ecs,
			       ecs, k, 'Y');
	   }
	}

      if (Yij[a] == ')')
       {
	if (!cero (Z.num))
	 {                                          /* Nos saltamos el ')', */
	  if (i == j)                               /* si  lo  hay, y  como */
	    op = SUMA;                              /* pasamos a  una nueva */
	  else                                      /* rama,  a¤adimos   la */
	    op = RESTA;                             /* suma de Z's actual a */
						    /* la pos. (i,j) e ini- */
	  /* Construimos 1 / Z(s) */                /* -cializamos la  suma */
	  pol_aux = Z.num;                          /* de Z's  a 0.         */
	  Z.num = Z.den;
	  Z.den = pol_aux;

	  /* A¤adimos 1/Z al t‚rmino (i,j) */
	  *T(i,j) = operar_func (op, *T(i,j), Z);
	 }
	memset (&Z, 0, sizeof (func_trans));
	++a;
       }
     }
   }
  }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void exp_var_Laplace (unsigned n_elems, elemento *elems, unsigned e,
		      unsigned n_ramas, rama *ramas,
		      unsigned n_ecs, char (*ecs)[100],
		      unsigned fila, char t)

/* Esta funci¢n pone en la fila 'fila' la expresi¢n de la variable de la que
   depende el generador apuntado por 'e', es decir;
   Si el gen. depende de una 'Vdep':
	      Vdep = (Z1 + Z2 + ... + Zn) x (ñ Ii ñ Ij ñ ...) + Ei + Ej + ...
   Si el gen. depende de una 'Idep':
	      Idep = ñ Ii ñ Ij ñ ...                                        */
{
 func_trans Z, Y, D, val;
 char *pt, *aux, cam[100];
 char nombre[5];
 int op;
 unsigned ni, nf, elem, b, i, j, r = 0;

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern int buscar_rama (unsigned n_ramas, rama *ramas, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern int camino (unsigned n_ramas, rama *ramas,
		    unsigned salida, unsigned llegada, char elem[100]);
 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);

 func_trans valor_tdo (elemento *elems, unsigned e);
 func_trans sumZ_tda (unsigned n_elems, elemento *elems,
		      rama *ramas, unsigned r);

 /* Ponemos un 1 en 'fila' y la col. correspondiente a este gen., de modo que
    empezamos la ec. "Vdep = ..." (Vdep = V de la que dep. el gen.)
	      o bien "Idep = ..." (Idep = I "   "  "   "   "   "  ) */

  T(fila, fila)->num.grado   = T(fila, fila)->den.grado = 0;
  T(fila, fila)->num.coef[0] = T(fila, fila)->den.coef[0] = 1;

  /* Si queremos la expresi¢n en Z... */
  if (t == 'Z')
   {
    /* Si el generador depende de una corriente... */
    if ((elems[e].tipo[0] == 'B') || (elems[e].tipo[0] == 'E'))
     {
      val.num.grado = val.den.grado = 0;
      val.num.coef[0] = val.den.coef[0] = 1;

      /* Ponemos en 'nombre' el del elem. por el que circula la Idep */
      strcpy (nombre, elems[e].caract.elem);

      /* Buscamos las mallas en las que aparece el elem. con la variable */
      for (i = 0; i < n_ecs; ++i)            /* Recorremos todas las mallas */
       {
	/* Si el elem. con la var. de la que dep. el gen. est  en la malla i.. */
	if ((pt = strstr (ecs[i], nombre)) != NIL)
	 {
	  while (*(pt--) != '(');   /* Retrocedemos a la posici¢n del signo */

	  /* A¤adimos el valor correspondiente pero con el signo cambiado ya
	     que pasamos del 2§ t‚rmino al 1§ */
	  *T(fila, i) = operar_func ((*pt == '-') ? SUMA : RESTA,
				     *T(fila, i), val);
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
       memset (&Z, 0, sizeof (func_trans));
       do
       {
	/* Tomamos un nombre interno de elemento de la rama */
	obtener_nombre (pt, nombre);

	/* Apuntamos al car cter siguiente al nombre */
	pt += 4;

	/* Buscamos el elemento en la lista */
	elem = buscar_elem (n_elems, elems, nombre);

	/* Obtenemos su valor transformado */
	val = valor_tdo (elems, elem);

	/* Si el elemento es un diodo.. */
	if (nombre[0] == 'Q')
	 {
	  /* ..a¤adimos su Rd */
	  D.num.grado = D.den.grado = 0;
	  D.num.coef[0] = 1 / valor_gen (elem, elems[elem].CT, 0,
					 0, 0, 0, 1, elems);
	  D.den.coef[0] = 1;

	  Z = operar_func (SUMA, Z, D);
	 }

	/* Si el elemento es pasivo.. */
	if ((nombre[0] == 'R') || (nombre[0] == 'L') || (nombre[0] == 'C'))

	    /* ..a¤adimos su valor a la suma de Z's */
	    Z = operar_func (SUMA, Z, val);

	/* pero si es activo.. */
	else

	  /* ..entonces, si es un gen. independiente (¢ modelo de diodo).. */
	  if ((nombre[0] == 'V') || (nombre[0] == 'Q'))

	    /* ..ponemos su valor en el t‚rmino indep. con el signo normal */
	    *T(fila, orden + gendep) =
			      operar_func (op, *T(fila, orden + gendep), val);

	  /* Pero, si es dependiente.. */
	  else
	   {
	    j = atoi ((char *) &nombre[1]);     /* Obtenemos su n§ de orden */

	    /* Si el gen. es del tipo 'A'.. */
	    if (nombre[0] == 'A')

	      /* ..ponemos su valor en la col. 'orden + j - 1' */
	      *T(fila, orden + j - 1) =
			      operar_func (-op, *T(fila, orden + j - 1), val);

	    /* Sin embargo, si es del tipo 'B'.. */
	    else

	      /* ..ponemos su valor en la col. 'orden + gendepv - j' */
	      *T(fila, orden + gendepv - j) =
			operar_func (-op, *T(fila, orden + gendepv - j), val);
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
	   *T(fila, i) = operar_func (((*aux == '-') ? SUMA : RESTA) * op,
				      *T(fila, i), Z);
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
	*T(fila,orden+gendep) = valor_tdo (elems, buscar_elem (n_elems, elems,
							       nombre));
       }

      /* pero, si hay un gen. de corriente dependiente.. */
      else
       {
	if ((pt = strstr (ramas[r].elem, "D")) == NIL)
	  pt = strstr (ramas[r].elem, "E");

	if (pt != NIL)
	 {
	  /* ..la ec. ser  "Idep = cte_gen x I'dep" */

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
	  *T(fila, j) = operar_func (RESTA, *T(fila, j),
				     valor_tdo (elems,
						buscar_elem (n_elems, elems,
							     nombre)));
	 }

	/* y si, finalmente, no hay ning£n gen. de corriente en la rama.. */
	else
	{
	 /* Calculamos la suma de las Z's de la rama y su inversa */
	 Z = sumZ_tda (n_elems, elems, ramas, r);
	 Y.num = Z.den;
	 Y.den = Z.num;

	 /* Ponemos -1/Z en 'fila' y la col. correspondiente a Vi */
	 if ((ni = ramas[r].nodoi) > 0)
	   *T(fila, ni - 1) = operar_func (RESTA, *T(fila, ni - 1), Y);

	 /* Ponemos 1/Z en 'fila' y la col. correspondiente a Vf */
	 if ((nf = ramas[r].nodof) > 0)
	   *T(fila, nf - 1) = Y;

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

	    /* Obtenemos su valor transfdo. dividido por el äZ de la rama */
	    val = div_func (valor_tdo (elems, buscar_elem (n_elems, elems,
							   nombre)), Z);

	    /* Si es independiente.. */
	    if (nombre[0] == 'V')
	     {
	      /* ..ponemos su valor con signo opuesto (pasamos de 2§ al 1§) */
	      for (b = 0; b <= val.num.grado; ++b)
		val.num.coef[b] = -val.num.coef[b];

	      *T(fila, orden + gendep) = val;
	     }

	    /* Pero, si es dependiente.. */
	    else
	     {
	      j = atoi ((char *) &nombre[1]);   /* Obtenemos su n§ de orden */

	      /* Si el gen. es del tipo 'A'.. */
	      if (nombre[0] == 'A')

		/* ..ponemos su valor en la col. 'orden + j - 1' */
		*T(fila, orden + j - 1) = val;

	      /* Sin embargo, si es del tipo 'B'.. */
	      else

		/* ..ponemos su valor en la col. 'orden + gendep - j' */
		*T(fila, orden + gendep - j) = val;
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
      /* ...la ec. a construir es "Vdep = Vi - Vf" -> "Vdep - Vi + Vf = 0" */

      /* poner un '-1' en 'fila' y la col. correspondiente a Vi */
      if ((ni = elems[e].caract.nodos[0]) > 0)
       {
	T(fila, ni - 1)->num.grado = T(fila, ni - 1)->den.grado = 0;
	T(fila, ni - 1)->num.coef[0] = -1;
	T(fila, ni - 1)->den.coef[0] = 1;
       }

      /* poner un '1' en 'fila' y la col. correspondiente a Vf */
      if ((nf = elems[e].caract.nodos[1]) > 0)
       {
	T(fila, nf - 1)->num.grado = T(fila, nf - 1)->den.grado = 0;
	T(fila, nf - 1)->num.coef[0] = T(fila, nf - 1)->den.coef[0] = 1;
       }
     }
   }
 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

func_trans valor_tdo (elemento *elems, unsigned e)

/* Esta funci¢n devuelve el valor Laplaciano del elemento n§ 'e' */
{
 func_trans v;
 float A;
 int i;

 /* Inicializamos la funci¢n resultado a 0 */
 memset (&v, 0, sizeof (func_trans));

 /* Obtenemos el valor del elemento */
 A = elems[e].valor;

 switch (elems[e].tipo[0])
 {
  case 'L':               /* Si es una bobina.. */
  {
   v.num.grado = 1;       /*     di             */
   v.num.coef[1] = A;     /*   LúÄÄ ==> LúsúI   */
   v.den.coef[0] = 1;     /*     dt             */
   break;
  }

  case 'C':               /* Si es un condensador.. */
  {
   v.num.coef[0] = 1;     /*   1 ô          1       */
   v.den.grado = 1;       /*   Äú³iúdt ==> ÄÄÄúI    */
   v.den.coef[1] = A;     /*   C õ         Cús      */
   break;
  }

  case 'V':               /* Si es un gen. independiente.. */
  case 'I':
  {
   /* A¤adimos el polinomio debido a los desfases */

   switch (elems[e].signal)
   {
    case 'P':                 /* Si es un pulso.. */
    {
     v.num.coef[0] = A;       /*              A   */
     v.den.grado = 1;         /*   Aúu(t) ==> Ä   */
     v.den.coef[1] = 1;       /*              s   */
     break;
    }
    case 'R':                 /* Si es una rampa.. */
    {
     v.num.coef[0] = A;       /*               A   */
     v.den.grado = 2;         /*  Aútúu(t) ==> Ä   */
     v.den.coef[2] = 1;       /*               sı  */
     break;
    }
    case 'T':                 /* Si es una par bola.. */
    {
     v.num.coef[0] = 2 * A;   /*                  A   */
     v.den.grado = 3;         /* Aútüúu(t) ==> núÄÄÄÄ */
     v.den.coef[3] = 1;       /*                  n+1 */
     break;                   /*                 s    */
    }
    case 'S':                 /* Si es un seno.. */
    {
     if (analisis == PERM)    /* ..en an lisis permanente.. */
      {
       v.num.coef[0] = A;     /*      Aúsen(wút) ==> A      */
       v.den.coef[0] = 1;
      }
     else                     /* ..pero, en an lisis de Fourier.. */
      {
       v.num.coef[0] = 2 * pi * elems[e].caract.frec; /*                 Aúwı */
       v.den.grado = 2;                               /* Aúsen(wút) ==> ÄÄÄÄÄ */
       v.den.coef[2] = 1;                             /*                sı+wı */
       v.den.coef[0] = v.num.coef[0] * v.num.coef[0];
       v.num.coef[0] *= A;
      }
     break;
    }
    case 'D':
    {
     v.num.grado = 10;
     for (i = 0; i <= 10; ++i)
       v.num.coef[i] = A * elems[e].f.coef[10-i];
     v.den.grado = 11;
     break;
    }
   }
   break;
  }

  case 'Q':               /* Si es un diodo, devolvemos Vcc / s */
  {
   if (analisis == TRAN)
    {
     v.num.coef[0] = elems[e].CT;
     v.den.grado = 1;
     v.den.coef[1] = 1;
    }
   break;
  }

  default:                /* Si es una resistencia.. */
  {
   v.num.coef[0] = A;     /*     Rúi ==> RúI         */
   v.den.coef[0] = 1;
  }
 }

 return (v);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

func_trans sumZ_tda (unsigned n_elems, elemento *elems,
		     rama *ramas, unsigned r)

/* Funci¢n que devuelve la suma de los valores de las impedancias en 'rama' */
{
 func_trans Z, D;
 unsigned e;
 char *pt;
 char nombre[5];

 extern int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5]);
 extern char *obtener_nombre (char *punt, char nombre[5]);
 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);

 func_trans valor_tdo (elemento *elems, unsigned e);

 memset (&Z, 0, sizeof (func_trans));       /* Inicializamos la suma de Z's */

 pt = ramas[r].elem;             /* Apuntamos al 1er car cter de los elems. */

 do                                                           /* Repetir... */
  {
   /* Tomamos un nombre interno de elemento de la rama */
   obtener_nombre (pt, nombre);
   pt += 4;

   /* Buscamos el elemento en la lista */
   e = buscar_elem (n_elems, elems, nombre);

   /* Si el elemento es pasivo a¤adimos su valor a la suma de Z's */
   if ((nombre[0] == 'R') || (nombre[0] == 'L') || (nombre[0] == 'C'))
     Z = operar_func (SUMA, Z, valor_tdo (elems, e));

   /* pero, si es un diodo, a¤adimos su Rd */
   else
     if (nombre[0] == 'Q')
      {
       D.num.grado = D.den.grado = 0;
       D.num.coef[0] = 1 / valor_gen (e, elems[e].CT, 0, 0, 0, 0, 1, elems);
       D.den.coef[0] = 1;

       Z = operar_func (SUMA, Z, D);
      }
  }
 while (*pt != 0);                    /* ...hasta que no haya m s elementos */

 return (Z);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void sistema_en_z (complejo Z)

/* Funci¢n que toma la matriz de transformadas apuntada por 't' y devuelve,
   en la matriz compleja apuntada por 'p', la primera particularizada en
   s = 'Z' */
{
 unsigned i, j;

 for (i = 0; i < orden + gendep; ++i)
   for (j = 0; j <= orden + gendep; ++j)
     *M(i,j) = func_en_z (*T(i,j), Z);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

func_trans huge *T (unsigned i, unsigned j)

/* Esta funci¢n nos ahorra el trabajo de calcular la posici¢n de memoria que
   corresponde al elemento de la matriz de an lisis transitorio */
{
 return ((t + (orden + gendep + 1)*i) + j);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/