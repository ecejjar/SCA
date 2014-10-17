#include "compilar.h"

/*---------------Zona de definici¢n de variables globales-------------------*/

unsigned no, nd;
FILE *Z, *Y;

extern char nom_fich[80];

/*--------------Comienzo de la zona de definici¢n de funciones--------------*/

void analizar (void)
{
 registro *rama_ini = NIL, *rama_fin = NIL, *actual = NIL, *rama_aux;

 void matriz_Z (registro *rama_ini, registro *rama_fin);
 void matriz_Y (registro *rama_ini, registro *rama_fin);
 void recupera_lista_ramas (registro **rama_ini, registro **rama_fin);

 recupera_lista_ramas (&rama_ini, &rama_fin);

 matriz_Z (rama_ini, rama_fin);

 matriz_Y (rama_ini, rama_fin);

 /* Liberamos el espacio reservado para la lista de ramas */
 actual = rama_ini;
 do
 {
  rama_aux = actual;
  actual = actual->siguiente;
  free (rama_aux);
 }
 while (actual != rama_fin);

 free (actual);

 return;
}

/*--------------------------------------------------------------------------*/

unsigned n_nodos (registro *rama_ini, registro *rama_fin)

/* Determina el n£mero de nodos que tiene el circuito (incluido el 0).
   Para hacerlo crea una ristra de bits y, cuando encuentra un nodo, com-
   prueba si el bit correspondiente a su n£mero es "1"; si no lo es, lo
   pone a "1" e incrementa el n§ de nodos.
   El n§ de ecs. para analizarlo por el m‚todo de los nudos es n_nodos-1 */
{
 registro *actual = NIL;
 unsigned n = 0, maximo = 0;
 char lista[255], mascara = 0;

 strset (lista, 0);

 do
 {
  actual = (actual == NIL) ? rama_ini : actual->siguiente;

  n = actual->datos.nodoi;
  mascara = 128 >> (n % 8);
  if ((lista[n / 8] & mascara) == 0)
   {
    ++maximo;
    lista[n / 8] |= mascara;
   }

  n = actual->datos.nodof;
  mascara = 128 >> (n % 8);
  if ((lista[n / 8] & mascara) == 0)
   {
    ++maximo;
    lista[n / 8] |= mascara;
   }
 }
 while (actual != rama_fin);

 return (maximo);
}

/*--------------------------------------------------------------------------*/

unsigned max_nodo (registro *rama_ini, registro *rama_fin)

/* Determina el m ximo n£mero de nodo que haya en la lista de ramas */
{
 registro *actual = NIL;
 unsigned maximo = 0;

 do
 {
  actual = (actual == NIL) ? rama_ini : actual->siguiente;
  maximo = max (maximo, max (actual->datos.nodoi, actual->datos.nodof));
 }
 while (actual != rama_fin);

 return (maximo);
}

/*--------------------------------------------------------------------------*/

unsigned n_ecs_Z (registro *rama_ini, registro *rama_fin)

/* Esta funci¢n calcula el n§ de ecuaciones precisas para analizar la red
   por mallas, que es igual al n§ de ramas de enlace. Este valor se ob-
   tiene como n§ de ramas - (n§ de nudos - 1) */
{
 unsigned necs = 0;
 registro *actual = NIL;

 unsigned n_nodos (registro *rama_ini, registro *rama_fin);

 do
 {
  actual = (actual == NIL) ? rama_ini : actual->siguiente;
  ++necs;
 }
 while (actual != rama_fin);

 necs -= n_nodos (rama_ini, rama_fin) - 1;

 return (necs);
}

/*--------------------------------------------------------------------------*/

void matriz_Z (registro *rama_ini, registro *rama_fin)

