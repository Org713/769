#include "rt_find.h"

static int DataNtxRtFindReq(Signal* sig)
{
	return 0;
}

void RtFindInit(Rt_Find* proc)
{

}
void RtFindSetup(Rt_Find* proc)
{
	unsigned short i,j;
	Signal* sig;
	for (i = 0; i < DATA_PRI_NUM; i++)
	{
		for (j = 0; j < MAX_NODE_CNT; j++)
		{
			sig = &proc->data_ntx_rt_find_req[i][j];
			sig->next=0;
			sig->src=0;
			sig->dst=proc;
			sig->func=DataNtxRtFindReq;
			sig->pri=SDL_PRI_NORM;
			sig->param = &proc->data_ntx_rt_find_req_param[i][j];
		}
	}
}