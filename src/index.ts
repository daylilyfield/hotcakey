import bindings from 'bindings'

/**
 * `codes` represents a physical key on the keyboard.
 *
 * NOTICE:
 * in the web, key code modifiers has ~Left or ~Right
 * suffix such as ShiftLeft, ControlRight and so on.
 * although almost all platforms do not distinguish
 * left and right modifiers in case of using as hotkey,
 * hotcakey handles them as one physical key.
 *
 * @see https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/code
 */
const codes = [
  'KeyA',
  'KeyB',
  'KeyC',
  'KeyD',
  'KeyE',
  'KeyF',
  'KeyG',
  'KeyH',
  'KeyI',
  'KeyJ',
  'KeyK',
  'KeyL',
  'KeyM',
  'KeyN',
  'KeyO',
  'KeyP',
  'KeyQ',
  'KeyR',
  'KeyS',
  'KeyT',
  'KeyU',
  'KeyV',
  'KeyW',
  'KeyX',
  'KeyY',
  'KeyZ',
  'Digit1',
  'Digit2',
  'Digit3',
  'Digit4',
  'Digit5',
  'Digit6',
  'Digit7',
  'Digit8',
  'Digit9',
  'Digit0',
  'Minus',
  'Equal',
  'BracketLeft',
  'BracketRight',
  'Backslash',
  'Semicolon',
  'Quote',
  'Backquote',
  'Comma',
  'Period',
  'Slash',
  'Enter',
  'Escape',
  'Backspace',
  'Tab',
  'Space',
  'CapsLock',
  'F1',
  'F2',
  'F3',
  'F4',
  'F5',
  'F6',
  'F7',
  'F8',
  'F9',
  'F10',
  'F11',
  'F12',
  'F13',
  'F14',
  'F15',
  'F16',
  'F17',
  'F18',
  'F19',
  'F20',
  'F21',
  'F22',
  'F23',
  'F24',
  'PrintScreen',
  'ScrollLock',
  'Pause',
  'Insert',
  'Home',
  'PageUp',
  'Delete',
  'End',
  'PageDown',
  'ArrowRight',
  'ArrowLeft',
  'ArrowDown',
  'ArrowUp',
  'NumLock',
  'NumpadDivide',
  'NumpadMultiply',
  'NumpadSubtract',
  'NumpadAdd',
  'NumpadEnter',
  'Numpad1',
  'Numpad2',
  'Numpad3',
  'Numpad4',
  'Numpad5',
  'Numpad6',
  'Numpad7',
  'Numpad8',
  'Numpad9',
  'Numpad0',
  'NumpadDecimal',
  'IntlBackslash',
  'ContextMenu',
  'NumpadEqual',
  'Power',
  'Help',
  'Undo',
  'Cut',
  'Copy',
  'Paste',
  'AudioVolumeMute',
  'AudioVolumeUp',
  'AudioVolumeDown',
  'NumpadComma',
  'IntlRo',
  'KanaMode',
  'IntlYen',
  'Convert',
  'NonConvert',
  'Lang1',
  'Lang2',
  'Lang3',
  'Lang4',
  'MediaTrackNext',
  'MediaTrackPrevious',
  'MediaStop',
  'Eject',
  'MediaPlayPause',
  'MediaSelect',
  'LaunchMail',
  'LaunchApp2',
  'LaunchApp1',
  'BrowserSearch',
  'BrowserHome',
  'BrowserBack',
  'BrowserForward',
  'BrowserStop',
  'BrowserRefresh',
  'BrowserFavorites',
  'Sleep',
  'WakeUp',
  'ControlRight',
  'ControlLeft',
  'ShiftRight',
  'ShiftLeft',
  'AltRight',
  'AltLeft',
  'MetaRight',
  'MetaLeft',

  // hotcakey only, not web standard
  'Control',
  'Shift',
  'Alt',
  'Meta',
] as const

/**
 * `Code` represents a physical key on the keyboard.
 */
export type Code = typeof codes[number]

export type Option = { verbose: boolean }
export type Unsubscribe = () => void
export type HotKeyEvent = { type: 'keydown' | 'keyup'; time: number }
export type ErrorEvent = { type: 'error'; code: string; time: number }
export type Event = HotKeyEvent | ErrorEvent
export type Listener = (event: Event) => void

const addon = bindings('hotcakey')
const defaultOption: Option = { verbose: false }

let verbose: boolean

export function activate(option: Option = defaultOption): Promise<void> {
  verbose = option.verbose
  return addon.activate(option)
}

export function inactivate(): Promise<void> {
  return addon.inactivate()
}

export function register(codes: Code[], listener: Listener): Unsubscribe {
  check(codes && codes.length > 0, 'missing shortcut keys to register')
  check(!!listener, 'missing hotkey listener')

  log('codes to register:', codes)

  check(codes.every(isCode), `some key is not a type of Code`)

  return addon.register(codes, listener)
}

function isCode(suspect: Code): boolean {
  return codes.includes(suspect)
}

//
// utilities
//

function log(message: string, ...args: unknown[]): void {
  if (verbose) {
    console.log('[hotcakey:dbg] ' + message, ...args)
  }
}

function check(condition: boolean, message: string) {
  if (!condition) {
    throw new Error('[hotcakey:err] ' + message)
  }
}
