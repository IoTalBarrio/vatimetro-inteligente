# Vatímetro inteligente de Iot al Barrio

El google sheets donde estamos haciendo las pruebas de recolleción de datoa es <link>


## Google Sheets
1. Crear un Google Sheets
2. Configura el archivo para que cualqueira con el link puede acceder al archivo
3. Extrae el ID del archivo de la URL
   ~~~shell
   https://docs.google.com/spreadsheets/d/18LgNUzvpz2nXAxljXm-M7lh4k4oUYM0yrNcw7t4t3cE/edit?usp=sharing
   GSS='18LgNUzvpz2nXAxljXm-M7lh4k4oUYM0yrNcw7t4t3cE'
   ~~~
4. 

## Google Apps Script

Por favor lee primero [Crea y administra implementaciones ](https://developers.google.com/apps-script/concepts/deployments?hl=es-419)

Petición:
~~~shell
DEPKEY=<"Llave de despliegue">
URL=https://script.google.com/macros/s/$DEPKEY/exec?hoja
curl -L $URL
curl -d "" -L $URL
DATA=$(./generador.sh 0 1024)
curl -H 'Content-Type: application/json' -d $DATA -L $URL?hoja=test
~~~

Cada vez que hace hace un cambio se tiene que re-desplegar la web app


### Pruebas

## Compilación ESP32

### Librerías

* HTTPSRedirect https://github.com/electronicsguy/HTTPSRedirect

### Linux

Libreria ESP32 version 2.X

Permisos para el puerto:

~~~shell
$ lsusb
...
Bus 001 Device 055: ID 10c4:ea60 Silicon Labs CP210x UART Bridge
...
~~~

~~~shell
$ ls -al /dev/ttyUSB0
crw-rw---- 1 root dialout 188, 0 jun 17 09:55 /dev/ttyUSB0
~~~

Aca notamos que el dispostivo esta solo para el grupo `dialout` así que procedemos a agregar el usuario con que se lanza la IDE (`$USER`) al grupo `dialout` y (`tty`):

~~~shell
$ sudo usermod -a -G dialout $USER
$ sudo usermod -a -G tty $USER
$ sudo chmod a+rw /dev/ttyUSB0
~~~

Cada vez que se conecte
~~~shell
$ lsusb
...
Bus 001 Device 003: ID 10c4:ea60 Silicon Labs CP210x UART Bridge
...
$ ls -al /dev/ttyUSB*
crw-rw---- 1 root dialout 188, 0 jun 18 12:38 /dev/ttyUSB0
$ sudo chmod 0666 /dev/ttyUSB0
$ ls -al /dev/ttyUSB*
crw-rw-rw- 1 root dialout 188, 0 jun 18 12:38 /dev/ttyUSB0./
~~~