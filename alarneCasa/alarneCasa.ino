

hw_timer_t *timer = NULL;

#include <WiFi.h>
#include <PubSubClient.h>
#define sensorArea   23
#define sensorQuarto 32
#define Saida1 17


const char* ssid="TP-LINK_6B4E9E";
const char* password= "edicula88";
const char* mqtt_server = "m15.cloudmqtt.com";
const char* mqtt_user = "xinuxccp";
const char* mqtt_pass = "bWpTqG0Nh5GX";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];

String sensor_quarto="";
String statusAlarme="";
char valueStr[20];
char valueStr2[20];
int buzina =0;
int alarme =0,flagAlarme=0,flag_buzina_liga=0;


//variaveis que indicam o núcleo
static uint8_t taskCoreZero = 0;
static uint8_t taskCoreOne  = 1;


//função que o temporizador irá chamar, para reiniciar o ESP32
void IRAM_ATTR resetModule(){
    ets_printf("(watchdog) reiniciar\n"); //imprime no log
    esp_restart_noos(); //reinicia o chip
}

void configureWatchdog(){
  
  //timerID 0, div 80 (clock do esp), contador progressivo
  timer = timerBegin(0, 80, true);
  
  //instancia de timer, função callback, interrupção de borda
  timerAttachInterrupt(timer, &resetModule, true);

  // instancia de timer, tempo (us),3.000.000 us = 3 segundos , repetição
  timerAlarmWrite(timer, 3000000, true);

  timerAlarmEnable(timer); //habilita a interrupção
   
}

void setup_wifi()
{
  delay(100);
//  Serial.println();
//  Serial.print("Connecting to ");
//  Serial.print(ssid);
  WiFi.begin(ssid,password);
  
  while(WiFi.status()!= WL_CONNECTED){
    delay(500);
   Serial.print("....");
  }

randomSeed(micros());

 timer = timerBegin(0, 80, true); //timerID 0, div 80
 //timer, callback, interrupção de borda
 timerAttachInterrupt(timer, &resetModule, true);
 //timer, tempo (us), repetição
 timerAlarmWrite(timer, 3000000, true);
 timerAlarmEnable(timer); //habilita a interrupção 

//Serial.println("");
//Serial.println("WiFi connected");
//Serial.println("IP address: ");
//Serial.println(WiFi.localIP());

 
}

