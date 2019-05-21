# gladys-esp8266-Witty  (lien vers [Gladys](https://gladysassistant.com/))
## Gestion des volets roulants
Module ESP8266 Witty + card 8 relays + card current sensor (option)

# Module ESP8266 Witty
Nous allons utiliser un module ESP8266 Witty pour gérer 2 relais

![ESP8266WITTY](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/ESP8266-Witty.jpg) + ![2relay](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/2relais.jpg) + ![CurrentSensor](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/CurrentSensor.jpg)

Matériel nécessaire :
- module ESP8266 [lien ebay](https://www.ebay.fr/itm/ESP8266-Serial-WIFI-Witty-Cloud-Development-Board-ESP-12F-Module-MINI-Nodemcu/264039039222?_trkparms=aid=111001&algo=REC.SEED&ao=1&asc=20160908105057&meid=7912a8f239d74abcb7126c140473364d&pid=100675&rk=6&rkt=15&sd=173503702813&itm=264039039222&_trksid=p2481888.c100675.m4236&_trkparms=pageci:8df836f6-4f16-11e9-aa46-74dbd1802d22%7Cparentrq:b59144011690ad794a1eef74fff9554d%7Ciid:1)
- une carte 2 relais avec optocoupleur [lien ebay](https://www.ebay.fr/itm/1-2-4-6-8-Channel-5V-Relay-Module-Board-Optocoupler-LED-for-Arduino-PiC-ARM-AVR/263072650467?ssPageName=STRK%3AMEBIDX%3AIT&var=562073380398&_trksid=p2057872.m2749.l2649) 
- une carte Current sensor (option) [lien ebay](https://www.ebay.fr/itm/Design-5A-Range-Current-Sensor-Module-ACS712-Module-Arduino-Module/173334859689?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2060353.m1438.l2649)

Avec tout ça, nous allons contrôler un volet commandé par un inter (3 commandes - Monte / Stop / Descent) via :

- Gladys (avec un script - pour le moment)
- par l'interface web du module. 

# Description

Téléversez le fichier ino ou le bin dans l'ESP

Nous allons utiliser une zone réservée appelée SPIFFS.

Afin de pouvoir télécharger des fichiers dans cette zone mémoire, il est nécessaire d’installer l’outil de téléchargement des fichiers dans la zone SPIFFS à l’IDE Arduino disponible sur [Github](https://github.com/esp8266/arduino-esp8266fs-plugin).

## Raccordement

Schéma

## Visualisation 
Le module ESP est équipé d'une led 3 couleurs, nous allons l'utiliser pour visualiser chaque étape.
> **led bleue clignotante (lent) :**
> Le module doit être configuré (la partie wifi)

> **led bleue clignotante (rapide) :**
> Le module est en configuration d'usine. Connectez vous sur l'interface de gestion pour entrer les paramètres.

> **led magenta clignotante :**
> Le volet descend

> **led jaune clignotante :**
> Le volet monte

> **led verte clignotante :**
> Indique la lecture du fichier de configuration

>**led verte clignotante (rapide) :**
> mode 'debug' activé

>**led fixe blanche :**
> le module est démarré, il attend les commandes et évènements

> **led blanche clignotante :**
> le module envoie des données à Gladys

> **led rouge clignotante :**
> indique une erreur

Si vous n'utilisez pas le module optionnel (ACS712), dès que le module se trouvera dans l'obscurité, la led sera coupée.
Avec le module, vous pouvez configurer l'état de la led par défaut (voir page de config) et +
 
## Actions
Le bouton de l'ESP va nous permettre d'effectuer différentes actions
> **un appui court :**
> active le mode debug

> **deux appuis courts :**
> désactive / active la led

> **deux appuis courts après avoir activé le mode debug :**
> imprime sur le port série la configuration

> **un appui long (8sec) :**
> réinitialisation des paramètres wifi, active le mode configuration. Utile lors d'un changement de paramètre.

# Première étape

## Initialisation
A l'allumage le module clignote en bleue, vous devez configurer le wifi.

C'est très simple, prenez votre smartphone, 

* connectez vous sur le réseau wifi ESP suivi de chiffre.
* Allez sur l'url "*http://192.168.4.1*", vous êtes sur la page d'accueil du module.
* Cliquez sur "*Configuration*".
* Renseignez les paramètres demandés
* Cliquez sur "*Save*"
* Retournez à la page d'accueil "*main page*"
* Notez l'adresse IP du module, cela peut servir sauf si vous utilisez une IP fixe
* Fermez la page "*Exit portal*"

![step1](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/step1.jpg)
![step2](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/step2.jpg)

![step3](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/step3.jpg)
![step4](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/step4-2.jpg)
![step4](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/step4-3.jpg)
![step5](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/step6.jpg)
![step6](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/step8-0.jpg)

La led bleue clignotante passe au fixe, puis le module redémarre.

S'il ne redémarre pas tout seul, débranchez l'alimentation puis rebranchez.

La led verte clignote puis passe au blanc clignotant ou bleu clignotant rapide. 

- bleu clignotant rapide: vous êtes en configuration d'usine. Allez sur la page 'système' du module

http://ip-de-lesp/sys

![conf1](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf1.jpg)
![conf2](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf2.jpg)
![conf3](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf3.jpg)
![conf4](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf4.jpg)

Vous pouvez :
- régler la position du volet roulant après une coupure électrique
- mettre un nom à votre volet roulant
- définir la pièce ou il est situé
- ....

> Après une coupure électrique, les commandes mécaniques ne sont pas prises en compte. Le volet est positionné à la valeur défini dans la configuration **[ position fermé / position ouvert / dernière position connue]**

- blanc clignotant : le module envoie l'état du volet à Gladys.

## Réglage des temps de courses (Monté et Descente)

### Réglage Manuel

- vous n'avez pas le module optionnel. Faites le réglage **manuelllement**

> Sur la page 'système', sélectionnez le menu 'Courses (Manuel)' et suivre les instructions

![conf3](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf3.jpg)

### Réglage Automatique

- vous avez raccordé le module optionnel. Faites le réglage en **automatique**

> Sur la page 'système', sélectionnez le menu 'Courses (Auto)' et suivre les instructions

![conf4](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf4.jpg)

## Gestion

Vous pouvez gérer votre volet roulant à l'aide :
- de Gladys (en utilisant des scripts)
- des commandes mécaniques
- depuis votre smartphone.

le tout en même temps

#### Script Gladys
commande à utiliser:
> http://ip_de_lesp/?token=votre_token&deviceid=ID_du_volet&cmd=COMMAND&position=POSITION

> COMMAND : open / stop / close / goto

> POSITION : indiquer un pourcentage ( 0 = ouvert / 100 = fermé ) utilisé avec la commande 'goto'

#### Depuis un smartphone

Allez sur la page 'système' puis sélectionnez le menu 'Gestion'

![conf1](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf1.jpg)
![gestion](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/gestion.jpg)
![gestion1](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/gestion1.jpg)

