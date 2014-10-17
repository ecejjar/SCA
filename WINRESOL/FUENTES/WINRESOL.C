#include <windows.h>
#include <resolver.h>
#include <winresol.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern long FAR PASCAL _export ProcVent 	(HWND, WORD, WORD, LONG);
extern long FAR PASCAL _export ProcBarraPorcent (HWND, WORD, WORD, LONG);

int           diagonaliza_comp (unsigned orden);
int           diagonaliza_real (unsigned orden, unsigned max_orden);
void          error (int numero);
double huge   *C (unsigned i, unsigned j);
complejo huge *M (unsigned i, unsigned j);
double huge   *RLC (double huge *puntero, unsigned i, unsigned j);
gen           (*G(unsigned i))[];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDefinici¢n de variables globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

extern gen (*g)[10];

HWND     hwnd; 				/* Handle a la ventana del programa */

HANDLE   hp;                          /* Handle al bloque de memoria global */
complejo huge *p;                /* Apunta al elemento (0,0) de (Zp) o (Yp) */

HANDLE   hr, hw;
double   huge *r, huge *w;      	 /* Apuntan a R y J respectivamente */
double	 Tnom = 300,              /* Temperatura nominal del circuito en §K */
	 Tact = 300,           		    /* Temperatura real de an lisis */
	 Vt = K * 300 / qe;        		         /* Tensi¢n t‚rmica */

long     mem_global = 0;	      /* Cantidad de memoria global ocupada */
unsigned mem_local = 0,	       	       /* Cantidad de memoria local ocupada */
	 *fg,  			         /* Apunta a la tabla de gens. dep. */
	 orden,                     /* orden contiene el orden de la matriz */
	 gendep,                    /* Contienen el n§ de gens. dep. total, */
	 gendepv = 0, gendepi = 0,  /* de V y de I repectivamente.          */
	 diodos = 0;                /* Contiene el n§ de diodos del circ.   */

int analisis;    /* Si vale 0 estamos en modo permanente, si no en transit. */

short    cyLine,		       /* Altura de una l¡nea de la ventana */
	 nHposScrl, nVposScrl,        /* Posiciones de las barras de scroll */
	 TieneFoco = BOTONFIN; /* ID del boton que tiene el foco de entrada */

char     szAppName[] = "Resolver",
	 szChildClass[] = "Porcentaje",    /* Nombre de la clase de la hija */
	 nom_fich[80],  	/* Nombre del fichero con el que trabajamos */
	 frase1[21], frase2[28], frase3[32], frase4[38], frase5[5];

