//Librería para el control de servomotores
#include <Servo.h>

//Instancias para el control de cada uno de los 4 servomotores
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

//Tiempo para el retraso de la velocidad del servomotor
int delayTime = 1;
//Matriz para el almacenamiento de las posiciones de los servomotores
int positions[] = {0, 90, 120, 90, 100};
//Matriz para el almacenamiento de los números de los pines de los servomotores
int motorPorts[] = {0, 5, 6, 7, 8};
/*
   Variable para el almacenamiento del voltaje en el divisor de voltaje
   entre el LDR y la resistencia de 10K
*/
int lightValue = 0;
/*
   Tiempo de espera antes de detener la banda transportadora por inactividad
   al no detectar ningún objeto. El tiempo por defecto es de 10 segundos. La
*/
int idleTime = 10000;
//Variable para el control del tiempo
unsigned long currentTime = 0;
//Estado lógico de la banda transportadora (encendida o apagada)
boolean band = true;
//Estado lógico del sistema (encendido o apagado)
boolean start = false;
//Variable tipo toggle para designar la función del botón que enciende o apaga el sistema
boolean startState = true;
//Modo de seteo para mover motores especificos determinada cantidad de grados
boolean settingMode = false;

//Pin digital del motor de la banda transportadora
int motorPin = motorPin;
//Pines digitales para cada uno de los anodos del LED RGB
int rPin = 18;
int gPin = 16;
int bPin = 15;

void setup() {
  //Se inicia la comunicación serial
  Serial.begin(9600);

  //Se setean los pines de los servomotores
  servo1.attach(motorPorts[1]);
  servo2.attach(motorPorts[2]);
  servo3.attach(motorPorts[3]);
  servo4.attach(motorPorts[4]);

  //Se establecen los angulos iniciales de los motores
  Serial.print("Setting zero...");
  resetPosition();
  Serial.println("done");

  //Se establecen los pines de salida digital
  pinMode(motorPin, OUTPUT);
  pinMode(bPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(rPin, OUTPUT);

  //Se setea el color del LED RGB en rojo
  setRGB(1, 0, 0);
}

void loop() {
  //Modo de configuracion por consola serial
  if (Serial.available() > 0) {
    //Se recibe el comando inicial
    String command = Serial.readString();
    //Modo de seteo
    if (command == "start set mode") {
      settingMode = true;
      Serial.println("Setting Mode activo...");
      stopSystem();
    }
    //Salida del modo seteo
    else if (command == "end set mode") {
      settingMode = false;
      Serial.println("Setting Mode inactivo...");
    } else {
      /**
         Si el sistema esta en modo seteo aceptara
         que el usuario cambie la posicion de motores
         especificos, nombrandolos por su numero (1, 2, 3 y 4)
      */
      if (settingMode) {
        int motorNumber = command.toInt();
        Serial.print("Se ha seleccionado el motor Nº ");
        Serial.println(motorNumber);
        while (Serial.available() == 0) {}
        int steps = Serial.readString().toInt();
        //Se setea la posicion de un motor en determinada cantidad de grados
        changePosition(motorNumber, steps);
      }
    }
  }

  //Si el sistema no se encuentra en modo setting...
  if (!settingMode) {
    //Bolque de código para la lectura del estado del botón
    if (analogRead(A0) > 750) {
      delay(500);
      /*
         Si se presiona el boton, se evalua el estado logico de
         la variable startState. En caso de que sea TRUE, se apaga el sistema.
         Si es FALSE, se inicia el sistema
      */
      if (startState) {
        startState = false;
        startSystem();
      }
      else
      {
        startState = true;
        stopSystem();
      }
    }
    /*
       El siguiente bloque de código es el que le da movilidad al sistema
       Si el sistema ha sido iniciado, se evalua si la banda debe moverse o no.
    */
    if (start) {
      if (band) {
        //Si la banda esta activa, se evalua si el laser incide sobre el LDR o no
        lightValue = analogRead(A7);
        if (lightValue > 250) {
          //Si el LDR no recibe la luz del laser...
          delay(250);
          //Se detiene la banda transportadora
          digitalWrite(motorPin, LOW);
          band = false;
          //Se cambia el color del LED RGB a azul
          setRGB(0, 0, 1);
          //Se dispara el funcionamiento del brazo
          armAction();
          //Se cambia el color del LED RGB a verde
          setRGB(0, 1, 0);
          //Se guarda el tiempo en el cual se dio el evento
          currentTime = millis();
        }
        else
        {
          /*
             En caso de que el LDR reciba la luz del LED Laser, se mantiene el motor
             de la banda transportadora encendido
          */
          digitalWrite(motorPin, HIGH);
          //En caso de que el tiempo de espera supere el idleTime...
          if ((millis() - currentTime) > idleTime) {
            Serial.println("Stopping by iddle time...");
            //Se apaga el sistema y la banda transportadora
            band = false;
            start = false;
            digitalWrite(motorPin, LOW);
            //Se cambia el estado del LED RGB a rojo
            setRGB(1, 0, 0);
          }
        }
      }
    }
    //Se escribe la posicion de cada uno de los 4 motores
    writePosition();
  }
}

//Metodo para el cambio de posicion de un motor en especifico
void changePosition(int motorNumber, int newPos) {
  int pos = positions[motorNumber];
  Serial.print("Motor Port: ");
  Serial.println(motorPorts[motorNumber]);
  Serial.print("Actual Position: ");
  Serial.println(positions[motorNumber]);
  Serial.print("New Position: ");
  Serial.println(newPos);
  if (newPos > pos) {
    for (int i = pos; i <= newPos; i++) {
      Serial.println(i);
      writePosition();
      delay(delayTime);
    }
  }
  else if (newPos < pos)
  {
    for (int i = pos; i >= newPos; i--) {
      Serial.println(i);
      writePosition();
      delay(delayTime);
    }
  }
  //Se actualizan las posiciones en la matriz
  positions[motorNumber] = newPos;
  writePosition();
}
//Metodo para iniciar el sistema
void startSystem() {
  start = true;
  band = true;
  currentTime = millis();
  Serial.println("Starting...");
  setRGB(0, 1, 0);
}
//Metodo para detener el sistema
void stopSystem() {
  start = false;
  band = false;
  digitalWrite(motorPin, LOW);
  Serial.println("Stopping...");
  setRGB(1, 0, 0);
}
/**
   Este metodo escribe la posicion de cada uno de los motores.
   Estas posiciones se guardan en la matriz positions
*/
void writePosition() {
  servo2.write(positions[2]);
  servo3.write(positions[3]);
  servo1.write(positions[1]);
  servo4.write(positions[4]);
}
/**
   Método que devuelve el brazo robotico a su posicion
   inicial con los angulos iniciales
*/
void resetPosition() {
  positions[1] = 90;
  positions[2] = 120;
  positions[3] = 90;
  positions[4] = 100;
  writePosition();
  band = true;
}
/**
   Metodo que dentro del proyecto en el que trabaje permite
   al brazo mecanico recoger las cajas y depositarlas en
   el recipiente. Esto puede variar de un proyecto a otro
*/
void armAction() {
  changePosition(3, 130);
  changePosition(2, 74);
  changePosition(4, 150);
  changePosition(2, 120);
  changePosition(1, 10);
  changePosition(3, 90);
  changePosition(2, 90);
  changePosition(4, 100);
  delay(500);
  resetPosition();
}
//Metodo para setear los colores del LED RGB
void setRGB(int r, int g, int b) {
  digitalWrite(bPin, b);
  digitalWrite(gPin, g);
  digitalWrite(rPin, r);
}

