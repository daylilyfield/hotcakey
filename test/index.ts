import hotcakey from '../'

async function main() {
  console.log('🥞 activate hotcakey...')

  await hotcakey.activate({ verbose: false })

  console.log('🥞 hotcakey baked!')

  let keydown = false
  let keyup = false

  let unsubscribe = hotcakey.register(['control', 'shift', '/'], (event) => {
    if (event.type === 'keydown') {
      keydown = true
      console.log('⭕️ keydown detected')
    }
    if (event.type === 'keyup') {
      keyup = true
      console.log('⭕️ keyup detected')
    }
  })

  //
  // hotcakey can detec keydown and keyup events
  //

  console.log('⌨️  please press and release "CTRL + SHIFT + /" in 5 seconds')

  await sleep(5)

  assert(keydown, '❌ keydown event missed')
  assert(keyup, '❌ keyup event missed')

  console.log('🎉 congrats! both keydown and keyup events detected')

  //
  // hotcakey can NOT detec keydown and keyup events
  //

  keydown = false
  keyup = false

  unsubscribe()

  console.log('⌨️  please press and release "CTRL + SHIFT + /" in 5 seconds')

  await sleep(5)

  assert(!keydown, '❌ keydown event detected')
  assert(!keyup, '❌ keyup event detected')

  console.log('🎉 congrats! neither keydown nor keyup events detected ---')

  console.log('🥞 inactivate hotcakey... ---')

  hotcakey.inactivate()

  console.log('🥞 hotcakey inactivated ---')
}

function assert(condition: boolean, message: string) {
  if (!condition) {
    throw new Error('Assertion Failed: ' + message)
  }
}

function sleep(seconds: number): Promise<void> {
  return new Promise((resolve) => {
    setTimeout(resolve, seconds * 1000)
  })
}

main().catch((err) => {
  console.error(err)
  hotcakey.inactivate()
})
