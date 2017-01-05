#ifndef _DATA_NRX_H
#define _DATA_NRX_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"

typedef struct _t_data_interface_rx_ind_param
{
	unsigned short *data;//����
	unsigned short len;//���ݳ���
	unsigned short sa;
	unsigned short pri;
	unsigned short svc_type;
}T_Data_Interface_Rx_Ind_Param;

typedef struct _data_ntx_to_data_nrx_test_param
{
	unsigned short *data;
	unsigned short len;
	unsigned short da;  
	unsigned short pri;
	unsigned short svc_type;
	unsigned short sa;  //����
}Data_Ntx_To_Data_Nrx_Test_Param;

typedef struct _t_data_rly_rx_ind_param
{
	unsigned short *net_pkt;
	unsigned short len;
	unsigned short pri;
}T_Data_Rly_Rx_Ind_Param;

typedef struct _data_nrx_ind_param
{
	unsigned short	*net_pkt;
	unsigned short	len;
	unsigned short	sa;
}Data_Nrx_Ind_Param;

typedef struct _data_nrx
{
	void *entity;
	Mib *mib;
	u8 state;

	unsigned short	data_pri_id;//�������ȼ�ID  0.1.2.3
	unsigned short	module_id;//��ģ���id�� 0-31  2016.12.28

	//Ϊÿ�ְ����ö����İ���ż�¼��Ϣ�������ж��Ƿ���չ��ð���Ϣ����
	unsigned short	unicast_pkt_sn;		//����  unicast
	unsigned short  rly_unicast_pkt_sn;	//����Ŀ�Ľڵ㵥�� rly_unicast
	unsigned short	broadcast_pkt_sn;	//�㲥  broadcast
	unsigned short  multicast_pkt_sn;	//�鲥  multicast

	/***************sig received*****************/
	//�� �� �� ����
	Signal	data_ntx_to_data_nrx_test;
	Data_Ntx_To_Data_Nrx_Test_Param data_ntx_to_data_nrx_test_param;

	Signal  data_nrx_ind;
	Data_Nrx_Ind_Param data_nrx_ind_param;
	/***************sig to send*****************/
	Signal* data_interface_rx_ind;
	Signal* data_rly_rx_ind;//�м� 2017.1.5
}Data_Nrx;

extern void DataNrxSetup(Data_Nrx* proc);
extern void DataNrxInit(Data_Nrx* proc);


#endif