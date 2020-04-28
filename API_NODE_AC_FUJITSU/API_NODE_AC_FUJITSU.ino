/*=============================================================================
               Código baseado no blog do Antonio Mancuso
  https://mancusoa74.blogspot.com/2018/02/simple-http-rest-server-on-esp8266.html
                       Grazie! - Thank You - Obrigado!
  Este programa é um software livre; você pode redistribuí-lo e/ou
  modificá-lo sob os termos da Licença Pública Geral GNU como publicada
  pela Free Software Foundation; na versão 3 da Licença, ou
  (a seu critério) qualquer versão posterior.
  Este programa é distribuído na esperança de que possa ser útil,
  mas SEM NENHUMA GARANTIA; sem uma garantia implícita de ADEQUAÇÃO
  a qualquer MERCADO ou APLICAÇÃO EM PARTICULAR. Veja a
  Licença Pública Geral GNU para mais detalhes.
  Você deve ter recebido uma cópia da Licença Pública Geral GNU junto
  com este programa. Se não, veja <http://www.gnu.org/licenses/>.
  ===============================================================================*/

#include <stdio.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

//Includes para funcoes do AC Fujitsu
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Fujitsu.h>

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

//Informa SSID e senha Wi-fi
const char* wifi_ssid = "SSID";
const char* wifi_passwd = "PASS";
//Fixar IP
#define IP "192.168.1.111"
#define GATEWAY "192.168.1.1"
#define SUBNET "255.255.255.0"

//Define o tamanho máximo do array - deverá ser a quantidade de pinos disponíveis
int size_array = 14;
int val_array = 1;
int qtd_items = 0;

//Inicializacao de pino de envio IR
const uint16_t kIrLed = 14;  // ESP8266 GPIO pin to use. Recommended: 14 (D5).
IRFujitsuAC ac(kIrLed);
bool ligado = false;





//ESP8266WebServer http_rest_server(HTTP_REST_PORT);
ESP8266WebServer http_rest_server(8080);

//Estrutura do led: modificar para ser uma estrutura de GPIO (entrada e saída)
typedef struct {
  int id;
  byte gpio;
  String pinMod;
  int potencia;
  byte status;
} Led;


//Estrutura do AC: informar sempre os parâmetros atuais de funcionamento do AC
typedef struct {
  String model;
  String power;
  String modeFunc;
  int temperature;
  String fan;
  String clean;
  String filter;
  String swing;
  String command;
  String quiet;
} AC;



AC ac_resource;

Led led_resource[14];


//Estrutura para definir os possíveis pinos que podem ser utilizados
typedef struct {
  byte gpio;
  byte situacao;
} GpioLivre;

GpioLivre gpioLivre[13];

