 // Replace with your network credentials
const char *ssid[]      = { "StopWatcher", "StopWatcher02" };   // список возможных сетей
const char *password[]  = { "StopWatcher", "StopWatcher02" };   // список паролей к сетям
const bool wsSSL[]      = { false, false };                     // если вебсокеты работают через SSL - true, обычно false
const char *wsHost[]    = { "192.168.4.1", "192.168.4.1" };     // адрес таймера
const char *wsPath[]    = { "/ws", "/ws" };                     // путь к вебсокетам
const int wsPort[]      = { 80, 80 };                           // порт вебсокетов
uint8_t wifi_id = 0;
