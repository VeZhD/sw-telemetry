const char index_html[] PROGMEM = { R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>MG StopWatcher</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" type="image/png" href="/favicon.ico">

  <script src="/NoSleep.min.js"></script>
  <script src="/script.js"></script>

  <style>
  html {
    font-family: "Consolas";
    text-align: center;
  }
  body {
    color: white;
    margin: 0;
    background: black;
  }
  .content {
    display: flex;
    width: 100vw;
    height: 100vh;
    align-content: center;
    justify-content: center;
    align-items: center;
    flex-direction: row;
  }
  .card {
    color: white;
    padding-top:10px;
    padding-bottom:20px;
    user-select: none;
    display: flex;
    flex-direction: column;
  }
  .top-time {
    font-size: 10vw;
    position: relative;
  }
  .time {
    position: relative;
    font-size: 25vw;
  }
  .config{
    font-family: fantasy;
    font-size: 3vw;
    color: white;
    position: absolute;
    padding: 1vw;
    border: 1px solid white;
    top: 10px;
    left: 10px;
    cursor: pointer;
    opacity: 0.3;
    user-select: none;

  }
  .m-b {
    display: block;
    margin-bottom: 15px;
  }
  .p-a {
    padding: 5px;
  }
  .top-toolbar,
  .bottom-toolbar {
    width: 100%;
  }
  .modal {
    width: 100%;
    padding: 15px;
    margin: 15px auto;
    text-align: left;
    box-sizing: border-box;
  }
  .container {
    border: 1px solid #676767;
    padding: 5px 10px;
  }
  .flex{
    display: flex;
  }
  .wrap {
    flex-wrap: wrap;
  }
  .between {
    justify-content: space-between;
  }
  .ss{font-family: sans-serif;}

  .btn {
    background: #0044ad;
    border-radius: 5px;
    color: white;
    border: none;
    padding: 3px 10px;
  }
  .btn.btn-success {
    background: #0e970e;
  }
  .btn.btn-danger {
    background: #a90000;
  }
  .ebtn {
    position: absolute;
    top: 0;
    display: block;
    border: 1px solid white;
    color: white;
    width: 24px;
    height: 24px;
    font-size: 20px;
    line-height: 24px;
    background: black;
  }
  .ebtn.plus {
    left: calc( 50% - 40px );
  }
  .ebtn.plus:before {
    content: "+";
  }
  .ebtn.minus {
    right: calc( 50% - 40px );
  }
  .ebtn.minus:before {
    content: "-";
  }
  .f-col{
    flex-direction: column;
  }
  .f-row{
    flex-direction: row;
  }

  .fbtn {
    font-family: fantasy;
    font-size: 14px;
    color: white;
    padding: 5px 10px;
    border: 1px solid white;
    user-select: none;
    cursor: pointer;
    opacity: 0.3;
    margin-bottom: 15px;
  }
</style>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div id="sw">
    <template v-if="isConfig">
      <div class="modal">
        <h2>Конфигурация таймера</h3>
        <div class="container m-b">
          <label class="m-b">
            Режим таймера
          </label>
          <div>
            <select v-model="configData.timer.mode">
              <option value="ss">Старт-стоп</option>
              <option value="lt">Лап-таймер</option>
            </select>
          </div>
          <label class="m-b">
            Задержка отображения результата в режиме кругового таймера (мс)
          </label>
          <div>
            <input type="text" v-model="configData.timer.printDelay" />
          </div>
          <label class="m-b">
            задержка срабатывания на луч (мс)
          </label>
          <div>
            <input type="text" v-model="configData.timer.stopDelay" />
          </div>
        </div>

        <div class="container m-b">
          <label class="m-b">Список сетей</label>
          <div v-for="(item, idx) in configData.wifi.list" class="container m-b">
            <div class="flex between">
              <label class="m-b">Сеть #{{ idx+1 }}</label>        
              <button v-if="idx" class="btn btn-danger" @click.prevent="delListItem(idx)">удалить</button>
            </div>
            <div class="flex wrap">
              <div class="m-b">
                <label>Режим WiFi</label>        
                <select v-model="item.mode" :disabled="!idx">
                  <option value="server">Server</option>
                  <option value="client">Client</option>
                </select>
              </div>
              <div class="m-b">
                <label>ssid</label>        
                <input v-model="item.ssid" type="text" />
              </div>
              <div class="m-b">
                <label>pass</label>        
                <input v-model="item.pass" type="text" />
              </div>
            

            </div>
          </div>
          <div>
            <button class="btn" @click.prevent="addListItem">добавить</button>
          </div>
        </div>
        <div class="container m-b">
          <label class="m-b">
            Тип датчика
          </label>
          <div class="flex">
            <select v-model="configData.sensor.type">
              <option value="pnp">PNP</option>
              <option value="npn">NPN</option>
            </select>
            <select v-model="configData.sensor.gate">
              <option value="no">Нормально разомкнутый</option>
              <option value="nc">Нормально замкнутый</option>
            </select>
          </div>
        </div>
        <div class="m-b flex between">
          <button class="btn btn-success" @click.prevent="storeConfig">Сохранить</button>
          <button class="btn btn-danger" @click.prevent="cancel">Отмена</button>
        </div>
      </div>
    </template>
    <template v-else>
      <div class="content between">
        <div class="top-toolbar">
          <div class="flex between f-col p-a">
            <div class="fbtn" @click.prevent="() => {mode = mode == 'ws' ? 'get' : 'ws' }">
              {{ mode == 'ws' ? 'WS' : 'GET' }}
            </div>
            <div class="fbtn" @click.prevent="toggleEdit">
              edit
            </div>
            <div class="fbtn" @click="getConfig">
              config
            </div>
            <div class="fbtn" @click="setFs">
              FullScreen
            </div>
          </div>
        </div>
        <div class="card">
          <div class="time" :style="{fontSize: editor.time.font + 'vw', lineHeight: (editor.time.font * 0.9 ) + 'vw'}">
            {{tm}}<span class="ss">:</span>{{ts}}<span class="ss">.</span>{{tms}}
            <div v-if="editor.isEdit" class="ebtn plus" @click="fontPlus(editor.time)"></div>
            <div v-if="editor.isEdit" class="ebtn minus" @click="fontMinus(editor.time)"></div>
          </div>
          <div class="top-time" :style="{fontSize: editor.topTime.font + 'vw', lineHeight: (editor.topTime.font * 0.9) + 'vw'}">
            {{ttm}}<span class="ss">:</span>{{tts}}<span class="ss">.</span>{{ttms}}
            <div v-if="editor.isEdit" class="ebtn plus" @click="fontPlus(editor.topTime)"></div>
            <div v-if="editor.isEdit" class="ebtn minus" @click="fontMinus(editor.topTime)"></div>
          </div>
        </div>
        <div class="bottom-toolbar">
          <div class="messages" v-text="msg">
          </div>      
        </div>

      </div>      
    </template>
  </div>  
<script>
  new Vue({
    el: '#sw',
    data: {
      ct: 0,
      gs: 0,
      cs: 0,
      topTime: 0,
      msg: '',
      ws: null,
      timer: null,
      mode: 'ws',

      editor: {
        isEdit: false,
        time: {
          font: 23,
        },
        topTime: {
          font: 10,
        }
      },

      configData: {
        timer: {
          mode: '',
          font_id: 1,
          printDelay: 10000,
          stopDelay: 1650,
        },
        wifi: {
          wifiid: 0,
          list: [],
        },
        sensor: {
          type: '',
          gate: '',
        }
      },
      isConfig: false,
    },
    computed: {
      tm(){
        return String(Math.floor(this.ct/60000)).padStart(1,'0');
      },
      ts(){
        return String(Math.floor(this.ct/1000) % 60).padStart(2, '0');
      },
      tms(){
        return String(Math.floor(this.ct) % 1000).padStart(3, '0');
      },
      ttm(){
        return String(Math.floor(this.topTime/60000)).padStart(1,'0');
      },
      tts(){
        return String(Math.floor(this.topTime/1000) % 60).padStart(2, '0');
      },
      ttms(){
        return String(Math.floor(this.topTime) % 1000).padStart(3, '0');
      }//,
      //sc(){
      //  return parseInt(this.gs) ? 'state-green' : 'state-red';
      //}
    },
    methods: {
      toggleEdit(){
        this.editor.isEdit = !this.editor.isEdit;
        this.saveSettings();
      },
      saveSettings(){
        localStorage.setItem('timeFont', this.editor.time.font);
        localStorage.setItem('topTimeFont', this.editor.topTime.font);
      },
      getSettings(){
        this.editor.time.font = localStorage.getItem('timeFont') || 23;
        this.editor.topTime.font = localStorage.getItem('topTimeFont') || 10;
      },
      fontPlus(time){
        time.font = Math.min(time.font + 1, 23);
      },
      fontMinus(time){
        time.font = Math.max(time.font - 1, 1);
      },
      addListItem(){
        this.configData.wifi.list.push({ssid: '', pass: '', mode: 'client'});
      },
      delListItem(idx){
        this.configData.wifi.list.splice(idx, 1);
      },
      getTopTime(){
          const xhr = new XMLHttpRequest();
          xhr.timeout = 250;
          xhr.responseType = "text";
          xhr.onload = () => {
              if (xhr.readyState == 4 && xhr.status == 200) {
                  let data = xhr.response;
                  this.topTime = parseInt(data);
              }
          };
          xhr.open("GET", "/toptime");
          xhr.send();
      },
      storeConfig(){
        const xhr = new XMLHttpRequest();
          xhr.open("POST", "/config");
          xhr.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
          xhr.send(JSON.stringify(this.configData));
          this.isConfig = false;
      },
      cancel(){
        this.isConfig = false;
      },
      getConfig(){
        if (document.fullscreenElement){
          document.exitFullscreen();          
        }
        const xhr = new XMLHttpRequest();
          xhr.timeout = 250;
          xhr.responseType = "json";
          xhr.onload = () => {
              if (xhr.readyState == 4 && xhr.status == 200) {
                  //let cfg = JSON.parse(xhr.response);
                  let cfg = xhr.response;
                  if (cfg.wifi){
                    if (cfg.wifi.wifiid){
                      this.configData.wifi.wifiid = cfg.wifi.wifiid;
                    }
                    if (cfg.wifi.list){
                      if (cfg.wifi.list.length){
                        this.configData.wifi.list = cfg.wifi.list;
                      } else {
                        this.configData.wifi.list = [{
                          ssid: 'StopWatcher',
                          pass: 'StopWatcher',
                          mode: 'server',
                        }];
                      }
                    }
                  }
                  if (cfg.sensor){
                    if (cfg.sensor.type){
                      this.configData.sensor.type = cfg.sensor.type;
                    }
                    if (cfg.sensor.gate){
                      this.configData.sensor.gate = cfg.sensor.gate;
                    }
                  }
                  if (cfg.timer){
                    if (cfg.timer.mode){
                      this.configData.timer.mode = cfg.timer.mode;
                    }
                    if (cfg.timer.font_id){
                      this.configData.timer.font_id = cfg.timer.font_id;
                    }
                    if (cfg.timer.printDelay){
                      this.configData.timer.printDelay = cfg.timer.printDelay;
                    }
                    if (cfg.timer.stopDelay){
                      this.configData.timer.stopDelay = cfg.timer.stopDelay;
                    }
                  }
                  this.isConfig = true;
              }
          };
          xhr.open("GET", "/config");
          xhr.send();
      },
      initScreenLock(){
        try{
          let wakeLock = null;
          wakeLock = navigator.wakeLock.request('screen');
          wakeLock.then( () => {
            console.log('Wake Lock is active!');             
          }).catch( (err) => {
            console.log(err);
          })           
        } catch (err){
          console.log(err);
        }
      },
      load(){
          const xhr = new XMLHttpRequest();
          xhr.timeout = 250;
          xhr.ontimeout = (e) => {
            this.load()
            };
          xhr.responseType = "text";
          xhr.onload = () => {
              if (xhr.readyState == 4 && xhr.status == 200) {
                  let data = xhr.response;
                  this.ct = parseInt(data.substr(2));
                  this.gs = parseInt(data.substr(0,1));
                  this.cs = parseInt(data.substr(1,1));
                  if (this.mode == 'get'){
                    setTimeout(() => {this.load();}, this.successInterval);
                  }
              } else {
                  console.log(`Error: ${xhr.status}`);
                  if (this.mode == 'get'){
                    setTimeout(() => {this.load();}, this.failInterval);                    
                  }
              }
          };
          xhr.onabort = () => {
            if (this.mode == 'get'){
              setTimeout(() => {this.load();}, this.failInterval);
            }
          }
          xhr.onerror = () => {
            if (this.mode == 'get'){
              setTimeout(() => {this.load();}, this.failInterval);
            }
          }

          xhr.open("GET", "/laptime");
          xhr.send();
      },
      setFs(){
        let elem = document.getElementById('sw');
        var noSleep = new NoSleep();
        if (!document.fullscreenElement) {
            noSleep.enable(); // keep the screen on!
        if (elem.requestFullscreen) {
          elem.requestFullscreen();
        } else if(elem.webkitRequestFullscreen) {
          elem.webkitRequestFullscreen();
        } else if(elem.msRequestFullscreen){
         elem.msRequestFullscreen();
        }}
        else {
           document.exitFullscreen();
        }
      },
      initWs(){
        var gateway = `ws://${window.location.hostname}/ws`;
        this.ws = new WebSocket(gateway);
        this.ws.onopen    = this.wsOnOpen;
        this.ws.onclose   = this.wsOnClose;
        this.ws.onmessage = this.wsOnMessage; // <-- add this line
      },
      wsOnOpen(){
        console.log('Connection opened');
        this.ws.send('toggle');
      },
      wsOnClose(){
        if (this.mode == 'ws'){
          setTimeout(this.initWs(), 2000);                  
        }
        console.log('Connection closed');
      },
      wsOnMessage(){
        if (event.data && this.mode == 'ws'){
          this.ct = parseInt(event.data.substr(2));
          this.gs = parseInt(event.data.substr(0,1));
          this.cs = parseInt(event.data.substr(1,1));
        }
      },
    },
   watch: {
      cs(nv, ov){
        if (nv && !ov){
          this.timer = setInterval(() => {
            this.ct = this.ct + 100;
          }, 100);
        } else if (!nv && ov){
          clearInterval(this.timer);
          this.getTopTime();
        }
      },
      mode(nv, ov){
        if (nv == 'ws'){
          this.initWs();
        } else {
          this.load();
        }
      }
    },
    mounted(){
      this.getSettings();
      if (this.mode == 'ws'){
        this.initWs();
      } else {
        this.load();
      }

      setInterval(this.getTopTime, 5000);
      this.initScreenLock();
    }
  })

</script>
</body>
</html>
)rawliteral"};
