Pebble.addEventListener("ready", function(e) {
  returnConfigToPebble();
});

function returnConfigToPebble() {
 Pebble.sendAppMessage({
   "TIMEZONEOFFSET" : parseInt(TimezoneOffsetSeconds()),
   "TIMEZONEABBR"   : getTimezoneAbbr()
 }, function(e) {
   console.log("sent succesfully");
 }, function(e) {
   console.log(e);
 });
}

function getTimezoneAbbr() {
  return String(String(new Date()).split("(")[1]).split(")")[0]; 
}

function TimezoneOffsetSeconds() {
  console.log("hi!");
  // Get the number of seconds to add to convert localtime to utc
  return new Date().getTimezoneOffset() * 60;
}
