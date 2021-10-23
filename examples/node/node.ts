import hotcakey from '../../'

async function main() {
  await hotcakey.activate({ verbose: true })

  console.log('🥞 hotcakey baked 🥞')
  console.log('⌨️i press "Shift + Space" to enjoy hotcakey')
  console.log('⏰ exit automatically in 5 seconds')

  hotcakey.register(['Shift', 'Space'], (event) => {
    console.log('%s event detected at %d', event.type, event.time)
  })

  await new Promise((resolve) => setTimeout(resolve, 5000))

  await hotcakey.inactivate()

  console.log('--- yumyum ---')
}

main()
