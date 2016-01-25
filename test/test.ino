
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

void setup()
{
    
    Serial.begin(9600);
    Serial1.begin(9600);
    
    while(!Serial.available());
    while(Serial.available())Serial.read();
    
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

void loop()
{
    while(Serial.available())Serial1.write(Serial.read());
    while(Serial1.available())Serial.write(Serial1.read());
}