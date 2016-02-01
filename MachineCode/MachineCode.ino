// Using SparkFun Serial LCD
#include <SoftwareSerial.h>
SoftwareSerial mySerial(0,1); // pin 1 = TX, pin 0 = RX (unused)

#include <Keypad.h>
const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 8, 7, 6, 5 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 4, 3, 2 }; 

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//Συναρτήσεις
int dataEntering();
void loading ();
void highSpeedFunc ();
void lowSpeedFunc();
void endLoading();
float getWeight ();
void dispWeightText ();
void dispWeight ();
void endDisp ();

//Ορισμός επαφών ARDUINO
const int footSwitch  = 9;     // the pin that the footswitch is attached to
const int arpagi = 13; // Bag grabbing device
const int lowSpeed = 11;
const int highSpeed = 12;

//Μεταβλητές συστήματος
int sysWeight = 0; //Βάρος ζυγαριάς
boolean fSwitchState = LOW;
int preKpdValue = 0;
int kpdValue =0;
float bagWeight =0;
int intPart = 0;
int decPart = 0; 
int loadCellOut = 0;
int changeSpeedVal = 0;
boolean firstConnection= 0;

void setup() {
mySerial.begin(9600); // set up serial port for 9600 baud
delay(1000); // wait for display to boot up
pinMode(footSwitch, INPUT);  // initialize the FOOT SWITCH pin as a input
pinMode (arpagi,OUTPUT);
pinMode (highSpeed,OUTPUT);
pinMode (lowSpeed,OUTPUT);
pinMode (A1,OUTPUT); // Led 'READY'
pinMode (A2,OUTPUT); // Led 'LOADING'
pinMode (A3,OUTPUT); // Led 'LOADED'
//Λήψη βάρους ζυγαριάς
sysWeight = analogRead(0);
delay (10);
 //Αρχικές ενδείξεις οθόνης
  mySerial.write(254); // cursor to beginning of first line
  mySerial.write(128);
  mySerial.write ("                "); // clearing screen
  mySerial.write(254); // cursor to beginning of 2nd line
  mySerial.write(192);
  mySerial.write("                ");
  mySerial.write(254); // cursor to beginning of first line
  mySerial.write(128);
  mySerial.write ("INITIALISATION");     
  delay (500);
  mySerial.write(254); // cursor to beginning of first line
  mySerial.write(128);
  mySerial.write ("                "); // clearing screen
}


//Γίνεται αλλαγή στον κώδικα με εντολή digitalRead (footSwitch); στην επαφή footSwitch 
void loop() {
//reset weight
  char key1 = kpd.getKey();
  switch (key1){
  case NO_KEY:
      break;

  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
  case '*':
      break;
        
  case '#':
      digitalWrite (A1,LOW);
      kpdValue=0;
      preKpdValue =0;
      break;    
  }

  fSwitchState = digitalRead (footSwitch);
  if (kpdValue== 0){
    dataEntering();
  }
  else if (fSwitchState== HIGH && kpdValue> 0 ){
    delay (10); //debouncing
    digitalWrite (arpagi,HIGH);
    loading();
  }
  else if (fSwitchState== LOW && kpdValue> 0 ){
    digitalWrite (A1,HIGH);
    digitalWrite (A2,LOW);
    digitalWrite (A3,LOW);
    digitalWrite (arpagi,LOW);
    digitalWrite (lowSpeed,LOW);
    digitalWrite (highSpeed,LOW);
    readyDisp ();  
  }
  
}

int dataEntering(){
char kpdValueStr[10], preKpdValueStr[10];
sprintf(kpdValueStr,"%4d",kpdValue); // create strings from the numbers
sprintf(preKpdValueStr,"%4d",preKpdValue); // create strings from the numbers
mySerial.write(254); // cursor to beginning of first line
mySerial.write(128);
mySerial.write ("                "); // clearing screen
mySerial.write(254); // cursor to beginning of 2nd line
mySerial.write(192);
mySerial.write("                ");
mySerial.write(254); // cursor to beginning of first line
mySerial.write(128);
mySerial.write ("SET BAGLOAD :"); 
mySerial.write(254); // cursor to beginning of 2nd line
mySerial.write(192);
mySerial.write(preKpdValueStr); // write out the KPD value
char key1 = kpd.getKey();
  if(key1)  // Check for a valid key.
  {
switch (key1)
   {
     case NO_KEY:
      break;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    preKpdValue = preKpdValue * 10 + (key1 - '0');
 
 if (preKpdValue > 99) {
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write ("                "); // clearing screen
    mySerial.write(254); // cursor to beginning of 2nd line
    mySerial.write(192);
    mySerial.write("                ");
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write ("MAXIMUM 99 KG"); // clearing screen
    mySerial.write(254); // cursor to beginning of 2nd line
    mySerial.write(192);
    mySerial.write("RESET BAGLOAD");
    preKpdValue = 0;
    delay (1000);
    break;
 }
break;
// Στο keypad τα πλήκτρα * και # είναι ανάποδα
case '*':
    kpdValue = preKpdValue;
    if (kpdValue >0) {
    digitalWrite (A1,HIGH);
    return kpdValue;
    break;
}
else {
    digitalWrite (A1,LOW);
    break;
    }
        break;     
     
case '#':
      digitalWrite (A1,LOW);
      kpdValue=0;
      preKpdValue =0;
      break;    
   }
  } 
  
}
// Συναρτήσεις γεμισματος

