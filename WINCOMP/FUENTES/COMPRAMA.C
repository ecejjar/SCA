#include "compilar.h"

/*----------------Zona de declaraci¢n de variables externas-----------------*/

extern FILE       *fuente;
extern reg_elem   *elem_ini;
extern reg_cuadri *cuad_ini;
extern unsigned   max_nudo,
		  errores, nErrores;
extern char       nom_fich[80],
		  c,
		  frase1[20], frase2[20], frase3[25], frase4[25], frase5[15],
		  frase6[15];

/*----------------Zona de declaraci¢n de funciones externas-----------------*/

extern size_t     mi_fread (void *, size_t, size_t, FILE *);
extern void       error (int codigo);
extern void       no_orden (reg_elem *primero, reg_elem *elem);
extern reg_elem   *buscar (reg_elem *primero, char nombre[5]);
extern reg_cuadri *buscarc (reg_cuadri *primero, char nombre[11]);
extern int        buscar_siguiente (char inf, char sup, char *c, short avance);
extern double     convertir (char *c);

/*------------Comienzo de la zona de definici¢n de funciones----------------*/

registro *compilar_ramas (void)

/* Compila la segunda secci¢n del fichero fuente (la de ramas y modelos).
   La variable global 'c' apuntar  al 1er car cter despu‚s del '#' de la
   secci¢n anterior */
{
 int        hFichero;
 OFSTRUCT   of;
 reg_elem   *elem_act = NIL, *elem_aux = NIL;
 reg_cuadri *cuad_act = NIL, *cuad_aux = NIL;
 registro   *rama_ini = NIL, *rama_act = NIL;
 int        nudos[4], ni, nf, lei, contador, comentario = 0,
	    *tabla_nudos;
 char       nom[84], e[50], aux[50], num[4], name[11], through[6], type,
	    parametro[15], numero[20], *pt;

 /* Reservamos espacio para la tabla de nudos que se utiliza en esta parte */
 tabla_nudos = (int *) malloc ((max_nudo + 1) * sizeof (int));

 /* Recorremos el resto del fichero fuente hasta el siguiente '#' */
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
  if (c != '#')
   {
    /* Leemos si es una rama ¢ un modelo de elemento */
    memset (parametro, 0, 15);
    contador = 0;
    while ((c >= 'A') && (c <= 'Z') && (c != '='))
    {
     parametro[contador] = c;
     ++contador;
     mi_fread (&c, 1, 1, fuente);
    }

    /* Si es una rama... */
    if (strcmp (parametro, "RAMA") == 0)
     {
      /* ..obtenemos sus nudos de conexi¢n */
      buscar_siguiente ('=', '=', &c, NO_AVANZAR);

      buscar_siguiente ('(', '(', &c, AVANZAR);

      buscar_siguiente ('0', '9', &c, AVANZAR);
      memset (numero, 0, 20);
      while ((c >= '0') && (c <= '9'))
      {
       numero[strlen (numero)] = c;
       mi_fread (&c, 1, 1, fuente);
      }

      ni = atoi (numero);

      buscar_siguiente ('0', '9', &c, NO_AVANZAR);
      memset (numero, 0, 20);
      while ((c >= '0') && (c <= '9'))
      {
       numero[strlen (numero)] = c;
       mi_fread (&c, 1, 1, fuente);
      }

      nf = atoi (numero);

      /* Avanzamos hasta el ')' */
      buscar_siguiente (')', ')', &c, NO_AVANZAR);

      /* Finalmente sus elementos */
      buscar_siguiente ('{', '{', &c, AVANZAR);

      strset (aux, 0);

      do
      {
       /* Localizamos el siguiente car cter v lido (¢ el '}') */
       while (((c < 33) || (c > 122) || (c == ',')) && (c != '}'))
       {
	mi_fread (&c, 1, 1, fuente);
       }

       /* Si no hemos llegado al fin de la rama... */
       if (c != '}')
	{
	 /* ...obtenemos el nombre del siguiente elemento de la rama */
	 memset (name, 0, 6);
	 contador = 0;
	 while ((c >= 33) && (c <= 122) && (contador < 5) &&
		(c != ',') && (c != 10))
	 {
	  name[contador] = c;
	  ++contador;
	  mi_fread (&c, 1, 1, fuente);
	 }

	 if ((c != ' ') && (c != '=') && (c != ','))
	   error (W_NOMMUYLARGO);

	 /* Si el elemento va precedido de un '-'.. */
	 if (name[0] == '-')
	  {
	   /* ..entonces, si es un generador, cambiamos su valor */
	   elem_act = buscar (elem_ini, &name[1]);
	   if (elem_act != NIL)
	     if ((elem_act->datos.tipo[0] != 'R') &&
		 (elem_act->datos.tipo[0] != 'L') &&
		 (elem_act->datos.tipo[0] != 'C') &&
		 (elem_act->datos.tipo[0] != 'Q'))
	       elem_act->datos.valor = -elem_act->datos.valor;
	  }

	 /* Buscamos el elemento en el fichero */
	 elem_act = buscar (elem_ini, name);

	 /* Si no tenemos constancia de ese elemento lo decimos */
	 if (elem_act == NIL)
	   error (E_ELEMNOENCON);

	 /* pero, si el elemento existe.. */
	 else
	  {
	   /* ..entonces, si no es un diodo.. */
	   if (elem_act->datos.tipo[0] != 'Q')
	    {
	     /* ..lo a¤adimos a la rama */
	     strcat (aux, elem_act->datos.tipo);

	     /* y comprobamos si lo utiliza alg£n generador dep. de I */
	     elem_aux = NIL;
	     do
	     {
	      /* Buscamos el siguiente gen. dep. de I de la lista */
	      do
	      {
	       elem_aux = (elem_aux == NIL) ? elem_ini : elem_aux->siguiente;
	      }
	      while ((elem_aux->datos.tipo[0] != 'B') &&
		     (elem_aux->datos.tipo[0] != 'E') &&
		     (elem_aux->siguiente != NIL));

	      /* Si hemos encontrado alguno.. */
	      if ((elem_aux->datos.tipo[0] == 'B') ||
		 (elem_aux->datos.tipo[0] == 'E'))

		/* ..entonces, si depende de la I por el elemento actual.. */
		if (strcmp (elem_aux->datos.caract.elem,
			     elem_act->datos.tipo) == 0)

		  /* ..si la rama cumple nodoi > nodof.. */
		  if (ni > nf)

		    /* ..entonces cambiaremos de signo el generador */
		    elem_aux->datos.valor = -elem_aux->datos.valor;
	     }
	     while (elem_aux->siguiente != NIL);
	    }

	   /* pero, si es un diodo.. */
	   else
	    {
	     /* Si no es el primer elemento en la rama, la cerramos */
	     if (strlen (aux) > 0)
	      {
	       if (rama_ini == NIL)
		rama_ini =
		rama_act = (registro*) malloc (sizeof (registro));
	       else
		rama_act =
		rama_act->siguiente = (registro*) malloc (sizeof (registro));

	       strcpy (rama_act->datos.elem, aux);
	       rama_act->datos.nodoi = ni;
	       rama_act->datos.nodof = ++max_nudo;
	       rama_act->datos.marca = 0;
	       rama_act->datos.usada = 0;
	       rama_act->siguiente   = NIL;

	       /* Preparamos la rama para el diodo */
	       ni = max_nudo;
	       strset (aux, 0);
	      }

	     /* Si no es el £ltimo elemento en la rama, a¤adimos una rama con
		el diodo. Para saberlo, localizamos el siguiente car cter
		v lido (¢ el '}') */
	     while (((c < 33) || (c > 122) || (c == ',')) && (c != '}'))
	     {
	      mi_fread (&c, 1, 1, fuente);
	     }

	     strcpy (aux, elem_act->datos.tipo);

	     if ((c != '}') || (elem_act->datos.f.coef[0] != 0))
	      {
	       if (rama_ini == NIL)
		rama_ini =
		rama_act = (registro*) malloc (sizeof (registro));
	       else
		rama_act =
		rama_act->siguiente = (registro*) malloc (sizeof (registro));

	       strcpy (rama_act->datos.elem, aux);

	       /* Hay que tener en cuenta que el nodo inicial de la rama tiene
		  que coincidir con el  nodo, y el nodo final con el c todo */
	       if (name[0] == '-')
		{
		 rama_act->datos.nodoi = ++max_nudo;
		 rama_act->datos.nodof = ni;
		}
	       else
		{
		 rama_act->datos.nodoi = ni;
		 rama_act->datos.nodof = ++max_nudo;
		}

	       rama_act->datos.marca = 0;
	       rama_act->datos.usada = 0;
	       rama_act->siguiente   = NIL;
	      }

	     /* A¤adimos la rama con la Cd, si existe */
	     if (elem_act->datos.caract.capacidad != 0)
	      {
	       rama_act =
	       rama_act->siguiente = (registro *) malloc (sizeof (registro));
	       strcpy (name, "Cd\x0");
	       strcat (name, &elem_act->datos.tipo[1]);
	       strcpy (rama_act->datos.elem,
		       buscar (elem_ini, name)->datos.tipo);
	       rama_act->datos.nodoi = ni;
	       if ((c != '}') || (elem_act->datos.f.coef[0] != 0))
		 rama_act->datos.nodof = max_nudo;
	       else
		 rama_act->datos.nodof = nf;
	       rama_act->datos.marca = 0;
	       rama_act->datos.usada = 0;
	       rama_act->siguiente   = NIL;
	      }

	     /* A¤adimos la rama con la Rs, si existe */
	     if (elem_act->datos.f.coef[0] != 0)
	      {
	       strcpy (name, "Rs\x0");
	       strcat (name, &elem_act->datos.tipo[1]);
	       strcpy (aux, buscar (elem_ini, name)->datos.tipo);
	       ni = max_nudo;

	       /* Si el diodo estaba al final de la rama, no hay que crear
		  un nuevo nudo; caso contrario, s¡ hay que crearlo */
	       if (c != '}')
		{
		 rama_act =
		 rama_act->siguiente = (registro*) malloc (sizeof (registro));
		 strcpy (rama_act->datos.elem, aux);
		 rama_act->datos.nodoi = ni;
		 rama_act->datos.nodof = ++max_nudo;
		 rama_act->datos.marca = 0;
		 rama_act->datos.usada = 0;
		 rama_act->siguiente   = NIL;

		 /* Empezamos nueva rama */
		 ni = max_nudo;
		 strset (aux, 0);
		}
	      }
	    }
	  }
	}
      }
      while (c != '}');

      /* Si la rama ten¡a alg£n elemento v lido (cuyo valor <> 0).. */
      if (aux[0] != 0)
       {
	/* ..entonces, si no est  unida al nudo de referencia.. */
	pt = aux;
	if ((ni != 0) && (nf != 0))
	 {
	  /* ..buscamos elementos que no sean generadores de V en la rama */
	  while ((*pt != 0) && ((*pt == 'A') || (*pt == 'B') || (*pt == 'V')))
	   {
	    pt += 4;
	   }
	 }

	/* Si no los hab¡a, damos un aviso */
	if (*pt == 0)
	  error (W_GENSINELEM);

	/* Guardamos la rama en el fichero */
	if (rama_ini == NIL)
	 rama_ini = rama_act = (registro*) malloc (sizeof (registro));
	else
	 rama_act = rama_act->siguiente = (registro*) malloc (sizeof (registro));

	strcpy (rama_act->datos.elem, aux);
	rama_act->datos.nodoi = ni;
	rama_act->datos.nodof = nf;
	rama_act->datos.marca = 0;
	rama_act->datos.usada = 0;
	rama_act->siguiente   = NIL;
       }
     }

    /* pero, si es un modelo de elemento... */
    else
      if (strcmp (parametro, "MODELO") == 0)
       {
	/* Obtenemos los nudos de emisor, base y colector */
	buscar_siguiente ('=', '=', &c, NO_AVANZAR);

	buscar_siguiente ('(', '(', &c, AVANZAR);

	ni = 0;
	buscar_siguiente ('0', '9', &c, AVANZAR);
	do
	{
	 /* Obtenemos el siguiente n£mero en forma de string */
	 memset (numero, 0, 20);
	 while ((c >= '0') && (c <= '9'))
	 {
	  numero[strlen (numero)] = c;
	  mi_fread (&c, 1, 1, fuente);
	 }

	 /* Convertimos el string a n£mero y lo guardamos */
	 if (numero[0] != 0)
	   tabla_nudos[ni++] = atoi (numero);
	 else
	   mi_fread (&c, sizeof (char), 1, fuente);

	 /* Avanzamos hasta el siguiente n£mero (o el ')') */
	 buscar_siguiente (')', '9', &c, NO_AVANZAR);
	}
	while (c != ')');

	/* Finalmente el nombre del modelo */
	buscar_siguiente ('{', '{', &c, AVANZAR);

	strset (aux, 0);

	/* Localizamos el siguiente car cter v lido (¢ el '}') */
	while (((c < 33) || (c > 122) || (c == ',')) && (c != '}'))
	{
	 mi_fread (&c, 1, 1, fuente);
	}

	/* Si no hemos llegado al fin de la rama... */
	if (c != '}')
	 {
	  /* ...obtenemos el nombre del modelo */
	  memset (name, 0, 6);
	  contador = 0;
	  while ((c >= 33) && (c <= 122) && (contador < 5))
	  {
	   name[contador] = c;
	   ++contador;
	   mi_fread (&c, 1, 1, fuente);
	  }
	 }

	if (contador >= 5)
	  error (W_NOMMUYLARGO);

	/* Buscamos el modelo en la lista de elementos */
	elem_act = buscar (elem_ini, name);

	if (elem_act == NIL)
	  error (E_ELEMNOENCON);
	else
	 {
	  /* Obtenemos su tipo */
	  type = elem_act->datos.tipo[0];

	  if (((type < 'A') || (type > 'E')) && ((type < 'J') || (type > 'K')) &&
	      ((type < 'M') || (type > 'R')) && (type != 'I') && (type != 'L') &&
	      (type != 'V'))
	    error (E_TIPODESCONOC);

	  /* Si es un transistor bipolar.. */
	  if ((type == 'P') || (type == 'N'))
	   {
	    /* .. construimos el modelo, empezando por la Rb */
	    strcpy (name, "Rb\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);
	    if (elem_aux != NIL)
	     {
	      /* Conectamos la Rb entre el nudo de base y un nuevo nudo */
	      if (rama_ini == NIL)
	       rama_ini = rama_act = (registro*) malloc (sizeof (registro));
	      else
	       rama_act =
	       rama_act->siguiente = (registro*) malloc (sizeof (registro));

	      rama_act->datos.nodoi = tabla_nudos[1];
	      rama_act->datos.nodof = ++max_nudo;
	      strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	      rama_act->datos.marca = 0;
	      rama_act->datos.usada = 0;
	      rama_act->siguiente   = NIL;

	      /* Convertimos el nudo de base en el nudo de base interno */
	      tabla_nudos[1] = max_nudo;
	     }

	    /* Seguimos con la Rc */
	    strcpy (name, "Rc\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);
	    if (elem_aux != NIL)
	     {
	      /* Conectamos la Rc entre el nudo de colector y un nuevo nudo */
	      if (rama_ini == NIL)
	       rama_ini = rama_act = (registro*) malloc (sizeof (registro));
	      else
	       rama_act =
	       rama_act->siguiente = (registro*) malloc (sizeof (registro));

	      rama_act->datos.nodoi = tabla_nudos[2];
	      rama_act->datos.nodof = ++max_nudo;
	      strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	      rama_act->datos.marca = 0;
	      rama_act->datos.usada = 0;
	      rama_act->siguiente   = NIL;

	      /* Convertimos el nudo de colector en el de colector interno */
	      tabla_nudos[2] = max_nudo;
	     }

	    /* Seguimos con la Re */
	    strcpy (name, "Re\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);
	    if (elem_aux != NIL)
	     {
	      /* Conectamos la Re entre el nudo de emisor y un nuevo nudo */
	      if (rama_ini == NIL)
	       rama_ini = rama_act = (registro*) malloc (sizeof (registro));
	      else
	       rama_act =
	       rama_act->siguiente = (registro*) malloc (sizeof (registro));

	      rama_act->datos.nodoi = tabla_nudos[0];
	      rama_act->datos.nodof = ++max_nudo;
	      strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	      rama_act->datos.marca = 0;
	      rama_act->datos.usada = 0;
	      rama_act->siguiente   = NIL;

	      /* Convertimos el nudo de emisor en el de emisor interno */
	      tabla_nudos[0] = max_nudo;
	     }

	    /* Conectamos los diodos seg£n la polaridad del transistor.
	       Primero el diodo de emisor */
	    ni = tabla_nudos[(type == 'P') ? 0 : 1];
	    nf = tabla_nudos[(type == 'P') ? 1 : 0];

	    strcpy (name, "Df\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);

	    if (rama_ini == NIL)
	      rama_ini = rama_act = (registro*) malloc (sizeof (registro));
	    else
	      rama_act =
	      rama_act->siguiente = (registro*) malloc (sizeof (registro));

	    rama_act->datos.nodoi = ni;
	    rama_act->datos.nodof = nf;
	    strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	    rama_act->datos.marca = 0;
	    rama_act->datos.usada = 0;
	    rama_act->siguiente   = NIL;

	    /* Ahora la capacidad asociada a dicho diodo */
	    strcpy (name, "Ce\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);

	    if (elem_aux != NIL)
	     {
	      rama_act =
	      rama_act->siguiente = (registro*) malloc (sizeof (registro));

	      rama_act->datos.nodoi = ni;
	      rama_act->datos.nodof = nf;
	      strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	      rama_act->datos.marca = 0;
	      rama_act->datos.usada = 0;
	      rama_act->siguiente   = NIL;
	     }

	    /* A continuaci¢n el diodo no ideal de emisor */
	    strcpy (name, "Nf\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);

	    if (elem_aux != NIL)
	     {
	      rama_act =
	      rama_act->siguiente = (registro*) malloc (sizeof (registro));

	      rama_act->datos.nodoi = ni;
	      rama_act->datos.nodof = nf;
	      strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	      rama_act->datos.marca = 0;
	      rama_act->datos.usada = 0;
	      rama_act->siguiente   = NIL;
	     }

	    /* Ahora el diodo de colector */
	    ni = tabla_nudos[(type == 'P') ? 2 : 1];
	    nf = tabla_nudos[(type == 'P') ? 1 : 2];

	    strcpy (name, "Dr\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);

	    rama_act =
	    rama_act->siguiente = (registro*) malloc (sizeof (registro));

	    rama_act->datos.nodoi = ni;
	    rama_act->datos.nodof = nf;
	    strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	    rama_act->datos.marca = 0;
	    rama_act->datos.usada = 0;
	    rama_act->siguiente   = NIL;

	    /* Ahora la capacidad asociada a dicho diodo */
	    strcpy (name, "Cc\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);

	    if (elem_aux != NIL)
	     {
	      rama_act =
	      rama_act->siguiente = (registro*) malloc (sizeof (registro));

	      rama_act->datos.nodoi = ni;
	      rama_act->datos.nodof = nf;
	      strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	      rama_act->datos.marca = 0;
	      rama_act->datos.usada = 0;
	      rama_act->siguiente   = NIL;
	     }

	    /* A¤adimos el diodo no ideal de colector */
	    strcpy (name, "Nr\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);

	    if (elem_aux != NIL)
	     {
	      rama_act =
	      rama_act->siguiente = (registro*) malloc (sizeof (registro));

	      rama_act->datos.nodoi = ni;
	      rama_act->datos.nodof = nf;
	      strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	      rama_act->datos.marca = 0;
	      rama_act->datos.usada = 0;
	      rama_act->siguiente   = NIL;
	     }

	    /* Conectamos los generadores de corriente, primero I1 */
	    ni = tabla_nudos[(type == 'P') ? 0 : 2];
	    nf = tabla_nudos[(type == 'P') ? 2 : 0];

	    strcpy (name, "I1\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);

	    rama_act =
	    rama_act->siguiente = (registro*) malloc (sizeof (registro));

	    rama_act->datos.nodoi = ni;
	    rama_act->datos.nodof = nf;
	    strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	    rama_act->datos.marca = 0;
	    rama_act->datos.usada = 0;
	    rama_act->siguiente   = NIL;

	    /* Despu‚s, y finalmente, I2 */
	    ni = tabla_nudos[(type == 'P') ? 2 : 0];
	    nf = tabla_nudos[(type == 'P') ? 0 : 2];

	    strcpy (name, "I2\x0");
	    strcat (name, &elem_act->datos.tipo[1]);
	    elem_aux = buscar (elem_ini, name);

	    rama_act =
	    rama_act->siguiente = (registro*) malloc (sizeof (registro));

	    rama_act->datos.nodoi = ni;
	    rama_act->datos.nodof = nf;
	    strcpy (rama_act->datos.elem, elem_aux->datos.tipo);
	    rama_act->datos.marca = 0;
	    rama_act->datos.usada = 0;
	    rama_act->siguiente   = NIL;
	   }

	  /* Pero, si es un transistor JFET.. */
	  else
	    if ((type == 'J') || (type == 'K'))
	     {
	     }

	    /* Pero, si es un MOSFET.. */
	    else
	      if ((type == 'M') || (type == 'O'))
	       {
	       }
	 }
       }

      /* y si, finalmente, es un cuadripolo.. */
      else
	if (strcmp (parametro, "CUADRIPOLO") == 0)
	 {
	  /* ..obtener los nudos de conexi¢n de las redes de entrada y salida */
	  buscar_siguiente ('=', '=', &c, NO_AVANZAR);
	  buscar_siguiente ('(', '(', &c, AVANZAR);
	  mi_fread (&c, 1, 1, fuente);

	  for (ni = 0; ni < 4; ++ni)
	   {
	    buscar_siguiente ('0', '9', &c, NO_AVANZAR);
	    memset (numero, 0, 20);
	    while ((c >= '0') && (c <= '9'))
	    {
	     numero[strlen (numero)] = c;
	     mi_fread (&c, 1, 1, fuente);
	    }

	    nudos[ni] = atoi (numero);
	   }

	  /* Avanzamos hasta el ')' */
	  buscar_siguiente (')', ')', &c, NO_AVANZAR);

	  /* Obtenemos el nombre del cuadripolo */
	  buscar_siguiente ('{', '{', &c, AVANZAR);

	  /* Localizamos el siguiente car cter v lido (¢ el '}') */
	  while (((c < 33) || (c > 122) || (c == ',')) && (c != '}'))
	  {
	   mi_fread (&c, 1, 1, fuente);
	  }

	  /* Si no hemos llegado al fin de la rama... */
	  if (c != '}')
	   {
	    /* ...leemos el nombre */
	    memset (name, 0, 11);
	    contador = 0;
	    while ((c >= 33) && (c <= 122) && (contador < 11))
	    {
	     name[contador] = c;
	     ++contador;
	     mi_fread (&c, 1, 1, fuente);
	    }
	   }

	  /* Comprobamos la longitud del nombre */
	  if ((c != ' ') && (c != '}'))
	    error (W_NOMMUYLARGO);

	  /* Buscamos el cuadripolo en la lista de cuadripolos */
	  cuad_act = buscarc (cuad_ini, name);

	  /* Si no estaba, damos un error */
	  if (cuad_act == NIL)
	    error (E_ELEMNOENCON);

	  /* pero si estaba, obtenemos el nombre interno */
	  else
	    strcpy (aux, cuad_act->datos.tipo);

	  /* Constru¡mos las ramas. Primero la de entrada.. */
	  if (rama_ini == NIL)
	   rama_ini = rama_act = (registro*) malloc (sizeof (registro));
	  else
	   rama_act = rama_act->siguiente = (registro*) malloc (sizeof (registro));

	  strcpy (rama_act->datos.elem, aux);
	  rama_act->datos.nodoi = nudos[0];
	  rama_act->datos.nodof = nudos[1];
	  rama_act->datos.marca = 0;
	  rama_act->datos.usada = 0;
	  rama_act->siguiente   = NIL;

	  /* ..y despu‚s la de salida */
	  if (rama_ini == NIL)
	   rama_ini = rama_act = (registro*) malloc (sizeof (registro));
	  else
	   rama_act = rama_act->siguiente = (registro*) malloc (sizeof (registro));

	  /* En esta rama ponemos 'Z-nn' en lugar de 'Z0nn', como la anterior,
	     para indicar que es la red de salida */
	  aux[1] = '-';
	  strcpy (rama_act->datos.elem, aux);
	  rama_act->datos.nodoi = nudos[2];
	  rama_act->datos.nodof = nudos[3];
	  rama_act->datos.marca = 0;
	  rama_act->datos.usada = 0;
	  rama_act->siguiente   = NIL;
	 }
	else
	  error (E_TERMDESCONOC);
   }
 }
 while (c != '#');

 /* Liberamos el espacio utilizado para volver a utilizarlo */
 free (tabla_nudos);

 /* Obtenemos una lista de los nudos que est n definidos y el n§ de veces que
    aparecen en la zona de definici¢n de ramas */
 tabla_nudos = (int *) malloc ((max_nudo + 1) * sizeof (int));
 memset (tabla_nudos, 0, (max_nudo + 1) * sizeof (int));
 for (rama_act = rama_ini; rama_act != NIL; rama_act = rama_act->siguiente)
  {
   ++tabla_nudos[rama_act->datos.nodoi];
   ++tabla_nudos[rama_act->datos.nodof];
  }

 /* Damos el aviso si no se ha definido el nudo 0 */
 if (tabla_nudos[0] == 0)
   error (W_NOHAYNUDOREF);

 /* Por £ltimo, comprobamos los elementos "colgantes", es decir, aquellos
    en los que alguno de sus extremos no est  conectado a nada */
 for (contador = 0; contador < max_nudo; ++contador)
   if (tabla_nudos[contador] == 1)
     error (W_ELEMNOCONEC);

 free (tabla_nudos);

 return (rama_ini);
}

/*--------------------------------------------------------------------------*/