void config_rest_server_routing() {



  http_rest_server.on("/", HTTP_GET, []() {

    //html da página principal
    String html = "<!DOCTYPE html><html><head><base href=\"https://html-online.com/editor/\"><link type=\"text/css\" rel=\"stylesheet\" href=\"https://html-online.com/editor/tinymce4_6_5/skins/lightgray/content.min.css\"></head><body id=\"tinymce\" class=\"mce-content-body \"><h1 style=\"color: #5e9ca0;\">Bem-vindo a API de utiliza&ccedil;&atilde;o do ESP8233 (NodeMCU)!</h1>";
    html += "<h2 style=\"color: #2e6c80;\">O que esta API faz?</h2>";
    html += "<p>Atrav&eacute;s dela, voc&ecirc; pode acionar as GPIOs do ESP8233 (NodeMCU), alterando os status para ligado ou desligado.</p>";
    html += "<p><img style=\"display: block; margin-left: auto; margin-right: auto;\" src=\"http://www.electronicwings.com/public/images/user_images/images/NodeMCU/NodeMCU%20Basics%20using%20ESPlorer%20IDE/NodeMCU%20GPIO/NodeMCU%20GPIOs.png\" alt=\"GPIOs\" width=\"321\" height=\"363\" /></p>";
    html += "<p>GPIOs em azul n&atilde;o podem ser utiizadas ou n&atilde;o s&atilde;o recomendadas para utiliza&ccedil;&atilde;o direta.</p>";
    html += "<h2 style=\"color: #2e6c80;\">Opera&ccedil;&otilde;es:</h2>";
    html += "<table class=\"editorDemoTable\" style=\"width: 812px;\">";
    html += "<thead>";
    html += "<tr>";
    html += "<td style=\"width: 69px;\">Verbo HTTP</td>";
    html += "<td style=\"width: 727px;\">Detalhamento</td>";
    html += "</tr>";
    html += "</thead>";
    html += "<tbody>";
    html += "<tr>";
    html += "<td style=\"width: 69px;\">POST</td>";
    html += "<td style=\"width: 727px;\">";
    html += "<p>Para inicializar a GPIO, os seguintes par&acirc;metros dever&atilde;o ser informados:</p>";
    html += "<p>GPIO: (preencher como \"gpio\"): indicar qual GPIO voc&ecirc; deseja utilizar entre as dispon&iacute;veis (2,4,5,12,13,14,15,16). Outros valores n&atilde;o s&atilde;o aceitos!</p>";
    html += "<p>Modo de entrada (n&atilde;o &eacute; obrigat&oacute;rio neste momento - preencher como \"pinMod\"): indicar ser&aacute; do tipo anal&oacute;gica ou digital, sendo que o default &eacute; DIGITAL. Caso queira mudar para anal&oacute;gico, obrigatoriamente voc&ecirc; dever&aacute; escrever ANALOG e qualquer outro valor &eacute; considerado DIGITAL.</p>";
    html += "<p>No caso de uma sa&iacute;da anal&oacute;gica, o par&acirc;metro \"potencia\" deve ser um valor entre 0 a 1023. Valores acima de 1023 s&atilde;o assumidos automaticamente como 1023. Quando o status estiver marcado como 0, automaticamente a API converte a pot&ecirc;ncia para 0.</p>";
    html += "<p>O Id &eacute; gerado automaticamente.</p>";
    html += "<p>JSON de request:</p>";
    html += "<p>{<br /> \"gpio\":12<br /> }</p>";
    html += "<p>JSON de response:</p>";
    html += "<p>{<br />\"id\": 1,<br />\"gpio\": 12,<br />\"pinMod\": \"\",<br />\"potencia\": 0,<br />\"status\": 0<br />}</p>";
    html += "<p>C&oacute;digos de resposta:</p>";
    html += "<p>201 (Created) - Request respondido com sucesso.</p>";
    html += "<p>400 (Bad request) - Erro na estrutura do JSON</p>";
    html += "<p>404 (Not found) - GPIO informada n&atilde;o est&aacute; na lista de op&ccedil;&otilde;es v&aacute;lidas</p>";
    html += "</td>";
    html += "</tr>";
    html += "<tr>";
    html += "<td style=\"width: 69px;\">PUT</td>";
    html += "<td style=\"width: 727px;\">";
    html += "<p>As GPIOs criadas via POST poder&atilde;o ter seu status (ligada ou desligada) atualizados por este m&eacute;todo. Para isso, o par&acirc;metro \"status\" deve ser respondido com 0 ou 1.</p>";
    html += "<p>No PUT &eacute; poss&iacute;vel alterar os par&acirc;metros:</p>";
    html += "<ul>";
    html += "<li>\"pinMod\": altera&ccedil;&atilde;o entre anal&oacute;gico (\"ANALOG\") e digital (\"DIGITAL\");</li>";
    html += "<li>\"potencia\": quando a configura&ccedil;&atilde;o &eacute; anal&oacute;gica, altera o total da tens&atilde;o no pino entre 0 (m&iacute;nimo) a 1023 (m&aacute;ximo)</li>";
    html += "<li>\"status\": o valor 0 desliga a GPIO e o valor 1 a liga</li>";
    html += "</ul>";
    html += "<p>O ID &eacute; obrigat&oacute;rio na opera&ccedil;&atilde;o, e n&atilde;o &eacute; poss&iacute;vel alterar a GPIO deste pino uma vez que ela &eacute; criada (s&oacute; pode haver um ID para cada GPIO dispon&iacute;vel).</p>";
    html += "<p>JSON de request:</p>";
    html += "<p>{<br />\"id\": 1,<br />\"pinMod\": \"ANALOG\",<br />\"potencia\": 1023,<br />\"status\": 1<br />}</p>";
    html += "<p>JSON de response:</p>";
    html += "<p>{<br />\"id\": 1,<br />\"gpio\": 12,<br />\"pinMod\": \"ANALOG\",<br />\"potencia\": 1023,<br />\"status\": 1<br />}</p>";
    html += "<p>C&oacute;digos de resposta:</p>";
    html += "<p>200 (OK) - Atualizado com sucesso</p>";
    html += "<p>400 (Bad request) - Erro na estrutura do JSON</p>";
    html += "<p>404 (Not found) - ID informado n&atilde;o existe</p>";
    html += "</td>";
    html += "</tr>";
    html += "<tr>";
    html += "<td style=\"width: 69px;\">GET</td>";
    html += "<td style=\"width: 727px;\">";
    html += "<p>Existem duas formas de chamar o m&eacute;todo GET:&nbsp;</p>";
    html += "<ul>";
    html += "<li>Por lista</li>";
    html += "<li>Por query/par&acirc;metros</li>";
    html += "</ul>";
    html += "<p>Ao informar o endere&ccedil;o local (http://xxx.xxx.xxx.xxx/leds), a API ir&aacute; mostrar a lista de GPIOs criadas com sua parametriza&ccedil;&atilde;o atual:</p>";
    html += "<p>JSON de response:</p>";
    html += "<p>[<br />{<br />\"id\": 1,<br />\"gpio\": 2,<br /> \"pinMod\": \"OUTPUT\",<br /> \"status\": 1<br /> },<br /> {<br /> \"id\": 2,<br /> \"gpio\": 4,<br /> \"pinMod\": \"OUTPUT\",<br /> \"status\": 1<br /> }<br />]</p>";
    html += "<p>&nbsp;</p>";
    html += "<p>J&aacute; quando &eacute; passado o par&acirc;metro id&nbsp;&nbsp;(http://xxx.xxx.xxx.xxx/leds?id=1), ele retorna apenas o elemento consultado, caso exista:</p>";
    html += "<p>C&oacute;digos de resposta:</p>";
    html += "<p>200 (OK) - Lista retornada com sucesso</p>";
    html += "<p>204 (No content) - Lista vazia de elementos: n&atilde;o existe nenhuma GPIO criada</p>";
    html += "<p>404 (Not found) - ID informado n&atilde;o existe</p>";
    html += "</td>";
    html += "</tr>";
    html += "</tbody>";
    html += "</table>";
    html += "<p>&nbsp;</p>";
    html += "<p>Douglas Melo - 12/2018 - douglasnogueiram@gmail.com</body></html>";
    http_rest_server.send(200, "text/html", html);
  });
  //Itens relacionados a API padrão
  http_rest_server.on("/leds", HTTP_GET, consulta_leds);
  http_rest_server.on("/leds", HTTP_POST, post_leds);
  http_rest_server.on("/leds", HTTP_PUT, put_leds);

  //Acionamento Ar Condicionado Fujitsu
  //Liga Ar Condicionado
  http_rest_server.on("/ac/liga", HTTP_POST, liga_ac);
  //Desliga Ar Condicionado
  http_rest_server.on("/ac/desliga", HTTP_POST, desliga_ac);
  //Define temperatura do Ar Condicionado, passando a temperatura entre 16 e 30 (Celsius)
  http_rest_server.on("/ac/temp", HTTP_POST, setTemp_ac);
  //Consulta todos os parâmetros do Ar Condicionado
  http_rest_server.on("/ac/", HTTP_GET, consulta_param_ac);
  //Liga/desliga Swing
  http_rest_server.on("/ac/swing/", HTTP_POST, setSwing_ac);
  //Define modo do Fan entre 0 a 4
  http_rest_server.on("/ac/mode/", HTTP_POST, setMode_ac);
    //Define velocidade do Fan entre 0 a 4
  http_rest_server.on("/ac/fan/", HTTP_POST, setFan_ac);

}


