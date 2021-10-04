import hotcakey from '../'

async function main() {
  await hotcakey.activate({ verbose: true })

  console.log('--- hotcakey baked ---')

  hotcakey.register(['control', 'shift', '/'], (event) => {
    console.log('%s event at %d', event.type, event.time)
  })
}

main()
