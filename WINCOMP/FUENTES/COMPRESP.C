#include "compilar.h"

/*----------------Zona de declaraci¢n de variables externas-----------------*/

extern FILE     *fuente;
extern reg_elem *elem_ini;
extern registro *rama_ini;
extern float    T;
extern unsigned errores;
extern char     nom_fich[80], c;

/*----------------Zona de declaraci¢n de funciones externas-----------------*/

extern size_t   mi_fread (void *, size_t, size_t, FILE *);
extern void     error (int codigo);
extern void     no_orden (reg_elem *primero, reg_elem *elem);
extern reg_elem *buscar (reg_elem *primero, char nombre[6]);
extern int      buscar_siguiente (char inf, char sup, char *c, short avance);
extern double   convertir (char *c);

/*------------Comienzo de la zona de definici¢n de funciones----------------*/

reg_medida *compilar_resp (void)

/* Compila la tercera y £ltima parte del fichero fuente */
{
 int        hFichero;
 OFSTRUCT   of;
 reg_elem   *elem_bus;
 registro   *rama_act = NIL;
 reg_medida *resp_ini = NIL, *resp_act = NIL;
 respuesta  resp;
 int        contador, comentario = 0;
 char       nom[84], num[4], name[6], through[6], parametro[15];
 BOOL       respproc[5];

 /* Ponemos todas las se¤ales de respuesta procesada a 0 */
 respproc[0] = respproc[1] = respproc[2] = respproc[3] = respproc[4] = 0;

 /* Abrimos el fichero que describir  los resultados deseados */
 strcpy (nom, nom_fich);
 *strrchr (nom, '.') = 0;
 strcat (nom, ".RES");
 hFichero = OpenFile (nom, &of, OF_CREATE | OF_SHARE_EXCLUSIVE);

 /* Recorremos la parte de petici¢n de resultados */
 comentario = 0;
 do
 {
  /* Localizamos el siguiente car cter v lido */
  do
  {
   mi_fread (&c, 1, 1, fuente);
   if (c == '/')
     comentario = -1;
   else
     if (c == 10)
       comentario = 0;
  }
  while ((c < 33) || (c > 122) || comentario);

  /* Si no es el de fin de fichero.. */
  if ((c != '#') && (!feof (fuente)))
   {
    /* Leemos el tipo de respuesta y sus l¡mites */
    memset (parametro, 0, 15);
    contador = 0;
    while ((c >= 'A') && (c <= 'Z') && (c != '='))
    {
     parametro[contador] = c;
     ++contador;
     mi_fread (&c, 1, 1, fuente);
    }

    /* Ponemos 'dominio' a 0, de modo que luego sepamos si el par metro */
    resp.dominio = 0;                                  /* es uno v lido */

    /* Obtenemos las medidas temporales */
    if ((strcmp (parametro, "TIEMPO") == 0) ||
	(strcmp (parametro, "PEQ_SE¥AL") == 0))
     {
      /* Comprobamos que no se haya procesado ninguna respuesta temporal y,
	 en caso afirmativo, apuntamos que la procesamos */
      if (respproc[1])
	error (E_DEFINDUPL);
      else
	respproc[1] = TRUE;

      /* Fijamos la escala y el dominio */
      resp.dominio = (strcmp (parametro, "TIEMPO") == 0) ? 'T' : 'S';
      resp.escala  = 'D';

      /* Obtenemos todos los par metros de la respuesta */
      buscar_siguiente ('=', '=', &c, NO_AVANZAR);

      buscar_siguiente ('(', '(', &c, AVANZAR);

      /* Obtenemos el instante inicial */
      resp.ini = convertir (&c);

      /* Obtenemos el instante final */
      resp.fin = convertir (&c);

      /* Obtenemos el n£mero m¡nimo de puntos */
      resp.pasos = convertir (&c);

      /* Avanzamos hasta el ')' */
      buscar_siguiente (')', ')', &c, NO_AVANZAR);
     }
    else
      if ((strcmp (parametro, "FRECUENCIA") == 0) ||
	  (strcmp (parametro, "FOURIER") == 0))
       {
	/* Comprobamos que no se haya procesado ninguna respuesta frec. y,
	   en caso afirmativo, apuntamos que la procesamos */
	if (strcmp (parametro, "FRECUENCIA") == 0)
	  if (respproc[2])
	    error (E_DEFINDUPL);
	  else
	    respproc[2] = TRUE;

	if (strcmp (parametro, "FOURIER") == 0)
	  if (respproc[3])
	    error (E_DEFINDUPL);
	  else
	    respproc[3] = TRUE;

	/* Fijamos el dominio */
	resp.dominio = (strcmp (parametro, "FRECUENCIA") == 0) ? 'F' : 'R';

	/* Obtenemos todos los par metros de la respuesta */
	buscar_siguiente ('=', '=', &c, NO_AVANZAR);

	buscar_siguiente ('(', '(', &c, AVANZAR);

	/* Obtenemos el instante inicial */
	resp.ini = convertir (&c);

	/* Obtenemos el instante final */
	resp.fin = convertir (&c);

	/* Obtenemos el n£mero m¡nimo de puntos */
	resp.pasos = convertir (&c);

	/* Obtenemos la escala */
	buscar_siguiente ('D', 'L', &c, NO_AVANZAR);

	if ((c == 'D') || (c == 'L'))
	  resp.escala = c;
	else
	  error (E_ESCALANOVALIDA);

	/* Avanzamos hasta el ')' */
	buscar_siguiente (')', ')', &c, AVANZAR);
       }
      else
	if (strcmp (parametro, "POLARIZACION") == 0)
	 {
	  /* Comprobamos que no se haya procesado ninguna respuesta de
	     polarizaci¢n y, caso afirmativo, apuntamos que la procesamos */
	  if (respproc[0])
	    error (E_DEFINDUPL);
	  else
	    respproc[0] = TRUE;

	  resp.dominio = 'P';
	 }
	else
	  if (strcmp (parametro, "SENSIBILIDAD") == 0)
	   {
	    /* Comprobamos que no se haya procesado ninguna respuesta de
	       sensibilidad y, caso afirmativo, apuntamos que la procesamos */
	    if (respproc[4])
	      error (E_DEFINDUPL);
	    else
	      respproc[4] = TRUE;

	    /* Obtenemos todos los par metros de la respuesta */
	    buscar_siguiente ('=', '=', &c, NO_AVANZAR);

	    buscar_siguiente ('(', '(', &c, AVANZAR);

	    /* Obtenemos el nombre del elemento respecto del cual se va a
	       calcular la sensibilidad */
	    buscar_siguiente (33, 122, &c, AVANZAR);
	    memset (name, 0, 6);
	    contador = 0;
	    while ((c >= 33) && (c <= 122) && (c != ')') && (contador < 5))
	    {
	     name[contador] = c;
	     ++contador;
	     mi_fread (&c, 1, 1, fuente);
	    }

	    /* Comprobamos que el nombre no exceda de 5 caracteres */
	    if ((c != ' ') && (c != ')'))
	      error (W_NOMMUYLARGO);

	    /* Nos aseguramos de que el elemento est‚ en la lista */
	    if ((elem_bus = buscar (elem_ini, name)) == NIL)
	      error (E_ELEMNOENCON);
	    else
	     {
	      /* Obtenemos el nombre interno del elemento */
	      strcpy (name, elem_bus->datos.tipo);

	      /* Guardamos el nombre del elemento de la siguiente forma;
		 - el primer car cter, indicativo del tipo, en 'escala'
		 - el n£mero de orden, convertido a entero en 'pasos' */
	      resp.escala = name[0];
	      resp.pasos = atoi (&name[1]);
	     }

	    /* Avanzamos hasta el ')' */
	    buscar_siguiente (')', ')', &c, NO_AVANZAR);

	    resp.dominio = 'S';
	   }
	  else
	    if (strcmp (parametro, "LAPLACE") == 0)
	     {
	      resp.dominio = 'L';
	     }
	    else
	      if (strcmp (parametro, "TEMP") == 0)
	       {
		/* Avanzamos hasta el '=' */
		buscar_siguiente ('=', '=', &c, NO_AVANZAR);

		/* Obtenemos la nueva temperatura de an lisis */
		T = convertir (&c);
	       }

    /* Si el par metro era v lido.. */
    if (resp.dominio != 0)
     {
      /* ..fijamos la temperatura de la medida */
      resp.temp = T;

      /* Avanzamos hasta el comienzo de las medidas */
      buscar_siguiente ('{', '{', &c, AVANZAR);

      do
      {
       /* Avanzamos hasta el comienzo de la medida (¢ el '}') */
       while (((c < 'A') || (c > 'Z')) && (c != '}'))
       {
	mi_fread (&c, 1, 1, fuente);
	if ((c < 'A') || (c > 'Z'))
	  if ((c != ' ') && (c != 10) && (c != '}'))
	    error (E_CARINESP);
       }

       /* Si no hemos llegado al '}'.. */
       if (c != '}')
	{
	 /* ..obtenemos el nombre de la medida (V, I, Vij, Ii) */
	 memset (parametro, 0, 15);
	 contador = 0;
	 while ((c >= 'A') && (c <= 'Z') && (contador < 3))
	 {
	  parametro[contador] = c;
	  ++contador;
	  mi_fread (&c, 1, 1, fuente);
	 }

	 if ((c != ' ') && (c != '('))
	   error (W_MEDMUYLARGA);

	 /* Avanzamos hasta el '(' */
	 buscar_siguiente ('(', '(', &c, NO_AVANZAR);

	 switch (parametro[0])
	 {
	  /* Si la medida es una tensi¢n.. */
	  case 'V':
	  {
	   resp.tipo = 'V';

	   /* Si la respuesta es una tensi¢n entre nudos.. */
	   if (strlen (parametro) == 1)
	    {
	     /* ..obtenemos los nodos entre los que se mide */
	     resp.medida.nodos[0] = convertir (&c);
	     resp.medida.nodos[1] = convertir (&c);
	    }

	   /* Pero si es una tensi¢n entre terminales de un modelo.. */
	   else
	    {
	     /* ..obtenemos el nombre del modelo */
	     memset (through, 0, 5);
	     contador = 0;
	     buscar_siguiente (33, 122, &c, AVANZAR);
	     while ((c >= 33) && (c <= 122) && (c != ')') && (contador < 5))
	      {
	       through[contador] = c;
	       ++contador;
	       mi_fread (&c, 1, 1, fuente);
	      }

	     /* Buscamos el modelo */
	     elem_bus = buscar (elem_ini, through);

	     /* Si el modelo existe, se le procesa */
	     if (elem_bus != NIL)
	      {
	       strcpy (num, &elem_bus->datos.tipo[1]);

	       /* Constru¡mos el nombre interno del elem. del modelo que se
		  encuentra entre los terminales especificados */
	       if ((parametro[1] == 'E') && (parametro[2] == 'C'))
		 strcpy (name, "I1\x0");
	       else
		 if ((parametro[1] == 'C') && (parametro[2] == 'E'))
		   strcpy (name, "I2\x0");
		 else
		   if ((parametro[1] == 'E') || (parametro[2] == 'E'))
		     strcpy (name, "Df\x0");
		   else
		     strcpy (name, "Dr\x0");

	       strcat (name, num);
	       strcpy (name, buscar (elem_ini, name)->datos.tipo);

	       /* Buscamos en qu‚ rama se encuentra para obtener los nudos */
	       for (rama_act = rama_ini;
		    strstr (rama_act->datos.elem, name) == NIL;
		    rama_act = rama_act->siguiente);

	       /* Obtenemos los nudos, seg£n el sentido de la medida */
	       if (parametro[1] == 'B')
		{
		 resp.medida.nodos[0] = rama_act->datos.nodoi;
		 resp.medida.nodos[1] = rama_act->datos.nodof;
		}
	       else
		 if (parametro[2] == 'B')
		  {
		   resp.medida.nodos[0] = rama_act->datos.nodof;
		   resp.medida.nodos[1] = rama_act->datos.nodoi;
		  }
		 else
		  {
		   resp.medida.nodos[0] = rama_act->datos.nodoi;
		   resp.medida.nodos[1] = rama_act->datos.nodof;
		  }
	      }
	     else
	       error (E_ELEMNOENCON);
	    }
	   break;
	  }

	  /* Si la medida es una corriente.. */
	  case 'I':
	  {
	   /* ..obtenemos el elemento en el que se mide */
	   resp.tipo = 'I';
	   memset (through, 0, 5);
	   contador = 0;
	   buscar_siguiente (33, 122, &c, AVANZAR);
	   while ((c >= 33) && (c <= 122) && (c != ')') && (contador < 5))
	    {
	     through[contador] = c;
	     ++contador;
	     mi_fread (&c, 1, 1, fuente);
	    }

	   if ((c != ' ') && (c != 10) && (c != ')'))
	     error (W_NOMMUYLARGO);

	   strcpy (resp.medida.elem, buscar (elem_ini, through)->datos.tipo);

	   break;
	  }

	  default:
	  {
	   error (E_MEDIDADESCONOC);
	   errores = NO_ERRORES;
	   buscar_siguiente (')', ')', &c, NO_AVANZAR);
	   errores = ERRORES;
	  }
	 }

	 /* Creamos un nuevo elemento en la lista de medidas */
	 if (resp_ini == NIL)
	   resp_ini = resp_act = (reg_medida *) malloc (sizeof (reg_medida));
	 else
	   resp_act =
	   resp_act->siguiente = (reg_medida *) malloc (sizeof (reg_medida));

	 /* Lo rellenamos con los datos de la £ltima medida procesada */
	 resp_act->datos = resp;
	 resp_act->siguiente = NIL;

	 /* Avanzamos al car cter siguiente al ')' */
	 buscar_siguiente (')', ')', &c, NO_AVANZAR);
	 mi_fread (&c, 1, 1, fuente);
	}
      }
      while (c != '}');
     }
   }
 }
 while ((c != '#') && (!feof (fuente)));

 /* Cerramos el fichero de respuestas */
 _lclose (hFichero);

 return (resp_ini);
}