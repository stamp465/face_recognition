#include <usbdrv.h>
#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
Servo myservo;
#define pass_length 10

#define RQ_SET_LED 0
#define RQ_SET_START 1
#define RQ_GET_PW 2
#define RQ_RESULT 3
#define RQ_GET_START_STATUS 4

#define button_sw !(digitalRead(PIN_PB0)&&digitalRead(PIN_PB1)&&digitalRead(PIN_PB2)&&digitalRead(PIN_PB3)&&digitalRead(PIN_PB4)&&digitalRead(PIN_PB5))

uint8_t password[pass_length];
uint8_t password_notsubmit[pass_length];
int size_now = 0;
int set_start = 0;
int old_result = 0;
int state = -1;
bool submit = false;
bool door = false;
bool waiting = false;
static uint8_t pw = 0;
static uint8_t n_pw = 0;

unsigned long savetime;


void print_start(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ENTER YOUR FACE");
}

void print_correct(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Correct");
}

void print_door(bool status){
  lcd.clear();
  lcd.setCursor(0,1);
  if(status){
    lcd.print("Open Door");
    /*
    servo ....
    */
  }
  else if(!status){
    lcd.print("Close Door");
    /*
    servo ....
    */
  }
    
}

void print_password(){
    lcd.clear();
    lcd.setCursor(0,0);
    if(waiting)     lcd.print("Waiting Result");
    else{
        if(!submit){
            if(old_result==1)   lcd.print("Wrong, ");
            lcd.print("Password :");
        }
        else    lcd.print(" Password Sent -->");
    }
    
    int i;
    for(i=0;i<size_now;i++){
        lcd.setCursor(i,1);
        if(password[i]!=0){
            lcd.print(password[i]);
        }
    }
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
    usbRequest_t *rq = (usbRequest_t *)data;
    static uint8_t switch_state[6];

    if (rq->bRequest == RQ_SET_LED){
        uint8_t led_val = rq->wValue.bytes[0];
        uint8_t led_no = rq->wIndex.bytes[0];
        //if (led_no == 0)    digitalWrite(PIN_PC2, led_val);
        return 0; 
    }
    else if (rq->bRequest == RQ_SET_START){
        uint8_t if_face = rq->wValue.bytes[0];
        if (if_face == 1 && set_start == 0 ){                // when see face and before status is close door
            set_start = 1; 
            state = 1;
        }
        /*else if (if_face == 0 && set_start != 2){            // when not see face and status is not open door
            set_start = 0; 
            state = 0;
        }*/
        return 0; 
    }
    else if (rq->bRequest == RQ_GET_PW){

        // check all is zero ?
        int i;
        bool allnotZERO = false;
        for(i=0;i<pass_length;i++)  if(password[i]!=0)  allnotZERO = true;

        if(submit && !waiting && allnotZERO){       // when submit and not waiting for result and all is not zero
            usbMsgPtr = (uint8_t*) &password;
            waiting = true;
            print_password();
        }
        else if(submit && !allnotZERO){             // when submit and all is zero --- ignore waiting
            submit = false;
            usbMsgPtr = (uint8_t*) &password_notsubmit;
            print_password();
        }
        else{                                       // when not submit ( python call all time )
            usbMsgPtr = (uint8_t*) &password_notsubmit;
        }
        return sizeof(password);
    }
    else if (rq->bRequest == RQ_RESULT){
        if(submit){
            submit = false;
            waiting = false;
            uint8_t open_door = rq->wValue.bytes[0];
            if(open_door==0){               // wrong password
                door = false;
                old_result = 1;
                print_password();
            }
            else if(open_door==1){          // correct password
                door = true;
                set_start = 2;
                state = 2;
                old_result = 0;
            }
        }
        return 0;
    }
    else if (rq->bRequest == RQ_GET_START_STATUS){
        usbMsgPtr = (uint8_t*) &set_start;
        return 1;
    }
    return 0;
}

void reset_password(){
    int i;
    for(i=0;i<pass_length;i++){
      password_notsubmit[i]=0;
      password[i]=0;
    }
    size_now = 0;
}

void setup()
{

    pinMode(PIN_PC0,OUTPUT);
    pinMode(PIN_PC1,OUTPUT);
    pinMode(PIN_PC2,OUTPUT);

    pinMode(PIN_PB0,INPUT_PULLUP);
    pinMode(PIN_PB1,INPUT_PULLUP);
    pinMode(PIN_PB2,INPUT_PULLUP);
    pinMode(PIN_PB3,INPUT_PULLUP);
    pinMode(PIN_PB4,INPUT_PULLUP);
    pinMode(PIN_PB5,INPUT_PULLUP);
    myservo.attach(PIN_PD6);
    //myservo.write(0);

    lcd.init();  //initialize the lcd
    lcd.backlight();  //open the backlight 
    print_start();
    delay(1000);

    reset_password();

    usbInit();

    /* enforce re-enumeration of USB devices */
    usbDeviceDisconnect();
    delay(300);
    usbDeviceConnect();

    

}
void loop()
{
    usbPoll();

    if(set_start==1){
        if(state==1){
            print_password();
            state = 0;
        }
        usbPoll();
        if(button_sw){
            if(!digitalRead(PIN_PB0) && size_now < pass_length ){           //if 3 pressed and password not full of screen
                password[size_now] = 3;
                size_now ++;
                while(!digitalRead(PIN_PB0))
                  usbPoll();
            }
            else if(!digitalRead(PIN_PB1) && size_now < pass_length ){      //if 4 pressed and password not full of screen
                password[size_now] = 5;
                size_now ++;
                while(!digitalRead(PIN_PB1))
                  usbPoll();
            }
            else if(!digitalRead(PIN_PB2) && size_now < pass_length ){      //if 4 pressed and password not full of screen
                password[size_now] = 7;
                size_now ++;
                while(!digitalRead(PIN_PB2))
                  usbPoll();
            }
            else if(!digitalRead(PIN_PB3) && size_now < pass_length ){      //if 4 pressed and password not full of screen
                password[size_now] = 9;
                size_now ++;
                while(!digitalRead(PIN_PB3))
                  usbPoll();
            }
            
            else if(!digitalRead(PIN_PB5) && size_now > 0){                 //backspace if have password in screen
                password[size_now-1] = 0;
                size_now --;
                while(!digitalRead(PIN_PB5))
                  usbPoll();
            }
            else if(!digitalRead(PIN_PB4) ){                                //if ENTER
                submit=true;
                while(!digitalRead(PIN_PB4))
                  usbPoll();
            }
            print_password();
        }

        

    }
    else if(set_start==2){
        if(state == 2){
            print_correct();
            savetime = millis();
            state = 22;
        }
        else if(state==22 && millis() - savetime > 1000){
            print_door(door);
            state = 222;
        }
        else if(state==222 && millis() - savetime > 8000){
            door = false;
            print_door(door);
            state = -1;
        }
        else if( millis() - savetime > 11000){
            state = 0;
            set_start = 0;
            reset_password();
        }
    }
    else{
        
        if(state == 0){
            print_start();
            reset_password();
            state = -1;
        }
        
    }

}
