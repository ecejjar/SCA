#include <windows.h>
#include <resolver.h>

extern HWND        hwnd;
extern HANDLE      hr, hw;
extern long        mem_global;
extern unsigned    mem_local;
extern short       cyLine, nVposScrl, minimizada, Fin, Abort;
extern char        frase2[], frase3[], frase4[], frase5[];
extern unsigned    orden, gendep, gendepv, gendepi;
extern double huge *r,
	      huge *w;
extern unsigned    *fg;
extern char        nom_fich[];
extern double      Tact;

extern int diagonaliza_real (unsigned orden, unsigned max_orden);

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄVariables y funciones globalesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

HANDLE      hl, hc;
double huge *l,
       huge *c;
gen         (*g)[10];

extern double huge *RLC (double huge *puntero, unsigned i, unsigned j);
extern gen         (*G (unsigned i))[];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄComienzo del c¢digoÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void resp_en_el_tiempo (unsigned n_elems, elemento *elems,
			unsigned n_ramas, rama *ramas,
			unsigned n_ecs_Z, char (*ecs_Z)[100],
			unsigned n_ecs_Y, char (*ecs_Y)[100])

/* Esta funci¢n obtiene la respuesta temporal del circuito bas ndose en el
   m‚todo de Runge-Kutta de 4§ orden, y la almacena en un fichero */
{
 RECT      rect;
 MSG	   msg;
 FILE      *temp;
 clock_t   comienzo, final;
 resultado *resp = NIL;
 double    t, v = 0,
	   h = 1e-12, h_6 = 1e-12/6,
	   H, *pr, *y, (*k)[4];
 float     fin, pasos;
 unsigned  n_resp, n_respV, n_respI,
	   num_comp, cond, i, j, m, a, b, nv, factor, num_iters = 0;
 int       *comp = NIL, *compV, *compI, error = 0, error_tpp = 0;
 char      nom[84], num[10], *pt;

 unsigned long n_pasos;

 extern double huge *J(unsigned i, unsigned j);
 extern unsigned recupera_respuestas (char tipo, resultado **respuestas);
 extern unsigned recupera_cond_inic (char tipo,
				     unsigned no_vars, double **cond_inic);
 extern int convertir (int sentido, unsigned n_elems, elemento *elems,
		       unsigned n_ramas, rama *ramas,
		       unsigned n_ecs_Z, char (*ecs_Z)[100],
		       unsigned n_ecs_Y, char (*ecs_Y)[100]);
 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);
 extern void temperatura (float T, unsigned n_elems, elemento *elems);
 extern int cond_inic_por_mallas (unsigned no_vars,
				  unsigned n_elems, elemento *elems,
				  unsigned n_ramas, rama *ramas,
				  unsigned n_ecs_Z, char (*ecs_Z)[100],
				  unsigned n_ecs_Y, char (*ecs_Y)[100]);
 extern int cond_inic_por_nudos  (unsigned no_vars,
				  unsigned n_elems, elemento *elems,
				  unsigned n_ramas, rama *ramas,
				  unsigned n_ecs_Z, char (*ecs_Z)[100],
				  unsigned n_ecs_Y, char (*ecs_Y)[100]);

 double f (unsigned subind, double t, double h, float inc, unsigned numero,
	   double *y, double (*k)[4], elemento *elems);
 int Temporal_por_mallas (unsigned n_elems, elemento *elems,
			  unsigned n_ramas, rama *ramas,
			  unsigned n_ecs, char (*ecs)[100]);
 int Temporal_por_nudos  (unsigned n_elems, elemento *elems,
			  unsigned n_ramas, rama *ramas,
			  unsigned n_ecs, char (*ecs)[100]);
 int calcular_derivadas (unsigned num, double h, float inc,
			 double *y, double (*k)[4], elemento *elems,
			 unsigned *fg);

 /* Inicializamos los flags de terminaci¢n */
 Fin = Abort = 0;

 /* Extraemos las respuestas temporales (tipo 'T') del fichero */
 n_resp = recupera_respuestas ('T', &resp);

 /* Si no queremos ninguna respuesta, simplemente regresamos */
 if (n_resp == 0)
   return;

 /* Obtenemos el nombre del fichero */
 strcpy (nom, nom_fich);
 strcat (nom, ".TMP");

 /* Nos aseguramos de que no tendr  nada en un principio */
 temp = fopen (nom, "wb");
 fclose (temp);

 /* Obtenemos las respuestas de tensi¢n y las de corriente */
 n_respV = n_respI = 0;
 for (i = 0; i < n_resp; ++i)
   if (resp[i].tipo == 'V')
     ++n_respV;
   else
     ++n_respI;

 /* Reservamos el espacio necesario para las tablas de componentes */
 compV = (int *) calloc (n_respV, 2 * sizeof (int));
 memset (compV, 0, n_respV * 2 * sizeof (int));

 compI = (int *) calloc (n_respI, n_ecs_Z * sizeof (int));
 memset (compI, 0, n_respI * n_ecs_Z * sizeof (int));

 /* El array de enteros apuntado por 'comp?', siendo ? una V ¢ una I, contiene
    los componentes del resultado, es decir, i, j, k,... donde Vm = Vi - Vj ¢
    Im = Ii ñ Ij ñ Ik ... */

 /* Primero, obtenemos las expresiones de las medidas de tensi¢n */
 b = 0;
 for (i = 0; i < n_resp; ++i)
   if (resp[i].tipo == 'V')
    {
     /* Colocamos los n£meros correspondientes a las componentes en la tabla */
     if (resp[i].medida.nodos[0] != 0)
      {
       *(compV + b*2)     =  resp[i].medida.nodos[0];
       *(compV + b*2 + 1) = -resp[i].medida.nodos[1];
      }
     else
       *(compV + b*2)     = -resp[i].medida.nodos[0];

     /* Apuntamos a la siguiente fila de 'compV' */
     ++b;
    }

 /* A continuaci¢n, las expresiones de las medidas de corriente */
 b = 0;
 for (i = 0; i < n_resp; ++i)
   if (resp[i].tipo == 'I')
    {
     /* Obtenemos las mallas en las que se encuentra el elemento */
     a = 0;       /* 'j' apunta la posici¢n actual en la lista de componentes */

     /* Recorremos todas las mallas */
     for (j = 0; j < n_ecs_Z; ++j)

      /* Si el elemento de medida est  en alguna de ellas.. */
      if ((pt = strstr (ecs_Z[j], resp[i].medida.elem)) != NIL)
       {
	/* ..obtenemos el signo de la corriente por la rama */
	while (*(pt--) != '(');

	/* Incorporamos la componente con su signo */
	if (*pt == '-')
	  *(compI + b*n_ecs_Z + a) = -j - 1; /* Es necesario a¤adir 1 al n§ de malla para */
	else                                 /* homogeneizar este formato con el usado en */
	  *(compI + b*n_ecs_Z + a) = j + 1;  /* una tensi¢n entre nudos.                  */

	++a;            /* Apuntamos a la posici¢n de la siguiente componente */
       }

     /* Apuntamos a la siguiente fila de 'compI' */
     ++b;
    }

 /* Contamos el n£mero de condensadores y generadores dependientes y adem s
    averiguamos si hay bobinas en el circuito */
 gendep = cond = 0;

 factor = 1;

 for (a = 0; a < n_elems; ++a)
  {
   if (elems[a].tipo[0] == 'L')
     factor = 2;
   else
     if (((elems[a].tipo[0] >= 'A') && (elems[a].tipo[0] <= 'E') &&
	  (elems[a].tipo[0] != 'C')) || (elems[a].tipo[0] == 'Q'))
       ++gendep;
     else
       if (elems[a].tipo[0] == 'C')
	 ++cond;
  }

 /* Este bucle se realiza dos veces, una con las respuestas tipo V y otra
    con las respuestas tipo I */
 for (m = 0; m < 2; ++m)
 {
  /* Escogemos las respuestas de tipo tensi¢n ¢ de tipo corriente */
  if (m == 0)
   {
    n_resp   = n_respV;
    num_comp = 2;
    comp     = compV;
   }
  else
   {
    n_resp   = n_respI;
    num_comp = n_ecs_Z;
    comp = compI;
   }

  if (n_resp > 0)
   {
    /* 'nv' significa 'n§ de variables' y su valor es 2*orden + gendep, si en
       el circuito hay bobinas, u orden + gendep en caso contrario, ya que si no
       hay bobinas, la matriz de C's ser  (0) */

    nv = factor*((m == 1) ? n_ecs_Z : (n_ecs_Y + cond)) + gendep;

    /* Creamos la matriz necesaria para calcular la respuesta temporal */
    y = (double *) calloc (8, nv * sizeof (double));
    mem_local += LocalSize (LocalHandle ((WORD) y));

    /* Calculamos el efecto de la temperatura */
    if (resp[0].temp != Tact)
     {
      Tact = resp[0].temp;
      temperatura (Tact, n_elems, elems);
      cond_inic_por_mallas (nv, n_elems, elems, n_ramas, ramas,
			    n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);
      Fin = 0;
      cond_inic_por_nudos  (nv, n_elems, elems, n_ramas, ramas,
			    n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);
     }

    /* Pasamos del circuito real al circuito para c lculo de la respuesta
       temporal, transformando los diodos en gens. no lineales (tipo "Shockley")
       de I dependientes de V, y despuŠs se construye el sistema de ecuaciones
       pas ndolo a forma diferencial */
    if (m == 1)
     {
      /* Obtenemos las condiciones iniciales en 'y' */
      if (recupera_cond_inic ('Z', nv, &y))
	Fin = 1;
      else
       {
	convertir (A_MALLAS, n_elems, elems, n_ramas, ramas,
		   n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

	Temporal_por_mallas (n_elems, elems, n_ramas, ramas, n_ecs_Z, ecs_Z);
       }
     }
    else
     {
      /* Obtenemos las condiciones iniciales en 'y' */
      if (recupera_cond_inic ('Y', nv, &y))
	Fin = 1;
      else
       {
	convertir (A_NUDOS, n_elems, elems, n_ramas, ramas,
		   n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

	Temporal_por_nudos  (n_elems, elems, n_ramas, ramas, n_ecs_Y, ecs_Y);

	/* Apa¤amos las condiciones iniciales para que las corrientes por los
	   condensadores est‚n entre las tensiones de nudo y las variables de
	   los generadores */
	for (i = orden + gendep - 1; i >= orden; --i)
	 {
	  y[i] = y[i-cond];
	  y[i-cond] = 0;
	 }
       }
     }

    if (!Fin)
     {
      /* Creamos el resto de matrices necesarias para el algoritmo */
      hw  = GlobalAlloc (GMEM_MOVEABLE,
			 (orden + gendep) * (orden+gendep+1) * sizeof (double));
      w   = (double huge *) GlobalLock (hw);
      pr  = (double *) malloc (nv * sizeof (double));
      k   = (double (*)[4]) calloc (nv, 4 * sizeof (double));

      mem_global += GlobalSize (hw);
      mem_local  += LocalSize (LocalHandle ((WORD) pr)) +
		    LocalSize (LocalHandle ((WORD) k));

      /* Enviamos a la ventana lo que estamos haciendo */
      strcpy (frase2, "Respuesta: Temporal\0");
      strcpy (frase3, "Puntos a calcular: Aun no lo se\x0");
      strcpy (frase4, "Puntos calculados: Calculando paso..\x0");
      strcpy (frase5, "0\x0");

      InvalidateRect (hwnd, NULL, TRUE);
      UpdateWindow (hwnd);

      /* Recordamos el instante de comienzo del c lculo de los 4 primeros puntos */
      comienzo = clock();

      /* Obtenemos el l¡mite del tama¤o de paso */
      H = (resp[0].fin - resp[0].ini) / resp[0].pasos;

      /* A continuaci¢n aplicamos Runge-Kutta de 4§ orden. Observar que comenzamos
	 el an lisis en t = t1, pero el 1er punto que obtengamos ser  el valor en
	 t = t1 + h ya que el valor en t = t1 viene dado por las cond. inic. */
      for (i = 1, t = resp[0].ini; i < 4; ++i, t += h)
       {
	/* Calculamos las k1 correspondientes a todas las variables */
	for (a = 0; a < orden + gendep; ++a)
	  *J(a, orden + gendep) = f(a, t, h,   0, 0, y + (i-1)*nv, k, elems);
	calcular_derivadas (0, h,   0, y + (i-1)*nv, k, elems, fg);
	for (a = orden + gendep; a < nv; ++a)
	  k[a][0] = f(a, t, h,   0, 0, y + (i-1)*nv, k, elems);

	/* A partir de las k1 calculamos las k2 para todas las variables */
	for (a = 0; a < orden + gendep; ++a)
	  *J(a, orden + gendep) = f(a, t, h, 0.5, 1, y + (i-1)*nv, k, elems);
	calcular_derivadas (1, h, 0.5, y + (i-1)*nv, k, elems, fg);
	for (a = orden + gendep; a < nv; ++a)
	  k[a][1] = f(a, t, h, 0.5, 1, y + (i-1)*nv, k, elems);

	/* A partir de las k2 calculamos las k3 */
	for (a = 0; a < orden + gendep; ++a)
	  *J(a, orden + gendep) = f(a, t, h, 0.5, 2, y + (i-1)*nv, k, elems);
	calcular_derivadas (2, h, 0.5, y + (i-1)*nv, k, elems, fg);
	for (a = orden + gendep; a < nv; ++a)
	  k[a][2] = f(a, t, h, 0.5, 2, y + (i-1)*nv, k, elems);

	/* Y a partir de las k3 obtenemos las k4 */
	for (a = 0; a < orden + gendep; ++a)
	  *J(a, orden + gendep) = f(a, t, h,   1, 3, y + (i-1)*nv, k, elems);
	calcular_derivadas (3, h,   1, y + (i-1)*nv, k, elems, fg);
	for (a = orden + gendep; a < nv; ++a)
	  k[a][3] = f(a, t, h,   1, 3, y + (i-1)*nv, k, elems);

	/* Finalmente aplicamos la f¢rmula del m‚todo R-K de 4§ orden */
	for (a = 0; a < nv; ++a)
	  y(i,a) = y(i-1,a) + h_6 * (k[a][0] + 2*k[a][1] + 2*k[a][2] + k[a][3]);
       }

      /* Calculamos los tiempos de CLK que se tarda en calcular 1 punto */
      final = (clock() - comienzo) / 3;

      /* Este bucle calcula la longitud de paso m xima para un error del 5% */
      do
      {
       /* Inicializamos el flag que se¤ala si superamos el error por paso m ximo */
       error_tpp = 0;

       /* Ahora empezamos con el Adams-Bashforth usando los puntos anteriores */
       for (i = 4, t = resp[0].ini + 3*h; (i < 7) && (num_iters < 25); ++i, t += h)
       {
	/* Obtenemos el predictor en pr e y[i]. Para ello, primero se obtienen las
	   derivadas para t, t-h, t-2h y t-3h en k[][0], k[][1], k[][2] y k[][3] */
	for (a = 0; a < orden + gendep; ++a)
	  *J(a, orden + gendep) = f (a, t, h,  0, 0, y + (i-1)*nv, k, elems);
	calcular_derivadas (0, h,  0, y + (i-1)*nv, k, elems, fg);
	for (a = orden + gendep; a < nv; ++a)
	  k[a][0] = f (a, t, h,  0, 0, y + (i-1)*nv, k, elems);

	for (a = 0; a < orden + gendep; ++a)
	  *J(a, orden + gendep) = f (a, t, h, -1, 0, y + (i-2)*nv, k, elems);
	calcular_derivadas (1, h, -1, y + (i-2)*nv, k, elems, fg);
	for (a = orden + gendep; a < nv; ++a)
	  k[a][1] = f (a, t, h, -1, 0, y + (i-2)*nv, k, elems);

	for (a = 0; a < orden + gendep; ++a)
	  *J(a, orden + gendep) = f (a, t, h, -2, 0, y + (i-3)*nv, k, elems);
	calcular_derivadas (2, h, -2, y + (i-3)*nv, k, elems, fg);
	for (a = orden + gendep; a < nv; ++a)
	  k[a][2] = f (a, t, h, -2, 0, y + (i-3)*nv, k, elems);

	for (a = 0; a < orden + gendep; ++a)
	  *J(a, orden + gendep) = f (a, t, h, -3, 0, y + (i-4)*nv, k, elems);
	calcular_derivadas (3, h, -3, y + (i-4)*nv, k, elems, fg);
	for (a = orden + gendep; a < nv; ++a)
	  k[a][3] = f (a, t, h, -3, 0, y + (i-4)*nv, k, elems);

	/* Finalmente calculamos el predictor a partir de las derivadas */
	for (a = 0; a < nv; ++a)
	 pr[a] = y(i-1,a) + h*(2.291666667*k[a][0] -
			       2.458333333*k[a][1] +
			       1.541666667*k[a][2] -
				     0.375*k[a][3]);

	memcpy (y + i*nv, pr, nv * sizeof (double));

	/* Aplicamos iterativamente el corrector. El resultado de cada iteraci¢n
	   se almacena temporalmente en y[7], y al final de la misma se traslada
	   a la posici¢n que le corresponde (la posici¢n 'i') */

	num_iters = 0;

	do
	{
	 /* Liberamos la memoria antes de entrar en el bucle de mensaje */
	 GlobalUnlock (hw);

	 /* Procesamos los mensajes que haya en la cola */
	 while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	 {
	  TranslateMessage (&msg);
	  DispatchMessage (&msg);
	 }

	 /* Rebloqueamos la memoria */
	 w = (double huge *) GlobalLock (hw);

	 /* Incrementamos el n£mero de iteraciones */
	 ++num_iters;

	 /* Reseteamos el flag 'error' */
	 error = 0;

	 /* Calculamos los resultados de la siguiente iteraci¢n */
	 for (a = 0; a < orden + gendep; ++a)
	   *J(a, orden + gendep) = f (a, t, h,  1, 0, y +     i*nv, k, elems);
	 calcular_derivadas (0, h,  1, y +     i*nv, k, elems, fg);
	 for (a = orden + gendep; a < nv; ++a)
	   k[a][0] = f (a, t, h,  1, 0, y +     i*nv, k, elems);

	 for (a = 0; a < orden + gendep; ++a)
	   *J(a, orden + gendep) = f (a, t, h,  0, 0, y + (i-1)*nv, k, elems);
	 calcular_derivadas (1, h,  0, y + (i-1)*nv, k, elems, fg);
	 for (a = orden + gendep; a < nv; ++a)
	   k[a][1] = f (a, t, h,  0, 0, y + (i-1)*nv, k, elems);

	 for (a = 0; a < orden + gendep; ++a)
	   *J(a, orden + gendep) = f (a, t, h, -1, 0, y + (i-2)*nv, k, elems);
	 calcular_derivadas (2, h, -1, y + (i-2)*nv, k, elems, fg);
	 for (a = orden + gendep; a < nv; ++a)
	   k[a][2] = f (a, t, h, -1, 0, y + (i-2)*nv, k, elems);

	 for (a = 0; a < orden + gendep; ++a)
	   *J(a, orden + gendep) = f (a, t, h, -2, 0, y + (i-3)*nv, k, elems);
	 calcular_derivadas (3, h, -2, y + (i-3)*nv, k, elems, fg);
	 for (a = orden + gendep; a < nv; ++a)
	   k[a][3] = f (a, t, h, -2, 0, y + (i-3)*nv, k, elems);

	 for (a = 0; a < nv; ++a)
	 {
	  y(7,a) = y(i-1,a) + h*(      0.375*k[a][0] +
				 0.791666667*k[a][1] -
				 0.208333333*k[a][2] +
				 0.041666667*k[a][3]);

	  /* Si superamos el error por paso m ximo, activamos el flag 'error' */
	  if (y(i,a) != 0)
	    if (fabs ((y(7,a) - y(i,a)) / y(7,a)) > MAXERR)
	      error = -1;
	 }

	 /* Los trasladamos a su posici¢n */
	 memcpy (y + i*nv, y + 7*nv, nv * sizeof (double));
	}

	/* Limitamos a 25 el n£mero de iteraciones */
	while (error && (num_iters < 25));

	/* Si no hemos conseguido el error m¡nimo, el FOR finalizar . Entonces,
	   copiamos el £ltimo resultado obtenido a la fila 6 de 'y' */
	if (num_iters == 25)
	  memcpy (y + 6*nv, y + i*nv, nv*sizeof (double));
       }

       /* Si el error de truncamiento por paso es m s del 5%, activar 'error_tpp' */
       error_tpp = 0;
       for (a = 0; a < nv; ++a)
	{
	 if (y(6,a) != 0)
	   if (fabs ((pr[a] - y(6,a)) / (5 * y(6,a))) > MAXERR)
	     error_tpp = -1;
	}

       /* Si no se ha superado el error del 5% en los pasos anteriores... */
       if (!error_tpp)
	{
	 /* ...duplicamos el tama¤o de paso */
	 h *= 2;

	 /* y trasladamos las respuestas hacia atr s para quedarnos con 4 */
	 memcpy (y +   nv, y + 2*nv, nv * sizeof(double));
	 memcpy (y + 2*nv, y + 4*nv, nv * sizeof(double));
	 memcpy (y + 3*nv, y + 6*nv, nv * sizeof(double));
	}

       /* Pero, si se ha superado el 5%... */
       else
	{
	 /* ...dividimos el tama¤o de paso y deshacemos los puntos anteriores */
	 t -= 3*h;
	 h /= 2;

	 /* y calculamos las respuestas anteriores interpolando linealmente */
	 for (a = 0; a < nv; ++a)
	   y(0,a) = (y(1,a) + y(2,a)) / 2;

	 memcpy (y + nv, y + 2*nv, nv * sizeof (double));

	 for (a = 0; a < nv; ++a)
	   y(2,a) = (y(1,a) + y(3,a)) / 2;
	}
      }
      while (!error_tpp && (2*h <= H) && !Fin);

      /* Calculamos el n£mero de pasos necesarios y la cantidad h/6 */
      n_pasos = ((float)(resp[0].fin - resp[0].ini) / h);
      resp[0].pasos = min (n_pasos, 65535);
      h_6 = h / 6;

      /* Una vez calculado el paso real, lo grabamos en el fichero, junto con
	 otros datos. Para ello, si estamos en la 1¦ pasada.. */
      if (m == 0)
       {
	/* ..abrimos el fichero de resultados para escritura */
	temp = fopen (nom, "wb");

	/* Guardamos los datos de los rangos etc. */
	fwrite (&resp[0].escala, sizeof (char), 1, temp);
	fwrite (&resp[0].ini, sizeof (float), 1, temp);
	fwrite (&resp[0].fin, sizeof (float), 1, temp);
	fwrite (&resp[0].pasos, sizeof (unsigned), 1, temp);

	/* Obtenemos las condiciones iniciales por nudos */
	recupera_cond_inic ('Y', nv, &y);

	/* Las apa¤amos para que las corrientes por los condensadores est‚n entre
	   las tensiones de nudo y las variables de los generadores */
	for (i = orden + gendep - 1; i >= orden; --i)
	 {
	  y[i] = y[i-cond];
	  y[i-cond] = 0;
	 }
       }
      else
       {
	/* ..abrimos el fichero para a¤adir */
	temp = fopen (nom, "ab");

	/* Guardamos los datos de los rangos etc., si es que la pasada
	   anterior del bucle (m = 0) no lo ha hecho */
	if (n_respV == 0)
	 {
	  fwrite (&resp[0].escala, sizeof (char), 1, temp);
	  fwrite (&resp[0].ini, sizeof (float), 1, temp);
	  fwrite (&resp[0].fin, sizeof (float), 1, temp);
	  fwrite (&resp[0].pasos, sizeof (unsigned), 1, temp);
	 }

	/* Obtenemos las condiciones iniciales por mallas */
	recupera_cond_inic ('Z', nv, &y);
       }

      /* Calculamos las respuestas en t = 0 a partir de las condiciones iniciales */
      for (b = 0; b < n_resp; ++b)
       {
	for (a = 0; a < num_comp; ++a)
	 {
	  if (*(comp + b*num_comp + a) > 0)
	    v += y[*(comp + b*num_comp + a) - 1];
	  else
	    if (*(comp + b*num_comp + a) < 0)
	      v -= y[-*(comp + b*num_comp + a) - 1];
	 }

	/* Las guardamos en el fichero */
	fseek (temp, sizeof (char) + 2*sizeof (float) + sizeof (unsigned) +
		     (m * n_respV + b) * n_pasos * sizeof (double), SEEK_SET);
	fwrite (&v, sizeof (double), 1, temp);
       }

      /* Enviamos a la ventana el n§ de pasos a calcular */
      strcpy (frase3, "Puntos a calcular: \x0");
      strcat (frase3, ltoa (n_pasos, num, 10));
      GetClientRect (hwnd, &rect);
      rect.top    = 5 * cyLine + nVposScrl * SCRLDESPL;
      rect.bottom = rect.top + cyLine - 1;
      InvalidateRect (hwnd, &rect, TRUE);

     // printf ("\n\nTengo que calcular %u pasos. Voy a tardar %5.3f.",
     //	   resp[0].pasos, resp[0].pasos*final/(3600*CLK_TCK));

      /* Una vez hallado el tama¤o del paso, calculamos la respuesta mediante
	 el m‚todo Runge-Kutta de 4§ orden */
      for (t = resp[0].ini, i = 1; (t <= resp[0].fin) && (!Fin); t += h, ++i)
       {
	/* Enviamos a la ventana el n§ de paso */
	strcpy (frase4, "Puntos calculados: \x0");
	strcat (frase4, ltoa (t/h, num, 10));
	strcpy (frase5, itoa ((t/h) / n_pasos * 100, num, 10));

	if (minimizada)
	  InvalidateRect (hwnd, NULL, FALSE);
	else
	 {
	  GetClientRect (hwnd, &rect);
	  rect.top    = 7 * cyLine - nVposScrl * SCRLDESPL;
	  rect.bottom = rect.top + 4*cyLine - 1;
	  InvalidateRect (hwnd, &rect, FALSE);
	 }

	 /* Liberamos la memoria antes de entrar en el bucle de mensaje */
	 GlobalUnlock (hw);

	/* Recogemos y procesamos los mensajes que haya en la cola */
	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	 {
	  TranslateMessage (&msg);
	  DispatchMessage  (&msg);
	 }

	 /* Rebloqueamos la memoria */
	 w = (double huge *) GlobalLock (hw);

	if (!Fin)
	 {
	  /* Calculamos las k1 correspondientes a todas las variables */
	  for (a = 0; a < orden + gendep; ++a)
	    *J(a, orden + gendep) = f(a, t, h,   0, 0, y, k, elems);
	  calcular_derivadas (0, h,   0, y, k, elems, fg);
	  for (a = orden + gendep; a < nv; ++a)
	    k[a][0] = f(a, t, h,   0, 0, y, k, elems);

	  /* A partir de las k1 calculamos las k2 para todas las variables */
	  for (a = 0; a < orden + gendep; ++a)
	    *J(a, orden + gendep) = f(a, t, h, 0.5, 1, y, k, elems);
	  calcular_derivadas (1, h, 0.5, y, k, elems, fg);
	  for (a = orden + gendep; a < nv; ++a)
	    k[a][1] = f(a, t, h, 0.5, 1, y, k, elems);

	  /* A partir de las k2 calculamos las k3 */
	  for (a = 0; a < orden + gendep; ++a)
	    *J(a, orden + gendep) = f(a, t, h, 0.5, 2, y, k, elems);
	  calcular_derivadas (2, h, 0.5, y, k, elems, fg);
	  for (a = orden + gendep; a < nv; ++a)
	    k[a][2] = f(a, t, h, 0.5, 2, y, k, elems);

	  /* Y a partir de las k3 obtenemos las k4 */
	  for (a = 0; a < orden + gendep; ++a)
	    *J(a, orden + gendep) = f(a, t, h,   1, 3, y, k, elems);
	  calcular_derivadas (3, h,   1, y, k, elems, fg);
	  for (a = orden + gendep; a < nv; ++a)
	    k[a][3] = f(a, t, h,   1, 3, y, k, elems);

	  /* Finalmente aplicamos la f¢rmula del m‚todo R-K de 4§ orden */
	  for (a = 0; a < nv; ++a)
	    y[a] += h_6 * (k[a][0] + 2*k[a][1] + 2*k[a][2] + k[a][3]);

	  /* Calculamos el valor de la respuesta y lo guardamos */
	  for (b = 0; b < n_resp; ++b)
	   {
	    v = 0;
	    for (a = 0; a < num_comp; ++a)
	     {
	      if (*(comp + b*num_comp + a) > 0)
		v += y[*(comp + b*num_comp + a) - 1];
	      else
		if (*(comp + b*num_comp + a) < 0)
		  v -= y[-*(comp + b*num_comp + a) - 1];
	     }

	    fseek (temp, sizeof(char) + 2*sizeof(float) + sizeof(unsigned) +
			 ((m * n_respV + b) * n_pasos + i) * sizeof (double),
			 SEEK_SET);
	    fwrite (&v, sizeof (double), 1, temp);
	   }
	 }
       }

      /* Si hemos terminado prematuramente, actualizamos el n§ de muestras */
      if (Fin)
       {
	/* Obtenemos los nuevos datos */
	fin = t;
	pasos = t / h;

	/* Nos colocamos en la posici¢n del instante final */
	fseek (temp, sizeof (resp[0].escala) + sizeof (resp[0].ini), SEEK_SET);

	/* Guardamos el instante al que hayamos llegado */
	fwrite (&fin, sizeof (fin), 1, temp);

	/* Guardamos el n£mero de puntos calculados */
	fwrite (&pasos, sizeof (pasos), 1, temp);
       }

      /* Cerramos el fichero */
      fclose (temp);

      /* Liberamos el espacio utilizado */
      mem_global -= GlobalSize (hw);
      mem_local  -= LocalSize (LocalHandle ((WORD)pr)) +
		    LocalSize (LocalHandle ((WORD) k));

      GlobalFree (hw);
      free (pr);
      free (k);
     }

    /* Liberamos el espacio utilizado */
    mem_global -= GlobalSize (hr) + GlobalSize (hl) + GlobalSize (hc);
    mem_local  -= LocalSize (LocalHandle ((WORD) y)) +
		  LocalSize (LocalHandle ((WORD)fg)) +
		  LocalSize (LocalHandle ((WORD) g));

    GlobalFree (hr);
    GlobalFree (hl);
    GlobalFree (hc);
    free (y);
    free (fg);
    free (g);
   }
 }

 free (compV);
 free (compI);
 free (resp);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

int calcular_derivadas (unsigned num, double h, float inc,
			double *y, double (*k)[4], elemento *elems,
			unsigned *fg)

/* Esta funci¢n calcula los valores de las derivadas de las variables del
   circuito, usando la matriz apuntada por 'w' para calcular los coeficientes
   que afectan a las derivadas y diagonalizar la matriz de dichos coefs.
   Tenemos en 'y' los valores anteriores de todas las variables, y en los
   t‚rminos independientes de 'w', -Rúi - Cúi + E.
   Los resultados los pondremos en la columna 'num' de 'k' */
{
 double huge *safe;
 double      var, aux;
 int         error, i, j;
 BOOL        sing;

 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);
 extern double huge *J(unsigned i, unsigned j);

 /* Copiamos la matriz 'l' a la 'w' */
 for (i = 0; i < orden + gendep; ++i)
   _fmemcpy (w + i*(orden + gendep + 1), l + i*(orden + gendep),
	     (orden + gendep) * sizeof (double));

 /* Particularizamos los t‚rminos de los generadores dependientes */
 for (i = 0; i < orden + gendep; ++i)
  for (j = orden; j < orden + gendep; ++j)
   {
    if (i != j)
     {
      aux = *J(i,j);
      if (aux != 0)
       {
	var = y[j];
	if (num > 0)
	  var += h * inc * k[j][num - 1];
	*J(i, j) = aux * valor_gen (fg[j - orden], var, 0, 0, 0, 0, 1, elems);
       }
     }
   }

 /* Diagonalizamos la matriz 'w' */
 safe = r;
 r = w;
 sing = diagonaliza_real (orden + gendep, orden + gendep);
 r = safe;

 /* Calculamos los resultados */
 for (i = 0; i < orden + gendep; ++i)
  {
   /* Si la matriz no ha resultado ser singular, obtenemos los nuevos valores */
   if (!sing)
     k[i][num] = *J(i, orden + gendep) / *J(i, i);

   /* pero, si lo ha sido, haremos los nuevos valores iguales a los anteriores */
   else
     k[i][num] = k[i][(num > 0) ? num - 1 : 3];
  }

 /* Devolvemos 0 */
 return (0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

polinomio interpolar (unsigned i, unsigned ult, unsigned nv,
		      double h, double *y)

/* Realiza una interpolaci¢n de Lagrange de 3er orden para las muestras de la
   soluci¢n de la ec. 'i' (columna 'i'), siendo 'ult' la £ltima muestra */
{
 polinomio p;
 double h_3 = h*h*h,
	K0, K1, K2, K3;

 K0 = y(ult-3,i) / (-6*h_3);
 K1 = y(ult-2,i) /  (2*h_3);
 K2 = y(ult-1,i) / (-2*h_3);
 K3 = y(ult  ,i) /  (6*h_3);

 p.grado = 3;
 p.coef[3] =           K0 +   K1 +   K2 +   K3;
 p.coef[2] =  -h * ( 6*K0 + 5*K1 + 4*K2 + 3*K3);
 p.coef[1] = h*h * (11*K0 + 6*K1 + 3*K2 + 2*K3);
 p.coef[0] = y(ult-3,i);

 return (p);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

double f (unsigned subind, double t, double h, float inc, unsigned numero,
	  double *y, double (*k)[4], elemento *elems)

/* Funci¢n que eval£a la pendiente en t + inc*h de la variable n§ 'subind'
   (ecuaci¢n n£mero 'subind' del sistema), utilizando las k anteriores */
{
 gen generador;
 double x = 0, R, C;
 unsigned i, j, despl;

 extern double valor_gen (unsigned i, double v, double v1,
			  double t, double h, float inc,
			  unsigned deriv, elemento *elems);

 despl = orden + gendep;

 /* Si la ecuaci¢n es una de las del cambio de variable.. */
 if (subind >= orden + gendep)
  {
   /* ..la expresi¢n a calcular es f      (i1,i2,..,j1,j2..) = i
				    subind                      subind */
   x = y[subind-despl];
   if (numero > 0)
     x += h * inc * k[subind-despl][numero-1];
  }

 /* Y, si no, calculamos la expresi¢n normalmente */
 else
  {
   /* Hacemos la suma de los coeficientes de las i's y j's */
   for (i = 0, j = despl; i < orden; ++i, ++j)
   {
    R = *(r + despl*subind + i);
    C = *(c + despl*subind + i);
    x += R * (y[i] + ((numero > 0) ? h * inc * k[i][numero-1] : 0));
    if (C != 0)
      x += C * (y[j] + ((numero > 0) ? h * inc * k[j][numero-1] : 0));
   }

   /* A¤adimos los t‚rminos correspondientes a los generadores dependientes */
   for (i = orden; i < orden + gendep; ++i)
   {
    R = *(r + despl*subind + i);
    if (R != 0)
      if (numero == 0)
	x += R * valor_gen (fg[i - orden], y[i], 0,
			    t, h, inc, 0, elems);
      else
	x += R * valor_gen (fg[i - orden], y[i] + h * inc * k[i][numero-1], 0,
			    t, h, inc, 0, elems);
   }

   /* Finalmente a¤adimos la se¤al de los generadores independientes */
   generador.puntero = 1;
   for (i = 0; (generador.puntero != 0) && (i < 10); ++i)
   {
    generador = (*(g + subind))[i];
    if (generador.puntero != 0)
     {
      if (generador.coef[0] != 0)
	x += generador.coef[0] * valor_gen (generador.puntero-1, 0, 0, t, h, inc,
					    0, elems);
      if (generador.coef[1] != 0)
	x += generador.coef[1] * valor_gen (generador.puntero-1, 0, 0, t, h, inc,
					    1, elems);
      if (generador.coef[2] != 0)
	x += generador.coef[2] * valor_gen (generador.puntero-1, 0, 0, t, h, inc,
					    2, elems);
     }
   }
  }

 return (x);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/