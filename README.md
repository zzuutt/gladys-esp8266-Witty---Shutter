# gladys-esp8266-Witty  (lien vers [Gladys](https://gladysassistant.com/))
## Gestion des volets roulants
Module ESP8266 Witty + card 8 relays + card current sensor (option) + 2 cards isolation / detection

_Si vous utilisez ce montage, pensez à m'envoyer un petit retour ;)_

# Module ESP8266 Witty
Nous allons utiliser un module ESP8266 Witty pour gérer 2 relais

![ESP8266WITTY](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/ESP8266-Witty.jpg) + ![2relay](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/2relais.jpg) + ![CurrentSensor](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/CurrentSensor-.jpg) + ![CurrentSensor](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/s-l500-.jpg) + ![alim 220VAC - 5VDC](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/220Vac-5vdc.jpg)

Matériel nécessaire :
- module ESP8266 Witty [lien ebay](https://www.ebay.fr/itm/ESP8266-Serial-WIFI-Witty-Cloud-Development-Board-ESP-12F-Module-MINI-Nodemcu/264039039222?_trkparms=aid=111001&algo=REC.SEED&ao=1&asc=20160908105057&meid=7912a8f239d74abcb7126c140473364d&pid=100675&rk=6&rkt=15&sd=173503702813&itm=264039039222&_trksid=p2481888.c100675.m4236&_trkparms=pageci:8df836f6-4f16-11e9-aa46-74dbd1802d22%7Cparentrq:b59144011690ad794a1eef74fff9554d%7Ciid:1)
- une carte 2 relais avec optocoupleur [lien ebay](https://www.ebay.fr/itm/1-2-4-6-8-Channel-5V-Relay-Module-Board-Optocoupler-LED-for-Arduino-PiC-ARM-AVR/263072650467?ssPageName=STRK%3AMEBIDX%3AIT&var=562073380398&_trksid=p2057872.m2749.l2649) 
- une carte Current sensor (option) [lien ebay](https://www.ebay.fr/itm/Design-5A-Range-Current-Sensor-Module-ACS712-Module-Arduino-Module/173334859689?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2060353.m1438.l2649)
- 2 x une carte de détection / isolation (option) [lien ebay](https://www.ebay.fr/itm/1-Bit-AC-220V-Optocoupler-Isolation-Module-Voltage-Detect-Board-Adaptive-for-PLC/173472400743?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2060353.m1438.l2649)
- Alim 220VAC - 5VDC [lien ebay](https://www.ebay.fr/itm/HLK-PM01-HLK-PM03-HLK-PM12-220V-to-5V-3-3V-12V-Step-Down-Power-Supply-Module/263200022486?ssPageName=STRK%3AMEBIDX%3AIT&var=562194362141&_trksid=p2057872.m2749.l2648)

Pour ne pas tout recâbler, j'ai gardé la partie commande mécanique en 220VAC. J'utilise 2 cartes de detection/isolation pour le raccordement à l'esp.

Avec tout ça, nous allons contrôler un volet commandé par un inter (3 commandes - Monte / Stop / Descent) via :

- [x] Gladys (avec un script - pour le moment)
- [x] par l'interface web du module. 
- [x] inter 3 commandes

# Description

Nous allons utiliser une zone réservée appelée SPIFFS.

Afin de pouvoir télécharger des fichiers dans cette zone mémoire, il est nécessaire d’installer l’outil de téléchargement des fichiers dans la zone SPIFFS à l’IDE Arduino disponible sur [Github](https://github.com/esp8266/arduino-esp8266fs-plugin).

Téléversez le fichier ino dans l'ESP puis les datas  **_(dans ce sens !)_**

## Raccordement

Schéma
<p><a target="_blank" href="https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/Volet%20roulant%20-%20Schema.jpg"><img src="https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/Volet%20roulant%20-%20Schema.jpg" width="350"></a> &nbsp; &nbsp; <img src="https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/gestion5.gif" height="278"><img src="https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/Gladys.jpg" height="278"></p>

**_Si vous respectez le raccordement du 220VAC sur les relais, vous ne risquez pas d'endommager le moteur. Même si vous appuyez sur la monté et la descente simultanément !_**

## Visualisation 
Le module ESP est équipé d'une led 3 couleurs, nous allons l'utiliser pour visualiser chaque étape.
> **led bleue clignotante (lent) :**
> Le module doit être configuré (la partie wifi)

> **led bleue clignotante (rapide) :**
> Le module est en configuration d'usine. Connectez vous sur l'interface de gestion pour entrer les paramètres.

> **led magenta clignotante :**
> Le volet monte

> **led jaune clignotante :**
> Le volet descend

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

Si vous n'utilisez pas le module optionnel (ACS712), dès que l'esp se trouvera dans l'obscurité, la led sera coupée.
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

>**Pour une gestion via Gladys, renseignez les parametres 'SERVEUR' 'GLADYS'**

![conf1](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf1.jpg)
![conf2](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf2.jpg)
![conf3](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf3.jpg)
![conf4](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf4.jpg)

Menu Configuration :
- Divers
  - Définition du capteur (Sensor)
  - Led active ?
  - Position du volet après une coupure électrique
- Serveur
  - Nom d'hôte (mDNS)
  - Gladys
- Définition des Groupes
  - nom des différents groupes
- Définition du périphérique
  - nom du volet
  - ID (-*id du device-type dans Gladys*- voir § Gestion via Gladys)
  - Groupe
  - Durée du temps de monté
  - Durée du temps de descente
  - Calibration Ratio

> Après une coupure électrique, les commandes mécaniques ne sont pas prises en compte. Le volet est positionné à la valeur défini dans la configuration **[ position fermé / position ouvert / dernière position connue]**

- blanc clignotant : le module envoie l'état du volet à Gladys.

## Réglage des temps de courses (Monté et Descente)

### Réglage Manuel

- vous n'avez pas le module optionnel. Faites le réglage **manuelllement**

> Sur la page 'système', sélectionnez le menu 'Courses (Manuel)' et suivre les instructions

![conf3](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf3.jpg)

### Réglage Automatique

- vous avez raccordé le module optionnel. Faites le réglage en **automatique** 

_N'oubliez pas de cocher la case sur la page de configuration_

> Sur la page 'système', sélectionnez le menu 'Courses (Auto)' et suivre les instructions

![conf4](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf4.jpg)

## Gestion

Vous pouvez gérer votre volet roulant à l'aide :
- [x] de Gladys (Directement par l'interface et en utilisant des scripts)
- [x] des commandes mécaniques
- [x] depuis votre smartphone.

le tout en même temps

#### Gestion via Gladys
![gladys](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/Gladys.jpg)

* installez le module (si ce n'est pas déjà fait) de **Mathieu**, [Gladys-Device-HTTP](https://github.com/MathieuAndrade/Gladys-Device-HTTP)
* Ajoutez un nouveau device, comme indiqué, dans le champ identifier renseignez l'adresse complète de votre device (http://ip_de_lesp)
![Gladys1](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/Screenshot_2019-06-10%20P%C3%A9riph%C3%A9riques%20Gladys.png)

* Une fois créé voir cliquez sur “voir plus”. Dans le champ identifier du deviceType entrez les paramètres suivants 
- Identifier: /?token=**VOTRE-TOKEN**&deviceid=**ID-FIGURANT-SUR-CETTE-MEME-LIGNE**&cmd=goto
- Type : pourcent
- Catégorie: Capteur d'ouverture fenêtre
- Unité: %
- min: 0
- max: 100

![Gladys1](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/Screenshot_2019-06-10%20P%C3%A9riph%C3%A9riques%20Gladys(1).png)

Cela donne cela:

[![Video Glagys](https://img.youtube.com/vi/iVRMdMqa7eU/0.jpg)](https://www.youtube.com/watch?v=iVRMdMqa7eU)


#### Script Gladys
commande à utiliser:
> http://ip_de_lesp/?token=votre_token&deviceid=ID_du_volet&cmd=COMMAND&position=POSITION

> COMMAND : open / stop / close / goto

> POSITION : indiquer un pourcentage ( 0 = ouvert / 100 = fermé ) utilisé avec la commande 'goto'

#### Depuis un smartphone

Allez sur la page 'système' puis sélectionnez le menu 'Gestion'

![conf1](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/conf1.jpg)
![gestion1](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/gestion5.gif)
![gestion](https://github.com/zzuutt/gladys-esp8266-Witty---Shutter/blob/master/images/gestion.jpg)


