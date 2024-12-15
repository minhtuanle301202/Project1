#include <Servo.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);


int servo = A0;
Servo myServo;
const byte ROWS = 4;  // 4 hàng  trong bàn phím
const byte COLS = 4;  // 4 cột trong bàn phím

// Khai bao cac chan su dung cho module RFID-RC522
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);

byte UID1[4] = { 0x73, 0x37, 0x97, 0x2F };
byte UID[4];

int numberCard = 1;
byte card[4][4] = {
  { 0x73, 0x37, 0x97, 0x2F }
};

// ma trận bàn phím
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 7, 6, 5, 4 };    // Chân của các hàng
byte colPins[COLS] = { 3, 2, A1, A2 };  // Chân của các hàng

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
char customKey;


char password[6] = "1234";
char newPass[6];
char current_pass[6];

void init_password() {
  if (EEPROM.read(0) == 255) {
    for (int i = 0; i < 4; i++) {
      EEPROM.write(i, password[i]);
    }
  } else {
    for (int i = 0; i < 4; i++) {
      password[i] = EEPROM.read(i);
    }
  }
  return;
}


void lcd_line_first(String str) {
  int startPosition = (16 - str.length()) / 2;
  lcd.setCursor(startPosition, 0);
  lcd.print(str);
  return;
}

void lcd_line_second(String str) {
  int startPosition = (16 - str.length()) / 2;
  lcd.setCursor(startPosition, 1);
  lcd.print(str);
  return;
}

// Trang thai ban đầu
void start_stage() {
  lcd.clear();
  lcd_line_first("SELECT MODE");
  lcd_line_second("1.PASSWORD2.CARD");
  return;
}


// Chế độ dùng thẻ
void mode_card() {
  lcd.clear();
  lcd_line_first("1.USE CARD");
  lcd_line_second("2.MANAGE CARD");
  char key = '\0';  // Khởi tạo giá trị mặc định cho key
  while (!key) {    // Lặp cho đến khi nhận được giá trị từ keypad
    key = customKeypad.getKey();
  }
  if (key) {
    if (key == '1') {
      mode_use_card();
      return;
    } else if (key == '2') {
      mode_manage_card();
      return;
    }
  }
}

void mode_use_card() {
  lcd.clear();
  lcd_line_first("WELCOME !!!");
  lcd_line_second("PUT YOUR CARD");
  read_card();
  int find_card = 0;
  for (int i = 0; i < numberCard; i++) {
    if (strncmp(UID, card[i], 4) == 0) {
      find_card = 1;
    }
  }
  if (find_card == 0) {
    lcd.clear();
    lcd_line_first("WRONG CARD");
    lcd_line_second("TRY AGAIN");
    delay(2000);

  } else {
    lcd.clear();
    lcd_line_first("CORRECT CARD");
    lcd_line_second("OPEN DOOR");

    myServo.write(100);
    delay(4000);
    myServo.write(-100);
    delay(500);
  }
  return;
}


void mode_manage_card() {
  lcd.clear();
  lcd_line_first("1.ADD CARD");
  lcd_line_second("2.REMOVE CARD");

  char key = '\0';  // Khởi tạo giá trị mặc định cho key
  while (!key) {    // Lặp cho đến khi nhận được giá trị từ keypad
    key = customKeypad.getKey();
  }

  if (key == '1') {
    mode_add_card();
  } else if (key == '2') {
    mode_remove_card();
  }

  return;
}

// Che do them the
void mode_add_card() {
  do {
    int j = 0;
    while (j < 2) {
      lcd.clear();
      lcd_line_first("ENTER PASSWORD");
      lcd_line_second("[____]");
      type_current_pass();
      if (compare_pass(password, current_pass)) {
        add_card();
        break;
      } else {
        wrong_password();
        j++;
      }
    }
    if (j == 2) {
      j = 0;
      int count = 5;
      while (count >= 0) {
        time_lock(count);
        count--;
      }
    }
  } while (!compare_pass(password, current_pass));
  return;
}