BOOL     hay_fichero = FALSE;     /* Indica si hemos abierto alg£n circuito */

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo del programa principalÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance,
		    LPSTR lpszCmdLine, int nCmdShow)
{
		 /* Definiciones de variables locales */

 WNDCLASS    wndclass;
 MSG         msg;
 FILE        *config;

 /* Tomamos el path del circuito */
 if (config = fopen ("config.sca", "rt"))
  {
   fscanf (config, "%[^\n]", nom_fich);
   fclose (config);
  }
 else
   strcpy (nom_fich, PATH);

 /* Si la l¡nea de comando existe, se toma */
 if (lstrlen (lpszCmdLine) > 0)
  {
   lstrcat (nom_fich, lpszCmdLine);
   hay_fichero = TRUE;
  }

 /* Registramos la clase de la ventana principal y la de porcentaje */
 if (!hPrevInstance)
  {
   wndclass.style	  = CS_HREDRAW | CS_VREDRAW;
   wndclass.lpfnWndProc   = ProcVent;
   wndclass.cbClsExtra    = 0;
   wndclass.cbWndExtra    = 0;
   wndclass.hInstance	  = hInstance;
   wndclass.hIcon	  = NULL;
   wndclass.hCursor	  = LoadCursor (NULL, IDC_ARROW);
   wndclass.hbrBackground = GetStockObject (WHITE_BRUSH);
   wndclass.lpszMenuName  = NULL;
   wndclass.lpszClassName = szAppName;

   RegisterClass (&wndclass);

   wndclass.lpfnWndProc   = ProcBarraPorcent;
   wndclass.cbClsExtra    = sizeof (WORD);
   wndclass.hIcon         = NULL;
   wndclass.lpszClassName = szChildClass;

   RegisterClass (&wndclass);
  }

 /* Creamos la ventana */
 srand (hInstance);
 hwnd = CreateWindow (szAppName, "Resolutor S.C.A. I",
		      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |
		      WS_HSCROLL | WS_VSCROLL,
		      random (GetSystemMetrics(SM_CXSCREEN) - 300),
		      random (GetSystemMetrics(SM_CYSCREEN) - 300),
		      308, 293, NULL, NULL, hInstance, NULL);
 ShowWindow   (hwnd, nCmdShow);
 UpdateWindow (hwnd);

 /* Estamos en el bucle de mensaje hasta que se escoja 'Salir'  */
 while (GetMessage (&msg, NULL, 0, 0))
 {
  TranslateMessage (&msg);
  DispatchMessage (&msg);
 }

 return (msg.wParam);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double valor_gen (unsigned i, double var, double var1,
		  double t, double h, float inc,
		  unsigned deriv, elemento *elems)

/* Esta funci¢n calcula el valor en el instante t + h*inc del generador 'i',
   o bien de su derivada de grado 'deriv'-‚simo, si 'deriv' es distinto de 0.
   Si el generador es dependiente, se calcula f(var, var1), siendo f la funci¢n que
   describe la respuesta del generador y var la variable de la que depende */
{
 double v = 0, T, Ti = 0, w, tr0, tr1, td0, td1, t_real;

 double argsenh (double);

 /* Si el generador es independiente... */
 if ((elems[i].tipo[0] == 'V') || (elems[i].tipo[0] == 'I'))
  {
   /* ...calculamos el instante de muestreo real y el instante referido a la
      fase 0 */
   t += h*inc;
   t_real = t;
   t -= elems[i].fase;

   /* Si no ha pasado el tiempo de retardo, el valor del gen. es nulo */
   if (t < 0)
     return (0);

   /* Si la se¤al es peri¢dica... */
   if (elems[i].caract.frec != 0)
    {
     /* ...obtenemos el periodo */
     T = 1 / elems[i].caract.frec;

     /* Obtenemos en qu‚ periodo nos hallamos (si se¤al no peri¢dica, Ti = 0) */
     Ti = floor (t / T);

     /* Obtenemos el instante referido al instante de comienzo del periodo */
     t = fmod (t, T);
    }

   /* Si ha pasado el tiempo de actividad, el valor del generador es nulo */
   if ((t - h*inc) > elems[i].Tact)
     return (0);

   /* pero, si no ha pasado, comprobamos si estamos en el flanco de ca¡da */
   else
    {
     /* Calculamos el comienzo y fin de la rampa de bajada */
     td0 = h * (floor ((elems[i].fase + T*Ti + elems[i].Tact) / h) + 0.25);
     td1 = td0 + 0.5*h;
    }

   /* Evaluamos el valor del generador seg£n su tipo de se¤al */
   switch (elems[i].signal)
   {
    case 'C':
    {
     /* Si es una se¤al continua, sus derivadas son 0 */
     v = (deriv == 0) ? elems[i].valor : 0;
     break;
    }

    case 'S':
    {
     /* Si es un seno, derivarlo es retrasarle 1/4 de periodo y multiplicar
	por w */
     w = 2*pi / T;
     if (deriv == 0)
       v = elems[i].valor * sin (w*t);
     else
       if (deriv == 1)
	{
	 /* Si estamos en el flanco de ca¡da, la derivada ser  una delta */
	 if ((t_real >= td0) && (t_real <= td1))
	   v = -elems[i].valor * sin (w*t) / h * 6/4;
	 else
	   v = elems[i].valor * w * cos (w*t);
	}
       else
	{
	 /* La f'', en el instante de comienzo de la se¤al, ser  una delta */
	 tr0 = h * (ceil ((elems[i].fase + T*Ti) / h) + 0.25);
	 tr1 = tr0 + 0.5*h;
	 if ((Ti == 0) && (t_real >= tr0) && (t_real <= tr1))
	   v = elems[i].valor * w / h * 6/4;
	 else
	   v = -elems[i].valor * w * w * sin (w*t);
	}
     break;
    }

    case 'P':
    {
     /* Si es un pulso, aproximamos por ñ(valor / h) y ñ(valor / h^2) */
     if (deriv == 0)
       v = elems[i].valor;
     else
      {
       /* Calculamos los instantes de comienzo y fin de la rampa de subida,
	  de modo que s¢lo se detecte en la mitad del intervalo */
       tr0 = h * (ceil ((elems[i].fase + T*Ti) / h) + 0.25);
       tr1 = tr0 + 0.5*h;

       /* Si estamos dentro de dicho intervalo, calculamos la aproximaci¢n */
       if ((t_real >= tr0) && (t_real <= tr1))
	 v = (deriv == 1) ? elems[i].valor/h * 6/4:
			    elems[i].valor/(h*h) * 6/4;

       /* pero si no estamos en la rampa de subida.. */
       else
	{
	 /* ..entonces comprobamos si estamos en la rampa de bajada */
	 if ((t_real >= td0) && (t_real <= td1))
	   v = (deriv == 1) ? -elems[i].valor/h * 6/4 :
			      -elems[i].valor / (h*h) * 6/4;

	 /* pero, si tampoco estamos en la rampa de bajada.. */
	 else

	   /* ..entonces la derivada valdr  0 */
	   v = 0;
	}
      }
     break;
    }

    case 'R':
    {
     /* Si es una rampa, la derivada ser  un escal¢n */
     if (deriv == 0)
       v = elems[i].valor * t;
     else
       if (deriv == 1)
	{
	 /* Si estamos en el flanco de ca¡da, la derivada es una delta */
	 if ((t_real >= td0) && (t_real <= td1))
	   v = -elems[i].valor * t / h * 6/4;
	 else
	   v = elems[i].valor;
	}
       else
	{
	 /* Calculamos los instantes de inicio y fin de la rampa de subida */
	 tr0 = h * (ceil ((elems[i].fase + T*Ti) / h) + 0.25);
	 tr1 = tr0 + 0.5*h;

	 /* Si estamos dentro de dicho intervalo calculamos la aproximaci¢n */
	 if ((t_real >= tr0) && (t_real <= tr1))
	   v = elems[i].valor/h * 6/4;

	 /* pero si no estamos en la rampa de subida.. */
	 else
	  {
	   /* ..comprobamos si estamos dentro de la rampa de bajada */
	   if ((t_real >= td0) && (t_real <= td1))
	     v = -elems[i].valor/h * 6/4;

	   /* pero, si tampoco estamos en la rampa de bajada.. */
	   else

	     /* ..entonces la derivada valdr  0 */
	     v = 0;
	  }
	}
     break;
    }

    case 'T':
    {
     /* Si es una par bola, la derivada ser  la rampa */
     if (deriv == 0)
       v = elems[i].valor * t * t;
     else
       if (deriv == 1)
	{
	 /* Si estamos en el flanco de ca¡da, la derivada ser  una delta */
	 if ((t_real >= td0) && (t_real <= td1))
	   v = -elems[i].valor * (t*t) / h * 6/4;
	 else
	   v = elems[i].valor * t;
	}
       else
	 v = elems[i].valor;
     break;
    }

    case 'D':
    {
     /* Si es un polinomio, las derivadas son inmediatas */
     if (deriv == 0)
       v = elems[i].valor * pol_en_x (elems[i].f, t);
     else
       if (deriv == 1)
	{
	 /* Si estamos en el flanco de ca¡da, la derivada ser  una delta */
	 if ((t_real >= td0) && (t_real <= td1))
	   v = -elems[i].valor * pol_en_x (elems[i].f, t) / h * 6/4;
	 else
	   v = elems[i].valor * pol_en_x (derivar_pol(elems[i].f), t);
	}
       else
	 v = elems[i].valor * pol_en_x (derivar_pol(derivar_pol(elems[i].f)), t);
     break;
    }
   }
  }

 /* pero, si es dependiente... */
 else
  {
   /* ...calculamos el valor de la funci¢n para el valor de la variable */
   switch (elems[i].signal)
   {
    case 'C':
    {
     /* Si el generador es del tipo Kúx, tendremos Kúx' y Kúx'' */
     if (deriv == 0)
       v = var;
     else
       v = 1;

     break;
    }

    case 'D':
    {
     /* Si el generador es del tipo P(x), tendremos P'(x)úx' y P''(x)úx'' */
     if (deriv == 0)
       v = pol_en_x (elems[i].f, var);
     else
       if (deriv == 1)
	 v = pol_en_x (derivar_pol(elems[i].f), var);
       else
	 v = pol_en_x (derivar_pol(derivar_pol(elems[i].f)), var);
     break;
    }

    case 'S':
    {
     /* Si el generador es del tipo ec. de Shockley, tendremos; */
     if (var > 2)
       var = 2;
					    /* Limitamos el  valor  de  la  */
     w = elems[i].f.coef[2];                /* var. depend. para evitar los */
     if (var < -(w + 2))                    /* posibles overflows al iterar */
       var = -(w + 2);

     if (deriv == 0)
       v = exp (var / Vt) - 1 -
	   elems[i].f.coef[3] / elems[i].valor * exp (-(var+w) / Vt);
     else
       if (deriv == 1)
	 v = (exp (var / Vt) +
	      elems[i].f.coef[3] / elems[i].valor * exp (-(var+w) / Vt)) / Vt;
       else
	 v = (exp (var / Vt) -
	      elems[i].f.coef[3] / elems[i].valor * exp (-(var+w) / Vt)) / (Vt * Vt);
     break;
    }

    case 'I':
    {
     #define IDS elems[i].valor
     #define VBR elems[i].f.coef[2]
     #define IBV elems[i].f.coef[3]

     /* Si el generador es tipo Shockley^-1, tendremos:
	Para Vbr < 35 v.; */
     if (VBR < 35)
      {
       if (deriv == 0)
	 v = Vt * (argsenh (exp (VBR/(2*Vt)) * var / (2*sqrt(IDS*IBV))) -
		   log (IDS/IBV) / 2) - VBR / 2;
       else
	 if (deriv == 1)
	   v = Vt / (IDS + sqrt (4*IDS*IBV*exp (-VBR/Vt) + var * var));
	 else
	   v = 4 * Vt * sqrt (4*IDS*IBV*exp (-VBR/Vt) + var * var) * var;
      }

     /* Para Vbr >= 35 v.; */
     else
      {
       /* Con iD >= 0.. */
       if (var >= 0)
	{
	 if (deriv == 0)
	   v = Vt * log (var / IDS + 1);
	 else
	   if (deriv == 1)
	     v = Vt / (var + IDS);
	   else
	     v = -Vt / pow (var + IDS, 2);
	}

       /* Con -(IDS+IBV) < iD < 0.. */
       else
	 if (var > -(IDS + IBV))
	  {
	   if (deriv == 0)
	     v = Vt * log (var / (IDS + IBV) + 1);
	   else
	     if (deriv == 1)
	       v = Vt / (var + IDS + IBV);
	     else
	       v = -Vt / pow (var + IDS + IBV, 2);
	  }

	 /* Con id <= -(IDS+IBV).. */
	 else
	  {
	   if (deriv == 0)
	     v = -VBR - Vt * log (-var / (IDS+IBV));
	   else
	     if (deriv == 1)
	       v = -Vt / var;
	     else
	       if (deriv == 2)
		 v = Vt / (var * var);
	  }
      }

     v /= elems[i].valor;

     #undef IDS
     #undef VBR
     #undef IBV
    }

    case 'J':
    {
     #define BETA  elems[i].f.coef[0]
     #define LANDA elems[i].f.coef[1]
     #define VTO   elems[i].f.coef[2]

     /* Si el generador es del tipo JFET, asumimos que 'var' contiene la Vgs
	y que 'var1' contiene la Vgd; Vds puede obtenerse como Vgs - Vgd */
     w = var - var1;

     /* Si estamos en modo invertido.. */
     if (w < 0)
      {
       w = -w;          /* ..cambiamos el signo de la Vds.. */
       var = var1;      /* ..y sustitu¡mos Vgs por Vgd */
      }

     if (deriv == 0)
      {
       if (var - VTO <= 0)
	 v = 0;
       else
	 if (var - VTO < w)
	   v = BETA * (1 + LANDA*w) * pow (var - VTO, 2);
	 else
	   v = BETA * (1 + LANDA*w) * w * (2*(var - VTO) - w);
      }
     else
       if (deriv == 1)
	{
	}
       else
	{
	}

     #undef BETA
     #undef LANDA
     #undef VTO
    }
   }
  }

 return (v);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double argsenh (double x)
{
 return (log (x + sqrt (x*x + 1)));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void temperatura (float T, unsigned n_elems, elemento *elems)

/* Funci¢n que calcula el efecto de pasar de Tnom a T */
{
#define VJ elems[i].f.coef[1]
#define EG elems[i].f.coef[5]
#define MJ elems[i].f.coef[6]

 FILE *elementos;
 elemento aux;
 double Eg, safe;
 unsigned i, j;
 char nom[84], nombre_elem[6];

 /* Calculamos la energ¡a del gap */
 Eg = 1.16 - 0.000702 * (T*T) / (T + 1108);

 /* Calculamos la tensi¢n t‚rmica */
 Vt = K*T / qe;

 /* Abrimos el fichero de elementos para obtener los valores originales */
 strcpy (nom, nom_fich);
 strcat (nom, ".ELM");
 elementos = fopen (nom, "rb");

 /* Recuperamos todos los valores originales (valores a la Tnom) */
 for (i = 0; i < n_elems; ++i)
  {
   fread (&aux, sizeof (elemento), 1, elementos);
   elems[i].valor = aux.valor;
   elems[i].f     = aux.f;
  }

 /* Cerramos el fichero de elementos */
 fclose (elementos);

 /* Recorremos la lista de elementos */
 for (i = 0; i < n_elems; ++i)
  {
   /* Si el elemento es un diodo.. */
   if (elems[i].tipo[0] == 'Q')
    {
     /* ..calculamos su Is */
     elems[i].valor *= exp ((T/Tnom - 1) * EG/Vt);

     /* Calculamos su Vj, salvando la anterior */
     safe = VJ;
     elems[i].f.coef[1] = safe * T/Tnom - 3*Vt*log(T/Tnom) -
			  1.115128 * T/Tnom + Eg;

     /* Constru¡mos el nombre de su  capacidad par sita */
     strcpy (nombre_elem, "C\x0");
     if ((elems[i].nombre[0] == 'D') && (elems[i].nombre[1] == 'f'))
	{
	 strcat (nombre_elem, "e");
	 strcat (nombre_elem, &elems[i].nombre[2]);
	}
       else
	 if ((elems[i].nombre[0] == 'D') && (elems[i].nombre[1] == 'r'))
	  {
	   strcat (nombre_elem, "c");
	   strcat (nombre_elem, &elems[i].nombre[2]);
	  }
	 else
	  {
	   strcat (nombre_elem, "d");
	   strcat (nombre_elem, &elems[i].tipo[1]);
	  }

     /* La buscamos en la lista para modificarla */
     for (j = 0; (j < n_elems) && (strcmp (elems[j].nombre, nombre_elem) != 0);
	  ++j);

     /* Si ten¡a una capacidad par sita, cambiamos su valor */
     if (j < n_elems)
       elems[j].valor *= 1 + MJ * (0.0004*(T - Tnom) + 1 - VJ/safe);
    }

   /* pero, si el elemento es un pasivo.. */
   else
     if ((elems[i].tipo[0] == 'R') || (elems[i].tipo[0] == 'L') ||
	 (elems[i].tipo[0] == 'C'))
       elems[i].valor *= 1 + elems[i].CT;
  }

 return;

#undef VJ
#undef EG
#undef MJ
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

unsigned recupera_cond_inic (char tipo, unsigned no_vars, double **cond_inic)

/* Funci¢n que lee las condiciones iniciales del circuito del fichero con
   extensi¢n ".CI<tipo>", almacen ndolas en 'cond_inic' */
{
 OFSTRUCT of;
 unsigned tamano;
 int      hFile, n = 0;
 char     nom[84], ext[4];

 /* Calculamos el n§ de bytes a ocupar */
 tamano = no_vars * sizeof (double);

 /* Si no ha sido ya reservado, reservamos el espacio necesario */
 if (*cond_inic == NIL)
  {
   *cond_inic = (double *) malloc (tamano);
   mem_local += LocalSize (LocalHandle ((WORD) *cond_inic));
  }

 /* Abrimos el fichero para lectura */
 strcpy (nom, nom_fich);
 strcpy (ext, ".CIZ");
 ext[3] = tipo;
 strcat (nom, ext);
 if ((hFile = OpenFile (nom, &of, OF_READ)) == -1)
  {
   error (FICHNOEXIS);
   return (1);
  }

 /* Leemos las condiciones iniciales del fichero */
 if ((n = _lread (hFile, (LPSTR) *cond_inic, tamano)) < tamano)
   error (FICHINCOMP);

 /* Cerramos el fichero */
 _lclose (hFile);

 memset ((char *)*cond_inic + n, 0, tamano - n);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

char *obtener_nombre (char *punt, char nombre[5])
/* Funci¢n que devuelve en 'nombre' los 4 caracteres siguientes a 'punt' */
{
 unsigned i;
 char *aux;

 for (i = 0, aux = punt; i < 4; ++i, ++aux)
   nombre[i] = *aux;
 nombre[4] = 0;

 return (nombre);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int buscar_elem (unsigned n_elems, elemento *elems, char tipo[5])

/* Esta funci¢n devuelve la posici¢n del elemento del circuito cuyo nombre
   interno se le pasa en 'tipo' */
{
 int posic;                    /* Variable utilizada para recorrer la lista */

 /* Recorremos la lista hasta que encontremos el elemento o bien lleguemos
    al final de la misma, en cuyo caso devolvemos un -1. */
 for (posic = 0; posic < n_elems; ++posic)
   if (strcmp (elems[posic].tipo, tipo) == 0)
     break;

 if (posic >= n_elems)
   posic = -1;

 return (posic);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int buscar_rama (unsigned n_ramas, rama *ramas, char tipo[5])

/* Esta funci¢n busca la rama en la que se encuentra el elemento cuyo nombre
   interno viene dado en 'tipo' */
{
 int posic;                    /* Variable utilizada para recorrer la lista */

 /* Recorremos la lista hasta que encontremos el elemento o bien lleguemos
    al final de la misma, en cuyo caso devolvemos un -1. */
 for (posic = 0; posic < n_ramas; ++posic)
   if (strstr (ramas[posic].elem, tipo) != NIL)
     break;

 if (posic >= n_ramas)
   posic = -1;

 return (posic);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

char *obtener_comunes (char Hii[100], char Hjj[100], char Hij[100])

/* Esta funci¢n devuelve en Hij el string de nombres internos de los elems.
   comunes a los elementos de la diagonal principal Hii, Hjj. */
{
 unsigned a = 0, b;
 char comp[100], sgn;
 char *p;

 strset (Hij, 0);        /* Inicializamos los elementos comunes a 'ninguno' */

 do
 {
  strset (comp, 0);     /* Inicializamos el string de comparaci¢n a 'vac¡o' */
  b = 0;                          /* Apuntamos a la 1¦ posici¢n de 'comp[]' */

  while (Hii[a++] != '(');         /* Localizamos el 1er par‚ntesis abierto */

  if (--a > 0)              /* Hacemos que 'sgn' tome el valor del car cter */
    sgn = Hii[a - 1];       /* anterior al par‚ntesis abierto, si existe    */
  else                      /* alguno, o si no le damos el valor ')' por    */
    sgn = ')';              /* defecto.                                     */

  do                              /* Llenamos el string de comparaci¢n con  */
    comp[b++] = Hii[a];           /* los elementos dentro de los par‚ntesis */
  while (Hii[a++] != ')');        /* , ‚stos inclusive.                     */

  comp[b] = 0;                     /* A¤adimos el car cter de fin de string */

  if ((p = strstr (Hjj, comp)) != NIL)          /* Si 'comp' est  en Hjj... */
  {
   --p;                /* Apuntamos al car cter anterior al 1§ que coincide */

   /* Si los signos de ambas ramas difieren, a¤adimos un '-' en Hij */
   if (((p < Hjj) && (sgn == '-')) || ((p >= Hjj) && (sgn != *p)))
      strcat (Hij, "-");

   strcat (Hij, comp);                    /* A¤adimos la rama a los comunes */
  }
 }
 while (Hii[a] != 0);          /* Repetir hasta que lleguemos al fin de Hii */

 return (Hij);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int camino (unsigned n_ramas, rama *ramas,
	    unsigned salida, unsigned llegada, char elem[100])

/* Esta funci¢n llama a buscador, que se encarga de buscar un camino entre
   los nodos 'salida' y 'llegada' sin pasar por generadores de corriente. */

{
 unsigned r, nudos, *nu;

 unsigned buscador (unsigned tipo_busqueda, unsigned n_ramas, rama *ramas,
		    unsigned no_volver[], unsigned salida, unsigned llegada,
		    char elem[100]);

 /* Marcamos como usadas las ramas con generadores de corriente */
 for (r = 0; r < n_ramas; ++r)
  {
   if ((strchr (ramas[r].elem, 'I') != NIL) ||
       (strchr (ramas[r].elem, 'D') != NIL) ||
       (strchr (ramas[r].elem, 'E') != NIL))
     ramas[r].usada = 1;
   else
     ramas[r].usada = 0;
   nudos = max (max (ramas[r].nodoi, ramas[r].nodof), nudos);
  }

 /* Creamos e inicializamos la matriz de nudos usados para 'buscador' */
 nu = (unsigned *) malloc (nudos * sizeof (unsigned));
 memset (nu, 0, nudos * sizeof (unsigned));
 nu[0] = salida;
 nu[1] = FIN_NU;
 memset (elem, 0, 100);
 buscador (0, n_ramas, ramas, nu, salida, llegada, elem);

 /* Limpiamos las marcas de la lista de ramas */
 for (r = 0; r < n_ramas; ++r)
   ramas[r].usada = 0;

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

unsigned buscador (unsigned tipo_busqueda, unsigned n_ramas, rama *ramas,
		   unsigned no_volver[], unsigned salida, unsigned llegada,
		   char elem[100])

/* Funci¢n recursiva que encuentra un camino entre los nudos 'salida' y
   'llegada', sin atravesar ning£n generador de corriente, y a¤ade a 'elem'
   todos los elementos que componen dicho camino. Esta funci¢n es recursiva
   y se basa en los algoritmos de exploraci¢n de estructuras en  rbol. Se
   utiliza para calcular la expresi¢n de la tensi¢n entre dos nudos cuales-
   quiera.
   En caso de que no encuentre un camino entre los nudos, vuelve con 0.
   Para asegurarse de que no recorre bucles cerrados, se le pasa el par me-
   -tro 'no_volver', que es igual a 'salida' en la 1¦ llamada a la funci¢n
   pero no se modifica en las llamadas recursivas, de modo que no se segui-
   -r  una rama que lleve a 'no_volver'.
   La b£squeda puede ser de dos tipos; de un camino entre los nudos (con
   tipo_busqueda = 0) ¢ de los cond. que bloquean un posible camino (con
   tipo_busqueda = 1) */
{
 unsigned llegue = 0, r, i;

 for (r = 0; r < n_ramas; ++r)
 {
  if (ramas[r].usada == 0)
   {
    if (ramas[r].nodoi == salida)
     {
      for(i = 0; (no_volver[i] != FIN_NU) && (no_volver[i] != ramas[r].nodof);
	  ++i);
      if (no_volver[i] == FIN_NU)
       {
	ramas[r].usada = 1;
	if (ramas[r].nodof == llegada)
	 {
	  if (tipo_busqueda == 0)
	   {
	    elem[0] = '(';
	    strcat (elem, ramas[r].elem);
	    strcat (elem, ")");
	   }
	  return (1);
	 }
	else
	 {
	  for (i = 0; no_volver[i] != FIN_NU; ++i);
	  no_volver[i]   = ramas[r].nodof;
	  no_volver[i+1] = FIN_NU;
	  llegue = buscador (tipo_busqueda, n_ramas, ramas, no_volver,
			     ramas[r].nodof, llegada, elem);
	 }
       }
     }
    else
      if (ramas[r].nodof == salida)
       {
	for(i = 0; (no_volver[i] != FIN_NU) && (no_volver[i] != ramas[r].nodoi);
	    ++i);
	if (no_volver[i] == FIN_NU)
	 {
	  ramas[r].usada = 1;
	  if (ramas[r].nodoi == llegada)
	   {
	    if (tipo_busqueda == 0)
	     {
	      elem[0] = '-';
	      elem[1] = '(';
	      strcat (elem, ramas[r].elem);
	      strcat (elem, ")");
	     }
	    return (1);
	   }
	  else
	   {
	    for (i = 0; no_volver[i] != FIN_NU; ++i);
	    no_volver[i]   = ramas[r].nodoi;
	    no_volver[i+1] = FIN_NU;
	    llegue = buscador (tipo_busqueda, n_ramas, ramas, no_volver,
			       ramas[r].nodoi, llegada, elem);
	   }
	 }
       }
   }
  else
    if (tipo_busqueda == 1)
      if (ramas[r].nodoi == salida)
       {
	strcat (elem, "(");
	strcat (elem, ramas[r].elem);
	strcat (elem, ")");
       }
      else
	if (ramas[r].nodof == salida)
	 {
	  strcat (elem, "-(");
	  strcat (elem, ramas[r].elem);
	  strcat (elem, ")");
	 }

  if (llegue == 1)
   {
    if (ramas[r].nodof == salida)
      strcat (elem, "-");
    strcat (elem, "(");
    strcat (elem, ramas[r].elem);
    strcat (elem, ")");
    return (1);
   }
 }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int convertir (int sentido, unsigned n_elems, elemento *elems,
			    unsigned n_ramas, rama *ramas,
			    unsigned n_ecs_Z, char (*ecs_Z)[100],
			    unsigned n_ecs_Y, char (*ecs_Y)[100])

/* Esta funci¢n convierte los datos del circuito a las necesidades de cada
   momento, seg£n el valor de 'sentido', a saber;
   * A_INICIAL, restaura el circuito a los valores contenidos en los ficheros
   * A_MALLAS, sustituye los diodos por gens. no lineales de V dep. de I
   * A_NUDOS,  sustituye los diodos por gens. no lineales de I dep. de V */
{
 unsigned i, j, genvdepi = 0;
 char viejo_nombre[5], t[5], *p;

 unsigned recupera_lista_elems (elemento **elementos);
 unsigned recupera_lista_ramas (rama **ramas);
 unsigned recupera_matriz (char tipo, char (**ecs)[100]);

 switch (sentido)
 {
  case A_INICIAL:
  {
   recupera_lista_elems (&elems);
   recupera_lista_ramas (&ramas);
   recupera_matriz ('Z', &ecs_Z);
   recupera_matriz ('Y', &ecs_Y);
   break;
  }

  case A_MALLAS:
  case A_NUDOS:
  {
   /* Damos una pasada para saber cu ntos gens. de I dep. de V hay */
   if (sentido == A_MALLAS)
    {
     for (i = 0; i < n_elems; ++i)
       if (elems[i].tipo[0] == 'B')
	 ++genvdepi;
    }
   else
    {
     for (i = 0; i < n_elems; ++i)
       if (elems[i].tipo[0] == 'D')
	 ++genvdepi;
    }

   /* Damos otra pasada sustituyendo los diodos por gens. de este tipo */
   for (i = 0; i < n_elems; ++i)
     if (elems[i].tipo[0] == 'Q')
      {
       /* Obtenemos una copia de su nombre interno */
       strcpy (viejo_nombre, elems[i].tipo);

       /* Obtenemos su nuevo nombre (DNNN, NNN = n§ anterior + genvdepi) */
       itoa (atoi (&elems[i].tipo[1]) + genvdepi, t, 10);

       if (sentido == A_MALLAS)
	 strcpy (elems[i].tipo, "B000\x0");
       else
	 strcpy (elems[i].tipo, "D000\x0");

       elems[i].tipo[4 - strlen (t)] = 0;
       strcat (elems[i].tipo, t);

       /* Le asignamos "Shockley" ¢ "Inverse Shockley" como funci¢n */
       elems[i].signal = (sentido == A_MALLAS) ? 'I' : 'S';

       /* Sustitu¡mos el nombre antiguo por el actual en todos los gens. que
	  dependan de la I por este elemento */
       for (j = 0; j < n_elems; ++j)
	 if ((elems[j].tipo[0] == 'B') || (elems[j].tipo[0] == 'E'))
	   if (strcmp (elems[j].caract.elem, viejo_nombre) == 0)
	     strcpy (elems[j].caract.elem, elems[i].tipo);

       /* Idem en la rama en la que se encuentra el diodo.. */
       for (j = 0; strcmp (ramas[j].elem, viejo_nombre) != 0; ++j);
       strcpy (ramas[j].elem, elems[i].tipo);

       /* ..y obtenemos la variable de la que depende el generador */
       if (sentido == A_MALLAS)
	 strcpy (elems[i].caract.elem, elems[i].tipo);
       else
	{
	 elems[i].caract.nodos[0] = ramas[j].nodoi;
	 elems[i].caract.nodos[1] = ramas[j].nodof;
	}

       /* Idem en todas las mallas */
       for (j = 0; j < n_ecs_Z; ++j)
	 if ((p = strstr (ecs_Z[j], viejo_nombre)) != NIL)
	   memcpy (p, elems[i].tipo, 4);

       /* Idem en todos los nudos */
       for (j = 0; j < n_ecs_Y; ++j)
	 if ((p = strstr (ecs_Y[j], viejo_nombre)) != NIL)
	   memcpy (p, elems[i].tipo, 4);
      }
   break;
  }
 }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int diagonaliza_real (unsigned orden, unsigned max_orden)

/* Esta funci¢n diagonaliza una submatriz de orden 'orden' de una matriz
   real de orden 'max_orden', siempre que sea posible. Si no lo es, la
   triangulariza y devuelve un valor <> 0 */
{
 int sm, i, j, posmp;
 double coef, coef1, mp, residuo;

 /* Este bucle triangulariza la mitad inferior de la matriz */
 for (sm = 0; sm < orden - 1; ++sm)
 {
  /* Buscamos la fila que tenga el m ximo pivote */
  mp = *C(sm, sm);
  posmp = sm;
  for (i = sm + 1; i < orden; ++i)
    if (fabs (*C(i, sm)) > fabs (mp))
     {
      mp = *C(i, sm);
      posmp = i;
     }

  /* Colocamos en la fila 'sm' la que tenga el m ximo pivote */
  for (j = 0; j <= max_orden; ++j)
   {
    coef = *C(sm, j);
    *C(sm, j) = *C(posmp, j);
    *C(posmp, j) = coef;
   }

  /* Si el coeficiente pivote es cero, no se puede diagonalizar */
  if (*C(sm, sm) == 0)
    return (-1);

  /* 'coef' contiene el coeficiente pivote */
  coef = *C(sm, sm);

  /* Pasamos por todas las filas haciendo 0 su sm-‚simo coeficiente */
  for (i = sm + 1; i < orden; ++i)

   /* Si el sm-‚simo coeficiente ya es cero, no hace falta procesar la fila */
   if (*C(i, sm) != 0)
   {
    /* Obtenemos el valor a(i,sm) / a(sm,sm) */
    coef1 = *C(i, sm);
    *C(i, sm) = 0;

    /* Le restamos a la fila 'i' multiplicada por 'coef' la fila 'sm'
       multiplicada por 'coef1' */
    for (j = sm + 1; j <= max_orden; ++j)
     {
      /* Si el residuo de la resta es 10^10 veces menor que lo que hab¡a
	 antes, redondeamos a 0 */
      residuo = *C(i, j) * coef - *C(sm, j) * coef1;
/*    if (fabs (residuo) < fabs (*C(i, j) * coef * 10e-10))
	*C(i, j) = 0;
      else */
	*C(i, j) = residuo / coef;
     }
   }
 }

 /* Si alg£n elemento de la diagonal es 0, no es posible la diagonalizaci¢n,
    dej ndose la matriz triangularizada inferiormente y devolviendo -1 */
 for (i = 0; i < orden; ++i)
   if (*C(i,i) == 0)
     return (-1);

 /* Este bucle triangulariza la mitad superior de la matriz */
 for (sm = orden - 1; sm > 0; --sm)
 {
  /* Obtenemos el coeficiente pivote, que en ning£n caso puede ser 0 */
  coef = *C(sm, sm);

  /* Pasamos por todas las filas, de abajo a arriba, haciendo ceros */
  for (i = sm - 1; i >= 0; --i)
  {
   /* Obtenemos el coeficiente que multiplicar  a la fila sm-‚sima */
   coef1 = *C(i, sm);

   /* Le restamos a la fila 'i' multiplicada por 'coef' la fila 'sm'
      multiplicada por 'coef1'. Puesto que los otros coeficientes de la
      fila sm son 0, s¢lo necesitamos restar los coeficientes sm-‚simos
      y los que no pertenezcan a la submatriz, adem s de los independientes
      de cada fila */
   for (j = orden; j <= max_orden; ++j)
     *C(i, j) = (*C(i, j) * coef - *C(sm, j) * coef1) / coef;
   *C(i, sm) = 0;
  }
 }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int diagonaliza_comp (unsigned orden)

/* Esta funci¢n diagonaliza una matriz compleja de orden 'orden', siempre
   que sea posible. Si no lo es, la triangulariza y devuelve un valor <> 0 */
{
 int sm, i, j;
 complejo coef, coef1;

 /* Este bucle triangulariza la mitad inferior de la matriz */
 for (sm = 0; sm < orden - 1; ++sm)
 {
  /* Si el coeficiente pivote es cero... */
  if ((M(sm, sm)->Re == 0) && (M(sm, sm)->Im == 0))
  {
   /* ...buscamos la siguiente fila cuyo coeficiente sea distinto de cero */
   for (i = sm + 1; (i < orden) && (M(i, sm)->Re == 0) && (M(i, sm)->Im != 0);
	++i);

   /* Si no hay ninguna, no se puede diagonalizar */
   if (i >= orden)
     return (-1);

   /* Intercambiamos las filas 'i' y 'sm' */
   for (j = 0; j <= orden; ++j)
   {
    coef = *M(sm, j);
    *M(sm, j) = *M(i, j);
    *M(i, j) = coef;
   }
  }

  /* 'coef' contiene el coeficiente pivote */
  coef = *M(sm, sm);

  /* Pasamos por todas las filas haciendo 0 su sm-‚simo coeficiente */
  for (i = sm + 1; i < orden; ++i)

   /* Si el sm-‚simo coeficiente ya es cero, no hace falta procesar la fila */
   if ((M(i, sm)->Re != 0) || (M(i, sm)->Im != 0))
   {
    /* Obtenemos el valor a(i,sm) / a(sm,sm) */
    coef1 = cdiv (*M(i, sm), coef);

    /* Le restamos a la fila 'i' la fila 'sm' multiplicada por 'coef1' */
    for (j = sm; j <= orden; ++j)
     *M(i, j) = res (*M(i, j), cmul (coef1, *M(sm, j)));
   }
 }

 /* Si alg£n t‚rmino de la diagonal es 0, no es posible la diagonalizaci¢n,
    dej ndose la matriz triangularizada inferiormente y devolviendo -1 */

 for (i = 0; i < orden; ++i)
   if ((M(i,i)->Re == 0) && (M(i,i)->Im == 0))
     return (-1);

 /* Este bucle triangulariza la mitad superior de la matriz */
 for (sm = orden - 1; sm > 0; --sm)
 {
  /* Obtenemos el coeficiente pivote, que en ning£n caso puede ser 0 */
  coef = *M(sm, sm);

  /* Pasamos por todas las filas, de abajo a arriba, haciendo ceros */
  for (i = sm - 1; i >= 0; --i)
  {
   /* Obtenemos el coeficiente que multiplicar  a la fila sm-‚sima */
   coef1 = cdiv (*M(i, sm), coef);

   /* Le restamos a la fila 'i' la fila 'sm' multiplicada por 'coef1'. Puesto
      que los otros coeficientes de la fila sm son 0, s¢lo necesitamos restar
      los coeficientes sm-‚simos y los independientes de cada fila */
   *M(i, orden) = res (*M(i, orden), cmul (coef1, *M(sm, orden)));
   M(i, sm)->Re = M(i, sm)->Im = 0;
  }
 }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void error (int numero)

/* Funci¢n que procesa los errores que se produzcan */
{
 static char *mensajes[] = { "Fichero inexistente", "Fichero incompleto",
			     "Memoria insuficiente", "Error de Overflow",
			     "Fichero de ELEMENTOS no localizado",
			     "Fichero de RAMAS no localizado",
			     "Fichero de ECUACIONES no localizado",
			     "Fichero de RESPUESTAS no localizado",
			     "Fichero de CONDS. INICIALES no localizado" };

 MessageBox (hwnd, mensajes[numero], frase1, MB_ICONINFORMATION | MB_OK);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int matherr (struct exception *e)
{
 switch (e->type)
 {
  case OVERFLOW:
  {
   e->retval = 1.7e308;
   return (1);
  }

  case UNDERFLOW:
  {
   e->retval = 0;
   return (1);
  }
 }

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double huge *C (unsigned i, unsigned j)

/* Esta funci¢n devuelve la direcci¢n del elemento (i,j) de la matriz de
   an lisis de las condiciones iniciales */
{
 return (r + (orden + gendep + 1)*i + j);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double huge *J (unsigned i, unsigned j)

/* Esta funci¢n devuelve la direcci¢n del elemento (i,j) del Jacobiano de la
   matriz de an lisis de las condiciones iniciales */
{
 return (w + (orden + gendep + 1)*i + j);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo huge *M (unsigned i, unsigned j)

/* La sub-funci¢n siguiente ahorra el trabajo de calcular el valor de la
   posici¢n de memoria correspondiente a una fila y columna de la matriz */
{
 return ((p + (orden + gendep + 1)*i) + j);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double huge *RLC (double huge *puntero, unsigned i, unsigned j)

/* Funci¢n que calcula la direcci¢n de un elemento de coordenadas (i,j)
   dentro de la matriz R, L ¢ C seg£n el valor de 'tipo' */
{
 return ((puntero + (orden + gendep)*i) + j);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

gen (*G (unsigned i))[10]

/* Funci¢n que calcula la direcci¢n de la fila 'i' en  G */
{
 return (g + i);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
