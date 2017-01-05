#include "rly_jam.h"

static int DataRlyTxJamInd(Signal* sig)
{
	Rly_Jam *proc = (Rly_Jam*)sig->dst;
	Data_Rly_Tx_Jam_Ind_Param *param = (Data_Rly_Tx_Jam_Ind_Param*)sig->param;

	return 0;
}

void RlyJamInit(Rly_Jam* proc)
{

}
void RlyJamSetup(Rly_Jam* proc)
{

	Signal* sig;

	sig = &proc->data_rly_tx_jam_ind;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataRlyTxJamInd;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_rly_tx_jam_ind_param;
}
	