void add_card() {
  lcd.clear();
  lcd_line_first("ADD CARD MODE");
  lcd_line_second("PUT YOUR CARD");
  read_card();
  int card_exist = 0;
  for (int i = 0; i < numberCard; i++) {
    if (strncmp(UID, card[i], 4) == 0) {
      card_exist = 1;
      break;
    }
  }
  if (card_exist) {
    lcd.clear();
    lcd_line_first("CARD");
    lcd_line_second("ALREADY EXISTS");
    delay(2000);
  } else {
    numberCard++;
    for (int i = 0; i < 4; i++) {
      card[numberCard - 1][i] = UID[i];
    }

    lcd.clear();
    lcd_line_first("CARD ADDED");
    delay(2000);
  }
  return;
}

void mode_remove_card() {
  do {
    int j = 0;
    while (j < 2) {
      lcd.clear();
      lcd_line_first("ENTER PASSWORD");
      lcd_line_second("[____]");
      type_current_pass();
      if (compare_pass(password, current_pass)) {
        remove_card();
        break;
      } else {
        wrong_password();
        j++;
      }
    }
    if (j == 2) {
      j = 0;
      int count = 5;
      while (count >= 0) {
        time_lock(count);
        count--;
      }
    }
  } while (!compare_pass(password, current_pass));
  return;
}

void remove_card() {
  lcd.clear();
  lcd_line_first("REMOVE CARD MODE");
  lcd_line_second("PUT YOUR CARD");
  read_card();
  int card_exist = 0;
  for (int i = 0; i < numberCard; i++) {
    if (strncmp(UID, card[i], 4) == 0) {
      card_exist = 1;
      break;
    }
  }
  if (card_exist == 0) {
    lcd.clear();
    lcd_line_first("CARD");
    lcd_line_second("NOT EXISTS");
    delay(2000);
  } else {
    int position_remove = 0;
    for (int i = 0; i < numberCard; i++) {
      if (strncmp(card[i], UID, 4) == 0) {
        position_remove = i;
        break;
      }
    }

    if (position_remove == numberCard - 1) {
      numberCard--;
    } else {
      for (int i = position_remove; i < numberCard - 1; i++) {
        for (int j = 0; i < 4; j++) {
          card[i][j] = card[i + 1][j];
        }
      }
      numberCard--;
    }

    lcd.clear();
    lcd_line_first("CARD REMOVED");
    delay(2000);
  }
  return;
}

void read_card() {
  int readed = 0;
  while (readed == 0) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      lcd.clear();
      String str3 = "Scanning";
      lcd.setCursor(0, 0);
      lcd.print(str3);
      Serial.print("UID của thẻ: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        UID[i] = mfrc522.uid.uidByte[i];
        Serial.print(UID[i], HEX);
        lcd.print(".");
        delay(300);
      }
      Serial.println("");
      readed = 1;
    }
  }
  return;
}

//Lựa chọn chế độ dùng mật khẩu
void mode_password() {
  lcd.clear();
  lcd_line_first("1.USE PASSWORD");
  lcd_line_second("2.CHANGE PASSWORD");

  char key = '\0';  // Khởi tạo giá trị mặc định cho key
  while (!key) {    // Lặp cho đến khi nhận được giá trị từ keypad
    key = customKeypad.getKey();
  }


  if (key == '1') {
    mode_use_password();
  } else if (key == '2') {
    mode_change_password();
  }

  return;
}

bool compare_pass(char *oldpass, char *newpass) {
  for (int i = 0; i < 4; i++) {
    if (oldpass[i] != newpass[i]) {
      return false;  // Trả về ngay khi phát hiện không khớp
    }
  }
  return true;  // Nếu vòng lặp kết thúc, nghĩa là tất cả khớp
}

