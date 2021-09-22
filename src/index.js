const hotcakey = require('bindings')('hotcakey')

function main() {
  const first = hotcakey.on('keydown', ['Shift', 'Space'], () => {
    console.log('first')
  })

  first()
}

main()
