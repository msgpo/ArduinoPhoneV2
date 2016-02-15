/*
  ArduinoPhoneV2 Source Code

  Author:Loovee

  2016-1-25
  
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
#include <SeeedTouchScreen.h>
#include <SPI.h>
#include <TimerOne.h>
#include <TFTv2.h>          // use TFT v2

#include "phone.h"
#include "UI_ArduinoPhone_dfs.h"
#include "UI_ArduinoPhone.h"


// serial data
char serialDta[100];
unsigned char serialDtaCnt = 0;     // when use data, it shoule be changed to zero(0)
bool serialGot  = false;            // when use data, it shoule be changed to false


int strdo(char *str)
{
    int sum = 0;
    //str_get(str);
    while(*(str)<'A' || *(str)>'Z')str++;

    if(*(str+12)>'0' && *(str+12)<'9')// >10
    {
        sum = (*(str+11)-'0')*10 + (*(str+12)-'0');
    }
    else
    sum = (*(str+11)-'0');

    return sum;
}

//const unsigned char __numAbc[10][6] PROGMEM =
char __numAbc[10][5] =
{
    {' ', '0', ' ', ' ', ' '},
    {',', '.', '!', '1', ' '},
    {'a', 'b', 'c', '2', ' '},
    {'d', 'e', 'f', '3', ' '},
    {'g', 'h', 'i', '4', ' '},
    {'j', 'k', 'l', '5', ' '},
    {'m', 'n', 'o', '6', ' '},
    {'p', 'q', 'r', 's', '7'},
    {'t', 'u', 'v', '8', ' '},
    {'w', 'x', 'y', 'z', '9'},
};

//const unsigned char __numAbcBit[10] PROGMEM =
unsigned char __numAbcBit[10] =
{
    2, 4, 4, 4, 4, 4, 4, 5, 4, 5,
};

// test if call in or msg in , return 0:  return : break;  stateNow, state
bool checkMsgCall(unsigned char stateNow)
{
    if(Phone.isMsg(serialDta))
    {
        UI.state_buf = stateNow;
        UI.state     = ST_GETSMS;

        serialGot    = false;
        serialDtaCnt = 0;
        UI.msgReadCodeMax = strdo(serialDta);
        return true;
    }

    if(Phone.isCall(serialDta))
    {
        serialGot    = false;
        serialDtaCnt = 0;
        UI.state_buf = stateNow;
        UI.state     = ST_GETCALL;
        return true;
    }

    serialGot    = false;
    serialDtaCnt = 0;
    return 0;
}
//state machine

long time1 = 0;
void stateMachine()
{
    long timer_s = millis();
    switch(UI.state)
    {
        //***************************************main page*************************
        case ST_SLEEP:
        
        Serial.println("----------------STATE: SLEEP----------------");
        
        while(1)
        {
            if(0 == digitalRead(2))
            {
                delay(20);
                if(0 == digitalRead(2))
                {
                    UI.state_buf = ST_SLEEP;
                    UI.state = ST_TIME;
                    
                    for(int i=0; i<5; i++)
                    {
                        while(0 == digitalRead(2));
                        delay(5);
                    }
                    break;
                }
            }
            
            if(serialGot)
            {
                disableTimer();
                if(checkMsgCall(ST_TIME)){ enableTimer();break;}
                enableTimer();
            }
        }
        
        Serial.println("light on, WAKE UP");
        blOn();
        break;
        
        case ST_TIME:
        Serial.println("----------------STATE: TIME-----------------");
        Tft.fillScreen();
        UI.getTime();
        UI.drawTimeDate(25, 90);
        UI.showMainPage();
        time1 = millis()-1001;
        
        while(1)
        {
            // get call
            if(blProcess())
            {
                UI.state = ST_SLEEP;
                UI.state_buf = ST_TIME;
                return;
            }
            
            long time = millis();
            
            if(time - time1 > 1000)
            {
                time1 = time;
                UI.getTime();
                if(UI.minute != UI.minute_buf)
                {
                    UI.drawTimeDate(25, 90);
                }
            }
            
            if(serialGot)
            {
                disableTimer();
                if(checkMsgCall(ST_TIME)){ enableTimer();break;}
                enableTimer();
            }

            if(UI.isTouch())// get touch
            {

                if(UI.getTouchRect(12, 200, 112, 270))   // phone
                {
                    UI.state        = ST_CALL;
                    UI.state_buf    = ST_TIME;
                    break;
                }
                else if(UI.getTouchRect(128, 200, 228, 270))  // sms
                {
                    UI.state        = ST_MSG;
                    UI.state_buf    = ST_TIME;
                    break;
                }
            }
            
            if(Serial.available())break;
        }

        break;

        //***************************************phone*****************************
        case ST_CALL:
        Serial.println("----------------STATE: CALL-----------------");
        blClear();
        if(UI.state_buf == ST_MSG)
        {
            UI.drawDialogBox();
            UI.showCall();
        }
        else
        {
            Tft.fillScreen();
            UI.drawDialogBox();
            UI.showPageKB();
            UI.showCall();
        }
    
        while(1)
        {
            if(UI.isTouch())
            {
                while(1)
                {
                    stateCall();

                    if(serialGot)
                    {
                        if(checkMsgCall(ST_CALL))break;
                    }
                    if(UI.state != ST_CALL)break;
                    
                    blProcess();
                }
                
                
            }
            
            if(serialGot)
            {
                disableTimer();
                if(checkMsgCall(ST_CALL))
                {
                    enableTimer();
                    break;
                }
                enableTimer();
            }
            // check if call in or msg
            blProcess();
            if(UI.state != ST_CALL)break;
        }

        UI.callCount = 0;

        break;

        //***************************************make phone calling***************
        case ST_CALLING:
        Serial.println("----------------STATE: CALLING--------------");
        Tft.fillScreen();
        Tft.drawString("calling...", 20, 100, 3, WHITE);
        Tft.fillRectangle(10, 200, 230, 50, GRAY1);
        Tft.drawString("HANDUP", 20, 215, 3, RED);
        while(1)
        {
            if(UI.getTouchRect(10, 200, 240, 250))// hand up
            {
                Phone.handUpCall();
                UI.state     = UI.state_buf;
                UI.state_buf = ST_CALLING;
                break;
            }

            // serial get data
            if(serialGot)
            {
                disableTimer();

                if(Phone.isCallOver(serialDta))
                {
                    UI.state     = UI.state_buf;
                    UI.state_buf = ST_CALLING;
                    enableTimer();
                    break;
                }
                serialGot    = false;
                serialDtaCnt = 0;
                enableTimer();
                delay(1);
            }

        }

        Tft.drawString("calling...", 20, 100, 3, BLACK);
        Tft.fillRectangle(10, 200, 230, 50, BLACK);
        Tft.drawString("HANDUP", 20, 215, 3, BLACK);

        break;
        //***************************************get call*************************
        case ST_GETCALL:
        Serial.println("----------------STATE: GETCALL--------------");
        Tft.fillScreen();
        Tft.drawString("calling...", 20, 100, 2, WHITE);
        Tft.fillRectangle(10, 200, 100, 50, GRAY1);
        Tft.fillRectangle(130, 200, 100, 50, GRAY1);
        Tft.drawString("ACCEPT", 15, 215, 2, GREEN);
        Tft.drawString("HANDUP", 135, 215, 2, RED);
        while(1)
        {
            // serial get data
            if(serialGot)
            {
                disableTimer();
                if(Phone.isCallOver(serialDta))
                {
                    UI.state     = UI.state_buf;
                    UI.state_buf = ST_GETCALL;
                    enableTimer();
                    break;
                }
                serialGot    = false;
                serialDtaCnt = 0;
                enableTimer();
            }

            if(UI.getTouchRect(10, 200, 110, 250))// accept
            {
                Phone.acceptCall();
                Tft.drawString("calling...", 20, 100, 2, BLACK);
                Tft.drawString("call connected!", 20, 100, 2, WHITE);
                Tft.drawString("call connected!", 20, 100, 2, BLACK);
            }

            // hand up
            if(UI.getTouchRect(130, 200, 230, 250)) // hand up
            {
                UI.state     = UI.state_buf;
                UI.state_buf = ST_GETCALL;
                Phone.handUpCall();
                break;
            }

        }

        Tft.fillRectangle(10, 200, 100, 50, BLACK);
        Tft.fillRectangle(130, 200, 100, 50, BLACK);

        break;
        //***************************************message***************************
        case ST_MSG:

        Serial.println("----------------STATE: MSG------------------");
        if(UI.state_buf == ST_CALL)
        {
            UI.drawMsgDialogBox();
            UI.showMsg();
        }
        else
        {
            Tft.fillScreen();
            UI.showPageKB();
            UI.drawMsgDialogBox();
            UI.showMsg();
        }

        while(1)
        {
            if(UI.isTouch())
            {
                while(1)
                {
                    stateMsg();
                    if(UI.state != ST_MSG)break;
                    if(serialGot)
                    {
                        disableTimer();
                        if(checkMsgCall(ST_MSG)){enableTimer();break;}
                        enableTimer();
                    }
                }
            }
            if(serialGot)
            {
                disableTimer();
                if(checkMsgCall(ST_MSG)){enableTimer();break;}
                enableTimer();
            }
            if(UI.state != ST_MSG)break;
        }

        UI.msgCount     = 0;
        UI.msgNumCount  = 0;


        break;

        //***************************************message sending*******************
        case ST_SENDSMS:
        Serial.println("----------------STATE: SENDSMS--------------");
        Tft.fillScreen();
        Tft.drawString("SMS Sending", 20, 120, 3, WHITE);
        while(1)
        {
            // delay(1000);
            if(Phone.isMsgSendOver())
            {
                UI.state_buf = ST_SENDSMS;
                UI.state     = ST_MSG;
                break;
            }
        }
        Tft.drawString("SMS Sending", 20, 120, 3, BLACK);

        break;
        //**************************************get msg******************************
        case ST_GETSMS:
        Serial.println("----------------STATE: GETSMS---------------");
        UI.drawReadMsgPage();
        // get new msg

        Serial.print("AT+CMGR=");Serial.println(UI.msgReadCodeMax, DEC);
        UI.msgReadCodeNow = UI.msgReadCodeMax;

        while(1)
        {
            stateMsgRead();
            if(UI.state != ST_GETSMS)
            break;
        }

        break;

        default:
        ;
    }
}

// state: phone call


void stateCall()
{
    unsigned char button = 0;
    long timercall = millis();
    UI.getTouch(&button);
    long t = millis();
    
    if(TOUCH_NOTHING != button)
    {
        blClear();
    }
    
    if(button>0 && button<10)
    {
        UI.callAdd(button);
    }
    else if(button == 11)
    {
        long deltT = t - timercall;

        if((UI.callCount < 1) && (deltT>400))
        {
            UI.callAdd(15);
        }
        else UI.callAdd(0);
    }
    else if(button == 12)
    {
        UI.callDel();
    }
    else if(button == 10)           // call
    {
        Phone.makeCall();
        UI.callCount = 0;
        UI.state_buf = ST_CALL;
        UI.state     = ST_CALLING;
    }
    else if(button == TOUCH_DRAG_LEFT)
    {
        UI.state_buf = ST_CALL;
        UI.state     = ST_MSG;
    }
    else if(button == TOUCH_DRAG_RIGHT)
    {
        UI.state_buf = ST_CALL;
        UI.state     = ST_TIME;
    }
}

// state: message
void stateMsg()
{
    long time =  millis();
    long time1 = time;
    unsigned char buttonNum     = 100;

    int count = 0;
    unsigned char button = 0;

    if(!UI.isTouch())return;

    unsigned char msgState = 1;
            
    while(1)
    {
        time1 = millis();
        msgState = UI.getMsgInputState();

        
        if(msgState == 1)
        {
            UI.state_msg = MSG_STATE_NUM;
            delay(100);
            button = 0;
        }
        else if(msgState == 2)
        {
            UI.state_msg = MSG_STATE_TXT;
            delay(100);
            button = 0;
        }
        else
        {
            button = UI.getTouch(&button);
        }
        
        // input txt
        
        if(button == TOUCH_DRAG_LEFT)
        {
            UI.state_buf = ST_MSG;
            UI.state     = ST_GETSMS;
            break;
        }
        else if(button == TOUCH_DRAG_RIGHT)
        {
            UI.state_buf = ST_MSG;
            UI.state     = ST_CALL;
            break;
        }
        else if(button>0 && button<10)
        {
            if(UI.state_msg == MSG_STATE_TXT)
            {
                buttonNum    = button;
                UI.msgAdd(__numAbc[buttonNum][count % __numAbcBit[buttonNum]], RED);
                count++;
                time = time1;
            }
            else
            {
                buttonNum = button;
                UI.msgNumAdd(buttonNum);
                buttonNum = 100;
            }
        }
        else if(button == 11)//0
        {
            if(UI.state_msg == MSG_STATE_TXT)
            {
                buttonNum = 10;
                UI.msgAdd(__numAbc[0][count % __numAbcBit[0]], RED);
                count++;
                time = time1;
            }
            else
            {
                long t = millis();
                long deltT = t - time1;

                if(deltT > 500 && UI.msgNumCount == 0)
                UI.msgNumAdd(15);
                else
                UI.msgNumAdd(0);
                buttonNum = 100;
            }
        }
        else if(button == 12) // del
        {
            if(UI.state_msg == MSG_STATE_TXT)
            UI.msgDel();
            else
            UI.msgNumDel();
            break;
        }
        else if(button == 10)       // send
        {
            Phone.msgSend();
            UI.state_buf  =  ST_MSG;
            UI.state      =  ST_SENDSMS;
            break;
        }

        if(((time1 - time) > 600)&& UI.state_msg == MSG_STATE_TXT)
        {
            if(buttonNum>0 && buttonNum<10)UI.msgAdd(__numAbc[buttonNum][(count-1) % __numAbcBit[buttonNum]]);
            else if(buttonNum == 10)UI.msgAdd(__numAbc[0][(count-1) % 2]);
            break;
        }
    }
}

// state read msg
void stateMsgRead()
{
    unsigned char button  = 0;
    unsigned char button1 = 0;
    unsigned char cnt = 0;
    unsigned flag_getMsgTime = 0;

    while(1)
    {
        if(serialGot)
        {
            disableTimer();

            if(flag_getMsgTime == 1)// get msg
            {
                UI.getReadMsgMsg(serialDta);
                UI.drawReadMsg();
                flag_getMsgTime = 0;
            }
            else if(UI.getReadMsgTimeAndFrom(serialDta, UI.msgReadNum, UI.msgReadTime))
            {
                flag_getMsgTime = 1;
            }

            serialGot = false;
            serialDtaCnt = 0;
            enableTimer();
        }

        if(UI.isTouch())
        {
            if(UI.getTouchRect(0, 270, 239, 319))
            {
                //UI.state     = UI.state_buf;
                UI.state     = UI.state_buf;
                UI.state_buf = ST_GETSMS;
                button1 = 0;
                break;
            }
            else button1 = UI.getTouch(&button);
        }
        else button1 = 0;
        //drag right: read format
        if(button1 == TOUCH_DRAG_RIGHT)
        {
            button1 = 0;
            cnt = 0;
            if(UI.msgReadCodeNow>2)
            UI.msgReadCodeNow--;
            Serial.print("AT+CMGR=");Serial.println(UI.msgReadCodeNow, DEC);
        }
        //drag left: read <-- msg
        else if(button1 == TOUCH_DRAG_LEFT)
        {
            cnt = 0;
            button1 = 0;
            //if(UI.msgReadCodeNow<UI.msgReadCodeMax && UI.msgReadCodeMax != 1)
            UI.msgReadCodeNow++;
            Serial.print("AT+CMGR=");Serial.println(UI.msgReadCodeNow, DEC);
        }
    }
}
// setup
void enableTimer()
{
    Timer1.attachInterrupt(__serialEvent);
    //Timer1.start();
}

void disableTimer()
{
    Timer1.detachInterrupt();
    //Timer1.stop();
}

// back light control
bool flgBlStatus        = 0;
unsigned long timerBlOn = 0;

void blOn()
{
    flgBlStatus = 1;
    digitalWrite(7, HIGH);
    timerBlOn = millis();
}

void blOff()
{
    flgBlStatus = 0;
    digitalWrite(7, LOW); 
}

void blClear()
{
    Serial.println("light cnt clear");
    timerBlOn = millis();
}

// background light
bool blProcess()
{
    if(flgBlStatus && (millis()-timerBlOn > SLEEPTIME)) // on and over 15s
    {
        UI.state = ST_SLEEP;
        Serial.println("light off, SLEEP");
        blOff();
        return 1;
    }
    return 0;
}

//setup
void setup()
{
    pinMode(7, OUTPUT);
    blOn();
    pinMode(2, INPUT);
    digitalWrite(2, HIGH);
    
    Phone.init();
    UI.init();
    Timer1.initialize(1000000);                 // 1s
    Timer1.attachInterrupt(__serialEvent);      // attach the service routine here
}

// loop
void loop()
{
    stateMachine();
    blProcess();
    // while(Serial.available())Serial1.write(Serial.read());
    // while(Serial1.available())Serial.write(Serial1.read());
}

// serial event
void __serialEvent()
{
    disableTimer();         // disable timer irq
    
    while (Serial1.available())
    {
        char inChar = (char)Serial1.read();
        Serial.write(inChar);
        serialDta[serialDtaCnt++]= inChar;
        if (inChar == '\n')
        {
            serialGot = true;
            serialDta[serialDtaCnt] = '\0';
            break;
        }
    }
    enableTimer();          // enable timer irq
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
