# Witch Potion Events - VS Code Extension

A VS Code extension that provides syntax highlighting and language support for Witch Potion event files.

## Features

- **Syntax Highlighting**: Color-coded syntax for event files
- **Code Snippets**: Pre-built templates for common event patterns
- **Auto-indentation**: Automatic indentation for event structure
- **Folding**: Code folding for events and their children
- **Comments**: Support for `//` line comments and `/* */` block comments

## Installation

### Manual Installation

1. Copy the `event-syntax` folder to your VS Code extensions directory:
   - Windows: `%USERPROFILE%\.vscode\extensions\`
   - macOS: `~/.vscode/extensions/`
   - Linux: `~/.vscode/extensions/`

2. Restart VS Code

3. Open a file with `.wpe` or `.events` extension

### Development Installation

1. Clone or download this extension
2. Open the `event-syntax` folder in VS Code
3. Press `F5` to run the extension in a new Extension Development Host window
4. Open a `.wpe` file to test the syntax highlighting

## File Extensions

The extension recognizes files with these extensions:
- `.wpe` (Witch Potion Events)
- `.events`

## Syntax Overview

### Event Header
```
!Event Title,icon_name
```

### Child Nodes
```
>id,type:
```

Types: `choice`, `dice`, `end`, `modify`

### Commands
- `+p:` - Description text
- `+c:` - Choice option
- `+dice:` - Dice specification
- `+add:` - Add resources
- `+rem:` - Remove resources
- `+n:` - Next node reference
- `+pass:` - Pass threshold
- `+fail:` - Fail threshold

### Resources
- `GOLD`
- `HERB_SPARKLEWEED`, `HERB_FEYMOSS`, etc.
- `REAG_SKYDUST`, `REAG_FAIRY_SALT`, etc.
- `POT_COLD_CURE`, `POT_DRAGON_SWEAT`, etc.

### Dice Faces
- `FIRE_MAGIC`
- `HEART_MAGIC`
- `LUCK`
- `CAT`
- `BLANK`

## Code Snippets

Type these prefixes to get code snippets:

- `event` - Complete event template
- `header` - Event header
- `choice` - Choice node
- `dice` - Dice node
- `modify` - Modify node
- `end` - End node
- `desc` - Description line
- `opt` - Choice option
- `dice-spec` - Dice specification
- `add` - Add resources
- `rem` - Remove resources
- `next` - Next node reference
- `pass` - Pass threshold
- `fail` - Fail threshold

## Example

```wpe
!The Wizard,icon_wizard
>0,choice
  +p: An old wizard enters your shop. He challenges you to a duel.
  +c: 1|Accept the challenge!
  +c: e|Decline politely
>1,dice
  +p: The wizard readies a magic spell!
  +dice: 3 FIRE_MAGIC
  +pass: 3
  +fail: 4
>e,end
>3,modify
  +p: You win! The wizard gives you 2 GOLD.
  +add: 2 GOLD
  +n: e
>4,modify
  +p: You lose! The wizard takes 2 HERB_SPARKLEWEED.
  +rem: 2 HERB_SPARKLEWEED
  +n: e
```

## Contributing

Feel free to submit issues or pull requests to improve the extension!
