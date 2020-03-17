#include "Ds64RomSearch.h"
#include "../Src/DS18B20/DS18B20.h"



volatile GuidSearchTypdef DsGuid[DS_DEVICE_NUM]={0};




//int16_t DS_SerchSpecificPosInArr(GuidSearchTypdef guidArr[], uint8_t maxNum, uint8_t destPos)
//{
//    if(NULL == guidArr){
//        return -1;
//    }
//    uint8_t index=0;
//    for(index=0; index<maxNum; ++index){
//        if(guidArr[index].pos == destPos){
//            break;
//        }
//    }
//    if(index == maxNum){
//        return -2;
//    }
//    return index;
//}

int16_t DS_FindAnUnusedGuid(GuidSearchTypdef pGuidSrch[], uint8_t deviceNum)
{
	if(NULL == pGuidSrch || 0 == deviceNum){
		return -1;
	}
	uint8_t index=0;
	for(index=0; index<deviceNum; ++index){
		if(0 == pGuidSrch[index].bitPos){
			break;
		}
	}
	return index;
}

int16_t Ds_RomSearch(GuidSearchTypdef pGuidSrch[], uint8_t deviceNum)
{
    DS_ROM_SRCH_PRINT("-> %s \r\n",__func__);
    if(NULL==pGuidSrch || 0==deviceNum){
        return -1;
    }
    int8_t  bitCnt=0,guidCnt=0;
    uint8_t readByte=0;
    uint8_t devCnt=1,lastDevCnt=0;
    int16_t index=0;

    
    for(guidCnt=0; guidCnt<deviceNum && lastDevCnt<devCnt; ++guidCnt){
        DS_ROM_SRCH_PRINT("%s, guidCnt %u\r\n",__func__,guidCnt);
        
        lastDevCnt=devCnt;
        
        DS_ROM_SRCH_DEV_RST();
        DS_ROM_SRCH_DELAY_US(480);  //  delay in DS18B20_Rst() is not enough
        DS_ROM_SRCH_WRITE_BYTE(0xF0); 
        
        
        for(bitCnt=1; bitCnt<=pGuidSrch[guidCnt].bitPos; ++bitCnt){
			readByte=DS_ROM_SRCH_READ2BITS(); // 2 read
            
            // write bits in pGuidSrch[guidCnt].guid.u64Bits from bit 0 to bit pGuidSrch[guidCnt].pos, that is bit[0, pos)            
            if(pGuidSrch[guidCnt].guid.u64Bits & ((uint64_t)1<<(bitCnt-1) )){
                DS_ROM_SRCH_WRITE_BIT( 1 ); 
                DS_ROM_SRCH_PRINT("%s, %2u: 1x\r\n",__func__,bitCnt);
            }
            else{
                DS_ROM_SRCH_WRITE_BIT( 0 ); 
                DS_ROM_SRCH_PRINT("%s, %2u: 0x\r\n",__func__,bitCnt);
            }
		}
		
        for( bitCnt=pGuidSrch[guidCnt].bitPos+1; bitCnt<=64; ++bitCnt){
            readByte=DS_ROM_SRCH_READ2BITS(); // 2 bits read -> readByte 
            readByte &= 0x03;

            pGuidSrch[guidCnt].bitPos=bitCnt;
            switch(readByte){
            case 0x00:  //conflict bit
                index=DS_FindAnUnusedGuid(pGuidSrch, DS_DEVICE_NUM);
                if(0 > index){
                    return -2;
                }
                pGuidSrch[index]=pGuidSrch[guidCnt];
                pGuidSrch[guidCnt].guid.u64Bits &= ~ ((uint64_t)1<<(bitCnt-1) );
                pGuidSrch[index].guid.u64Bits   |=   ((uint64_t)1<<(bitCnt-1) );

                DS_ROM_SRCH_WRITE_BIT( 0 ); // 1 bits write 0


                ++devCnt;
                DS_ROM_SRCH_PRINT("%s, %2u: 00---index:%d  pos:%d\r\n",__func__,bitCnt,index,pGuidSrch[guidCnt].bitPos);
                DS_ROM_SRCH_PRINT("%s, %2u: 00\r\n",__func__,bitCnt);
                break;	
            case 0x01:  //all are zero
                pGuidSrch[guidCnt].guid.u64Bits &= ~((uint64_t)1<<(bitCnt-1) );

                DS_ROM_SRCH_WRITE_BIT( 0 ); // 1 bits write 0
                
                DS_ROM_SRCH_PRINT("%s, %2u: 01\r\n",__func__,bitCnt);
                break;	
            case 0x02:  //all are one
                pGuidSrch[guidCnt].guid.u64Bits |= ((uint64_t)1<<(bitCnt-1) );

                DS_ROM_SRCH_WRITE_BIT( 1 ); // 1 bits write 1
            
                DS_ROM_SRCH_PRINT("%s, %2u: 10\r\n",__func__,bitCnt);
                break;	
            case 0x03:  //no device
                devCnt=0;
                guidCnt=0;
            
                DS_ROM_SRCH_PRINT("%s, %2u: 11\r\n",__func__,bitCnt);	
                break;

            default:
                DS_ROM_SRCH_PRINT("%s, default, readByte: %#X\r\n",__func__, readByte);
                break;
            }
            
        }
    }
    
    //DS_ROM_SRCH_PRINT("%s, ROM_GUID[0]: %#X \r\n",__func__, pGuidSrch[0].guid.u64Bits);
    
    for(index=0; index<devCnt; ++index){
        DS_ROM_SRCH_PRINT("%s, device %u ROM_GUID: ",__func__,index);
        for(bitCnt=0; bitCnt<8; ++bitCnt){
            DS_ROM_SRCH_PRINT("%02X",pGuidSrch[index].guid.bytes[bitCnt]);
        }
        DS_ROM_SRCH_PRINT("\r\n");
    }
    
    DS_ROM_SRCH_PRINT("%s, guidCnt: %d,  devCnt:%d \r\n",__func__,guidCnt,devCnt);
    
    DS_ROM_SRCH_PRINT("<- %s \r\n",__func__);
    return devCnt;
}



