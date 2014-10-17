#include <windows.h>
#include <winresol.h>
#include <resolver.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de variables globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern HWND  	hwnd, hwndHija, hwndBotonFin, hwndBotonAbort;
extern long     mem_global;
extern unsigned mem_local;
extern int      analisis;
extern short    Fin, Abort;
extern char  	nom_fich[80];
extern BOOL  	hay_fichero;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern void error (int numero);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void Analisis (void)

/* Lleva a cabo todos los an lisis pedidos al circuito */
{
 elemento *lista_elems = NIL;
 rama *lista_ramas = NIL;
 char (*ecs_Z)[100] = NIL, (*ecs_Y)[100] = NIL;
 unsigned n_elems, n_ramas, n_ecs_Z, n_ecs_Y;

	       /* Declaraciones de funciones utilizadas */

 unsigned recupera_lista_elems (elemento **elementos);
 unsigned recupera_lista_ramas (rama **ramas);
 unsigned recupera_matriz      (char tipo, char (**ecs)[100]);
 unsigned recupera_respuestas  (char tipo, resultado **respuestas);

 extern void punto_de_trabajo   (unsigned n_elems, elemento *elems,
				 unsigned n_ramas, rama *ramas,
				 unsigned n_ecs_Z, char (*ecs_Z)[100],
				 unsigned n_ecs_Y, char (*ecs_Y)[100]);

 extern void resp_en_el_tiempo  (unsigned n_elems, elemento *elems,
				 unsigned n_ramas, rama *ramas,
				 unsigned n_ecs_Z, char (*ecs_Z)[100],
				 unsigned n_ecs_Y, char (*ecs_Y)[100]);

 extern void resp_en_frecuencia (char gen_ind[5],
				 unsigned n_elems, elemento *elems,
				 unsigned n_ramas, rama *ramas,
				 unsigned n_ecs_Z, char (*ecs_Z)[100],
				 unsigned n_ecs_Y, char (*ecs_Y)[100]);

 extern void sensibilidad       (unsigned n_elems, elemento *elems,
				 unsigned n_ramas, rama *ramas,
				 unsigned n_ecs_Z, char (*ecs_Z)[100],
				 unsigned n_ecs_Y, char (*ecs_Y)[100]);

 /* Recuperamos todos los datos necesarios de los ficheros */
 if ((n_elems = recupera_lista_elems (&lista_elems)) == 0)
   error (NOELEMS);
 else
   if ((n_ramas = recupera_lista_ramas (&lista_ramas)) == 0)
     error (NORAMAS);
   else
     if (((n_ecs_Z = recupera_matriz ('Z', &ecs_Z)) == 0) ||
	 ((n_ecs_Y = recupera_matriz ('Y', &ecs_Y)) == 0))
       error (NOECS);
     else
      {
       /* Fijamos modo de an lisis permanente */
       analisis = PERM;

       /* Calculamos el punto de trabajo del circuito */
       if (!Abort)
	 punto_de_trabajo (n_elems, lista_elems, n_ramas, lista_ramas,
			   n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

       if (!Abort)
	 resp_en_el_tiempo (n_elems, lista_elems, n_ramas, lista_ramas,
			    n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

       if (!Abort)
	 resp_en_frecuencia ("V001\x0", n_elems, lista_elems,
			     n_ramas, lista_ramas, n_ecs_Z, ecs_Z,
			     n_ecs_Y, ecs_Y);

       if (!Abort)
	 sensibilidad (n_elems, lista_elems, n_ramas, lista_ramas,
		       n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);
      }

 mem_local -= LocalSize (LocalHandle ((WORD) ecs_Z)) +
	      LocalSize (LocalHandle ((WORD) ecs_Y)) +
	      LocalSize (LocalHandle ((WORD) lista_ramas)) +
	      LocalSize (LocalHandle ((WORD) lista_elems));
 free (ecs_Z);
 free (ecs_Y);
 free (lista_ramas);
 free (lista_elems);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

unsigned recupera_lista_elems (elemento **elementos)

/* Esta funci¢n crea la lista de elementos del circuito a partir del fichero
   generado por el compilador, o, si ya est  creada, la reinicializa a los
   valores almacenados en dicho fichero. Devuelve el n§ de elementos del
   circuito */
{
 FILE *elems;
 elemento aux;
 char nom[84];
 unsigned i, n_elems;

 /* Abrimos el fichero para lectura */
 strcpy (nom, nom_fich);
 strcat (nom, ".ELM");
 if ((elems = fopen (nom, "rb")) == 0)
  {
   error (FICHNOEXIS);
   return (0);
  }

 /* Obtenemos el n§ de elementos del circuito dividiendo la longitud del
    mismo entre la longitud de una variable tipo elemento */
 n_elems = (unsigned) filelength (fileno (elems)) / sizeof (elemento);

 /* Reservamos el espacio necesario */
 if (*elementos == NIL)
  {
   *elementos = (elemento *) calloc (n_elems, sizeof (elemento));
   mem_local += LocalSize (LocalHandle ((WORD) *elementos));
  }

 /* Leemos los elementos del fichero */
 for (i = 0; i < n_elems; ++i)
   fread (*elementos + i, sizeof (elemento), 1, elems);

 /* Cerramos el fichero */
 fclose (elems);

 return (n_elems);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

unsigned recupera_lista_ramas (rama **lista_ramas)

/* Esta funci¢n crea la lista de ramas del circuito a partir del fichero
   creado por el compilador, o, si ya est  creada, la reinicializa a los
   valores almacenados en dicho fichero. Devuelve el n§ de ramas del cir-
   cuito */
{
 FILE *circuito;
 rama aux;
 char nom[84];
 unsigned i, n_ramas;

 /* Abrimos el fichero de ramas */
 strcpy (nom, nom_fich);
 strcat (nom, ".CIR");
 if ((circuito = fopen (nom, "rb")) == 0)
  {
   error (FICHNOEXIS);
   return (0);
  }

 /* Obtenemos el n§ de ramas del circuito dividiendo la longitud del mismo
    entre la longitud de una variable tipo rama */
 n_ramas = (unsigned) filelength (fileno (circuito)) / sizeof (rama);

 /* Reservamos el espacio necesario */
 if (*lista_ramas == NIL)
  {
   *lista_ramas = (rama *) calloc (n_ramas, sizeof (rama));
   mem_local += LocalSize (LocalHandle ((WORD) *lista_ramas));
  }

 /* Leemos las ramas del fichero */
 for (i = 0; i < n_ramas; ++i)
   fread (*lista_ramas + i, sizeof (rama), 1, circuito);

 fclose (circuito);

 return (n_ramas);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

unsigned recupera_respuestas (char tipo, resultado **respuestas)

/* Esta funci¢n crea la lista de respuestas de tipo 'tipo' requeridas del
   circuito a partir del fichero creado por el compilador, o, si ya est 
   creada, la reinicializa a los valores almacenados en dicho fichero.
   Devuelve el n§ de respuestas */
{
 FILE *resp;
 resultado aux;
 char nom[84];
 unsigned i, n_resp = 0;

 /* Abrimos el fichero de ramas */
 strcpy (nom, nom_fich);
 strcat (nom, ".RES");
 if ((resp = fopen (nom, "rb")) == 0)
  {
   error (FICHNOEXIS);
   return (0);
  }

 /* Obtenemos el n§ de respuestas de tipo 'tipo' dando una pasada al fichero */
 do
 {
  i = fread (&aux, sizeof (resultado), 1, resp);
  if (i > 0)
    if (aux.dominio == tipo)
      ++n_resp;
 }
 while (feof (resp) == 0);

 /* Tiramos las respuestas anteriores y reservamos el espacio necesario */
 if (*respuestas != NIL)
   free (*respuestas);
 *respuestas = (resultado *) calloc (n_resp, sizeof (resultado));

 /* Leemos las respuestas del fichero */
 fseek (resp, 0, SEEK_SET);
 for (i = 0; i < n_resp; ++i)
  {
   do
   {
    fread (&aux, sizeof (resultado), 1, resp);
   }
   while (aux.dominio != tipo);

   *(*respuestas + i) = aux;
  }

 fclose (resp);

 return (n_resp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

unsigned recupera_matriz (char tipo, char (**ecs)[100])

/* Funci¢n que lee del fichero correspondiente la diagonal de la matriz Z ¢
   Y, seg£n se especifique en 'tipo', y devuelve el n§ de elementos de la
   misma. */
{
 FILE *matriz;
 char nom[84], ext[4], aux[100];
 unsigned i, n_ecs;

 ext[0] = '.';                         /* Formamos la extensi¢n del fichero */
 ext[1] = tipo;                        /* seg£n lo especificado en 'tipo'.  */
 ext[2] = 0;

 /* Abrimos el fichero para lectura */
 strcpy (nom, nom_fich);
 strcat (nom, ext);
 if ((matriz = fopen (nom, "rb")) == 0)
  {
   error (FICHNOEXIS);
   return (0);
  }

 /* En el 1er registro se encuentra el n§ de ecs. */
 fread (aux, sizeof (aux), 1, matriz);
 n_ecs = atoi (aux);

 /* Reservamos el espacio necesario */
 if (*ecs == NIL)
  {
   *ecs = (char (*)[100]) calloc (n_ecs, sizeof (aux));
   mem_local += LocalSize (LocalHandle ((WORD) *ecs));
  }

 /* Leemos las ecs. del fichero */
 for (i = 0; i < n_ecs; ++i)
   if (fread (*ecs + i, sizeof (aux), 1, matriz) == 0)
    {
     error (FICHINCOMP);
     n_ecs = 0;
     break;
    }

 /* Cerramos el fichero */
 fclose (matriz);

 return (n_ecs);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/