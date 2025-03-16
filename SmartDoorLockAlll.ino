#include<Wire.h>
#include<LiquidCrystal_I2C.h>
#include<DHT.h> 
//#include<Keypad.h>
//#include <SimpleKeypad.h>
#include<WiFi.h>
#include<ESP_Mail_Client.h>
#include "arduino_secrets.h"
#include "thingProperties.h"
#include<SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define dhtPin 15
#define dhtType DHT11

// Initialize Telegram BOT
#define BOTtoken "7230981972:AAEFo2FxODCeX03-a0aqH33Ka8xSZwcum3I"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "5029073982"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

SoftwareSerial gsm(16, 17); //rx tx of ESP32

LiquidCrystal_I2C lcd(0x3F, 20, 4);

DHT dht(dhtPin,dhtType);

#define SS_PIN 21
#define RST_PIN 22

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

#define row 4
#define col 3

#define button1 32
#define button2 35
#define button3 34
#define button4 39

#define card "DA9ADFB"
#define tag  "C3349DD"
#define nfc " 04DE90CA792B80"


String tagID = "";
const byte relay = 4; const byte button = 36;
const byte alert1 = 2; const byte alert2 = 5; //Alert1 Red, Alert2 Green   2/5
float temperature; byte humidity; 

byte rowPins[row] = {13, 12, 14, 27};
byte colPins[col] = {26, 25, 33};


char keys[row][col] = { //keySet

  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}

};

//Keypad myKeypad = Keypad(makeKeymap(keySet), rowPins, colPins, row, col);
//SimpleKeypad myKeypad((char*)key_chars, rowPins, colPins, row, col);

char key;

const byte passwordSize = 7;
char mainPassword[passwordSize] = ("123456");
char inputPassword[passwordSize];
unsigned long otp;
byte count = 0;


bool count1 = 0;

//// WiFi Credinetials

const char* ssid = "EEE" ;
const char* password = "qnh1013hg" ;

// Email Credinetials

#define SMTP_server "smtp.gmail.com"

#define SMTP_Port 465

#define sender_email "rehanaakterrumi71@gmail.com"

#define sender_password "sjuhbwjneoglsaab"

#define Recipient_email "hoquerafi727@gmail.com"

#define Recipient_name "Rafi"

SMTPSession smtp;

String msg1 = "Door has been opened. Thank You.";
String msg2 = "Suspicious unlocking attempt detected.";

byte lock[8] = {
  0b01110,
  0b10001,
  0b10001,
  0b11111,
  0b11011,
  0b11011,
  0b11111,
  0b11111
};

byte unlock[8] = {
  0b00110,
  0b00001,
  0b00001,
  0b11111,
  0b11011,
  0b11011,
  0b11111,
  0b11111
};

byte angry[8] = {
  0b11111,
  0b11111,
  0b01110,
  0b11111,
  0b11011,
  0b11111,
  0b10001,
  0b11111
};

byte tick[8] = {
  0b00000,
  0b00000,
  0b00001,
  0b00010,
  0b10100,
  0b01000,
  0b00000,
  0b00000
};

