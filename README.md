# Integrated ISA Project (Labs 1–5)

This project integrates all components from **Lab 1 to Lab 5** into a single working system:
a shell-driven language implementation with parsing, IR generation, virtual machine execution,
debugging support, and garbage collection.

---

## Build Instructions

Requirements:
- C++17 compiler (clang++ / g++)
- flex
- bison
- make

Build the project:

```bash
make
````

This produces the executable:

```bash
ipshell
```

---

## Running the Shell

Start the interactive shell:

```bash
./ipshell
```

You should see:

```text
ipshell>
```

All commands below are entered **inside `ipshell`**, not in the OS terminal.

---

## Shell Commands

### Program Management

* `submit <file>`
  Parses and loads a program, assigns it a PID.

* `run <pid>`
  Executes the program using the virtual machine.

* `kill <pid>`
  Terminates the program and frees its resources.

### Debugging (Lab 4)

* `debug <pid>`
  Runs the program in instruction-level debug mode.

  Debug commands:

  * `s` → step one IR instruction
  * `c` → continue execution
  * `q` → exit debug mode

### Garbage Collection (Lab 5)

* `memstat <pid>`
  Displays the number of heap objects tracked by the GC.

* `gc <pid>`
  Triggers garbage collection.

* `leaks <pid>`
  Reports remaining heap objects.

### Exit

* `exit`
  Exits the shell.

---

## Example Session

```text
ipshell> submit example.lang
PID = 1
ipshell> debug 1
(debug pc=0)> s
(debug pc=1)> c
Program finished
ipshell> memstat 1
Heap objects: 1
ipshell> gc 1
GC complete
ipshell> kill 1
Program 1 killed
ipshell> exit
```

---

## Architecture Overview

```
Shell
  ↓
Parser (Flex/Bison)
  ↓
AST
  ↓
Intermediate Representation (IR)
  ↓
Virtual Machine (VM)
  ↓
Garbage Collector (GC)
```

* The **shell** manages programs and PIDs.
* The **parser** builds an AST from source code.
* The **AST** lowers into an intermediate representation (IR).
* The **VM** executes IR instructions.
* The **GC** tracks heap objects and performs garbage collection.
* Debugging and memory management operate through the VM.

---

## Integration Guarantee

All labs are fully integrated:

* Removing the **parser** prevents program submission.
* Removing the **AST** prevents IR generation.
* Removing the **IR** breaks VM execution.
* Removing the **VM** stops all execution.
* Removing the **GC** breaks memory commands.

This demonstrates full integration of **Labs 1–5** as required.

---

## Notes

* Execution is performed **only by the virtual machine**.
* The AST is used strictly for IR generation.
* Garbage collection is integrated into the VM and exposed via shell commands.
* The project builds and runs on macOS and Linux environments.

---

## Author

Student project for ISA — Integrated Labs 1–5 by Astitwa Saxena and Abhishek Gupta
