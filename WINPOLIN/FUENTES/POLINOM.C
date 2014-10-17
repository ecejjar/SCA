/*
 ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º                                                                          º
 º     Fichero de librer¡a para polinomios y funciones de transferencia     º
 º                                                                          º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/

#include <stdlib.h>
#include <math.h>
#include <complejo.h>

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDefinici¢n de constantesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

#define SUMA 1
#define RESTA -1
#define precision 1E-8

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDefinici¢n de tiposÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

typedef struct
	{
	 int grado;
	 float coef[11];
	} polinomio;

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

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄDeclaraci¢n de funciones contenidasÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int cero (polinomio p);
long fact (unsigned i);
polinomio operar (int operador, polinomio p, polinomio q);
polinomio multiplicar (polinomio p, polinomio q);
polinomio dividir (polinomio p, polinomio q, polinomio *r);
polinomio derivar_pol (polinomio p);
func_trans operar_func (int operador, func_trans A, func_trans B);
func_trans mult_func (func_trans A, func_trans B);
func_trans div_func (func_trans A, func_trans B);
func_trans derivar_func (func_trans G);
complejo pol_en_z (polinomio p, complejo z);
double pol_en_x (polinomio p, double x);
complejo func_en_z (func_trans G, complejo z);
raices_pol raices (polinomio p);
void reducir (func_trans *G);
fracc_simples descomponer (func_trans G);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo de la zona de funcionesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int cero (polinomio p)

/* Funci¢n que devuelve 1 si el polinomio p es el polinomio 0, ¢ 0 en caso
   contrario */
{
 if ((p.grado == 0) && (p.coef[0] == 0))
   return (1);

 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

long fact (unsigned n)

/* Esta funci¢n calcula el factorial de n */
{
 unsigned i;
 long f;

 if (n == 0)
   f = 1;
 else
  {
   f = n;
   for (i = 1; i <= (n-1); ++i)
     f *= i;
  }
 return (f);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

polinomio operar (int operador, polinomio p, polinomio q)

/* Esta funci¢n suma (o resta) los polinomios p y q, dando el resultado */
{
  polinomio r;
  unsigned i;

  if (operador < 0)
    for (i = 0; i <= q.grado; ++i)
      q.coef[i] = -q.coef[i];

  for (i = 0; i <= p.grado; ++i)
    if (i <= q.grado)
      r.coef[i] = p.coef[i] + q.coef[i];
    else
      r.coef[i] = p.coef[i];

  if (q.grado > p.grado)
    {
      for (i = p.grado + 1; i <= q.grado; ++i)
	  r.coef[i] = q.coef[i];
      r.grado = q.grado;
    }
  else
    r.grado = p.grado;

return (r);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

polinomio multiplicar (polinomio p, polinomio q)

/* Funci¢n que multiplica los polinomios p y q, devolviendo el resultado.
   El algoritmo es multiplicar, desplazar y sumar. */
{
  polinomio r;
  unsigned i, j;

  /* Ponemos el polinomio 'r' a 0 */
  memset (&r, 0, sizeof (polinomio));

  /* Si ninguno de los polinomios es cero, realizamos la multiplicaci¢n */
  if ((cero (p) == 0) && (cero(q) == 0))
   {
    /* El grado del resultado es la suma de los grados de los factores */
    r.grado = p.grado + q.grado;

    for (i = 0; i <= p.grado; ++i)
      for (j = 0; j <= q.grado; ++j)
	r.coef[i + j] += p.coef[i] * q.coef[j];
   }
  return (r);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

polinomio dividir (polinomio p, polinomio q, polinomio *r)

/* Funci¢n que divide el polinomio p entre el q, devolviendo el resultado,
  , y el resto en r. Si la divisi¢n es por el pol. 0, se asume que se divide
  por el pol. 1 y convertimos el divisor en dicho polinomio. */
{
 polinomio c;
 unsigned j, n, m;

  /* Obtenemos los grados de ambos polinomios */
  n = p.grado;
  m = q.grado;

  /* Ponemos el polinomio 'c' a 0 */
  memset (&c, 0, sizeof (polinomio));

  /* Si el polinomio divisor es el polinomio 0.. */
  if (cero (q))
   {
    /* ..se asume el polinomio 1, el resultado es el dividendo y el resto 0 */
    c = p;
    memset (r, 0, sizeof (polinomio));
   }

  /* Pero, si el divisor no es cero.. */
  else
   {
    /* El grado del resultado es la dif. de los grados de los factores */
    c.grado = n - m;

    while (n >= m)
     {
      c.coef[n - m] = p.coef[n] / q.coef[m];
      for (j = 0; j <= m; ++j)
	p.coef[n - j] -= c.coef[n - m] * q.coef[m - j];
      --n;
     }

    *r = p;
    (*r).grado = m - 1;
   }

  return (c);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

polinomio derivar_pol (polinomio p)

/* Funci¢n que obtiene la derivada del polinomio p */
{
 polinomio d;
 unsigned i;


  d.grado = p.grado - 1;
  for (i = 0; i <= d.grado; ++i)
    d.coef[i] = p.coef[i + 1] * (i + 1);

  return (d);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

func_trans derivar_func (func_trans G)

/* Funci¢n que obtiene la derivada de la funci¢n racional G */
{
 func_trans GD;

  /* Si el denominador es el polinomio 0.. */
  if (cero (G.den))
    /* ..lo transformamos en el pol. 1 */
    G.den.coef[0] = 1;

  /* Derivada del num. por el den. menos el num. por la derivada del den. */
  GD.num = operar (RESTA, multiplicar (derivar_pol (G.num), G.den),
			  multiplicar (G.num, derivar_pol (G.den)));

  /* Denominador es el producto de denominadores */
  GD.den = multiplicar (G.den, G.den);

  return (GD);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo pol_en_z (polinomio p, complejo z)

/* Funci¢n que obtiene el valor de p particularizado en z */
{
 int i;
 complejo s, z_i;

 s.Re = p.coef[0];
 s.Im = 0;

 if (p.grado > 0)
   for (i = 1; i <= p.grado; ++i)
    {
     z_i = elevar (z, (double) i);
     s.Re += p.coef[i] * z_i.Re;
     s.Im += p.coef[i] * z_i.Im;
    }

 return (s);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double pol_en_x (polinomio p, double x)

/* Funci¢n que obtiene el valor de p particularizado en x */
{
 int i;
 double s;

 s = p.coef[0];

 if (p.grado > 0)
   for (i = 1; i <= p.grado; ++i)
     s += p.coef[i] * pow (x, i);

 return (s);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

complejo func_en_z (func_trans G, complejo z)

/* Funci¢n que eval£a el valor de la funci¢n compleja G en z devolviendo el
  resultado */
{
 if (cero (G.den))
   return (pol_en_z (G.num, z));
 else
   return (cdiv (pol_en_z (G.num, z), pol_en_z (G.den, z)));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

func_trans operar_func (int operador, func_trans A, func_trans B)

/* Funci¢n que suma o resta las funciones racionales A y B */

{
 func_trans C;

 /* Si el denom. de A es el pol. 0, se asume el pol. 1 */
 if (cero (A.den))
   A.den.coef[0] = 1;

 /* Si el denom. de B es el pol. 0, se asume el pol. 1 */
 if (cero (B.den))
   B.den.coef[0] = 1;

 /* Obtiene el numerador del producto cruzado de nums. y dens. de A y B */
 C.num = operar (operador,
		 multiplicar (A.num, B.den), multiplicar (A.den, B.num));

 /* El denominador es el producto de denominadores */
 C.den = multiplicar (A.den, B.den);

 return (C);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

func_trans mult_func (func_trans A, func_trans B)
/* Funci¢n que multiplica las funciones de transferencia A y B */
{
 func_trans R;

 /* Si el denom. de A es el pol. 0, se transforma en el pol. 1 */
 if (cero (A.den))
   A.den.coef[0] = 1;

 /* Si el denom. de B es el pol. 0, se transforma en el pol. 1 */
 if (cero (B.den))
   B.den.coef[0] = 1;

 R.num = multiplicar (A.num, B.num);
 R.den = multiplicar (A.den, B.den);

 return (R);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

func_trans div_func (func_trans A, func_trans B)
/* Funci¢n que divide las funciones de transferencia A y B */
{
 func_trans R;

 /* Si el denom. de A es el pol. 0, se transforma en el pol. 1 */
 if (cero (A.den))
   A.den.coef[0] = 1;

 /* Si el denom. de B es el pol. 0, se transforma en el pol. 1 */
 if (cero (B.den))
   B.den.coef[0] = 1;

 R.num = multiplicar (A.num, B.den);
 R.den = multiplicar (A.den, B.num);

 return (R);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

raices_pol raices (polinomio p)

/* Funci¢n que calcula las ra¡ces del polinomio p, haciendo una 1¦ aproxim.
   por el m‚todo de Newton y otra m s precisa por aprox. sucesivas */
{
 raices_pol r;
 polinomio d, resto;
 complejo pz, dz, z, zAux, best_z, x, xAux, best_x;
 double px, dx, c, b;
 unsigned i, j, k, iters;

 r.numero = p.grado;      /* El n£mero de ra¡ces es igual al grado del pol. */

 /* Primero extraemos las ra¡ces en el origen, para evitarnos su c lculo y
    otras complicaciones */
 while (p.coef[0] == 0)
 {
  /* Desplazamos el polinomio entero un lugar a la derecha (lo div. por s) */
  for (i = 1; i <= p.grado; ++i)
    p.coef[i-1] = p.coef[i];
  --p.grado;

  /* Hacemos una de las ra¡ces igual a cero */
  r.valor[r.numero - p.grado].Re = r.valor[r.numero - p.grado].Im = 0;
 }

 if (p.grado > 1)

   /* Buscamos las ra¡ces restantes */
   for (i = (r.numero - p.grado + 1); i < r.numero; ++i)
    {
     /* Punto inicial para la iteraci¢n de Newton, x para la real y z para la
	compleja. */
     x.Re = x.Im = xAux.Im = z.Re = 0;

     /* Inicializamos el contador de iteraciones */
     iters = 0;

     /* Si z.Im = 0 buscar  s¢lo ra¡ces reales    */
     z.Im = 1;

     px = pol_en_z (p, x).Re;            /* Obtenemos los valores iniciales */
     pz = pol_en_z (p, z);               /* de la funci¢n para la iteraci¢n */
     d = derivar_pol (p);       /* Obtiene en D la derivada del polinomio P */

     do                                   /* Bucle que realiza la iteraci¢n */
     {
      /* Nos aseguramos de que f'(x) es distinta de 0 */
      while (fabs (dx = pol_en_z (d, x).Re) <= precision)
       {
	x.Re += 1;
	px = pol_en_z (p, x).Re;
       }

      /* Resultado parcial p(x) / p'(x) */
      xAux.Re = px / dx;

      /* Iteraci¢n x[i+1] = x[i] - f(x[i]) / f'(x[i]) */
      x.Re -= xAux.Re;

      /* Nos aseguramos de que f'(z) es distinta de 0 */
      while (polar (dz = pol_en_z (d, z)).mod <= precision)
       {
	z.Re += 1;
	pz = pol_en_z (p, z);
       }

      /* Resultado parcial p(z) / p'(z) */
      zAux = cdiv (pz, dz);

      /* Iteraci¢n z[i+1] = z[i] - f(z[i]) / f'(z[i]) */
      z.Re -= zAux.Re;
      z.Im -= zAux.Im;
     }
     while ((fabs (px = pol_en_z (p, x).Re) > precision) &&
	    (polar (pz = pol_en_z (p, z)).mod > precision) &&
	    (++iters < 20));
     /* Iterar hasta obtener una primera aproximaci¢n, o bien el algoritmo
	no encuentre la ra¡z en 20 iteraciones, con lo que entra en acci¢n
	el algoritmo de aproximaciones sucesivas. */

     d = p;                                /* Obtenemos una copia de p en d */

     /* Tomamos el 1er t‚rmino de la forma K*s^j no nulo. N¢tese que tiene que
       haber alguno ya que el m¡nimo grado de 'p' dentro de este bucle es 2 */
     j = 1;
     while (d.coef[j] == 0)
       ++j;

     /* Obtenemos el coeficiente que multiplica a s^j en 'c'. */
     c = -d.coef[j];

     /* Si dicho t‚rmino es el de mayor orden, todos excepto el 0 son nulos, y
	al despejar el 'j', el polinomio pasar  a ser de grado 0 */
     if (j == d.grado)
       d.grado = 0;

     d.coef[j] = 0;                    /* Despejamos la s^j para aplicar el */
     for (k = 0; k <= d.grado; ++k)    /* algoritmo de aprox. sucesivas;    */
      d.coef[k] = d.coef[k] / c;       /* s[n+1] = A*s[n]^m +...+ N*s^2 + M */


     /* Reinicializamos el contador de iteraciones */
     iters = 0;

     if (fabs (px) <= polar (pz).mod)     /* Si la ra¡z obtenida es real... */
      {
       /* Condici¢n de convergencia: ³g'(s)³ < 1 */
       if (pol_en_z (derivar_pol (d), x).Re < 1)
	 do
	 {                                     /* Aplicamos el algoritmo de */
	  xAux = x;                            /* aprox. sucesivas hasta    */
	  x = pol_en_z (d, xAux);              /* obtener la prec. deseada  */
	  if (j > 1)                           /* ¢ hasta que se hayan rea- */
	    x = elevar (x, 1/(double) j);      /* -lizado 20 iteraciones.   */
	 }
	 while ((fabs (pol_en_z (p, x).Re) < fabs (pol_en_z (p, xAux).Re) &&
		(++iters) < 20));
       else
	 xAux = x;

       r.valor[i].Re = xAux.Re;         /* Asignamos el valor x a la raiz,  */
       r.valor[i].Im = 0;               /* , haciendo 0 la parte imaginaria */

       memset (&d, 0, sizeof (polinomio));    /* Inicializamos la variable. */
       d.grado = 1;                     /* Estas tres l¡neas forman la ex-  */
       d.coef[0] = -x.Re;               /* presi¢n de la ra¡z del polinomio */
       d.coef[1] = 1;                   /* , es decir, (z - c), c = ra¡z.   */
      }
     else                               /* , pero si la ra¡z es compleja... */
      {
       /* Condici¢n de convergencia: ³g'(s)³ < 1 */
       if (polar (pol_en_z (derivar_pol (d), z)).mod < 1)
	 do
	 {                                     /* Aplicamos el algoritmo de */
	  zAux = z;                            /* aprox. sucesivas hasta    */
	  z = pol_en_z (d, zAux);              /* obtener la prec. deseada  */
	  if (j > 1)                           /* ¢ hasta que se hayan rea- */
	    z = elevar (z, 1/(double) j);      /* -lizado 20 iteraciones.   */
	 }
	 while ((polar (pol_en_z (p, z)).mod < polar (pol_en_z (p, zAux)).mod)
		&& (++iters < 20));
       else
	 zAux = z;

       r.valor[i] = zAux;                 /* Asignamos el valor z a la ra¡z */

       /* Evitamos el c lculo de la ra¡z conjugada salt ndonos una ra¡z y
	  d ndole su valor directamente */
       r.valor[++i] = conj (z);

       memset (&d, 0, sizeof (polinomio));    /* Inicializamos la variable. */
       d.grado = 2;                  /* Formamos la expresi¢n de las ra¡ces */
       d.coef[0] = z.Re*z.Re + z.Im*z.Im;           /* complejas conjugadas */
       d.coef[1] = -2 * z.Re;              /* como (z - Re)ı + Imı, siendo  */
       d.coef[2] = 1;                      /* Re ñ jIm las ra¡ces complejas */
      }

     /* Dividiendo el polinomio entre la ra¡z se la extraemos dejando las
	restantes. */
     p = dividir (p, d, (polinomio *) &resto);
    }

 /* El polinomio restante, si queda alguno, es justamente la £ltima ra¡z */
 if (p.grado > 0)
  {
   p.coef[0] = -p.coef[0];
   r.valor[r.numero].Re = p.coef[0] / p.coef[1];
  }

 return (r);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void reducir (func_trans *G)

/* Funci¢n que elimina las ra¡ces comunes a num. y den. de G */
{
 raices_pol r;
 polinomio raiz, menor, mayor, coc, resto;
 unsigned i, j;

 /* En 'menor' ponemos el pol. de menor grado, y en 'mayor' el otro */
 if (G->num.grado > G->den.grado)
  {
   mayor = G->num;
   menor = G->den;
  }
 else
  {
   mayor = G->den;
   menor = G->num;
  }

 /* Calculamos las ra¡ces del que menos tenga */
 r = raices (menor);

 /* Hacemos 'menor' igual a 1 */
 menor.grado = 0;
 menor.coef[0] = 1;

 /* Recorremos las ra¡ces una por una */
 for (i = 1; i <= r.numero; ++i)
  {
   /* Si tenemos una ra¡z compleja.. */
   if (r.valor[i].Im > precision)
    {
     /* ..constru¡mos el t‚rmino (s - Re)ı + Imı */
     raiz.grado = 2;
     raiz.coef[2] = 1;
     raiz.coef[1] = -2 * r.valor[i].Re;
     raiz.coef[0] = r.valor[i].Re*r.valor[i].Re + r.valor[i].Im*r.valor[i].Im;

     /* Nos saltamos la siguiente ra¡z ya que ser  la conjugada */
     ++i;
    }

   /* pero, si la ra¡z es real.. */
   else
    {
     /* ..constru¡mos el t‚rmino (s - Re) */
     raiz.grado = 1;
     raiz.coef[1] = 1;
     raiz.coef[0] = -r.valor[i].Re;
    }

   /* Si la raiz r[i] de 'menor' lo es tambi‚n de 'mayor'... */
   if (polar (pol_en_z (mayor, r.valor[i])).mod <= precision)

     /* ...reducimos el polinomio 'mayor' al anterior sin la ra¡z r[i] */
     mayor = dividir (mayor, raiz, (polinomio *) &resto);

   /* Pero, si r[i] no es ra¡z de ambos... */
   else

     /* ...a¤adimos la ra¡z no simplificable al polinomio 'menor' */
     menor = multiplicar (menor, raiz);
  }

  /* Finalmente, ponemos cada uno en su sitio */
 if (G->num.grado > G->den.grado)
  {
   /* Multiplicamos los polinomios resultantes por las ctes. que multiplican
      a los t‚rminos de mayor orden de la funci¢n original */
   raiz.grado = 0;
   raiz.coef[0] = G->num.coef[G->num.grado];
   G->num = multiplicar (mayor, raiz);
   raiz.coef[0] = G->den.coef[G->den.grado];
   G->den = multiplicar (menor, raiz);
  }
 else
  {
   /* Multiplicamos los polinomios resultantes por las ctes. que multiplican
      a los t‚rminos de mayor orden de la funci¢n original */
   raiz.grado = 0;
   raiz.coef[0] = G->den.coef[G->den.grado];
   G->den = multiplicar (mayor, raiz);
   raiz.coef[0] = G->num.coef[G->num.grado];
   G->num = multiplicar (menor, raiz);
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

fracc_simples descomponer (func_trans G)

/* Esta funci¢n descompone G en fracciones simples */
{
 raices_pol polos;
 fracc_simples d;
 unsigned i, j, orden;
 func_trans H;
 polinomio raiz, res_parc, temp;
 complejo z;
 modarg u, v;

 polos = raices (G.den);           /* Calculamos las ra¡ces del denominador */
 d.numero = polos.numero;    /* El n§ de fracciones ser  igual al de ra¡ces */

 for (i = 1; i <= polos.numero; ++i)    /* Bucle que calcula las fracciones */
  {
   /* Localizamos el siguiente polo no procesado */
   while ((polos.valor[i].Re == 0) && (polos.valor[i].Im == 0))
    ++i;

   z = polos.valor[i];
   u = polar (z);
   orden = 0;

   /* Recorremos las ra¡ces buscando las de orden m£ltiple */
   for (j = 1; j <= polos.numero; ++j)
    {
     /* Tomamos una ra¡z en forma m¢dulo-argumento */
     v = polar (polos.valor[i + orden]);

     /* Si est  lo bastante cerca de la que estamos procesando (la 'i').. */
     if ((v.mod < u.mod +   precision) && (v.mod > u.mod -   precision))
       {
	/* ..anulamos esta para que no sea procesada de nuevo */
	polos.valor[i + orden].Re = polos.valor[i + orden].Im = 0;

	/* Incrementamos el orden si las ra¡ces parecen la misma */
	if ((v.arg < u.arg + 5*precision) && (v.arg > u.arg - 5*precision))
	  ++orden;
       }
    }

   res_parc.grado = 0;      /* Inicializamos al valor real 1 la var. que    */
   res_parc.coef[0] = 1;    /* contendr  la expresi¢n de la ra¡z de orden n */

   H = G;                            /* Obtenemos una copia de la funci¢n G */

   if (z.Im != 0)
    {
     raiz.grado = 2;
     raiz.coef[2] = 1;
     raiz.coef[1] = -2 * z.Re;
     raiz.coef[0] = z.Re * z.Re + z.Im * z.Im;
    }
   else
    {
     raiz.grado = 1;                       /* Formamos la expresi¢n de la  */
     raiz.coef[0] = -z.Re;                 /* ra¡z de orden 1 en raiz.     */
     raiz.coef[1] = 1;
    }

   /* Obtenemos la de orden n en res_parc */
   for (j = 1; j <= orden; ++j)
     res_parc = multiplicar (raiz, res_parc);

   /* Extraemos el polo de orden n */
   H.den = dividir (G.den, res_parc, (polinomio *) &temp);

   /* Obtenemos el residuo */
   d.fraccion[i + orden - 1].num = func_en_z (H, z);

   /* Lo guardamos junto con el polo */
   d.fraccion[i + orden - 1].den = z;

   /* Si el polo es complejo, habr  otro t‚rmino cuyo residuo es conjugado
      del obtenido y cuyo polo ser  conjugado del procesado */
   if (z.Im != 0)
    {
     d.fraccion[i + 2*orden - 1].num.Re =  d.fraccion[i + orden -1].num.Re;
     d.fraccion[i + 2*orden - 1].num.Im = -d.fraccion[i + orden -1].num.Im;
     d.fraccion[i + 2*orden - 1].den.Re =  z.Re;
     d.fraccion[i + 2*orden - 1].den.Im = -z.Im;
    }

   /* Si el polo es de orden mayor de 1... */
   if (orden > 1)

     /* El primer residuo es el de orden 0, el £ltimo el de orden n. */
     for (j = orden - 2; j >= 0; --j)
      {
       /* Calculamos la derivada de orden j */
       H = derivar_func (H);

       /* Particularizamos la derivada en z */
       d.fraccion[i + j].num = func_en_z (H, z);

       /* Dividimos por el factorial de (j + 1) */
       d.fraccion[i + j].num.Re /= fact (j + 1);
       d.fraccion[i + j].num.Im /= fact (j + 1);

       /* Ponemos el polo en el denominador */
       d.fraccion[i + j].den = z;

       /* Si el polo es complejo, a¤adir t‚rmino residuo y polo conjugados */
       if (z.Im != 0)
	{
	 d.fraccion[i + orden + j].num.Re =  d.fraccion[i + j].num.Re;
	 d.fraccion[i + orden + j].num.Im = -d.fraccion[i + j].num.Im;
	 d.fraccion[i + orden + j].den.Re =  z.Re;
	 d.fraccion[i + orden + j].den.Im = -z.Im;
	}
      }
  }

 return (d);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void simplificar (func_trans *G)

/* Esta funci¢n reduce la funci¢n G eliminando los polos en el origen comunes
   a numerador y denominador */
{
 polinomio r, dummy;
 unsigned i, j;

 memset (&r, 0, sizeof (polinomio));

 if (!cero (G->num))
  {
   /* Obtenemos cu ntas ra¡ces en el origen tienen el num. y el den. */
   for (i = 0; (i <= G->num.grado) && (G->num.coef[i] == 0); ++i);
   for (j = 0; (j <= G->den.grado) && (G->den.coef[i] == 0); ++j);

   /* Si hab¡a alguna.. */
   if ((i <= G->num.grado) && (j <= G->den.grado))
    {
     /* ..constru¡mos la ra¡z com£n a ambos */
     r.grado = min (i,j);
     r.coef[r.grado] = 1;

     /* Reducimos los polinomios num. y den. */
     G->num = dividir (G->num, r, &dummy);
     G->den = dividir (G->den, r, &dummy);
    }
  }

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void determ (unsigned orden, unsigned sust, func_trans *G)

/* Calcula el determinante del sistema G de orden 'orden', sustituyendo la
   columna 'sust' por el t‚rmino independiente */
{
 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
