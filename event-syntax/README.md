# Carcer Special Events - VS Code/Cursor Extension

A VS Code/Cursor extension that provides syntax highlighting and language support for Carcer special event files (`.se`).

## Features

- **Syntax Highlighting**: Color-coded syntax for special event files
  - Control characters (#>+|) in **red**
  - Variables (@VAR_NAME) in **purple**
  - Type keywords (k, kDup, kSwitch, etc.) in **green**
  - Text following colon without space (START_QUEST:WoodThief) in **orange**
  - Event titles in **green**
  - Event IDs in **blue**
  - Other event arguments in **purple**
  - Text after = or , in **purple**
  - Quoted text in **yellow**
- **Code Snippets**: Pre-built templates for common event patterns
- **Auto-indentation**: Automatic indentation for event structure
- **Folding**: Code folding for events and their children
- **Comments**: Support for `//` line comments

## Installation

### Manual Installation

1. Copy the `event-syntax` folder to your VS Code/Cursor extensions directory:
   - Windows: `%USERPROFILE%\.vscode\extensions\` or `%USERPROFILE%\.cursor\extensions\`
   - macOS: `~/.vscode/extensions/` or `~/.cursor/extensions/`
   - Linux: `~/.vscode/extensions/` or `~/.cursor/extensions/`

2. Restart VS Code/Cursor

3. Open a file with `.se` extension

4. (Optional) Select the "Carcer Special Events" theme from the theme selector for the custom color scheme

### Development Installation

1. Clone or download this extension
2. Open the `event-syntax` folder in VS Code/Cursor
3. Press `F5` to run the extension in a new Extension Development Host window
4. Open a `.se` file to test the syntax highlighting

## File Extensions

The extension recognizes files with the `.se` extension (Carcer Special Events).

## Syntax Overview

### Event Header
```
#eventId,Event Title,icon_name
```
- Event ID is highlighted in blue
- Event Title is highlighted in green
- Icon name is highlighted in purple

### Variables
```
@VAR_NAME=value
@VAR_NAME
```
Variables are highlighted in purple both when declared and used.

### Child Nodes
```
>childId,childType
```
- Child ID is highlighted in blue
- Child Type is highlighted in green

### Properties
```
+p: "Text description"
+k: keywordName|"Text response"
+kDup: keywordName|targetKeyword
+kSwitch: switchKeyword
  |condition:nextId
  |default:defaultNext
+n: nextId
+e: COMMAND:arg1|SET:@VAR_NAME
```
- Property keywords (k, kDup, kSwitch, kChild, e, n, p, c, keyword, pass, fail, check) are highlighted in green
- Text following colon without space (e.g., `COMMAND:arg1`) has the argument highlighted in orange

### Quoted Strings
```
"This is a quoted string"
```
Quoted strings are highlighted in yellow.

## Example

```se
// this is an example event based around a "choice" child
#event0,Event 0,icon_0
>0,choice
  +p: "This is a basic choice that you can make."
  +c: 1|Continue.
  +c: 4|I already know what to do.
>1,bool
  +check: vars.hasBeenHereBefore
  +pass: pass1
  +fail: fail1
>pass1,exec
  +p: "Looks like you've been here before."
  +n: 4
>4,exec
  +p: "This is the last node."
  +n: endEvent
>endEvent,end
```

## Color Scheme

The extension includes a custom color theme that can be selected from the theme picker:
- **Control Characters**: Red (#FF0000)
- **Variables**: Purple (#CE93D8)
- **Type Keywords**: Green (#4EC9B0)
- **Event IDs**: Blue (#569CD6)
- **Colon Arguments**: Orange (#D7BA7D)
- **Quoted Strings**: Yellow (#CE9178)

## Contributing

Feel free to submit issues or pull requests to improve the extension!