/* Esta funci¢n construye la matriz Z a partir del arbol y las ramas.
   El proceso consiste en ir recorriendo la tabla de ramas, cogiendo cada
   rama que no forme parte del arbol, y uniendo los elementos que est‚n
   en la malla formada a¤adiendo al arbol la rama escogida. Con los ele-
   mentos unidos por cada malla, formamos los elementos de la diagonal
   de la matriz Z, obteniendo los restantes mediante los elementos comunes
   a dos elementos de la diagonal */
{
 registro *actual = NIL;
 char nom[13], Zii[100];
 unsigned a;

 void arbol (registro *rama_ini, registro *rama_fin);
 unsigned n_ecs_Z (registro *rama_ini, registro *rama_fin);
 void limpia_lista (registro *rama_ini, registro *rama_fin);
 unsigned malla (registro *rama_ini, registro *rama_fin, registro *rama,
		 unsigned salida, unsigned llegada, char elem[100]);

 strcpy (nom, nom_fich);
 *strrchr (nom, '.') = 0;
 strcat (nom, ".Z");
 Z = fopen (nom, "wb");

 /* Construye el arbol del circuito */
 arbol (rama_ini, rama_fin);

 /* Calcula el n§ de ecs. del circuito (n§ de mallas) */
 a = n_ecs_Z (rama_ini, rama_fin);

 /* Lo convierte a string */
 itoa (a, Zii, 10);

 /* Guarda el n§ de ecs. en el 1er registro del fichero */
 fwrite (Zii, sizeof (Zii), 1, Z);

 do                                   /* Bucle que pasa por todas las ramas */
 {
  actual = (actual == NIL) ? rama_ini : actual->siguiente;

  if (actual->datos.marca == 0)       /* Si la rama no pertenece al arbol...*/
  {
   limpia_lista (rama_ini, rama_fin);    /* ...limpia los  flags de 'usada' */

   for (a = 0; a < 100; ++a)
     Zii[a] = 0;                         /* Pone Zii a cadena vac¡a         */

   actual->datos.usada = 1;              /* Marca la rama actual como usada */

   /* Llama a malla que localiza los elementos que pertenecen a la malla,
      excepto el que est  en la rama apuntada por 'actual' */
   malla (rama_ini, rama_fin, actual, actual->datos.nodof, actual->datos.nodoi,
	  Zii);

   /* A¤ade dicho elemento a la malla */
   strcat (Zii, "(");
   strcat (Zii, actual->datos.elem);
   strcat (Zii, ")");

   fwrite (Zii, sizeof (Zii), 1, Z);    /* Guardamos la malla en el fichero */
  }
 }
 while (actual != rama_fin);

 /* Cerramos el fichero */
 fclose (Z);

 return;
}

/*--------------------------------------------------------------------------*/

void arbol (registro *rama_ini, registro *rama_fin)

/* Esta funci¢n marca las ramas que forman parte del arbol (pone marca=1).
   Utiliza una tabla llamada nodos_usados que contiene una fila por cada
   nudo del circuito. Cada fila contiene los nudos con los cuales est 
   unido el nudo a que corresponde mediante ramas del arbol. De este modo,
   para saber si una rama forma lazo, se busca el nudo opuesto al que
   estemos considerando en la tabla, y si est , entonces la rama formar 
   lazo. Si no forma lazo, marcamos la rama y a¤adimos el nodo no encontr.
   a la fila del nodo que estemos procesando.
   N¢tese que no forman parte del arbol las ramas que contengan generadores
   de corriente, ya sean dependientes ¢ independientes. */