void setup(void) {
  Serial.begin(115200);

  init_led_resource();
  if (init_wifi() == WL_CONNECTED) {
    Serial.print("Connetted to ");
    Serial.print(wifi_ssid);
    Serial.print("--- IP: ");
    Serial.println(WiFi.localIP());

        //Configurações da rede
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    ip.fromString(IP);
    gateway.fromString(GATEWAY);
    subnet.fromString(SUBNET);
    
    Serial.println(ip);

    //Envia para o roteador as configurações que queremos para o ESP
    WiFi.config(ip, gateway, subnet);

    Serial.println(WiFi.localIP());
    
  }
  else {
    Serial.print("Error connecting to: ");
    Serial.println(wifi_ssid);
  }

  config_rest_server_routing();

  http_rest_server.begin();
  Serial.println("HTTP REST Server Started");

  //Inicialização especifica para AC
  ac.begin();
  delay(200);
  Serial.println("Estado padrão para o controle do AC Fujitsu");

  //Incialização de parâmetros AC
  ac.setModel(ARREB1E);
  //ac.setSwing(kFujitsuAcSwingVert);
  //ac.setMode(kFujitsuAcModeCool);
  //ac.setFanSpeed(kFujitsuAcFanAuto);

}

void loop(void) {
  http_rest_server.handleClient();
}




void init_gpio_livre() {

  //Indisponível
  gpioLivre[0].gpio = 0;
  gpioLivre[0].situacao = 1;

  //Indisponível
  gpioLivre[1].gpio = 1;
  gpioLivre[1].situacao = 1;

  gpioLivre[2].gpio = 2;
  gpioLivre[2].situacao = 0;

  //Indisponível
  gpioLivre[3].gpio = 3;
  gpioLivre[3].situacao = 1;

  gpioLivre[4].gpio = 4;
  gpioLivre[4].situacao = 0;

  gpioLivre[5].gpio = 5;
  gpioLivre[5].situacao = 0;

  //Indisponível
  gpioLivre[6].gpio = 9;
  gpioLivre[6].situacao = 1;

  //Indisponível
  gpioLivre[7].gpio = 10;
  gpioLivre[7].situacao = 1;

  gpioLivre[8].gpio = 12;
  gpioLivre[8].situacao = 0;

  gpioLivre[9].gpio = 13;
  gpioLivre[9].situacao = 0;

  //Indisponível, para o emissor de IR
  gpioLivre[10].gpio = 14;
  gpioLivre[10].situacao = 1;

  gpioLivre[11].gpio = 15;
  gpioLivre[11].situacao = 0;

  gpioLivre[12].gpio = 16;
  gpioLivre[12].situacao = 0;

}




