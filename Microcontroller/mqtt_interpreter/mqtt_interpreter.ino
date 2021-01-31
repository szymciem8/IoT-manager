#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SIZE 100
#define WORDS_NUMBER 10
#define WORD_LENGTH 10
#define PIN_NUMBER 8

typedef struct{
  char* name; 
  char* group; 
  int pin[PIN_NUMBER]; 
  int state[PIN_NUMBER];   
}info;

info device = {"node1", 
               "test", 
               {D0, D1, D2, D3, D5, D6, D7, D8}}; 

//Array that holds deconstructed command
char command[WORDS_NUMBER][WORD_LENGTH];
char separator = ' '; 

char msg[SIZE];
String read_string;

//WiFi
const char* ssid = "SSID";
const char* wifi_password = "PASSWORD";

//MQTT
const char* mqtt_server = "192.168.0.1";
const char* mqtt_topic = "test";
const char* client_id = "node1";

WiFiClient wifi_client;
PubSubClient client(wifi_client);

//Prototypes of functions 
void interpret_command(String data);
int execute_command();
void clear_command_array();
bool compare_char_arrays(char array1, char array2);
void send_activity_status();
void receive_msg(char* topic, byte* payload, unsigned int length);

//------------------------------------------------------------------//
//-----------------------------SETUP--------------------------------//
//------------------------------------------------------------------//
void setup() {
  Serial.begin(115200);

  for(int i=0; i<PIN_NUMBER; i++){
    device.state[i] = LOW; 
    pinMode(device.pin[i], OUTPUT);
    digitalWrite(device.pin[i], device.state[i]);
    yield();
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, wifi_password);

  //wait for the connection to be established
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();

  //WiFi connection info
  Serial.println("WiFi connected");

  //set MQTT broker 
  client.setServer(mqtt_server, 1883);
  client.setCallback(receive_msg);

  //connect to broker 
  if (client.connect(client_id)){
    Serial.println("Connected to MQTT broker!");  
    client.subscribe("test");
  }
  else{
    Serial.println("Connection to MQTT broker failed");  
  }

  Serial.println("Setup is finished!");

  Serial.println();
  Serial.println("---------------------------------");
  Serial.print("Name: ");
  Serial.println(device.name);
  Serial.print("Group: ");
  Serial.println(device.group);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("---------------------------------");
  Serial.println();
}

//------------------------------------------------------------------//
//-----------------------------LOOP---------------------------------//
//------------------------------------------------------------------//
void loop() {
  //Serial.println("test");
  if (Serial.available() > 0){
    read_string = Serial.readString();
    interpret_command(read_string);
  }

  client.loop();
}

//------------------------------------------------------------------//
//--------------------------FUNCTIONS-------------------------------//
//------------------------------------------------------------------//

//Command construction
//name-action-pin-value
//
//Example commands
//node1-set-5-HIGH
//node1-set-6-LOW
//
//Jak na razie działa jedynie zapis wartości na pinie 
//Jeszcze nie ma odczytu 

void interpret_command(String data){
  int data_size = data.length()+1;
  char char_data[data_size];
  int word_num = 0; 
  int char_num = 0;

  data.toCharArray(char_data, data_size);
  //Serial.println(data.length());
  Serial.println(data);

  for (int i=0; i<data_size; i++){
    //Check if there is a separator
    //Make sure size of each word is within given constraint
    if(char_data[i] != separator){
      command[word_num][char_num] = char_data[i];
      char_num++;
    }
    //If there is a separator go to the next word
    else if (char_data[i] == separator) {
      word_num++;     //Go to the next word
      char_num = 0;
    }
  }

  //Show interpreted command
  /*
  Serial.print("Interpreted Command: ");
  for(int i=0; i<=word_num; i++){
    Serial.print(command[i]);
    if (i != word_num){
      Serial.print(" + ");
    }
  }
  Serial.println();
  */

  execute_command();
  clear_command_array();
}

