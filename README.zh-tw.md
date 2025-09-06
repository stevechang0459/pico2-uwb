# Raspberry Pi Pico 2 W 開發指南

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

[Read this in English](README.md)

此說明是專為 Raspberry Pi Pico 2 W 開發者設計，利用官方的 Raspberry Pi Pico Visual Studio Code Extension（或稱 Pico VS Code Extension）來簡化 C/C++ 專案的建立、編譯及燒錄流程。

Pico 2 W 基於 RP2350 微控制器晶片，並內建無線網路功能 (CYW43439)。此擴充功能支援所有 Pico 系列裝置，包括 Pico 2 W。

---

## 1. 環境準備與擴充功能安裝

使用 VS Code 擴充功能是設定 Pico 2 W 開發環境最便捷的方法，它能自動管理複雜的建置系統、SDK 和工具鏈。

### 1.1 安裝 VS Code

Visual Studio Code (VS Code) 是一個受歡迎的開源編輯器，推薦用於 Pico 專案開發。

* **macOS 和 Windows：** 請從 [VS Code 官方網站](https://code.visualstudio.com/)下載安裝。
* **macOS：** 也可以使用 Homebrew 透過指令安裝：`$ brew install --cask visual-studio-code`。
* **Linux/Raspberry Pi OS：** 執行以下指令安裝：

```bash
$ sudo apt update
$ sudo apt install code
```

### 1.2 安裝擴充功能及依賴項

1. 在 VS Code 中，開啟「**Extensions**」分頁（或按下 `Ctrl+Shift+X` 或 `Cmd+Shift+X`）。
2. 在搜尋欄中輸入 **Raspberry Pi Pico**，找到由 Raspberry Pi 發布的擴充功能，點擊 **Install** 安裝。
3. **依賴項 (Dependencies)：**
    * 在 **Raspberry Pi OS** 或 **Windows** 上，通常不需要額外的依賴項。
    * 在 **macOS** 上，需要執行 `xcode-select --install` 來安裝 Git、Tar 和原生 C/C++ 編譯器等要求。
    * 在 **Linux** 上，可能需要安裝 Python 3.9 或更高版本、Git、Tar 和原生的 C/C++ 編譯器（支援 GCC）。

安裝成功後，VS Code 活動欄（左側）會出現一個標籤為 "Raspberry Pi Pico Project" 的 Pico 圖標。

## 2. 建立專案 (C/C++)

您可以選擇建立一個新的空白專案，或從範例開始 (例如 `blink_simple`)。

1. 點擊活動側邊欄中的 **Raspberry Pi Pico Project** 圖標。
2. 選擇 **New Project** 或 **New Project from Examples**。
3. **專案配置：**
    * 在 **Name** 欄位輸入專案名稱（例如 `hello_uart`）。
    * **選擇板型 (Board type)：** 這是關鍵步驟。
        * **`pico 2 W`**：**（建議）** 選擇此項。這會自動為您的專案設定並連結 Pico 2 W 的無線網路 (CYW43439) 相關函式庫。
        * **`Pico 2`**：這是指不含無線功能的 Pico 2 (RP2350) 基板。如果您的專案**不需要** Wi-Fi 或藍牙功能，可以選擇此項以節省資源。
    * 選擇專案檔案的儲存位置。
    * **設定 STDIO 支援：** 由於 Pico 2 W 內建無線功能，通常建議選擇 **USB CDC**，這允許透過同一條 USB 線查看序列埠輸出 (Serial Console Output)。
4. 點擊 **Create**。擴充功能將自動下載 SDK 和工具鏈，並生成專案文件。首次運行可能需要 5-10 分鐘來安裝工具鏈。

## 3. 編譯與執行專案

編譯過程會將您的 C/C++ 程式碼轉換為可在 Pico 2 W 上運行的 `.uf2` 韌體檔案。

### 3.1 編譯

1. 在 VS Code 底部狀態欄中，找到 **Compile** 按鈕，點擊即可開始編譯。
2. 或者，也可以在 Pico 側邊欄中選擇 **Compile Project**。
3. 編譯進度將顯示在終端機 (**Terminal**) 底部面板中。成功後，您會在建置目錄中找到 `.uf2` 檔案。

### 3.2 進入 BOOTSEL 模式

要將韌體燒錄到 Pico 2 W 設備上，必須進入 USB 大容量儲存模式 (USB Mass Storage Mode)：

1. 按住 Pico 2 W 上的 **BOOTSEL** 按鈕。
2. 透過 Micro USB 連接線將設備連接到您的電腦。
3. 一旦設備掛載為一個名為 `RPI-RP2` 的磁碟，即可釋放 **BOOTSEL** 按鈕。

### 3.3 運行 (燒錄)

當設備處於 BOOTSEL 模式時：

1. 點擊底部狀態欄中的 **Run** 按鈕，或在側邊欄中點擊 **Run project**。
2. 擴充功能會自動找到設備，將編譯好的韌體（`.uf2` 文件）上傳到 `RPI-RP2` 虛擬磁碟。
3. 上傳完成後，設備將自動重新啟動，並開始運行您的新應用程式。

## 4. 查看控制台輸出 (序列埠監控)

如果您的專案啟用了 USB CDC (USB Serial) 支援（建議用於 Pico 2 W）：

1. 在 VS Code 頂部菜單中選擇 **View**，然後選擇 **Terminal** 打開底部面板。
2. 在該面板中，導航至 **Serial Monitor** 分頁。
3. 選擇正確的序列埠，並將波特率 (Baud Rate) 設定為 **115200**。
4. 選擇 **Start Monitoring** 即可查看程式運行時的輸出（例如 ` Hello, UART!`）。

## 5. 除錯 (Debugging)

若要進行更進階的除錯，您可以使用 Raspberry Pi Debug Probe，或使用第二個 Pico 或 Pico 2 設備燒錄 `debugprobe` 韌體充當調試器。

* 將 Debug Probe 連接到 Pico 2 W 的 3-pin Arm serial wire debug (SWD) port。
* 在 VS Code 中，可以點擊側邊欄中的 Pico 圖標，選擇 **Debug Project** 或按下 **F5** 開始除錯。
* 在提示時，選擇 `Pico Debug (Cortex-Debug)` 作為調試器。調試器將自動下載程式碼，在 `main` 函數的開頭設定一個中斷點並運行到該處。

---

## 授權 (License)

本專案採用 MIT 授權 - 詳情請參閱 [LICENSE](LICENSE) 檔案。
