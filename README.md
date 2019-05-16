# gladys-esp8266-Witty  (lien vers [Gladys](https://gladysassistant.com/))
## Gestion des lumières et/ou prises   - mode Push ou Switch
Module ESP8266 Witty + extend card MCP23017 + card 8 relays + card 8 input

Exemple de câblage:
<p><a target="_blank" href="https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/schema.jpg"><img src="https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/schema.jpg" width="350"></a> &nbsp; &nbsp; <img src="https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step9.jpg" height="278"></p>

# Module ESP8266 Witty
Nous allons utiliser un module ESP8266 Witty pour gérer 8 relais et 8 entrées

![ESP8266WITTY](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/ESP8266-Witty.jpg) + ![MCP23017](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/mcp23017.jpg) + ![8relay](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/8relay.jpg)![8input](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/8input.jpg)

Matériel nécessaire :
- module ESP8266 [lien ebay](https://www.ebay.fr/itm/ESP8266-Serial-WIFI-Witty-Cloud-Development-Board-ESP-12F-Module-MINI-Nodemcu/264039039222?_trkparms=aid=111001&algo=REC.SEED&ao=1&asc=20160908105057&meid=7912a8f239d74abcb7126c140473364d&pid=100675&rk=6&rkt=15&sd=173503702813&itm=264039039222&_trksid=p2481888.c100675.m4236&_trkparms=pageci:8df836f6-4f16-11e9-aa46-74dbd1802d22%7Cparentrq:b59144011690ad794a1eef74fff9554d%7Ciid:1)
- une carte extension mcp23017
- une carte 8 relais avec optocoupleur [lien ebay](https://www.ebay.fr/itm/1-2-4-8-16-12V-Channel-Relay-Module-With-optocoupler-For-PIC-AVR-DSP-ARM-Arduino/173604730014?var=472283979123&_trkparms=aid=222007&algo=SIM.MBE&ao=2&asc=20170831090034&meid=e1b7924e5ee348e0948b592fde06df41&pid=100005&rk=1&rkt=9&sd=391975892322&itm=472283979123&_trksid=p2047675.c100005.m1851) 
- une carte Optocoupler Isolation Voltage [lien ebay](https://www.ebay.fr/itm/Optocoupler-Isolation-Voltage-Test-Board-8-Channel-AC-220V-MCU-TTL-for-PLC/263162567047?_trkparms=aid=111001&algo=REC.SEED&ao=1&asc=20160908105057&meid=7912a8f239d74abcb7126c140473364d&pid=100675&rk=4&rkt=15&sd=173503702813&itm=263162567047&_trksid=p2481888.c100675.m4236&_trkparms=pageci:8df836f6-4f16-11e9-aa46-74dbd1802d22%7Cparentrq:b59144011690ad794a1eef74fff9554d%7Ciid:1)

Avec tout ça, nous allons contrôler des lampes raccordées à des télérupteurs. 

Le câblage existant est conservé. 

La carte relais sera relié aux télérupteurs.

La carte "Optocoupler isolation 8 entrées" sera reliée aux lampes pour visualiser leurs états.

# Description

Téléversez le fichier ino ou le bin dans l'ESP

A partir de la version 3.00, nous allons utiliser une zone réservée appelée SPIFFS.

Afin de pouvoir télécharger des fichiers dans cette zone mémoire, il est nécessaire d’installer l’outil de téléchargement des fichiers dans la zone SPIFFS à l’IDE Arduino disponible sur [Github](https://github.com/esp8266/arduino-esp8266fs-plugin).

## Raccordement mcp23017

Le port A du mcp23017 est utilisé pour commander les relais

Le port B est utilisé pour contrôler l'état des lampes

Je vous recommande de raccorder l'I2C comme ci-dessous, mais pas d’inquiétude vous pourrez rechercher comment est raccordé votre mcp23017 (voir Menu Système -> Scan I2C) 

| pin |      mcp23017      |     ESP8266
|-----|--------------------|--------------------
|I2C SDA | I2C SDA | pin GPIO5
|I2C SCL | I2C SCL | pin GPIO0
|Interrupt | INT B | pin GPIO14

## Visualisation 
Le module ESP est équipé d'une led 3 couleurs, nous allons l'utiliser pour visualiser chaque étape.
> **led clignotante bleue :**
> Le module doit être configuré (la partie wifi)

> **led magenta clignotante :**
> Le module est en configuration usine - Vous devez aller sur la page système pour saisir vos paramètres 

> **led verte clignotante :**
> Indique la lecture du fichier de configuration

>**led verte clignotante (rapide) :**
>mode 'debug' activé

>**led fixe blanche :**
>le module est démarré, il attend les commandes et évènements

> **led blanche clignotante :**
> le module envoie des données à Gladys

> **led rouge clignotante :**
> indique une erreur

Si vous avez coché l'option '***Light setting***', dès que le module se trouvera dans l'obscurité, la led sera coupée
 
## Actions
Le bouton de l'ESP va nous permettre d'effectuer différentes actions
> **un appui court :**
> active le mode debug

> **deux appuis courts après avoir activé le mode debug :**
> imprime sur le port série la configuration

> **un appui long (8sec) :**
> réinitialisation des paramètres wifi, active le mode configuration. Utile lors d'un changement de paramètre.

## Première étape
Configurez Gladys
* installez le module (si ce n'est pas déjà fait) de **Mathieu**, [Gladys-Device-HTTP](https://github.com/MathieuAndrade/Gladys-Device-HTTP)
* Ajoutez un nouveau device, comme indiqué, dans le champ identifier renseignez l'adresse complète de votre device (http://ip_de_lesp)
* Cliquez sur "Edit" et créé un devicetype. Dans le champ identifier du deviceType entrez les paramètres suivants */?token=**votre_token**&deviceid=**ID_du_devicetype**&cmd=**COMMAND***

Les **COMMAND**es peuvent être (***en majuscule***) :

* PUSH   *(pour les télérupteurs) la durée de l'appui est modifiable voir les paramètres*
* SWITCH

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

![step1](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step1.jpg)
![step2](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step2.jpg)

![step3](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step3.jpg)
![step4](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step4-2.jpg)
![step4](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step4-3.jpg)
![step5](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step6.jpg)
![step6](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step8-0.jpg)

La led bleue clignotante passe au fixe, puis le module redémarre.

S'il ne redémarre pas tout seul, débranchez l'alimentation puis rebranchez.

La led verte clignote puis passe au blanc clignotant ou magenta clignotant. 

- magenta clignotant : vous êtes en configuration d'usine. Allez sur la page 'système' du module

http://ip-de-lesp/sys

![step7](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step5-1.jpg)
![step8](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step6-2.jpg)
![step9](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step7-1.jpg)
![step10](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step8-1.jpg)

- blanc clignotant : le module envoie l'état des lampes à Gladys.

## Gestion

Vous pouvez gérer vos périphériques directement depuis votre smartphone.

Allez sur la page système puis sélectionnez le menu gestion

![step7](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step5-1.jpg)
![gestion](https://github.com/zzuutt/gladys-esp8266-Witty/blob/master/images/step9.jpg)
