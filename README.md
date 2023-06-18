# Vatímetro inteligente de Iot al Barrio

El google sheets donde estamos haciendo las pruebas de recolleción de datoa es <link>


## Google Apps Script

<https://script.google.com/u/3/home/start>

## Compilación ESP32

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