{
 registro *actual = NIL;
 unsigned n = 0, u, i, j, nodo = 0, nodos, nodo_escogido, *nodos_usados;
 int encontrado = 0;

 unsigned n_nodos (registro *rama_ini, registro *rama_fin);

 /* Limpiamos las 'marcas' de la lista de ramas */
 do
 {
  actual = (actual == NIL) ? rama_ini : actual->siguiente;
  actual->datos.marca = 0;
 }
 while (actual != rama_fin);

 nodos = n_nodos (rama_ini, rama_fin);          /* Obtenemos el n§ de nudos */

 /* Creamos la matriz de nodos usados */
 nodos_usados = (unsigned *) calloc (nodos, nodos * sizeof (unsigned));

 /* Comprobamos todas las ramas que confluyan en el nudo 0 */
 actual = NIL;
 do
 {
  /* Pasa a la siguiente rama que no contenga gen. de corriente */
  actual = (actual == NIL) ? rama_ini : actual->siguiente;

  if ((strchr (actual->datos.elem, 'D') == NIL) &&
      (strchr (actual->datos.elem, 'E') == NIL) &&
      (strchr (actual->datos.elem, 'I') == NIL))
   {
    if (actual->datos.nodoi == 0)                /* Si la rama apuntada por */
      nodo = actual->datos.nodof;                /* 'actual' va al nodo 0,  */
    else					 /* hacemos 'nodo' igual al */
      if (actual->datos.nodof == 0)              /* nudo opuesto, y si no,  */
	nodo = actual->datos.nodoi;              /* hacemos 'nodo' = 0.     */
      else
	nodo = 0;

    if (nodo != 0)                          /* Si hemos cogido alg£n nudo.. */
     {
      /* ..lo buscamos en la fila del nudo 0 */
      for (u = 0; (nu(0,u) != nodo) && (u < nodos); ++u);

      if (u >= nodos)                          /* Si no estaba en la fila.. */
       {
	nu(0,n) = nodo;                             /* ..lo ponemos en ella */
	++n;                                        /* y a¤adimos la rama   */
	actual->datos.marca = 1;                    /* al  rbol.            */
       }

      actual->datos.usada = 1; 			  /* La marcamos como usada */
     }
   }
 }
 while (actual != rama_fin);

 for (nodo = 0; nodo < nodos; ++nodo)  /* Bucle que recorre todos los nudos */
 {
  /* Bucle que recorre la fila n§ 'nodo' de la tabla de nodos usados */
  for (u = 0; (n = nu(nodo,u)) != 0; ++u)
  {
   actual = NIL;
   do  				       /* Bucle que recorre todas las ramas */
   {
    /* Pasa a la siguiente rama que no contenga gens. de corriente */
    actual = (actual == NIL) ? rama_ini : actual->siguiente;

    if ((strchr (actual->datos.elem, 'D') == NIL) &&
	(strchr (actual->datos.elem, 'E') == NIL) &&
	(strchr (actual->datos.elem, 'I') == NIL))

      if (actual->datos.usada == 0)
      {
       if (actual->datos.nodoi == n)             /* Si uno de los nudos de  */
	 nodo_escogido = actual->datos.nodof;    /* la rama actual coincide */
       else                                      /* con el nodo considerado */
	 if (actual->datos.nodof == n)           /* captura el otro extremo */
	   nodo_escogido = actual->datos.nodoi;  /* Si no, pone 'nodo_esc.' */
	 else                                    /* a 0.		    */
	   nodo_escogido = 0;

       if (nodo_escogido != 0)           /* Si se ha capturado alg£n nodo.. */
       {
	encontrado = 0;
	for (i = 0; (i < nodos) && (!encontrado); ++i)
	  for (j = 0; nu(i,j) != 0; ++j)         /* ..recorre la tabla  de  */
	    if (nodo_escogido == nu(i,j))        /* nudos usados hasta que  */
	     {                                   /* encuentra nodo_escogido */
	      encontrado = -1;                   /* o llega al final de la  */
	      break;                             /* tabla.                  */
	     }

	if (!encontrado)
	{                                        /* Si nodo_escogido no es- */
	 actual->datos.marca = 1;                /* t  en la tabla,marca la */
	 for (i = 0; nu(n,i) != 0; ++i);         /* rama como perteneciente */
	 nu(n,i) = nodo_escogido;                /* al arbol y mete  en  la */
	}                                        /* tabla  de  nodos_usados */
						 /* el nodo considerado.    */
	actual->datos.usada = 1;                /* Marca la rama como usada */
       }
      }
   }
   while (actual != rama_fin);
  }
 }

 return;
}

