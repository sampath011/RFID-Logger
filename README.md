# RFID-Logger | Firmware Details
---------------------------------
Pay attention to the version mentioned in the file name. The latest update is # V6

 # V6 Includings
 - - - - - - - - 

 * There are 3 messages sent from the device to the MQTT broker.
   1. RFID card details  - Sent whenever detected
   2. Analog Pin Values  - Sent each 1 second. There are 4 analog pins (declared at the beginning)
   3. Digital Pin Values - Sent whenever a change (LOW to HIGH) is detected
  
 # Common Features
 - - - - - - - - -

 * A reset button, restarts the device whenever pressed.
 * Displays events on the LCD.
 * Connects to WiFi and MQTT broker and synchronizes the time when turned on.
