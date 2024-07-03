const char index_html_ws[] PROGMEM = { R"rawliteral(
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
    <div class="fullscreen" @click="setFs">
      FullScreen
    </div>
    <div class="messages" v-text="msg">
    </div>
  </div>  
<script>
  new Vue({
    el: '#sw',
    data: {
      ct: 0,
      gs: 0,
      cs: 0,
      msg: '',
      ws: null,
      timer: null,
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
      }//,
      //sc(){
      //  return parseInt(this.gs) ? 'state-green' : 'state-red';
      //}
    },
    methods: {
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
        setTimeout(this.initWs(), 2000);        
        console.log('Connection closed');
      },
      wsOnMessage(){
        if (event.data){
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
        }
      }
    },
    mounted(){
      var websocket;
      this.initWs();
      this.initScreenLock();
    }
  })

</script>
</body>
</html>
)rawliteral"}; //"
