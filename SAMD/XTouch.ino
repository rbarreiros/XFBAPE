/*
  The XTouch-protocol is used to communicate with one or more XTouch-devices via the ethernet-connection
  of the SAMD21. XTouch uses a UDP-connection to communicate.

  This file uses some parts of the MackieMCU-struct to keep names and meters in sync. But all XTouch and X32 DAW
  can display different modes (Audio / DMX512) and different channels. This allows a maximum of degree of freedom
  on adjusting the values.
*/

#if USE_XTOUCH == 1
  void xctlSendGeneralData(uint8_t i_xtouch) {
    uint8_t XCtl_TxMessage[300]; // we are using at least 78 bytes. The other bytes are for button-updates depending on button state
    uint16_t buttonCounter = 0;
    uint16_t channel;

    // for the XTouch we are not checking the DMX-flag at the moment. Maybe this will be changed in a future release
    // handle button-light-counter
    for (uint8_t i=0; i<103; i++) {
      if ((XCtl[i_xtouch].buttonLightOn[i] < 254) && (XCtl[i_xtouch].buttonLightOn[i] > 1)) {
        XCtl[i_xtouch].buttonLightOn[i] -= 1;
      }
    }

    // update scribble-strips
    XCtl_TxMessage[0]=0xf0; // begin of message
    XCtl_TxMessage[1]=0x00;
    XCtl_TxMessage[2]=0x00;
    XCtl_TxMessage[3]=0x66;
    XCtl_TxMessage[4]=0x58;
    XCtl_TxMessage[21] = 0xf7; // end of message

    String topText;
    String botText;
    uint8_t color;

    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      uint8_t hardwareFader = i_ch;
      XCtl_TxMessage[5]=0x20 + hardwareFader;

      // prepare values
      if (XCtl[i_xtouch].dmxMode) {
        channel = (uint16_t)hardwareFader + XCtl[i_xtouch].channelOffsetDmx;
        color = MackieMCU.channelDmx[channel].color; // 0=BLACK, 1=RED, 2=GREEN, 3=YELLOW, 4=BLUE, 5=PINK, 6=CYAN, 7=WHITE
        topText = "DMX " + String(channel + 1) + "    ";
        if ( ((XCtl[i_xtouch].showNames) && (XCtl[i_xtouch].hardwareChannel[hardwareFader].showValueCounter > 0)) || (!XCtl[i_xtouch].showNames) ) {
          // displaying current audio-level in second line
          botText = String(MackieMCU.channelDmx[channel].faderPosition/64.247058f, 0) + "/" + String(MackieMCU.channelDmx[channel].faderPosition/163.83f, 0) + "%  ";
        }else{
          // display channel-name in second line
          botText = MackieMCU.channelDmx[channel].name;
        }
      }else{
        channel = hardwareFader + XCtl[i_xtouch].channelOffset;
        color = MackieMCU.channel[channel].color; // 0=BLACK, 1=RED, 2=GREEN, 3=YELLOW, 4=BLUE, 5=PINK, 6=CYAN, 7=WHITE
        topText = "Ch" + String(channel + 1) + " " + XCtl_panString(MackieMCU.channel[channel].encoderValue);

        if ( ((XCtl[i_xtouch].showNames) && (XCtl[i_xtouch].hardwareChannel[hardwareFader].showValueCounter > 0)) || (!XCtl[i_xtouch].showNames) ) {
          // displaying current audio-level in second line
          botText = String(playerinfo.volumeCh[channel], 2) + "dB    ";
        }else{
          // display channel-name in second line
          botText = MackieMCU.channel[channel].name;
        }
      }      
      
      // set color
      XCtl_TxMessage[6] = color;

      // prepare text
      for (uint8_t i=0; i<7; i++) {
        XCtl_TxMessage[7+i] = topText[i];
        XCtl_TxMessage[14+i] = botText[i];
      }

      // send data
      xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 22); // send 22 bytes (Ctl_TxMessage[0..21]) to port 10111
    }

    // update all buttons
    // Sets the state of a button light (OFF, FLASHING, ON)
    // 0 to 7 - Rec buttons
    // 8 to 15 - Solo buttons
    // 16 to 23 - Mute buttons
    // 24 to 31 - Select buttons
    // 40 to 45 - encoder assign buttons (track, send, pan,  plugin, eq, inst)
    // 46 to 47 - Fader bank left / right
    // 48 to 49 - Channel left / right
    // 50 Flip
    // 51 Global view
    // 52 Display NAME/VALUE
    // 53 Button SMPTE/BEATS
    // 54 to 61 - Function buttons F1 to F8
    // 62 to 69 - Buttons under 7-seg displays
    // 70 to 73 - Modify buttons (shift, option, control, alt)
    // 74 to 79 - Automation buttons (read, write, trim, touch, latch, group)
    // 80 to 83 - Utility buttons (save, undo, cacel, enter)
    // 84 to 90 - Transport buttons (marker, nudge, cycle, drop, replace, click, solo)
    // 91 to 95 - Playback control (rewind, fast-forward, stop, play, record)
    // 96 to 100 - Cursor keys (up, down, left, right, middle)
    // 101 Scrub
    // 113 Smpte
    // 114 Beats
    // 115 Solo - on 7-seg display

    XCtl_TxMessage[0] = 0x90;

    if (XCtl[i_xtouch].dmxMode) {
      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        channel = (uint16_t)i_ch + XCtl[i_xtouch].channelOffsetDmx;
        XCtl_TxMessage[1+i_ch*2] = i_ch; // rec buttons 0...7
        XCtl_TxMessage[2+i_ch*2] = MackieMCU.channelDmx[channel].rec;

        XCtl_TxMessage[8*2+1+i_ch*2] = 8+i_ch; // solo buttons 8...15
        XCtl_TxMessage[8*2+2+i_ch*2] = MackieMCU.channelDmx[channel].solo;

        XCtl_TxMessage[16*2+1+i_ch*2] = 16+i_ch; // mute buttons 16..23
        XCtl_TxMessage[16*2+2+i_ch*2] = MackieMCU.channelDmx[channel].mute;

        XCtl_TxMessage[24*2+1+i_ch*2] = 24+i_ch; // select buttons 24..31
        XCtl_TxMessage[24*2+2+i_ch*2] = MackieMCU.channelDmx[channel].select;
      }
    }else{
      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        channel = i_ch + XCtl[i_xtouch].channelOffset;
        XCtl_TxMessage[1+i_ch*2] = i_ch; // rec buttons 0...7
        XCtl_TxMessage[2+i_ch*2] = MackieMCU.channel[channel].rec;

        XCtl_TxMessage[8*2+1+i_ch*2] = 8+i_ch; // solo buttons 8...15
        XCtl_TxMessage[8*2+2+i_ch*2] = MackieMCU.channel[channel].solo;

        XCtl_TxMessage[16*2+1+i_ch*2] = 16+i_ch; // mute buttons 16..23
        XCtl_TxMessage[16*2+2+i_ch*2] = MackieMCU.channel[channel].mute;

        XCtl_TxMessage[24*2+1+i_ch*2] = 24+i_ch; // select buttons 24..31
        XCtl_TxMessage[24*2+2+i_ch*2] = MackieMCU.channel[channel].select;
      }
    }
    
    // 91 to 95 - Playback control (rewind, fast-forward, stop, play, record)
    XCtl_TxMessage[65] = 50;
    XCtl_TxMessage[66] = XCtl[i_xtouch].dmxMode << 1; // flip
    XCtl_TxMessage[67] = 91;
    XCtl_TxMessage[68] = (playerinfo.currentTrackNumber > 0) << 1; // rewind
    XCtl_TxMessage[69] = 92;
    XCtl_TxMessage[70] = (playerinfo.currentTrackNumber < (tocEntries - 1)) << 1; // forward
    XCtl_TxMessage[71] = 93;
    XCtl_TxMessage[72] = 2; // stop
    XCtl_TxMessage[73] = 94;
    XCtl_TxMessage[74] = 2; // play
    XCtl_TxMessage[75] = 95;
    XCtl_TxMessage[76] = XCtl[i_xtouch].dmxMode << 1; // record

    buttonCounter = 0;
    for (uint16_t i=0; i<103; i++) {
      if (XCtl[i_xtouch].buttonLightOn[i] == 255) {
        // turn button on
        buttonCounter += 1;
        XCtl[i_xtouch].buttonLightOn[i] = 0; // disable this channel
        XCtl_TxMessage[77 + (buttonCounter-1)*2] = i;
        XCtl_TxMessage[78 + (buttonCounter-1)*2] = 2; // turnOn Button
      }else if (XCtl[i_xtouch].buttonLightOn[i] == 254) {
        // button with auto-turnOff
        buttonCounter += 1;
        XCtl[i_xtouch].buttonLightOn[i] = 40; // preload timer-value to 4 (=400ms)
        XCtl_TxMessage[77 + (buttonCounter-1)*2] = i;
        XCtl_TxMessage[78 + (buttonCounter-1)*2] = 2; // turnOn Button
      }else if (XCtl[i_xtouch].buttonLightOn[i] == 1) {
        // turn button off
        buttonCounter += 1;
        XCtl[i_xtouch].buttonLightOn[i] = 0; // disable this channel
        XCtl_TxMessage[77 + (buttonCounter-1)*2] = i;
        XCtl_TxMessage[78 + (buttonCounter-1)*2] = 0; // turnOff Button
      }
    }
    xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 77+(buttonCounter*2)); //send 77+(buttonCounter*2) bytes (Ctl_TxMessage[0..77+(buttonCounter*2)]) to port 10111

    // Update encoderLevels around PanKnob
    XCtl_TxMessage[0] = 0xB0;
    uint16_t encoderLevelRaw = 0;
    if (XCtl[i_xtouch].dmxMode) {
      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        // render as volume-level = growing bar-level
        encoderLevelRaw = 0;
        channel = (uint16_t)i_ch + XCtl[i_xtouch].channelOffsetDmx;
        for (uint16_t i=0; i<=(uint16_t)(MackieMCU.channelDmx[channel].encoderValue/21.25f); i++){
          encoderLevelRaw += (1 << i);
        }

        XCtl_TxMessage[1 + i_ch*4] = 0x30 + i_ch;
        XCtl_TxMessage[2 + i_ch*4] = encoderLevelRaw & 0x7F;
        XCtl_TxMessage[3 + i_ch*4] = 0x38 + i_ch;
        XCtl_TxMessage[4 + i_ch*4] = (encoderLevelRaw >> 7) & 0x7F;
      }
    }else{
      /*
      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        // render as pan-level = single-mark
        channel = i_ch + XCtl[i_xtouch].channelOffset;
        encoderLevelRaw = (uint8_t)(MackieMCU.channel[channel].encoderValue/21.25f);
        encoderLevelRaw = (1 << encoderLevelRaw);

        XCtl_TxMessage[1 + i_ch*4] = 0x30 + i_ch;
        XCtl_TxMessage[2 + i_ch*4] = encoderLevelRaw & 0x7F;
        XCtl_TxMessage[3 + i_ch*4] = 0x38 + i_ch;
        XCtl_TxMessage[4 + i_ch*4] = (encoderLevelRaw >> 7) & 0x7F;
      }
      */
      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        // render as pan-level = growing bars to left or right
        encoderLevelRaw = 0;
        channel = i_ch + XCtl[i_xtouch].channelOffset;
        uint8_t panLeft = saturate(MackieMCU.channel[channel].encoderValue-21, 0, 127); // make sure, that center-led is on
        uint8_t panRight = saturate(MackieMCU.channel[channel].encoderValue-128, 0, 128);

        uint8_t encoderLevelRawLeft = 0;
        if (panLeft>0) {
          for (uint16_t i=0; i<=(int16_t)(panLeft/21.25f); i++){
            encoderLevelRawLeft += (1 << i);
          }
        }

        uint8_t encoderLevelRawRight = 0;
        if (panRight>0) {
          for (uint16_t i=0; i<=(int16_t)(panRight/21.25f); i++){
            encoderLevelRawRight += (1 << i);
          }
        }

        XCtl_TxMessage[1 + i_ch*4] = 0x30 + i_ch;
        XCtl_TxMessage[2 + i_ch*4] = (~encoderLevelRawLeft) & 0x7F;
        XCtl_TxMessage[3 + i_ch*4] = 0x38 + i_ch;
        XCtl_TxMessage[4 + i_ch*4] = (encoderLevelRawRight) & 0x7F;
      }
    }
    xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 34); //send 34 bytes (Ctl_TxMessage[0..33]) to port 10111

    // Set 7-Segment-Displays
    XCtl_TxMessage[0] = 0xB0;
    for (uint8_t i=0; i<12; i++) {
      XCtl_TxMessage[1 + i*2] = 0x60 + i;

      // 0x30 to 0x37 - Left hand sides of knobs
      // 0x38 to 0x3F - Right hand sides of knobs
      // 0x60 - Left hand assignment digit
      // 0x61 - Right hand assignment digit
      // 0x62-0x64 - Bars digits
      // 0x65-0x66 - Beats digits
      // 0x67-0x68 - Sub division digits
      // 0x69-0x6B - Ticks digits
      // 0x70-0x7B - same as above but with . also lit
      // Value: 7-bit bitmap of segments to illuminate
      XCtl_TxMessage[2 + i*2] = xctlGetSegmentBitmap(XCtl[i_xtouch].segmentDisplay[i]);
    }
    xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 25); //send 25 bytes (Ctl_TxMessage[0..24]) to port 10111
  }

  void xctlSendFaderData(uint8_t i_xtouch) {
    uint8_t XCtl_TxMessage[9]; // we are using at least 78 bytes. The other bytes are for button-updates depending on button state
    uint16_t channel;

    if (XCtl[i_xtouch].dmxMode) {
      // update channel-faders
      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        channel = (uint16_t)i_ch + XCtl[i_xtouch].channelOffsetDmx;
        if (XCtl[i_xtouch].hardwareChannel[i_ch].faderNeedsUpdate || XCtl[i_xtouch].forceUpdate) {
          XCtl[i_xtouch].hardwareChannel[i_ch].faderNeedsUpdate = false;
          XCtl_TxMessage[0] = 0xE0 + i_ch;
          XCtl_TxMessage[1] = MackieMCU.channelDmx[channel].faderPosition & 0x7F; // MIDI-Values between 0 and 127
          XCtl_TxMessage[2] = (MackieMCU.channelDmx[channel].faderPosition >> 7) & 0x7F;
          xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 3); //send 3 bytes (Ctl_TxMessage[0..2]) to port 10111
        }
      }
      // update masterfader
      if (XCtl[i_xtouch].hardwareMainfader.faderNeedsUpdate || XCtl[i_xtouch].forceUpdate) {
        XCtl[i_xtouch].hardwareMainfader.faderNeedsUpdate = false;
        XCtl_TxMessage[0] = 0xE8; // E8=Masterfader
        XCtl_TxMessage[1] = MackieMCU.channelDmx[512].faderPosition & 0x7F; // MIDI-Values between 0 and 127
        XCtl_TxMessage[2] = (MackieMCU.channelDmx[512].faderPosition >> 7) & 0x7F;
        xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 3); //send 3 bytes (Ctl_TxMessage[0..2]) to port 10111
      }
      XCtl[i_xtouch].forceUpdate = false;

      // update meter-levels
      XCtl_TxMessage[0] = 0xD0;
      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        channel = (uint16_t)i_ch + XCtl[i_xtouch].channelOffset;
        XCtl_TxMessage[1 + i_ch] = (i_ch << 4) + XCtl[i_xtouch].hardwareChannel[i_ch].meterLevel; // 0..8
      }
      xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 9); //send 9 bytes (Ctl_TxMessage[0..8]) to port 10111
    }else{
      // update channel-faders
      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        if (XCtl[i_xtouch].hardwareChannel[i_ch].faderNeedsUpdate || XCtl[i_xtouch].forceUpdate) {
          XCtl[i_xtouch].hardwareChannel[i_ch].faderNeedsUpdate = false;
          XCtl_TxMessage[0] = 0xE0 + i_ch;
          XCtl_TxMessage[1] = MackieMCU.channel[i_ch + XCtl[i_xtouch].channelOffset].faderPosition & 0x7F; // MIDI-Values between 0 and 127
          XCtl_TxMessage[2] = (MackieMCU.channel[i_ch + XCtl[i_xtouch].channelOffset].faderPosition >> 7) & 0x7F;
          xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 3); //send 3 bytes (Ctl_TxMessage[0..2]) to port 10111
        }
      }
      // update masterfader
      if (XCtl[i_xtouch].hardwareMainfader.faderNeedsUpdate || XCtl[i_xtouch].forceUpdate) {
        XCtl[i_xtouch].hardwareMainfader.faderNeedsUpdate = false;
        XCtl_TxMessage[0] = 0xE8; // E8=Masterfader
        XCtl_TxMessage[1] = MackieMCU.channel[32].faderPosition & 0x7F; // MIDI-Values between 0 and 127
        XCtl_TxMessage[2] = (MackieMCU.channel[32].faderPosition >> 7) & 0x7F;
        xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 3); //send 3 bytes (Ctl_TxMessage[0..2]) to port 10111
      }
      XCtl[i_xtouch].forceUpdate = false;

      // update meter-levels
      XCtl_TxMessage[0] = 0xD0;
      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        channel = i_ch + XCtl[i_xtouch].channelOffset;
        XCtl_TxMessage[1 + i_ch] = (i_ch << 4) + XCtl[i_xtouch].hardwareChannel[i_ch].meterLevel; // 0..8
      }
      xctlSendUdpPacket(i_xtouch, XCtl_TxMessage, 9); //send 9 bytes (Ctl_TxMessage[0..8]) to port 10111
    }
  }

  void xctlHandleCommunication(uint8_t udpDevice) {
    // message start: F0
    // message terminator: F7

    // every 2 seconds XTouch sends 00 20 32 58 54 00
    // we have to send 00 00 66 14 00

    if (XCtlUdp[udpDevice].parsePacket() > 0) {
      // it seems, that the Arduino Ethernet library is routing all incoming traffic to
      // the first udpObject, when two udpServers are using the same UdpPort
      // so first check for which device we are receiving based on the remoteIP
      uint8_t i_xtouch = 0;
      for (uint8_t i=0; i<XTOUCH_COUNT; i++) {
        if (XCtlUdp[udpDevice].remoteIP().toString().indexOf(XCtl[i].ip.toString()) > -1) {
          i_xtouch = i; // we found the sender. So route all incoming data to this device
          break;
        }
      }

      // enable this device as we are receiving from it
      XCtl[i_xtouch].online = true;

      uint8_t rxData[18]; //buffer to hold incoming packet,
      uint8_t len = XCtlUdp[udpDevice].read(rxData, 18);
      uint16_t channel = 0;
      int16_t value = 0;
      uint8_t hardwareFader = 0;

      // check if packet is valid
      if ((len>0) && (rxData[0] == 0xF0) && (rxData[len-1] == 0xF7)) {
        // Message has expected start- and end-byte
        // check the received data

        if ((len == 8) && (memcmp(rxData, XCtl_Probe, 8) == 0)) {
          // we received a Probe-Message
          xctlSendUdpPacket(i_xtouch, XCtl_ProbeResponse, 8);
        }else if ((len == 18) && (memcmp(rxData, XCtl_ProbeB, 18) == 0)) {
          // Ignore ProbeB MSG
        }else if ((len == 18) && (memcmp(rxData, XCtl_ProbeC, 18) == 0)) {
          // Ignore ProbeC MSG
        }else{
          // we received a unknown message between F0 and F7
        }
      }else{
        // we received values to parse

        // check for correct length
        if (len == 3) {
          // check for touched fader
          if ((rxData[0] == 0x90) && (rxData[1] >= 0x68) && (rxData[1] <= 0x70)) {
            hardwareFader = (rxData[1] - 0x68);

            // check if touch is removed from fader and update fader
            //if ((XCtl[i_xtouch].hardwareChannel[hardwareFader].faderTouched) && !(rxData[2] != 0))  {
            //  // fader is untouched
            //  XCtl[i_xtouch].hardwareChannel[hardwareFader].faderNeedsUpdate = true;
            //}

            XCtl[i_xtouch].hardwareChannel[hardwareFader].faderTouched = rxData[2] != 0;
          }

          // read faderlevel
          if ((rxData[0] & 0xF0) == 0xE0) {
            // fader = rxData[0] & 0x0F;
            // value = rxData[1] + (rxData[2] << 7);

            if (XCtl[i_xtouch].dmxMode) {
              if ((rxData[0] & 0x0F) <= 7) {
                // values of faders 1 to 8
                hardwareFader = (rxData[0] & 0x0F);
                channel = (uint16_t)hardwareFader + XCtl[i_xtouch].channelOffsetDmx;
                value = rxData[1] + (rxData[2] << 7); // 0...16383

                XCtl[i_xtouch].hardwareChannel[hardwareFader].faderPositionHW = value;
                if (XCtl[i_xtouch].hardwareChannel[hardwareFader].faderTouched) {
                  MackieMCU.channelDmx[channel].faderPosition = value;

                  // send new dmx-value to NINA
                  uint8_t newDmxValue = (value/64.24705882352941f);
                  SerialNina.println("dmx512:output:ch" + String(channel + 1) + "@" + String(newDmxValue));

                  // reset nameCounter to display current value in displays instead of names for some time
                  XCtl[i_xtouch].hardwareChannel[hardwareFader].showValueCounter = 20; // show value for 2 seconds as counter is at 100ms
                }
              }else if ((rxData[0] & 0x0F) == 8){
                // masterfader = fader 9
                value = rxData[1] + (rxData[2] << 7); // 0...16383
                
                XCtl[i_xtouch].hardwareMainfader.faderPositionHW = value;
                if (XCtl[i_xtouch].hardwareMainfader.faderTouched) {
                  MackieMCU.channelDmx[512].faderPosition = value;
                  // we are not using the Masterfader in DMX-Mode at the moment
                }
              }
            }else{
              if ((rxData[0] & 0x0F) <= 7) {
                // values of faders 1 to 8
                uint8_t hardwareFader = (rxData[0] & 0x0F);
                channel = hardwareFader + XCtl[i_xtouch].channelOffset;
                value = rxData[1] + (rxData[2] << 7); // 0...16383

                XCtl[i_xtouch].hardwareChannel[hardwareFader].faderPositionHW = value;
                if (XCtl[i_xtouch].hardwareChannel[hardwareFader].faderTouched) {
                  float newVolume = ((value/16383.0f) * 54.0f) - 48.0f;
                  playerinfo.volumeCh[channel] = newVolume; // we are receiving this value from NINA with a bit delay again

                  // send new channel-volume to NINA
                  SerialNina.println("mixer:volume:ch" + String(channel + 1) + "@" + String(newVolume, 2));

                  // reset nameCounter to display current value in displays instead of names for some time
                  XCtl[i_xtouch].hardwareChannel[hardwareFader].showValueCounter = 20; // show value for 2 seconds as counter is at 100ms
                }
              }else if ((rxData[0] & 0x0F) == 8){
                // masterfader = fader 9
                value = rxData[1] + (rxData[2] << 7); // 0...16383
                
                XCtl[i_xtouch].hardwareMainfader.faderPositionHW = value;
                if (XCtl[i_xtouch].hardwareMainfader.faderTouched) {
                  float newVolume = ((value/16383.0f) * 54.0f) - 48.0f;
                  playerinfo.volumeMain = newVolume;

                  // send new main-volume
                  SerialNina.println("mixer:volume:main@" + String(newVolume, 2));
                }
              }
            }
          }

          // read rotation
          if (rxData[0] == 0xB0) {
            if ((rxData[2] & 0x40) == 0x40) {
              value = (0 - (int16_t)(rxData[2] & 0x0F));
            }else{
              value = (rxData[2] & 0x0F);
            }

            if (XCtl[i_xtouch].dmxMode) {
              if ((rxData[1] >= 16) && (rxData[1] <= 23)) {
                // channelDials
                if (value>0) {
                  // turn right
                  channel = (uint16_t)(rxData[1] - 16) + XCtl[i_xtouch].channelOffsetDmx;
                  if (MackieMCU.channelDmx[channel].encoderValue + value > 255) {
                    MackieMCU.channelDmx[channel].encoderValue = 255;
                  }else{
                    MackieMCU.channelDmx[channel].encoderValue += value;
                  }
                  // we are not using this encoder-value in DMX-Mode at the moment
                }else{
                  // turn left
                  channel = (rxData[1] - 16) + XCtl[i_xtouch].channelOffsetDmx;
                  if ((int16_t)MackieMCU.channelDmx[channel].encoderValue + value < 0) {
                    MackieMCU.channelDmx[channel].encoderValue = 0;
                  }else{
                    MackieMCU.channelDmx[channel].encoderValue += value;
                  }
                  // we are not using this encoder-value in DMX-Mode at the moment
                }
              }else if (rxData[1] == 60) {
                // large jog-dial
                if (value>0) {
                  // turn right

                  if (XCtl[i_xtouch].jogDialValueDmx + value > 255) {
                    XCtl[i_xtouch].jogDialValueDmx = 255;
                  }else{
                    XCtl[i_xtouch].jogDialValueDmx += value;
                  }
                }else{
                  // turn left

                  if ((int16_t)XCtl[i_xtouch].jogDialValueDmx + value < 0) {
                    XCtl[i_xtouch].jogDialValueDmx = 0;
                  }else{
                    XCtl[i_xtouch].jogDialValueDmx += value;
                  }
                }
              }
            }else{
              if ((rxData[1] >= 16) && (rxData[1] <= 23)) {
                // channelDials
                if (value>0) {
                  // turn right
                  channel = (rxData[1] - 16) + XCtl[i_xtouch].channelOffset;
                  if (MackieMCU.channel[channel].encoderValue + value > 255) {
                    MackieMCU.channel[channel].encoderValue = 255;
                  }else{
                    MackieMCU.channel[channel].encoderValue += value;
                  }
                  // set balance
                  playerinfo.balanceCh[channel] = MackieMCU.channel[channel].encoderValue; // we are receiving this value from NINA with a bit delay again
                  SerialNina.println("mixer:balance:ch" + String(channel + 1) + "@" + String(MackieMCU.channel[channel].encoderValue / 2.55f));
                }else{
                  // turn left
                  channel = (rxData[1] - 16) + XCtl[i_xtouch].channelOffset;
                  if ((int16_t)MackieMCU.channel[channel].encoderValue + value < 0) {
                    MackieMCU.channel[channel].encoderValue = 0;
                  }else{
                    MackieMCU.channel[channel].encoderValue += value;
                  }
                  // set balance
                  playerinfo.balanceCh[channel] = MackieMCU.channel[channel].encoderValue; // we are receiving this value from NINA with a bit delay again
                  SerialNina.println("mixer:balance:ch" + String(channel + 1) + "@" + String(MackieMCU.channel[channel].encoderValue / 2.55f));
                }
              }else if (rxData[1] == 60) {
                // large jog-dial
                if (value>0) {
                  // turn right

                  if (XCtl[i_xtouch].jogDialValue + value > 255) {
                    XCtl[i_xtouch].jogDialValue = 255;
                  }else{
                    XCtl[i_xtouch].jogDialValue += value;
                  }
                }else{
                  // turn left

                  if ((int16_t)XCtl[i_xtouch].jogDialValue + value < 0) {
                    XCtl[i_xtouch].jogDialValue = 0;
                  }else{
                    XCtl[i_xtouch].jogDialValue += value;
                  }
                }
              }
            }
          }

          // read button
          if (rxData[0] == 0x90) {
            uint8_t button = rxData[1];
            uint8_t buttonState = rxData[2] > 0;

            // 0 to 7 - Rec buttons
            // 8 to 15 - Solo buttons
            // 16 to 23 - Mute buttons
            // 24 to 31 - Select buttons
            // 40 to 45 - encoder assign buttons (track, send, pan,  plugin, eq, inst)
            // 46 to 47 - Fader bank left / right
            // 48 to 49 - Channel left / right
            // 50 Flip
            // 51 Global view
            // 52 Display Name/Value
            // 53 Button SMTPE/BEATS 
            // 54 to 61 - Function buttons F1 to F8
            // 62 to 69 - Buttons under 7-seg displays
            // 70 to 73 - Modify buttons (shift, option, control, alt)
            // 74 to 79 - Automation buttons (read, write, trim, touch, latch, group)
            // 80 to 83 - Utility buttons (save, undo, cacel, enter)
            // 84 to 90 - Transport buttons (marker, nudge, cycle, drop, replace, click, solo)
            // 91 to 95 - Playback control (rewind, fast-forward, stop, play, record)
            // 96 to 100 - Cursor keys (up, down, left, right, middle)
            // 101 Scrub
            // 113 Smpte
            // 114 Beats
            // 115 Solo - on 7-seg display

            // turn on LED for the common buttons on button-press
            //if (buttonState && ((button >= 32) && (button < 102))) {
            //  XCtl[i_xtouch].buttonLightOn[button] = 254; // 255=TurnOn, 254=enable with auto-TurnOff, 1=TurnOff
            //}

            // turn on LED for the common buttons on button-press
            if (((button >= 32) && (button<102)) && !((button>=40) && (button<=45) && !(button==50) && !(button==52) && !(button==95))) { // 40 to 45 - encoder assign buttons (track, send, pan,  plugin, eq, inst)
              if (buttonState) {
                XCtl[i_xtouch].buttonLightOn[button] = 255; // on
              }else{
                XCtl[i_xtouch].buttonLightOn[button] = 1; // one step before off
              }
            }

            if (XCtl[i_xtouch].dmxMode) {
              // individual buttons
              if ((button >= 0) && (button<=7)) {
                // rec-buttons
                if (buttonState) {
                  // pressed
                  channel = (uint16_t)button + XCtl[i_xtouch].channelOffsetDmx;
                  if (MackieMCU.channelDmx[channel].rec == 0) {
                    MackieMCU.channelDmx[channel].rec = 2;
                  }else{
                    MackieMCU.channelDmx[channel].rec = 0;
                  }
                }else{
                  // released
                }
              }

              if ((button >= 8) && (button<=15)) {
                // solo-buttons
                if (buttonState) {
                  // pressed
                  channel = (uint16_t)(button - 8) + XCtl[i_xtouch].channelOffsetDmx;
                  if (MackieMCU.channelDmx[channel].solo == 0) {
                    MackieMCU.channelDmx[channel].solo = 2;
                  }else{
                    MackieMCU.channelDmx[channel].solo = 0;
                  }
                }else{
                  // released
                }
              }

              if ((button >= 16) && (button<=23)) {
                // mute-buttons
                if (buttonState) {
                  // pressed
                  channel = (uint16_t)(button - 16) + XCtl[i_xtouch].channelOffsetDmx;
                  if (MackieMCU.channelDmx[channel].mute == 0) {
                    MackieMCU.channelDmx[channel].mute = 2;
                  }else{
                    MackieMCU.channelDmx[channel].mute = 0;
                  }
                }else{
                  // released
                }
              }

              if ((button >= 24) && (button<=31)) {
                // select-buttons
                if (buttonState) {
                  // pressed
                  channel = (uint16_t)(button - 24) + XCtl[i_xtouch].channelOffsetDmx;
                  if (MackieMCU.channelDmx[channel].select == 0) {
                    // disable all select-channels
                    for (uint8_t i=0; i<32; i++) {
                      MackieMCU.channelDmx[i].select = 0;
                    }

                    MackieMCU.channelDmx[channel].select = 2;
                  }else{
                    MackieMCU.channelDmx[channel].select = 0;
                  }
                }else{
                  // released
                }
              }

              if ((button >= 32) && (button<=39)) {
                // encoder-buttons
                channel = (uint16_t)(button - 32) + XCtl[i_xtouch].channelOffsetDmx;
                if (buttonState) {
                  // pressed
                  // reset encoder to 0%
                  MackieMCU.channelDmx[channel].encoderValue = 0;
                }else{
                  // released
                }
              }

              if ((button == 46) && (buttonState)) {
                // fader bank left
                if (XCtl[i_xtouch].channelOffsetDmx >= 32) {
                  XCtl[i_xtouch].channelOffsetDmx -= 32;
                }else{
                  XCtl[i_xtouch].channelOffsetDmx = 0;
                }
                XCtl[i_xtouch].forceUpdate = true;
              }

              if ((button == 47) && (buttonState)) {
                // fader bank right
                if (XCtl[i_xtouch].channelOffsetDmx <= 472) {
                  XCtl[i_xtouch].channelOffsetDmx += 32;
                }else{
                  XCtl[i_xtouch].channelOffsetDmx = 504;
                }
                XCtl[i_xtouch].forceUpdate = true;
              }

              if ((button == 48) && (buttonState)) {
                // channel left
                if (XCtl[i_xtouch].channelOffsetDmx >= 8) {
                  XCtl[i_xtouch].channelOffsetDmx -= 8;
                }else{
                  XCtl[i_xtouch].channelOffsetDmx = 0;
                }
                XCtl[i_xtouch].forceUpdate = true;
              }

              if ((button == 49) && (buttonState)) {
                // channel right
                if (XCtl[i_xtouch].channelOffsetDmx <= 496) {
                  XCtl[i_xtouch].channelOffsetDmx += 8;
                }else{
                  XCtl[i_xtouch].channelOffsetDmx = 504;
                }
                XCtl[i_xtouch].forceUpdate = true;
              }
            }else{
              // individual buttons
              if ((button >= 0) && (button<=7)) {
                // rec-buttons
                if (buttonState) {
                  // pressed
                  if (MackieMCU.channel[button + XCtl[i_xtouch].channelOffset].rec == 0) {
                    MackieMCU.channel[button + XCtl[i_xtouch].channelOffset].rec = 2;
                  }else{
                    MackieMCU.channel[button + XCtl[i_xtouch].channelOffset].rec = 0;
                  }
                }else{
                  // released
                }
              }

              if ((button >= 8) && (button<=15)) {
                // solo-buttons
                if (buttonState) {
                  // pressed
                  channel = (button-8) + XCtl[i_xtouch].channelOffset;
                  if (MackieMCU.channel[channel].solo == 0) {
                    MackieMCU.channel[channel].solo = 2;
                  }else{
                    MackieMCU.channel[channel].solo = 0;
                  }

                  // send value to NINA
                  SerialNina.println("mixer:solo:ch" + String(channel + 1) + "@" + String(MackieMCU.channel[channel].solo > 0));
                }else{
                  // released
                }
              }

              if ((button >= 16) && (button<=23)) {
                // mute-buttons
                if (buttonState) {
                  // pressed
                  channel = (button-16) + XCtl[i_xtouch].channelOffset;
                  if (MackieMCU.channel[channel].mute == 0) {
                    MackieMCU.channel[channel].mute = 2;
                  }else{
                    MackieMCU.channel[channel].mute = 0;
                  }

                  // send value to NINA
                  SerialNina.println("mixer:mute:ch" + String(channel + 1) + "@" + String(MackieMCU.channel[channel].mute > 0));
                }else{
                  // released
                }
              }

              if ((button >= 24) && (button<=31)) {
                // select-buttons
                if (buttonState) {
                  // pressed
                  if (MackieMCU.channel[(button-24) + XCtl[i_xtouch].channelOffset].select == 0) {
                    // disable all select-channels
                    for (uint8_t i=0; i<32; i++) {
                      MackieMCU.channel[i].select = 0;
                    }

                    MackieMCU.channel[(button-24) + XCtl[i_xtouch].channelOffset].select = 2;
                  }else{
                    MackieMCU.channel[(button-24) + XCtl[i_xtouch].channelOffset].select = 0;
                  }
                }else{
                  // released
                }
              }

              if ((button >= 32) && (button<=39)) {
                // encoder-buttons
                if (buttonState) {
                  // pressed
                  // reset panning to 50%
                  channel = (button-32) + XCtl[i_xtouch].channelOffset;
                  MackieMCU.channel[channel].encoderValue = 128;
                  playerinfo.balanceCh[channel] = 128; // we are receiving this value from NINA with a bit delay again
                  SerialNina.println("mixer:balance:ch" + String(channel + 1) + "@" + String(MackieMCU.channel[channel].encoderValue / 2.55f));
                }else{
                  // released
                }
              }

              if ((button == 46) && (buttonState)) {
                // fader bank left
                if (XCtl[i_xtouch].channelOffset >= 8) {
                  XCtl[i_xtouch].channelOffset -= 8;
                }else{
                  XCtl[i_xtouch].channelOffset = 0;
                }
                XCtl[i_xtouch].forceUpdate = true;
              }

              if ((button == 47) && (buttonState)) {
                // fader bank right
                if (XCtl[i_xtouch].channelOffset <= 16) {
                  XCtl[i_xtouch].channelOffset += 8;
                }else{
                  XCtl[i_xtouch].channelOffset = 24;
                }
                XCtl[i_xtouch].forceUpdate = true;
              }

              if ((button == 48) && (buttonState)) {
                // channel left
                if (XCtl[i_xtouch].channelOffset >= 1) {
                  XCtl[i_xtouch].channelOffset -= 1;
                }else{
                  XCtl[i_xtouch].channelOffset = 0;
                }
                XCtl[i_xtouch].forceUpdate = true;
              }

              if ((button == 49) && (buttonState)) {
                // channel right
                if (XCtl[i_xtouch].channelOffset <= 23) {
                  XCtl[i_xtouch].channelOffset += 1;
                }else{
                  XCtl[i_xtouch].channelOffset = 24;
                }
                XCtl[i_xtouch].forceUpdate = true;
              }
            }

            if ((button == 52) && (buttonState)) {
              // button "Display/Name"
              XCtl[i_xtouch].showNames = !XCtl[i_xtouch].showNames;
              
              if (XCtl[i_xtouch].showNames) {
                XCtl[i_xtouch].buttonLightOn[button] = 1; // turn off
              }else{
                XCtl[i_xtouch].buttonLightOn[button] = 255; // turn on
              }
            }

            // 91 to 95 - Playback control (rewind, fast-forward, stop, play, record)
            if ((button == 91) && (buttonState)) {
              // button "rewind"
              SerialNina.println("player:prev");
            }
            if ((button == 92) && (buttonState)) {
              // button "forward"
              SerialNina.println("player:next");
            }
            if ((button == 93) && (buttonState)) {
              // button "stop"
              SerialNina.println("player:stop");
            }
            if ((button == 94) && (buttonState)) {
              // button "play"
              SerialNina.println("player:pause");
            }

            if (((button == 50) || (button == 95)) && (buttonState)) {
              // button "Flip" or "Record"
              XCtl[i_xtouch].dmxMode = !XCtl[i_xtouch].dmxMode;
              XCtl[i_xtouch].forceUpdate = true;

              if (XCtl[i_xtouch].dmxMode) {
                XCtl[i_xtouch].buttonLightOn[button] = 255;
              }else{
                XCtl[i_xtouch].buttonLightOn[button] = 1;
              }
            }
          }
        }
      }
    }
  }

  void xctlPrepareData(uint8_t i_xtouch) {
    // Update Segment Display
    if (XCtl[i_xtouch].dmxMode) {
      String playtime = secondsToHMS_B(playerinfo.time); // 00:00:00
      XCtl[i_xtouch].segmentDisplay[0] = '0';
      XCtl[i_xtouch].segmentDisplay[1] = 49 + i_xtouch; // 49 = ASCII '1'
      XCtl[i_xtouch].segmentDisplay[2] = '-';
      XCtl[i_xtouch].segmentDisplay[3] = '-';
      XCtl[i_xtouch].segmentDisplay[4] = '-';
      XCtl[i_xtouch].segmentDisplay[5] = '-';
      XCtl[i_xtouch].segmentDisplay[6] = '-';
      XCtl[i_xtouch].segmentDisplay[7] = '-';
      XCtl[i_xtouch].segmentDisplay[8] = '-';

      String valueString = String(XCtl[i_xtouch].jogDialValueDmx);
      if (XCtl[i_xtouch].jogDialValueDmx < 10) {
        XCtl[i_xtouch].segmentDisplay[9] = '0';
        XCtl[i_xtouch].segmentDisplay[10] = '0';
        XCtl[i_xtouch].segmentDisplay[11] = valueString[0];
      }else if (XCtl[i_xtouch].jogDialValueDmx < 100) {
        XCtl[i_xtouch].segmentDisplay[9] = '0';
        XCtl[i_xtouch].segmentDisplay[10] = valueString[0];
        XCtl[i_xtouch].segmentDisplay[11] = valueString[1];
      }else{
        XCtl[i_xtouch].segmentDisplay[9] = valueString[0];
        XCtl[i_xtouch].segmentDisplay[10] = valueString[1];
        XCtl[i_xtouch].segmentDisplay[11] = valueString[2];
      }

      // update all faders and buttons for the current channel-selection
      for (uint16_t i_ch=XCtl[i_xtouch].channelOffsetDmx; i_ch<(8+XCtl[i_xtouch].channelOffsetDmx); i_ch++) {
        uint8_t hardwareFader = i_ch - XCtl[i_xtouch].channelOffsetDmx;
        XCtl[i_xtouch].hardwareChannel[hardwareFader].faderNeedsUpdate = xctlCheckIfFaderNeedsUpdate(MackieMCU.channelDmx[i_ch].faderPosition, XCtl[i_xtouch].hardwareChannel[hardwareFader].faderPositionHW) && (!XCtl[i_xtouch].hardwareChannel[hardwareFader].faderTouched);
        if (!XCtl[i_xtouch].hardwareChannel[hardwareFader].faderTouched) {
          XCtl[i_xtouch].hardwareChannel[hardwareFader].faderPositionHW = MackieMCU.channelDmx[i_ch].faderPosition;
        }
        XCtl[i_xtouch].hardwareChannel[hardwareFader].meterLevel = MackieMCU.channelDmx[i_ch].faderPosition/2047;
      }

      /*
      // we are not using the master-fader at the moment - maybe in a later revision
      // update Masterfader
      uint16_t newFaderValue = ...;
      XCtl[i_xtouch].hardwareMainfader.faderNeedsUpdate = xctlCheckIfFaderNeedsUpdate(newFaderValue, MackieMCU.channel[512].faderPosition);
      if (!XCtl[i_xtouch].hardwareMainfader.faderTouched) {
        XCtl[i_xtouch].hardwareMainfader.faderPositionHW = newFaderValue; // this prevents the fader to snap to desired position after untouching it!
      }
      MackieMCU.channel[512].faderPosition = newFaderValue; // convert volumeMain from dBfs to 0...16388 but keep logarithmic scale
      */
    }else{
      String playtime = secondsToHMS_B(playerinfo.time); // 00:00:00
      XCtl[i_xtouch].segmentDisplay[0] = '0';
      XCtl[i_xtouch].segmentDisplay[1] = 49 + i_xtouch; // 49 = ASCII '1'
      XCtl[i_xtouch].segmentDisplay[2] = ' ';
      XCtl[i_xtouch].segmentDisplay[3] = playtime[0]; // h
      XCtl[i_xtouch].segmentDisplay[4] = playtime[1]; // h
      XCtl[i_xtouch].segmentDisplay[5] = playtime[3]; // min
      XCtl[i_xtouch].segmentDisplay[6] = playtime[4]; // min
      XCtl[i_xtouch].segmentDisplay[7] = playtime[6]; // s
      XCtl[i_xtouch].segmentDisplay[8] = playtime[7]; // s

      String valueString = String(XCtl[i_xtouch].jogDialValue);
      if (XCtl[i_xtouch].jogDialValue < 10) {
        XCtl[i_xtouch].segmentDisplay[9] = '0';
        XCtl[i_xtouch].segmentDisplay[10] = '0';
        XCtl[i_xtouch].segmentDisplay[11] = valueString[0];
      }else if (XCtl[i_xtouch].jogDialValue < 100) {
        XCtl[i_xtouch].segmentDisplay[9] = '0';
        XCtl[i_xtouch].segmentDisplay[10] = valueString[0];
        XCtl[i_xtouch].segmentDisplay[11] = valueString[1];
      }else{
        XCtl[i_xtouch].segmentDisplay[9] = valueString[0];
        XCtl[i_xtouch].segmentDisplay[10] = valueString[1];
        XCtl[i_xtouch].segmentDisplay[11] = valueString[2];
      }

      // update all faders and buttons for the current channel-selection
      uint16_t newFaderValue;
      for (uint8_t i_ch=XCtl[i_xtouch].channelOffset; i_ch<(8+XCtl[i_xtouch].channelOffset); i_ch++) {
        uint8_t hardwareFader = i_ch - XCtl[i_xtouch].channelOffset;
        newFaderValue = round(((playerinfo.volumeCh[i_ch] + 48.0f)/54.0f) * 16383.0f);
        XCtl[i_xtouch].hardwareChannel[hardwareFader].faderNeedsUpdate = xctlCheckIfFaderNeedsUpdate(newFaderValue, XCtl[i_xtouch].hardwareChannel[hardwareFader].faderPositionHW) && (!XCtl[i_xtouch].hardwareChannel[hardwareFader].faderTouched);
        MackieMCU.channel[i_ch].faderPosition = newFaderValue; // 0..16383
        if (!XCtl[i_xtouch].hardwareChannel[hardwareFader].faderTouched) {
          XCtl[i_xtouch].hardwareChannel[hardwareFader].faderPositionHW = newFaderValue;
        }
        XCtl[i_xtouch].hardwareChannel[hardwareFader].meterLevel = ((uint16_t)playerinfo.vuMeterCh[i_ch] * 8) / 255; // scale 0..255 -> 0..8 (with CLIP-LED)
      }

      // update Masterfader
      newFaderValue = round(((playerinfo.volumeMain + 48.0f)/54.0f) * 16383.0f);
      XCtl[i_xtouch].hardwareMainfader.faderNeedsUpdate = xctlCheckIfFaderNeedsUpdate(newFaderValue, XCtl[i_xtouch].hardwareMainfader.faderPositionHW) && (!XCtl[i_xtouch].hardwareMainfader.faderTouched);
      if (!XCtl[i_xtouch].hardwareMainfader.faderTouched) {
        XCtl[i_xtouch].hardwareMainfader.faderPositionHW = newFaderValue;
      }
      MackieMCU.channel[32].faderPosition = newFaderValue; // convert volumeMain from dBfs to 0...16388 but keep logarithmic scale
    }
  }

  String XCtl_panString(uint8_t value) {
    if (value == 128) {
      return "<C>";
    }else if (value < 128) {
      return "L" + String(50 - (value/2.55), 0);
    }else{
      return "R" + String((value/2.55) - 50, 0);
    }
  }

  bool xctlCheckIfFaderNeedsUpdate(uint16_t desiredPosition, uint16_t hardwarePosition) {
    int16_t difference = desiredPosition - hardwarePosition;
    return abs(difference) > 2;
  }

  uint8_t xctlGetSegmentBitmap(char c) {
    switch (c) {
      case ' ': return 0x00; break;
      case '0': return 0x3f; break;
      case '1': return 0x06; break;
      case '2': return 0x5b; break;
      case '3': return 0x4f; break;
      case '4': return 0x66; break;
      case '5': return 0x6d; break;
      case '6': return 0x7d; break;
      case '7': return 0x07; break;
      case '8': return 0x7f; break;
      case '9': return 0x6f; break;
      case '-': return 0x40; break;
      default: return 0x00; break;
    }
  }

  void xctlInit(uint8_t i_xtouch) {
    // set default names for all 32 channels. Can be changed via UART lateron
    // we are using MackieMCU-functions for XCtl. So we are updating the MackieMCU-struct
    for (uint8_t i_ch=0; i_ch<32; i_ch++) {
      MackieMCU.channel[i_ch].name = "Ch " + String(i_ch + 1) + "   ";
    }

    XCtlWatchdogCounter[i_xtouch] = 20;

    XCtlUdp[i_xtouch].begin(10111);
  }

  void xctlStop(uint8_t i_xtouch) {
    XCtlUdp[i_xtouch].stop();
  }

  void xctlSendUdpPacket(uint8_t i_xtouch, const uint8_t *buffer, uint16_t size) {
    bool connectionOK = XCtlUdp[i_xtouch].beginPacket(XCtl[i_xtouch].ip, 10111);

    if (connectionOK) {
      uint16_t writtenBytes = XCtlUdp[i_xtouch].write(buffer, size);

      if (writtenBytes > 0) {
        // connection was OK, so close it gracefully
        if (!XCtlUdp[i_xtouch].endPacket()) {
          XCtl[i_xtouch].online = false; // disable this X-Touch-device, otherwise we jeopardize the usability of the system 
        }
      }else{
        // we have a connection-problem -> disable this X-Touch-device, otherwise we jeopardize the usability of the system 
        XCtl[i_xtouch].online = false;
      }
    }else{
      // we have a connection-problem -> disable this X-Touch-device, otherwise we jeopardize the usability of the system 
      XCtl[i_xtouch].online = false;
    }
  }

  void xctlSendWatchDogMessage(uint8_t i_xtouch) {
    xctlSendUdpPacket(i_xtouch, XCtl_IdlePacket, 7);
  }
#endif