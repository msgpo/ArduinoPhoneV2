/*
  phone.cpp

  Author:Loovee

  2013-9-10

  The MIT License (MIT)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#include <TimerOne.h>
#include <Arduino.h>
#include "UI_ArduinoPhone.h"
#include "phone.h"

#define SIM800_POWER_PIN        9
#define SIM800_POWER_STATUS     3

int sendATTest()
{

    Serial1.println("AT");
    unsigned long timer_s = millis();
    while(1)
    {
        if(millis()-timer_s > 50)return -1;
        if(Serial1.available())
        {
            if('K' == Serial1.read())   // get ok
            {
                while(Serial1.available())Serial1.read();
                return 0;
            }
        }
    }
}

void phone::init()
{
    Serial.begin(9600);
    Serial1.begin(9600);
    
    Serial.println("begin to start...");
    
    pinMode(SIM800_POWER_STATUS, INPUT);
    delay(10);
    if(LOW == digitalRead(SIM800_POWER_STATUS))
    {
        if(sendATTest() != 0)
        {
            delay(800);
            digitalWrite(SIM800_POWER_PIN,HIGH);
            delay(200);
            digitalWrite(SIM800_POWER_PIN,LOW);
            delay(2000);
            digitalWrite(SIM800_POWER_PIN,HIGH);
            delay(3000);  
        }
        while(sendATTest() != 0);                
        //Serial.println("Init O.K!");         
    }
    else
    {
        Serial.println("Power check failed!");  
    }
    
    Serial.println("init ok");
}

void phone::msgSend()
{

    Serial1.print("AT+CMGF=1\r");
    delay(100);
    // Serial.println("AT + CMGS = \"+8613425171053\"");
    Serial1.print("AT+CMGS=\"");
    Serial1.print(UI.msgNum);
    Serial1.println("\"");

    delay(100);
    Serial1.println(UI.msg);
    Serial1.println((char)26);
    Serial1.println();
}

void phone::makeCall()
{
    Serial1.print("ATD");
    Serial1.println(UI.callNum);
    Serial1.println();
}

bool phone::isMsg(char *str)       // check if get some message
{
    if(strCmp(8, str, "CMTI: \"SM\","))return 1;
    else return 0;
}

bool phone::isMsgSendOver()
{
    // add code here
    delay(1000);
    return 1;
}

bool phone::strCmp(unsigned char n, char *p1, const char *p2)
{
    char *pt = p1;
    while(*(pt)<'A' || *(pt)>'Z')
    {
        pt++;
    }

    for(int i = 0; i<n; i++)
    {
        if(*(pt+i) == *(p2+i))continue;
        else return 0;
    }
    return 1;
}

bool phone::isCall(char *str)      // check if some one call
{
    if(strCmp(4, str, "RING"))return 1;
    return 0;
}

bool phone::isCallOver(char *str)    // check if call over
{
    // add code here
    if(strCmp(5, str, "NO CARRIER"))return 1;
    return 0;
}

// hand up a call
void phone::handUpCall()
{
    Serial1.println("ATH");
}

void phone::acceptCall()
{
    Serial1.println("ATA");
}


phone Phone;