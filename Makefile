all	:pserver pclient runajob
install	: pserver pclient runajob
	 mv pserver /usr/bin
	 mv pclient /usr/bin
	 mv runajob /usr/bin
	 cp share/pserverd /etc/rc.d/init.d/	
	 cp share/lfsfunctions /etc/rc.d/init.d/	
	 ln -s -f /etc/rc.d/init.d/pserverd /etc/rc.d/rc3.d/S98pserverd
	 ln -s -f /etc/rc.d/init.d/pserverd /etc/rc.d/rc3.d/K98pserverd
	 ln -s -f /etc/rc.d/init.d/pserverd /etc/rc.d/rc5.d/S98pserverd
	 ln -s -f /etc/rc.d/init.d/pserverd /etc/rc.d/rc5.d/K98pserverd
clean	:
	  rm -f *.o
pserver	: pserver.o
	 cc -static -opserver pserver.o -lpthread
pclient	: pclient.o
	 cc -static -opclient pclient.o
runajob	: runajob.o
	 cc -static -orunajob runajob.o
pserver.o	: pserver.c
		 cc -c pserver.c
pclient.o	: pclient.c
		 cc -c pclient.c
runajob.o	: runajob.c
		 cc -c runajob.c
