import hotcakey from '../'

async function main() {
  await hotcakey.activate()

  console.log('hotcakey baked!')

  const subscription = hotcakey.register(['shift', 'space'], (event) => {
    console.log('%s: shift + space at %d', event.type, event.time)
  })

  await new Promise((resolve) => setTimeout(resolve, 5000))

  subscription()

  console.log('unsubscribed')
}

main()
