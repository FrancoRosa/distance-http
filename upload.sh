pkill -f picocom
echo '... compile & upload'
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 distance-http.ino -u -p /dev/ttyUSB0
echo '... done'
# picocom /dev/ttyUSB0 -b 115200