void reconnect()
{
  //Loop until we are reconnected
  while(!client.connected())
  {
//    Serial.print("Attempting MQTT connection...");
    //Create a ramdom client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    //Attempt to connect
    if(client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
    {
    
//      Serial.println("connected");
      client.publish("outTopic","hello world");
      //... and resubscribe
      client.subscribe("alarmeCasa");
      
          
    }else{
//       Serial.println("falied, rc=");
//       Serial.print(client.state());
//       Serial.println("try again in 5 seconds");
       delay(5000);
          
     }
  }
}
void callback(char* topic, byte* payload, unsigned int length)
{
  
  char PAYLOAD[10] = "";
//  Serial.print("Message arrived[");
//  Serial.print(topic);
//  Serial.print("]");
  
  for(int i = 0; i < length; i++){
    PAYLOAD[i] = (char)payload[i];
  }
//  Serial.println(PAYLOAD);

  if (String(topic) ==  "alarmeCasa") 
  {
    if (payload[0] == 'l' && payload[1] == 'i' && payload[2] == '3' && payload[3] == '2' && payload[4] == '1')
    {
      alarme = 1;
      sensor_quarto = "OK";
    }
    if (payload[0] == 'd' && payload[1] == 'e' && payload[2] == '3' && payload[3] == '2' && payload[4] == '1')
    {
      sensor_quarto = "inoperante";
      alarme =0;
      flagAlarme =0;
    }

  }
 
}

void setup() {
  Serial.begin(9600);
  pinMode(sensorArea,INPUT);
  pinMode(sensorQuarto,INPUT_PULLUP);
  digitalWrite(sensorArea,LOW);
  
  pinMode(Saida1, OUTPUT);
  digitalWrite(Saida1,LOW);

   //cria uma tarefa que será executada na função coreTaskZero, com prioridade 1 e execução no núcleo 0
  //coreTaskZero: piscar LED e contar quantas vezes
  xTaskCreatePinnedToCore(
                    coreTaskZero,   /* função que implementa a tarefa */
                    "coreTaskZero", /* nome da tarefa */
                    10000,      /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    NULL,       /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    2,          /* prioridade da tarefa (0 a N) */
                    NULL,       /* referência para a tarefa (pode ser NULL) */
                    taskCoreZero);         /* Núcleo que executará a tarefa */
                    
  delay(500); //tempo para a tarefa iniciar

  //cria uma tarefa que será executada na função coreTaskOne, com prioridade 2 e execução no núcleo 1
  //coreTaskOne: atualizar as informações do display
  xTaskCreatePinnedToCore(
                    coreTaskOne,   /* função que implementa a tarefa */
                    "coreTaskOne", /* nome da tarefa */
                    10000,      /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    NULL,       /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    1,          /* prioridade da tarefa (0 a N) */
                    NULL,       /* referência para a tarefa (pode ser NULL) */
                    taskCoreOne);         /* Núcleo que executará a tarefa */

    delay(500); //tempo para a tarefa iniciar
  
}

void loop() {
 
//  if(digitalRead(sensorArea) == HIGH)
//  {
//    Serial.println("o pino esta em nivel logico alto");
//    
//  }
//  else
//  {
//   Serial.println("o pino esta em nivel logico baixo");
//   
//  }
 
}

void coreTaskZero( void * pvParameters )
{
 
    while(true)
    {
    
     //Ligou Alarme?
     if(alarme == 1)
     {
        //soa 2 toques buzina ao ligar
        if(flag_buzina_liga == 0)
        {
          toque_buzina();
          flag_buzina_liga = 1;   
        }
        
        statusAlarme = "Ligado";
        if(digitalRead(sensorQuarto) == HIGH)
        {
          //Serial.println("o pino esta em nivel logico alto");
          
          buzina =1;
          ControlaBuzina();
          sensor_quarto = "quartoInvadido";
           
        }
        else
        {
         //Serial.println("o pino esta em nivel logico baixo");
         
         if(flagAlarme = 0)  
          sensor_quarto = "OK";
        }
     }
     
     //Alarme esta desligado?
     if(alarme == 0){
       statusAlarme = "Desligado";
       sensor_quarto = "inoperante";
       buzina =0;
       ControlaBuzina();

        //o alarme estava ligado?
        if(flag_buzina_liga == 1){
          toque_buzina();
          flag_buzina_liga = 0;
        }
       
     }
   
    
     
      
     delay(300);
     timerWrite(timer, 0); //reseta o temporizador (alimenta o watchdog) 
    } 
}


void coreTaskOne( void * pvParameters )
{
     
     setup_wifi();
     client.setServer(mqtt_server,13750);
     client.setCallback(callback);
     
     while(true)
     {
        if(!client.connected())
        {
           reconnect();
        }
  
        client.loop();
        sensor_quarto.toCharArray(valueStr,20);
        statusAlarme.toCharArray(valueStr2,20);
        client.publish("sensorQuarto",valueStr);
        client.publish("statusAlarme",valueStr2);
             
  
        delay(1000);  
     } 
}

void ControlaBuzina()
{
  
  if(buzina == 1 )
  {
    flagAlarme =1;
    digitalWrite(Saida1,HIGH);
  }
  if(buzina == 0)
  {
    digitalWrite(Saida1,LOW);
  }
 
}

void toque_buzina(){
  
  digitalWrite(Saida1,HIGH);
  delay(1000);
  digitalWrite(Saida1,LOW);
  delay(1000);
  digitalWrite(Saida1,HIGH);
  delay(1000);
  digitalWrite(Saida1,LOW);
}