byte bell[] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00000,
  B00100,
  B00000
};

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    if (text == "/unlock") {
      digitalWrite(relay, LOW);
      lockState=false; 
      lockChart =1;
      ArduinoCloud.update();

      bot.sendMessage(CHAT_ID, "Door has been unlocked from Telegram", "");

      lcd.clear(); 
      lcd.setCursor(0, 0);
      lcd.print("REMOTE ACCESS ");
      lcd.write(byte(4)); lcd.write(byte(4)); lcd.write(byte(4));

      lcd.setCursor(0, 1);
      lcd.print("        ");
      lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3));
      lcd.print("        ");

      lcd.setCursor(0, 3);
      lcd.print("DOOR UNLOCKED ");
      digitalWrite(alert2, HIGH);
      lcd.write(byte(1)); lcd.write(byte(1)); lcd.write(byte(1));

      delay(3000);
      digitalWrite(relay, HIGH);
      digitalWrite(alert2, LOW);
      lockState=true; 
      lockChart =0;
      lcd.clear();

      delay(200);
    }


    else {
      bot.sendMessage(CHAT_ID, "Unknown command. Use /unlock", "");
    }
  }
}


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Wire.begin(3, 1,100000);

  gsm.begin(9600);
  dht.begin();
  SPI.begin();
  mfrc522.PCD_Init();   // Initiate MFRC522


  WiFi.begin(ssid , password);

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  bot.sendMessage(CHAT_ID, "Door Lock System Booted", "");

  lcd.init();
  lcd.backlight();

  lcd.createChar(0, lock);
  lcd.createChar(1, unlock);
  lcd.createChar(2, angry);
  lcd.createChar(3, tick);
  lcd.createChar(4, bell);

  lcd.setCursor(0, 0);
  lcd.print("WELCOME TO RAFI'S");
  lcd.setCursor(0, 1);
  lcd.print("SMART DOOR LOCK");
  lcd.setCursor(0, 2);
  lcd.print("UNDERGRAD STUDENT");
  lcd.setCursor(0, 3);
  lcd.print("DEPT. OF EEE, BUET");

  Serial.println("Initializing Netwrok...");

  gsm.println("AT");
  checkSerial();

  gsm.println("AT+CSQ");
  checkSerial();

  gsm.println("AT+CCID");
  checkSerial();

  gsm.println("AT+CREG?");
  checkSerial();

  gsm.println("AT+CBC");
  checkSerial();

  gsm.println("AT+COPS?");
  checkSerial();

  gsm.println("AT+CMGF=1"); // Initializes the text mode
  checkSerial();

  gsm.println("AT+CNMI=2,2,0,0,0"); // Decides how newly arrived messages will be handled
  checkSerial();

  //  gsm.println("AT+CMGD=1,4");
  //  checkSerial();

  delay(4000);
  lcd.clear();

  // Connect to Wi-Fi
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(1000);
  //    Serial.println("Connecting to WiFi...");
  //  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


  
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);
  pinMode(button, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(alert1, OUTPUT);
  pinMode(alert2, OUTPUT);
  digitalWrite(relay, HIGH);
  digitalWrite(alert1, LOW);
  digitalWrite(alert2, LOW);

  // Set row pins as OUTPUT
  for (int i = 0; i < row; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
  }

  // Set column pins as INPUT_PULLDOWN
  for (int i = 0; i < col; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }

}

// Function to read a key press
char readKeypad() {
  for (int r = 0; r < row; r++) {
    // Activate one row at a time
    digitalWrite(rowPins[r], LOW);
    
    for (int c = 0; c < col; c++) {
      if (digitalRead(colPins[c]) == LOW) { // If key is pressed
        while (digitalRead(colPins[c]) == LOW); // Wait until key is released
        digitalWrite(rowPins[r], HIGH); // Reset the row
        return keys[r][c]; // Return the detected key
      }
    }

    digitalWrite(rowPins[r], HIGH); // Reset the row before next iteration
  }
  return '\0'; // Return null character if no key is pressed
}


