#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dir.h>
#include "polinom.h"

/*---------------------Zona de definici¢n de constantes---------------------*/

#define NIL 0
#define PATH "C:\\"
#define NOMBRE "CIRCUITO"
#define INFINITO 3.4e+38

#define ERRORES          1
#define NO_ERRORES       0

#define AVANZAR          1
#define NO_AVANZAR       0

#define W_NOMMUYLARGO    0
#define W_MEDMUYLARGA    1
#define W_ELEMNOCONEC    2
#define W_GENSINELEM     3
#define W_NOHAYNUDOREF   4
#define W_MUCHOSTERMINOS 5
#define W_GRADOMAYOR10   6

#define SCRLDESPL 	 5

#define E_TIPODESCONOC   25
#define E_CARINESP       26
#define E_PARAMDESCONOC  27
#define E_TERMDESCONOC   28
#define E_ELEMNOENCON    29
#define E_FICHNOENCON    30
#define E_ESCALANOVALIDA 31
#define E_FINDEFICHERO   32
#define E_MEDIDADESCONOC 33
#define E_FINDESECCION   34
#define E_DEFINDUPL      35

/*-----------------------Zona de definici¢n de macros-----------------------*/

#define nu(i,j) *(nodos_usados + nodos*(i) + (j))

/*-----------------------Zona de definici¢n de tipos------------------------*/

typedef struct            /* Estructura que define un elemento del circuito */
	{
	 char nombre[6], tipo[5];  /* En 'nombre' se almacena el nombre que */
	 float valor;              /* le da el usuario, y en 'tipo' se pone */
	 char signal;              /* una letra indicativa y un n§ de orden */
	 float fase;               /* 'fase' indica el retardo de la se¤al  */
	 float Tact;               /* Indica el tiempo de actividad del gen */
	 polinomio f;              /* Si es != 0 define la alinealidad      */
	 float CT;                 /* Representa el coeficiente de temper.  */
	 union                            /* 'signal' determina el tipo de  */
	 {                                /* se¤al que genera un generador. */
	  float cond_inic;        /* Si es elem pasivo -> condici¢n inicial */
	  float frec;             /* Si es un gen. indep. -> frecuencia     */
	  float capacidad;        /* Si es un diodo -> capac. de la uni¢n   */
	  unsigned nodos[2];      /* Si es dep. de V -> nodos de medida     */
	  char elem[5];           /* Si es dep. de I -> elemento de medida  */
	 } caract;
	} elemento;

/* Los tipos de elementos permitidos son    Los tipos de se¤al posibles son */
/* R = Resistencia                          P = Pulso                       */
/* L = Inductancia                          R = Rampa                       */
/* C = Capacidad                            T = Par bola                    */
/* V = Gen. independiente de tensi¢n        S = Senoidal                    */
/* I = Gen. independiente de corriente      F = Fichero ".SIG"              */
/* A = Gen. de tensi¢n dep. de tensi¢n      D = definida por 'polinomio'    */
/* B = Gen. de tensi¢n dep. de corriente    C = Generador de c.c.           */
/* D = Gen. de corriente dep. de tensi¢n                                    */
/* E = Gen. de corriente dep. de corriente				    */
/* Q = Diodo semiconductor						    */
/* P = Transistor bipolar PNP						    */
/* N = Transistor bipolar NPN						    */
/* J = Transistor JFET canal P 						    */
/* K = Transistor JFET canal N */
/* M = Transistor MOSFET canal P */
/* O = Transistor MOSFET canal N */
/* Z = Cuadripolo en par metros 'Z' */

typedef struct lista_elems
	{
	 elemento datos;
	 struct lista_elems *siguiente;
	} reg_elem;

typedef struct
	{
	 char elem[50];
	 unsigned nodoi, nodof, marca, usada;
	} rama;

typedef struct lista_rama
	{
	 rama datos;
	 struct lista_rama *siguiente;
	} registro;

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

typedef struct lista_medidas
	{
	 respuesta datos;
	 struct lista_medidas *siguiente;
	} reg_medida;

typedef struct
	{
	 char nombre[11];
	 char tipo[5];
	 func_trans Ti, Tr, Tf, To;
	} cuadripolo;

typedef struct lista_cuadri
	{
	 cuadripolo datos;
	 struct lista_cuadri *siguiente;
	} reg_cuadri;
