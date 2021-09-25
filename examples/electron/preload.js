const { ipcRenderer, contextBridge } = require('electron')

contextBridge.exposeInMainWorld('ipcRenderer', {
  // YES, I KNOW WHAT I DO.
  on: ipcRenderer.on.bind(ipcRenderer),
})
