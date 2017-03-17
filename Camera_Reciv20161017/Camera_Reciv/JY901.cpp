#include "stdafx.h"
#include "JY901.h"
#include "string.h"
CJY901 ::CJY901 ()
{
}

void CJY901 ::CopeSerialData(unsigned char ucData[],unsigned short usLength)
{
	static unsigned char chrTemp[2000];
	static unsigned char ucRxCnt = 0;	
	static unsigned short usRxLength = 0;


    memcpy(chrTemp,ucData,usLength);
	usRxLength += usLength;
    while (usRxLength >= 11)
    {
        if (chrTemp[0] != 0x55)
        {
			usRxLength--;
			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);                        
            continue;
        }
		switch(chrTemp[1])
		{
			case 0x50:	memcpy(&stcTime,&chrTemp[2],8);break;
			case 0x51:	memcpy(&stcAcc,&chrTemp[2],8);break;
			case 0x52:	memcpy(&stcGyro,&chrTemp[2],8);break;
			case 0x53:	memcpy(&stcAngle,&chrTemp[2],8);break;
			case 0x54:	memcpy(&stcMag,&chrTemp[2],8);break;
			case 0x55:	memcpy(&stcDStatus,&chrTemp[2],8);break;
			case 0x56:	memcpy(&stcPress,&chrTemp[2],8);break;
			case 0x57:	memcpy(&stcLonLat,&chrTemp[2],8);break;
			case 0x58:	memcpy(&stcGPSV,&chrTemp[2],8);break;
		}
		usRxLength -= 11;
		memcpy(&chrTemp[0],&chrTemp[11],usRxLength);                     
    }
}
CJY901 JY901 = CJY901();
