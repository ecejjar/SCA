editor.exe : editor.obj editor.def editor.res
        tlink /Tw /c /3 /v c0wm editor,\
                         editor,\
                         ,\
                         import cwinm cm,\
                         editor.def
        rc editor.res

editor.obj : editor.c
        bcc -c -mm -v -WE editor.c

editor.res : editor.rc editor.ico
	rc -r editor.rc
