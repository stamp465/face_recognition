#include <usbdrv.h>
#include <Wire.h>
#include <Servo.h>
Servo myservo;
#define RQ_SET_LED 0
#define RQ_GET_SWITCH 1
#define RQ_GET_PW 2
#define RQ_SET_SERVO 3

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
    myservo.attach(PIN_PB0);
    myservo.write(0);
    delay(1000);
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
}
