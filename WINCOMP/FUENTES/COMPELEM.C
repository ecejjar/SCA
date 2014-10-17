#include "compilar.h"

/*----------------Zona de declaraci¢n de variables externas-----------------*/

extern FILE     *fuente;
extern unsigned max_nudo,
		nMaxLin, nMaxCol,
		nLinea, nCol,
		errores;
extern BOOL     precalc;
extern char     nom_fich[80], c;

/*----------------Zona de declaraci¢n de funciones externas-----------------*/

extern size_t     mi_fread (void *, size_t, size_t, FILE *);
extern void       error (int codigo);
extern void       no_orden (reg_elem *primero, reg_elem *elem);
extern reg_elem   *buscar (reg_elem *primero, char nombre[6]);
extern int        buscar_siguiente (char inf, char sup, char *c, short avance);
extern double     convertir (char *c);
extern func_trans obt_func_trans (char *c);

/*------------Comienzo de la zona de definici¢n de funciones----------------*/

void precalculo (void)

/* Calcula el n§ de nudos del circuito y de l¡neas del fichero fuente, y
   tambi‚n la longitud de l¡nea m xima del mismo. Emplea para los resulta-
   dos las variables globales 'max_nudo', 'nMaxLin' y 'nMaxCol' respect.  */
{
 int  ni, contador, comentario = 0;
 char parametro[15], numero[20];

 /* Hacemos una primera pasada para conocer el n§ de nudos del circuito */
 precalc = TRUE;
 errores = NO_ERRORES;
 nMaxLin = nMaxCol = nLinea = nCol = 0;

 buscar_siguiente ('#', '#', &c, AVANZAR);
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
    /* Leemos si es una rama ¢ un multipolo */
    memset (parametro, 0, 15);
    contador = 0;
    while ((c >= 'A') && (c <= 'Z') && (c != '=') && (contador < 6))
    {
     parametro[contador] = c;
     ++contador;
     mi_fread (&c, 1, 1, fuente);
    }

    /* Obtenemos todos los nudos que relaciona */
    buscar_siguiente ('=', '=', &c, NO_AVANZAR);

    buscar_siguiente ('(', '(', &c, NO_AVANZAR);

    buscar_siguiente (')', '9', &c, NO_AVANZAR);

    do
    {
     /* Obtenemos el n£mero de nudo en forma de string */
     memset (numero, 0, 20);
     while ((c >= '0') && (c <= '9'))
     {
      numero[strlen (numero)] = c;
      mi_fread (&c, 1, 1, fuente);
     }

     /* Si hemos obtenido alg£n nudo.. */
     if (numero[0] != 0)
      {
       /* ..convertimos el string a n£mero entero */
       ni = atoi (numero);

       /* Cada vez que obtenemos un nudo, actualizamos 'max_nudo' */
       max_nudo = max (ni, max_nudo);
      }
     else
       mi_fread (&c, sizeof (char), 1, fuente);

     /* Avanzamos hasta el siguiente n£mero o hasta el ')' */
     buscar_siguiente (')', '9', &c, NO_AVANZAR);
    }
    while (c != ')');

    buscar_siguiente ('}', '}', &c, NO_AVANZAR);
   }
 }
 while (c != '#');

 buscar_siguiente ('#', '#', &c, AVANZAR);

 /* Obtenemos el n§ de l¡neas del fichero fuente */
 nMaxLin = nLinea;

 precalc = FALSE;

 return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

reg_elem *compilar_elems (reg_cuadri **primer_cuad)

