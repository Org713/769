#ifndef _MIB_H
#define _MIB_H

#include "..\common.h"
#include "..\sdl\sdl.h"
#include "..\mib\mib.h"
typedef struct _mib
{
	//unsigned char rsv;
	u8 local_id;
	u8 cur_uts;
	u8 cur_ts;
	u32 cur_tf;
	u8 cur_slot;
	Slot_Table slot_table;
	volatile u8 RND;
	volatile u8 mac_rx_ind_flag;
}Mib;



extern void MibInit(Mib*);

#endif