void init_led_resource()
{
  Serial.println("Inicializando GPIOs...");
  for (int i = 0; i < size_array; i++) {

    //Inicializa o array de led
    led_resource[i].id = 0;
    led_resource[i].gpio = 0;
    led_resource[i].pinMod = "OUTPUT";
    led_resource[i].potencia = 0;
    led_resource[i].status = LOW;

  }
  Serial.println("GPIOs OK!");

  //Marca todos as GPIOs como disponíveis
  Serial.println("Marcando todas GPIOs como disponíveis");
  init_gpio_livre();
}



int init_wifi() {
  int retries = 0;

  Serial.println("Connecting to WiFi AP..........");

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_passwd);
  // check the status of WiFi connection to be WL_CONNECTED
  while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
    retries++;
    delay(WIFI_RETRY_DELAY);
    Serial.print("#");
  }
  return WiFi.status(); // return the WiFi connection status
}


void consulta_leds() {

  //Verifica se foram informados argumentos/query de busca na URI
  if (http_rest_server.args() > 0) {

    Serial.println("Consulta por query ");
    Serial.print("Quantidade de argumentos: ");
    Serial.println(http_rest_server.args());
    get_leds();

  } else {
    Serial.print("Consulta por lista ");
    get_lista_leds();

  }


}


void get_lista_leds() {

  //Inicialmente verifica se existem elementos gerados
  if (qtd_items == 0) {

    http_rest_server.send(204);
    Serial.print("Operação get em lista. Lista vazia de elementos. Quantidade de elementos: ");
    Serial.println(qtd_items);
  } else {
    Serial.print("Operação get em lista. Quantidade de elementos a exibir: ");
    Serial.println(qtd_items);

    //Define o buffer do array json e o cria
    StaticJsonBuffer<1350> jsonBuffer;
    char JSONmessageBuffer[900];

    //Cria um objeto raiz
    JsonObject& rootObject = jsonBuffer.createObject();

    //Cria um array para a consulta em lista
    JsonArray& arr = rootObject.createNestedArray("arr");

    //Para gerar a lista de itens, itera a quantidade máxima do array
    for (int i = 1; i < size_array; i++) {

      //Se o id do recurso é diferente de zero (não foi populado) não inclui
      if (led_resource[i].id != 0) {
        JsonObject& led = arr.createNestedObject();
        led["id"] = led_resource[i].id;
        led["gpio"] = led_resource[i].gpio;
        led["pinMod"] = led_resource[i].pinMod;
        led["potencia"] = led_resource[i].potencia;
        led["status"] = led_resource[i].status;
        //arr.add(led);
      }
    }

    //Formata o JSON
    arr.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    //Envia a mensagem com código 200
    http_rest_server.send(200, "application/json", JSONmessageBuffer);


    Serial.println("Json resultado: ");
    Serial.println(JSONmessageBuffer);
  }
}


void get_leds() {

  //Define o buffer do objeto json e o cria
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[200];

  //Inicializa a variável para consulta do elemento criado
  int id_item = 0;

  if (http_rest_server.args() > 0) {
    //id_item = http_rest_server.arg(“id”);
    Serial.print("Nome argumento ");
    Serial.println(http_rest_server.argName(0));
    Serial.print("Valor argumento ");
    Serial.println(http_rest_server.arg(0));
    //Serial.println(http_rest_server.arg(“id”));

    if (http_rest_server.argName(0) == "id") {
      id_item = http_rest_server.arg(0).toInt();
      Serial.print("Operação get. Id a consultar: ");
      Serial.println(id_item);
    }
  }




  if (led_resource[id_item].id == 0)
    //http_rest_server.send(404);
    sendError(404, "[NotFound] - recurso não encontrado", "led_resource[id_item].id == 0");
  else {
    Serial.print("Operação get. Id que será retornado: ");
    Serial.println(led_resource[id_item].id);

    jsonObj["id"] = led_resource[id_item].id;
    jsonObj["gpio"] = led_resource[id_item].gpio;
    jsonObj["pinMod"] = led_resource[id_item].pinMod;
    jsonObj["potencia"] = led_resource[id_item].potencia;
    jsonObj["status"] = led_resource[id_item].status;


    jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);


    Serial.println("Json resultado: ");
    Serial.println(JSONmessageBuffer);
  }
}