void loop() {
  // put your main code here, to run repeatedly:

  ArduinoCloud.update();
  temperature = dht.readTemperature(); temperature1=temperature; 
  humidity = dht.readHumidity(); humidity1=humidity;

  lcd.setCursor(0, 0);
  lcd.print("ENTER 1 FOR PWD");
  lcd.setCursor(0, 1);
  lcd.print("ENTER 2 FOR OTP");
  lcd.setCursor(0, 2);
  lcd.print("ENTER 3 FOR RFID");
  lcd.setCursor(0, 3);
  lcd.print("T&H: ");
  lcd.print(temperature);
  lcd.write(223);
  lcd.print("C & ");
  lcd.print(humidity);
  lcd.print("%");
  lcd.print("     ");

  if (digitalRead(button1) == HIGH) {
    count1 = 1;
    //mainPassword[passwordSize] = ("123456");
    lcd.clear();

    while (count1) {

      lcd.setCursor(0, 0);
      lcd.print("DOOR IS LOCKED  ");
      lcd.write(byte(0)); lcd.write(byte(0)); lcd.write(byte(0));

      lcd.setCursor(0, 2);
      lcd.print("ENTER PASSWORD: ");
      char key = readKeypad(); // Call function to read a key



      //key = myKeypad.getKey();

      if (key) {

        lcd.setCursor(count, 3);
        lcd.print('*');
        inputPassword[count] = key;
        count++;
        Serial.print(key);
      }

      if (count == passwordSize - 1) {

        lcd.clear();

        if (strcmp(mainPassword, inputPassword) == 0) {

          digitalWrite(relay, LOW);
          lockState=false; 
          lockChart =1;

          lcd.setCursor(0, 0);
          lcd.print("PASSWORD MATCHED ");
          lcd.write(byte(4)); lcd.write(byte(4)); lcd.write(byte(4));

          lcd.setCursor(0, 1);
          lcd.print("        ");
          lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3));
          lcd.print("        ");

          lcd.setCursor(0, 3);
          lcd.print("DOOR UNLOCKED ");
          digitalWrite(alert2, HIGH);
          lcd.write(byte(1)); lcd.write(byte(1)); lcd.write(byte(1));
          ArduinoCloud.update();

          delay(3000);
          digitalWrite(relay, HIGH);
          digitalWrite(alert2, LOW);
          lockState=true; 
          lockChart =0;
          lcd.clear();

          delay(200);

        }

        else {

          digitalWrite(alert1, HIGH);
          lcd.setCursor(0, 0);
          lcd.print("PASSWORD MISSMATCHED");

          lcd.setCursor(0, 1);
          lcd.print("        ");
          lcd.write(byte(2)); lcd.write(byte(2)); lcd.write(byte(2)); lcd.write(byte(2));
          lcd.print("        ");

          lcd.setCursor(0, 3);
          lcd.print("PLEASE TRY AGAIN");
          for (int i = 0; i < 10; i++) {
            digitalWrite(alert1, HIGH);
            delay(300);
            digitalWrite(alert1, LOW);
            delay(300);  
          }
//          delay(3000);
//          digitalWrite(alert1, LOW);

          lcd.clear();
          delay(200);

          lcd.setCursor(0, 0);
          lcd.print("SENDING FIRST ALERT ");
          lcd.setCursor(0, 2);
          lcd.print("PLEASE WAIT A BIT...");

          sendmail(msg2);

          lcd.setCursor(0, 0);
          lcd.print("SENDING SECOND ALERT");
          sms(msg2);

          lcd.setCursor(0, 0);
          lcd.print("SENDING SECOND ALERT");
          bot.sendMessage(CHAT_ID, msg2, "");

          lcd.clear();


        }
        delay(200);
        count = 0;
        count1 = 0;

      }
    }

  }


 //This section deals with the otp feature

  if (digitalRead(button2) == HIGH) {

    count1 = 1;
    lcd.clear();

    

      otp = random(100000, 999999);
      sprintf(mainPassword, "%lu", otp);
      Serial.println(mainPassword);

      lcd.setCursor(0, 0);
      lcd.print("SENDING OTP ");
      lcd.setCursor(0, 2);
      lcd.print("PLEASE WAIT A BIT...");
      msg4();
      // Create a message string with the OTP
      String message = "Your OTP is: " + String(mainPassword);
      
      // Send the message to Telegram
      bot.sendMessage(CHAT_ID, message, "");
      lcd.clear();
      delay(200);
      
   while (count1) {
      lcd.setCursor(0, 0);
      lcd.print("OTP SENT TO PHONE ");
      lcd.write(byte(4)); lcd.write(byte(4));
      lcd.setCursor(0, 2);
      lcd.print("ENTER OTP: ");

      //key = myKeypad.getKey();
      char key = readKeypad(); // Call function to read a key

      if (key)  {

        lcd.setCursor(count+11, 2);
        lcd.print('*');
        inputPassword[count] = key;
        count++;
      }

      if (count == passwordSize - 1) {

        if (strcmp(mainPassword, inputPassword) == 0) {

          digitalWrite(relay, LOW);
          digitalWrite(alert2, HIGH);
          lockState=false; 
          lockChart =1;
         

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("OTP VERIFIED ");
          lcd.write(byte(4)); lcd.write(byte(4)); lcd.write(byte(4));

          lcd.setCursor(0, 1);
          lcd.print("        ");
          lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3));
          lcd.print("        ");

          lcd.setCursor(0, 3);
          lcd.print("DOOR UNLOCKED ");
          lcd.write(byte(1)); lcd.write(byte(1)); lcd.write(byte(1));
          lcd.print("   ");
          ArduinoCloud.update();
          delay(3000);

          digitalWrite(relay, HIGH);
          digitalWrite(alert2, LOW);
          lockState=true; 
          lockChart =0;
          lcd.clear();
          delay(200);


        }

        else {

          digitalWrite(alert1, HIGH);

          lcd.setCursor(0, 0);
          lcd.print("PASSWORD MISSMATCHED");

          lcd.setCursor(0, 1);
          lcd.print("        ");
          lcd.write(byte(2)); lcd.write(byte(2)); lcd.write(byte(2)); lcd.write(byte(2));
          lcd.print("        ");

          lcd.setCursor(0, 2);
          lcd.print("                    ");

          lcd.setCursor(0, 3);
          lcd.print("PLEASE TRY AGAIN");

          for (int i = 0; i < 10; i++) {
            digitalWrite(alert1, HIGH);
            delay(300);
            digitalWrite(alert1, LOW);  
            delay(300);  
          }

//          delay(3000);
//
//          digitalWrite(alert1, LOW);
          lcd.clear();
          delay(200);

          lcd.setCursor(0, 0);
          lcd.print("SENDING FIRST ALERT ");
          lcd.setCursor(0, 2);
          lcd.print("PLEASE WAIT A BIT...");
          sendmail(msg2);

          lcd.setCursor(0, 0);
          lcd.print("SENDING SECOND ALERT");
          sms(msg2);

          lcd.setCursor(0, 0);
          lcd.print("SENDING THIRD ALERT");
          bot.sendMessage(CHAT_ID, msg2, "");

          lcd.clear();
          delay(200);

        }

        count = 0;
        count1 = 0;
      }

    }

  }

  if (digitalRead(button3) == HIGH) {

    count1 = 1;
    lcd.clear();
    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("SCAN YOUR ID...");
    lcd.setCursor(0, 3);
    lcd.print("4 --> RETURN HOME");

    while (count1) {

      if (digitalRead(button4) == HIGH) {
        lcd.clear(); 
        break; 
      } 
      while (getID()) {
        Serial.println(getID());

        Serial.println("Card Detected!");
        lcd.setCursor(0, 2);
        lcd.print("TAG DETECTED");
        Serial.print("Tag ID: ");
        Serial.println(tagID);

        if ((tagID == card) || ( tagID == tag)) //change here the UID of the card/cards that you want to give access
        {

          digitalWrite(relay, LOW);
          lockState=true; 
          lockChart =1;
         

          Serial.println("Authorized access to this card");
          Serial.println();


          lcd.clear();
          delay(300);
          lcd.setCursor(0, 0);
          lcd.print("ID VERIFIED ");
          lcd.write(byte(4)); lcd.write(byte(4)); lcd.write(byte(4));
          lcd.print("    ");

          lcd.setCursor(0, 1);
          lcd.print("        ");
          lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3));
          lcd.print("        ");

          lcd.setCursor(0, 2);
          lcd.print("                    ");

          lcd.setCursor(0, 3);
          lcd.print("DOOR UNLOCKED ");
          digitalWrite(alert2, HIGH);
          lcd.write(byte(1)); lcd.write(byte(1)); lcd.write(byte(1));
          ArduinoCloud.update();

          delay(3000);
          digitalWrite(relay, HIGH);
          lockState=false; 
          lockChart =0;
          digitalWrite(alert2, LOW);
          lcd.clear();
          delay(500);
          count1 = 0;

        }

        else
        {

          Serial.println(" Access denied");

          byte k;

          lcd.clear();
          delay(300);
          lcd.setCursor(0, 0);
          lcd.print("UNAUTHORIZED ID!    ");

          lcd.setCursor(0, 1);
          lcd.print("        ");
          lcd.write(byte(2)); lcd.write(byte(2)); lcd.write(byte(2)); lcd.write(byte(2));
          lcd.print("        ");

          lcd.setCursor(0, 2);
          lcd.print("                    ");

          lcd.setCursor(0, 3);
          lcd.print("PLEASE TRY AGAIN    ");

          for (k = 0; k <= 5; k++)
          {
            digitalWrite (alert1, HIGH);
            delay(300);
            digitalWrite (alert1, LOW);
            delay(300);
          }


          lcd.clear();
          delay(500);

          lcd.setCursor(0, 0);
          lcd.print("SENDING FIRST ALERT ");
          lcd.setCursor(0, 2);
          lcd.print("PLEASE WAIT A BIT...");

          sendmail(msg2);

          lcd.setCursor(0, 0);
          lcd.print("SENDING SECOND ALERT");
          sms(msg2);

          lcd.setCursor(0, 0);
          lcd.print("SENDING THIRD ALERT");
          bot.sendMessage(CHAT_ID, msg2, "");

          lcd.clear();
          count1 = 0;
        }

      }

    }

  }



  // Unlocking the door from inside


  if (digitalRead(button) == HIGH) {

    digitalWrite(relay, LOW);
    digitalWrite(alert2, HIGH);
    
    lockState=false; 
    lockChart =1;

    lcd.setCursor(0, 0);
    lcd.print("BUTTON PRESSED  ");
    lcd.write(byte(4)); lcd.write(byte(4)); lcd.write(byte(4));

    lcd.setCursor(0, 1);
    lcd.print("        ");
    lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3));
    lcd.print("        ");

    lcd.setCursor(0, 2);
    lcd.print("                    ");

    lcd.setCursor(0, 3);
    lcd.print("DOOR UNLOCKED ");
    lcd.write(byte(1)); lcd.write(byte(1)); lcd.write(byte(1));
    lcd.print("   ");
    digitalWrite(alert2, HIGH);
    ArduinoCloud.update();

    delay(3000);

    digitalWrite(relay, HIGH);
    digitalWrite(alert2, LOW);
    lockState=true; 
    lockChart =0;
    lcd.clear();
    delay(500);

  }

  //Config Tg Rx Msg Interval

  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("New Response from Telegram");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

}

