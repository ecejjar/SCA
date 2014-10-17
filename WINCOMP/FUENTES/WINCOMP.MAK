wincomp.exe : wincomp.obj \
	      compdebg.obj compelem.obj comprama.obj compresp.obj companal.obj \ 
	      wincomp.def wincomp.res
	tlink /Tw /c /3 /v c0wm wincomp compdebg compelem comprama compresp companal,\
			 wincomp,\
			 ,\
			 import cwm mathwm cm,\
			 wincomp.def
	rc wincomp.res

wincomp.obj : wincomp.c complejo.h polinom.h compilar.h
	bcc -c -mm -v -WE wincomp.c

compdebg.obj : compdebg.c
	bcc -c -mm -v -WE compdebg.c

compelem.obj : compelem.c complejo.h polinom.h compilar.h
	bcc -c -mm -v -WE compelem.c

comprama.obj : comprama.c complejo.h polinom.h compilar.h
	bcc -c -mm -v -WE comprama.c

compresp.obj : compresp.c complejo.h polinom.h compilar.h
	bcc -c -mm -v -WE compresp.c

companal.obj : companal.c complejo.h polinom.h compilar.h
	bcc -c -mm -v -WE companal.c

wincomp.res : comphead.rc compmenu.rc \
			  wincomp.ico \
			  wincomp.bmp \
			  compdlg1.dlg compdlg2.dlg compdlg3.dlg
	copy comphead.rc+compmenu.rc+compdlg1.dlg+compdlg2.dlg+compdlg3.dlg \
	     wincomp.rc
	rc -r wincomp.rc
