var Clay = require("clay");
var clayConfig = require("config.json");
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

Pebble.addEventListener("showConfiguration", function(e) {
	Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener("webviewclosed", function(e) {
	var options = clay.getSettings(e.response);

	console.log(JSON.stringify(options));
	
	var color = parseInt(options.colorscheme, 10);
	
	console.log(color);
	
	var colorscheme;

	if (options.colortoggle == 0 || options.colortoggle == "0") {
		colorscheme = 99;
	} else {
		colorscheme = parseInt(options.colorscheme, 10);
	}

	var primary = options.primary;
	var secondary = options.secondary;

	Pebble.sendAppMessage(
	{
		"KEY_ANIMATION": options.animation,
		"KEY_COLOR": colorscheme,
		"KEY_PRIMARY": primary,
		"KEY_SECONDARY": secondary
	},
	function(e) {
		console.log("Success sending to Pebble!");
	}, function(e) {
		console.log("Error sending to Pebble!");
	});

});
