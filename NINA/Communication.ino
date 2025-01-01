void initWifi() {
  // set friendly hostname
  WiFi.setHostname(hostname.c_str());

  if (WiFiAP) {
    // enable WiFi in accesspoint-mode
    WiFi.mode(WIFI_AP);

    // configure WiFi-accesspoint to desired IP-configuration and SSID
    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(ssid.c_str(), password.c_str()); // WiFi.softAP(ssid, password, APCHAN)
  } else {
    // enable WiFi as client (station)
    WiFi.mode(WIFI_STA);

    // if desired, use static IP
    #if USE_STATICIP == 1
      WiFi.config(ip, gateway, subnet, primaryDNS, secondaryDNS);
    #endif
    WiFi.begin(ssid.c_str(), password.c_str());

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }

  if (MDNS.begin(hostname)) {
    MDNS.addService("http", "tcp", 80);
  }
}

void handleUartCommunication() {
  String command;

  if (Serial.available() > 0) {
    command = Serial.readStringUntil('\n');
    command.trim();

    if (command.indexOf("SAMD: OK") > -1) {
      // received "OK" from SAMD -> last command is OK
    }else if (command.indexOf("SAMD: ERROR") > -1) {
      // received "ERROR" from SAMD -> last command had an error
    }else if (command.indexOf("SAMD: UNKNOWN_CMD") > -1) {
      // SAMD received some garbage or communication-problem -> ignore it for the moment
    }else if (command.indexOf("fpga:") == 0) {
      if (command.indexOf("ver?") > -1){
        // return version of fpga
        Serial.println("v" + FPGA_Version);
      }else if (command.indexOf("*IDN?") > -1){
        // return info-string of fpga
        Serial.println("X-f/bape FPGA v" + String(FPGA_Version));
      }else{
        // this is a command for the FPGA
        Serial1.print(command);
        Serial.println("OK"); // FPGA will not return values instead of the version up to now
      }
    }else{
      // this is a command for our command-processor
      Serial.println(executeCommand(command));
    }
  }
}

// FPGA-Receiver
uint16_t fpgaRingBufferPointerOverflow(uint16_t bufPointer) {
  if (bufPointer>=SCI_RINGBUF_LEN) {
    return (bufPointer-SCI_RINGBUF_LEN);
  }else{
    return bufPointer;
  }
}

void fpgaSearchCmd() {
  uint16_t i;
  uint16_t j;
  uint16_t ErrorCheckData;
  uint16_t payloadSum;
  uint8_t newData[SCI_PAYLOAD_LEN];

  for (i = 0; i < (SCI_RINGBUF_LEN + SCI_CMD_LEN - 1); i++) { // increment (SCI_CMD_LEN-1) beyond SCI_RINGBUF_LEN to catch commands around the border
    if ((fpgaRingBuffer[fpgaRingBufferPointerOverflow(i)] == 65) && // check for "A"
            (fpgaRingBuffer[fpgaRingBufferPointerOverflow(i + SCI_CMD_LEN - 1)] == 69)) // check for "E"
    {
      // check error-check-byte
      payloadSum = 0;
      for (j=i+1; j<=(i + SCI_PAYLOAD_LEN); j++) { // summarize all values between "A" and "E"
        payloadSum += fpgaRingBuffer[fpgaRingBufferPointerOverflow(j)];
      }
      uint16_t ErrorCheckData = (fpgaRingBuffer[fpgaRingBufferPointerOverflow(i + SCI_CMD_LEN - 3)] << 8) + fpgaRingBuffer[fpgaRingBufferPointerOverflow(i + SCI_CMD_LEN - 2)];

      if (payloadSum == ErrorCheckData) {
        // we received valid data

        // copy data to newData-array
        for (j=0; j<(SCI_PAYLOAD_LEN); j++) {
            newData[j] = fpgaRingBuffer[fpgaRingBufferPointerOverflow(i+1+j)];
        }

        // newData[0] contains version of FPGA * 100
        if (FPGA_Version.length() == 1) {
          FPGA_Version = String(newData[0]/100.0f, 2);
          Serial.println("samd:version:fpga@" + FPGA_Version); // send FPGA version to SAMD21
        }else{
          FPGA_Version = String(newData[0]/100.0f, 2);
        }

        audioStatusInfo = 0;
        // newData[1] contains information about the noisegate(s)
        gateStatusInfo = newData[1];
        if (newData[1] & 0b00000001) {
          // audiogate is closed
          audioStatusInfo += 1;
        }

        // newData[2] contains information about the compressors
        compStatusInfo = newData[2];
        if (newData[2] & 0b00000001) {
          // audio left/right is being compressed/limited
          audioStatusInfo += 2;
        }
        if (newData[2] & 0b00000010) {
          // audio subwoofer is being compressed/limited
          audioStatusInfo += 4;
        }

        // newData[3] contains information about clipping
        clipStatusInfo = newData[3];
        if (newData[3] & 0b00000001) {
          // left is clipping
          audioStatusInfo += 8;
        }
        if (newData[3] & 0b00000010) {
          // right is clipping
          audioStatusInfo += 16;
        }
        if (newData[3] & 0b00000100) {
          // sub is clipping
          audioStatusInfo += 32;
        }

        // newData[4] ... newData[6] contain VU-meter information for left, right and subwoofer
        for (int i=0; i<3; i++) {
          // we are taking only larger values than the current value
          uint8_t new_value = log2(newData[i+4] + 1) * (255/8);

          if (new_value > audiomixer.vuMeterMain[i]) {
            // convert bit-values to a linear scale
            audiomixer.vuMeterMain[i] = new_value; // direct update
            //audiomixer.vuMeterMain[i] = audiomixer.vuMeterMain[i] + (new_value - audiomixer.vuMeterMain[i])*0.2f; // slow update -> smoother transition
          }else{
            // decay the vu-meter with the given slope
            audiomixer.vuMeterMain[i] = audiomixer.vuMeterMain[i] * vumeter_decay;
          }
        }

        // newData[7] ... newData[38] contain VU-meter information for ch1 .. ch32
        for (int i=0; i<32; i++) {
          // we are taking only larger values than the current value
          uint8_t new_value = log2(newData[i+7] + 1) * (255/8);

          if (new_value > audiomixer.vuMeterCh[i]) {
            // convert bit-values to a linear scale
            audiomixer.vuMeterCh[i] = new_value; // direct update
            //audiomixer.vuMeterCh[i] = audiomixer.vuMeterCh[i] + (new_value - audiomixer.vuMeterCh[i])*0.2f; // slow update -> smoother transition
          }else{
            // decay the vu-meter with the given slope
            audiomixer.vuMeterCh[i] = audiomixer.vuMeterCh[i] * vumeter_decay;
          }
        }
      }else{
        // wrong error-check-data
      }

      // set array-elements to zero as we have executed the command
      for (j=i; j<(i + SCI_CMD_LEN); j++) {
        fpgaRingBuffer[fpgaRingBufferPointerOverflow(j)] = 0;
      }
    }else{
      // wrong begin or end
    }
  }
}

