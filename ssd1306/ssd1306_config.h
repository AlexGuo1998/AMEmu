#pragma once
//for internal use only, do NOT include this file


#define RESOURCE_FILE "pixel.bmp"

//define REDUCE_REFRESHING to refresh screen only when data actually changed, rather than update when writing anything
//will probably reduce GPU load, but increase CPU load
//#define REDUCE_REFRESHING


//define CHARGE_PUMP_AVALIABLE to enable decoding of charge pump commands
#define CHARGE_PUMP_AVALIABLE
