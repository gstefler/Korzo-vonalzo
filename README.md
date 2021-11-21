#  Feladat

![rendered_image](https://github.com/gstefler/Korzo-vonalzo/blob/master/img/rendered_image.png)

Készítsen "vonalzó + körző" 2D rajzoló programot. A program egy 10 cm x 10 cm-es világot mutat, amelynek indulásakor félmagasságban egy vízszintes vonal van, annak közepén és onnan 1 cm-re jobbra egy-egy pont látható. A megengedett szerkesztési műveletek:

1. Körző befogása: 's' + klikk egy létező pontra, ahova a körzőt szúrjuk + klikk egy létező pontra, ahova a körzőt nyitjuk
2. Kör rajzolás a befogott sugárral: 'c' + klikk egy létező pontra, ami a kör középpontja lesz
3. Egyenes rajzlolás: 'l' + klikk egy létező pontra, ami az egyenes egy pontja + klikk egy létező pontra, ami az egyenes másik pontja
4. Metszéspont: 'i' + klikk egy egyenesre vagy körre + klikk egy egyenesre vagy körre, amellyel az összes metszéspontot kiszámoljuk és megjelenítjük

A pontok sárgák, az egyenesek pirosak, a körök cián színűek. A klikkel kijelölt pont, egyenes, kör ideiglenesen fehér lesz, amíg a művelet befejeződik. Csak meglévő pontokból lehet építkezni. A klikkelést 0.1 cm tűréssel kell megvalósítani. A képernyő felbontása 600 x 600 pixel.
