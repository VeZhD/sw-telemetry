const char index_toptime[] PROGMEM = { R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>MG StopWatcher</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" type="image/png" href="/favicon.ico">
  <script src="/NoSleep.min.js"></script>
  <script src="/script.js"></script>
  <style>
  html {
    font-family: monospace;
    text-align: center;
  }
  body {
    margin: 0;
    background: black;
  }
  p{
    margin: 0;
    padding: 0;
  }
  .content {
    display: flex;
    width: 100vw;
    height: 100vh;
    align-content: center;
    justify-content: center;
    align-items: center;
    flex-direction: column;
  }
  .card {
    color: white;
    padding-top:10px;
    padding-bottom:20px;
    font-size: 25vw;
    user-select: none;
  }
  .indicator {
    width: 9vw;
    height: 9vw;
    display: inline-block;
    border-radius: 500px;
    background: yellow;
    user-select: none;
  }
  .indicator.state-green {
    background: lime;
  }
  .fullscreen {
    font-family: fantasy;
    font-size: 3vw;
    color: white;
    position: absolute;
    padding: 1vw;
    border: 1px solid white;
    bottom: 10px;
    right: 10px;
    cursor: pointer;
    opacity: 0.3;
    user-select: none;
  }
  .ss{font-family: sans-serif;}
</style>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div id="sw" class="content">
    <div class="card">
      <p>{{tm}}<span class="ss">:</span>{{ts}}<span class="ss">.</span>{{tms}}</p>
    </div>
    <div class="indicators">
      <div class="indicator" :class="sc"></div>
    </div>
    <div class="fullscreen" @click="setFs">
      FullScreen
    </div>
    <div class="messages" v-text="msg">
    </div>
  </div>  
<script>
  let app = new Vue({
    el: '#sw',
    data: {
      ct: 0,
      gs: 0,
      cs: 0,
      msg: '',
      ws: null,
      timer: null,
      successInterval: 50,
      failInterval: 250,
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
      sc(){
        return parseInt(this.gs) ? 'state-green' : 'state-red';
      }
    },
    methods: {
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
                  setTimeout(() => {this.load();}, this.successInterval);
              } else {
                  console.log(`Error: ${xhr.status}`);
                  setTimeout(() => {this.load();},
                   this.failInterval);
              }
          };
          xhr.onabort = () => {
              setTimeout(() => {this.load();}, this.failInterval);
          }
          xhr.onerror = () => {
              setTimeout(() => {this.load();}, this.failInterval);
          }

          xhr.open("GET", "/laptime");
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
    },
    mounted(){
      this.load();
      this.initScreenLock();
    }
  })

</script>
</body>
</html>
)rawliteral"};