void onLockCloudChange()  {
  // Add your code here to act upon LockCloud change

     if (lockCloud) {

        digitalWrite(relay, LOW);
        digitalWrite(alert2, HIGH);
        lockState=false; 
        lockChart =1;

        lcd.setCursor(0, 0);
        lcd.print("BUTTON PRESSED  ");
        lcd.write(byte(4)); lcd.write(byte(4)); lcd.write(byte(4));

        lcd.setCursor(0, 1);
        lcd.print("        ");
        lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3));
        lcd.print("        ");

        lcd.setCursor(0, 2);
        lcd.print("                    ");

        lcd.setCursor(0, 3);
        lcd.print("DOOR UNLOCKED ");
        lcd.write(byte(1)); lcd.write(byte(1)); lcd.write(byte(1));
        lcd.print("   ");
        digitalWrite(alert2, HIGH);
        bot.sendMessage(CHAT_ID, "Door Unlocked from Cloud", "");
        ArduinoCloud.update();
        delay(3000);

        digitalWrite(relay, HIGH);
        digitalWrite(alert2, LOW);
        lockState=true; 
        lockChart =0;

        lcd.clear();
        delay(500);

     }

}



// Send email

void sendmail(String msg) {

  smtp.debug(1);

  ESP_Mail_Session session;

  session.server.host_name = SMTP_server ;

  session.server.port = SMTP_Port;

  session.login.email = sender_email;

  session.login.password = sender_password;

  session.login.user_domain = "";

  /* Declare the message class */

  SMTP_Message message;

  message.sender.name = "DOOR ALERT";

  message.sender.email = sender_email;

  message.subject = "Alert From Rafi's Automation";

  message.addRecipient(Recipient_name, Recipient_email);


  //Send simple text message

  String textMsg = msg; // "Door has been opened. Thank You."

  message.text.content = textMsg.c_str();

  message.text.charSet = "us-ascii";

  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session))

    return;

  if (!MailClient.sendMail(&smtp, &message))

    Serial.println("Error sending Email, " + smtp.errorReason());
}

void sms (String MSG) {

  delay(500);

  gsm.println("AT+CMGF=1");
  checkSerial();

  gsm.println("AT+CMGS=\"+8801988448287\""); // Set Destination Phone Number
  checkSerial();

  gsm.print(MSG); // Set Message Content
  checkSerial();

  gsm.write(26);

}

void msg4 () {

  delay(500);

  gsm.println("AT+CMGF=1");
  checkSerial();

  gsm.println("AT+CMGS=\"+8801988448287\""); // Set Destination Phone Number
  checkSerial();

  gsm.print("Your Door Lock OTP is : "); // Set Message Content
  gsm.println(mainPassword); // Set Message Content
  checkSerial();

  gsm.write(26);

}



void checkSerial() {

  delay(500); // Used to ensure enough lag time between the at commands

  //  while (Serial.available())
  //    gsm.write(Serial.read());

  while (gsm.available())
    Serial.write(gsm.read());

}

boolean getID()
{
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return false;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    return false;
  }
  tagID = "";
  for ( uint8_t i = 0; i < 4; i++) { // The MIFARE PICCs that we use have 4 byte UID
    //readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA(); // Stop reading
  return true;
}
