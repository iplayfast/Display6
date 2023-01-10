
/* Code for running a 6 chip display */

/*
Note: We ran out of IO so we are disabling Cursor by making CUE and CU high. 
Then we are moving A5 and A4 to A3 and A2 (the old CUE and CU). This will allow us to use A5 and A4 as I2C
to communicate with another arduino
*/
#define MAXLONG 2147483647L

#define MINLONG -2147483648L

// digit selection
#define A_0 3  //good
#define A_1 4  // good

#define WR 5  // good

// character selection
#define D0 6
#define D1 7
#define D2 8
#define D3 9
#define D4 12
#define D5 11
#define D6 10

#define SCL A5  //PC2  //  A5 (circuit error)
#define SDA A4  //PC33    // A4  (circuit error)

// chip selection
#define D0CS A3  // A3 (circuit error)
#define D1CS A2  // A2 (circuit error)
#define D2CS A1  //A1
#define D3CS A0  // A0
#define D4CS 13  // D13
#define D5CS 2

// usb we avoid this
#define USB_NA1 PD0  // D0
#define USB_NA2 PD1  // D1



#define e_pulse 1
void ddigitalWrite(int p, int v) {
  Serial.print("Pin ");
  Serial.print(p);
  Serial.print(" value ");
  Serial.println(v);
  digitalWrite(p, v);
}

// display numbered 0-5, Pos numbered 0-3
void WriteChar(int Display, int Pos, unsigned char ch) {
  if (ch >= 'a' && ch <= 'z')  // make lowercase to upper
    ch = ch - ('a' - 'A');
  if (ch < ' ' || ch > '_')  // bad character
    ch = '*';

  // pos of digits numbered 3,2,1,0
  digitalWrite(D0, (ch & 1) ? HIGH : LOW);  // digit to display
  digitalWrite(D1, (ch & 2) ? HIGH : LOW);
  digitalWrite(D2, (ch & 4) ? HIGH : LOW);
  digitalWrite(D3, (ch & 8) ? HIGH : LOW);
  digitalWrite(D4, (ch & 16) ? HIGH : LOW);
  digitalWrite(D5, (ch & 32) ? HIGH : LOW);
  digitalWrite(D6, (ch & 64) ? HIGH : LOW);

  digitalWrite(A_1, (Pos & 2) ? HIGH : LOW);  // digit on display
  digitalWrite(A_0, (Pos & 1) ? HIGH : LOW);


  digitalWrite(D0CS, (Display == 0) ? LOW : HIGH);  // which display (1 through 6)
  digitalWrite(D1CS, (Display == 1) ? LOW : HIGH);
  digitalWrite(D2CS, (Display == 2) ? LOW : HIGH);
  digitalWrite(D3CS, (Display == 3) ? LOW : HIGH);
  digitalWrite(D4CS, (Display == 4) ? LOW : HIGH);
  digitalWrite(D5CS, (Display == 5) ? LOW : HIGH);
  delay(e_pulse);
  digitalWrite(WR, LOW);
  delay(e_pulse);
  digitalWrite(WR, HIGH);
}
void Clear() {
  for (int Display = 0; Display < 6; Display++)
    for (int Pos = 0; Pos < 4; Pos++)
      WriteChar(Display, Pos, ' ');
}

void setup() {
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(A_0, OUTPUT);
  pinMode(A_1, OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(D0CS, OUTPUT);
  pinMode(D1CS, OUTPUT);
  pinMode(D2CS, OUTPUT);
  pinMode(D3CS, OUTPUT);
  pinMode(D4CS, OUTPUT);
  pinMode(D5CS, OUTPUT);

  Clear();
  Serial.begin(115200);  // Any baud rate should work
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB
  }
  Serial.println("Hello Arduino\n");
}

/* serial utility routines */
void PromptPrint(const char *prompt, long Num) {
  Serial.print(prompt);
  Serial.print(Num);
}
void PromptPrintln(const char *prompt, long Num) {
  PromptPrint(prompt, Num);
  Serial.println("");
}
void PromptPrint(const char *prompt, const char *p) {
  Serial.print(prompt);
  Serial.print(p);
}
void PromptPrintln(const char *prompt, const char *p) {

  PromptPrint(prompt, p);
  Serial.println("");
}


/* display routies */
// row number 1..3 (two displays work together)
void WriteDisplays(int row, char *prompt) {
  row--;
  row *= 2;
  for (int i = 0; i < 8; i++)
    WriteChar(i < 4 ? row + 1 : row, 3 - (i & 3), *prompt ? *prompt++ : ' ');
}

