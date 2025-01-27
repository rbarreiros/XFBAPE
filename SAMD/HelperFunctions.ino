// saturates a value to a specific minimum and maximum value
int16_t saturate(int16_t value, int16_t min, int16_t max) {
  if (value>max) {
    return max;
  }else if (value<min) {
    return min;
  }else{
    return value;
  }
}

// saturates a float-value to a specific minimum and maximum value
float saturate_f(float value, float min, float max) {
  if (value>max) {
    return max;
  }else if (value<min) {
    return min;
  }else{
    return value;
  }
}

float dBfs2pu(float dBfs, float min) {
  float dBfs_tmp;
  if (dBfs < min) {
    dBfs_tmp = -120;
  }else{
    dBfs_tmp = dBfs;
  }

  float pu = pow(10.0f, dBfs_tmp/100.0f); // 10^(dBfs/100)
  if (pu > 1) {
    pu = 1;
  }
  return pu;
}

String split(String s, char parser, int index) {
  String rs="";
  int parserCnt=0;
  int rFromIndex=0, rToIndex=-1;
  while (index >= parserCnt) {
    rFromIndex = rToIndex+1;
    rToIndex = s.indexOf(parser,rFromIndex);
    if (index == parserCnt) {
      if (rToIndex == 0 || rToIndex == -1) return "";
      return s.substring(rFromIndex,rToIndex);
    } else parserCnt++;
  }
  return rs;
}

uint8_t getNumberOfTocEntries(char separator) {
  uint8_t entries = 0;
  for (uint8_t i=0; i<TOC.length(); i++) {
    if (TOC[i] == separator) entries++;
  }
  return entries;
}

// convert seconds to string of hour, minute and seconds
String secondsToHMS(uint32_t seconds){
  unsigned int tme=0;
  tme = seconds;

  int hr = tme/3600;                        //Number of seconds in an hour
  int mins = (tme-hr*3600)/60;              //Remove the number of hours and calculate the minutes.
  int sec = tme-hr*3600-mins*60;            //Remove the number of hours and minutes, leaving only seconds.
  sec %= 60;
  mins %= 60;
  hr %= 24;

  if (seconds < 60) {
    // show only seconds
    return String(sec) + "s";
  }else if (seconds < 3600){
    return (String(mins) + "min " + String(sec) + "s");
  }else{
    return (String(hr) + "h " + String(mins) + "min " + String(sec) + "s");
  }
}

String secondsToHMS_B(uint32_t seconds, bool withDots = true){
  unsigned int tme=0;
  tme = seconds;

  int hr = tme/3600;                        //Number of seconds in an hour
  int mins = (tme-hr*3600)/60;              //Remove the number of hours and calculate the minutes.
  int sec = tme-hr*3600-mins*60;            //Remove the number of hours and minutes, leaving only seconds.
  sec %= 60;
  mins %= 60;
  hr %= 24;

  String s_hr;
  String s_min;
  String s_sec;
  if (hr<10) {
    s_hr = "0" + String(hr);
  }else{
    s_hr = String(hr);
  }
  if (mins<10) {
    s_min = "0" + String(mins);
  }else{
    s_min = String(mins);
  }
  if (sec<10) {
    s_sec = "0" + String(sec);
  }else{
    s_sec = String(sec);
  }

  if (withDots) {
    return (s_hr + ":" + s_min + ":" + s_sec);
  }else{
    return (s_hr + s_min + s_sec);
  }
}

#if USE_DISPLAY == 1
  String formatSize(uint32_t size) {
    if (size < 1024) {
      return String(size) + " B";
    }else if (size < (1024*1024)) {
      return String(size/1024.0f, 2) + " kB";
    }else if (size < (1024*1024*1024)) {
      return String(size/(1048576.0f), 2) + " MB";
    }else{
      return String(size/(1073741824.0f), 2) + " GB";
    }
  }
#endif

String IpAddress2String(const IPAddress& address){
  return String(address[0]) + "." + address[1] + "." + address[2] + "." + address[3];
}

String mac2String(byte ar[]) {
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%02X", ar[i]); // J-M-L: slight modification, added the 0 in the format for padding 
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}

void saveConfig(){
  eeprom.writeBlock(0, (uint8_t *) &eeprom_config, sizeof(eeprom_config));
}

void initEeprom() {
  eeprom.begin();

  if (!eeprom.isConnected())
  {
    Serial.println("Fatal Error: Can't find eeprom!!!");
    while(1);
  }

  // read installed EEPROM-size
  //Serial.print("Found EEPROM of size ");
  //Serial.print(eeprom.determineSize(false));
  //Serial.println(" byte.");

  // now load setup from eeprom
  eeprom.readBlock(0, (uint8_t *) &eeprom_config, sizeof(eeprom_config));
  eeprom.readBlock(0xFA, (uint8_t *) &config.mac, sizeof(config.mac)); // MAC address is in a non-writable ROM-part of this EEPROM

  // check if we have a valid IP-Address
  if ((eeprom_config.ip[0]==eeprom_config.ip[1]) && (eeprom_config.ip[1]==eeprom_config.ip[2]) && (eeprom_config.ip[2]==eeprom_config.ip[3])) {
    // IP-Address is invalid. So the configuration seems to be bad
    eeprom_config.ip = IPAddress(192, 168, 0, 52);
  }

  for (uint8_t i_xtouch=0; i_xtouch<XTOUCH_COUNT; i_xtouch++) {
    if ((eeprom_config.xtouchip[i_xtouch][0]==eeprom_config.xtouchip[i_xtouch][1]) && (eeprom_config.xtouchip[i_xtouch][1]==eeprom_config.xtouchip[i_xtouch][2]) && (eeprom_config.xtouchip[i_xtouch][2]==eeprom_config.xtouchip[i_xtouch][3])) {
      // IP-Address is invalid. So the configuration seems to be bad
      eeprom_config.xtouchip[i_xtouch] = IPAddress(192, 168, 0, 53+i_xtouch);
    }
  }
}

String intToHex(uint32_t val, uint8_t outputLength) {
  String hexString;
  for (int8_t shift = outputLength * sizeof(val) - 4; shift >= 0; shift -= 4) {
    uint8_t hexDigit = (val >> shift) & 0xF;
    hexString += String(hexDigit, HEX);
  }
  return hexString;
}

uint32_t hexToInt(String hexString){
  char c[hexString.length() + 1];
  hexString.toCharArray(c, hexString.length() + 1);
  return strtol(c, NULL, 16); 
}

void charToString(char *data, String &s)
{
  uint8_t ptr = 0;
  s = ""; // init string
  while (data[ptr]) {
    s.concat(data[ptr++]);
  }
}