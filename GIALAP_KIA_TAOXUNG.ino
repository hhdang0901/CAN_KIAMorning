#include <avr/interrupt.h>

#define ANALOG_TPS A0
#define PIN_CMP  2
#define PIN_CKP  3
#define PIN_ISS  4
#define PIN_OSS  5
#define PIN_VSS  6
#define PIN_A   23
#define PIN_B   25
#define PIN_C   27
#define PIN_D   29
#define PIN_AT_P  31
#define PIN_AT_N  33
#define PIN_AT_D  35
#define PIN_AT_2  37
#define PIN_AT_L  39
#define PIN_AT_R  41

#define F_CPU 16000000UL
#define pi  3.14
#define rw  0.3 // thong so lop 175/70R14
#define io 4.587

volatile bool skip_two_pulses;
volatile  int count_ckp;
long ESS,ISS,OSS,VSS;
long freq_ESS, freq_ISS, freq_OSS, freq_VSS;
long TOP_timer1, TOP_timer3, TOP_timer4, TOP_timer5;

// Ngắt Timer1 Compare Match A
ISR(TIMER1_COMPA_vect) {
  digitalWrite(PIN_ISS, !digitalRead(PIN_ISS));
  OCR1A = TOP_timer1;
}

// Ngắt Timer3 Compare Match A
ISR(TIMER3_COMPA_vect) {
  digitalWrite(PIN_OSS, !digitalRead(PIN_OSS));
  OCR3A = TOP_timer3;
}

// Ngắt Timer4 Compare Match A
ISR(TIMER4_COMPA_vect) {
  digitalWrite(PIN_VSS, !digitalRead(PIN_VSS));
  OCR4A = TOP_timer4;
}

ISR(TIMER5_COMPA_vect) {  
  if (skip_two_pulses) {
    static uint8_t skip_counter = 0;
    skip_counter++;
    if (skip_counter >= 4) {
      skip_two_pulses = false;
      skip_counter = 0;
    }
    return;
  }
  count_ckp++;
  digitalWrite(PIN_CKP, !digitalRead(PIN_CKP));   
  if (count_ckp == 74)   digitalWrite(PIN_CMP, !digitalRead(PIN_CMP));
  if (count_ckp >= 116) {
    count_ckp = 0;
    skip_two_pulses = true;
  }
  OCR5A = TOP_timer5;
}

void initTimers() {
  cli(); // Tắt ngắt toàn cục

  // Timer1 setup
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS11) | (1 << CS10); // Prescaler = 64
  OCR1A = 415; // Initial compare value
  TIMSK1 = (1 << OCIE1A); // Enable Timer1 Compare Match A Interrupt

  // Timer3 setup
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3B |= (1 << WGM32); // CTC mode
  TCCR3B |= (1 << CS31) | (1 << CS30); // Prescaler = 64
  OCR3A = 415; // Initial compare value
  TIMSK3 = (1 << OCIE3A); // Enable Timer3 Compare Match A Interrupt

  // Timer4 setup
  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= (1 << WGM42); // CTC mode
  TCCR4B |= (1 << CS41) | (1 << CS40); // Prescaler = 64
  OCR4A = 415; // Initial compare value
  TIMSK4 = (1 << OCIE4A); // Enable Timer4 Compare Match A Interrupt

  // Timer5 setup
  TCCR5A = 0;
  TCCR5B = 0;
  TCCR5B |= (1 << WGM52); // CTC mode
  TCCR5B |= (1 << CS51) | (1 << CS50); // Prescaler = 64
  OCR5A = 415; // Initial compare value
  TIMSK5 = (1 << OCIE5A); // Enable Timer5 Compare Match A Interrupt

  sei(); // Bật lại ngắt toàn cục
}

void setup() {
  initTimers();
  Serial.begin(9600);

  pinMode(PIN_CMP, OUTPUT);
  pinMode(PIN_CKP, OUTPUT);
  pinMode(PIN_ISS, OUTPUT);
  pinMode(PIN_OSS, OUTPUT);
  pinMode(PIN_VSS, OUTPUT);

}

  int read_valve() {
  // Đọc trạng thái từng van
  int vanA = digitalRead(PIN_A);
  int vanB = digitalRead(PIN_B);
  int vanC = digitalRead(PIN_C);
  int vanD = digitalRead(PIN_D);

  // Chuyển đổi trạng thái van thành giá trị số
  int giaTri = vanA * 8 + vanB * 4 + vanC * 2 + vanD;
  Serial.println(giaTri,BIN); 



  // Trả về giá trị tương ứng với trạng thái van
  switch (giaTri) {
    case 0b1001: // N,P
      return 0;
    case 0b0011: // 1
      return 1;   
    case 0b0110: // 2
      return 2;
    case 0b0111: // 2
      return 2;
    case 0b1010: // 3
      return 3;
    case 0b1100: // 4
      return 4;
    case 0b1101: // R
      return 5;
    default:
      return 3; // Giá trị khi lỗi
  }
}

float xacDinhGearRatio(int i) {
  switch (i) {
    case 1: 
      return 2.919;
    case 2: 
      return 1.551;
    case 3: 
      return 1.0;
    case 4: 
      return 0.713;
    case 5: 
      return 2.480;
    default:
      return 1.0; 
  }
}

int xacDinhmode() {
  int mode; 

  int pin_status_1 = digitalRead(PIN_AT_P);
  int pin_status_2 = digitalRead(PIN_AT_N);
  int pin_status_3 = digitalRead(PIN_AT_D);
  int pin_status_4 = digitalRead(PIN_AT_2);
  int pin_status_5 = digitalRead(PIN_AT_L);
  int pin_status_6 = digitalRead(PIN_AT_R);

  if (pin_status_1 == LOW || pin_status_2 == LOW ) {
    mode = 1;
    //Serial.println("Che do NP ");     
  } else if (pin_status_3 == LOW || pin_status_4 == LOW || pin_status_5 == LOW || pin_status_6 == LOW) {
    mode = 2;
    //Serial.println("Che do D2LR ");     
  }

  return mode;
}

void loop() {

  
  int value = analogRead(ANALOG_TPS);
  freq_ESS = map(value, 0, 1023, 0, 6000);
  // freq_ESS = 000;
      
float current_gear_ratio = xacDinhGearRatio(read_valve());

 int mode = xacDinhmode();
 ESS = freq_ESS;
  switch (mode) {
    case 1:
      ISS = 0; OSS = 0;  VSS = 0;
      break;
    case 2:
      ISS = 0.95 * ESS;
      OSS = ISS / current_gear_ratio;
      VSS = (3.6*ESS*pi*rw)/(30*current_gear_ratio*io);
      break;
  }

  //-------(1) Chuyen tu gia tri sang tan so -------- 
  long freq_ISS = 0.5 * ISS;
  long freq_OSS = OSS;
  long freq_VSS = 0.7 * VSS;  
  //-------(2) Chuyen tu gia tri sang tan so --------   
    TOP_timer1 = F_CPU / (2 * freq_ISS * 64) - 1;
    TOP_timer3 = F_CPU / (2 * freq_OSS * 64) - 1;
    TOP_timer4 = F_CPU / (2 * freq_VSS * 64) - 1;
    TOP_timer5 = F_CPU / (2 * freq_ESS * 64) - 1;

}
   