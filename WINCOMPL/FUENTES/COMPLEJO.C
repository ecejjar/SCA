/****************************************************************************
 *                                                                          *
 *              Librer¡a de funciones matem ticas complejas                 *
 *                                                                          *
 ****************************************************************************/

#include <math.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDefinici¢n de constantesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

#define infinito 1.7E+308

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDefinici¢n de tiposÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

typedef struct
	{
	 double Re, Im;
	} complejo;

typedef struct
	{
	 double mod, arg;
	} modarg;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones contenidasÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

modarg polar (complejo Z);
complejo cart (modarg X);
complejo conj (complejo Z);
complejo sum (complejo A, complejo B);
complejo res (complejo A, complejo B);
complejo cmul (complejo A, complejo B);
complejo cdiv (complejo A, complejo B);
complejo cexp (complejo Z);
complejo elevar (complejo Z, double i);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

modarg polar (complejo Z)
/* Funci¢n que pasa de Re+jIm a |mod|arg */
{
 modarg X;
 struct complex z;

 z.x = Z.Re;
 z.y = Z.Im;

 X.mod = cabs (z);
 if (Z.Re == 0)
   X.arg = (Z.Im > 0) ? M_PI / 2 : -M_PI / 2;
 else
   X.arg = atan2 (Z.Im, Z.Re);

 return (X);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo cart (modarg X)
/* Funci¢n que pasa de |M|arg a Re+jIm */
{
 complejo Z;

 Z.Re = X.mod * cos (X.arg);
 Z.Im = X.mod * sin (X.arg);

 return (Z);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo conj (complejo Z)
/* Funci¢n que calcula el complejo conjugado de Z */
{
 Z.Im = -Z.Im;
 return (Z);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo sum (complejo A, complejo B)
/* Funci¢n que suma dos complejos */
{
 complejo Z;

 Z.Re = A.Re + B.Re;
 Z.Im = A.Im + B.Im;

 return (Z);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo res (complejo A, complejo B)
/* Funci¢n que resta dos complejos */
{
 complejo Z;

 Z.Re = A.Re - B.Re;
 Z.Im = A.Im - B.Im;

 return (Z);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo cmul (complejo A, complejo B)
/* Funci¢n que calcula el producto de dos complejos */
{
 complejo Z;

 Z.Re = A.Re * B.Re - A.Im * B.Im;
 Z.Im = A.Re * B.Im + A.Im * B.Re;

 return (Z);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo cdiv (complejo A, complejo B)
/* Funci¢n que calcula el cociente de dos complejos */
{
 complejo Z;
 double denom;

 denom = B.Re * B.Re + B.Im * B.Im;
 if (denom == 0)
  {
   Z.Re = (A.Re < 0) ? -infinito : infinito;
   Z.Im = (A.Im < 0) ? -infinito : infinito;
  }
 else
  {
   Z.Re = (A.Re * B.Re + A.Im * B.Im) / denom;
   Z.Im = (A.Im * B.Re - A.Re * B.Im) / denom;
  }

 return (Z);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo cexp (complejo Z)
/* Calcula la exponencial e^Z */
{
 complejo C;
 double coef;

 coef = exp (Z.Re);
 C.Re = coef * cos (Z.Im);
 C.Im = coef * sin (Z.Im);

 return (C);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo elevar (complejo Z, double i)
/* Eleva el complejo Z a la i-‚sima potencia */
{
 complejo s;
 double m, f;

 if (i == 0)
  {
   s.Re = 1;
   s.Im = 0;
  }
 else
   if ((Z.Re == 0) && (Z.Im == 0))
     s.Re = s.Im = 0;
   else
    {
     m = pow (sqrt (Z.Re*Z.Re + Z.Im*Z.Im), i);
     if (Z.Re == 0)
       f = (Z.Im > 0) ? i * M_PI / 2 : i * -M_PI / 2;
     else
       f = i * atan2 (Z.Im, Z.Re);
     s.Re = m * cos (f);
     s.Im = m * sin (f);
    }

 return (s);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
