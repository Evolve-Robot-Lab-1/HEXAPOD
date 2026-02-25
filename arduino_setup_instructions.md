# Arduino CLI Setup and Sketch Upload Instructions

This document outlines the steps to set up the Arduino CLI, identify connected boards, and upload a basic "Blink" sketch, including common troubleshooting for permission issues on Linux.

## 1. Install Arduino CLI

The Arduino CLI is required to interact with your Arduino boards from the command line.

**Command to install:**
```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/bin sh
```
*   This command downloads and runs the official Arduino CLI installation script, placing the `arduino-cli` executable in your `~/bin` directory.
*   You may need to add `~/bin` to your system's `PATH` environment variable if you want to run `arduino-cli` from any directory without specifying the full path.

## 2. Check for Connected Arduino Boards

After installing the CLI, you can list connected boards.

**Command to list boards:**
```bash
~/bin/arduino-cli board list
```
*   This command will show a list of connected Arduino boards, their ports, and FQBNs (Fully Qualified Board Names).
*   Example output:
    ```
    Port         Protocol Type              Board Name  FQBN            Core
    /dev/ttyACM0 serial   Serial Port (USB) Arduino Uno arduino:avr:uno arduino:avr
    ```

## 3. Upload a "Blink" Sketch

This section covers creating, compiling, and uploading a standard "Blink" sketch.

### 3.1. Create the Sketch

First, create a new sketch directory and file.

**Command to create sketch:**
```bash
/home/evolve/bin/arduino-cli sketch new Blink
```
*   This creates a directory named `Blink` (e.g., `/home/evolve/Arduino/Spider/Blink`) and an empty `Blink.ino` file inside it.

### 3.2. Write the Blink Code

Populate the `Blink.ino` file with the standard Arduino Blink example.

**Content for `Blink.ino`:**
```cpp
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
```

### 3.3. Compile the Sketch

Compile the sketch for your specific board. In this example, we use `arduino:avr:uno` as the FQBN for an Arduino Uno.

**Command to compile:**
```bash
/home/evolve/bin/arduino-cli compile --fqbn arduino:avr:uno /home/evolve/Arduino/Spider/Blink
```

### 3.4. Upload the Sketch (and Permission Fix)

Attempt to upload the compiled sketch to the board. If you encounter permission errors, follow the troubleshooting steps.

**Initial Upload Command:**
```bash
/home/evolve/bin/arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno /home/evolve/Arduino/Spider/Blink
```
*   Replace `/dev/ttyACM0` with your board's port and `arduino:avr:uno` with your board's FQBN if different.

#### Troubleshooting: Permission Denied Error (`avrdude: ser_open(): can't open device "/dev/ttyACM0": Permission denied`)

If you receive a "Permission denied" error, your user account does not have the necessary permissions to access the serial port.

**Solution: Add user to `dialout` group**

1.  **Add your user to the `dialout` group:**
    ```bash
    sudo usermod -a -G dialout $USER
    ```
    *   This command adds your current user (`$USER`) to the `dialout` group, which grants access to serial ports. You will be prompted for your password.
    *   **Important:** After running this command, you **must either log out and log back in** to your Linux session for the changes to take effect, or use `su - $USER` to start a new login shell with updated group permissions.

2.  **Re-attempt Upload (after logging in/using `su -`):**
    After updating your group permissions (by logging out/in or using `su -`), you can try the upload command again. If using `su -`, you'll wrap the upload command:
    ```bash
    su - $USER -c "/home/evolve/bin/arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno /home/evolve/Arduino/Spider/Blink"
    ```
    *   Again, replace port and FQBN as needed.