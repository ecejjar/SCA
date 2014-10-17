wingraf.exe : wingraf.obj grafgraf.obj grafdlgs.obj\
	      C:\BORLANDC\LIB\wincompl.lib C:\BORLANDC\LIB\winpolin.lib \
	      wingraf.def wingraf.res
	tlink /Tw /c /3 /v c0wm wingraf grafgraf grafdlgs,\
			 wingraf,\
			 ,\
			 wincompl winpolin import cwm mathwm cm,\
			 wingraf.def
	rc wingraf.res

wingraf.obj : wingraf.c \BORLANDC\INCLUDE\complejo.h \
			\BORLANDC\INCLUDE\polinom.h  \
			\BORLANDC\INCLUDE\graficos.h
	bcc -c -mm -v -WE wingraf.c

grafgraf.obj : grafgraf.c \BORLANDC\INCLUDE\complejo.h \
			  \BORLANDC\INCLUDE\polinom.h  \
			  \BORLANDC\INCLUDE\graficos.h
	bcc -c -mm -v -WE grafgraf.c

grafdlgs.obj : grafdlgs.c \BORLANDC\INCLUDE\graficos.h
	bcc -c -mm -v -WE grafdlgs.c

wingraf.res : grafhead.rc grafmenu.rc gsubmenu.rc \
			  wingraf.ico grafgraf.ico \
			  wingraf.bmp \
			  grafgraf.cur \
			  grafdlg1.dlg grafdlg2.dlg grafdlg3.dlg \
			  parametr.dlg escalas.dlg colores.dlg
	copy grafhead.rc + grafmenu.rc + gsubmenu.rc wingraf.rc
	copy wingraf.rc + grafdlg1.dlg + grafdlg2.dlg + grafdlg3.dlg + \
	     parametr.dlg + escalas.dlg + colores.dlg \
	     wingraf.rc
	rc -r wingraf.rc
