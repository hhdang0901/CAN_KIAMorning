#include <mcp_can.h>
#include <mcp_can_dfs.h>

// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13

#define PID_ESS             0x0C
#define PID_VSS             0x0D
#define PID_ECT             0x05
#define PID_MONITOR_STATUS  0x01
#define PID_CONTROL_MODULE_VOLTAGE 0x42
#define PID_MAP             0x0B
#define PID_IAT             0x0F
#define PID_TPS             0x11
#define PID_LOAD            0x04
#define PID_MAF             0x10

#define CAN_ID_PID          0x7DF

//------
#define ANALOG_INPUT A0
//------

#include <SPI.h>
#include "mcp_can.h"
#include "Arduino.h"
#include "string.h"


/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;
int32_t value;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

unsigned char stmp[8] = {0x02, 0x01, 0, 0, 0, 0, 0, 0};
int ESS, ECT, VSS, MAP, MAF, IAT, TPS, LOAD;
int MIL_Status, CHARGE_Status;
void set_mask_filt() 
{
    CAN.init_Mask(0, 0, 0x7FF);

    CAN.init_Filt(0, 0, 0x7E8);
    CAN.init_Filt(1, 0, 0x7E9);
    CAN.init_Filt(2, 0, 0x7EA);
}

void setup() 
{
    SERIAL.begin(115200);

    stmp[5] = 0;  
    stmp[6] = 0;  
    stmp[7] = 0;

    while (CAN_OK != CAN.begin(CAN_1000KBPS)) {            // init can bus : baudrate = 1000k
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println(" Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");
    set_mask_filt();
}

void printData (int ESS, int ECT, int VSS, int MIL_Status, int CHARGE_Status,int MAF, int IAT, int TPS, int LOAD)
{
  SERIAL.print("a");
  SERIAL.print(ESS);
  SERIAL.print("b");
  SERIAL.print(ECT);
  SERIAL.print("c");
  SERIAL.print(VSS);
  SERIAL.print("d");
  SERIAL.print(MIL_Status);
  SERIAL.print("e");
  SERIAL.print(CHARGE_Status);
  SERIAL.print("f");
  SERIAL.print(MAP);
  SERIAL.print("g");
  SERIAL.print(IAT);
  SERIAL.print("h");
  SERIAL.print(TPS);
  SERIAL.print("i");
  SERIAL.print(LOAD);
  SERIAL.print("j");
  SERIAL.println();
}


void Read_Print (void)
{
  unsigned char len = 0;    
  unsigned char buf[8];
  if (CAN_MSGAVAIL == CAN.checkReceive()) 
    {
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
    unsigned long canId = CAN.getCanId();
    if (canId == 0x7E8 || canId == 0x7E9 || canId == 0x7EA)
      {       
      if (buf[1] == 0x41)
        {
          if (buf[2] == PID_ECT)
                  {      
                    ECT = buf[3]-40;
                  }
          else if (buf[2] == PID_VSS)
                  {
                    VSS = buf[3];
                  }       
          else if (buf[2] == PID_MONITOR_STATUS)
                  {
                    MIL_Status = (((buf[3] & 0x80)>>7)!=0);
                  }
          else if (buf[2] == PID_CONTROL_MODULE_VOLTAGE)
                  {
                    CHARGE_Status = ((0.256*buf[3]+0.001*buf[4])<13);
                  }
          else if (buf[2] == PID_MAP)
                  {
                    MAP = buf[3];
                  }
          else if (buf[2] == PID_IAT)
                  {
                    IAT = buf[3] - 40;
                  }
          else if (buf[2] == PID_TPS)
                  {
                    TPS = (buf[3]*100)/255;
                  }
          else if (buf[2] == PID_LOAD)
                  {
                    LOAD = (buf[3]*100)/255;
                  }
          else if (buf[2] == PID_ESS)
                  {
                  ESS = (buf[3]*256+buf[4])/4;
                  }                                      
        }
      }
    }
  printData(ESS,ECT,VSS,MIL_Status,CHARGE_Status, MAF, IAT, TPS, LOAD);
  delay(10);
}

void loop() 
{   
    stmp[2] = PID_ESS; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp); Read_Print();
    stmp[2] = PID_MAP; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp);Read_Print();
    stmp[2] = PID_IAT; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp);Read_Print();
    stmp[2] = PID_ECT; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp);Read_Print();
    stmp[2] = PID_MAP; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp);Read_Print();
    stmp[2] = PID_IAT; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp);Read_Print();    
    stmp[2] = PID_TPS; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp);Read_Print();
    stmp[2] = PID_ESS; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp); Read_Print();
    stmp[2] = PID_LOAD; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp);Read_Print();
    stmp[2] = PID_VSS; 
    CAN.sendMsgBuf(0x7DF, 0, 8, stmp);Read_Print();        
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