void handleFPGACommunication() {
  if (Serial1.available() > 0) {
    uint8_t rxChar = Serial1.read();

    if (debugRedirectFpgaSerialToUSB) {
      // redirect all incoming bytes to SAMD-processor
      Serial.write(rxChar);
    }

    // FPGA sends 20 messages per second
    // read new byte into ringbuffer
    fpgaRingBuffer[fpgaRingBufferPointer++] = rxChar;

    if (fpgaRingBufferPointer>=SCI_RINGBUF_LEN) {
      // reset to beginning of sciRingbuffer
      fpgaRingBufferPointer = 0;
    }
    // check for complete message if we receive expected end of message
    if (rxChar == 69) { // received an 'E'
      fpgaSearchCmd();
    }
  }
}

#if USE_TCPSERVER == 1
  void handleTCPCommunication() {
    // listen for incoming clients
    if (cmdclient) {
      // client is connected

      // check if we have some data to receive
      if (cmdclient.available() > 0) {
        String command = cmdclient.readStringUntil('\n');
        command.trim();

        // execute command with our command-processor and return the answer to client
        cmdclient.println(executeCommand(command));
      }
    }else{
      // no current client connected. Try to catch a new connection
      cmdclient = cmdserver.available();
    }
  }
#endif

#if USE_MQTT == 1
  void initMQTT() {
    // configure MQTT-client/server-connection
    mqttclient.setServer(mqtt_server, mqtt_serverport);
    mqttclient.setCallback(mqttCallback);

    // connect to MQTT-broker
    if (mqttclient.connect(mqtt_id)) {
      // we have a working connection

      // subscribe to the desired topics
      mqttclient.subscribe("fbape/wifi/ssid");
      mqttclient.subscribe("fbape/wifi/password");
      mqttclient.subscribe("fbape/wifi/ip");
      mqttclient.subscribe("fbape/wifi/gateway");
      mqttclient.subscribe("fbape/wifi/subnet");
      mqttclient.subscribe("fbape/wifi/mode");
      mqttclient.subscribe("fbape/player/play");
      mqttclient.subscribe("fbape/player/loop");
      mqttclient.subscribe("fbape/player/pause");
      mqttclient.subscribe("fbape/player/stop");
      mqttclient.subscribe("fbape/player/position");
      mqttclient.subscribe("fbape/player/volume");
      mqttclient.subscribe("fbape/mixer/volume/main");
      mqttclient.subscribe("fbape/mixer/balance/main");
      mqttclient.subscribe("fbape/mixer/volume/sub");
      for (uint8_t i=1; i<=MAX_AUDIO_CHANNELS; i++) {
        mqttclient.subscribe(String("fbape/mixer/volume/ch" + String(i)).c_str());
      }
      mqttclient.subscribe("fbape/mixer/reset");
      mqttclient.subscribe("fbape/mixer/eq/reset");
      mqttclient.subscribe("fbape/mixer/dynamics/reset");
      mqttclient.subscribe("fbape/mixer/eq/peq");
      mqttclient.subscribe("fbape/mixer/eq/lp");
      mqttclient.subscribe("fbape/mixer/eq/hp");
      mqttclient.subscribe("fbape/mixer/adc/gain1");
      mqttclient.subscribe("fbape/mixer/gate1");
      mqttclient.subscribe("fbape/mixer/comp1");
      mqttclient.subscribe("fbape/mixer/comp2");
      mqttclient.subscribe("fbape/mixer/sync");
    }
  }

  void handleMQTT() {
    // check if we are connected to WiFi
    if (WiFi.status() == WL_CONNECTED) {
      if (!mqttclient.connected()) {
        // reconnect to MQTT-broker
        initMQTT();
      }else{
        // we connected to MQTT-broker
        mqttclient.loop(); // process incoming messages
      }
    }
  }

  void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
    payload[length] = '\0'; // null-terminate byte-array

    // convert MQTT-message "str/str/str/str payload" to command "str:str:str:str@payload"
    String sTopic = String(topic);
    sTopic.replace("/", ":");
    executeCommand(sTopic + "@" + String((char*)payload));
  }

  void mqttPublish() {
    if ((WiFi.status() == WL_CONNECTED) && mqttclient.connected()) {
      #if USE_SDPLAYER == 1
        uint32_t audioPosition = audio.getAudioCurrentTime();
        uint32_t audioDuration = audio.getAudioFileDuration();
        mqttclient.publish("fbape/status/player/playing", String(audio.isRunning()).c_str());
        mqttclient.publish("fbape/status/player/filesize", String(audio.getFileSize()).c_str());
        mqttclient.publish("fbape/status/player/duration", String(audioDuration).c_str());
        mqttclient.publish("fbape/status/player/position", String(audioPosition).c_str());
        if (audioDuration > 0) {
          mqttclient.publish("fbape/status/player/progress", String(((float)audioPosition/(float)audioDuration)*100.0f, 1).c_str());
        }else{
          mqttclient.publish("fbape/status/player/progress", "0");
        }
      #endif
      mqttclient.publish("fbape/status/player/currentfile", currentAudioFile.c_str());

      mqttclient.publish("fbape/status/mixer/vumeter/left", String(audiomixer.vuMeterMain[0]).c_str());
      mqttclient.publish("fbape/status/mixer/vumeter/right", String(audiomixer.vuMeterMain[1]).c_str());
      mqttclient.publish("fbape/status/mixer/vumeter/sub", String(audiomixer.vuMeterMain[2]).c_str());

      mqttclient.publish("fbape/status/mixer/volume/main", String(audiomixer.mainVolume).c_str());
      mqttclient.publish("fbape/status/mixer/volume/sub", String(audiomixer.mainVolumeSub).c_str());
      mqttclient.publish("fbape/status/mixer/volume/sdcard", String(audiomixer.cardVolume).c_str());
      #if USE_BLUETOOTH == 1
        mqttclient.publish("fbape/status/mixer/volume/bt", String(audiomixer.btVolume).c_str());
      #endif
      
      /*
      // TODO: here we could publish more data about individual channel-volumes, EQs, Gates, Compressors, etc.
      mqttclient.publish("fbape/status/mixer/volume/ch1", String(audiomixer.chVolume[0]).c_str());
      ...
      mqttclient.publish("fbape/status/mixer/volume/ch32", String(audiomixer.chVolume[31]).c_str());
      */
    }
  }
