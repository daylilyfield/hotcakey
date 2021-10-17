# 🥞 hotcakey 🥞

hotcaKey is the global shortcut (aka hotkey) module for node.js and electron.

hotcakey is now actively under deploment, so api may have breaking changes even in a short term.

## motivation

i need to detect global shortcut `keydown` and `keyup` events on electron platform. i found a few solution such as [iohook](https://github.com/wilix-team/iohook/). iohook is the excellent module but too powerfull for me. i want to know only key events which has the predetermined combination.

## feature

- detecting key combination globally even if your application does not have a focus.
- working with node.js and electron.
- working on macOS, windows. (and linux for near future)

## install

```sh
npm i hotcakey
```

if you use hotcakey with electron, you may need to run [electron-rebuild](https://github.com/electron/electron-rebuild).

## usage

```typescript
import hotcakey from 'hotcakey'

async function main() {
  await hotcakey.activate()

  hotcakey.register(['shift', 'space'], (event) => {
    console.log('%s event at %d', event.type, event.time)
  })
}

main()
```

## examples

please check out the './examples' directory. you can find these for node.js and electron there.

## supported os

- [x] macOS 10.7 or higher
- [x] windows 10 or higher
- [ ] linux

## supported platform

- [x] node.js 14.14 or higher
- [x] electron 14 or higher
