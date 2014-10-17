winresol.exe : winresol.obj resprocs.obj resoldlg.obj \ 
	       resolall.obj resolptr.obj resolfrc.obj \
	       resoltmp.obj restmp2.obj  resolsns.obj\
	       C:\BORLANDC\LIB\wincompl.lib C:\BORLANDC\LIB\winpolin.lib \
	       winresol.def winresol.res winresol.rsp
	tlink @winresol.rsp
	rc winresol.res

winresol.obj : winresol.c C:\BORLANDC\INCLUDE\winresol.h \
			  C:\BORLANDC\INCLUDE\resolver.h
	bcc -c -mm -v -y -WE winresol.c

resprocs.obj : resprocs.c C:\BORLANDC\INCLUDE\winresol.h \
			  C:\BORLANDC\INCLUDE\resolver.h
	bcc -c -mm -v -y -WE resprocs.c

resoldlg.obj : resoldlg.c C:\BORLANDC\INCLUDE\winresol.h \
			  C:\BORLANDC\INCLUDE\resolver.h
	bcc -c -mm -v -y -WE resoldlg.c

resolall.obj : resolall.c C:\BORLANDC\INCLUDE\winresol.h \
			  C:\BORLANDC\INCLUDE\resolver.h
	bcc -c -mm -v -y -WE resolall.c

resolptr.obj : resolptr.c C:\BORLANDC\INCLUDE\resolver.h
	bcc -c -mm -v -y -WE resolptr.c

resolfrc.obj : resolfrc.c C:\BORLANDC\INCLUDE\resolver.h
	bcc -c -mm -v -y -WE resolfrc.c

resoltmp.obj : resoltmp.c C:\BORLANDC\INCLUDE\resolver.h
	bcc -c -mm -v -y -WE resoltmp.c

restmp2.obj  : restmp2.c  C:\BORLANDC\INCLUDE\resolver.h
	bcc -c -mm -v -y -WE restmp2.c

resolsns.obj : resolsns.c C:\BORLANDC\INCLUDE\resolver.h
	bcc -c -mm -v -y -WE resolsns.c

winresol.res : reshead.rc resmenu.rc resabrir.dlg resacerc.dlg \
	       winresol.ico winresol.bmp
	copy reshead.rc + resmenu.rc + resabrir.dlg + resacerc.dlg winresol.rc
	rc -r winresol.rc