#endif

// command-interpreter
String executeCommand(String Command) {
  String Answer;

  if (Command.length()>2){
    // we got a new command. Lets find out what we have to do today...

    // enable red LED and set HoldCounter to 3 = 300ms
    ledcFade(LED_BLUE, 255, 0, 500); // pin, StartDutyCycle, TargetDutyCycle, FadeTime in ms

    if (Command.indexOf("wifi:ssid") > -1){
      // we received "wifi:ssid@TEXT"
      ssid = Command.substring(Command.indexOf("@")+1);
      ssid.trim(); // remove CR, LF and spaces

      Answer = "OK";
    }else if (Command.indexOf("wifi:password") > -1){
      // we received "wifi:password@TEXT"
      password = Command.substring(Command.indexOf("@")+1);
      password.trim(); // remove CR, LF and spaces

      Answer = "OK";
    }else if (Command.indexOf("wifi:ip?") > -1){
      if (WiFiAP)
        Answer = WiFi.softAPIP().toString();
      else
        Answer = WiFi.localIP().toString();
    }else if (Command.indexOf("wifi:ip") > -1){
      // we received "wifi:ip@192.168.0.1"
      String ParameterString = Command.substring(Command.indexOf("@")+1) + ".";
      uint8_t ip0 = split(ParameterString, '.', 0).toInt();
      uint8_t ip1 = split(ParameterString, '.', 1).toInt();
      uint8_t ip2 = split(ParameterString, '.', 2).toInt();
      uint8_t ip3 = split(ParameterString, '.', 3).toInt();
      
      ip = IPAddress(ip0, ip1, ip2, ip3);

      Answer = "OK";
    }else if (Command.indexOf("wifi:gateway") > -1){
      // we received "wifi:gateway@192.168.0.1"
      String ParameterString = Command.substring(Command.indexOf("@")+1) + ".";
      uint8_t ip0 = split(ParameterString, '.', 0).toInt();
      uint8_t ip1 = split(ParameterString, '.', 1).toInt();
      uint8_t ip2 = split(ParameterString, '.', 2).toInt();
      uint8_t ip3 = split(ParameterString, '.', 3).toInt();
      
      gateway = IPAddress(ip0, ip1, ip2, ip3);

      Answer = "OK";
    }else if (Command.indexOf("wifi:subnet") > -1){
      // we received "wifi:subnet@192.168.0.1"
      String ParameterString = Command.substring(Command.indexOf("@")+1) + ".";
      uint8_t ip0 = split(ParameterString, '.', 0).toInt();
      uint8_t ip1 = split(ParameterString, '.', 1).toInt();
      uint8_t ip2 = split(ParameterString, '.', 2).toInt();
      uint8_t ip3 = split(ParameterString, '.', 3).toInt();
      
      subnet = IPAddress(ip0, ip1, ip2, ip3);

      Answer = "OK";
    }else if (Command.indexOf("wifi:mode") > -1){
      // we received "wifi:mode@1" 0=client, 1=accesspoint
      // change WiFi-mode
      WiFiAP = (Command.substring(Command.indexOf("@")+1).toInt() == 1);

      Answer = "OK";
    }else if (Command.indexOf("wifi:enabled") > -1){
      if (Command.substring(Command.indexOf("@")+1).toInt() == 1) {
        // enable WiFi
        initWifi();
        //wifi.resume(); // alternatively
      }else{
        // disable WiFi
        WiFi.mode(WIFI_OFF);
      }
      Answer = "OK";
    #if USE_MQTT == 1
      }else if (Command.indexOf("wifi:mqttserver") > -1){
        // we received "wifi:mqttserver@192.168.0.1"
        String ParameterString = Command.substring(Command.indexOf("@")+1) + ".";
        uint8_t ip0 = split(ParameterString, '.', 0).toInt();
        uint8_t ip1 = split(ParameterString, '.', 1).toInt();
        uint8_t ip2 = split(ParameterString, '.', 2).toInt();
        uint8_t ip3 = split(ParameterString, '.', 3).toInt();
        
        mqtt_server = IPAddress(ip0, ip1, ip2, ip3);

        initMQTT();
        Answer = "OK";
    #endif
    #if USE_SDPLAYER == 1
      }else if (Command.indexOf("player:filesize?") > -1){
        // received command "player:filesize?"
        Answer = String(audio.getFileSize());
      }else if ((Command.indexOf("player:file") > -1) || (Command.indexOf("player:stream") > -1)){
        // received command "player:file@FILENAME.WAV" or "player:stream@http://...."
        currentAudioFile = Command.substring(Command.indexOf("@")+1);
        bool response = false;
        if (currentAudioFile.indexOf("http") > -1){
          // play stream
          response = audio.connecttohost(currentAudioFile.c_str());
        }else{
          // play file from SD-card
          response = audio.connecttoFS(SD, currentAudioFile.c_str()); // play audio-file
        }
        if (response) {
          Answer = "OK";
        }else{
          Answer = "ERROR";
        }
      }else if (Command.indexOf("player:currentfile?") > -1){
        Answer = currentAudioFile;
      }else if (Command.indexOf("player:toc?") > -1){
        Answer = SD_getTOC(1); // send as CSV to USB
      }else if (Command.indexOf("player:samdtoc?") > -1){
        Answer = "samd:toc@" + SD_getTOC(1); // send as CSV to SAMD
      }else if (Command.indexOf("player:play") > -1){
        // received command "player:play"
        bool response = false;
        if (currentAudioFile.indexOf("http") > -1){
          // play stream
          response = audio.connecttohost(currentAudioFile.c_str());
        }else{
          // play file from SD-card
          response = audio.connecttoFS(SD, currentAudioFile.c_str()); // play current audio-file
        }
        if (response) {
          Answer = "OK";
        }else{
          Answer = "ERROR";
        }
      }else if (Command.indexOf("player:loop") > -1){
        // received command "player:loop@X"
        bool looping = (Command.substring(Command.indexOf("@")+1).toInt() == 1);
        if (audio.setFileLoop(looping)) {
          Answer = "OK";
        }else{
          Answer = "ERROR";
        }
      }else if (Command.indexOf("player:pause") > -1){
        // received command "player:pause"
        if (audio.pauseResume()) {
          Answer = "OK";
        }else{
          Answer = "ERROR";
        }
      }else if (Command.indexOf("player:stop") > -1){
        // received command "player:stop"
        if (audio.stopSong()) {
          Answer = "OK";
        }else{
          Answer = "ERROR";
        }
      }else if (Command.indexOf("player:duration?") > -1){
        // received command "player:duration?"
        Answer = String(audio.getAudioFileDuration());
      }else if (Command.indexOf("player:position?") > -1){
        // received command "player:position?"
        Answer = String(audio.getAudioCurrentTime());
      }else if (Command.indexOf("player:position") > -1){
        // received command "player:position@"
        if (currentAudioFile.indexOf("http") > -1){
          Answer = "ERROR: Cannot set position for stream!";
        }else{
          uint32_t position = Command.substring(Command.indexOf("@")+1).toInt();
          if (audio.setAudioPlayPosition(position)) {
            Answer = "OK";
          }else{
            Answer = "ERROR";
          }
        }
      }else if (Command.indexOf("player:percent") > -1){
        // received command "player:percent@0...100"
        if (currentAudioFile.indexOf("http") > -1){
          Answer = "ERROR: Cannot set position for stream!";
        }else{
          uint32_t position = (audio.getAudioFileDuration() * Command.substring(Command.indexOf("@")+1).toInt()) / 100;
          if (audio.setAudioPlayPosition(position)) {
            Answer = "OK";
          }else{
            Answer = "ERROR";
          }
        }
      }else if (Command.indexOf("player:volume") > -1){
        // player:volume@-140dBfs...0dBfs
        audio.setVolume(pow(10, Command.substring(Command.indexOf("@")+1).toFloat()/20.0f) * 21.0f);
        Answer = "OK";
    #endif
    #if USE_BLUETOOTH == 1
      }else if (Command.indexOf("bt:enabled") > -1){
        if (Command.substring(Command.indexOf("@")+1).toInt() == 1) {
          // enable bluetooth
          initBluetooth();
        }else{
          // disable bluetooth
          a2dp_sink.end(); // call with true to free memory, but with no chance to restart bluetooth
        }
        Answer = "OK";
      }else if (Command.indexOf("bt:play") > -1){
        // received command "bt:play"
        a2dp_sink.play();
        Answer = "OK";
      }else if (Command.indexOf("bt:pause") > -1){
        // received command "bt:pause"
        a2dp_sink.pause();
        Answer = "OK";
      }else if (Command.indexOf("bt:stop") > -1){
        // received command "bt:stop"
        a2dp_sink.stop();
        Answer = "OK";
      }else if (Command.indexOf("bt:next") > -1){
        // received command "bt:next"
        a2dp_sink.next();
        Answer = "OK";
      }else if (Command.indexOf("bt:prev") > -1){
        // received command "bt:prev"
        a2dp_sink.previous();
        Answer = "OK";
      }else if (Command.indexOf("bt:ff") > -1){
        // received command "bt:ff"
        a2dp_sink.fast_forward();
        Answer = "OK";
      }else if (Command.indexOf("bt:rw") > -1){
        // received command "bt:rw"
        a2dp_sink.rewind();
        Answer = "OK";
    #endif
    }else if (Command.indexOf("mixer:volume:main") > -1){
      // received command "mixer:volume:main@value"
      audiomixer.volumeMain = Command.substring(Command.indexOf("@")+1).toFloat();
      sendVolumeToFPGA(0); // update main-channel
      Answer = "OK";
    }else if (Command.indexOf("mixer:balance:main") > -1){
      // received command "mixer:balance:main@value"
      audiomixer.balanceMain = Command.substring(Command.indexOf("@")+1).toInt();
      sendVolumeToFPGA(0); // update main-channel
      Answer = "OK";
    }else if (Command.indexOf("mixer:volume:sub") > -1){
      // received command "mixer:volume:sub@value"
      audiomixer.volumeSub = Command.substring(Command.indexOf("@")+1).toFloat();
      sendVolumeToFPGA(0); // update main-channel
      Answer = "OK";
    }else if (Command.indexOf("mixer:volume:ch") > -1){
      // received command "mixer:volume:chxx@value"
      uint8_t channel = Command.substring(15, Command.indexOf("@")).toInt();
      float value = Command.substring(Command.indexOf("@")+1).toFloat();

      if ((channel>=1) && (channel<=MAX_AUDIO_CHANNELS) && (value>=-140) && (value<=6)) {
        audiomixer.volumeCh[channel-1] = value;
        sendVolumeToFPGA(channel);
        Answer = "OK";
      }else{
        Answer = "ERROR: Channel or value out of range!";
      }
    }else if (Command.indexOf("mixer:mute:ch") > -1){
      // received command "mixer:mute:chxx@value"
      uint8_t channel = Command.substring(13, Command.indexOf("@")).toInt();
      bool muted = (Command.substring(Command.indexOf("@")+1).toInt() == 1);

      if ((channel>=1) && (channel<=MAX_AUDIO_CHANNELS)) {
        // set this channel to -48dBfs in FPGA without changing the volume in struct to keep the volume-level for unmuting

        audiomixer.muteCh[channel - 1] = muted; // keep track of muted and unmuted channels

        if (muted) {
          sendVolumeToFPGA(-channel); // negative number mutes the channel without changing the current volume-level in struct
        }else{
          sendVolumeToFPGA(channel); // positive number sets the channel to the current volume again (= unmute)
        }
        Answer = "OK";
      }else{
        Answer = "ERROR: Channel or value out of range!";
      }
    }else if (Command.indexOf("mixer:solo:ch") > -1){
      // received command "mixer:solo:chxx@value"
      uint8_t channel = Command.substring(13, Command.indexOf("@")).toInt();
      bool solo = (Command.substring(Command.indexOf("@")+1).toInt() == 1);

      if ((channel>=1) && (channel<=MAX_AUDIO_CHANNELS)) {
        // set all channels except current channel to -48dBfs in FPGA without changing the volume in struct to keep the volume-level for unmuting

        audiomixer.soloCh[channel - 1] = solo; // keep track of soloed channels

        // check if at least one channel is soloed
        bool soloInUse = false;
        for (uint8_t i=0; i<MAX_AUDIO_CHANNELS; i++) {
          if (audiomixer.soloCh[i]) {
            // at least one single channel in solo-mode
            soloInUse = true;
            break;
          }
        }

        for (uint8_t i=0; i<MAX_AUDIO_CHANNELS; i++) {
          // mute all channels except soloed channels if solo is in use. Otherwise unmute all channels
          if ((audiomixer.soloCh[i]) || (!soloInUse)) {
            // channel is soloed -> enable it or no solo at all
            sendVolumeToFPGA(i + 1); // positive number sets the channel to the current volume again (= unmute)
          }else{
            // channel is not soloed -> mute it
            sendVolumeToFPGA(-(i + 1)); // negative number mutes the channel without changing the current volume-level in struct
          }
        }

        Answer = "OK";
      }else{
        Answer = "ERROR: Channel or value out of range!";
      }
    }else if ((Command.indexOf("mixer:volume:card") > -1) || (Command.indexOf("mixer:volume:sd") > -1)){
      // received command "mixer:volume:card@value"
      float value = Command.substring(Command.indexOf("@")+1).toFloat();

      // we are setting volume for SD and bluetooth as stereo-pair at commands 
	    audiomixer.volumeCard = value;
      sendStereoVolumeToFPGA(0, Command.substring(Command.indexOf("@")+1).toFloat());
      Answer = "OK";
    }else if (Command.indexOf("mixer:volume:bt") > -1){
      // received command "mixer:volume:bt@value"
      float value = Command.substring(Command.indexOf("@")+1).toFloat();

	    audiomixer.volumeBt = value;
      sendStereoVolumeToFPGA(1, Command.substring(Command.indexOf("@")+1).toFloat());
      Answer = "OK";
    }else if (Command.indexOf("mixer:balance:ch") > -1){
      // received command "mixer:balance:chxx@yyy"
      uint8_t channel = Command.substring(16, Command.indexOf("@")).toInt();
      uint8_t value = Command.substring(Command.indexOf("@")+1).toInt();

      if ((channel>=1) && (channel<=MAX_AUDIO_CHANNELS) && (value>=0) && (value<=100)) {
        audiomixer.balanceCh[channel-1] = value;
        sendVolumeToFPGA(channel);
        Answer = "OK";
      }else{
        Answer = "ERROR: Channel or value out of range!";
      }
    }else if (Command.indexOf("mixer:reset") > -1){
      initAudiomixer();
      Answer = "OK";
    }else if (Command.indexOf("mixer:eq:reset") > -1){
      // this function will set eq-coefficients to some standard values
      resetAudioFilters();
      sendFiltersToFPGA();
      Answer = "OK";
    }else if (Command.indexOf("mixer:dynamics:reset") > -1){
      // this function will set data for dynamics to some standard values
      resetDynamics();
      sendDynamicsToFPGA();
      Answer = "OK";
    }else if (Command.indexOf("mixer:eq:peq") > -1){
      // received command "mixer:eq:peq1@aaa,bbb,ccc,ddd"
      uint8_t peq = Command.substring(12, Command.indexOf("@")).toInt() - 1;
      String ParameterString = Command.substring(Command.indexOf("@")+1) + ",";
      uint8_t type = split(ParameterString, ',', 0).toInt();
      float frequency = split(ParameterString, ',', 1).toFloat();
      float quality = split(ParameterString, ',', 2).toFloat();
      float gain = split(ParameterString, ',', 3).toFloat();

      if ((peq>=0) && (peq<MAX_EQUALIZERS) && (type>=0) && (type<=7)) {
        audiomixer.peq[peq].type = type;
        audiomixer.peq[peq].fc = frequency;
        audiomixer.peq[peq].Q = quality;
        audiomixer.peq[peq].gain = gain;

        recalcFilterCoefficients_PEQ(&audiomixer.peq[peq]);
        setResetFlags(1, -1, -1, -1); // disable eqs [eqs, compressor, crossover, upsampler]

        // bypass PEQ if gain is 0
        if (gain == 0) {
          // bypass current PEQ automatically
          bitSet(audiomixer.bypassFlags, peq);
        }else{
          // release bypass
          bitClear(audiomixer.bypassFlags, peq);
        }
        data_16b fpga_data;
        fpga_data.u16 = audiomixer.bypassFlags;
        sendDataToFPGA(FPGA_IDX_AUX_CMD+1, &fpga_data);

        for (int i=0; i<3; i++) { sendDataToFPGA(FPGA_IDX_PEQ+(peq*5)+i, &audiomixer.peq[peq].a[i]); } // 3 a-coefficients
        for (int i=0; i<2; i++) { sendDataToFPGA(FPGA_IDX_PEQ+3+(peq*5)+i, &audiomixer.peq[peq].b[i+1]); } // only 2 b-coefficients
        delay(10);
        setResetFlags(0, -1, -1, -1); // enable eqs [eqs, compressor, crossover, upsampler]
        Answer = "OK";
      }else{
        Answer = "ERROR: PEQ out of range! PEQ #" + String(peq+1) + " and Type = " + String(type);
      }
    }else if (Command.indexOf("mixer:eq:bypass") > -1){
      // received command "mixer:eq:bypass1@aaa"
      uint8_t bypassIndex = Command.substring(15, Command.indexOf("@")).toInt() - 1;
      bool bypass = (Command.substring(Command.indexOf("@")+1).toInt() == 1);
      
      // bypass PEQ (or crossover)
      if (bypass == 1) {
        // bypass current index
        bitSet(audiomixer.bypassFlags, bypassIndex);
      }else{
        // release bypass for current index
        bitClear(audiomixer.bypassFlags, bypassIndex);
      }
      data_16b fpga_data;
      fpga_data.u16 = audiomixer.bypassFlags;
      sendDataToFPGA(FPGA_IDX_AUX_CMD+1, &fpga_data);

      Answer = "OK";
    }else if (Command.indexOf("mixer:eq:lp") > -1){
      // received command "mixer:eq:lp@yyy"
      // this function will set LowPass-frequency
      float frequency = Command.substring(Command.indexOf("@")+1).toFloat();
      
      // check if desired frequency is above lowest supported value
      // background: Q35-fixed-point-numbers will limit to this value.
      if (frequency>=15) {
        audiomixer.LR24_LP_Sub.fc = frequency;
        recalcFilterCoefficients_LR24(&audiomixer.LR24_LP_Sub);

        // transmit the calculated coefficients to FPGA
        setResetFlags(-1, -1, 1, -1); // disable crossover [eqs, compressor, crossover, upsampler]
        setBypassFlags(-1, -1, -1, -1, -1, -1, audiomixer.LR24_LP_Sub.fc>=19000, -1); // [eq1, eq2, eq3, eq4, eq5, crossoverLR, crossoverSub]
        for (int i=0; i<5; i++) { sendDataToFPGA(FPGA_IDX_XOVER+9+i, &audiomixer.LR24_LP_Sub.a[i]); }
        for (int i=0; i<4; i++) { sendDataToFPGA(FPGA_IDX_XOVER+14+i, &audiomixer.LR24_LP_Sub.b[i+1]); } // we are not using b0
        delay(10);
        setResetFlags(-1, -1, 0, -1); // enable crossover [eqs, compressor, crossover, upsampler]
        Answer = "OK";
      }else{
        Answer = "ERROR: Frequency out of range! f_c = " + String(frequency);
      }
    }else if (Command.indexOf("mixer:eq:hp") > -1){
      // received command "mixer:eq:hp@yyy"
      // this function will set HighPass-frequency
      float frequency = Command.substring(Command.indexOf("@")+1).toFloat();
      
      // check if desired frequency is above lowest supported value
      // background: Q35-fixed-point-numbers will limit to this value.
      if (frequency>=15) {
        audiomixer.LR24_HP_LR.fc = frequency;
        recalcFilterCoefficients_LR24(&audiomixer.LR24_HP_LR);

        // transmit the calculated coefficients to FPGA
        setResetFlags(-1, -1, 1, -1); // disable crossover [eqs, compressor, crossover, upsampler]
        setBypassFlags(-1, -1, -1, -1, -1, -1, -1, audiomixer.LR24_HP_LR.fc<=19); // [eq1, eq2, eq3, eq4, eq5, crossoverLR, crossoverSub]
        for (int i=0; i<5; i++) { sendDataToFPGA(FPGA_IDX_XOVER+i, &audiomixer.LR24_HP_LR.a[i]); }
        for (int i=0; i<4; i++) { sendDataToFPGA(FPGA_IDX_XOVER+5+i, &audiomixer.LR24_HP_LR.b[i+1]); } // we are not using b0
        delay(10);
        setResetFlags(-1, -1, 0, -1); // enable crossover [eqs, compressor, crossover, upsampler]
        Answer = "OK";
      }else{
        Answer = "ERROR: Frequency out of range! f_c = " + String(frequency);
      }
    }else if (Command.indexOf("mixer:adc:gain") > -1){
      // received command "mixer:adc:gain1@gain"
      uint8_t channel = Command.substring(14, Command.indexOf("@")).toInt() - 1;
      uint8_t gain = Command.substring(Command.indexOf("@")+1).toInt();

      setADCgain(channel, gain);
      Answer = "OK";
    }else if (Command.indexOf("mixer:gate") > -1){
      // received command "mixer:gate1@aaa,bbb,ccc,ddd,eee"
      uint8_t gate = Command.substring(10, Command.indexOf("@")).toInt() - 1;
      String ParameterString = Command.substring(Command.indexOf("@")+1) + ",";

      if ((gate>=0) && (gate < MAX_NOISEGATES)) {
        audiomixer.gates[gate].threshold = split(ParameterString, ',', 0).toFloat();
        audiomixer.gates[gate].range = split(ParameterString, ',', 1).toFloat();
        audiomixer.gates[gate].attackTime_ms = split(ParameterString, ',', 2).toFloat();
        audiomixer.gates[gate].holdTime_ms = split(ParameterString, ',', 3).toFloat();
        audiomixer.gates[gate].releaseTime_ms = split(ParameterString, ',', 4).toFloat();

        // calculate values for FPGA
        recalcNoiseGate(&audiomixer.gates[gate]);

        // send data to FPGA
        sendDataToFPGA(FPGA_IDX_GATE+0+(gate*5), &audiomixer.gates[gate].value_threshold);
        sendDataToFPGA(FPGA_IDX_GATE+1+(gate*5), &audiomixer.gates[gate].value_gainmin);
        sendDataToFPGA(FPGA_IDX_GATE+2+(gate*5), &audiomixer.gates[gate].value_coeff_attack);
        sendDataToFPGA(FPGA_IDX_GATE+3+(gate*5), &audiomixer.gates[gate].value_hold_ticks);
        sendDataToFPGA(FPGA_IDX_GATE+4+(gate*5), &audiomixer.gates[gate].value_coeff_release);

        Answer = "OK";
      }else{
        Answer = "ERROR: Gate out of range! Gate #" + String(gate+1);
      }
    }else if (Command.indexOf("mixer:comp") > -1){
      // received command "mixer:comp1@aaa,bbb,ccc,ddd,eee,fff"
      uint8_t comp = Command.substring(10, Command.indexOf("@")).toInt() - 1;
      String ParameterString = Command.substring(Command.indexOf("@")+1) + ",";
      uint8_t ratio = split(ParameterString, ',', 1).toInt();

      // check if ratio is power of 2
      // within the FPGA a bit-shift is implemented for ratio-calculation, so we allow the following ratios: 0=oo:1, 1=1:1, 2=2:1, 4=4:1, 8=8:1, 16=16:1, 32=32:1, 64=64:1 and map it to 0,1,2,3,4,5,6,7
      if ((comp>=0) && (comp < MAX_COMPRESSORS) && ((ratio<=64) && ((ratio & (ratio - 1)) == 0))) {
        audiomixer.compressors[comp].threshold = split(ParameterString, ',', 0).toFloat();
        audiomixer.compressors[comp].ratio = ratio;
        audiomixer.compressors[comp].makeup = split(ParameterString, ',', 2).toInt();
        audiomixer.compressors[comp].attackTime_ms = split(ParameterString, ',', 3).toFloat();
        audiomixer.compressors[comp].holdTime_ms = saturateMin_f(split(ParameterString, ',', 4).toFloat(), 0.6f); // limit to minimum 0.6ms. Below this we have some distortions
        audiomixer.compressors[comp].releaseTime_ms = split(ParameterString, ',', 5).toFloat();

        // calculate values for FPGA
        recalcCompressor(&audiomixer.compressors[comp]);

        // send data to FPGA
        setResetFlags(-1, 1, -1, -1); // disable compressor [eqs, compressor, crossover, upsampler]
        sendDataToFPGA(FPGA_IDX_COMP+0+(comp*6), &audiomixer.compressors[comp].value_threshold);
        sendDataToFPGA(FPGA_IDX_COMP+1+(comp*6), &audiomixer.compressors[comp].value_ratio);
        sendDataToFPGA(FPGA_IDX_COMP+2+(comp*6), &audiomixer.compressors[comp].value_makeup);
        sendDataToFPGA(FPGA_IDX_COMP+3+(comp*6), &audiomixer.compressors[comp].value_coeff_attack);
        sendDataToFPGA(FPGA_IDX_COMP+4+(comp*6), &audiomixer.compressors[comp].value_hold_ticks);
        sendDataToFPGA(FPGA_IDX_COMP+5+(comp*6), &audiomixer.compressors[comp].value_coeff_release);
        delay(10);
        setResetFlags(-1, 0, -1, -1); // enable compressor [eqs, compressor, crossover, upsampler]
        
        Answer = "OK";
      }else{
        Answer = "ERROR: Invalid value for compressor! Comp #" + String(comp+1) + " Ratio = " + String(ratio) + ":1";
      }
/*
    }else if (Command.indexOf("test:senddata") > -1){
      // received command "test:senddata@yyy"
      float value = Command.substring(Command.indexOf("@")+1).toFloat();

      data_64b data;
      data.u64 = value * pow(2, 32);
      sendDataToFPGA(254, &data);
      Answer = "OK";
*/
    #if USE_DMX512 == 1
      }else if (Command.indexOf("dmx512:output") > -1){
        // received command "dmx512:output@{"ch": 1; "value": 0}"
        JsonDocument doc;
        String valueStr = Command.substring(Command.indexOf("@")+1);
        DeserializationError error = deserializeJson(doc, valueStr);

        if (!error) {
          uint16_t dmxCh = doc["ch"];
          uint8_t dmxValue = doc["value"];
          dmxData[dmxCh] = dmxValue;
          Answer = "OK";
        }else{
          Answer = "ERROR";
        }
      #if USE_DMX512_RX
        }else if (Command.indexOf("dmx512:input") > -1){
          // received command "dmx512:output@{"ch": 1}"
          JsonDocument doc;
          String valueStr = Command.substring(Command.indexOf("@")+1);
          DeserializationError error = deserializeJson(doc, valueStr);

          if (!error) {
            uint16_t dmxCh = doc["ch"];
            Answer = String(dmxRxData[dmxCh]);
          }else{
            Answer = "ERROR";
          }
      #endif
    #else
    }else if (Command.indexOf("dmx512:output:ch") > -1) {
      // received command "dmx512:output:ch1@value"
      uint16_t channel = Command.substring(16, Command.indexOf("@")).toInt();
      uint8_t value = Command.substring(Command.indexOf("@")+1).toInt();

      setDmx512(channel, value); // send values to FPGA
      Answer = "OK";
    #endif
    }else if (Command.indexOf("ver?") > -1){
      // return version of controller
      Answer = versionstring;
    }else if (Command.indexOf("*IDN?") > -1){
      Answer = "X-f/bape MainCtrl " + String(versionstring) + " built on " + String(compile_date);
    }else if (Command.indexOf("info?") > -1){
      Answer = "X-f/bape X32 Expansion Card " + String(versionstring) + " built on " + String(compile_date) + "\nFPGA v" + FPGA_Version + "\n(c) Dr.-Ing. Christian Noeding\nhttps://www.github.com/xn--nding-jua/Audioplayer";
    }else if (Command.indexOf("system:debug:fpga") > -1){
      // receiving command "system:debug:fpga@0"
      debugRedirectFpgaSerialToUSB = (Command.substring(Command.indexOf("@")+1).toInt() == 1);
      Answer = "OK";
    }else if (Command.indexOf("system:init") > -1){
      initSystem();
      Answer = "OK";
    }else if (Command.indexOf("system:stop") > -1){
      // set system-flag to offline
      systemOnline = false;

      // stop the timers
      TimerSeconds.detach();

      Answer = "OK";
    }else if (Command.indexOf("system:card:init") > -1){
      // reinitialize the SD-Card
      initStorage();

      Answer = "OK";
    }else if (Command.indexOf("system:config:read") > -1){
      configRead("/" + Command.substring(Command.indexOf("@")+1), 100); // read maximum of 100 lines
      Answer = "OK";
    }else if (Command.indexOf("system:config:write") > -1){
      configWrite("/" + Command.substring(Command.indexOf("@")+1));
      Answer = "OK";
    }else if (Command.indexOf("system:wificonfig:write") > -1){
      configWiFiWrite("/wifi.cfg");
      Answer = "OK";
    }else if (Command.indexOf("X-f/bape USBCtrl") > -1){
      USBCtrlIDN = Command;
      Answer = "OK";
    }else{
      // unknown command
      Answer = "UNKNOWN_CMD: " + Command;
    }
  }else{
    Answer = "ERROR";
  }

  return Answer;
}

