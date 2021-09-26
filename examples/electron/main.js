const path = require('path')
const { app, BrowserWindow } = require('electron')
const hotcakey = require('hotcakey')

let win

function createWindow() {
  win = new BrowserWindow({
    width: 400,
    height: 200,
    webPreferences: {
      nodeIntegration: false,
      enableRemoteModule: true,
      contextIsolation: true,
      preload: path.join(__dirname, 'preload.js'),
    },
  })

  win.loadFile('index.html')
}

app.on('ready', async () => {
  await hotcakey.activate()

  hotcakey.register(['shift', 'space'], (event) => {
    console.log('%s event at %d', event.type, event.time)
    win.webContents.send(event.type)
  })

  createWindow()
})

app.on('will-quit', async () => {})
