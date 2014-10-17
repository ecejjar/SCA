/****************************************************************************
 *                                                                          *
 *     Fichero de cabecera para polinomios y funciones de transferencia     *
 *                                                                          *
 ****************************************************************************/

#include <complejo.h>

/*------------------------Definici¢n de constantes--------------------------*/

#define SUMA 1
#define RESTA -1
#define precision 1E-8

/*---------------------------Definici¢n de tipos----------------------------*/

typedef struct
	{
	 int grado;
	 float coef[11];
	} polinomio;

typedef struct
	{
	 int grado;
	 double coef[11];
	} super_polinomio;

typedef struct
	{
	 polinomio num, den;
	} func_trans;

typedef struct
	{
	 unsigned numero;
	 complejo valor[11];
	} raices_pol;

typedef struct
	{
	 unsigned numero;
	 struct {complejo num, den;} fraccion[11];
	} fracc_simples;

/*-------------------Declaraci¢n de variables globales----------------------*/

// extern func_trans *t;

/*------------------Declaraci¢n de funciones contenidas---------------------*/

int cero (polinomio p);
polinomio operar (int operador, polinomio p, polinomio q);
polinomio          multiplicar (polinomio p, polinomio q);
polinomio              dividir (polinomio p, polinomio q, polinomio *r);
polinomio          derivar_pol (polinomio p);
complejo 	      pol_en_z (polinomio p, complejo z);
double 		      pol_en_x (polinomio p, double x);
raices_pol              raices (polinomio p);
func_trans operar_func (int operador, func_trans A, func_trans B);
func_trans                 mult_func (func_trans A, func_trans B);
func_trans 		    div_func (func_trans A, func_trans B);
func_trans 		derivar_func (func_trans G);
void 			     reducir (func_trans *G);
complejo 		   func_en_z (func_trans G, complejo z);
fracc_simples            descomponer (func_trans G);

/*----------------------------Fin del fichero-------------------------------*/