void json_to_resource(JsonObject& jsonBody, int val) {
  int id, gpio, potencia, status;
  String pinMod;


  //id = jsonBody["id"];
  id = val;
  gpio = jsonBody["gpio"];
  pinMod = jsonBody["pinMod"].as<String>();
  potencia = jsonBody["potencia"].as<int>();
  status = jsonBody["status"];


  led_resource[val].id = id;
  led_resource[val].gpio = gpio;
  led_resource[val].pinMod = pinMod;
  led_resource[val].potencia = potencia;
  led_resource[val].status = status;

  Serial.print("Novo item criado. Id do item: ");
  Serial.println(val);

  Serial.print("id: ");
  Serial.println(id);
  Serial.print("gpio: ");
  Serial.println(gpio);
  Serial.print("pinMod: ");
  Serial.println(pinMod);
  Serial.print("potencia: ");
  Serial.println(potencia);
  Serial.print("status: ");
  Serial.println(status);
}


void json_to_resource_alterar(JsonObject& jsonBody, int alterar) {
  int id, gpio, potencia, status;
  String pinMod;



  id = jsonBody["id"];

  gpio = jsonBody["gpio"];
  pinMod = jsonBody["pinMod"].as<String>();
  potencia = jsonBody["potencia"].as<int>();
  status = jsonBody["status"];

  led_resource[alterar].id = id;
  //led_resource[alterar].gpio = gpio;
  led_resource[alterar].pinMod = pinMod;
  led_resource[alterar].potencia = potencia;
  led_resource[alterar].status = status;

  Serial.print("Item alterado. Id do item: ");
  Serial.println(id);

  Serial.print("id: ");
  Serial.println(id);
  Serial.print("gpio: ");
  Serial.println("Não é possível alterar GPIO depois de criada");
  Serial.print("pinMod: ");
  Serial.println(jsonBody["pinMod"].as<String>());
  Serial.print("potencia: ");
  Serial.println(potencia);
  Serial.print("status: ");
  Serial.println(status);
}


void post_leds() {
  int encontrado = 0;

  //Cria um objeto para o request
  StaticJsonBuffer<500> jsonBuffer;
  String post_body = http_rest_server.arg("plain");
  Serial.println("JSON de entrada para inclusão:");
  Serial.println(post_body);

  JsonObject& jsonBody = jsonBuffer.parseObject(http_rest_server.arg("plain"));

  //Cria um objeto para o response do post
  StaticJsonBuffer<200> jsonBufferResult;
  JsonObject& jsonObjResult = jsonBufferResult.createObject();
  char JSONmessageBuffer[200];



  Serial.print("HTTP Method: ");
  Serial.println(http_rest_server.method());

  if (!jsonBody.success()) {
    Serial.println("error in parsin json body");
    sendError(400, "[BadRequest] - error in parsin json body", "!jsonBody.success()");
  }
  else {

    //Verifica se o valor do recurso está dentro da lista e se está disponível

    for (int i = 0; i < 13; i++) {

      //Serial.print("Verificando item : ");
      //Serial.println(i);

      if (jsonBody["gpio"].as<int>() == gpioLivre[i].gpio) {

        //Seta a variável como encontrado:
        encontrado = 1;
        //Se encontrado, valida se a porta já foi ocupada
        if (gpioLivre[i].situacao == 1) {

          sendError(404, "[NotFound] - O recurso solicitado já está em uso ou não pode ser utilizado.", "gpioLivre[i].situacao == 1");
          //http_rest_server.send(404, "application/json", erro);


          Serial.print("A GPIO já foi utilizada. GPIO em uso: ");
          Serial.println(jsonBody["gpio"].as<int>());
          break;
        } else {

          Serial.print("A GPIO está disponível. GPIO: ");
          Serial.println(jsonBody["gpio"].as<int>());

          json_to_resource(jsonBody, val_array);
          http_rest_server.sendHeader("Location", "/leds/" + String(led_resource[val_array].id));
          // http_rest_server.send(201);


          jsonObjResult["id"] = led_resource[val_array].id;
          jsonObjResult["gpio"] = led_resource[val_array].gpio;
          jsonObjResult["pinMod"] = led_resource[val_array].pinMod;
          jsonObjResult["potencia"] = led_resource[val_array].potencia;
          jsonObjResult["status"] = led_resource[val_array].status;
          jsonObjResult.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
          http_rest_server.send(201, "application/json", JSONmessageBuffer);

          //Marca a GPIO como indisponível
          gpioLivre[i].situacao = 1;

          //Define como padrão a porta modo saída
          pinMode(led_resource[val_array].gpio, OUTPUT);



          //Caso seja enviada que a porta será de input, definir input. Qualquer outro valor output (default)
          //                        if(led_resource[val_array].pinMod == "INPUT") {
          //                           pinMode(led_resource[val_array].gpio, INPUT);
          //                        } else {
          //                            pinMode(led_resource[val_array].gpio, OUTPUT);
          //                        }
          //

          Serial.print("Put realizado. Val_array antes: ");
          Serial.println(val_array);
          val_array = val_array + 1;
          qtd_items = qtd_items + 1;
          Serial.print("Put realizado. Val_array depois: ");
          Serial.println(val_array);
          break;

        }

      }

    }

    if (encontrado == 0) {
      Serial.print("GPIO informada não está entre as disponíveis. GPIO informada: ");
      Serial.println(jsonBody["gpio"].as<int>());

      sendError(404, "[NotFound] - GPIO informada não está entre as disponíveis.", "encontrado == 0");
      //http_rest_server.send(404, "application/json", erro);

    }




  }
}






