# Operating Systems

## Fall 24-25

### Medipol Shell: A Custom Command-Line Interface

--- 

## Overview

This project implements a custom command-line shell named **Medipol Shell (medsh)**, written in C. Designed for Linux, Unix, or macOS systems, it leverages the GNU Readline library for enhanced input handling. The shell supports user management, command history, alias expansion, and background command execution.

## Features

- **Custom Prompt**: Updates dynamically when a user is set.
- **User Management**: Allows setting and unsetting a user.
- **Command History**: Saves the last 10 commands for easy access.
- **Alias Support**: Loads and expands aliases from a profile file.
- **Background Execution**: Runs commands in the background using `&`.

## How It Works

1. **Initialization**:  
   - The shell starts with the default prompt `medsh>`.
   - It automatically loads alias definitions from `~/.profile_medsh` if the file exists.

2. **User Management and Prompt Update**:  
   - Use `setusr <username>` to assign a user, updating the prompt to `medsh(username)>`.
   - Use `unsetusr` to remove the current user, reverting the prompt to `medsh>`.

3. **Command History and Alias Expansion**:  
   - The shell saves the last 10 commands, enabling quick access via the history mechanism.
   - Aliases defined in the profile file are expanded, allowing shortcuts for frequently used commands.

4. **Background Execution**:  
   - Commands ending with `&` are executed in the background, and the shell displays the corresponding process ID.

## Requirements

- **Compiler**: GCC (GNU Compiler Collection)  
- **Library**: GNU Readline  
  - On Ubuntu, install with:  
    
    ```bash
    sudo apt-get install libreadline-dev
    ```
  
- **Platform**: Linux, Unix, or macOS

## Setup and Usage

### 1. Compile the Shell

Open a terminal in the directory containing `medsh.c` and run:

```sh
gcc -o medsh medsh.c -lreadline
```

This command compiles the source code and creates an executable named `medsh`.

### 2. Run the Shell

Start the shell by executing:

```sh
./medsh
```

The prompt will appear as `medsh>`.
If a user is set using `setusr`, the prompt will change to `medsh(username)>`.

### 3. Use Built-in Commands

- **Set User**:

  ```sh
  setusr <username>
  ```

- **Unset User**:

  ```sh
  unsetusr
  ```

- **Exit the Shell**:

  ```sh
  exit
  ```

- **View Command History**:

  ```sh
  history
  ```

### 4. Configure Aliases

Create or modify the file `~/.profile_medsh` to define your aliases. For example:

```sh
alias ll='ls -l'
alias hi='echo Hello, Medipol!'
```

These aliases will be loaded automatically when the shell starts.

### 5. Background Command Execution

To execute a command in the background, append an `&` at the end. For example:

```sh
sleep 10 &
```

The shell will execute the command in the background and display its process ID.

---  
 
**Student Name:** Laith Ismail Ahmed Hijazi  
**Student ID:** 64220039  
**Email:** laith.hijazi@std.medipol.edu.tr