// Chế độ sử dụng mật khẩu để mở cửa
void mode_use_password() {
  do {
    int j = 0;
    while (j < 2) {
      lcd.clear();
      lcd_line_first("ENTER PASSWORD");
      lcd_line_second("[____]");
      type_current_pass();
      if (compare_pass(password, current_pass)) {
        correct_password();
        break;
      } else {
        wrong_password();
        j++;
      }
    }
    if (j == 2) {
      j = 0;
      int count = 5;
      while (count >= 0) {
        time_lock(count);
        count--;
      }
    }
  } while (!compare_pass(password, current_pass));
  return;
}

// Nhập đúng mật khẩu
void correct_password() {
  lcd.clear();
  lcd_line_first("PASS ACCCEPTED");
  lcd_line_second("OPEN DOOR");
  myServo.write(100);
  delay(3000);
  myServo.write(-100);
  delay(500);
  return;
}

void wrong_password() {
  lcd.clear();
  lcd_line_first("WRONG PASSWORD");
  lcd_line_second("TRY AGAIN");
  delay(1500);
  return;
}

//Chế độ đổi mật khẩu
void mode_change_password() {
  bool comparePass;
  do {
    int j = 0;
    while (j < 2) {
      lcd.clear();
      lcd_line_first("TYPE CURRENTPASS");
      lcd_line_second("[____]");
      type_current_pass();
      comparePass = compare_pass(password, current_pass);
      if (comparePass) {
        type_new_pass();
        break;
      } else {
        wrong_password();
        j++;
      }
    }
    if (j == 2) {
      j = 0;
      int count = 5;
      while (count >= 0) {
        time_lock(count);
        count--;
      }
    }
  } while (!comparePass);
  return;
}

// Nhập mật khẩu hiện tại
void type_current_pass() {
  int j = 0;
  int position = 5;
  while (j < 4) {
    char key = customKeypad.getKey();
    if (key) {
      current_pass[j++] = key;
      lcd.setCursor(++position, 1);
      lcd.print("*");
    }
  }
  delay(200);
  return;
}

// Nhập mật khẩu mới
void type_new_pass() {
  lcd.clear();
  lcd_line_first("TYPE NEWPASS");
  lcd_line_second("[____]");
  int j = 0;
  int position = 5;
  while (j < 4) {
    char key = customKeypad.getKey();
    if (key) {
      password[j++] = key;
      lcd.setCursor(++position, 1);
      lcd.print("*");
    }
  }
  for (int i = 0; i < 4; i++) {
    EEPROM.write(i, password[i]);
  }
  delay(200);
  lcd.clear();
  lcd_line_first("CHANGE PASSWORD");
  lcd_line_second("SUCCESSFULLY");
  delay(1500);
  return;
}



// Đếm ngược thời gian nếu như nhập quá thời gian quy định
void time_lock(int t) {
  String time, hour, minute, second;
  int h = t / 3600;
  if (h > 0) {
    t = t - h * 3600;
  }

  int m = t / 60;
  if (m > 0) {
    t = t - m * 60;
  }

  int s = t;
  lcd.clear();
  hour = String(h);
  minute = String(m);
  second = String(s);
  if (h < 10) {
    hour = "0" + hour;
  }
  if (m < 10) {
    minute = "0" + minute;
  }
  if (t < 10) {
    second = "0" + second;
  }
  time = hour + ":" + minute + ":" + second;
  lcd.clear();
  int startPosition = (16 - time.length()) / 2;
  lcd.setCursor(startPosition, 0);
  lcd.print(time);
  delay(1000);
}

void setup() {
  Serial.begin(9600);
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card
  myServo.attach(servo);
  lcd.init();
  lcd.backlight();
  init_password();
  start_stage();
  Serial.println(EEPROM.read(0));
}

void loop() {
  // put your main code here, to run repeatedly:
  start_stage();

  customKey = '\0';     // Khởi tạo giá trị mặc định cho key
  while (!customKey) {  // Lặp cho đến khi nhận được giá trị từ keypad
    customKey = customKeypad.getKey();
  }

  if (customKey) {
    if (customKey == '1') {
      mode_password();
    } else if (customKey == '2') {
      mode_card();
    }
  }
}
