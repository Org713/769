#ifndef _ENTITY_H
#define _ENTITY_H

#include "..\mib\mib.h"
#include "..\sdl\sdl.h"
#include "..\common.h"

#include "..\interface\data_interface_tx.h"   //2016.12.26
#include "..\data_net\data_ntx.h"

#include "..\interrupt_handler_test\interrupt_handler.h" //2016.12.27
#include "..\interface\data_interface_rx.h"  //2016.12.27
#include "..\data_net\data_nrx.h"

#include "..\data_dlcpump\data_dlctxpump.h" //2017.1.2
#include "..\data_rly\data_rly_tx.h"
#include "..\rt\rt_find.h"
#include "..\rt\rt_ctrl.h"

#include "..\data_rly\rly_jam.h"   //2017.1.5
#include "..\data_dlcrxfilter\data_dlc_rxfilter.h"
typedef struct _entity
{
	void* myself;//must be at the first 
	Mib mib;//must be at the third


	Data_Interface_Tx data_interface_tx[DATA_PRI_NUM];//2016.12.26
	Data_Ntx data_ntx[DATA_PRI_NUM][MAX_NODE_CNT];//2016.12.28

	Interrupt_Handler interrupt_handler;//2016.12.27

	Data_Interface_Rx data_interface_rx[DATA_PRI_NUM];//2016.12.27
	Data_Nrx data_nrx[DATA_PRI_NUM][MAX_NODE_CNT];//2016.12.28

	Data_Dlctxpump data_dlctxpump;//2017.1.2
	Data_Rly_Tx data_rly_tx;
	Rt_Ctrl rt_ctrl;
	Rt_Find rt_find;

	Data_Dlc_RxFilter data_dlc_rxfilter;//2017.1.5
	Rly_Jam rly_jam;

}Entity;

extern void EntitySetup(Entity* ent);
extern void EntityInit(Entity* ent);
//extern void ChanSimTestMdlInit(Entity *ent);
#endif