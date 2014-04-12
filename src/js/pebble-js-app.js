var initialized = false;

Pebble.addEventListener("ready", function() {
  initialized = true;
});

Pebble.addEventListener("showConfiguration", function() {
  Pebble.openURL('https://asilaghi.github.com/beam-up-date-longlife-ro');
});

Pebble.addEventListener("webviewclosed", function(e) {
  var options = JSON.parse(decodeURIComponent(e.response));
  console.log(JSON.stringify(options));
});