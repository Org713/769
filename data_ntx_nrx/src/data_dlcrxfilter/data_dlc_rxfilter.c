#include "data_dlc_rxfilter.h"
enum {IDLE};


void DataDlcRxFilterInit(Data_Dlc_RxFilter* proc)
{
	proc->state = IDLE;
}

void DataDlcRxFilterSetup(Data_Dlc_RxFilter* proc)
{

}
