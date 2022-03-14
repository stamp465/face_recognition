#include <usbdrv.h>
#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
Servo myservo;
#define pass_length 4
#define RQ_SET_LED 0
#define RQ_SET_START 1
#define RQ_GET_PW 2
#define RQ_RESULT 3

#define button_sw !(digitalRead(PIN_PB0)&&digitalRead(PIN_PB1)&&digitalRead(PIN_PB2)&&digitalRead(PIN_PB3))

uint8_t password[pass_length];
uint8_t password_notsubmit[pass_length];
int noww = 0;
int set_start = 0;
bool submit = false;
bool door = false;
bool waiting = false;
static uint8_t pw = 0;
static uint8_t n_pw = 0;
int old_result = 0;
int state = -1;
unsigned long savetime;


void delay_eiei(int period){
    int last_time = millis();
    while( millis() - last_time > period) {
                
    }
}

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
  }
  else if(!status){
    lcd.print("Close Door");
  }
    
}

void print_password(){
    lcd.clear();
    lcd.setCursor(0,0);
    if(waiting){
        lcd.print("Waiting Result");
    }
    else{
        if(!submit){
            if(old_result==1){
                lcd.print("Wrong, ");
            }
            lcd.print("Password :");
        }
        else{
            lcd.print(" Password Sent -->");
        }
    }
    

    
    int i;
    for(i=0;i<noww;i++){
        lcd.setCursor(i+1,1);
        if(password[i]!=0){
            lcd.print(password[i]);
        }
    }
}

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
            set_start = 1; 
            state = 1;
        }
        else if (set_st == 0 && set_start != 2){
            set_start = 0; 
            state = 0;
        }
        return 0; // return no data back to host
    }
    else if (rq->bRequest == RQ_GET_PW){
        int i;
        bool allnotZERO = false;
        for(i=0;i<pass_length;i++){
            if(password[i]!=0){
                allnotZERO = true;
            }
        }
        if(submit && !waiting && allnotZERO){
            usbMsgPtr = (uint8_t*) &password;
            //submit = false;
            waiting = true;
            print_password();
        }
        else if(submit && !allnotZERO){
            submit = false;
            usbMsgPtr = (uint8_t*) &password_notsubmit;
            print_password();
        }
        else{
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

    return 0;
}

void reset_password(){
    int i;
    for(i=0;i<pass_length;i++){
      password_notsubmit[i]=0;
      password[i]=0;
    }
}

void setup()
{
    pinMode(PIN_PC2,OUTPUT);
    pinMode(PIN_PB0,INPUT_PULLUP);
    pinMode(PIN_PB1,INPUT_PULLUP);
    pinMode(PIN_PB2,INPUT_PULLUP);
    pinMode(PIN_PB3,INPUT_PULLUP);
    pinMode(PIN_PC1,OUTPUT);
    pinMode(PIN_PC0,OUTPUT);
    //myservo.attach(PIN_PB0);
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
            //digitalWrite(PIN_PD3, HIGH); //remove this line when lcd prompt
            //delay(5); //remove this line when lcd prompt
            if(!digitalRead(PIN_PB0) && noww < pass_length ){ //if A pressed and password not full
                password[noww] = 3;
                noww ++;
                //digitalWrite(PIN_PC0,1);
                while(!digitalRead(PIN_PB0));
                //digitalWrite(PIN_PC0,0);
            }
            else if(!digitalRead(PIN_PB1) && noww < pass_length ){ //if B pressed and password not full
                password[noww] = 4;
                noww ++;
                //digitalWrite(PIN_PC1,1);
                while(!digitalRead(PIN_PB1));
                //digitalWrite(PIN_PC1,0);
            }
            else if(!digitalRead(PIN_PB2) && noww > 0){ //if B pressed and password not full
                password[noww-1] = 0;
                noww --;
                //digitalWrite(PIN_PC1,1);
                while(!digitalRead(PIN_PB2));
                //digitalWrite(PIN_PC1,0);
            }
            else if(!digitalRead(PIN_PB3) ){ //if ENTER pressed and password is full
                submit=true;
                //digitalWrite(PIN_PC2,1);
                while(!digitalRead(PIN_PB3));
                //digitalWrite(PIN_PC2,0);
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
            print_door(true);
            state = -1;
        }
        else if( millis() - savetime > 8000){
                state = 0;
                set_start = 0;
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
