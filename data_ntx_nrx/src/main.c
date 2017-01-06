#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "entity/entity.h"

#define N 3
Sdl_Core sdlc;
Entity ent_test[N];
//unsigned short tx_data_0[]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
unsigned long long systime = 0;

unsigned long long SysGetTime(void)
{
	return systime;
}

int main(void)
{
	unsigned int i = 0,j = 0;
	Signal *sig;
	SdlCoreInit();	
	for(i = 0;i<N;i++)
	{		
		ent_test[i].mib.local_id = i;
		EntityInit(&ent_test[i]);
		EntitySetup(&ent_test[i]);
	}
	while(1)
	{
		DbgPrint("\n-# %d #-------------------------------------\n",systime);
		
		if (systime == 2)
		{
			//测试接口
			//sig = &ent_test[0].interrupt_handler.test_ind;
			//sig->func(sig);

			//测试收数据
			sig = &ent_test[0].data_dlc_rxfilter.test_ind;
			sig->func(sig);
		}


		// sdl execsignal
		for(i=0 ; i<30; i++)
		{
			CheckSignal( );
		}

		CheckTimer( );

		systime ++;
		//DbgPrint("system++,=%d \n",systime);
		if (systime == 20)
		{
			break;
		}
	}
	system("pause"); 
	return 0;
}


