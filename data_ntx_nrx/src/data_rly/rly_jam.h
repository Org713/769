#ifndef _RLY_JAM_H
#define _RLY_JAM_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"

typedef struct _data_rly_tx_jam_ind_param
{
	unsigned short jam_flag;//jam_flag=0/1, 1:ÓµÈû   0£ºÓµÈû½â³ý
}Data_Rly_Tx_Jam_Ind_Param;

typedef struct _rly_jam
{
	void *entity;
	Mib *mib;
	u8 state;

	/***************sig received*****************/
	Signal	data_rly_tx_jam_ind;
	Data_Rly_Tx_Jam_Ind_Param data_rly_tx_jam_ind_param;
	/***************sig to send*****************/

}Rly_Jam;

extern void RlyJamInit(Rly_Jam* proc);
extern void RlyJamSetup(Rly_Jam* proc);


#endif