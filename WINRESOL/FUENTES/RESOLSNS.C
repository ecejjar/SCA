#include <windows.h>
#include <resolver.h>

extern short    Fin, Abort;
extern char     frase2[], frase3[], frase4[], frase5[];
extern unsigned orden, gendep, gendepv, gendepi;
extern long     mem_global;
extern unsigned mem_local;
extern char     nom_fich[];

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

void sensibilidad (unsigned n_elems, elemento *elems,
		   unsigned n_ramas, rama *ramas,
		   unsigned n_ecs_Z, char (*ecs_Z)[100],
		   unsigned n_ecs_Y, char (*ecs_Y)[100])

/* Funci¢n que realiza el an lisis de sensibilidad del circuito (variaciones
   en las se¤ales debidas a variaciones en los valores de los elementos) y
   almacena los resultados pedidos en el fichero de extensi¢n '.SNS' */
{
 FILE      *sens;
 resultado *resp = NIL;
 double    v = 0, leido, h, *yZ = NIL, *yY = NIL;
 unsigned  elem, n_resp, i, j, a, b, nv, factor, Z, Y;
 char      nom[84], nombre_elem[5], *pt;

 extern unsigned recupera_respuestas (char tipo, resultado **respuestas);
 extern unsigned recupera_cond_inic (char tipo,
				     unsigned no_vars, double **cond_inic);
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

 /* 'nv' significa 'n§ de variables' y su valor es 2*orden + gendep, si en
    el circuito hay bobinas, u orden + gendep en caso contrario, ya que si no
    hay bobinas, la matriz de C's ser  (0) y no habr  cambios de variable */

 gendep = 0;

 factor = 1;

 for (a = 0; a < n_elems; ++a)
  {
   if (elems[a].tipo[0] == 'L')
     factor = 2;
   else
     if (((elems[a].tipo[0] >= 'A') && (elems[a].tipo[0] <= 'E') &&
	  (elems[a].tipo[0] != 'C')) || (elems[a].tipo[0] == 'Q'))
       ++gendep;
  }

 /* Extraemos las respuestas de sensibilidad (tipo 'S') del fichero */
 n_resp = recupera_respuestas ('S', &resp);

 /* Si no queremos ninguna respuesta, simplemente regresamos */
 if (n_resp == 0)
   return;

 /* Localizamos el elemento respecto de cuyo valor queremos analizar la
    sensibilidad */
 nombre_elem[0] = resp[0].escala;
 sprintf (&nombre_elem[1], "%03u", resp[0].pasos);
 elem = buscar_elem (n_elems, elems, nombre_elem);

 /* Calculamos el paso entre puntos h, que ser  el 1% del valor del elemento */
 h = 0.01 * elems[elem].valor;

 /* Abrimos el fichero que contendr  los resultados */
 strcpy (nom, nom_fich);
 strcat (nom , ".SNS");
 sens = fopen (nom, "+wb");

 /* Calcularemos dos puntos de la funci¢n; f(x-h) y f(x+h) */
 elems[elem].valor -= h;
 for (b = 0; (b < 2) && !Fin; ++b)
 {
  /* Hacemos generadores cont¡nuos todos los generadores independientes */
  for (i = 0; i < n_elems; ++i)
    if ((elems[i].tipo[0] == 'V') || (elems[i].tipo[0] == 'I'))
      elems[i].signal = 'C';

  /* Calculamos las condiciones iniciales */
  nv = factor*n_ecs_Z + gendep;
  cond_inic_por_mallas (nv, n_elems, elems, n_ramas, ramas,
			n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

  Fin = 0;

  nv = factor*n_ecs_Y + gendep;
  cond_inic_por_nudos  (nv, n_elems, elems, n_ramas, ramas,
			n_ecs_Z, ecs_Z, n_ecs_Y, ecs_Y);

  /* Las recuperamos */
  Z = recupera_cond_inic ('Z', nv, &yZ);
  Y = recupera_cond_inic ('Y', nv, &yY);

  /* Nos colocamos al principio del fichero de respuestas */
  fseek (sens, 0, SEEK_SET);

  /* Recorremos la tabla de respuestas */
  for (a = 0; a < n_resp; ++a)
   {
    /* Si la respuesta deseada es una V.. */
    if (resp[a].tipo == 'V')
     {
      /* ..obtenemos su valor */
      if (Y == 0)
	v = yY[resp[a].medida.nodos[0] - 1] - yY[resp[a].medida.nodos[1] - 1];
      else
	v = 0;
     }
    else
     {
      /* Inicializamos la var. que contendr  la corriente por el elemento */
      v = 0;

      if (Z == 0)
       {
	/* Obtenemos el valor de dicha corriente */
	/* Recorremos todas las mallas */
	for (i = 0; i < n_ecs_Z; ++i)

	 /* Si el elemento de medida est  en alguna de ellas.. */
	 if ((pt = strstr (ecs_Z[i], resp[a].medida.elem)) != NIL)
	  {
	   /* ..obtenemos el signo de la corriente por la rama */
	   while (*(pt--) != '(');

	   /* Incorporamos la corriente con su signo */
	   v += (*pt == '-') ? -yZ[i] : yZ[i];
	  }
       }
     }

    /* Calculamos el t‚rmino de la f¢rmula que resulta de derivar la
       f¢rmula del punto 5 de Lagrange */
    if (b == 0)
      v = -v;
    else
     {
      fread (&leido, sizeof (leido), 1, sens);
      v = (leido + v) / (2*h);
     }

    /* Guardamos la respuesta en el fichero */
    fwrite (&v, sizeof (v), 1, sens);

    /* Pasamos al segundo punto de la funci¢n */
    elems[elem].valor += 2*h;
   }
 }

 /* Cerramos el fichero de respuestas */
 fclose (sens);

 /* Liberamos el espacio que ya no nos es £til */
 mem_local -= LocalSize (LocalHandle ((WORD) yZ)) +
	      LocalSize (LocalHandle ((WORD) yY));
 free (yZ);
 free (yY);

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄFin del ficheroÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
