# Raspberry Pi Pico 2 W Development Guide

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

[閱讀中文版](README.zh-tw.md)

This guide is designed for Raspberry Pi Pico 2 W developers, utilizing the official Raspberry Pi Pico Visual Studio Code Extension (Pico VS Code Extension) to simplify the C/C++ project creation, compilation, and flashing workflow.

The Pico 2 W is based on the RP2350 microcontroller chip and includes built-in wireless networking (CYW43439). This extension supports all Pico series devices, including the Pico 2 W.

---

## 1. Environment Setup and Extension Installation

Using the VS Code extension is the most convenient way to set up the Pico 2 W development environment, as it automatically manages the complex build system, SDK, and toolchain.

### 1.1 Install VS Code

Visual Studio Code (VS Code) is a popular open-source editor recommended for Pico project development.

* **macOS and Windows:** Please download and install from the [VS Code official website](https://code.visualstudio.com/).
* **macOS:** You can also install via Homebrew: `$ brew install --cask visual-studio-code`.
* **Linux/Raspberry Pi OS:** Run the following commands to install:

```bash
$ sudo apt update
$ sudo apt install code
```

### 1.2 Install the Extension and Dependencies

1. In VS Code, open the **Extensions** tab (or press `Ctrl+Shift+X` or `Cmd+Shift+X`).
2. In the search bar, type **Raspberry Pi Pico**, find the extension published by Raspberry Pi, and click **Install**.
3. **Dependencies:**
    * On **Raspberry Pi OS** or **Windows**, no additional dependencies are typically required.
    * On **macOS**, you need to run `xcode-select --install` to install requirements like Git, Tar, and the native C/C++ compiler.
    * On **Linux**, you may need to install Python 3.9 or higher, Git, Tar, and a native C/C++ compiler (with GCC support).

After successful installation, a Pico icon labeled "Raspberry Pi Pico Project" will appear in the VS Code Activity Bar (left side).

## 2. Creating a Project (C/C++)

You can choose to create a new blank project or start from an example (like `blink_simple`).

1. Click the **Raspberry Pi Pico Project** icon in the Activity Bar.
2. Select **New Project** or **New Project from Examples**.
3. **Project Configuration:**
    * Enter a project name in the **Name** field (e.g., `hello_uart`).
    * **Select Board type:** This is a critical step.
        * **`Pico 2 W`**: **(Recommended)** Select this. This will automatically configure and link the required libraries for the Pico 2 W's wireless chip (CYW43439).
        * **`Pico 2`**: This refers to the base Pico 2 (RP2350) board *without* wireless. If your project does **not** need Wi-Fi or Bluetooth, you can select this to save resources.
    * Choose a location to save the project files.
    * **Set STDIO support:** Since the Pico 2 W has built-in wireless, it is generally recommended to select **USB CDC**, which allows you to view serial console output over the same USB cable.
4. Click **Create**. The extension will automatically download the SDK and toolchain and generate the project files. The first run may take 5-10 minutes to install the toolchain.

## 3. Compiling and Running the Project

The compilation process converts your C/C++ code into a `.uf2` firmware file that can run on the Pico 2 W.

### 3.1 Compile

1. In the VS Code bottom status bar, find the **Compile** button and click it to start compiling.
2. Alternatively, you can select **Compile Project** from the Pico sidebar.
3. Compilation progress will be shown in the **Terminal** bottom panel. When successful, you will find the `.uf2` file in the build directory.

### 3.2 Entering BOOTSEL Mode

To flash the firmware to your Pico 2 W device, you must enter USB Mass Storage Mode:

1. Press and hold the **BOOTSEL** button on the Pico 2 W.
2. Connect the device to your computer via a Micro USB cable.
3. Once the device mounts as a disk named `RPI-RP2`, you can release the **BOOTSEL** button.

### 3.3 Run (Flashing)

While the device is in BOOTSEL mode:

1. Click the **Run** button in the bottom status bar, or click **Run project** in the sidebar.
2. The extension will automatically find the device and upload the compiled firmware (`.uf2` file) to the `RPI-RP2` virtual disk.
3. After the upload is complete, the device will automatically restart and begin running your new application.

## 4. Viewing Console Output (Serial Monitor)

If your project has USB CDC (USB Serial) support enabled (recommended for Pico 2 W):

1. From the top menu in VS Code, select **View**, then **Terminal** to open the bottom panel.
2. In that panel, navigate to the **Serial Monitor** tab.
3. Select the correct serial port and set the **Baud Rate** to **115200**.
4. Select **Start Monitoring** to see your program's runtime output (e.g., " Hello, UART!").

## 5. Debugging

For more advanced debugging, you can use a Raspberry Pi Debug Probe, or use a second Pico or Pico 2 device flashed with the `debugprobe` firmware to act as a debugger.

* Connect the Debug Probe to the Pico 2 W's 3-pin Arm serial wire debug (SWD) port.
* In VS Code, you can click the Pico icon in the sidebar and select **Debug Project** or press **F5** to start debugging.
* When prompted, select `Pico Debug (Cortex-Debug)` as the debugger. The debugger will automatically upload the code, set a breakpoint at the beginning of the `main` function, and run to that point.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
