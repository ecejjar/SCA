#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <io.h>
#include <math.h>
#include <polinom.h>

/*컴컴컴컴컴컴컴컴컴컴컴횯efinici줻 de constantes컴컴컴컴컴컴컴컴컴컴컴컴컴�*/

#define NIL 0
#define PATH "C:\\BORLANDC\\BIN\\BCWORK\\CIRCUITO\\"
#define NOMBRE "CIRCUITO"
#define LIN 0
#define LOG 1

#define AVANZAR    1
#define NO_AVANZAR 0

/* Indices a los WORDS de la estructura de clase de ventana de gr쟣ica */
#define TIPO        0
#define GRAFICA     2
#define TABLA       4
#define NDIVSX      6
#define NDIVSY      8
#define TRACKING   10
#define COORDS     12
#define COLORESC   14
#define COLORGRF   16
#define HWNDCOORDS 18

/* Identificadores para los colores puros */
#define ROJO        1
#define VERDE       2
#define AZUL        4

/*컴컴컴컴컴컴컴컴컴컴컴컴컴Definici줻 de tipos컴컴컴컴컴컴컴컴컴컴컴컴컴컴�*/

typedef struct
	 {
	  char escala;
	  float ini, fin;
	  unsigned n_puntos;
	 }
	datos_grafica;

typedef struct
	 {
	  float min, max, escalax, escalay;
	 }
	extremos_grafica;

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
	} respuesta;
