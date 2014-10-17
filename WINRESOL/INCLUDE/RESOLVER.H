#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mem.h>
#include <time.h>
#include <polinom.h>

/*컴컴컴컴컴컴컴컴컴컴컴컴Definici줻 de constantes컴컴컴컴컴컴컴컴컴컴컴컴컴*/

#define NIL        0
#define PATH       "C:\\BORLANDC\\BIN\\BCWORK\\CIRCUITO\\"
#define NOMBRE	   "CIRCUITO"   /* Nombre por defecto de todos los ficheros */
#define SCRLDESPL  5          /* Desplazamiento en pixels por cada scroll */
#define PERM       0
#define TRAN       1
#define NUMITERS   200    /* N즡ero de iteraciones para el m굏odo de Newton */
#define MAXERR     0.05            /* Error m쟸imo en la respuesta temporal */
#define INFINITO   3.4e+38                       /* M쟸imo n즡ero permitido */
#define K          1.3804e-23                         /* Cte. de Boltzmann  */
#define qe         1.6021e-19                         /* Carga del electr줻 */
#define A_INICIAL  0
#define A_MALLAS   1
#define A_NUDOS    2
#define FIN_NU     65535          /* Marca el fin de tabla de nudos usados  */
#define I_MIN      1e-20          /* M죒ima corriente considerada como <> 0 */

/* Constantes correspondientes a los errores */
#define FICHNOEXIS   0
#define FICHINCOMP   1
#define NOSUFMEM     2
#define DESBORDAM    3
#define NOELEMS      4
#define NORAMAS      5
#define NOECS        6
#define NORESP       7
#define NOCONDINIC   8


/*컴컴컴컴컴컴컴컴컴컴컴컴컴Definici줻 de macros컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/

#define y(f,c) *(y + (f)*nv + c)

/*컴컴컴컴컴컴컴컴컴컴컴컴컴횯efinici줻 de tipos컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/

typedef struct            /* Estructura que define un elemento del circuito */
	{
	 char nombre[6], tipo[5];  /* En 'nombre' se almacena el nombre que */
	 float valor;              /* le da el usuario, y en 'tipo' se pone */
	 char signal;              /* una letra indicativa y un n� de orden */
	 float fase;               /* 'fase' indica el retardo de la se쨅l  */
	 float Tact;               /* Indica el tiempo de actividad del gen */
	 polinomio f;              /* Si no es 0 define la alinealidad      */
	 float CT;                 /* Representa el coeficiente de temper.  */
	 union                            /* 'signal' determina el tipo de  */
	 {                                /* se쨅l que genera un generador. */
	  float cond_inic;        /* Si es elem pasivo -> condici줻 inicial */
	  float frec;             /* Si es un gen. indep. -> frecuencia     */
	  unsigned nodos[2];      /* Si es dep. de V -> nodos de medida     */
	  char elem[5];           /* Si es dep. de I -> elemento de medida  */
	 } caract;
	} elemento;

typedef struct                   /* Estructura que define una rama con los  */
	{                        /* elems. que contiene y los nodos que une */
	 char elem[50];
	 unsigned nodoi, nodof, marca, usada;
	} rama;

typedef struct
	{
	 char tipo, dominio, escala;
	 float ini, fin, temp;
	 unsigned pasos;
	 union
	 {
	  unsigned nodos[2];
	  char elem[5];
	 } medida;
	} resultado;

typedef struct
	{
	 double coef[3];
	 unsigned puntero;
	} gen;
