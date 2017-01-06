#include "entity.h"

void EntitySetup(Entity* ent)
{
	unsigned short i,j;
	ent->myself=ent;
	/**** setup ************/

	for (i = 0; i < DATA_PRI_NUM; i++)
	{
		ent->data_interface_tx[i].entity = ent;//2016.12.26 
		DataInterfaceTxSetup(&ent->data_interface_tx[i]);
		for (j = 0; j < MAX_NODE_CNT; j++) //2016.12.28
		{
			ent->data_ntx[i][j].entity = ent;
			DataNtxSetup(&ent->data_ntx[i][j]);

			ent->data_nrx[i][j].entity = ent;
			DataNrxSetup(&ent->data_nrx[i][j]);
		}
		
		ent->data_interface_rx[i].entity = ent;//2016.12.27
		DataInterfaceRxSetup(&ent->data_interface_rx[i]);
	}
	
	ent->interrupt_handler.entity = ent;//2016.12.27
	InterruptHandlerSetup(&ent->interrupt_handler);

	ent->data_dlctxpump.entity = ent; //2017.1.2
	DataDlcTxPumpSetup(&ent->data_dlctxpump);

	ent->data_rly_tx.entity = ent;
	DataRlyTxSetup(&ent->data_rly_tx);

	ent->rt_ctrl.entity = ent;
	RtCtrlSetup(&ent->rt_ctrl);

	ent->rt_find.entity = ent;
	RtFindSetup(&ent->rt_find);

	ent->data_dlc_rxfilter.entity = ent;//2017.1.5
	DataDlcRxFilterSetup(&ent->data_dlc_rxfilter);

	ent->rly_jam.entity = ent;
	RlyJamSetup(&ent->rly_jam);
	/****** ***** ***** **** signal link ******************************/

	for ( i = 0; i < DATA_PRI_NUM; i++)    //信号连接 2016.12.27
	{
		ent->interrupt_handler.svc_data_tx_req[i] = &ent->data_interface_tx[i].svc_data_tx_req;//  发
		ent->data_interface_tx[i].svc_data_tx_cfm = &ent->interrupt_handler.svc_data_tx_cfm[i];

		ent->data_interface_rx[i].svc_data_rx_ind = &ent->interrupt_handler.svc_data_rx_ind[i];//收

		for ( j = 0; j < MAX_NODE_CNT; j++)
		{
			ent->data_interface_tx[i].data_interface_tx_req[j] = &ent->data_ntx[i][j].data_interface_tx_req;//2016.12.28
			ent->data_ntx[i][j].data_interface_tx_cfm = &ent->data_interface_tx[i].data_interface_tx_cfm;

			ent->data_nrx[i][j].data_interface_rx_ind = &ent->data_interface_rx[i].data_interface_rx_ind;//收

			//测试
			ent->data_ntx[i][j].data_ntx_to_data_nrx_test = &ent->data_nrx[i][j].data_ntx_to_data_nrx_test;//2016.12.28

			ent->data_ntx[i][j].data_ntx_rt_find_req = &ent->rt_find.data_ntx_rt_find_req[i][j];//2017.1.4
			ent->rt_find.data_ntx_rt_find_cfm[i][j] = &ent->data_ntx[i][j].data_ntx_rt_find_cfm;

			ent->data_ntx[i][j].data_ntx_rt_req = &ent->rt_ctrl.data_ntx_rt_req[i][j];//2017.1.4
			ent->rt_ctrl.data_ntx_rt_cfm[i][j] = &ent->data_ntx[i][j].data_ntx_rt_cfm;//信号对应关系

			ent->data_rly_tx.data_rly_tx_req[i][j] = &ent->data_ntx[i][j].data_rly_tx_req;//2017.1.5
			ent->data_ntx[i][j].data_rly_tx_cfm =  &ent->data_rly_tx.data_rly_tx_cfm[i][j];

			ent->data_ntx[i][j].dlc_txpump_tx_req = &ent->data_dlctxpump.dlc_txpump_tx_req[i][j];//2017.1.3
			ent->data_dlctxpump.dlc_txpump_tx_cfm[i][j] = &ent->data_ntx[i][j].dlc_txpump_tx_cfm;//

			ent->data_dlc_rxfilter.data_nrx_ind[i][j] = &ent->data_nrx[i][j].data_nrx_ind;//2017.1.5
			ent->data_rly_tx.data_rly_tx_jam_ind = &ent->rly_jam.data_rly_tx_jam_ind;

			ent->data_nrx[i][j].data_rly_rx_ind = &ent->data_rly_tx.data_rly_rx_ind;//2017.1.6

		}
	}
}
void EntityInit(Entity* ent)
{
	unsigned short i,j;
	//SdlCoreInit(&ent->sdlc);
	MibInit(&ent->mib);   // 注意修改
	/************************* mib *****************************/
	
	
	for ( i = 0; i < DATA_PRI_NUM; i++)
	{
		ent->data_interface_tx[i].mib = &ent->mib;//2016.12.26
		ent->data_interface_rx[i].mib = &ent->mib;//2016.12.27
		for (j = 0; j < MAX_NODE_CNT; j++)
		{
			ent->data_ntx[i][j].mib = &ent->mib;
			ent->data_nrx[i][j].mib = &ent->mib;
		}
	}

	ent->interrupt_handler.mib = &ent->mib;//2016.12.27

	ent->data_dlctxpump.mib = &ent->mib;//2017.1.2
	ent->data_rly_tx.mib = &ent->mib;
	ent->rt_ctrl.mib = &ent->mib;
	ent->rt_find.mib	 = &ent->mib;

	ent->data_dlc_rxfilter.mib = &ent->mib;//2017.1.5
	ent->rly_jam.mib = &ent->mib;
	/************************* entity init ********************/



	for (i = 0; i < DATA_PRI_NUM; i++)
	{
		ent->data_interface_tx[i].data_pri_id = i;//0 1 2 3 数据优先级对应模块
		DataInterfaceTxInit(&ent->data_interface_tx[i]);//2016.12.26

		for (j = 0; j < MAX_NODE_CNT; j++)
		{
			ent->data_ntx[i][j].data_pri_id = i;//2017.1.4
			ent->data_ntx[i][j].module_id = j;//2016.12.28  优先级i对应的节点j的data_ntx
			DataNtxInit(&ent->data_ntx[i][j]);

			ent->data_nrx[i][j].data_pri_id = i;
			ent->data_nrx[i][j].module_id = j;
			DataNrxInit(&ent->data_nrx[i][j]);
		}		
		ent->data_interface_rx[i].data_pri_id = i;  //2016.12.27
		DataInterfaceRxInit(&ent->data_interface_rx[i]);	
	}
	InterruptHandlerInit(&ent->interrupt_handler);//2016.12.27

	DataDlcTxPumpInit(&ent->data_dlctxpump);//2017.1.2
	DataRlyTxInit(&ent->data_rly_tx);
	RtCtrlInit(&ent->rt_ctrl);
	RtFindInit(&ent->rt_find);

	DataDlcRxFilterInit(&ent->data_dlc_rxfilter);//2017.1.5
	RlyJamInit(&ent->rly_jam);
}