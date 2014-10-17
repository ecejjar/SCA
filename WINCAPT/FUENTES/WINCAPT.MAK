wincapt.exe : wincapt.obj captfich.obj captdibu.obj captdlgs.obj\
	      wincapt.def wincapt.res
	tlink /Tw /c /3 /v c0wm wincapt captdlgs captfich captdibu,\
			 wincapt,\
			 ,\
			 import cwm mathwm cm,\
			 wincapt.def
	rc wincapt.res

wincapt.obj : wincapt.c wincapt.h capturar.h
	bcc -Qx -c -mm -v -WE wincapt.c

captfich.obj : captfich.c wincapt.h capturar.h
	bcc -Qx -c -mm -v -WE captfich.c

captdibu.obj : captdibu.c wincapt.h capturar.h
	bcc -Qx -c -mm -v -WE captdibu.c

captdlgs.obj : captdlgs.c wincapt.h capturar.h
	bcc -Qx -c -mm -v -WE captdlgs.c

wincapt.res : capthead.rc captmenu.rc \
			  pinza.cur alicate.cur soldador.cur \
			  wincapt.ico \
			  wincapt.bmp \
			  capabrir.dlg capguard.dlg capacerc.dlg \
			  capcarac.dlg carcuad.dlg editfunc.dlg \
			  biblio0.dlg biblio1.dlg biblio2.dlg \
			  biblio3.dlg biblio4.dlg
	copy capthead.rc+captmenu.rc+\
	     capabrir.dlg+capguard.dlg+capacerc.dlg+capcarac.dlg \
	     wincapt.rc
	copy wincapt.rc+biblio0.dlg+biblio1.dlg+biblio2.dlg+\
	     biblio3.dlg+biblio4.dlg \
	     wincapt.rc
	rc -r wincapt.rc

capcarac.dlg : carhead.dlg carrlc.dlg cartranb.dlg cartranj.dlg cartranm.dlg \
	       cardiodo.dlg cargen.dlg carmedid.dlg carnudos.dlg carcuad.dlg \
	       editfunc.dlg
	copy carhead.dlg+carrlc.dlg+cartranb.dlg+cartranj.dlg+\
	     cartranm.dlg+cardiodo.dlg+cargen.dlg+carmedid.dlg \
	     capcarac.dlg
	copy capcarac.dlg+carcuad.dlg+editfunc.dlg \
	     capcarac.dlg