int execute_command(){
   int step = 0; 
   int which_pin;
   String state_array = "";

   //Check called device's name 
   if (compare_char_arrays(command[step], device.name)){
     step++;

     //---WRITE---
     if (compare_char_arrays(command[step], "write")){
        step++;

        //---ALL---
        //Choose every pin 
        if(compare_char_arrays(command[step], "all")){
            step++;

          //---STATE---
          //Choose state
          //---HIGH---
          if(compare_char_arrays(command[step], "high")){
            for(int i=0; i<PIN_NUMBER; i++){
              digitalWrite(device.pin[i], HIGH);
              device.state[i] = HIGH; 
            }
          }
          //---LOW---
          else if(compare_char_arrays(command[step], "low")){
            for(int i=0; i<PIN_NUMBER; i++){
              digitalWrite(device.pin[i], LOW);
              device.state[i] = LOW; 
            }
          }
          //---PWM---
          else if(compare_char_arrays(command[step], "pwm")){
            step++;
            int pwm = atoi(command[step]);
            for(int i=0; i<PIN_NUMBER; i++){
              analogWrite(device.pin[i], pwm);
              device.state[i] = pwm; 
            }
          }
        }

        //---ONE-PIN---
        else{
          //Find which pin should change state
          which_pin = atoi(command[step]);
          step++;

          //---STATE---
          //Find state
          //---HIGH---
          if(compare_char_arrays(command[step], "high")){
            digitalWrite(device.pin[which_pin], HIGH);
            Serial.print("Pin"); 
            Serial.print(command[step-1]);
            Serial.println(" set to HIGH");
            device.state[which_pin] = HIGH;
          }
          //---LOW---
          else if(compare_char_arrays(command[step], "low")){
            digitalWrite(device.pin[which_pin], LOW);
            Serial.print("Pin"); 
            Serial.print(command[step-1]);
            Serial.println(" set to LOW");
            device.state[which_pin] = HIGH;
          }
          //---PWM---
          else if(compare_char_arrays(command[step], "pwm")){
            step++;
            int pwm = atoi(command[step]);
            analogWrite(device.pin[which_pin], pwm);
            device.state[which_pin] = pwm; 
          }
        }
     }
     
     //---READ-ACTION---
     else if (compare_char_arrays(command[step], "read")){
       step++;

       state_array = "return" + separator + (String) device.name + separator + "all" + separator; 
       //state_array = device.name;
       //int offset = sizeof(state_array)/sizeof(state_array[0]);
       //state_array += "-";

       //---ALL---
       if(compare_char_arrays(command[step], "all")){
         for(int i=0; i<PIN_NUMBER; i++){
          /*
           if(digitalRead(device.pin[i]) == HIGH){
             state_array += "1";
           }
           else if(digitalRead(device.pin[i]) == LOW){
             state_array += "0";
           }
          */
          state_array += String(device.state[i]);
          state_array += ".";
         }
         char char_array[state_array.length()+1]; 
         state_array.toCharArray(char_array, state_array.length()+1);
         Serial.println(state_array);
         client.publish("test", char_array);
       }

       //---ONE-PIN---
       else{
         //Find which pin to check
         which_pin = atoi(command[step]);

         //---STATE---
         if(digitalRead(device.pin[which_pin]) == HIGH){
            client.publish("test", "HIGH");
         }
         else if(digitalRead(device.pin[which_pin]) == LOW){
            client.publish("test", "LOW");
         }
       }
     }

     //IFONLINE
     else if(compare_char_arrays(command[step], "ifonline")){
      
     }

     //CONFIGURE
     else if(compare_char_arrays(command[step], "configure")){
      step++;

      if(compare_char_arrays(command[step], "group")){
        step++;
        device.group = command[step];
        Serial.println("Group has been updated!");
      }
      else if(compare_char_arrays(command[step], "name")){
        step++;
        device.name = command[step];
        Serial.print("Name has been updated to ");
        Serial.print(device.name);
        Serial.println("!");
      }
      
     }
     else{
       Serial.println("Your command was incorrect - choose the right action!"); 
     }
   }
}

//Clears command array 
void clear_command_array(){
    for(int i=0; i<WORDS_NUMBER; i++){
      for(int j=0; j<WORD_LENGTH; j++){
        command[i][j] = NULL;  
      }  
    }
}

//Function that compares two char arrays
bool compare_char_arrays(char* array1, char* array2){
  bool the_same = false;
  //Calculate number of elements
  int size1 = sizeof(array1)/sizeof(array1[0]);
  int size2 = sizeof(array2)/sizeof(array2[0]);

  if (size1 != size2) return false;

  for (int i=0; i<size1; i++){
    if (array1[i] == array2[i]){
      the_same = true;
    }
    else{
      return false;  
    }
  }
  return the_same;
}

void receive_msg(char* topic, byte* payload, unsigned int length){
  String mqtt_command = "";
  for(int i = 0; i < length; i ++)
  {
    //Serial.print(char(payload[i]));
    mqtt_command += char(payload[i]);
  }
 
  interpret_command(mqtt_command);
}

