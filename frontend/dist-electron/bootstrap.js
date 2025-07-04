// dist-electron/bootstrap.js
const { app } = require('electron');

// MUST be set before app is even accessed
app.commandLine.appendSwitch('no-sandbox');

// Now start your real main script
require('./main.js');
