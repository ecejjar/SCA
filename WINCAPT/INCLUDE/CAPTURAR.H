#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <io.h>
#include "polinom.h"

/* Definiciones generales */
#define PATH   "C:\\"
#define NOMBRE "SINNOMBR"
#define INFINITO 3.4e+38
#define UNION_INI 1
#define UNION_FIN 2
#define SUPERPOS_INI 1
#define SUPERPOS_FIN 2

/* N£mero m ximo de partes de los esquemas y diagramas */
#define MAX_BLOQUES   50
#define MAX_PUERTOS  100
#define MAX_TEXTOS   200
#define MAX_ELEMS    500
#define MAX_CABLES  1000
#define MAX_NUDOS   2000
#define MAX_CUAD      20

/* Definiciones para las extensiones de ficheros temporales */
#define TMPELEMS       5
#define TMPCUADS       6
#define TMPBLOQUES     7
#define TMPTEXTOS      8

/* Definiciones para los tipos de dibujo que se pueden hacer */
#define ELEMENTO       1
#define CABLE          2
#define NUDO           3
#define MASA           4
#define CUADRIPOLO     5
#define BORRAR         6
#define DESPLAZAR      7
#define MOVER          8
#define BLOQUE         9
#define TEXTO         10
#define PUERTO        11
#define GRUPO         12

/* Macros para acceder a los datos de los elementos */
#define e(i,j) *(elementos +  6*(i) + (j))
#define c(i,j) *(cables    +  6*(i) + (j))
#define n(i,j) *(nudos     +  3*(i) + (j))
#define C(i,j) *(cuad      +  5*(i) + (j))
#define b(i,j) *(bloques   +  4*(i) + (j))
#define t(i,j) *(textos    +  5*(i) + (j))
#define p(i,j) *(puertos   + 11*(i) + (j))

/* Tipo estructura para manejar InfoBloques ¢ InfoVentanas */
typedef struct tagInfoGeneral
	{
	 char   path[80], nom_fich[9];
	 BOOL   minimizada, cambios;
	 short  nVmaxScrl, nVposScrl, nHmaxScrl, nHposScrl, cyLine;
	 HANDLE hmem1, hmem2, hmem3, hmem4;
	 short  n1, n2, n3, n4;
	 HFONT  hNuevoFont;
	} InfoGeneral;

/* Tipos estructura relacionados con Diagramas de Bloques */
typedef struct tagInfoBloque
	{
	 char   path[80], nom_fich[9];
	 BOOL   minimizada, cambios;
	 short  nVmaxScrl, nVposScrl, nHmaxScrl, nHposScrl, cyLine;
	 HANDLE hmemBloques, hmemPuertos, hmemCables, hmemTextos;
	 short	n_bloques, n_puertos, n_cables, n_textos;
	} InfoBloque;

typedef struct
	{
	 char texto[51], fichero[84], metafile[84];
	 unsigned int alinh, alinv;
	} bloque;

typedef struct
	{
	 short x, y, horiz, vert, espejo;
	 char  nombre[12];
	} puerto;

typedef struct
	{
	 short longitud;
	 char *txt;
	} texto;

/* Tipos estructura relacionados con Esquemas */
typedef struct tagInfoVentana
	{
	 char   path[80], nom_fich[9];
	 BOOL   minimizada, cambios;
	 short  nVmaxScrl, nVposScrl, nHmaxScrl, nHposScrl, cyLine;
	 HANDLE hmemElems, hmemNudos, hmemCables, hmemCuad;
	 short  n_elems, n_nudos, n_cables, n_cuad;
	 HFONT  hNuevoFont;
	} InfoVentana;

typedef struct
	{
	 char  nombre[6], tipo[5];
	 float valor;
	 char  signal;
	 float fase;
	 float Tact;
	 float f[20];
	 float CT;
	 union
	 {
	  float cond_inic;
	  float frec;
	  unsigned nodos[2];
	  char elem[5];
	 } caract;
	} elemento;

typedef struct
	{
	 char nombre[11];
	 short tipo_parms;
	 func_trans Ti, Tr, Tf, To;          /* Par metros del cuadripolo */
	} cuadripolo;

typedef struct
	{
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

/* Tipos estructura relacionados con Bibliotecas */
typedef struct tagInfoBiblio
	{
	 char   path[80], nom_fich[9];
	 BOOL   minimizada, cambios;
	 short  nVmaxScrl, nVposScrl, nHmaxScrl, nHposScrl, nAnchoMax;
	 HFILE  hIndice;
	 short  nPos, n_elems;
	} InfoBiblio;


