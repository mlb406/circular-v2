var Clay = require("clay");
var clayConfig = require("config.json");
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

Pebble.addEventListener("showConfiguration", function(e) {
	Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener("webviewclosed", function(e) {
	var options = clay.getSettings(e.response);

	console.log(JSON.stringify(options));
	
	var animation;

	Pebble.sendAppMessage({"KEY_ANIMATION": options.animation},
	function(e) {
		console.log("Success sending to Pebble!");
	}, function(e) {
		console.log("Error sending to Pebble!");
	});

});
