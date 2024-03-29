# Старт-стоп таймер для мотоджимханы
[Схема подключения](https://github.com/VeZhD/sw-telemetry/blob/main/sw_client_LCD-I2C-1602/sw_client_LCD-I2C-1602_connection%20scheme.png)

### Список компонентов

Сенсор - [фотоэлектрический](https://aliexpress.ru/item/1005002336796482.html?sku_id=12000020156768877) или [лазерный](https://aliexpress.ru/item/1005002273898725.html?sku_id=12000019867321864)

ESP32 - [Lolin S2 mini](https://aliexpress.ru/item/1005003145192016.html?sku_id=12000024338777694), можно найти дешевле, но нужно чтобы обязательно был чип с PSRAM.

[DC-DC]([https://aliexpress.ru/item/1005002525645136.html](https://aliexpress.ru/item/1005001578810552.html?sku_id=12000016666979656)) или [этот](https://aliexpress.ru/item/10000000656280.html?sku_id=20000000002551824), а можно и  [этот](https://aliexpress.ru/item/1005003249843639.html?sku_id=12000024854854504)

OLED I2C display [2.4"](https://aliexpress.ru/item/1005005867315821.html?sku_id=12000034633566861) или [2.4"](https://aliexpress.ru/item/1005001565591155.html), [1.54"](https://aliexpress.ru/item/1005005317314760.html?sku_id=12000032606581542), [1.3"](https://aliexpress.ru/item/1005005438758215.html?sku_id=12000033085858096), [0.96"](https://aliexpress.ru/item/1005001603008505.html?sku_id=12000016754783852)

Пара резисторов необходимы в виде делителся напряжения с 12В до 3В, в схеме используется на 10кОм и 3.3кОм, при этом 3.3кОм вкчючается между GND и 6 пином на ESP, 
Кнопки можно использовать любые нормально открытые кнопки, как вариант тактовые кнопки
Аккумулятор использовал любой именющийся в наличии формата 18650, литий-ионный или повербанк
Подобрать корпус и хорошо зафиксировать, фотки собранного устройства появяться чуть позже как будет выглядить уже как завершенное устройство.

### Принцип работы

Таймер стартует при первом прохождении линии сенсора и не может быть остановлен быстрее чем за 3 секунды(в дальнейшем будет возможность изменения этого времени). Финиш фиксируется так же при первом пересечении линии.

Кнопки на данный момент заложены для будующих функций - изменения времени следующего срабоатывания датчика, изменение алгоритма работы(при каждом срабатывании новый круг, как у китайской телеметрии или старт и финиш). 

Пересечение линии дублируется строчкой об ошибки сенсора на экране, так как эта "ошибка" сигнализирует о наличии препятствия перед сенсором, о пересечении линиии или не правильной установки сенсора и обратной его сторона(отражателя)

Для изменения имени Точки досупа и пароля необходимо изменить переменные в файле "SSID_server.h"

Для подключения с телефона\ноутбука\etc. нужно подключиться к точки доступа "StopWatcher" (пароль по умолчанию - StopWatcher) или с даннсыми изменеными ранее в файле "SSID_server.h", андроид может оповестить, что при подключении к этой точки доступа нет доступа к интернету - необходимо поставить галочку - "Больше не спрашивать" и нажать "Да" соглашаясь с подключеним или заранее отключить мобильный интернет на телефоне. Как все происходит на iPhone я не знаю точно и не тестировал, но должно быть аналогично. Далее заходим в браузер и переходим по адресу 192.168.4.1 , сразу после загрузки страницы будет отображение времени. Так же в прошивке есть DNS, так что можно в строке ввода url можно ввести любые символы и "/" - например "str675/" и также должны загрузиться страничка телеметрии.