/*--------------------------------------------------------------------------*/

void limpia_lista (registro *rama_ini, registro *rama_fin)

/* Esta funci¢n pone a 0 las marcas de 'usada' de la lista de ramas */
{
 registro *actual = NIL;

 do
 {
  actual = (actual == NIL) ? rama_ini : actual->siguiente;
  actual->datos.usada = 0;
 }
 while (actual != rama_fin);
 return;
}

/*--------------------------------------------------------------------------*/

unsigned malla (registro *rama_ini, registro *rama_fin, registro *rama,
		unsigned salida, unsigned llegada, char elem[100])

/* Funci¢n recursiva que construye la malla formada por el arbol y la rama
   apuntada por el par metro rama. Es recursiva y se basa en un algoritmo
   de exploraci¢n de las estructuras tipo arbol. A partir de la rama se¤a-
   lada por 'rama', empieza en el nudo 'salida' y va explorando todos los
   caminos existentes que partan de ‚l. Uno de estos caminos tiene que
   llevar a 'llegada'. Cuando lo localiza, al ir volviendo de cada nivel
   de recursi¢n va a¤adiendo a 'elem' los elementos que estuviesen en la
   rama 'actual' del nivel.
   El sentido de recorrido supuesto es de 'nodoi' a 'nodof'. Si una rama se
   recorre en el sentido contrario, se antepone un signo "-" al elemento en
   la expresi¢n de la malla. Con este convenio, al construir la matriz Z
   completa, si los elementos comunes a dos elementos de la diagonal tienen
   signos diferentes, el elemento resultante deber  llevar signo negativo. */
{
 unsigned llegue = 0;
 registro *actual = NIL;

 do
 {
  actual = (actual == NIL) ? rama_ini : actual->siguiente;

  if ((actual->datos.marca != 0) &&
      (actual->datos.usada == 0) && (actual != rama))
  {
   if (actual->datos.nodoi == salida)
    if (actual->datos.nodof == llegada)
    {
     elem[0] = '(';
     strcat (elem, actual->datos.elem);
     strcat (elem, ")");
     actual->datos.usada = 1;
     return (1);
    }
    else
      llegue = malla (rama_ini, rama_fin, actual, actual->datos.nodof,
		      llegada, elem);
   else
    if (actual->datos.nodof == salida)
     if (actual->datos.nodoi == llegada)
      {
       elem[0] = '-';
       elem[1] = '(';
       strcat (elem, actual->datos.elem);
       strcat (elem, ")");
       actual->datos.usada = 1;
       return (1);
      }
      else
       llegue = malla (rama_ini, rama_fin, actual, actual->datos.nodoi,
		       llegada, elem);
  }

  if (llegue == 1)
  {
    if (actual->datos.nodof == salida)
      strcat (elem, "-");
    strcat (elem, "(");
    strcat (elem, actual->datos.elem);
    strcat (elem, ")");
    actual->datos.usada = 1;
    return (1);
  }
 }
 while (actual != rama_fin);

 rama->datos.usada = 1;
 return (0);
}

/*--------------------------------------------------------------------------*/

void matriz_Y (registro *rama_ini, registro *rama_fin)