// sending 48-bit values to fpga
void sendDataToFPGA(uint8_t cmd, data_64b *data) {
  uint8_t SerialCommand[11];
  data_16b ErrorCheckWord;

  ErrorCheckWord.u16 = data->u8[0] + data->u8[1] + data->u8[2] + data->u8[3] + data->u8[4] + data->u8[5];

  SerialCommand[0] = 65;  // A = start of command
  SerialCommand[1] = cmd;
  SerialCommand[2] = data->u8[5]; // MSB of payload
  SerialCommand[3] = data->u8[4];
  SerialCommand[4] = data->u8[3];
  SerialCommand[5] = data->u8[2];
  SerialCommand[6] = data->u8[1];
  SerialCommand[7] = data->u8[0]; // LSB of payload
  SerialCommand[8] = ErrorCheckWord.u8[1]; // MSB
  SerialCommand[9] = ErrorCheckWord.u8[0]; // LSB
  SerialCommand[10] = 69;  // E =  end of command

  // copy signed-bit from 64-bit value to 48bit-value
  if (data->u8[7] & 0b10000000) {
    bitSet(SerialCommand[2], 7);
  }else{
    bitClear(SerialCommand[2], 7);
  }

  Serial1.write(SerialCommand, sizeof(SerialCommand));
  delay(10); // delay for 10ms
}

