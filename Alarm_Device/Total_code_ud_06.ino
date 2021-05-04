

#include <NTPClient.h>
#include<ESP8266WiFi.h>
#include<WiFiUdp.h>


WiFiUDP u;
NTPClient n(u, "2.asia.pool.ntp.org", 7*3600);



#define button    D1         // D1 is gpio 5
#define triac     D2          // D2 is gpio 4
//#define testLed   D4


short int hours, mins;
volatile boolean u1StateButton, u1Mode, u1StateLed;
volatile short int count;



void setup() 
{

      /* Monitor */
//      Serial.begin(9600);
  
      /* Set I/O */
      pinMode(button, INPUT);
      pinMode(triac, OUTPUT);
//      pinMode(testLed, OUTPUT);

      /* INITIAL STATE */
      u1StateButton = false;
      u1Mode = false;
      u1StateLed = false;
      count = 0;
      
      /* setup Wifi */
      WiFi.disconnect();
      WiFi.mode(WIFI_STA);
      WiFi.begin("D@ITIN", "khongco@12345");
      while(WiFi.status() != WL_CONNECTED)
      {
        
        digitalWrite(triac, HIGH);
        delay(1000);
//        Serial.print("connecting...");
//        Serial.println("................");
      }
      digitalWrite( triac , LOW );
     
     /* get time */
      n.begin();


      /* Interrupts */
      noInterrupts();                                // Switch off interrupts whilst they are set up
      timer0_isr_init();                             // Initialise Timer0
      timer0_attachInterrupt(ISR);                   // Goto the ISR function below when an interrupt occurs
      timer0_write(ESP.getCycleCount() + 80000000L); // Pre-load timer with a time value (1-second)
      attachInterrupt(digitalPinToInterrupt(button),ISR_button, CHANGE);
      interrupts();                                  // Switch interrupts back on
  
}



void loop() 
{

    getTimes();
    timeLed();
    controlLed();
//    printSerial();
//    delay(500);

}

  /***********************interrupts ********************************/
ICACHE_RAM_ATTR void ISR (void) 
{                                // Timer reached zero, now re-load it to repeat
  timer0_write(ESP.getCycleCount() + 80000000L); // Reset the timer, do this first for timing accuracy
  count++;     
  
}


ICACHE_RAM_ATTR void ISR_button()
{
  buttonLed();
}






/*********************** subroubtine *****************************/
/********************* control led with button ******************/
void buttonLed()
{
  if(   digitalRead(button) != u1StateButton  )
      {
        delay(200);
        if(   digitalRead(button) != u1StateButton  )
        {
          u1Mode = true;
          u1StateLed = !u1StateLed;
          u1StateButton = digitalRead(button);
        }
        delay(69);
      }
}
/************************ control led with time **************/
void timeLed()
{

      /* use the ato time */
      if (    (  (17 == hours) && ( 35<= mins) &&( 40 >= mins)  ) || ( (20 == hours) && ( 00 <= mins) &&( 3 >= mins)) )
        {
          
          u1Mode = false;
          
        }

        /* SET MODE LED WITH TIME */
      if(   false ==  u1Mode  )
      {
        
           if( (  hours == 17 && mins>= 36 ) || (hours==18) || (hours == 19)  )   
            {
                u1StateLed = true;        
            }
            else if( hours == 20)
            {
              u1StateLed = false;
            }
            
            else
            {
               u1StateLed = false;
            }
            
       }
  
}

/******************* control led ************************/
void controlLed()
{
        /* ON OFF led */
          if ( true == u1StateLed  )
          {
            /* ON LED */
            digitalWrite(triac, HIGH);
//            digitalWrite(D4, LOW);  
          }
        
          else
          {
            digitalWrite(triac, LOW);
//            digitalWrite(D4, HIGH); 
          }
}


/* get time*/
void getTimes()
{

    // get time
    if(count >= 20)
    {
      count = 0;
      for( int i = 0; i < 10; i++)
      {
            n.update();
            hours= n.getHours();
            mins = n.getMinutes();
      }
     }  
  
}

/* serial */
//void printSerial()
//{
//  Serial.print("time:");
//  Serial.println(hours);
//  Serial.print(':');
//  Serial.println(mins);
//  Serial.print("button:");
//  Serial.println(digitalRead(button));
//  Serial.print("state button:");
//  Serial.println(u1StateButton);
//  Serial.print("mode:");
//  Serial.println(u1Mode);
//  Serial.print("led:");
//  Serial.println(u1StateLed);
//  Serial.println("--------------------");
//}
