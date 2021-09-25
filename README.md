# 🥞 hotcakey 🥞

hotcaKey is a global shortcut or hotkey utility for node.js and electron.

## install

```
npm i hotcakey
```

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

## supported os

- [x] macOS 10.7 or higher
- [ ] windows

## supported platform

- [x] node.js 12 or higher
- [ ] electron 11 or higher
