#include <LiquidCrystal.h>    //LCD library
#include <Wire.h>
#include <DS3231.h> //RTC library
#include <string.h>

//lcd declarations
volatile bool lcd_update = false; 
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 6; //lcd pins
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//ISR declarations
const int button = 2; 
volatile bool direction = HIGH;
int x = 0;
int test = 0;

//Motor 
const int enA = 8;  //PIN 1 ENABLE 
const int in1 = 10; //PIN 2 of L293
const int in2 = 9;  //PIN 7 of L293
int n = 1; //set speed
int speeds[4] = {0, 127, 191, 255};
String outputs[4]= {" 0%", " 50%", " 75%", " 100%"};

DS3231 clock;
RTCDateTime dt;


void setup() {
  //timer1 interrupt at 1Hz
  cli();
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz
  OCR1A = 15624;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
  
  //RTC
  Serial.begin(9600);
  clock.begin();
  clock.setDateTime(__DATE__, __TIME__); 

  //LCD
  lcd.begin(16, 2);  // set up the LCD's number of columns and rows:

  //Motor
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  //Button
  pinMode(button, INPUT);

  //set button to interrupt
  attachInterrupt(digitalPinToInterrupt(button), Fan_direction, RISING);
}

//timer interrupt
ISR(TIMER1_COMPA_vect){ 
  lcd_update = true;
}

//button interrupt
void Fan_direction(){ 
  direction = !direction;
}

void loop() {
  if(lcd_update){
  lcd.clear();

  lcd.setCursor(0, 0);
  dt = clock.getDateTime();
  lcd.print(dt.month);
  lcd.print('/');
  lcd.print(dt.day);
  lcd.print('/');
  lcd.print(dt.year);
  lcd.print(" ");
  if(direction == HIGH)
    lcd.print("C");
  if(direction == LOW)
    lcd.print("CC");
  
  lcd.setCursor(0, 1);
  lcd.print(dt.hour);
  lcd.print(':');
  lcd.print(dt.minute);
  lcd.print(':');
  lcd.print(dt.second);
  lcd.print("  ");
  lcd.print(outputs[n]);
  
  lcd_update = false;
  }
  
  if(dt.second < 30)
    run_fan();
  if(dt.second >= 30)
    stop_fan();
}

void run_fan() {
  analogWrite(enA, speeds[n]);
  digitalWrite(in1, direction);
  digitalWrite(in2, !direction);
}

void stop_fan() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}
