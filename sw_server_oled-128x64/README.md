# Старт-стоп таймер для мотоджимханы
[Схема подключения NPN](https://github.com/VeZhD/sw-telemetry/blob/7f64bcfa51613b42df29e8827b83ce04b0554e23/sw_server_oled-128x64/sw_server_oled-128x64_NPN-NO_connection%20scheme.png)

[Схема подключения PNP](https://github.com/VeZhD/sw-telemetry/blob/7f64bcfa51613b42df29e8827b83ce04b0554e23/sw_server_oled-128x64/sw_server_oled-128x64_PNP-NO_connection%20scheme.png)

Тип используемого датчика меняется в самом скетче в строке:

`#define SENSOR_NPN // при использовании сенсора с NPN укзать SENSOR_NPN, при использовании сенсора с PNP укзать SENSOR_PNP`
`#define SENSOR_NO // при использовании сенсора с NO(Normal Open, нормально открытый) укзать SENSOR_NO, при использовании сенсора NC(Normal Closed, нормально закрытый) укзать SENSOR_NC`

### Список компонентов

Сенсор NPN - [фотоэлектрический e3z-R61](https://aliexpress.ru/item/1005005223667588.html?sku_id=12000035829027950) или [лазерный](https://aliexpress.ru/item/1005002273898725.html?spm=a2g2w.orderdetail.0.0.18424aa6WVS9s4&sku_id=12000019867321862), предпочтительно NPN NO, но можно любой, так как изменения под другой минимальны.

Сенсор PNP - [фотоэлектрический e3z-R81](https://aliexpress.ru/item/1005005223667588.html?sku_id=12000035829027951) или [лазерный](https://aliexpress.ru/item/1005002273898725.html?sku_id=12000019867321864)

[Ссылка на отражатель](https://aliexpress.ru/item/1005005658259272.html), так как не со всеми датчиками идет в комплекте

Так же скорее всего подойдут следующие датчики - [Link1](https://aliexpress.ru/item/1005004251139454.html?sku_id=12000028524452661), [Link2](https://aliexpress.ru/item/1005005325512127.html?sku_id=12000032633027413)

ESP32s2 - [Lolin S2 mini](https://aliexpress.ru/item/1005003145192016.html?sku_id=12000024338777694) или ESP32s3 - [Lolin S3 mini](https://aliexpress.ru/item/1005006646247867.html)


[DC-DC]([https://aliexpress.ru/item/1005002525645136.html](https://aliexpress.ru/item/1005001578810552.html?sku_id=12000016666979656)) или [этот](https://aliexpress.ru/item/10000000656280.html?sku_id=20000000002551824), а можно и  [этот](https://aliexpress.ru/item/1005003249843639.html?sku_id=12000024854854504)

OLED I2C display [2.4"](https://aliexpress.ru/item/1005005867315821.html?sku_id=12000034633566861) или [2.4"](https://aliexpress.ru/item/1005001565591155.html), [1.54"](https://aliexpress.ru/item/1005005317314760.html?sku_id=12000032606581542), [1.3"](https://aliexpress.ru/item/1005005438758215.html?sku_id=12000033085858096), [0.96"](https://aliexpress.ru/item/1005001603008505.html?sku_id=12000016754783852)

Для PNP сенсора пара резисторов необходимы в виде делителся напряжения с 12В до 3В, в схеме используется на 10кОм и 3.3кОм, при этом 3.3кОм вкчючается между GND и 6 пином на ESP.

Кнопки можно использовать любые нормально открытые кнопки, как вариант тактовые кнопки.

Аккумулятор использовал любой именющийся в наличии формата 18650, литий-ионный или повербанк.
Подобрать корпус и хорошо зафиксировать, фотки собранного устройства появяться чуть позже как будет выглядить уже как завершенное устройство.

### Принцип работы

Таймер стартует при первом прохождении линии сенсора и не может быть остановлен быстрее чем за 3 секунды(в дальнейшем будет возможность изменения этого времени). Финиш фиксируется так же при первом пересечении линии.

~~Кнопки на данный момент заложены для будующих функций - изменения времени следующего срабоатывания датчика, изменение алгоритма работы(при каждом срабатывании новый круг, как у китайской телеметрии или старт и финиш).~~

*По функционалу, что изменилось и добавилось:*
- При нажатии на кнопки на дисплее должно отображаться какую кнопку нажал - "1", "2" или "3"
- Для смены шрифта нужно однократно нажать на кнопку "1", шрифтов 3 с большими цифрами и 1 стандартный вместе с ним отображается название ТК и пароль для подключения к ней
- Для просмотра "LastTime" - однократное нажатие "2"
- Для смены режима работы(старт-стоп или лаптаймер) - нужно нажать и держать "3" и однократно нажать "1"
- Для смены режима WiFi раздача ТК или подключение - нужно нажать и держать "3" и однократно нажать "2"

Пересечение линии дублируется строчкой об ошибки сенсора на экране, так как эта "ошибка" сигнализирует о наличии препятствия перед сенсором, о пересечении линиии или не правильной установки сенсора и обратной его сторона(отражателя)

Для изменения имени Точки досупа и пароля необходимо изменить переменные в файле "SSID_server.h", обязательно должно быть первым в списке, все что будет указано далее в массиве, то с такими данными телеметрия будет пытататься подключить в случае смены режима - раздача или подключение, при этом в одном из вариантов шрифта будет вывод к какой сети подключены ворота и следующей строчкой будет выведен IP адрес выданный для телеметрии.

Для подключения с телефона\ноутбука\etc. нужно подключиться к точки доступа "StopWatcher" (пароль по умолчанию - StopWatcher) или с даннсыми изменеными ранее в файле "SSID_server.h", андроид может оповестить, что при подключении к этой точки доступа нет доступа к интернету - необходимо поставить галочку - "Больше не спрашивать" и нажать "Да" соглашаясь с подключеним или заранее отключить мобильный интернет на телефоне. Как все происходит на iPhone я не знаю точно и не тестировал, но должно быть аналогично. Далее заходим в браузер и переходим по адресу http://192.168.4.1/ , сразу после загрузки страницы будет отображение времени. Так же в прошивке есть DNS, так что можно в строке ввода url можно ввести любые символы и "/" - например "str675/" и также должны загрузиться страничка телеметрии.