void put_leds() {
  StaticJsonBuffer<500> jsonBuffer;
  String post_body = http_rest_server.arg("plain");
  Serial.println("JSON de entrada para alteração:");
  Serial.println(post_body);

  JsonObject& jsonBody = jsonBuffer.parseObject(http_rest_server.arg("plain"));

  //Cria um objeto para o response do post
  StaticJsonBuffer<200> jsonBufferResult;
  JsonObject& jsonObjResult = jsonBufferResult.createObject();
  char JSONmessageBuffer[200];

  Serial.print("HTTP Method: ");
  Serial.println(http_rest_server.method());

  if (!jsonBody.success()) {
    Serial.println("error in parsin json body");

    sendError(400, "[BadRequest] - error in parsin json body", "!jsonBody.success()");
    //http_rest_server.send(400, "application/json", erro);
  }
  else {

    //Validar se o id informado existe
    int val_alterar = jsonBody["id"];
    Serial.print("Id a ser alterado: ");
    Serial.println(val_alterar);

    //Verifica se o id informado existe como recurso e se ele é diferente de zero
    if (jsonBody["id"] == led_resource[val_alterar].id && jsonBody["id"] != 0) {
      json_to_resource_alterar(jsonBody, val_alterar);
      http_rest_server.sendHeader("Location", "/leds/" + String(led_resource[val_alterar].id));
      //http_rest_server.send(200);


      jsonObjResult["id"] = led_resource[val_alterar].id;
      jsonObjResult["gpio"] = led_resource[val_alterar].gpio;
      jsonObjResult["pinMod"] = led_resource[val_alterar].pinMod;
      jsonObjResult["potencia"] = led_resource[val_alterar].potencia;
      jsonObjResult["status"] = led_resource[val_alterar].status;
      jsonObjResult.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

      http_rest_server.send(200, "application/json", JSONmessageBuffer);

      //De acordo com a definição de pinMod, define como analógica ou digital
      Serial.print("Verificar tipo de GPIO: ");
      Serial.println(led_resource[val_alterar].pinMod);

      if (led_resource[val_alterar].pinMod == "ANALOG") {
        Serial.print("GPIO modo analógico. Valor do pino: ");

        //Verifica se o status é 0. Sendo 0, ele coloca a analogwrite no valor mínimo
        if (led_resource[val_alterar].status == 0) {
          analogWrite(led_resource[val_alterar].gpio, 0);
          Serial.println("0");

        } else {
          //Se o valor for maior que 1023, limita em 1023
          if (led_resource[val_alterar].potencia > 1023) {
            analogWrite(led_resource[val_alterar].gpio, 1023);
            Serial.println("1023");
          } else {
            analogWrite(led_resource[val_alterar].gpio, led_resource[val_alterar].potencia);
            Serial.println(led_resource[val_alterar].potencia);
          }

        }

      } else {
        digitalWrite(led_resource[val_alterar].gpio, led_resource[val_alterar].status);
      }



    } else {
      Serial.print("O id informado não existe. Id ");
      Serial.println(val_alterar);
      sendError(404, "[NotFound] - O id informado não existe.", "jsonBody[\"id\"] == led_resource[val_alterar].id");
      //http_rest_server.send(404);
    }
  }
}

//Ligar Ar Condicionado
void liga_ac() {
  ac.setCmd(kFujitsuAcCmdTurnOn);
  Serial.println("Enviando comando de ligar AC ...");
  delay(100);
#if SEND_FUJITSU_AC
  ac.send();
  ligado = true;

#else  // SEND_FUJITSU_AC
  Serial.println("Can't send because SEND_FUJITSU_AC has been disabled.");
#endif  // SEND_FUJITSU_AC

  //Ativar parametros padrao
  ac.setSwing(kFujitsuAcSwingVert);
  ac.send();
  delay(1000);

  ac.setMode(kFujitsuAcModeCool);
  ac.send();
  delay(1000);

  ac.setFanSpeed(kFujitsuAcFanAuto);
  ac.send();


  http_rest_server.send(201, "application/json", printState());



}

void desliga_ac() {
  ac.setCmd(kFujitsuAcCmdTurnOff);
  Serial.println("Enviando comando de desligar AC ...");
#if SEND_FUJITSU_AC
  ac.send();
  ligado = false;
#else  // SEND_FUJITSU_AC
  Serial.println("Can't send because SEND_FUJITSU_AC has been disabled.");
#endif  // SEND_FUJITSU_AC

  http_rest_server.send(201, "application/json", printState());
}