/* Esta funci¢n construye la diagonal principal de la matriz Y del circuito
   y la almacena en un fichero de extensi¢n ".Y". El resto de elementos de
   la matriz puede obtenerse mediante los elementos comunes a dos elementos
   de la diagonal.
   El algoritmo se basa en que el elemento 'i' de la diagonal consta de todas
   las ramas que confluyen en el nodo 'i'. */
{
 registro *actual = NIL;
 char nom[13], Yii[100];
 unsigned n, maximo, nodo;
 int a;

 unsigned n_nodos  (registro *rama_ini, registro *rama_fin);
 unsigned max_nodo (registro *rama_ini, registro *rama_fin);

 /* Abrimos el fichero para escritura */
 strcpy (nom, nom_fich);
 *strrchr (nom, '.') = 0;
 strcat (nom, ".Y");
 Y = fopen (nom, "wb");

 /* Obtenemos el n§ de nudos del circuito */
 n = n_nodos (rama_ini, rama_fin);

 /* Obtenemos el n§ de ecs. (n§ de nudos - 1) y lo convertimos a string */
 itoa (n - 1, Yii, 10);

 /* Guarda el n§ de ecs. en el 1er registro del fichero */
 fwrite (Yii, sizeof (Yii), 1, Y);

 /* Obtenemos el m ximo n£mero de nudo del circuito */
 maximo = max_nodo (rama_ini, rama_fin);

 /* Bucle que pasa por todos los nodos uno por uno. Si los nudos est n sal-
   teados (su numeraci¢n no es cont¡nua), aquellos n£meros que no pertenecen
   a ning£n nudo resultar n en una Yii en blanco */
 for (nodo = 1; nodo <= maximo; ++nodo)
 {
  actual = NIL;  /* Inicializa la variable utilizada para recorrer la lista */

  for (a = 0; a < 100; ++a)                /* Inicializa Yii a cadena vac¡a */
    Yii[a] = 0;

  a = 0;        /* 'a' se utiliza como flag para poner un nudo en el buffer */

  /* Pasamos por todas las ramas buscando aquellas que confluyan en 'nodo' */
  do
  {
   actual = (actual == NIL) ? rama_ini : actual->siguiente;

   /* Si la rama actual est  unida con el nodo siendo procesado,
      se la a¤ade al elemento de la diagonal */
   if ((actual->datos.nodoi == nodo) || (actual->datos.nodof == nodo))
    {
     strcat (Yii, "(");
     strcat (Yii, actual->datos.elem);
     strcat (Yii, ")");
     if (actual->datos.marca != 0)   /* Si la rama contiene £nicamente un   */
       a = 1;                        /* gen. de tensi¢n, activamos 'a' para */
    }                                /* indicar que el nudo ir  al buffer.  */
  }
  while (actual != rama_fin);          /* Sigue hasta que no haya m s ramas */

  if (strlen (Yii) > 0)
    fwrite (Yii, sizeof (Yii), 1, Y);           /* Lo ponemos en el fichero */
 }

 fclose (Y);                                         /* Cerramos el fichero */
 return;
}

/*--------------------------------------------------------------------------*/

void recupera_lista_ramas (registro **rama_ini, registro **rama_fin)

/* Esta funci¢n crea la lista de ramas del circuito a partir del fichero
   creado por el compilador, o, si ya est  creada, la reinicializa a los
   valores almacenados en dicho fichero. */
{
 registro *puntero = NIL;
 FILE *circuito;
 rama aux;
 char nom[13];
 int lei;

 strcpy (nom, nom_fich);
 *strrchr (nom, '.') = 0;
 strcat (nom, ".CIR");
 circuito = fopen (nom, "rb");

 if (*rama_ini == NIL)
   *rama_ini = *rama_fin = (registro *) malloc (sizeof (registro));

 puntero = *rama_ini;

 fread (&puntero->datos, sizeof (rama), 1, circuito);

 while (feof (circuito) == 0)
 {
  lei = fread (&aux, sizeof (rama), 1, circuito);

  if (lei != 0)
   {
    if (puntero == *rama_fin)
      puntero = puntero->siguiente = *rama_fin =
				     (registro *) malloc (sizeof (registro));
    else
      puntero = puntero->siguiente;

    puntero->datos = aux;
   }
 }
 fclose (circuito);

 return;
}

/*---------------------------Fin del fichero--------------------------------*/
