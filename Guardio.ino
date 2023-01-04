#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <Servo.h>

Servo myservo;
int pos = 0;

const int led_pin_accept = 13;
const int led_pin_deny = 12;
volatile int finger_status = -1;

SoftwareSerial mySerial(11, 10); // TX/RX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id;

const byte numRows = 4;
const byte numCols = 3;

byte rowPins[numRows] = {9,8,7,6}; 
byte colPins[numCols]= {5,4,3}; 

char keymap[numRows][numCols]= 
{
    {'1', '2', '3'}, 
    {'4', '5', '6'}, 
    {'7', '8', '9'},
    {'*', '0', '#'}
};

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

char* girSifre = "4267";  // change the girSifre here, just pick any 3 numbers
char keypressed;
int yeri = 0;

void setup()
{
    myservo.attach(2);
    Serial.begin(9600);


  pinMode(led_pin_accept,OUTPUT);
  pinMode(led_pin_deny,OUTPUT);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
}

void loop()
{
   keypressed = myKeypad.getKey();
   if (keypressed != NO_KEY)
   {
      Serial.println(keypressed);
      Serial.println(yeri);

      if (keypressed == girSifre[yeri])
         {yeri ++;}
      else
         {yeri = 0;}

      if (yeri == String(girSifre).length())
      {
         Serial.println("Ready to enroll a fingerprint!");
         keypressed = myKeypad.getKey();
         id = keypressed;
         if (id == 0) 
         {// ID #0 not allowed, try again!
            if (keypressed)
            {
               Serial.print("bastı  "); 
               Serial.println(keypressed); 
               return;
            }
        }
        Serial.print("Enrolling ID #");
        Serial.println(id);
                      
        while (!  getFingerprintEnroll() );
           yeri=0;
      }          
   }
   Serial.print("duduk");
   finger_status = getFingerprintIDez();
   if (finger_status!=-1 and finger_status!=-2)
   {
      digitalWrite(led_pin_accept,HIGH);
      delay(1000);
      for (pos = 0; pos <= 180; pos += 1) {
         myservo.write(pos);                                    
      }
      delay(1000);
      for (pos = 180; pos >= 0; pos -= 1) {
         myservo.write(pos);   }
   } else{
      if (finger_status==-2){
         for (int ii=0;ii<5;ii++){
            digitalWrite(led_pin_deny,HIGH);
            delay(50);
            digitalWrite(led_pin_deny,LOW);
            delay(50);
          }
       }
       digitalWrite(led_pin_accept,LOW);
   }
   delay(50);            //don't ned to run this at full speed.   
}


//readnumber fonksiyonu
uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

//getfingerprint fonksiyonu
  uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p!=2){
    Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;
  
  p = finger.image2Tz();
  if (p!=2){
    Serial.println(p);
  }
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -2;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}
