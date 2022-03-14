#include <usbdrv.h>
#include <Wire.h>
#include <Servo.h>
Servo myservo;
#define RQ_SET_LED 0
#define RQ_SET_START 1
#define RQ_GET_PW 2
#define RQ_RESULT 4
#define RQ_SET_SERVO 3

#define button_sw !(digitalRead(PIN_PB0)&&digitalRead(PIN_PB1)&&digitalRead(PIN_PB2)&&digitalRead(PIN_PB3))

uint8_t password[8];
uint8_t password_notsubmit[8];
int pass_length = 4;
int noww = 0;
bool set_start = false;
bool submit = false;
bool door = false;

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
    usbRequest_t *rq = (usbRequest_t *)data;
    static uint8_t switch_state[6];

    if (rq->bRequest == RQ_SET_LED)
    {
        uint8_t led_val = rq->wValue.bytes[0];
        uint8_t led_no = rq->wIndex.bytes[0];

        if (led_no == 0)
            digitalWrite(PIN_PC2, led_val);
        
        return 0; // return no data back to host
    }
    else if (rq->bRequest == RQ_SET_START){
        uint8_t set_st = rq->wValue.bytes[0];
        if (set_st == 1){
            set_start = true; 
        }
        else if (set_st == 0){
            set_start = false; 
        }
        return 0; // return no data back to host
    }
    else if (rq->bRequest == RQ_RESULT){
        if(submit){
            uint8_t open_door = rq->wValue.bytes[0];
            if(open_door==0){               // wrong password
                submit = false;
                door = false;
            }
            else if(open_door==1){          // correct password
                submit = false;
                door = false;
            }
        }
        return 0;
    }
    else if (rq->bRequest == RQ_GET_PW){
        if(submit){
            usbMsgPtr = (uint8_t*) &password;
            //noww = 0;
        }
        else{
            usbMsgPtr = (uint8_t*) &password_notsubmit;
        }
        return sizeof(password);
    }
    else if (rq->bRequest == RQ_SET_SERVO){
        uint8_t degree = rq->wValue.bytes[0];
        myservo.write(degree);
        //delay(1000); 

    }
    return 0;
}

void setup()
{
    pinMode(PIN_PC2,OUTPUT);
    pinMode(PIN_PB0,INPUT_PULLUP);
    pinMode(PIN_PB1,INPUT_PULLUP);
    pinMode(PIN_PB2,INPUT_PULLUP);
    pinMode(PIN_PB3,INPUT_PULLUP);
    myservo.attach(PIN_PB0);
    myservo.write(0);
    delay(1000);

    int i;
    for(i=0;i<pass_length;i++){
      password_notsubmit[i]=0;
      password[i]=0;
    }

    usbInit();

    /* enforce re-enumeration of USB devices */
    usbDeviceDisconnect();
    delay(300);
    usbDeviceConnect();
}
void loop()
{
    usbPoll();
    /*myservo.write(0);
    delay(1000);
    myservo.write(90);  // สั่งให้ Servo หมุนไปองศาที่ 90
    delay(1000);        // หน่วงเวลา 1000ms
    myservo.write(180); // สั่งให้ Servo หมุนไปองศาที่ 180
    delay(1000);        // หน่วงเวลา 1000ms
    */

    if(set_start && !submit){
        usbPoll();
        if(button_sw){
            //digitalWrite(PIN_PD3, HIGH); //remove this line when lcd prompt
            //delay(5); //remove this line when lcd prompt
            if(!digitalRead(PIN_PB0) && noww<=pass_length){ //if A pressed and password not full
                password[noww]=1;
                noww++;
                while(!digitalRead(PIN_PB0));
            }
            else if(!digitalRead(PIN_PB1) && noww<=pass_length){ //if B pressed and password not full
                password[noww]=2;
                noww++;
                while(!digitalRead(PIN_PB1));
            }
            else if(!digitalRead(PIN_PB2) && noww>0){ //if BACK pressed and password not blank
                noww--;
                while(!digitalRead(PIN_PB2));
            }
            else if(!digitalRead(PIN_PB3) ){ //if ENTER pressed and password is full
                submit=true;
                while(!digitalRead(PIN_PB3));
            }
        }
    }
}
