/****************************************************************************
 *                                                                          *
 *           Fichero de cabecera para las matem ticas complejas             *
 *                                                                          *
 ****************************************************************************/

/*------------------------Definici¢n de constantes--------------------------*/

#define pi 3.141592654

/*--------------------------Definici¢n de tipos-----------------------------*/

typedef struct
	{
	 double Re, Im;
	} complejo;

typedef struct
	{
	 double mod, arg;
	} modarg;

/*-----------------Declaraci¢n de funciones contenidas----------------------*/

modarg polar (complejo Z);
complejo cart (modarg X);
complejo conj (complejo Z);
complejo sum (complejo A, complejo B);
complejo res (complejo A, complejo B);
complejo cmul (complejo A, complejo B);
complejo cdiv (complejo A, complejo B);
complejo cexp (complejo Z);
complejo elevar (complejo Z, double i);

/*---------------------------Fin del fichero--------------------------------*/
