import hotcakey from '../../'

async function main() {
  await hotcakey.activate()

  console.log('--- hotcakey baked ---')

  hotcakey.register(['shift', 'space'], (event) => {
    console.log('%s event at %d', event.type, event.time)
  })

  // enjoy hotcakey in 5 seconds
  await new Promise((resolve) => setTimeout(resolve, 5000))

  await hotcakey.inactivate()

  console.log('--- yumyum ---')
}

main()