void setTemp_ac() {

  if (http_rest_server.args() > 0) {
    if (http_rest_server.arg(0).toInt() == 0) {
      sendError(404, "Valores possíveis para a opção temperature (em C): 16 a 30. Valor informado: ", http_rest_server.arg(0));

    } else if (http_rest_server.arg(0).toInt() < 16 || http_rest_server.arg(0).toInt() > 30 ) {
      sendError(404, "Valores possíveis para a opção temperature (em C): 16 a 30. Valor informado: ", http_rest_server.arg(0));

    } else {
      if (ligado) {
        ac.setTemp(http_rest_server.arg(0).toInt());  // 24C
        Serial.println("Enviando comando de ajuste de temparatura AC ...");
#if SEND_FUJITSU_AC
        ac.send();
#else  // SEND_FUJITSU_AC
        Serial.println("Can't send because SEND_FUJITSU_AC has been disabled.");
#endif  // SEND_FUJITSU_AC

        http_rest_server.send(201, "application/json", printState());
      } else {
        sendError(409, "O equipamento nao está ligado.", http_rest_server.arg(0));
      }
    }

  } else {
    sendError(404, "Valores possíveis para a opção temperature (em C): 16 a 30. Valor informado: ", http_rest_server.arg(0));
  }






}

void consulta_param_ac() {

  http_rest_server.send(200, "application/json", printState());
}

void setSwing_ac() {

  if (http_rest_server.args() > 0) {
    if (ligado) {
      if (http_rest_server.arg(0) == "0") {
        ac.setSwing(kFujitsuAcSwingOff);
        ac.send();
        http_rest_server.send(201, "application/json", printState());

      } else if (http_rest_server.arg(0) == "1") {
        ac.setSwing(kFujitsuAcSwingVert);
        ac.send();
        http_rest_server.send(201, "application/json", printState());

      } else {
        sendError(404, "Valores possíveis para a opção Swing: 0 - Desligado e 1 - Ligado. Valor informado: ", http_rest_server.arg(0));

      }
    } else {
      sendError(409, "O equipamento nao está ligado.", http_rest_server.arg(0));
    }

  } else {
    sendError(404, "Valores possíveis para a opção Swing: 0 - Desligado e 1 - Ligado. Valor informado: ", http_rest_server.arg(0));
  }
}


void setMode_ac() {

  
  if(ligado){
  if (http_rest_server.args() > 0) {

    
    if (http_rest_server.arg(0) == "0") {
      ac.setMode(kFujitsuAcModeAuto);
      ac.send();
      http_rest_server.send(201, "application/json", printState());

    } else if (http_rest_server.arg(0).toInt() > 0 && http_rest_server.arg(0).toInt() < 5 ) {

      Serial.println("Valor para switch: ");
      Serial.println(http_rest_server.arg(0).toInt());
      switch (http_rest_server.arg(0).toInt()) {
        case 1:
          ac.setMode(kFujitsuAcModeCool);
          Serial.println("Caiu cool");
          break;
        case 2:
          ac.setMode(kFujitsuAcModeDry);
          Serial.println("Caiu dry");
          break;
        case 3:
          ac.setMode(kFujitsuAcModeFan);
          Serial.println("Caiu Fan");
          break;
        case 4:
          ac.setMode(kFujitsuAcModeHeat);
          Serial.println("Caiu heat");
          break;
        default:
          ac.setMode(kFujitsuAcModeCool);
          Serial.println("Caiu cool default");
          break;
      }
      ac.send();
      http_rest_server.send(201, "application/json", printState());

    } else {
      sendError(404, "Valores possíveis para a opção Fan: 0 (Auto), 1 (Cool), 2 (Dry), 3 (Fan), 4 (Heat) - Valor enviado: ", http_rest_server.arg(0));

    }

  } else {
    sendError(404, "Valores possíveis para a opção Fan: 0 (Auto), 1 (Cool), 2 (Dry), 3 (Fan), 4 (Heat) - Valor enviado: ", http_rest_server.arg(0));
  }
  }else{
    sendError(409, "O equipamento nao está ligado.", http_rest_server.arg(0));
    }
}