/* Funci¢n que compila el fichero ".FNT" hasta el 1er '#'. Devuelve un pun-
   tero al 1er elemento. La lista compilada termina en NIL .*/
{
 static short n_cuad = 0;
 FILE       *elementos;
 reg_elem   *elem_ini = NIL, *elem_act = NIL, *elem_aux = NIL;
 reg_cuadri *cuad_ini = NIL, *cuad_act = NIL, *cuad_aux = NIL;
 int        ni, nf, lei, contador, comentario = 0;
 char       e[50], aux[50], num[4], name[11], through[6], type, sig,
	    parametro[15], numero[20];
 float      value, frequency, phase, cond_inic, tempcoef, activetime,
	    Is, Cjo, FC, transtime, Eg, Rb, Rc, Re,
	    Bf, Br, Isc, Ise, Vaf, Var, tf, tr, Cje, Cjc, Vje, Vjc, Mje, Mjc,
	    Isf, Xti, Isr, Vto, VtoTc, Beta, Xtb, Landa, M, Pb, Cgs, Cgd,
	    Rs, Rd,
	    faux;
 polinomio  poly;
 func_trans Ti, Tr, Tf, To, Taux;

 /* Recorremos el fichero fuente car cter por car cter */
 errores = ERRORES;
 comentario = 0;

 fseek (fuente, 0, SEEK_SET);

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
  while (((c < 33) || (c > 122) || comentario) && (!feof (fuente)));

  /* Si no es el de fin de secci¢n ¢ de fichero.. */
  if ((c != '#') && (!feof (fuente)))
   {
    /* Leemos los datos del siguiente elemento. Primero su nombre */
    memset (name, 0, 11);
    contador = 0;
    while ((c >= 33) && (c <= 122) && (c != '=') && (contador < 10))
    {
     name[contador] = c;
     ++contador;
     mi_fread (&c, 1, 1, fuente);
    }

    /* Comprobamos que el nombre no exceda de 10 caracteres, en principio */
    if ((c != ' ') && (c != '='))
      error (W_NOMMUYLARGO);

    /* Comprobamos que no haya un elemento con ese nombre */
    if (buscar (elem_ini, name) != NIL)
      error (E_DEFINDUPL);

    /* Ahora su tipo */
    if (buscar_siguiente ('=', '=', &c, NO_AVANZAR))
      return (NULL);

    if (buscar_siguiente ('A', 'Z', &c, AVANZAR))
      return (NULL);

    type = c;

    /* Una vez averiguado el tipo, si no es cuadripolo y el nombre excede de
       5 caracteres, avisamos y recortamos */
    if ((type != 'Z') && (strlen (name) > 5))
     {
      error (W_NOMMUYLARGO);
      name[5] = 0;
     }

    if (((type < 'A') || (type > 'E')) && ((type < 'P') || (type > 'R')) &&
	(type != 'I') && (type != 'L') && (type != 'N') && (type != 'V') &&
	(type != 'Z'))
      error (E_TIPODESCONOC);

    /* A continuaci¢n los par metros, que comienzan por un '{' */
    if (buscar_siguiente ('{', '{', &c, AVANZAR))
      return (NULL);
    mi_fread (&c, 1, 1, fuente);

    /* Par metros generales */
    value = 1;
    poly.grado = 0;
    activetime = 3600;
    frequency = phase = cond_inic = tempcoef = 0;

    /* Inicializamos el resto de par metros a valores por defecto */
    switch (type)
    {
     case 'V':
     case 'I':
     {
      sig = 'C';
      break;
     }

     case 'Q':
     {
      /* Par metros del diodo semiconductor */
      Eg = 1.11;
      FC = 0.5;
      Is  = 1e-14;
      Cjo = 2e-12;
      transtime = 0;
      poly.coef[0] = 0;        /* Rs  del diodo */
      poly.coef[1] = 0.4;	     /* Vj  del diodo */
      poly.coef[2] = INFINITO; /* Vbr del diodo */
      poly.coef[3] = 1e-10;    /* Ibv del diodo */
      poly.coef[4] = 0.5;      /* FC  del diodo */
      poly.coef[5] = 1.11;     /* Eg  del diodo */
      poly.coef[6] = 0.5;      /* Mj  del diodo */

      break;
     }

     case 'P':
     case 'N':
     {
      /* Par metros del transistor bipolar */
      Rb = Rc = Re = 0;
      Isc = Ise = 0;
      Bf = 100;
      Br = 1;
      Vaf = Var = INFINITO;
      tf = tr = 0;
      Vjc = Vje = 0.75;
      Cjc = Cje = 2e-12;
      Mjc = Mje = 0.33;
      FC = 0.5;
      Eg = 1.11;

      break;
     }

     case 'J':
     case 'K':
     {
      /* Par metros del transistor JFET */
      Isf = 1e-14;
      Isr = 0;
      Vto = -2;
      Beta = 1e-4;
      Landa = 0;
      M = FC = 0.5;
      Pb = 1;
      Xtb = Xti = VtoTc =
      Cgs = Cgd = Rs = Rd = 0;

      break;
     }

     case 'Z':
     {
      /* Par metros Z del cuadripolo */
      Ti.num.grado = Tr.num.grado = Tf.num.grado = To.num.grado =
      Ti.den.grado = Tr.den.grado = Tf.den.grado = To.den.grado = 0;

      Ti.num.coef[0] = Tr.num.coef[0] = Tf.num.coef[0] = To.num.coef[0] = 0;
      Ti.den.coef[0] = Tr.den.coef[0] = Tf.den.coef[0] = To.den.coef[0] = 1;

      break;
     }
    }

    do
    {
     /* Avanzamos hasta el comienzo del nombre del par metro (¢ el '}') */
     while (((c < 'A') || (c > 'Z')) && (c != '}') && (!feof (fuente)))
     {
      if ((c != ',') && (c != ' ') && (c != 10))
	error (E_CARINESP);
      mi_fread (&c, 1, 1, fuente);
     }

     if ((c != '}') && (!feof (fuente)))
      {
       memset (parametro, 0, 15);
       while ((((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) ||
	       (c == '_')) && (strlen (parametro) < 15))
       {
	parametro[strlen(parametro)] = c; /* Leemos el nombre del par metro */
	mi_fread (&c, 1, 1, fuente);
       }

       if (buscar_siguiente ('=', '=', &c, NO_AVANZAR))
	 return (NULL);                           /* Avanzamos hasta el '=' */

       if (strcmp (parametro, "VALOR") == 0)
	 value = convertir (&c);
       else
	 if (strcmp (parametro, "CT") == 0)
	   tempcoef = convertir (&c);
	 else
	   if ((type == 'V') || (type == 'I'))
	    {
	     if (strcmp (parametro, "FORMA") == 0)
	      {
	       buscar_siguiente ('A', 'Z', &c, AVANZAR);
	       sig = c;
	       mi_fread (&c, 1, 1, fuente);     /* Salta al siguiente car cter */
	      }
	     else
	       if (strcmp (parametro, "FRECUENCIA") == 0)
		 frequency = convertir (&c);
	       else
		 if (strcmp (parametro, "RETARDO") == 0)
		   phase = convertir (&c);
		 else
		   if (strcmp (parametro, "DURACION") == 0)
		     activetime = convertir (&c);
		   else
		    {
		     error (E_PARAMDESCONOC);
		     convertir (&c);
		    }
	    }
	   else
	     if ((type == 'A') || (type == 'D'))
	      {
	       if (strcmp (parametro, "NUDOS") == 0)
		{
		 ni = convertir (&c);
		 nf = convertir (&c);
		}
	       else
		 if (strcmp (parametro, "FUNCION") == 0)
		  {
		   buscar_siguiente ('A', 'Z', &c, AVANZAR);
		   sig = c;

		   if (sig == 'D')
		    {
		     if (buscar_siguiente ('(', '(', &c, AVANZAR))
		       return (NULL);
		     contador = 0;
		     do
		     {
		      poly.coef[contador++] = convertir (&c);
		     }
		     while ((contador < 11) && (c != ')'));
		     poly.grado = contador - 1;
		    }
		   else
		     if ((sig != 'C') && (sig != 'S') && (sig != 'I'))
		       error (E_TERMDESCONOC);

		   mi_fread (&c, 1, 1, fuente);
		  }
		 else
		  {
		   error (E_PARAMDESCONOC);
		   convertir (&c);
		  }
	      }
	     else
	      if ((type == 'B') || (type == 'E'))
	       {
		if (strcmp (parametro, "ELEMENTO") == 0)
		 {
		  buscar_siguiente (33, 122, &c, AVANZAR);
		  memset (through, 0, 6);
		  contador = 0;
		  while ((c >= 33) && (c <= 122) && (contador < 5))
		  {
		   through[contador] = c;
		   ++contador;
		   mi_fread (&c, 1, 1, fuente);
		  }
		 }
		else
		  if (strcmp (parametro, "SENTIDO") == 0)
		   {
		    /* Obtenemos los nudos de origen y destino */
		    ni = convertir (&c);
		    nf = convertir (&c);

		    /* Se asume sentido del nodo menor hacia el mayor */
		    if (ni > nf)
		      value = -value;
		   }
		  else
		    if (strcmp (parametro, "FUNCION") == 0)
		     {
		      buscar_siguiente ('A', 'Z', &c, AVANZAR);
		      sig = c;
		      if (sig == 'D')
		       {
			buscar_siguiente ('(', '(', &c, AVANZAR);
			contador = 0;
			do
			{
			 poly.coef[contador++] = convertir (&c);
			}
			while ((contador < 11) && (c != ')'));
			poly.grado = contador - 1;
		       }
		      mi_fread (&c, 1, 1, fuente);
		     }
		    else
		     {
		      error (E_PARAMDESCONOC);
		      convertir (&c);
		     }
	       }
	      else
		if ((type == 'L') || (type == 'C'))
		 {
		  if (strcmp (parametro, "VALOR_INICIAL") == 0)
		    cond_inic = convertir(&c);
		  else
		   {
		    error (E_PARAMDESCONOC);
		    convertir (&c);
		   }
		 }
		else
		  if (type == 'Q')
		   {
		    faux = convertir (&c);

		    if (strcmp (parametro, "IS") == 0)
		      Is = faux;
		    else
		      if (strcmp (parametro, "CJ") == 0)
			Cjo = faux;
		      else
			if (strcmp (parametro, "TT") == 0)
			  transtime = faux;
			else
			  if (strcmp (parametro, "RS") == 0)
			    poly.coef[0] = faux;
			  else
			    if (strcmp (parametro, "VJ") == 0)
			      poly.coef[1] = faux;
			    else
			      if (strcmp (parametro, "VBR") == 0)
				poly.coef[2] = faux;
			      else
				if (strcmp (parametro, "IBV") == 0)
				  poly.coef[3] = faux;
				else
				  if (strcmp (parametro, "FC") == 0)
				    poly.coef[4] = faux;
				  else
				    if (strcmp (parametro, "EG") == 0)
				      poly.coef[5] = faux;
				    else
				      if (strcmp (parametro, "MJ") == 0)
					poly.coef[6] = faux;
				      else
					error (E_PARAMDESCONOC);
		   }
		  else
		    if ((type == 'P') || (type == 'N'))
		     {
		      faux = convertir(&c);

		      if (strcmp (parametro, "IS") == 0)
			Is = faux;
		      else
		       if (strcmp (parametro, "ISC") == 0)
			 Isc = faux;
		       else
			if (strcmp (parametro, "ISE") == 0)
			  Ise = faux;
			else
			 if (strcmp (parametro, "BR") == 0)
			   Br = faux;
			 else
			  if (strcmp (parametro, "BF") == 0)
			    Bf = faux;
			  else
			   if (strcmp (parametro, "VAR") == 0)
			     Var = faux;
			   else
			    if (strcmp (parametro, "VAF") == 0)
			      Vaf = faux;
			    else
			     if (strcmp (parametro, "TR") == 0)
			       tr = faux;
			     else
			      if (strcmp (parametro, "TF") == 0)
				tf = faux;
			      else
			       if (strcmp (parametro, "CJC") == 0)
				 Cjc = faux;
			       else
				if (strcmp (parametro, "CJE") == 0)
				  Cje = faux;
				else
				 if (strcmp (parametro, "VJC") == 0)
				   Vjc = faux;
				 else
				  if (strcmp (parametro, "VJE") == 0)
				    Vje = faux;
				  else
				   if (strcmp (parametro, "MJC") == 0)
				     Mjc = faux;
				   else
				    if (strcmp (parametro, "MJE") == 0)
				      Mje = faux;
				    else
				     if (strcmp (parametro, "XTB") == 0)
				       tempcoef = faux;
				     else
				      if (strcmp (parametro, "RB") == 0)
					Rb = faux;
				      else
				       if (strcmp (parametro, "RC") == 0)
					 Rc = faux;
				       else
					if (strcmp (parametro, "RE") == 0)
					  Re = faux;
					else
					  if (strcmp (parametro, "FC") == 0)
					    FC = faux;
					  else
					    if (strcmp (parametro, "EG") == 0)
					      Eg = faux;
					    else
					      error (E_PARAMDESCONOC);
		     }
		    else
		      if ((type == 'J') || (type == 'K'))
		       {
			faux = convertir (&c);

			if (strcmp (parametro, "IS") == 0)
			  Isf = faux;
			else
			 if (strcmp (parametro, "ISR") == 0)
			   Isr = faux;
			 else
			  if (strcmp (parametro, "VTO") == 0)
			    Vto = faux;
			  else
			   if (strcmp (parametro, "BETA") == 0)
			     Beta = faux;
			   else
			    if (strcmp (parametro, "LANDA") == 0)
			      Landa = faux;
			    else
			     if (strcmp (parametro, "M") == 0)
			       M = faux;
			     else
			      if (strcmp (parametro, "PB") == 0)
				Pb = faux;
			      else
			       if (strcmp (parametro, "FC") == 0)
				 FC = faux;
			       else
				if (strcmp (parametro, "CGS") == 0)
				  Cgs = faux;
				else
				 if (strcmp (parametro, "CGD") == 0)
				   Cgd = faux;
				 else
				  if (strcmp (parametro, "RS") == 0)
				    Rs = faux;
				  else
				   if (strcmp (parametro, "RD") == 0)
				     Rd = faux;
				   else
				    if (strcmp (parametro, "VTOTC") == 0)
				      VtoTc = faux;
				    else
				     if (strcmp (parametro, "XTB") == 0)
				       Xtb = faux;
				     else
				      if (strcmp (parametro, "XTI") == 0)
					Xti = faux;
				      else
				       if (strcmp (parametro, "EG") == 0)
					 Eg = faux;
				       else
					 error (E_PARAMDESCONOC);
		       }
		      else
			if (type == 'Z')
			 {
			  Taux = obt_func_trans (&c);

			  if (strcmp (parametro, "Zee") == 0)
			    Ti = Taux;
			  else
			    if (strcmp (parametro, "Zse") == 0)
			      Tr = Taux;
			    else
			      if (strcmp (parametro, "Zes") == 0)
				Tf = Taux;
			      else
				if (strcmp (parametro, "Zss") == 0)
				  To = Taux;
				else
				  error (E_PARAMDESCONOC);
			 }
			else
			 {
			  error (E_PARAMDESCONOC);
			  convertir (&c);
			 }
      }
    }
    while (c != '}');

    /* Si tenemos un elemento.. */
    if (type != 'Z')
     {
      /* ..a¤adimos a la lista el elemento en cuesti¢n si su valor es <> 0 */
      if ((((type == 'Q') || (type == 'P') || (type == 'N')) && (Is != 0)) ||
	  ((type != 'Q') && (value != 0)))
       {
	if (elem_ini == NIL)
	  elem_ini = elem_act = (reg_elem *) malloc (sizeof (reg_elem));
	else
	  elem_act =
	  elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));

	elem_act->siguiente = NIL;

	strcpy (elem_act->datos.nombre, name);

	elem_act->datos.tipo[0] = type;
	elem_act->datos.tipo[1] = 0;

	no_orden (elem_ini, elem_act);

	elem_act->datos.f = poly;

	elem_act->datos.CT = tempcoef;

	switch (type)
	{
	 case 'V':
	 case 'I':
	 {
	  elem_act->datos.valor = value;
	  elem_act->datos.signal = sig;
	  elem_act->datos.caract.frec = frequency;
	  elem_act->datos.fase = phase;
	  elem_act->datos.Tact = activetime;
	  break;
	 }

	 case 'A':
	 case 'D':
	 {
	  elem_act->datos.valor = value;
	  elem_act->datos.signal = sig;
	  elem_act->datos.caract.nodos[0] = ni;
	  elem_act->datos.caract.nodos[1] = nf;
	  break;
	 }

	 case 'B':
	 case 'E':
	 {
	  elem_act->datos.valor = value;
	  if ((elem_aux = buscar (elem_ini, through)) != NIL)
	    strcpy (elem_act->datos.caract.elem, elem_aux->datos.tipo);
	  else
	    error (E_ELEMNOENCON);
	  elem_act->datos.signal = sig;
	  break;
	 }

	 case 'R':
	 case 'L':
	 case 'C':
	 {
	  elem_act->datos.valor = value;
	  elem_act->datos.caract.cond_inic = cond_inic;
	  break;
	 }

	 case 'Q':
	 {
	  elem_act->datos.valor = Is;
	  elem_act->datos.signal = 'S';
	  elem_act->datos.fase = transtime;
	  elem_act->datos.caract.capacidad = Cjo;
	  break;
	 }
	}

	/* Si el elemento es un diodo, a¤adiremos sus elementos par sitos */
	if (type == 'Q')
	 {
	  /* Recordamos al diodo que les da origen */
	  elem_aux = elem_act;

	  /* Si existe la capacidad de la uni¢n.. */
	  if (Cjo != 0)
	   {
	    /* ..creamos el espacio preciso para ella */
	    elem_act =
	    elem_act->siguiente = (reg_elem *) malloc (sizeof(reg_elem));
	    elem_act->siguiente = NIL;

	    /* Su nombre ser  de la forma "CdNNN", siendo NNN el n§ de orden del
	       diodo al que corresponde */
	    strcpy (elem_act->datos.nombre, "Cd\x0");
	    strcat (elem_act->datos.nombre, &elem_aux->datos.tipo[1]);

	    /* Obtenemos el nombre interno */
	    elem_act->datos.tipo[0] = 'C';
	    elem_act->datos.tipo[1] = 0;
	    no_orden (elem_ini, elem_act);

	    /* Damos valores al resto de par metros */
	    elem_act->datos.valor = Cjo;
	    elem_act->datos.f.grado = elem_act->datos.f.coef[0] = 0;
	    elem_act->datos.CT = 0;
	    elem_act->datos.caract.cond_inic = 0;
	   }

	  /* Si hay resistencia par sita.. */
	  if (poly.coef[0] != 0)
	   {
	    /* ..creamos espacio para ella tambi‚n */
	    elem_act =
	    elem_act->siguiente = (reg_elem *) malloc (sizeof(reg_elem));

	    elem_act->siguiente = NIL;

	    /* Su nombre ser  de la forma "RsNNN", siendo NNN el n§ de orden del
	       diodo al que corresponde */
	    strcpy (elem_act->datos.nombre, "Rs\x0");
	    strcat (elem_act->datos.nombre, &elem_aux->datos.tipo[1]);

	    /* Obtenemos el nombre interno */
	    elem_act->datos.tipo[0] = 'R';
	    elem_act->datos.tipo[1] = 0;
	    no_orden (elem_ini, elem_act);

	    /* Damos valores al resto de par metros */
	    elem_act->datos.valor = poly.coef[0];
	    elem_act->datos.f.grado = elem_act->datos.f.coef[0] = 0;
	    elem_act->datos.CT = 0;
	   }
	 }

	/* pero, si el elemento es un transistor bipolar, a¤adimos los
	   elementos del modelo */
	else
	  if ((type =='P') || (type == 'N'))
	   {
	    /* ..obtenemos el n§ de orden del transistor */
	    strcpy (num, &elem_act->datos.tipo[1]);

	    /* Creamos los elementos del modelo, empezando por el diodo B-C */
	    if (elem_ini == NIL)
	      elem_ini = elem_act = (reg_elem *) malloc (sizeof (reg_elem));
	    else
	      elem_act =
	      elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	    elem_act->siguiente = NIL;

	    /* El nombre ser  de la forma "DrNNN", NNN = n§ de orden del tr. */
	    strcpy (elem_act->datos.nombre, "Dr\x0");
	    strcat (elem_act->datos.nombre, num);

	    elem_act->datos.tipo[0] = 'Q';
	    elem_act->datos.tipo[1] = 0;
	    no_orden (elem_ini, elem_act);

	    elem_act->datos.valor     = Is / Br;
	    elem_act->datos.signal    = 'S';
	    elem_act->datos.fase      = tr;
	    elem_act->datos.CT        = 0;
	    elem_act->datos.f.grado   = 0;
	    elem_act->datos.f.coef[0] = 0;
	    elem_act->datos.f.coef[1] = Vjc;
	    elem_act->datos.f.coef[2] = INFINITO;
	    elem_act->datos.f.coef[3] = 1e-10;
	    elem_act->datos.f.coef[4] = FC;
	    elem_act->datos.f.coef[5] = Eg;
	    elem_act->datos.f.coef[6] = Mjc;
	    elem_act->datos.caract.capacidad = Cjc;

	    /* Creamos el diodo base-emisor */
	    elem_act =
	    elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	    elem_act->siguiente = NIL;

	    /* El nombre ser  de la forma "DfNNN", NNN = n§ de orden del tr. */
	    strcpy (elem_act->datos.nombre, "Df\x0");
	    strcat (elem_act->datos.nombre, num);

	    elem_act->datos.tipo[0] = 'Q';
	    elem_act->datos.tipo[1] = 0;
	    no_orden (elem_ini, elem_act);

	    elem_act->datos.valor     = Is / Bf;
	    elem_act->datos.signal    = 'S';
	    elem_act->datos.fase      = tf;
	    elem_act->datos.CT        = 0;
	    elem_act->datos.f.grado   = 0;
	    elem_act->datos.f.coef[0] = 0;
	    elem_act->datos.f.coef[1] = Vje;
	    elem_act->datos.f.coef[2] = INFINITO;
	    elem_act->datos.f.coef[3] = 1e-10;
	    elem_act->datos.f.coef[4] = FC;
	    elem_act->datos.f.coef[5] = Eg;
	    elem_act->datos.f.coef[6] = Mje;
	    elem_act->datos.caract.capacidad = Cje;

	    /* Creamos las capacidades de las uniones, primero la de colec. */
	    if (Cjc != 0)
	     {
	      elem_act =
	      elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	      elem_act->siguiente = NIL;

	      /* El nombre ser  de la forma "CcNNN", NNN = n§ de orden del tr. */
	      strcpy (elem_act->datos.nombre, "Cc\x0");
	      strcat (elem_act->datos.nombre, num);

	      elem_act->datos.tipo[0] = 'C';
	      elem_act->datos.tipo[1] = 0;
	      no_orden (elem_ini, elem_act);

	      elem_act->datos.valor     = Cjc;
	      elem_act->datos.fase      = 0;
	      elem_act->datos.CT        = 0;
	      elem_act->datos.f.grado   = 0;
	      elem_act->datos.f.coef[0] = 0;
	      elem_act->datos.caract.cond_inic = 0;
	     }

	    /* Ahora la de emisor */
	    if (Cje != 0)
	     {
	      elem_act =
	      elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	      elem_act->siguiente = NIL;

	      /* El nombre ser  de la forma "CeNNN", NNN = n§ de orden del tr. */
	      strcpy (elem_act->datos.nombre, "Ce\x0");
	      strcat (elem_act->datos.nombre, num);

	      elem_act->datos.tipo[0] = 'C';
	      elem_act->datos.tipo[1] = 0;
	      no_orden (elem_ini, elem_act);

	      elem_act->datos.valor     = Cje;
	      elem_act->datos.fase      = 0;
	      elem_act->datos.CT        = 0;
	      elem_act->datos.f.grado   = 0;
	      elem_act->datos.f.coef[0] = 0;
	      elem_act->datos.caract.cond_inic = 0;
	     }

	    /* Creamos los diodos B-E y B-C no ideales */
	    if (Isc != 0)
	     {
	      elem_act =
	      elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	      elem_act->siguiente = NIL;

	      /* El nombre ser  de la forma "NrNNN", NNN = n§ de orden del tr. */
	      strcpy (elem_act->datos.nombre, "Nr\x0");
	      strcat (elem_act->datos.nombre, num);

	      elem_act->datos.tipo[0] = 'Q';
	      elem_act->datos.tipo[1] = 0;
	      no_orden (elem_ini, elem_act);

	      elem_act->datos.valor     = Isc;
	      elem_act->datos.signal    = 'S';
	      elem_act->datos.fase      = 0;
	      elem_act->datos.CT        = tempcoef;
	      elem_act->datos.f.grado   = 0;
	      elem_act->datos.f.coef[0] = 0;
	      elem_act->datos.f.coef[1] = 0;
	      elem_act->datos.f.coef[2] = INFINITO;
	      elem_act->datos.f.coef[3] = 1e-10;
	      elem_act->datos.f.coef[4] = 0;
	      elem_act->datos.f.coef[5] = Eg;
	      elem_act->datos.f.coef[6] = 0;
	      elem_act->datos.caract.capacidad = 0;
	     }

	    if (Ise != 0)
	     {
	      elem_act =
	      elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	      elem_act->siguiente = NIL;

	      /* El nombre ser  de la forma "NfNNN", NNN = n§ de orden del tr. */
	      strcpy (elem_act->datos.nombre, "Nf\x0");
	      strcat (elem_act->datos.nombre, num);

	      elem_act->datos.tipo[0] = 'Q';
	      elem_act->datos.tipo[1] = 0;
	      no_orden (elem_ini, elem_act);

	      elem_act->datos.valor     = Ise;
	      elem_act->datos.signal    = 'S';
	      elem_act->datos.fase      = 0;
	      elem_act->datos.CT        = tempcoef;
	      elem_act->datos.f.grado   = 0;
	      elem_act->datos.f.coef[0] = 0;
	      elem_act->datos.f.coef[1] = 0;
	      elem_act->datos.f.coef[2] = INFINITO;
	      elem_act->datos.f.coef[3] = 1e-10;
	      elem_act->datos.f.coef[4] = 0;
	      elem_act->datos.f.coef[5] = Eg;
	      elem_act->datos.f.coef[6] = 0;
	      elem_act->datos.caract.capacidad = 0;
	     }

	    /* Creamos los generadores de corriente C-E y E-C */
	    elem_act =
	    elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	    elem_act->siguiente = NIL;

	    /* El nombre ser  de la forma "I1NNN", NNN = n§ de orden del tr. */
	    strcpy (elem_act->datos.nombre, "I1\x0");
	    strcat (elem_act->datos.nombre, num);

	    /* Obtenemos el nombre interno */
	    elem_act->datos.tipo[0] = 'E';
	    elem_act->datos.tipo[1] = 0;
	    no_orden (elem_ini, elem_act);

	    /* Fijamos el resto de par metros */
	    elem_act->datos.valor     = Bf;
	    elem_act->datos.signal    = 'C';
	    elem_act->datos.fase      = 0;
	    elem_act->datos.CT        = 0;
	    elem_act->datos.f.grado   = 0;
	    elem_act->datos.f.coef[0] = 0;
	    strcpy (through, "Df\x0");
	    strcat (through, num);
	    strcpy (elem_act->datos.caract.elem,
		    buscar (elem_ini, through)->datos.tipo);

	    elem_act =
	    elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	    elem_act->siguiente = NIL;

	    /* El nombre ser  de la forma "I2NNN", NNN = n§ de orden del tr. */
	    strcpy (elem_act->datos.nombre, "I2\x0");
	    strcat (elem_act->datos.nombre, num);

	    /* Obtenemos el nombre interno */
	    elem_act->datos.tipo[0] = 'E';
	    elem_act->datos.tipo[1] = 0;
	    no_orden (elem_ini, elem_act);

	    /* Fijamos el resto de par metros */
	    elem_act->datos.valor     = Br;
	    elem_act->datos.signal    = 'C';
	    elem_act->datos.fase      = 0;
	    elem_act->datos.CT        = 0;
	    elem_act->datos.f.grado   = 0;
	    elem_act->datos.f.coef[0] = 0;
	    strcpy (through, "Dr\x0");
	    strcat (through, num);
	    strcpy (elem_act->datos.caract.elem,
		    buscar (elem_ini, through)->datos.tipo);

	    /* A¤adimos las resistencias de B, E y C */
	    if (Rb != 0)
	     {
	      elem_act =
	      elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	      elem_act->siguiente = NIL;

	      /* El nombre ser  de la forma "RbNNN", NNN = n§ de orden del tr. */
	      strcpy (elem_act->datos.nombre, "Rb\x0");
	      strcat (elem_act->datos.nombre, num);

	      elem_act->datos.tipo[0] = 'R';
	      elem_act->datos.tipo[1] = 0;
	      no_orden (elem_ini, elem_act);

	      elem_act->datos.valor     = Rb;
	      elem_act->datos.CT        = 0;
	      elem_act->datos.f.grado   = 0;
	      elem_act->datos.f.coef[0] = Var;
	      elem_act->datos.f.coef[1] = Vaf;
	     }

	    if (Rc != 0)
	     {
	      elem_act =
	      elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	      elem_act->siguiente = NIL;

	      /* El nombre ser  de la forma "RcNNN", NNN = n§ de orden del tr. */
	      strcpy (elem_act->datos.nombre, "Rc\x0");
	      strcat (elem_act->datos.nombre, num);

	      elem_act->datos.tipo[0] = 'R';
	      elem_act->datos.tipo[1] = 0;
	      no_orden (elem_ini, elem_act);

	      elem_act->datos.valor     = Rc;
	      elem_act->datos.CT        = 0;
	      elem_act->datos.f.grado   = 0;
	      elem_act->datos.f.coef[0] = 0;
	     }

	    if (Re != 0)
	     {
	      elem_act =
	      elem_act->siguiente = (reg_elem *) malloc (sizeof (reg_elem));
	      elem_act->siguiente = NIL;

	      /* El nombre ser  de la forma "ReNNN", NNN = n§ de orden del tr. */
	      strcpy (elem_act->datos.nombre, "Re\x0");
	      strcat (elem_act->datos.nombre, num);

	      elem_act->datos.tipo[0] = 'R';
	      elem_act->datos.tipo[1] = 0;
	      no_orden (elem_ini, elem_act);

	      elem_act->datos.valor     = Re;
	      elem_act->datos.CT        = 0;
	      elem_act->datos.f.grado   = 0;
	      elem_act->datos.f.coef[0] = 0;
	     }
	   }
       }
     }

    /* pero, si tenemos un cuadripolo.. */
    else
     {
      /* ..lo a¤adimos a la lista de cuadripolos */
      if (cuad_ini == NIL)
	cuad_ini = cuad_act = (reg_cuadri *) malloc (sizeof (reg_cuadri));
      else
	cuad_act =
	cuad_act->siguiente = (reg_cuadri *) malloc (sizeof (reg_cuadri));

      cuad_act->siguiente = NIL;

      strcpy (cuad_act->datos.nombre, name);
      sprintf (cuad_act->datos.tipo, "Z%#03.3u", ++n_cuad);

      cuad_act->datos.Ti = Ti;
      cuad_act->datos.Tr = Tr;
      cuad_act->datos.Tf = Tf;
      cuad_act->datos.To = To;
     }
   }
 }
 while ((c != '#') && (!feof (fuente)));

 /* Almacenamos la direccion de la lista de cuadripolos */
 *primer_cuad = cuad_ini;

 return (elem_ini);
}