void loading(){
 changeSpeedVal = 0.8*kpdValue;
 getWeight();
 
 if (bagWeight <= changeSpeedVal && bagWeight <= kpdValue)
    {highSpeedFunc ();}
 else if (bagWeight > changeSpeedVal && bagWeight <= kpdValue)
    {lowSpeedFunc();}
 else if (bagWeight > changeSpeedVal && bagWeight > kpdValue) 
    {endLoading();}  
}

void highSpeedFunc (){
 digitalWrite (A1,LOW);
 digitalWrite (A2,HIGH); //LOADING
 digitalWrite (A3,LOW);
 digitalWrite (highSpeed,HIGH);
 dispLoad();
 dispWeightText ();
 dispIntWeight ();
 dispDecWeight ();
}

void lowSpeedFunc(){
  //Αποφυγή ταυτοχρονης επαφης υψηλής και χαμηλής ταχύτητας
  if (firstConnection == 0){
    digitalWrite (A1,LOW);
    digitalWrite (A2,HIGH); //LOADING
    digitalWrite (A3,LOW);
    digitalWrite (highSpeed,LOW);
    delay (200);
    digitalWrite (lowSpeed,HIGH);
    firstConnection = 1;
    dispLoad();
    dispWeightText ();
    dispIntWeight ();
    dispDecWeight ();
  }
  else {
    digitalWrite (A1,LOW);
    digitalWrite (A2,HIGH); //LOADING
    digitalWrite (A3,LOW);
    digitalWrite (highSpeed,LOW);
    digitalWrite (lowSpeed,HIGH);
    dispLoad();
    dispWeightText ();
    dispIntWeight ();
    dispDecWeight ();
  }
}

void endLoading(){
    digitalWrite (A1,LOW);
    digitalWrite (A2,LOW); //LOADING
    digitalWrite (A3,HIGH);//LOADED
    digitalWrite (highSpeed,LOW);
    digitalWrite (lowSpeed,LOW);
    firstConnection = 0;
    endDisp();
}

float getWeight () {
  loadCellOut = analogRead(0);
  delay (10);
  bagWeight = loadCellOut - sysWeight;
  bagWeight = map(bagWeight, 0, 1023, 0, 100);
  intPart = bagWeight;
  decPart = bagWeight*10 - intPart*10;
  return bagWeight;
  return intPart;
  return decPart;
}

//Συναρτήσεις ενδείξεων LCD κατά την λειτουργια
void dispLoad(){
    char kpdValueStr[10];
    sprintf(kpdValueStr,"%2d",kpdValue); // create strings from the numbers
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write ("                "); // clearing screen
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write("BAGLOAD:"); 
    mySerial.write(254); // cursor to 10th position of 2nd line
    mySerial.write(137);
    mySerial.write(kpdValueStr);
    mySerial.write(254); // cursor to 13th position of 2nd line
    mySerial.write(140);
    mySerial.write("KG");   
}

void dispWeightText (){
  mySerial.write(254); // cursor to beginning of 2nd line
  mySerial.write(192);
  mySerial.write("                ");
  mySerial.write(254); // cursor to beginning of 2nd line
  mySerial.write(192);
  mySerial.write("BAGWEIGHT:");
}

void dispIntWeight (){
  char intPartStr[10], decPartStr[10];
  sprintf(intPartStr,"%2d",intPart); // create strings from the numbers (integer)
  mySerial.write(254); // cursor to 12th position of 2nd line
  mySerial.write(203);
  mySerial.write(intPartStr); // write out the integer value
  mySerial.write(254); // cursor to 14th position of 2nd line
  mySerial.write(205);
  mySerial.write('.'); // write out the separator
}

void dispDecWeight (){
  char decPartStr[10];
  sprintf(decPartStr,"%1d",decPart); // create strings from the numbers (decimal)
  mySerial.write(254); // cursor to 15th position of 2nd line
  mySerial.write(206);
  mySerial.write(decPartStr); // write out the decimal value
}


void readyDisp (){
    char kpdValueStr[10];
    sprintf(kpdValueStr,"%2d",kpdValue); // create strings from the numbers
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write ("                "); // clearing screen
    mySerial.write(254); // cursor to beginning of 2nd line
    mySerial.write(192);
    mySerial.write("                ");
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write ("READY TO LOAD"); 
    mySerial.write(254); // cursor to beginning of 2nd line
    mySerial.write(192);
    mySerial.write("BAGLOAD:"); 
    mySerial.write(254); // cursor to 10th position of 2nd line
    mySerial.write(201);
    mySerial.write(kpdValueStr);
    mySerial.write(254); // cursor to 13th position of 2nd line
    mySerial.write(204);
    mySerial.write("KG"); 
}

void endDisp (){
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write ("                "); // clearing screen
    mySerial.write(254); // cursor to beginning of 2nd line
    mySerial.write(192);
    mySerial.write("                ");
    mySerial.write(254); // cursor to beginning of first line
    mySerial.write(128);
    mySerial.write ("BAG LOADED !"); 
    mySerial.write(254); // cursor to beginning of 2nd line
    mySerial.write(192);
    mySerial.write("PLACE NEW BAG."); 
}