void setFan_ac() {

  
  if(ligado){
  if (http_rest_server.args() > 0) {

    
    if (http_rest_server.arg(0) == "0") {
      ac.setFanSpeed(kFujitsuAcFanAuto);
      ac.send();
      http_rest_server.send(201, "application/json", printState());

    } else if (http_rest_server.arg(0).toInt() > 0 && http_rest_server.arg(0).toInt() < 5 ) {

      Serial.println("Valor para switch velocidade fan: ");
      Serial.println(http_rest_server.arg(0).toInt());
      switch (http_rest_server.arg(0).toInt()) {
        case 1:
          ac.setFanSpeed(kFujitsuAcFanHigh);
          Serial.println("Caiu high");
          break;
        case 2:
          ac.setFanSpeed(kFujitsuAcFanMed);
          Serial.println("Caiu med");
          break;
        case 3:
          ac.setFanSpeed(kFujitsuAcFanLow);
          Serial.println("Caiu low");
          break;
        case 4:
          ac.setFanSpeed(kFujitsuAcFanQuiet);
          Serial.println("Caiu quiet");
          break;
        default:
          ac.setFanSpeed(kFujitsuAcFanAuto);
          Serial.println("Caiu fan speed default");
          break;
      }
      ac.send();
      http_rest_server.send(201, "application/json", printState());

    } else {
      sendError(404, "Valores possíveis para a opção velocidade fan: 0 (Auto), 1 (High), 2 (Med), 3 (Low), 4 (Quiet) - Valor enviado: ", http_rest_server.arg(0));

    }

  } else {
   sendError(404, "Valores possíveis para a opção velocidade fan: 0 (Auto), 1 (High), 2 (Med), 3 (Low), 4 (Quiet) - Valor enviado: ", http_rest_server.arg(0));
  }
  }else{
    sendError(409, "O equipamento nao está ligado.", http_rest_server.arg(0));
    }
}




//Apresenta o status atual do AC
char* printState() {
  // Display the settings.
  Serial.println("Fujitsu A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
  // Display the encoded IR sequence.
  unsigned char* ir_code = ac.getRaw();
  Serial.print("IR Code: 0x");
  for (uint8_t i = 0; i < ac.getStateLength(); i++)
    Serial.printf("%02X", ir_code[i]);
  Serial.println();

  Serial.println(ac.getModelName());

  //Atualiza os dados do AC para informar no response
  ac_resource.model = ac.getModelName();
  ac_resource.power = ac.getState();
  ac_resource.modeFunc = ac.getModeFunc();
  ac_resource.temperature = ac.getTemperature();
  ac_resource.fan = ac.getFan();
  ac_resource.clean = ac.getCleanAc();
  ac_resource.filter = ac.getFilterAc();
  ac_resource.swing = ac.getSwingAc();
  ac_resource.command = ac.getCommandAc();
  ac_resource.quiet = ac.getQuietAc();

  String transf = ac_resource.model;
  transf.replace("Model: ", "");
  ac_resource.model = transf;

  transf = ac_resource.power;
  transf.replace("Power: ", "");
  ac_resource.power = transf;

  transf = ac_resource.modeFunc;
  transf.replace(", Mode: ", "");
  ac_resource.modeFunc = transf;

  transf = ac_resource.fan;
  transf.replace(", Fan: ", "");
  ac_resource.fan = transf;

  transf = ac_resource.clean;
  transf.replace("Clean: ", "");
  ac_resource.clean = transf;

  transf = ac_resource.filter;
  transf.replace("Filter: ", "");
  ac_resource.filter = transf;

  transf = ac_resource.swing;
  transf.replace("Swing: ", "");
  ac_resource.swing = transf;

  transf = ac_resource.command;
  transf.replace(", Command: ", "");
  ac_resource.command = transf;

  transf = ac_resource.quiet;
  transf.replace("Outside Quiet: ", "");
  ac_resource.quiet = transf;


  //Cria um objeto para o response do post
  StaticJsonBuffer<400> jsonBufferResult;
  JsonObject& jsonObjResult = jsonBufferResult.createObject();
  char JSONmessageBuffer[400];

  jsonObjResult["model"] = ac_resource.model;
  jsonObjResult["power"] = ac_resource.power;
  jsonObjResult["modeFunc"] = ac_resource.modeFunc;
  jsonObjResult["temperature"] = ac_resource.temperature;
  jsonObjResult["fan"] = ac_resource.fan;
  jsonObjResult["clean"] = ac_resource.clean;
  jsonObjResult["filter"] = ac_resource.filter;
  jsonObjResult["swing"] = ac_resource.swing;
  jsonObjResult["command"] = ac_resource.command;
  jsonObjResult["quiet"] = ac_resource.quiet;
  jsonObjResult.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  //http_rest_server.send(201, "application/json", JSONmessageBuffer);

  return JSONmessageBuffer;


}



void sendError(int id, String errorMessage, String errorType) {

  //Cria um objeto para o response do post
  StaticJsonBuffer<200> jsonBufferResult;
  JsonObject& jsonObjResult = jsonBufferResult.createObject();
  char JSONmessageBuffer[500];

  jsonObjResult["errorMessage"] = errorMessage;
  jsonObjResult["errorType"] = errorType;
  jsonObjResult["stackTrace"] = "[]";

  jsonObjResult.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  http_rest_server.send(id, "application/json", JSONmessageBuffer);


}
