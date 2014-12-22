var locationOptions = {
  enableHighAccuracy: true, 
  maximumAge: 10000, 
  timeout: 10000
};

Pebble.addEventListener("ready", function(e) {
  returnConfigToPebble();
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
});

Pebble.addEventListener("appmessage", function(e) {
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);  
});

function locationError(err) {
  getMetar("http://api.av-wx.com/search?type=metar&ip=@detect");
}

function locationSuccess(pos) {
  getMetar("http://api.av-wx.com/search?type=metar&geo=" + pos.coords.latitude + "," + pos.coords.longitude);
}

function getMetar(url) {
  var oReq = new XMLHttpRequest();
  oReq.onreadystatechange = function() {
    if (oReq.readyState == 4 && oReq.status == 200) {
        var reports = JSON.parse(oReq.responseText);
        var metar = reports.reports[0].raw_text;
        Pebble.sendAppMessage({
          "METAR" : metar
        }, function(e) {
          console.log("metar sent succesfully");
        }, function(e) {
          console.log(e);
        }
      );
    }
  };
  oReq.onerror = function() { console.log("ERR"); };
 
  oReq.open("get", url, true);
  oReq.send();
 }

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
  return new Date().getTimezoneOffset() * 60;
}