// Display indicates which 4 char display is used, numbered 1..6
void WriteDisplay(int Display, char *text) {
  Display--;
  for (int i = 0; i < 4; i++) {
    if (text[i])
      WriteChar(Display, 3 - i, text[i]);
    else return;
  }
}

// Display indicates which 4 char display is used, numbered 1..6
void WriteDisplay(int Display, int n) {
  
  char number[20];
  if (n > 9999) WriteDisplay(Display, ">999");
  else {
    if (n < -999) WriteDisplay(Display, "<-99");
    else {
      Display--;
      char lastchar = n<0 ? '-' : ' ';
      
      if (n < 0) 
        n = -n;  // in this case n can't be larger than 999 at this point     
      int d1 = n - (n / 10) * 10;       // eg 123  d1 = 3
      WriteChar(Display, 0, d1 + '0');  // we right justify the number always print first digit 
      n /= 10;
      int d2 = n - (n / 10) * 10;  //  eg 12   d2 = 2
      WriteChar(Display, 1,(d2 || n) ? d2 + '0' : ' '); // print digit if either digit is non-zero or n is non-zero
      n /= 10;
      int d3 = n - (n / 10) * 10;
      n /= 10;
      WriteChar(Display, 2,(d3 || n) ? d3 + '0' : ' ');
      WriteChar(Display, 3, n ?  n + '0' : lastchar);  // don't print leading zeros,

    }
  }
}


/* row is which 8 digit display to put value on, numbered from 0 to 2, n is the value to be displayed */
void WriteDisplays(int row, long n) {
  char display[20];

  char overflow = '\0';
  if (n > 99999999) strcpy(display, "Overflow");
  else {
    if (n < -9999999) strcpy(display, "Underflow");
    else sprintf(display, "%ld", n);
  }
  display[8] = '\0';
  //if (n>99999999)
  //    display[0] = '>';
  WriteDisplays(row, display);
}


long g = 999000;
long u = 1;
long lastu = -1;

void TestLoop() {


  for (char a = 'A'; a <= 'Z'; a++)
    for (int display = 0; display < 6; display++)
      for (int digit = 0; digit < 4; digit++) {
        WriteChar(display, digit, a);
        delay(10);
      }
  delay(1000);
}

void loop() {
int Test = 1;
  Clear();
  PromptPrintln("6 displays showing overflow  Test:",Test++);
  WriteDisplay(1,11111);
  delay(1000);
  WriteDisplay(2,22222);
   delay(1000);
  WriteDisplay(3,13333);
   delay(1000);
  WriteDisplay(4,14444);
   delay(1000);
  WriteDisplay(5,15555);
   delay(1000);
  WriteDisplay(6,16666);
  delay(5000);
  Clear();
  PromptPrintln("6 displays showing underflow  Test:",Test++);
  WriteDisplay(1,-1111);
   delay(1000);
  WriteDisplay(2,-2222);
   delay(1000);
  WriteDisplay(3,-3333);
   delay(1000);
  WriteDisplay(4,-4444);
   delay(1000);
  WriteDisplay(5,-5555);
   delay(1000);
  WriteDisplay(6,-6666);
  delay(5000);
    
  PromptPrintln("6 displays counting from -1000 to +1000 by 11s  Test:",Test++);
  for(int i=-1000;i<1000;i+=11)    
    for(int d=1;d<7;d++)
      WriteDisplay(d,i);
  delay(5000);
  
  Clear();
  PromptPrintln("6 displays showing their number as text   Test:", Test++);
  
  WriteDisplay(1, "one1");
  WriteDisplay(2, "two2");
  WriteDisplay(3, "Thr3");
  WriteDisplay(4, "Fou4");
  WriteDisplay(5, "Fiv5");
  WriteDisplay(6, "six6");
  delay(5000);
  Clear();
  PromptPrintln("3 display lines showing their number as text   Test:", Test++);
  WriteDisplays(1, "1st row ");
  WriteDisplays(2, "2nd Row");
  WriteDisplays(3, "3rd Row ");
  delay(5000);
  
  Clear();
  PromptPrint("3 display lines counting from -100000 to +10000000   Test:", Test++);
  for (int i = -10000; i < 9999; i++)
    for (int d = 1; d < 4; d++)
      WriteDisplays(d, i);
  delay(5000);
  Clear();
}