// some overloaded functions for smaller data. For better handling with signed values
// within the FPGA, we will shift all data to the left to use the upper bits in FPGA
void sendDataToFPGA(uint8_t cmd, data_32b *data) {
  data_64b data64;
  data64.u64 = ((uint64_t)data->u32 << 16); // converting 32bit to 48bit

  // copy the signed-bit of 32-bit value to 64-bit-value to be able to use original sendDataToFPGA() function that expects 64-bit-values
  if (data64.u8[5] & 0b10000000) {
    bitSet(data64.u8[7], 7);
  }else{
    bitClear(data64.u8[7], 7);
  }

  sendDataToFPGA(cmd, &data64);
}
void sendDataToFPGA(uint8_t cmd, data_16b *data) {
  data_32b data32;
  data32.u32 = ((uint32_t)data->u16 << 16);
  sendDataToFPGA(cmd, &data32);
}
void sendDataToFPGA(uint8_t cmd) {
  data_64b data64; // some dummy-information
  sendDataToFPGA(cmd, &data64);
}

void updateSAMD() {
  uint32_t audioTime = audio.getAudioCurrentTime();
  uint32_t audioDuration = audio.getAudioFileDuration();

  String audioProgress_s;
  if (audioDuration > 0) {
    audioProgress_s = String(((float)audioTime/(float)audioDuration)*100.0f, 0);
  }else{
    audioProgress_s = "0";
  }

  // send updatepacket
  String txString;
  txString = "samd:update:info@" + currentAudioFile + "," + // value 0
	String(audioTime) + "," + // value 1
	String(audioDuration) + "," + // value 2
	audioProgress_s + "," + // value 3
	String(audiomixer.volumeMain, 1) + "," + // value 4
	String(audiomixer.balanceMain) + "," + // value 5
	String(audiomixer.volumeSub, 1) + "," + // value 6
	String(audiomixer.volumeCard, 1) + "," + // value 7
	String(audiomixer.volumeBt, 1) + "," + // value 8
	String(audioStatusInfo) + "," + // value 9
	SD_getTOC(2) + "|,"; // request TOC in PSV-format. We need a trailing "|" so that the split() function in SAMD can work correctly

  // send 32 volumeCh[] values as Float-Strings to ensure high accuracy between the faders of MackieMCU and XTouch
  for (uint8_t i=0; i<32; i++) {
    // "-48.4," uses 6 bytes per value, the raw float-value would use only 4 bytes. So we are loosing 32*2 = 64 bytes here using Strings
    // maybe we can change the communication to a binary-communication to reduce the overload to a minimum?
    txString = txString + String(audiomixer.volumeCh[i], 1) + ",";
  }
  // send 32 balanceCh[] values as concatenated HEX-STRINGs
  for (uint8_t i=0; i<32; i++) {
    // we are using HEX here, to use only 2 bytes per value
    txString = txString + intToHex(audiomixer.balanceCh[i], 2);
  }
  txString = txString + ",";
  // send 32 vuMeterCh[] values as concatenated HEX-STRINGs
  for (uint8_t i=0; i<32; i++) {
    // we are using HEX here, to use only 2 bytes per value
    txString = txString + intToHex(audiomixer.vuMeterCh[i], 2);
  }

	txString = txString + ",E"; // add a final comma to use the split-function without problem and terminate the command

  Serial.println(txString);
}

void setX32state(bool state) {
  data_16b fpga_data;
  if (state) {
    // enable X32
    fpga_data.u16 = 1;
  }else{
    // disable X32
    fpga_data.u16 = 0;
  }
  sendDataToFPGA(FPGA_IDX_AUX_CMD+4, &fpga_data);
}


void setDmx512(uint16_t address, uint8_t data) {
  data_32b fpga_data;
  fpga_data.u16[0] = address; // dmx Address
  fpga_data.u8[2] = data; // dmx Data
  fpga_data.u8[3] = 0; // unused for now
  sendDataToFPGA(FPGA_IDX_AUX_CMD+5, &fpga_data);
}