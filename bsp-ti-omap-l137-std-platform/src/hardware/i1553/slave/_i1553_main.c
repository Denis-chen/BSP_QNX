/*
 * Halliburton ESG
 * 1553 Driver (Slave)
 * Author: Wei Sun
 * Platform: OMAPL137-HT standard design platform
 * Create Date: 07/05/2016
 */


#include <sys/procmgr.h>
#include <dlfcn.h>
#include "proto.h"

int main(int argc, char *argv[])
{
  i1553_dev_t  *head = NULL, *tail = NULL, *dev;
  void         *drventry, *dlhdl;
  siginfo_t    info;
  sigset_t     set;
  int          i, c, devnum = 0;

  if(ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
	  perror("ThreadCtl");
	  return(!EOK);
  }

  _i1553_init_iofunc();

  while((c = getopt(argc, argv, "u:d")) != -1)
  {
	switch(c){
        case 'u':
		     devnum = strtol(optarg, NULL, 0);
		     break;

        case 'd':
        	if((drventry = _i1553_dlload(&dlhdl, optarg)) == NULL) {
        		perror("i1553_load_driver() failed");
        		return(-1);
		  }
        	do {
        		if ((dev = calloc(1, sizeof(i1553_dev_t))) == NULL)
        			goto cleanup;

        		if(argv[optind] == NULL || *argv[optind] == '-')
        			dev->opts = NULL;
        		else
        			dev->opts = strdup(argv[optind]);

        		++optind;
        		dev->funcs = (i1553_funcs_t *)drventry;
        		dev->devnum = devnum++;
        		dev->dlhdl = dlhdl;

        		i = _i1553_create_instance(dev);
        		if(dev->opts)
        		{
        			free(dev->opts);
        		}

        		if(i != EOK) {
        			perror("spi_create_instance() failed");
        			free(dev);
        			goto cleanup;
        		}

        		if(head) {
        			tail->next = dev;
        			tail = dev;
        		}
        		else
        			head = tail = dev;
        	} while (optind < argc && *(optarg = argv[optind]) != '-');

        	/*
        	 *  Now we only support one dll
        	 */
        	goto start_i1553;
        	break;
	}
  }

start_i1553:
   if (head)
   {
	   /*background the process */
	   procmgr_daemon(0, PROCMGR_DAEMON_NOCLOSE | PROCMGR_DAEMON_NODEVNULL);

	   sigemptyset(&set);
	   sigaddset(&set, SIGTERM);

	   for(;;) {
		   if (SignalWaitInfo(&set, &info) == -1)
			   continue;

		   if (info.si_signo == SIGTERM)
			   break;
	   }
   }


cleanup:
   while (dev == head ){
	   dispatch_unblock(dev->ctp);
	   dispatch_destroy(dev->dpp);
	   resmgr_detach(dev->dpp, dev->id, _RESMSG_DETACH_ALL);

	   /*
	    * Disable hardware
	    */
	   dev->funcs->fini(dev->drvhdl);
	   head = dev->next;
	   free(dev);
   }

   dlclose(dlhdl);
  return (EOK);

}
